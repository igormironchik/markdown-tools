/**
 * SPDX-FileCopyrightText: (C) 2005 Dominik Seichter <domseichter@web.de>
 * SPDX-FileCopyrightText: (C) 2020 Francesco Pretto <ceztko@gmail.com>
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include <podofo/private/PdfDeclarationsPrivate.h>
#include "PdfImage.h"

#ifdef __MINGW32__
// Workaround <csetjmp> inlcuding <Windows.h> in MINGW
// See https://github.com/podofo/podofo/commit/939ec73578e09aab11012bd38a034a74da1a202c#commitcomment-141054513
#include <podofo/private/WindowsLeanMean.h>
#endif // __MINGW32__

#include <csetjmp>

#ifdef PODOFO_HAVE_TIFF_LIB
#ifdef USE_WIN32_FILEIO
// Workaround possible definition of USE_WIN32_FILEIO, which
// will cause inclusion of Windows.h, ending in clashes.
// It's not really needed, so just undef it
// See https://github.com/podofo/podofo/issues/152
#undef USE_WIN32_FILEIO
#endif // USE_WIN32_FILEIO
extern "C" {
#include <tiffio.h>
}
#endif // PODOFO_HAVE_TIFF_LIB

#include <utf8cpp/utf8.h>

#include <podofo/private/FileSystem.h>
#include <podofo/private/ImageUtils.h>

#include <pdfium/core/fxcodec/fax/faxmodule.h>

#include "PdfDocument.h"
#include "PdfDictionary.h"
#include "PdfArray.h"
#include "PdfColor.h"
#include "PdfObjectStream.h"
#include <podofo/auxiliary/StreamDevice.h>

// TIFF and JPEG headers already included through "PdfFiltersPrivate.h",
// although in opposite order (first JPEG, then TIFF), if available of course

using namespace std;
using namespace PoDoFo;

#ifdef PODOFO_HAVE_PNG_LIB
#include <png.h>
static void pngReadData(png_structp pngPtr, png_bytep data, png_size_t length);
static void loadFromPngContent(PdfImage& image, png_structp png, png_infop info);
static void createPngContext(png_structp& png, png_infop& pnginfo);
#endif // PODOFO_HAVE_PNG_LIB

static void fetchPDFScanLineRGB(unsigned char* dstScanLine,
    unsigned width, const unsigned char* srcScanLine, PdfPixelFormat srcPixelFormat);

PdfImage::PdfImage(PdfDocument& doc)
    : PdfXObject(doc, PdfXObjectType::Image), m_ColorSpace(PdfColorSpaceFilterFactory::GetUnkownInstance()), m_Width(0), m_Height(0), m_BitsPerComponent(0)
{
}

void PdfImage::DecodeTo(charbuff& buffer, PdfPixelFormat format, int scanLineSize) const
{
    buffer.resize(getBufferSize(format));
    SpanStreamDevice stream(buffer);
    DecodeTo(stream, format, scanLineSize);
}

void PdfImage::DecodeTo(const bufferspan& buffer, PdfPixelFormat format, int scanLineSize) const
{
    SpanStreamDevice stream(buffer);
    DecodeTo(stream, format, scanLineSize);
}

// TODO: Improve performance and format support
void PdfImage::DecodeTo(OutputStream& stream, PdfPixelFormat format, int scanLineSize) const
{
    auto istream = GetObject().MustGetStream().GetInputStream();
    auto& mediaFilters = istream.GetMediaFilters();
    charbuff imageData;
    ContainerStreamDevice device(imageData);
    istream.CopyTo(device);

    // TODO: Consider premultiplying alpha for buffer formats
    //  that don't have an alpha chnanel. Consider also opt-out flag
    charbuff smaskData;
    switch (format)
    {
        case PdfPixelFormat::RGBA:
        case PdfPixelFormat::BGRA:
        case PdfPixelFormat::ARGB:
        case PdfPixelFormat::ABGR:
        {
            auto smaskObj = GetDictionary().FindKey("SMask");
            if (smaskObj != nullptr)
            {
                unique_ptr<const PdfImage> smask;
                if (!PdfXObject::TryCreateFromObject(*smaskObj, smask) ||
                    (smask->GetObject().MustGetStream().CopyTo(smaskData), smaskData.size() < (size_t)m_Width * m_Height))
                {
                    PoDoFo::LogMessage(PdfLogSeverity::Warning, "Invalid /SMask");
                    smaskData.clear();
                }
            }
            break;
        }
        default:
            break;
    }

    if (mediaFilters.size() == 0)
    {
        if ((size_t)m_ColorSpace->GetSourceScanLineSize(m_Width, m_BitsPerComponent) * m_Height > imageData.size())
            PODOFO_RAISE_ERROR_INFO(PdfErrorCode::UnsupportedImageFormat, "The source buffer size is too small");

        utls::FetchImage(stream, format, scanLineSize, (const unsigned char*)imageData.data(),
            m_Width, m_Height, m_BitsPerComponent, *m_ColorSpace, smaskData);
    }
    else
    {
        switch (mediaFilters[0])
        {
            case PdfFilterType::DCTDecode:
            {
#ifdef PODOFO_HAVE_JPEG_LIB
                jpeg_decompress_struct ctx;

                JpegErrorHandler jerr;
                try
                {
                    InitJpegDecompressContext(ctx, jerr);

                    PoDoFo::jpeg_memory_src(&ctx, reinterpret_cast<JOCTET*>(imageData.data()), imageData.size());

                    if (jpeg_read_header(&ctx, TRUE) <= 0)
                        PODOFO_RAISE_ERROR(PdfErrorCode::UnexpectedEOF);

                    if (ctx.out_color_space != JCS_CMYK)
                    {
                        // out_color_space must be set after jpeg_read_header() and before jpeg_start_decompress()
                        ctx.out_color_space = format == PdfPixelFormat::Grayscale ? JCS_GRAYSCALE : JCS_RGB;
                    }

                    jpeg_start_decompress(&ctx);

                    utls::FetchImageJPEG(stream, format, scanLineSize, &ctx, m_Width, m_Height, smaskData);
                }
                catch (...)
                {
                    jpeg_destroy_decompress(&ctx);
                    throw;
                }

                jpeg_destroy_decompress(&ctx);
#else
                PODOFO_RAISE_ERROR_INFO(PdfErrorCode::NotImplemented, "Missing jpeg support");
#endif
                break;
            }
            case PdfFilterType::CCITTFaxDecode:
            {
                int k = 0;
                bool endOfLine = false;
                bool encodedByteAlign = false;
                bool blackIs1 = false;
                int columns = 1728;
                int rows = 0;
                auto decodeParms = istream.GetMediaDecodeParms()[0];
                if (decodeParms != nullptr)
                {
                    k = (int)decodeParms->FindKeyAs<int64_t>("K");
                    endOfLine = decodeParms->FindKeyAs<bool>("EndOfLine");
                    encodedByteAlign = decodeParms->FindKeyAs<bool>("EncodedByteAlign");
                    blackIs1 = decodeParms->FindKeyAs<bool>("BlackIs1");
                    columns = (int)decodeParms->FindKeyAs<int64_t>("Columns", 1728);
                    rows = (int)decodeParms->FindKeyAs<int64_t>("Rows");
                }
                auto decoder = fxcodec::FaxModule::CreateDecoder(
                    pdfium::span<const uint8_t>((const uint8_t *)imageData.data(), imageData.size()),
                    (int)m_Width, (int)m_Height, k, endOfLine, encodedByteAlign, blackIs1, columns, rows);

                utls::FetchImageCCITT(stream, format, scanLineSize, *decoder, m_Width, m_Height, smaskData);
                break;
            }
            case PdfFilterType::JBIG2Decode:
            case PdfFilterType::JPXDecode:
            default:
                PODOFO_RAISE_ERROR(PdfErrorCode::UnsupportedFilter);
        }
    }
}

charbuff PdfImage::GetDecodedCopy(PdfPixelFormat format)
{
    charbuff buffer;
    DecodeTo(buffer, format);
    return buffer;
}

PdfImage::PdfImage(PdfObject& obj)
    : PdfXObject(obj, PdfXObjectType::Image)
{
    m_Width = static_cast<unsigned>(this->GetDictionary().FindKeyAsSafe<int64_t>("Width"));
    m_Height = static_cast<unsigned>(this->GetDictionary().FindKeyAsSafe<int64_t>("Height"));
    m_BitsPerComponent = static_cast<unsigned>(this->GetDictionary().FindKeyAsSafe<int64_t>("BitsPerComponent"));

    auto csObj = GetDictionary().FindKey("ColorSpace");
    if (csObj == nullptr || !PdfColorSpaceFilterFactory::TryCreateFromObject(*csObj, m_ColorSpace))
        m_ColorSpace = PdfColorSpaceFilterFactory::GetUnkownInstance();
}

void PdfImage::SetSoftMask(const PdfImage& softmask)
{
    GetDictionary().AddKeyIndirect("SMask"_n, softmask.GetObject());
}

void PdfImage::SetData(const bufferview& buffer, unsigned width, unsigned height, PdfPixelFormat format, int scanLineSize)
{
    SpanStreamDevice stream(buffer);
    SetData(stream, width, height, format, scanLineSize);
}

void PdfImage::SetData(InputStream& stream, unsigned width, unsigned height, PdfPixelFormat format, int scanLineSize)
{
    m_Width = width;
    m_Height = height;
    m_BitsPerComponent = 8;

    PdfColorSpaceType colorSpace;
    unsigned defaultScanLineSize;
    unsigned pdfScanLineSize;
    bool needFetch = false;
    switch (format)
    {
        case PdfPixelFormat::Grayscale:
            colorSpace = PdfColorSpaceType::DeviceGray;
            defaultScanLineSize = 4 * ((width + 3) / 4);
            pdfScanLineSize = width;
            break;
        case PdfPixelFormat::RGB24:
            colorSpace = PdfColorSpaceType::DeviceRGB;
            defaultScanLineSize = 4 * ((3 * width + 3) / 4);
            pdfScanLineSize = 3 * width;
            break;
        case PdfPixelFormat::BGR24:
            colorSpace = PdfColorSpaceType::DeviceRGB;
            defaultScanLineSize = 4 * ((3 * width + 3) / 4);
            pdfScanLineSize = 3 * width;
            needFetch = true;
            break;
        case PdfPixelFormat::RGBA:
        case PdfPixelFormat::BGRA:
        case PdfPixelFormat::ARGB:
        case PdfPixelFormat::ABGR:
            PODOFO_RAISE_ERROR_INFO(PdfErrorCode::NotImplemented, "Missing transparency support");
        default:
            PODOFO_RAISE_ERROR(PdfErrorCode::InvalidEnumValue);
    }

    auto output = GetObject().GetOrCreateStream().GetOutputStream();
    charbuff lineBuffer(scanLineSize < 0 ? defaultScanLineSize : (unsigned)scanLineSize);
    if (needFetch)
    {
        // The format is not compatible with PDF layout
        charbuff pdfLineBuffer(pdfScanLineSize);
        for (unsigned i = 0; i < height; i++)
        {
            stream.Read(lineBuffer.data(), lineBuffer.size());
            fetchPDFScanLineRGB((unsigned char*)pdfLineBuffer.data(), width, (const unsigned char*)lineBuffer.data(), format);
            output.Write(pdfLineBuffer.data(), pdfScanLineSize);
        }
    }
    else
    {
        for (unsigned i = 0; i < height; i++)
        {
            stream.Read(lineBuffer.data(), lineBuffer.size());
            output.Write(lineBuffer.data(), pdfScanLineSize);
        }
    }

    auto& dict = GetDictionary();
    dict.AddKey("Width"_n, static_cast<int64_t>(width));
    dict.AddKey("Height"_n, static_cast<int64_t>(height));
    dict.AddKey("BitsPerComponent"_n, static_cast<int64_t>(8));
    dict.AddKey("ColorSpace"_n, PdfName(PoDoFo::ToString(colorSpace)));
    // Remove possibly existing /Decode array
    dict.RemoveKey("Decode");
}

void PdfImage::SetDataRaw(const bufferview& buffer, const PdfImageInfo& info)
{
    SpanStreamDevice stream(buffer);
    SetDataRaw(stream, info);
}

void PdfImage::SetDataRaw(InputStream& stream, const PdfImageInfo& info)
{
    if (info.ColorSpace.IsNull())
        PODOFO_RAISE_ERROR_INFO(PdfErrorCode::InvalidHandle, "Missing color space");

    m_ColorSpace = info.ColorSpace.GetFilterPtr();
    m_Width = info.Width;
    m_Height = info.Height;
    m_BitsPerComponent = info.BitsPerComponent;

    auto& dict = GetDictionary();
    dict.AddKey("Width"_n, static_cast<int64_t>(info.Width));
    dict.AddKey("Height"_n, static_cast<int64_t>(info.Height));
    dict.AddKey("BitsPerComponent"_n, static_cast<int64_t>(info.BitsPerComponent));
    if (info.DecodeArray.size() == 0)
    {
        dict.RemoveKey("Decode");
    }
    else
    {
        PdfArray decodeArr;
        for (unsigned i = 0; i < info.DecodeArray.size(); i++)
            decodeArr.Add(PdfObject(info.DecodeArray[i]));

        dict.AddKey("Decode"_n, decodeArr);
    }

    dict.AddKey("ColorSpace"_n, info.ColorSpace.GetExportObject(GetDocument().GetObjects()));

    if (info.Filters.has_value())
        GetObject().GetOrCreateStream().SetData(stream, *info.Filters, true);
    else
        GetObject().GetOrCreateStream().SetData(stream);
}

void PdfImage::Load(const string_view& filepath, unsigned imageIndex)
{
    if (filepath.length() > 3)
    {
        auto extension = fs::u8path(filepath).extension().u8string();
        extension = utls::ToLower(extension);

#ifdef PODOFO_HAVE_TIFF_LIB
        if (extension == ".tif" || extension == ".tiff")
        {
            loadFromTiff(filepath, imageIndex);
            return;
        }
#endif

#ifdef PODOFO_HAVE_JPEG_LIB
        if (extension == ".jpg" || extension == ".jpeg")
        {
            loadFromJpeg(filepath);
            return;
        }
#endif

#ifdef PODOFO_HAVE_PNG_LIB
        if (extension == ".png")
        {
            loadFromPng(filepath);
            return;
        }
#endif

    }
    PODOFO_RAISE_ERROR_INFO(PdfErrorCode::UnsupportedImageFormat, filepath);
}

void PdfImage::LoadFromBuffer(const bufferview& buffer, unsigned imageIndex)
{
    if (buffer.size() <= 4)
        return;

    unsigned char magic[4];
    memcpy(magic, buffer.data(), 4);

#ifdef PODOFO_HAVE_TIFF_LIB
    if ((magic[0] == 0x4D &&
        magic[1] == 0x4D &&
        magic[2] == 0x00 &&
        magic[3] == 0x2A) ||
        (magic[0] == 0x49 &&
            magic[1] == 0x49 &&
            magic[2] == 0x2A &&
            magic[3] == 0x00))
    {
        loadFromTiffData((const unsigned char*)buffer.data(), buffer.size(), imageIndex);
        return;
    }
#endif

#ifdef PODOFO_HAVE_JPEG_LIB
    if (magic[0] == 0xFF &&
        magic[1] == 0xD8)
    {
        loadFromJpegData((const unsigned char*)buffer.data(), buffer.size());
        return;
    }
#endif

#ifdef PODOFO_HAVE_PNG_LIB
    if (magic[0] == 0x89 &&
        magic[1] == 0x50 &&
        magic[2] == 0x4E &&
        magic[3] == 0x47)
    {
        loadFromPngData((const unsigned char*)buffer.data(), buffer.size());
        return;
    }
#endif
    PODOFO_RAISE_ERROR_INFO(PdfErrorCode::UnsupportedImageFormat, "Unknown magic number");
}

void PdfImage::ExportTo(charbuff& buff, PdfExportFormat format, PdfArray args) const
{
    buff.clear();
    switch (format)
    {
        case PdfExportFormat::Png:
            PODOFO_RAISE_ERROR(PdfErrorCode::NotImplemented);
        case PdfExportFormat::Jpeg:
#ifdef PODOFO_HAVE_JPEG_LIB
            exportToJpeg(buff, args);
#else
            PODOFO_RAISE_ERROR_INFO(PdfErrorCode::NotImplemented, "Missing jpeg support");
#endif
            break;
        default:
            PODOFO_RAISE_ERROR(PdfErrorCode::InvalidEnumValue);
    }
}

#ifdef PODOFO_HAVE_JPEG_LIB

void PdfImage::loadFromJpeg(const string_view& filename)
{
    FILE* file = utls::fopen(filename, "rb");
    if (file == nullptr)
    {
        PODOFO_RAISE_ERROR_INFO(PdfErrorCode::FileNotFound,
            "{} not found or error opening file", filename);
    }

    jpeg_decompress_struct ctx;
    JpegErrorHandler jerr;
    try
    {
        InitJpegDecompressContext(ctx, jerr);
        jpeg_stdio_src(&ctx, file);

        PdfImageInfo info;
        loadFromJpegInfo(ctx, info);

        FileStreamDevice input(filename);
        this->SetDataRaw(input, info);
    }
    catch (...)
    {
        jpeg_destroy_decompress(&ctx);
        fclose(file);
        throw;
    }

    jpeg_destroy_decompress(&ctx);
    fclose(file);
}

void PdfImage::exportToJpeg(charbuff& destBuff, const PdfArray& args) const
{
    int jquality = 85;
    double quality;
    if (args.GetSize() >= 1 && args[0].TryGetReal(quality))
    {
        // Assume first argument is jpeg quality in range [0, 1]
        jquality = (int)(std::clamp(quality, 0.0, 1.0) * 100);
    }

    charbuff inputBuff;
    DecodeTo(inputBuff, PdfPixelFormat::RGB24);

    jpeg_compress_struct ctx;
    JpegErrorHandler jerr;

    try
    {
        InitJpegCompressContext(ctx, jerr);

        JpegBufferDestination jdest;
        PoDoFo::SetJpegBufferDestination(ctx, destBuff, jdest);

        ctx.image_width = m_Width;
        ctx.image_height = m_Height;
        ctx.input_components = 3;
        ctx.in_color_space = JCS_RGB;

        jpeg_set_defaults(&ctx);

        jpeg_set_quality(&ctx, jquality, TRUE);
        jpeg_start_compress(&ctx, TRUE);

        unsigned scanLineSize = 4 * ((m_Width * 3 + 3) / 4);
        JSAMPROW row_pointer[1];
        for (unsigned i = 0; i < m_Height; i++)
        {
            row_pointer[0] = (unsigned char*)(inputBuff.data() + i * scanLineSize);
            (void)jpeg_write_scanlines(&ctx, row_pointer, 1);
        }

        jpeg_finish_compress(&ctx);
    }
    catch (...)
    {
        jpeg_destroy_compress(&ctx);
        throw;
    }

    jpeg_destroy_compress(&ctx);
}

void PdfImage::loadFromJpegData(const unsigned char* data, size_t len)
{
    jpeg_decompress_struct ctx;
    JpegErrorHandler jerr;

    try
    {
        InitJpegDecompressContext(ctx, jerr);
        jpeg_memory_src(&ctx, data, len);

        PdfImageInfo info;
        loadFromJpegInfo(ctx, info);

        SpanStreamDevice input((const char*)data, len);
        this->SetDataRaw(input, info);
    }
    catch (...)
    {
        jpeg_destroy_decompress(&ctx);
        throw;
    }
    jpeg_destroy_decompress(&ctx);
}

void PdfImage::loadFromJpegInfo(jpeg_decompress_struct& ctx, PdfImageInfo& info)
{
    if (jpeg_read_header(&ctx, TRUE) <= 0)
    {
        jpeg_destroy_decompress(&ctx);
        PODOFO_RAISE_ERROR(PdfErrorCode::UnexpectedEOF);
    }

    jpeg_start_decompress(&ctx);

    info.Width = ctx.output_width;
    info.Height = ctx.output_height;
    info.BitsPerComponent = 8;
    info.Filters = { PdfFilterType::DCTDecode };

    // I am not sure whether this switch is fully correct.
    // it should handle all cases though.
    // Index jpeg files might look strange as jpeglib+
    // returns 1 for them.
    switch (ctx.output_components)
    {
        case 3:
        {
            info.ColorSpace = PdfColorSpaceFilterFactory::GetDeviceRGBInstace();
            break;
        }
        case 4:
        {
            info.ColorSpace = PdfColorSpaceFilterFactory::GetDeviceCMYKInstace();

            // The jpeg-doc isn't specific on this point, but cmyk's seem to be stored
            // in an inverted fashion. Fix by attaching a decode array
            info.DecodeArray.push_back(1.0);
            info.DecodeArray.push_back(0.0);
            info.DecodeArray.push_back(1.0);
            info.DecodeArray.push_back(0.0);
            info.DecodeArray.push_back(1.0);
            info.DecodeArray.push_back(0.0);
            info.DecodeArray.push_back(1.0);
            info.DecodeArray.push_back(0.0);
            break;
        }
        default:
        {
            info.ColorSpace = PdfColorSpaceFilterFactory::GetDeviceGrayInstace();
            break;
        }
    }
}

#endif // PODOFO_HAVE_JPEG_LIB

#ifdef PODOFO_HAVE_TIFF_LIB

static void TIFFErrorWarningHandler(const char*, const char*, va_list)
{
    // Do nothing
}

void PdfImage::loadFromTiffHandle(void* handle, unsigned imageIndex)
{
    TIFF* hInTiffHandle = (TIFF*)handle;

    int32_t row, width, height;
    uint16_t samplesPerPixel, bitsPerSample;
    uint16_t* sampleInfo;
    uint16_t extraSamples;
    uint16_t planarConfig, photoMetric, orientation;
    int32_t resolutionUnit;

    // Set the page/image index in the tiff context
    TIFFSetDirectory(hInTiffHandle, (uint16_t)imageIndex);

    TIFFGetField(hInTiffHandle, TIFFTAG_IMAGEWIDTH, &width);
    TIFFGetField(hInTiffHandle, TIFFTAG_IMAGELENGTH, &height);
    TIFFGetFieldDefaulted(hInTiffHandle, TIFFTAG_BITSPERSAMPLE, &bitsPerSample);
    TIFFGetFieldDefaulted(hInTiffHandle, TIFFTAG_SAMPLESPERPIXEL, &samplesPerPixel);
    TIFFGetFieldDefaulted(hInTiffHandle, TIFFTAG_PLANARCONFIG, &planarConfig);
    TIFFGetFieldDefaulted(hInTiffHandle, TIFFTAG_PHOTOMETRIC, &photoMetric);
    TIFFGetFieldDefaulted(hInTiffHandle, TIFFTAG_EXTRASAMPLES, &extraSamples, &sampleInfo);
    TIFFGetFieldDefaulted(hInTiffHandle, TIFFTAG_ORIENTATION, &orientation);

    resolutionUnit = 0;
    float resX;
    float resY;
    TIFFGetFieldDefaulted(hInTiffHandle, TIFFTAG_XRESOLUTION, &resX);
    TIFFGetFieldDefaulted(hInTiffHandle, TIFFTAG_YRESOLUTION, &resY);
    TIFFGetFieldDefaulted(hInTiffHandle, TIFFTAG_RESOLUTIONUNIT, &resolutionUnit);

    int colorChannels = samplesPerPixel - extraSamples;

    int bitsPixel = bitsPerSample * samplesPerPixel;

    // TODO: implement special cases
    if (TIFFIsTiled(hInTiffHandle))
        PODOFO_RAISE_ERROR(PdfErrorCode::UnsupportedImageFormat);

    if (planarConfig != PLANARCONFIG_CONTIG && colorChannels != 1)
        PODOFO_RAISE_ERROR(PdfErrorCode::UnsupportedImageFormat);

    PdfImageInfo info;
    info.Width = width;
    info.Height = height;
    info.BitsPerComponent = (unsigned char)bitsPerSample;
    switch (photoMetric)
    {
        case PHOTOMETRIC_MINISBLACK:
        {
            if (bitsPixel == 1)
            {
                info.DecodeArray.push_back(0);
                info.DecodeArray.push_back(1);
                info.ColorSpace = PdfColorSpaceFilterFactory::GetDeviceGrayInstace();
            }
            else if (bitsPixel == 8 || bitsPixel == 16)
            {
                info.ColorSpace = PdfColorSpaceFilterFactory::GetDeviceGrayInstace();
            }
            else
            {
                PODOFO_RAISE_ERROR(PdfErrorCode::UnsupportedImageFormat);
            }
            break;
        }
        case PHOTOMETRIC_MINISWHITE:
        {
            if (bitsPixel == 1)
            {
                info.DecodeArray.push_back(1);
                info.DecodeArray.push_back(0);
                info.ColorSpace = PdfColorSpaceFilterFactory::GetDeviceGrayInstace();
            }
            else if (bitsPixel == 8 || bitsPixel == 16)
            {
                info.ColorSpace = PdfColorSpaceFilterFactory::GetDeviceGrayInstace();
            }
            else
            {
                PODOFO_RAISE_ERROR(PdfErrorCode::UnsupportedImageFormat);
            }
            break;
        }
        case PHOTOMETRIC_RGB:
        {
            if (bitsPixel != 24)
                PODOFO_RAISE_ERROR(PdfErrorCode::UnsupportedImageFormat);

            info.ColorSpace = PdfColorSpaceFilterFactory::GetDeviceRGBInstace();
            break;
        }
        case PHOTOMETRIC_SEPARATED:
        {
            if (bitsPixel != 32)
                PODOFO_RAISE_ERROR(PdfErrorCode::UnsupportedImageFormat);

            info.ColorSpace = PdfColorSpaceFilterFactory::GetDeviceCMYKInstace();
            break;
        }
        case PHOTOMETRIC_PALETTE:
        {
            unsigned numColors = (1 << bitsPixel);
            info.DecodeArray.push_back(0);
            info.DecodeArray.push_back(numColors - 1);

            uint16_t* rgbRed;
            uint16_t* rgbGreen;
            uint16_t* rgbBlue;
            TIFFGetField(hInTiffHandle, TIFFTAG_COLORMAP, &rgbRed, &rgbGreen, &rgbBlue);

            charbuff data(numColors * 3);

            for (unsigned clr = 0; clr < numColors; clr++)
            {
                data[3 * clr + 0] = (char)(rgbRed[clr] / 257);
                data[3 * clr + 1] = (char)(rgbGreen[clr] / 257);
                data[3 * clr + 2] = (char)(rgbBlue[clr] / 257);
            }

            // Create a colorspace object
            auto& idxObj = this->GetDocument().GetObjects().CreateDictionaryObject();
            idxObj.GetOrCreateStream().SetData(data);

            // Add the colorspace to our image
            info.ColorSpace = PdfColorSpaceInitializer(std::make_shared<PdfColorSpaceFilterIndexed>(
                PdfColorSpaceFilterFactory::GetDeviceRGBInstace(), numColors, std::move(data)));
            break;
        }

        default:
        {
            PODOFO_RAISE_ERROR(PdfErrorCode::UnsupportedImageFormat);
            break;
        }
    }

    size_t scanlineSize = TIFFScanlineSize(hInTiffHandle);
    size_t bufferSize = scanlineSize * height;
    charbuff buffer(bufferSize);
    for (row = 0; row < height; row++)
    {
        if (TIFFReadScanline(hInTiffHandle,
            &buffer[row * scanlineSize],
            row) == (-1))
        {
            PODOFO_RAISE_ERROR(PdfErrorCode::UnsupportedImageFormat);
        }
    }

    SpanStreamDevice input(buffer);
    SetDataRaw(input, info);
}

void PdfImage::loadFromTiff(const string_view& filename, unsigned imageIndex)
{
    TIFFSetErrorHandler(TIFFErrorWarningHandler);
    TIFFSetWarningHandler(TIFFErrorWarningHandler);

    if (filename.length() == 0)
        PODOFO_RAISE_ERROR(PdfErrorCode::InvalidHandle);

#ifdef _WIN32
    auto filename16 = utf8::utf8to16((string)filename);
    TIFF* hInfile = TIFFOpenW((wchar_t*)filename16.c_str(), "rb");
#else
    TIFF* hInfile = TIFFOpen(filename.data(), "rb");
#endif

    if (hInfile == nullptr)
    {
        PODOFO_RAISE_ERROR_INFO(PdfErrorCode::FileNotFound,
            "{} not found or error opening file", filename);
    }

    try
    {
        loadFromTiffHandle(hInfile, imageIndex);
    }
    catch (...)
    {
        TIFFClose(hInfile);
        throw;
    }

    TIFFClose(hInfile);
}

struct TiffData
{
    TiffData(const unsigned char* data, tsize_t size) :m_data(data), m_pos(0), m_size(size) {}

    tsize_t read(tdata_t data, tsize_t length)
    {
        tsize_t bytesRead = 0;
        if (length > m_size - static_cast<tsize_t>(m_pos))
        {
            memcpy(data, &m_data[m_pos], m_size - (tsize_t)m_pos);
            bytesRead = m_size - (tsize_t)m_pos;
            m_pos = m_size;
        }
        else
        {
            memcpy(data, &m_data[m_pos], length);
            bytesRead = length;
            m_pos += length;
        }
        return bytesRead;
    }

    toff_t size()
    {
        return m_size;
    }

    toff_t seek(toff_t pos, int whence)
    {
        if (pos == 0xFFFFFFFF) {
            return 0xFFFFFFFF;
        }
        switch (whence)
        {
            case SEEK_SET:
                if (static_cast<tsize_t>(pos) > m_size)
                {
                    m_pos = m_size;
                }
                else
                {
                    m_pos = pos;
                }
                break;
            case SEEK_CUR:
                if (static_cast<tsize_t>(pos + m_pos) > m_size)
                {
                    m_pos = m_size;
                }
                else
                {
                    m_pos += pos;
                }
                break;
            case SEEK_END:
                if (static_cast<tsize_t>(pos) > m_size)
                {
                    m_pos = 0;
                }
                else
                {
                    m_pos = m_size - pos;
                }
                break;
        }
        return m_pos;
    }

private:
    const unsigned char* m_data;
    toff_t m_pos;
    tsize_t m_size;
};
tsize_t tiff_Read(thandle_t st, tdata_t buffer, tsize_t size)
{
    TiffData* data = (TiffData*)st;
    return data->read(buffer, size);
};
tsize_t tiff_Write(thandle_t st, tdata_t buffer, tsize_t size)
{
    (void)st;
    (void)buffer;
    (void)size;
    return 0;
};
int tiff_Close(thandle_t)
{
    return 0;
};
toff_t tiff_Seek(thandle_t st, toff_t pos, int whence)
{
    TiffData* data = (TiffData*)st;
    return data->seek(pos, whence);
};
toff_t tiff_Size(thandle_t st)
{
    TiffData* data = (TiffData*)st;
    return data->size();
};
int tiff_Map(thandle_t, tdata_t*, toff_t*)
{
    return 0;
};
void tiff_Unmap(thandle_t, tdata_t, toff_t)
{
    return;
};
void PdfImage::loadFromTiffData(const unsigned char* data, size_t len, unsigned imageIndex)
{
    TIFFSetErrorHandler(TIFFErrorWarningHandler);
    TIFFSetWarningHandler(TIFFErrorWarningHandler);

    if (data == nullptr)
        PODOFO_RAISE_ERROR(PdfErrorCode::InvalidHandle);

    TiffData tiffData(data, (tsize_t)len);
    TIFF* hInHandle = TIFFClientOpen("Memory", "r", (thandle_t)&tiffData,
        tiff_Read, tiff_Write, tiff_Seek, tiff_Close, tiff_Size,
        tiff_Map, tiff_Unmap);
    if (hInHandle == nullptr)
        PODOFO_RAISE_ERROR(PdfErrorCode::InvalidHandle);

    try
    {
        loadFromTiffHandle(hInHandle, imageIndex);
    }
    catch (...)
    {
        TIFFClose(hInHandle);
        throw;
    }

    TIFFClose(hInHandle);
}

#endif // PODOFO_HAVE_TIFF_LIB

#ifdef PODOFO_HAVE_PNG_LIB

void PdfImage::loadFromPng(const string_view& filename)
{
    FILE* file = utls::fopen(filename, "rb");
    if (file == nullptr)
    {
        PODOFO_RAISE_ERROR_INFO(PdfErrorCode::FileNotFound,
            "{} not found or error opening file", filename);
    }

    try
    {
        loadFromPngHandle(file);
    }
    catch (...)
    {
        fclose(file);
        throw;
    }

    fclose(file);
}

void PdfImage::loadFromPngHandle(FILE* stream)
{
    png_byte header[8];
    if (fread(header, 1, 8, stream) != 8 ||
        png_sig_cmp(header, 0, 8))
    {
        PODOFO_RAISE_ERROR_INFO(PdfErrorCode::UnsupportedImageFormat, "The file could not be recognized as a PNG file");
    }

    png_structp png = nullptr;
    png_infop pnginfo = nullptr;
    try
    {
        createPngContext(png, pnginfo);
        png_init_io(png, stream);
        loadFromPngContent(*this, png, pnginfo);
    }
    catch (...)
    {
        png_destroy_read_struct(&png, &pnginfo, (png_infopp)nullptr);
        throw;
    }

    png_destroy_read_struct(&png, &pnginfo, (png_infopp)nullptr);
}

struct PngData
{
    PngData(const unsigned char* data, png_size_t size) :
        m_data(data), m_pos(0), m_size(size) {}

    void read(png_bytep data, png_size_t length)
    {
        if (length > m_size - m_pos)
        {
            memcpy(data, &m_data[m_pos], m_size - m_pos);
            m_pos = m_size;
        }
        else
        {
            memcpy(data, &m_data[m_pos], length);
            m_pos += length;
        }
    }

private:
    const unsigned char* m_data;
    png_size_t m_pos;
    png_size_t m_size;
};

void PdfImage::loadFromPngData(const unsigned char* data, size_t len)
{
    if (data == nullptr)
        PODOFO_RAISE_ERROR(PdfErrorCode::InvalidHandle);

    PngData pngData(data, len);
    png_byte header[8];
    pngData.read(header, 8);
    if (png_sig_cmp(header, 0, 8))
        PODOFO_RAISE_ERROR_INFO(PdfErrorCode::UnsupportedImageFormat, "The file could not be recognized as a PNG file");

    png_structp png;
    png_infop pnginfo;
    try
    {
        createPngContext(png, pnginfo);
        png_set_read_fn(png, (png_voidp)&pngData, pngReadData);
        loadFromPngContent(*this, png, pnginfo);
    }
    catch (...)
    {
        png_destroy_read_struct(&png, &pnginfo, (png_infopp)nullptr);
        throw;
    }

    png_destroy_read_struct(&png, &pnginfo, (png_infopp)nullptr);
}

void loadFromPngContent(PdfImage& image, png_structp png, png_infop pnginfo)
{
    png_set_sig_bytes(png, 8);
    png_read_info(png, pnginfo);

    // Begin
    png_uint_32 width;
    png_uint_32 height;
    int depth;
    int color_type;
    int interlace;

    png_get_IHDR(png, pnginfo,
        &width, &height, &depth,
        &color_type, &interlace, NULL, NULL);

    // convert palette/gray image to rgb
    // expand gray bit depth if needed
    if (color_type == PNG_COLOR_TYPE_GRAY)
    {
#if PNG_LIBPNG_VER >= 10209
        png_set_expand_gray_1_2_4_to_8(png);
#else
        png_set_gray_1_2_4_to_8(pPng);
#endif
    }
    else if (color_type != PNG_COLOR_TYPE_PALETTE && depth < 8)
    {
        png_set_packing(png);
    }

    // transform transparency to alpha
    if (color_type != PNG_COLOR_TYPE_PALETTE && png_get_valid(png, pnginfo, PNG_INFO_tRNS))
        png_set_tRNS_to_alpha(png);

    if (depth == 16)
        png_set_strip_16(png);

    if (interlace != PNG_INTERLACE_NONE)
        png_set_interlace_handling(png);

    //png_set_filler (pPng, 0xff, PNG_FILLER_AFTER);

    // recheck header after setting EXPAND options
    png_read_update_info(png, pnginfo);
    png_get_IHDR(png, pnginfo,
        &width, &height, &depth,
        &color_type, &interlace, NULL, NULL);
    // End

    // Read the file
    size_t rowLen = png_get_rowbytes(png, pnginfo);
    size_t len = rowLen * height;
    charbuff buffer(len);

    unique_ptr<png_bytep[]> rows(new png_bytep[height]);
    for (unsigned int y = 0; y < height; y++)
    {
        rows[y] = reinterpret_cast<png_bytep>(buffer.data() + y * rowLen);
    }

    png_read_image(png, rows.get());

    png_bytep paletteTrans = nullptr;
    int numTransColors = 0;
    if (color_type & PNG_COLOR_MASK_ALPHA
        || (color_type == PNG_COLOR_TYPE_PALETTE
            && png_get_valid(png, pnginfo, PNG_INFO_tRNS)
            && png_get_tRNS(png, pnginfo, &paletteTrans, &numTransColors, NULL)))
    {
        // Handle alpha channel and create smask
        charbuff smask((size_t)width * height);
        png_uint_32 smaskIndex = 0;
        if (color_type == PNG_COLOR_TYPE_PALETTE)
        {
            for (png_uint_32 r = 0; r < height; r++)
            {
                png_bytep row = rows[r];
                for (png_uint_32 c = 0; c < width; c++)
                {
                    png_byte color;
                    switch (depth)
                    {
                        case 8:
                            color = row[c];
                            break;
                        case 4:
                            color = c % 2 ? row[c / 2] >> 4 : row[c / 2] & 0xF;
                            break;
                        case 2:
                            color = (row[c / 4] >> c % 4 * 2) & 3;
                            break;
                        case 1:
                            color = (row[c / 4] >> c % 8) & 1;
                            break;
                        default:
                            PODOFO_RAISE_ERROR(PdfErrorCode::InvalidEnumValue);
                    }

                    smask[smaskIndex++] = color < numTransColors ? paletteTrans[color] : 0xFF;
                }
            }
        }
        else if (color_type == PNG_COLOR_TYPE_RGB_ALPHA)
        {
            for (png_uint_32 r = 0; r < height; r++)
            {
                png_bytep row = rows[r];
                for (png_uint_32 c = 0; c < width; c++)
                {
                    memmove(buffer.data() + 3 * smaskIndex, row + 4 * c, 3); // 3 byte for rgb
                    smask[smaskIndex++] = row[c * 4 + 3]; // 4th byte for alpha
                }
            }
            len = 3 * (size_t)width * height;
        }
        else if (color_type == PNG_COLOR_TYPE_GRAY_ALPHA)
        {
            for (png_uint_32 r = 0; r < height; r++)
            {
                png_bytep row = rows[r];
                for (png_uint_32 c = 0; c < width; c++)
                {
                    buffer[smaskIndex] = row[c * 2]; // 1 byte for gray
                    smask[smaskIndex++] = row[c * 2 + 1]; // 2nd byte for alpha
                }
            }
            len = (size_t)width * height;
        }
        PdfImageInfo smaksInfo;
        smaksInfo.Width = (unsigned)width;
        smaksInfo.Height = (unsigned)height;
        smaksInfo.BitsPerComponent = (unsigned char)depth;
        smaksInfo.ColorSpace = PdfColorSpaceFilterFactory::GetDeviceGrayInstace();

        auto smakeImage = image.GetDocument().CreateImage();
        smakeImage->SetDataRaw(smask, smaksInfo);
        image.SetSoftMask(*smakeImage);
    }

    PdfImageInfo info;
    info.Width = (unsigned)width;
    info.Height = (unsigned)height;
    info.BitsPerComponent = (unsigned char)depth;
    // Set color space
    if (color_type == PNG_COLOR_TYPE_PALETTE)
    {
        png_color* colors;
        int colorCount;
        png_get_PLTE(png, pnginfo, &colors, &colorCount);

        charbuff data(colorCount * 3);
        for (int i = 0; i < colorCount; i++, colors++)
        {
            data[3 * i + 0] = colors->red;
            data[3 * i + 1] = colors->green;
            data[3 * i + 2] = colors->blue;
        }

        info.ColorSpace = PdfColorSpaceInitializer(std::make_shared<PdfColorSpaceFilterIndexed>(
            PdfColorSpaceFilterFactory::GetDeviceRGBInstace(), colorCount, std::move(data)));
    }
    else if (color_type == PNG_COLOR_TYPE_GRAY || color_type == PNG_COLOR_TYPE_GRAY_ALPHA)
    {
        info.ColorSpace = PdfColorSpaceFilterFactory::GetDeviceGrayInstace();
    }
    else
    {
        info.ColorSpace = PdfColorSpaceFilterFactory::GetDeviceRGBInstace();
    }

    // Set the image data and flate compress it
    image.SetDataRaw(buffer, info);
}

void createPngContext(png_structp& png, png_infop& pnginfo)
{
    png = png_create_read_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
    if (png == nullptr)
        PODOFO_RAISE_ERROR_INFO(PdfErrorCode::InvalidHandle, "png_create_read_struct");

    pnginfo = png_create_info_struct(png);
    if (pnginfo == nullptr)
        PODOFO_RAISE_ERROR_INFO(PdfErrorCode::InvalidHandle, "png_create_info_struct");

    if (setjmp(png_jmpbuf(png)))
        PODOFO_RAISE_ERROR_INFO(PdfErrorCode::InvalidHandle, "Error when reading the image");
}

void pngReadData(png_structp pngPtr, png_bytep data, png_size_t length)
{
    PngData* a = (PngData*)png_get_io_ptr(pngPtr);
    a->read(data, length);
}

#endif // PODOFO_HAVE_PNG_LIB

void PdfImage::SetChromaKeyMask(int64_t r, int64_t g, int64_t b, int64_t threshold)
{
    PdfArray array;
    array.Add(r - threshold);
    array.Add(r + threshold);
    array.Add(g - threshold);
    array.Add(g + threshold);
    array.Add(b - threshold);
    array.Add(b + threshold);

    this->GetDictionary().AddKey("Mask"_n, array);
}

void PdfImage::SetInterpolate(bool value)
{
    this->GetDictionary().AddKey("Interpolate"_n, value);
}

Rect PdfImage::GetRect() const
{
    return Rect(0, 0, m_Width, m_Height);
}

unsigned PdfImage::getBufferSize(PdfPixelFormat format) const
{
    switch (format)
    {
        case PdfPixelFormat::RGBA:
        case PdfPixelFormat::BGRA:
        case PdfPixelFormat::ARGB:
        case PdfPixelFormat::ABGR:
            return 4 * m_Width * m_Height;
        case PdfPixelFormat::RGB24:
        case PdfPixelFormat::BGR24:
            return 4 * ((3 * m_Width + 3) / 4) * m_Height;
        case PdfPixelFormat::Grayscale:
            return 4 * ((m_Width + 3) / 4) * m_Height;
        default:
            PODOFO_RAISE_ERROR(PdfErrorCode::InvalidEnumValue);
    }
}

void fetchPDFScanLineRGB(unsigned char* dstScanLine, unsigned width, const unsigned char* srcScanLine, PdfPixelFormat srcPixelFormat)
{
    switch (srcPixelFormat)
    {
        case PdfPixelFormat::BGR24:
        case PdfPixelFormat::BGRA:
        {
            for (unsigned i = 0; i < width; i++)
            {
                dstScanLine[i * 3 + 0] = srcScanLine[i * 3 + 2];
                dstScanLine[i * 3 + 1] = srcScanLine[i * 3 + 1];
                dstScanLine[i * 3 + 2] = srcScanLine[i * 3 + 0];
            }
            break;
        }
        case PdfPixelFormat::ABGR:
        {
            for (unsigned i = 0; i < width; i++)
            {
                dstScanLine[i * 3 + 0] = srcScanLine[i * 3 + 3];
                dstScanLine[i * 3 + 1] = srcScanLine[i * 3 + 2];
                dstScanLine[i * 3 + 2] = srcScanLine[i * 3 + 1];
            }
            break;
        }
        default:
            PODOFO_RAISE_ERROR_INFO(PdfErrorCode::UnsupportedPixelFormat, "Unsupported pixel format");
    }
}

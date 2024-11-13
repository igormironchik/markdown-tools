/**
 * SPDX-FileCopyrightText: (C) 2022 Francesco Pretto <ceztko@gmail.com>
 * SPDX-License-Identifier: LGPL-2.0-or-later
 * SPDX-License-Identifier: MPL-2.0
 */

#ifndef PDF_COLOR_SPACE_FILTER_H
#define PDF_COLOR_SPACE_FILTER_H

#include "PdfColor.h"

namespace PoDoFo {

    class PdfIndirectObjectList;

    /** Output pixel format for a PdfColorSpaceFilter
     */
    enum class PdfColorSpacePixelFormat
    {
        Unknown = 0,
        Grayscale,
        RGB,
        CMYK,
        // TODO:
        // Custom    ///< Used for /DeviceN colorspaces
    };

    /** A class that implements methods to sample colors from a scanline buffer
     */
    class PODOFO_API PdfColorSpaceFilter
    {
        friend class PdfColorSpaceFilterUnkown;
        friend class PdfColorSpaceDeviceGray;
        friend class PdfColorSpaceFilterDeviceRGB;
        friend class PdfColorSpaceFilterDeviceCMYK;
        friend class PdfColorSpaceFilterIndexed;
        friend class PdfColorSpaceFilterSeparation;
        friend class PdfColorSpaceFilterLab;
        friend class PdfColorSpaceFilterICCBased;

    private:
        PdfColorSpaceFilter();
    public:
        virtual ~PdfColorSpaceFilter();
        virtual PdfColorSpaceType GetType() const = 0;
        /** True if the code space doesn't perform any non-trivial
         * encoding/filtering. In other words pixels can be sampled
         * by just copying scan lines
         */ 
        virtual bool IsRawEncoded() const;
        /** True if the color space is fully identified by its name
         */
        virtual bool IsTrivial() const;
        /** Get the output pixel format of this color space
         */
        virtual PdfColorSpacePixelFormat GetPixelFormat() const = 0;
        /** Get the size of the scan line to sample from
         */
        virtual unsigned GetSourceScanLineSize(unsigned width, unsigned bitsPerComponent) const = 0;
        /** Get the size of the scan line to sample to
         */
        virtual unsigned GetScanLineSize(unsigned width, unsigned bitsPerComponent) const = 0;
        /** Fetch the actual scanline of the exported format from/to the given buffers
         */
        virtual void FetchScanLine(unsigned char* dstScanLine, const unsigned char* srcScanLine,
            unsigned width, unsigned bitsPerComponent) const = 0;
        /** Get an export object
         */
        virtual PdfObject GetExportObject(PdfIndirectObjectList& objectsj) const = 0;
        /** Get the number of the color components for this color space
         */
        virtual unsigned char GetColorComponentCount() const = 0;
    };

    /** Convenience alias for a constant PdfColorSpaceFilter shared ptr
     */
    using PdfColorSpaceFilterPtr = std::shared_ptr<const PdfColorSpaceFilter>;

    /** Unknown color space filter that default throws on implementations
     */
    class PODOFO_API PdfColorSpaceFilterUnkown final : public PdfColorSpaceFilter
    {
        friend class PdfColorSpaceFilterFactory;
    private:
        PdfColorSpaceFilterUnkown();
    public:
        PdfColorSpaceType GetType() const override;
        PdfColorSpacePixelFormat GetPixelFormat() const override;
        unsigned GetSourceScanLineSize(unsigned width, unsigned bitsPerComponent) const override;
        unsigned GetScanLineSize(unsigned width, unsigned bitsPerComponent) const override;
        void FetchScanLine(unsigned char* dstScanLine, const unsigned char* srcScanLine,
            unsigned width, unsigned bitsPerComponent) const override;
        PdfObject GetExportObject(PdfIndirectObjectList& objects) const override;
        unsigned char GetColorComponentCount() const override;
    };

    class PODOFO_API PdfColorSpaceDeviceGray final : public PdfColorSpaceFilter
    {
        friend class PdfColorSpaceFilterFactory;
    private:
        PdfColorSpaceDeviceGray();
    public:
        PdfColorSpaceType GetType() const override;
        bool IsRawEncoded() const override;
        bool IsTrivial() const override;
        PdfColorSpacePixelFormat GetPixelFormat() const override;
        unsigned GetSourceScanLineSize(unsigned width, unsigned bitsPerComponent) const override;
        unsigned GetScanLineSize(unsigned width, unsigned bitsPerComponent) const override;
        void FetchScanLine(unsigned char* dstScanLine, const unsigned char* srcScanLine,
            unsigned width, unsigned bitsPerComponent) const override;
        PdfObject GetExportObject(PdfIndirectObjectList& objects) const override;
        unsigned char GetColorComponentCount() const override;
    };

    class PODOFO_API PdfColorSpaceFilterDeviceRGB final : public PdfColorSpaceFilter
    {
        friend class PdfColorSpaceFilterFactory;
    private:
        PdfColorSpaceFilterDeviceRGB();
    public:
        PdfColorSpaceType GetType() const override;
        bool IsRawEncoded() const override;
        bool IsTrivial() const override;
        PdfColorSpacePixelFormat GetPixelFormat() const override;
        unsigned GetSourceScanLineSize(unsigned width, unsigned bitsPerComponent) const override;
        unsigned GetScanLineSize(unsigned width, unsigned bitsPerComponent) const override;
        void FetchScanLine(unsigned char* dstScanLine, const unsigned char* srcScanLine,
            unsigned width, unsigned bitsPerComponent) const override;
        PdfObject GetExportObject(PdfIndirectObjectList& objects) const override;
        unsigned char GetColorComponentCount() const override;
    };

    class PODOFO_API PdfColorSpaceFilterDeviceCMYK final : public PdfColorSpaceFilter
    {
        friend class PdfColorSpaceFilterFactory;
    private:
        PdfColorSpaceFilterDeviceCMYK();
    public:
        PdfColorSpaceType GetType() const override;
        bool IsRawEncoded() const override;
        bool IsTrivial() const override;
        PdfColorSpacePixelFormat GetPixelFormat() const override;
        unsigned GetSourceScanLineSize(unsigned width, unsigned bitsPerComponent) const override;
        unsigned GetScanLineSize(unsigned width, unsigned bitsPerComponent) const override;
        void FetchScanLine(unsigned char* dstScanLine, const unsigned char* srcScanLine,
            unsigned width, unsigned bitsPerComponent) const override;
        PdfObject GetExportObject(PdfIndirectObjectList& objects) const override;
        unsigned char GetColorComponentCount() const override;
    };

    /** Color space as described by ISO 32000-2:2020 "8.6.6.3 Indexed colour spaces"
     */
    class PODOFO_API PdfColorSpaceFilterIndexed final : public PdfColorSpaceFilter
    {
    public:
        PdfColorSpaceFilterIndexed(const PdfColorSpaceFilterPtr& baseColorSpace, unsigned mapSize, charbuff&& lookup);
    public:
        PdfColorSpaceType GetType() const override;
        PdfColorSpacePixelFormat GetPixelFormat() const override;
        unsigned GetSourceScanLineSize(unsigned width, unsigned bitsPerComponent) const override;
        unsigned GetScanLineSize(unsigned width, unsigned bitsPerComponent) const override;
        void FetchScanLine(unsigned char* dstScanLine, const unsigned char* srcScanLine,
            unsigned width, unsigned bitsPerComponent) const override;
        PdfObject GetExportObject(PdfIndirectObjectList& objects) const override;
        unsigned char GetColorComponentCount() const override;
    private:
        PdfColorSpaceFilterPtr m_BaseColorSpace;
        unsigned m_MapSize;
        charbuff m_lookup;
    };

    class PODOFO_API PdfColorSpaceFilterLab final : public PdfColorSpaceFilter
    {
    public:
        PdfColorSpaceFilterLab(const std::array<double, 3>& whitePoint,
            nullable<const std::array<double, 3>&> blackPoint = nullptr,
            nullable<const std::array<double, 4>&> range = nullptr);
    public:
        PdfColorSpaceType GetType() const override;
        bool IsRawEncoded() const override;
        PdfColorSpacePixelFormat GetPixelFormat() const override;
        unsigned GetSourceScanLineSize(unsigned width, unsigned bitsPerComponent) const override;
        unsigned GetScanLineSize(unsigned width, unsigned bitsPerComponent) const override;
        void FetchScanLine(unsigned char* dstScanLine, const unsigned char* srcScanLine,
            unsigned width, unsigned bitsPerComponent) const override;
        PdfObject GetExportObject(PdfIndirectObjectList& objects) const override;
        unsigned char GetColorComponentCount() const override;
    private:
        std::array<double, 3> m_WhitePoint;
        std::array<double, 3> m_BlackPoint;
        std::array<double, 4> m_Range;
    };

    class PODOFO_API PdfColorSpaceFilterSeparation final : public PdfColorSpaceFilter
    {
    public:
        /** Create a new PdfColor object with
         *  a separation-name and an equivalent color
         *
         *  \param name Name of the separation color
         *  \param density the density value of the separation color
         *  \param alternateColor the alternate color, must be of type gray, rgb, cmyk or cie
         */
        PdfColorSpaceFilterSeparation(const std::string_view& name, const PdfColor& alternateColor);

        /** Create a new PdfColor object with
         *  Separation color None.
         *
         */
        static std::unique_ptr<PdfColorSpaceFilterSeparation> CreateSeparationNone();

        /** Create a new PdfColor object with
         *  Separation color All.
         *
         */
        static std::unique_ptr<PdfColorSpaceFilterSeparation> CreateSeparationAll();

    public:
        PdfColorSpaceType GetType() const override;
        bool IsRawEncoded() const override;
        PdfColorSpacePixelFormat GetPixelFormat() const override;
        unsigned GetSourceScanLineSize(unsigned width, unsigned bitsPerComponent) const override;
        unsigned GetScanLineSize(unsigned width, unsigned bitsPerComponent) const override;
        void FetchScanLine(unsigned char* dstScanLine, const unsigned char* srcScanLine,
            unsigned width, unsigned bitsPerComponent) const override;
        PdfObject GetExportObject(PdfIndirectObjectList& objects) const override;
        unsigned char GetColorComponentCount() const override;

    public:
        const std::string& GetName() const { return m_Name; }
        const PdfColorRaw& GetAlternateColor() const;
        const PdfColorSpaceFilter& GetColorSpace() const;

    private:
        std::string m_Name;
        PdfColor m_AlternateColor;
    };

    class PODOFO_API PdfColorSpaceFilterFactory final
    {
    public:
        static bool TryCreateFromObject(const PdfObject& obj, PdfColorSpaceFilterPtr& colorSpace);

        /** True if the filter is trivial like /DeviceRGB, /DeviceGray or /DeviceCMYK
         */
        static PdfColorSpaceFilterPtr GetTrivialFilter(PdfColorSpaceType type);

        /** Singleton method which returns a global instance
         *  of Unknown color space
         */
        static PdfColorSpaceFilterPtr GetUnkownInstance();

        /** Singleton method which returns a global instance
         *  of /DeviceGray color space
         */
        static PdfColorSpaceFilterPtr GetDeviceGrayInstace();

        /** Singleton method which returns a global instance
         *  of /DeviceRGB color space
         */
        static PdfColorSpaceFilterPtr GetDeviceRGBInstace();

        /** Singleton method which returns a global instance
         *  of /DeviceCMYK color space
         */
        static PdfColorSpaceFilterPtr GetDeviceCMYKInstace();
    };

    /** Color space as described by ISO 32000-2:2020 "8.6.5.5 ICCBased colour spaces"
 */
    class PODOFO_API PdfColorSpaceFilterICCBased final : public PdfColorSpaceFilter
    {
    public:
        PdfColorSpaceFilterICCBased(const PdfColorSpaceFilterPtr& alternateColorSpace, charbuff&& iccprofile);
    public:
        PdfColorSpaceType GetType() const override;
        PdfColorSpacePixelFormat GetPixelFormat() const override;
        unsigned GetSourceScanLineSize(unsigned width, unsigned bitsPerComponent) const override;
        unsigned GetScanLineSize(unsigned width, unsigned bitsPerComponent) const override;
        void FetchScanLine(unsigned char* dstScanLine, const unsigned char* srcScanLine,
            unsigned width, unsigned bitsPerComponent) const override;
        PdfObject GetExportObject(PdfIndirectObjectList& objects) const override;
        unsigned char GetColorComponentCount() const override;
    private:
        PdfColorSpaceFilterPtr m_AlternateColorSpace;
        charbuff m_iccprofile;
    };
}

#endif // PDF_COLOR_SPACE_FILTER_H

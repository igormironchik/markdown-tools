/*  -*- c++ -*-
    SPDX-FileCopyrightText: 2001-2002 Marc Mutz <mutz@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KCODECS_QP_H
#define KCODECS_QP_H

#include "kcodecs.h"

namespace KCodecs
{
/*
  A class representing the codec for QuotedPrintable as specified in
   RFC2045 (section 6.7).
*/
class QuotedPrintableCodec : public Codec
{
public:
    constexpr QuotedPrintableCodec() = default;

    constexpr ~QuotedPrintableCodec() override = default;

    constexpr const char *name() const override
    {
        return "quoted-printable";
    }

    qsizetype maxEncodedSizeFor(qsizetype insize, NewlineType newline = Codec::NewlineLF) const override
    {
        // all chars encoded:
        qsizetype result = 3 * insize;
        // then after 25 hexchars comes a soft linebreak: =(\r)\n
        result += (newline == Codec::NewlineCRLF ? 3 : 2) * (insize / 25);

        return result;
    }

    qsizetype maxDecodedSizeFor(qsizetype insize, NewlineType newline = Codec::NewlineLF) const override;

    Encoder *makeEncoder(NewlineType newline = Codec::NewlineLF) const override;

    Decoder *makeDecoder(NewlineType newline = Codec::NewlineLF) const override;
};

/*
  A class representing the codec for the Q encoding as specified
  in RFC2047Q.
*/
class Rfc2047QEncodingCodec : public Codec
{
public:
    constexpr Rfc2047QEncodingCodec() = default;

    constexpr ~Rfc2047QEncodingCodec() override = default;

    constexpr const char *name() const override
    {
        return "q";
    }

    qsizetype maxEncodedSizeFor(qsizetype insize, Codec::NewlineType newline = Codec::NewlineLF) const override
    {
        Q_UNUSED(newline);
        // this one is simple: We don't do linebreaking, so all that can
        // happen is that every char needs encoding, so:
        return 3 * insize;
    }

    qsizetype maxDecodedSizeFor(qsizetype insize, Codec::NewlineType newline = Codec::NewlineLF) const override;

    Encoder *makeEncoder(Codec::NewlineType newline = Codec::NewlineLF) const override;

    Decoder *makeDecoder(Codec::NewlineType newline = Codec::NewlineLF) const override;
};

/*
  A class representing the codec for RFC2231.
*/
class Rfc2231EncodingCodec : public Codec
{
public:
    constexpr Rfc2231EncodingCodec() = default;

    constexpr ~Rfc2231EncodingCodec() override = default;

    constexpr const char *name() const override
    {
        return "x-kmime-rfc2231";
    }

    qsizetype maxEncodedSizeFor(qsizetype insize, Codec::NewlineType newline = Codec::NewlineLF) const override
    {
        Q_UNUSED(newline);
        // same as for "q" encoding:
        return 3 * insize;
    }

    qsizetype maxDecodedSizeFor(qsizetype insize, Codec::NewlineType newline = Codec::NewlineLF) const override;

    Encoder *makeEncoder(Codec::NewlineType newline = Codec::NewlineLF) const override;

    Decoder *makeDecoder(Codec::NewlineType newline = Codec::NewlineLF) const override;
};

} // namespace KCodecs

#endif // KCODECS_QP_H

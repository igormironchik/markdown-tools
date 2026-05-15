/*  -*- c++ -*-
    SPDX-FileCopyrightText: 2002 Marc Mutz <mutz@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KCODECS_UUENCODE_H
#define KCODECS_UUENCODE_H

#include "kcodecs.h"

namespace KCodecs
{
/*
  A class representing the UUEncode codec.
*/
class UUCodec : public Codec
{
public:
    constexpr UUCodec() = default;

    constexpr ~UUCodec() override = default;

    constexpr const char *name() const override
    {
        return "x-uuencode";
    }

    qsizetype maxEncodedSizeFor(qsizetype insize, NewlineType newline = Codec::NewlineLF) const override
    {
        Q_UNUSED(newline);
        return insize; // we have no encoder!
    }

    qsizetype maxDecodedSizeFor(qsizetype insize, NewlineType newline = Codec::NewlineLF) const override
    {
        // assuming all characters are part of the uuencode stream (which
        // does almost never hold due to required linebreaking; but
        // additional non-uu chars don't affect the output size), each
        // 4-tupel of them becomes a 3-tupel in the decoded octet
        // stream. So:
        qsizetype result = ((insize + 3) / 4) * 3;
        // but all of them may be \n, so
        if (newline == Codec::NewlineCRLF) {
            result *= 2; // :-o
        }
        return result;
    }

    Encoder *makeEncoder(NewlineType newline = Codec::NewlineLF) const override;

    Decoder *makeDecoder(NewlineType newline = Codec::NewlineLF) const override;
};

} // namespace KCodecs

#endif // KCODECS_UUENCODE_H

/*  -*- C++ -*-
    SPDX-FileCopyrightText: 1998 Netscape Communications Corporation <developer@mozilla.org>

    SPDX-License-Identifier: MIT
*/

#ifndef nsMBCSGroupProber_h__
#define nsMBCSGroupProber_h__

#include "nsCharSetProber.h"

#include <array>
#include <cstdint>
#include <memory>
#include <span>

#define NUM_OF_PROBERS 6
namespace kencodingprober
{
class KCODECS_NO_EXPORT nsMBCSGroupProber : public nsCharSetProber
{
public:
    enum class Prober : uint8_t {
        Unicode = 0,
        SJIS = 1,
        EUCJP = 2,
        GB18030 = 3,
        EUCKR = 4,
        Big5 = 5,
    };

    nsMBCSGroupProber();
    explicit nsMBCSGroupProber(std::span<const Prober> selected);
    ~nsMBCSGroupProber() override = default;

    nsProbingState HandleData(const char *aBuf, unsigned int aLen) override;
    const char *GetCharSetName() override;
    nsProbingState GetState(void) override
    {
        return mState;
    }
    float GetConfidence(void) override;

#ifdef DEBUG_PROBE
    void DumpStatus() override;
#endif

protected:
    nsProbingState mState = eDetecting;
    std::array<std::unique_ptr<nsCharSetProber>, NUM_OF_PROBERS> mProbers;
    std::array<bool, NUM_OF_PROBERS> mIsActive = {false};
    const std::array<bool, NUM_OF_PROBERS> mIsSelected = {true};
    int mBestGuess = -1;
    unsigned int mActiveNum = 0;
};
}

#endif /* nsMBCSGroupProber_h__ */

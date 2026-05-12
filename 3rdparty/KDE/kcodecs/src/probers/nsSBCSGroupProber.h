/*
    The Original Code is Mozilla Universal charset detector code.

    SPDX-FileCopyrightText: 2001 Netscape Communications Corporation
    SPDX-FileContributor: Shy Shalom <shooshX@gmail.com>

    SPDX-License-Identifier: MPL-1.1 OR GPL-2.0-or-later OR LGPL-2.1-or-later
*/

#ifndef nsSBCSGroupProber_h__
#define nsSBCSGroupProber_h__

#include "nsCharSetProber.h"

#include <array>
#include <memory>

#define NUM_OF_SBCS_PROBERS 12

namespace kencodingprober
{
class KCODECS_NO_EXPORT nsSBCSGroupProber : public nsCharSetProber
{
public:
    nsSBCSGroupProber();
    ~nsSBCSGroupProber() override = default;

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
    std::array<std::unique_ptr<nsCharSetProber>, NUM_OF_SBCS_PROBERS> mProbers = {nullptr};
    std::array<bool, NUM_OF_SBCS_PROBERS> mIsActive = {false};
    int mBestGuess = -1;
};
}

#endif /* nsSBCSGroupProber_h__ */

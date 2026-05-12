/*  -*- C++ -*-
    SPDX-FileCopyrightText: 1998 Netscape Communications Corporation <developer@mozilla.org>

    SPDX-License-Identifier: MIT
*/

#ifndef nsBig5Prober_h__
#define nsBig5Prober_h__

#include "CharDistribution.h"
#include "nsCharSetProber.h"
#include "nsCodingStateMachine.h"

#include <memory>

namespace kencodingprober
{
class KCODECS_NO_EXPORT nsBig5Prober : public nsCharSetProber
{
public:
    nsBig5Prober();
    ~nsBig5Prober() override = default;

    nsProbingState HandleData(const char *aBuf, unsigned int aLen) override;
    const char *GetCharSetName() override
    {
        return "Big5";
    }
    nsProbingState GetState(void) override
    {
        return mState;
    }
    float GetConfidence(void) override;

protected:
    std::unique_ptr<nsCodingStateMachine> mCodingSM;
    nsProbingState mState = eDetecting;

    // Big5ContextAnalysis mContextAnalyser;
    Big5DistributionAnalysis mDistributionAnalyser;

    char mLastChar[2] = {0};
};
}

#endif /* nsBig5Prober_h__ */

/*  -*- C++ -*-
    SPDX-FileCopyrightText: 1998 Netscape Communications Corporation <developer@mozilla.org>

    SPDX-License-Identifier: MIT
*/

#ifndef nsEUCKRProber_h__
#define nsEUCKRProber_h__

#include "CharDistribution.h"
#include "nsCharSetProber.h"
#include "nsCodingStateMachine.h"

#include <memory>

namespace kencodingprober
{
class KCODECS_NO_EXPORT nsEUCKRProber : public nsCharSetProber
{
public:
    nsEUCKRProber();
    ~nsEUCKRProber() override = default;

    nsProbingState HandleData(const char *aBuf, unsigned int aLen) override;
    const char *GetCharSetName() override
    {
        return "EUC-KR";
    }
    nsProbingState GetState(void) override
    {
        return mState;
    }
    float GetConfidence(void) override;

protected:
    std::unique_ptr<nsCodingStateMachine> mCodingSM;
    nsProbingState mState = eDetecting;

    // EUCKRContextAnalysis mContextAnalyser;
    EUCKRDistributionAnalysis mDistributionAnalyser;

    char mLastChar[2] = {0};
};
}

#endif /* nsEUCKRProber_h__ */

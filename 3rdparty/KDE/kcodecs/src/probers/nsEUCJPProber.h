/*  -*- C++ -*-
    SPDX-FileCopyrightText: 1998 Netscape Communications Corporation <developer@mozilla.org>

    SPDX-License-Identifier: MIT
*/

// for S-JIS encoding, observe characteristic:
// 1, kana character (or hankaku?) often have high frequency of appearance
// 2, kana character often exist in group
// 3, certain combination of kana is never used in japanese language

#ifndef nsEUCJPProber_h__
#define nsEUCJPProber_h__

#include "CharDistribution.h"
#include "JpCntx.h"
#include "nsCharSetProber.h"
#include "nsCodingStateMachine.h"

#include <memory>

namespace kencodingprober
{
class KCODECS_NO_EXPORT nsEUCJPProber : public nsCharSetProber
{
public:
    nsEUCJPProber();
    ~nsEUCJPProber() override = default;

    nsProbingState HandleData(const char *aBuf, unsigned int aLen) override;
    const char *GetCharSetName() override
    {
        return "EUC-JP";
    }
    nsProbingState GetState(void) override
    {
        return mState;
    }
    float GetConfidence(void) override;

protected:
    std::unique_ptr<nsCodingStateMachine> mCodingSM;
    nsProbingState mState = eDetecting;

    EUCJPContextAnalysis mContextAnalyser;
    EUCJPDistributionAnalysis mDistributionAnalyser;

    char mLastChar[2] = {0};
};
}

#endif /* nsEUCJPProber_h__ */

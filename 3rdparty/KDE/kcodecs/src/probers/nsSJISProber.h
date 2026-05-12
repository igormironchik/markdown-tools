/*
    The Original Code is mozilla.org code.

    SPDX-FileCopyrightText: 1998 Netscape Communications Corporation

    SPDX-License-Identifier: MPL-1.1 OR GPL-2.0-or-later OR LGPL-2.1-or-later
*/

// for S-JIS encoding, observe characteristic:
// 1, kana character (or hankaku?) often have high frequency of appearance
// 2, kana character often exist in group
// 3, certain combination of kana is never used in japanese language

#ifndef nsSJISProber_h__
#define nsSJISProber_h__

#include "CharDistribution.h"
#include "JpCntx.h"
#include "nsCharSetProber.h"
#include "nsCodingStateMachine.h"

#include <memory>

namespace kencodingprober
{
class KCODECS_NO_EXPORT nsSJISProber : public nsCharSetProber
{
public:
    nsSJISProber();
    ~nsSJISProber() override = default;

    nsProbingState HandleData(const char *aBuf, unsigned int aLen) override;
    const char *GetCharSetName() override
    {
        return "Shift_JIS";
    }
    nsProbingState GetState(void) override
    {
        return mState;
    }
    float GetConfidence(void) override;

protected:
    std::unique_ptr<nsCodingStateMachine> mCodingSM;
    nsProbingState mState = eDetecting;

    SJISContextAnalysis mContextAnalyser;
    SJISDistributionAnalysis mDistributionAnalyser;

    char mLastChar[2] = {0};
};
}

#endif /* nsSJISProber_h__ */

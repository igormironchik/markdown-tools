/*  -*- C++ -*-
    SPDX-FileCopyrightText: 1998 Netscape Communications Corporation <developer@mozilla.org>

    SPDX-License-Identifier: MIT
*/

#include "nsBig5Prober.h"
#include "nsMBCSSM.h"

namespace kencodingprober
{
nsBig5Prober::nsBig5Prober()
    : mCodingSM{std::make_unique<nsCodingStateMachine>(&Big5SMModel)}
{
}

nsProbingState nsBig5Prober::HandleData(const char *aBuf, unsigned int aLen)
{
    if (aLen == 0) {
        return mState;
    }

    for (unsigned int i = 0; i < aLen; i++) {
        const nsSMState codingState = mCodingSM->NextState(aBuf[i]);
        if (codingState == eError) {
            mState = eNotMe;
            break;
        }
        if (codingState == eItsMe) {
            mState = eFoundIt;
            break;
        }
        if (codingState == eStart) {
            unsigned int charLen = mCodingSM->GetCurrentCharLen();

            if (i == 0) {
                mLastChar[1] = aBuf[0];
                mDistributionAnalyser.HandleOneChar(mLastChar, charLen);
            } else {
                mDistributionAnalyser.HandleOneChar(aBuf + i - 1, charLen);
            }
        }
    }

    mLastChar[0] = aBuf[aLen - 1];

    if (mState == eDetecting) {
        if (mDistributionAnalyser.GotEnoughData() && GetConfidence() > SHORTCUT_THRESHOLD) {
            mState = eFoundIt;
        }
    }

    return mState;
}

float nsBig5Prober::GetConfidence(void)
{
    float distribCf = mDistributionAnalyser.GetConfidence();

    return (float)distribCf;
}
}

/*  -*- C++ -*-
    SPDX-FileCopyrightText: 1998 Netscape Communications Corporation <developer@mozilla.org>

    SPDX-License-Identifier: MIT
*/

#include "nsSBCharSetProber.h"

#include <stdio.h>

namespace kencodingprober
{
template class nsSingleByteCharSetProber<false>;
template class nsSingleByteCharSetProber<true>;

template<bool Reversed>
nsProbingState nsSingleByteCharSetProber<Reversed>::HandleData(const char *aBuf, unsigned int aLen)
{
    for (unsigned int i = 0; i < aLen; i++) {
        const unsigned char order = mModel->charToOrderMap[(unsigned char)aBuf[i]];

        if (order < SYMBOL_CAT_ORDER) {
            mTotalChar++;
        }
        if (order < SAMPLE_SIZE) {
            mFreqChar++;

            if (mLastOrder < SAMPLE_SIZE) {
                mTotalSeqs++;
                unsigned int index = Reversed ? mLastOrder + (SAMPLE_SIZE * order) : (mLastOrder * SAMPLE_SIZE) + order;
                ++(mSeqCounters[(int)mModel->precedenceMatrix[index]]);
            }
        }
        mLastOrder = order;
    }

    if (mState == eDetecting) {
        if (mTotalSeqs > 1024) {
            float cf = GetConfidence();
            if (cf > 0.95f) {
                mState = eFoundIt;
            } else if (cf < 0.05) {
                mState = eNotMe;
            }
        }
    }

    return mState;
}

//#define NEGATIVE_APPROACH 1

template<bool Reversed>
float nsSingleByteCharSetProber<Reversed>::GetConfidence(void)
{
#ifdef NEGATIVE_APPROACH
    if (mTotalSeqs > 0)
        if (mTotalSeqs > mSeqCounters.front() * 10) {
            return (mTotalSeqs - mSeqCounters.front() * 10.f) / mTotalSeqs * mFreqChar / mTotalChar;
        }
    return 0.01f;
#else // POSITIVE_APPROACH

    if (mTotalSeqs > 0) {
        float r = 1.0f * mSeqCounters.back() / mTotalSeqs / mModel->mTypicalPositiveRatio;
        r = r * mFreqChar / mTotalChar;
        if (r >= 0.99f) {
            r = 0.99f;
        }
        return r;
    }
    return 0.01f;
#endif
}

template<bool Reversed>
const char *nsSingleByteCharSetProber<Reversed>::GetCharSetName()
{
    return mModel->charsetName;
}

#ifdef DEBUG_PROBE
template<bool Reversed>
void nsSingleByteCharSetProber<Reversed>::DumpStatus()
{
    printf("  SBCS: %1.3f [%s]\r\n", GetConfidence(), GetCharSetName());
}
#endif
}

/*  -*- C++ -*-
    SPDX-FileCopyrightText: 1998 Netscape Communications Corporation <developer@mozilla.org>

    SPDX-License-Identifier: MIT
*/

#include "nsEscCharsetProber.h"
#include "nsEscSM.h"

namespace kencodingprober
{
nsEscCharSetProber::nsEscCharSetProber(void)
{
    mCodingSM[0] = std::make_unique<nsCodingStateMachine>(&ISO2022JPSMModel);
    mCodingSM[1] = std::make_unique<nsCodingStateMachine>(&HZSMModel);
}

nsEscCharSetProber::~nsEscCharSetProber(void) = default;

nsProbingState nsEscCharSetProber::HandleData(const char *aBuf, unsigned int aLen)
{
    if (mState != eDetecting) {
        return mState;
    }

    int activeSM = mCodingSM.size();

    for (auto &codingSM : mCodingSM) {
        for (unsigned int i = 0; i < aLen; i++) {
            // byte is feed to all active state machine
            auto codingState = codingSM->NextState(aBuf[i]);
            if (codingState == eError) {
                // got negative answer for this state machine, make it inactive
                activeSM--;
                if (activeSM == 0) {
                    mState = eNotMe;
                    return mState;
                }
            } else if (codingState == eItsMe) {
                mState = eFoundIt;
                mDetectedCharset = codingSM->GetCodingStateMachine();
                return mState;
            }
        }
    }

    return mState;
}
}

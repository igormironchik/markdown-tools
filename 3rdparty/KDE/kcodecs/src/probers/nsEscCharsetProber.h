/*  -*- C++ -*-
    SPDX-FileCopyrightText: 1998 Netscape Communications Corporation <developer@mozilla.org>

    SPDX-License-Identifier: MIT
*/

#ifndef nsEscCharSetProber_h__
#define nsEscCharSetProber_h__

#include <array>
#include <memory>

#include "nsCharSetProber.h"
#include "nsCodingStateMachine.h"

namespace kencodingprober
{
class KCODECS_NO_EXPORT nsEscCharSetProber : public nsCharSetProber
{
public:
    nsEscCharSetProber(void);
    ~nsEscCharSetProber(void) override;
    nsProbingState HandleData(const char *aBuf, unsigned int aLen) override;
    const char *GetCharSetName() override
    {
        return mDetectedCharset;
    }
    nsProbingState GetState(void) override
    {
        return mState;
    }
    float GetConfidence(void) override
    {
        return (float)0.99;
    }

protected:
    std::array<std::unique_ptr<nsCodingStateMachine>, 2> mCodingSM;
    nsProbingState mState = eDetecting;
    const char *mDetectedCharset = nullptr;
};
}
#endif /* nsEscCharSetProber_h__ */

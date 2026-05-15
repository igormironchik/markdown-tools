/*  -*- C++ -*-
    SPDX-FileCopyrightText: 2008 Wang Kai <wkai@gmail.com>

    SPDX-License-Identifier: MIT
*/

#ifndef UNICODEGROUPPROBER_H
#define UNICODEGROUPPROBER_H

#include "nsCharSetProber.h"
#include "nsCodingStateMachine.h"

#include <memory>

#define NUM_OF_UNICODE_CHARSETS 3
namespace kencodingprober
{
class KCODECS_NO_EXPORT UnicodeGroupProber : public nsCharSetProber
{
public:
    UnicodeGroupProber();
    ~UnicodeGroupProber() override = default;

    nsProbingState HandleData(const char *aBuf, unsigned int aLen) override;
    const char *GetCharSetName() override
    {
        return mDetectedCharset;
    }
    nsProbingState GetState(void) override
    {
        return mState;
    }
    float GetConfidence() override;
#ifdef DEBUG_PROBE
    void DumpStatus() override;
#endif

protected:
    std::unique_ptr<nsCodingStateMachine> mCodingSM[NUM_OF_UNICODE_CHARSETS];
    unsigned int mActiveSM = NUM_OF_UNICODE_CHARSETS;
    nsProbingState mState = eDetecting;
    const char *mDetectedCharset = "UTF-8";
};
}
#endif /* UNICODEGROUPPROBER_H */

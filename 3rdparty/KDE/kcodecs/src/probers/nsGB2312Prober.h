/*  -*- C++ -*-
    SPDX-FileCopyrightText: 1998 Netscape Communications Corporation <developer@mozilla.org>

    SPDX-License-Identifier: MIT
*/

#ifndef nsGB2312Prober_h__
#define nsGB2312Prober_h__

#include "CharDistribution.h"
#include "nsCharSetProber.h"
#include "nsCodingStateMachine.h"

#include <memory>

// We use gb18030 to replace gb2312, because 18030 is a superset.
namespace kencodingprober
{
class KCODECS_NO_EXPORT nsGB18030Prober : public nsCharSetProber
{
public:
    nsGB18030Prober();
    ~nsGB18030Prober() override = default;

    nsProbingState HandleData(const char *aBuf, unsigned int aLen) override;
    const char *GetCharSetName() override
    {
        return "gb18030";
    }
    nsProbingState GetState(void) override
    {
        return mState;
    }
    float GetConfidence(void) override;

protected:
    std::unique_ptr<nsCodingStateMachine> mCodingSM;
    nsProbingState mState = eDetecting;

    // GB2312ContextAnalysis mContextAnalyser;
    GB2312DistributionAnalysis mDistributionAnalyser;

    char mLastChar[2] = {0};
};
}

#endif /* nsGB2312Prober_h__ */

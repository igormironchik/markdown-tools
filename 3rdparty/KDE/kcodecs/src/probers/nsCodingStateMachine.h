/*  -*- C++ -*-
    SPDX-FileCopyrightText: 1998 Netscape Communications Corporation <developer@mozilla.org>

    SPDX-License-Identifier: MIT
*/

#ifndef nsCodingStateMachine_h__
#define nsCodingStateMachine_h__

#include "../kencodingprober_p.h"

#include "kcodecs_export.h"

#include <array>
#include <cstdint>
#include <span>

namespace kencodingprober
{
enum {
    eStart = 0,
    eError = 1,
    eItsMe = 2,
};
using nsSMState = int;

using nsClassTable = const std::array<const uint8_t, 256> &;
using nsStateTable = std::span<const uint8_t>;

// state machine model
typedef struct {
    nsClassTable classTable;
    unsigned int classFactor;
    nsStateTable stateTable;
    const unsigned int *charLenTable;
    const char *name;
} SMModel;

class KCODECS_NO_EXPORT nsCodingStateMachine
{
public:
    nsCodingStateMachine(const SMModel *sm)
        : mModel(sm)
    {
    }
    nsSMState NextState(char c)
    {
        // for each byte we get its class, if it is first byte, we also get byte length
        const uint8_t index = static_cast<uint8_t>(c);
        unsigned int byteCls = mModel->classTable[index];
        if (mCurrentState == eStart) {
            mCurrentCharLen = mModel->charLenTable[byteCls];
        }
        // from byte's class and stateTable, we get its next state
        mCurrentState = mModel->stateTable[mCurrentState * mModel->classFactor + byteCls];
        return mCurrentState;
    }
    unsigned int GetCurrentCharLen(void)
    {
        return mCurrentCharLen;
    }
    void Reset(void)
    {
        mCurrentState = eStart;
    }
    const char *GetCodingStateMachine()
    {
        return mModel->name;
    }
#ifdef DEBUG_PROBE
    const char *DumpCurrentState()
    {
        switch (mCurrentState) {
        case eStart:
            return "eStart";
        case eError:
            return "eError";
        case eItsMe:
            return "eItsMe";
        default:
            return "OK";
        }
    }
#endif

protected:
    int mCurrentState = eStart;
    unsigned int mCurrentCharLen = 0;

    const SMModel *mModel = nullptr;
};
}
#endif /* nsCodingStateMachine_h__ */

/*  -*- C++ -*-
    SPDX-FileCopyrightText: 1998 Netscape Communications Corporation <developer@mozilla.org>

    SPDX-License-Identifier: MIT
*/

#ifndef NSSBCHARSETPROBER_H
#define NSSBCHARSETPROBER_H

#include "nsCharSetProber.h"

#include <array>

#define SAMPLE_SIZE 64
#define SYMBOL_CAT_ORDER 250

namespace kencodingprober
{
typedef struct {
    const unsigned char *charToOrderMap; // [256] table use to find a char's order
    const char *precedenceMatrix; // [SAMPLE_SIZE][SAMPLE_SIZE]; table to find a 2-char sequence's frequency
    float mTypicalPositiveRatio; // = freqSeqs / totalSeqs
    bool keepEnglishLetter; // says if this script contains English characters (not implemented)
    const char *charsetName;
} SequenceModel;

template<bool Reversed = false>
class KCODECS_NO_EXPORT nsSingleByteCharSetProber : public nsCharSetProber
{
public:
    explicit nsSingleByteCharSetProber(const SequenceModel *model)
        : mModel(model)
    {
    }

    const char *GetCharSetName() override;
    nsProbingState HandleData(const char *aBuf, unsigned int aLen) override;
    nsProbingState GetState(void) override
    {
        return mState;
    }
    float GetConfidence(void) override;

#ifdef DEBUG_PROBE
    void DumpStatus() override;
#endif

protected:
    nsProbingState mState = eDetecting;
    const SequenceModel *mModel = nullptr;

    // char order of last character
    unsigned char mLastOrder = 255;

    unsigned int mTotalSeqs = 0;
    std::array<unsigned int, 4> mSeqCounters = {0};

    unsigned int mTotalChar = 0;
    // characters that fall in our sampling range
    unsigned int mFreqChar = 0;
};

extern const SequenceModel Koi8rModel;
extern const SequenceModel Win1251Model;
extern const SequenceModel Latin5Model;
extern const SequenceModel MacCyrillicModel;
extern const SequenceModel Ibm866Model;
extern const SequenceModel Ibm855Model;
extern const SequenceModel Latin7Model;
extern const SequenceModel Win1253Model;
extern const SequenceModel Latin5BulgarianModel;
extern const SequenceModel Win1251BulgarianModel;
extern const SequenceModel Latin2HungarianModel;
extern const SequenceModel Win1250HungarianModel;
extern const SequenceModel Win1255Model;
}
#endif /* NSSBCHARSETPROBER_H */

/*
    This file is part of the KDE libraries

    SPDX-FileCopyrightText: 2017 Forrest Smith <forrestthewoods@gmail.com>
    SPDX-FileCopyrightText: 2021 Waqar Ahmed   <waqar.17a@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/
#include "kfuzzymatcher.h"

#include <QList>
#include <QString>
#include <QStringView>

/*
 * Custom toLower function which is much faster than
 * c.toLower() directly
 */
static inline constexpr QChar toLower(QChar c)
{
    return c.isLower() ? c : c.toLower();
}

// internal
// clang-format off
static bool match_recursive(QStringView::const_iterator pattern,
                            QStringView::const_iterator str,
                            int &outScore,
                            const QStringView::const_iterator strBegin,
                            const QStringView::const_iterator strEnd,
                            const QStringView::const_iterator patternEnd,
                            const uint8_t *srcMatches,
                            uint8_t *matches,
                            int nextMatch,
                            int &totalMatches,
                            int &recursionCount)
{
    static constexpr int recursionLimit = 10;
    // max number of matches allowed, this should be enough
    static constexpr int maxMatches = 256;

    // Count recursions
    ++recursionCount;
    if (recursionCount >= recursionLimit) {
        return false;
    }

    // Detect end of strings
    if (pattern == patternEnd || str == strEnd) {
        return false;
    }

    // Recursion params
    bool recursiveMatch = false;
    uint8_t bestRecursiveMatches[maxMatches];
    int bestRecursiveScore = 0;

    // Loop through pattern and str looking for a match
    bool firstMatch = true;
    QChar currentPatternChar = toLower(*pattern);
    while (pattern != patternEnd && str != strEnd) {
        // Found match
        if (currentPatternChar == toLower(*str)) {
            // Supplied matches buffer was too short
            if (nextMatch >= maxMatches) {
                return false;
            }

            // "Copy-on-Write" srcMatches into matches
            if (firstMatch && srcMatches) {
                memcpy(matches, srcMatches, nextMatch);
                firstMatch = false;
            }

            // Recursive call that "skips" this match
            uint8_t recursiveMatches[maxMatches];
            int recursiveScore = 0;
            const auto strNextChar = std::next(str);
            if (match_recursive(pattern, strNextChar, recursiveScore, strBegin,
                                strEnd, patternEnd, matches, recursiveMatches,
                                nextMatch, totalMatches, recursionCount)) {
                // Pick best recursive score
                if (!recursiveMatch || recursiveScore > bestRecursiveScore) {
                    memcpy(bestRecursiveMatches, recursiveMatches, maxMatches);
                    bestRecursiveScore = recursiveScore;
                }
                recursiveMatch = true;
            }

            // Advance
            matches[nextMatch++] = (uint8_t)(std::distance(strBegin, str));
            ++pattern;
            currentPatternChar = toLower(*pattern);
        }
        ++str;
    }

    // Determine if full pattern was matched
    const bool matched = pattern == patternEnd;

    // Calculate score
    if (matched) {
        static constexpr int firstSepScoreDiff = 3;

        static constexpr int sequentialBonus = 20;
        static constexpr int separatorBonus = 25; // bonus if match occurs after a separator
        static constexpr int firstLetterBonus = 15; // bonus if the first letter is matched
        static constexpr int firstLetterSepMatchBonus = firstLetterBonus - firstSepScoreDiff; // bonus if the first matched letter is camel or separator

        static constexpr int unmatchedLetterPenalty = -1; // penalty for every letter that doesn't matter

        int nonBeginSequenceBonus = 10;
        // points by which nonBeginSequenceBonus is increment on every matched letter
        static constexpr int nonBeginSequenceIncrement = 4;

        // Initialize score
        outScore = 100;

#define debug_algo 0
#if debug_algo
#define dbg(...) qDebug(__VA_ARGS__)
#else
#define dbg(...)
#endif

        // Apply unmatched penalty
        const int unmatched = (int)(std::distance(strBegin, strEnd)) - nextMatch;
        outScore += unmatchedLetterPenalty * unmatched;
        dbg("unmatchedLetterPenalty, unmatched count: %d, outScore: %d", unmatched, outScore);

        bool inSeparatorSeq = false;
        int i = 0;
        if (matches[i] == 0) {
            // First letter match has the highest score
            outScore += firstLetterBonus + separatorBonus;
            dbg("firstLetterBonus, outScore: %d", outScore);
            inSeparatorSeq = true;
        } else {
            const QChar neighbor = *(strBegin + matches[i] - 1);
            const QChar curr = *(strBegin + matches[i]);
            const bool neighborSeparator = neighbor == QLatin1Char('_') || neighbor == QLatin1Char(' ');
            if (neighborSeparator || (neighbor.isLower() && curr.isUpper())) {
                // the first letter that got matched was a special char .e., camel or at a separator
                outScore += firstLetterSepMatchBonus + separatorBonus;
                dbg("firstLetterSepMatchBonus at %d, letter: %c, outScore: %d", matches[i], curr.toLatin1(), outScore);
                inSeparatorSeq = true;
            } else {
                // nothing
                nonBeginSequenceBonus += nonBeginSequenceIncrement;
            }
            // We didn't match any special positions, apply leading penalty
            outScore += -(matches[i]);
            dbg("LeadingPenalty because no first letter match, outScore: %d", outScore);
        }
        i++;

        bool allConsecutive = true;
        // Apply ordering bonuses
        for (; i < nextMatch; ++i) {
            const uint8_t currIdx = matches[i];
            const uint8_t prevIdx = matches[i - 1];
            // Sequential
            if (currIdx == (prevIdx + 1)) {
                if (i == matches[i]) {
                    // We are in a sequence beginning from first letter
                    outScore += sequentialBonus;
                    dbg("sequentialBonus at %d, letter: %c, outScore: %d", matches[i], (strBegin + currIdx)->toLatin1(), outScore);
                } else if (inSeparatorSeq) {
                    // we are in a sequence beginning from a separator like camelHump or underscore
                    outScore += sequentialBonus - firstSepScoreDiff;
                    dbg("in separator seq, [sequentialBonus - 5] at %d, letter: %c, outScore: %d", matches[i], (strBegin + currIdx)->toLatin1(), outScore);
                } else {
                    // We are in a random sequence
                    outScore += nonBeginSequenceBonus;
                    nonBeginSequenceBonus += nonBeginSequenceIncrement;
                    dbg("nonBeginSequenceBonus at %d, letter: %c, outScore: %d", matches[i], (strBegin + currIdx)->toLatin1(), outScore);
                }
            } else {
                allConsecutive = false;

                // there is a gap between matching chars, apply penalty
                int penalty = -((currIdx - prevIdx)) - 2;
                outScore += penalty;
                inSeparatorSeq = false;
                nonBeginSequenceBonus = 10;
                dbg("gap penalty[%d] at %d, letter: %c, outScore: %d", penalty, matches[i], (strBegin + currIdx)->toLatin1(), outScore);
            }

            // Check for bonuses based on neighbor character value
            // Camel case
            const QChar neighbor = *(strBegin + currIdx - 1);
            const QChar curr = *(strBegin + currIdx);
            // if camel case bonus, then not snake / separator.
            // This prevents double bonuses
            const bool neighborSeparator = neighbor == QLatin1Char('_') || neighbor == QLatin1Char(' ');
            if (neighborSeparator || (neighbor.isLower() && curr.isUpper())) {
                outScore += separatorBonus;
                dbg("separatorBonus at %d, letter: %c, outScore: %d", matches[i], (strBegin + currIdx)->toLatin1(), outScore);
                inSeparatorSeq = true;
                continue;
            }
        }

        if (allConsecutive && nextMatch >= 4) {
            outScore *= 2;
            dbg("allConsecutive double the score, outScore: %d", outScore);
        }
    }

    totalMatches = nextMatch;

    // Return best result
    if (recursiveMatch && (!matched || bestRecursiveScore > outScore)) {
        // Recursive score is better than "this"
        memcpy(matches, bestRecursiveMatches, maxMatches);
        outScore = bestRecursiveScore;
        return true;
    } else if (matched) {
        // "this" score is better than recursive
        return true;
    } else {
        // no match
        return false;
    }
}
// clang-format on

static bool match_internal(QStringView pattern, QStringView str, int &outScore, unsigned char *matches)
{
    if (pattern.isEmpty()) {
        return true;
    }

    int recursionCount = 0;

    auto strIt = str.cbegin();
    auto patternIt = pattern.cbegin();
    const auto patternEnd = pattern.cend();
    const auto strEnd = str.cend();

    int total = 0;
    return match_recursive(patternIt, strIt, outScore, strIt, strEnd, patternEnd, nullptr, matches, 0, total, recursionCount);
}

/**************************************************************/

bool KFuzzyMatcher::matchSimple(QStringView pattern, QStringView str)
{
    auto patternIt = pattern.cbegin();
    /*
     * Instead of doing
     *
     *      strIt.toLower() == patternIt.toLower()
     *
     * we convert patternIt to Upper / Lower as needed and compare with strIt. This
     * saves us from calling toLower() on both strings, making things a little bit faster
     */
    bool lower = patternIt->isLower();
    QChar cUp = lower ? patternIt->toUpper() : *patternIt;
    QChar cLow = lower ? *patternIt : patternIt->toLower();
    for (auto strIt = str.cbegin(); strIt != str.cend() && patternIt != pattern.cend(); ++strIt) {
        if (*strIt == cLow || *strIt == cUp) {
            ++patternIt;
            lower = patternIt->isLower();
            cUp = lower ? patternIt->toUpper() : *patternIt;
            cLow = lower ? *patternIt : patternIt->toLower();
        }
    }

    return patternIt == pattern.cend();
}

KFuzzyMatcher::Result KFuzzyMatcher::match(QStringView pattern, QStringView str)
{
    /*
     * Simple substring matching to flush out non-matching strings
     */
    const bool simpleMatch = matchSimple(pattern, str);

    KFuzzyMatcher::Result result;
    result.matched = false;
    result.score = 0;

    if (!simpleMatch) {
        return result;
    }

    // actual algorithm
    int score = 0;
    uint8_t matches[256];
    const bool matched = match_internal(pattern, str, score, matches);
    result.matched = matched;
    result.score = score;
    return result;
}

QList<KFuzzyMatcher::Range> KFuzzyMatcher::matchedRanges(QStringView pattern, QStringView str, RangeType type)
{
    QList<KFuzzyMatcher::Range> ranges;
    if (pattern.isEmpty()) {
        return ranges;
    }

    int totalMatches = 0;
    int score = 0;
    int recursionCount = 0;

    auto strIt = str.cbegin();
    auto patternIt = pattern.cbegin();
    const auto patternEnd = pattern.cend();
    const auto strEnd = str.cend();

    uint8_t matches[256];
    auto res = match_recursive(patternIt, strIt, score, strIt, strEnd, patternEnd, nullptr, matches, 0, totalMatches, recursionCount);
    // didn't match? => We don't care about results
    if (!res && type == RangeType::FullyMatched) {
        return {};
    }

    int previousMatch = 0;
    for (int i = 0; i < totalMatches; ++i) {
        auto matchPos = matches[i];
        /*
         * Check if this match is part of the previous
         * match. If it is, we increase the length of
         * the last range.
         */
        if (!ranges.isEmpty() && matchPos == previousMatch + 1) {
            ranges.last().length++;
        } else {
            /*
             * This is a new match inside the string
             */
            ranges.push_back({/* start: */ matchPos, /* length: */ 1});
        }
        previousMatch = matchPos;
    }

    return ranges;
}

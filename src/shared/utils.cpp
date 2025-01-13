/*
    SPDX-FileCopyrightText: 2024-2025 Igor Mironchik <igor.mironchik@gmail.com>
    SPDX-License-Identifier: GPL-3.0-or-later
*/

// shared include.
#include "utils.hpp"

// Qt include.
#include <QtResource>

void initSharedResources()
{
    Q_INIT_RESOURCE(qt);
    Q_INIT_RESOURCE(icon);
}

bool isRightToLeft(const QChar &ch)
{
    switch (ch.direction()) {
    case QChar::DirAL:
    case QChar::DirAN:
    case QChar::DirR:
        return true;

    default:
        return false;
    }
}

QVector<QPair<QString, bool>> splitString(const QString &str, bool skipSpaces)
{
    QVector<QPair<QString, bool>> res;
    qsizetype first = 0;
    bool space = false;
    bool previousRTL = false;

    for(qsizetype i = 0; i < str.length(); ++i) {
        if (str[i].isSpace() && !space) {
            if (first < i) {
                const auto word = str.sliced(first, i - first);
                bool rtl = false;

                if (word[0].direction() != QChar::DirON) {
                    previousRTL = isRightToLeft(word[0]);
                }

                rtl = previousRTL;

                res.append({word, rtl});
            }

            if (!skipSpaces) {
                res.append({QStringLiteral(" "), false});
            }

            space = true;
        } else {
            if (space) {
                first = i;
            }

            space = false;
        }
    }

    if (!space && first < str.length()) {
        const auto word = str.sliced(first);
        res.append({word, isRightToLeft(word[0])});
    }

    return res;
}

void orderWords(QVector<QPair<QString, bool>> & text)
{
    qsizetype start = -1;
    qsizetype end = -1;

    auto reverseItems = [] (qsizetype start, qsizetype end, QVector<QPair<QString, bool>> & data) {
        if (start > -1 && end > start) {
            while (end - start > 0) {
                data.swapItemsAt(start, end);
                ++start;
                --end;
            }
        }
    };

    for (qsizetype i = 0; i < text.size(); ++i) {
        if (text[i].first != QStringLiteral(" ")) {
            if (!text[i].second) {
                if (start == -1 ) {
                    start = i;
                    end = i;
                } else {
                    end = i;
                }
            } else {
                reverseItems(start, end, text);

                start = -1;
                end = -1;
            }
        }
    }

    reverseItems(start, end, text);
}

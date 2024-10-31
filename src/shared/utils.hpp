/*
    SPDX-FileCopyrightText: 2024 Igor Mironchik <igor.mironchik@gmail.com>
    SPDX-License-Identifier: GPL-3.0-or-later
*/

#pragma once

// Qt include.
#include <QString>
#include <QVector>

void initSharedResources();

bool isRightToLeft(const QChar &ch);

QVector<QPair<QString, bool>> splitString(const QString &str, bool skipSpaces);

void orderWords(QVector<QPair<QString, bool>> & text);

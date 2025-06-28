/*
    SPDX-FileCopyrightText: 2024-2025 Igor Mironchik <igor.mironchik@gmail.com>
    SPDX-License-Identifier: GPL-3.0-or-later
*/

#pragma once

// Qt include.
#include <QString>
#include <QVector>

//! Init shared resources, like *.qrc.
void initSharedResources();

//! \return Is a given character RTL one?
bool isRightToLeft(const QChar &ch);

/*!
 * \brief Split string by spaces.
 * \param str String.
 * \param skipSpaces If false in returned vector will be spaces too.
 * \return Vector of words with flag indicates RTL.
 */
QVector<QPair<QString, bool>> splitString(const QString &str, bool skipSpaces);

//! Order words for painting with Qt with RTL, LTR rules.
void orderWords(QVector<QPair<QString, bool>> & text);

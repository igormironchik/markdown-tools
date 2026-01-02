/*
    SPDX-FileCopyrightText: 2026 Igor Mironchik <igor.mironchik@gmail.com>
    SPDX-License-Identifier: GPL-3.0-or-later
*/

#pragma once

// Qt include.
#include <QString>
#include <QVector>

// md4qt include.
#include <md4qt/src/parser.h>

// shared include.
#include "plugins_page.h"

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
QVector<QPair<QString,
              bool>>
splitString(const QString &str,
            bool skipSpaces);

//! Order words for painting with Qt with RTL, LTR rules.
void orderWords(QVector<QPair<QString,
                              bool>> &text);

//! Set plugins to parser.
void setPlugins(MD::Parser &parser,
                const MdShared::PluginsCfg &cfg);

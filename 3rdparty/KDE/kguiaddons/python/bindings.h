// SPDX-FileCopyrightText: 2024 Manuel Alcaraz Zambrano <manuelalcarazzam@gmail.com>
// SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL

#pragma once

// Make "signals:", "slots:" visible as access specifiers
#define QT_ANNOTATE_ACCESS_SPECIFIER(a) __attribute__((annotate(#a)))

#include <QDate>
#include <QMimeData>

#include <KColorMimeData>
#include <KColorSchemeWatcher>
#include <KColorUtils>
#include <KCountryFlagEmojiIconEngine>
#include <KCursorSaver>
#include <KDateValidator>
#include <KFontUtils>
#include <KIconUtils>
#include <KJobWindows>
#include <KKeySequenceRecorder>
#include <KModifierKeyInfo>
#include <KSystemClipboard>
#include <KWordWrap>
#include <klocalimagecacheimpl.h>

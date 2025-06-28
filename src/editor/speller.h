/*
    SPDX-FileCopyrightText: 2024-2025 Igor Mironchik <igor.mironchik@gmail.com>
    SPDX-License-Identifier: GPL-3.0-or-later
*/

// Sonnet include.
#include <Sonnet/Speller>

// Qt include.
#include <QSharedPointer>
#include <QMutex>

namespace MdEditor
{

//
// Speller
//

//! Singleton for spelling stuff.
struct Speller {
    QSharedPointer<Sonnet::Speller> m_speller;
    QMutex m_mutex;

    static Speller &instance();

private:
    Speller();
    ~Speller() = default;

    static void cleanup();

    Q_DISABLE_COPY(Speller)
}; // struct Speller

}

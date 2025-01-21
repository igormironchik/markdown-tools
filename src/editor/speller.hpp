/*
    SPDX-FileCopyrightText: 2024-2025 Igor Mironchik <igor.mironchik@gmail.com>
    SPDX-License-Identifier: GPL-3.0-or-later
*/

// Sonnet include.
#include <Sonnet/Speller>
#include <Sonnet/GuessLanguage>

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
    QSharedPointer<Sonnet::GuessLanguage> m_guessLanguage;
    QMutex m_mutex;

    static Speller &instance();

private:
    Speller();
    ~Speller() = default;

    Q_DISABLE_COPY(Speller)
}; // struct Speller

}

/*
    SPDX-FileCopyrightText: 2024-2025 Igor Mironchik <igor.mironchik@gmail.com>
    SPDX-License-Identifier: GPL-3.0-or-later
*/

// md-editor include.
#include "speller.hpp"

// Qt include.
#include <QCoreApplication>

namespace MdEditor
{

//
// Speller
//

static Speller * s_speller = nullptr;

Speller &Speller::instance()
{
    if (!s_speller) {
        s_speller = new Speller;

        qAddPostRoutine(&Speller::cleanup);
    }

    return *s_speller;
}

Speller::Speller()
    : m_speller(new Sonnet::Speller)
    , m_guessLanguage(new Sonnet::GuessLanguage)
{
}

void Speller::cleanup()
{
    delete s_speller;

    s_speller = nullptr;
}

}

/*
    SPDX-FileCopyrightText: 2024-2025 Igor Mironchik <igor.mironchik@gmail.com>
    SPDX-License-Identifier: GPL-3.0-or-later
*/

// md-editor include.
#include "speller.hpp"

namespace MdEditor
{

//
// Speller
//

Speller &Speller::instance()
{
    static Speller speller;
    return speller;
}

Speller::Speller()
    : m_speller(new Sonnet::Speller)
    , m_guessLanguage(new Sonnet::GuessLanguage)
{
}

}

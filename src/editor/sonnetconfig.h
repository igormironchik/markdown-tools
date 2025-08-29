/*
    SPDX-FileCopyrightText: 2025 Igor Mironchik <igor.mironchik@gmail.com>
    SPDX-License-Identifier: GPL-3.0-or-later
*/

#pragma once

// Sonnet include.
#include <Sonnet/ConfigWidget>

namespace MdEditor
{

//
// SonnetConfig
//

//! Sonnet configuration widget.
class SonnetConfig : public Sonnet::ConfigWidget
{
    Q_OBJECT

public:
    SonnetConfig(QWidget *parent);
    ~SonnetConfig() override = default;

private:
    Q_DISABLE_COPY(SonnetConfig)
}; // class SonnetConfig

} /* namespace MdEditor */

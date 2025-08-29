/*
    SPDX-FileCopyrightText: 2025 Igor Mironchik <igor.mironchik@gmail.com>
    SPDX-License-Identifier: GPL-3.0-or-later
*/

#pragma once

// Qt include.
#include <QWidget>

// md-editor include.
#include "ui_font.h"

namespace MdEditor
{

//
// FontPage
//

//! Settings page with font settings.
class FontPage : public QWidget
{
    Q_OBJECT

public:
    explicit FontPage(QWidget *parent = nullptr);
    ~FontPage() override = default;

    //! \return UI component.
    Ui::FontPage &ui();
    //! \return Current font.
    QFont currentFont() const;
    //! Init UI with font.
    void initWithFont(const QFont &f);

private slots:
    void onShowOnlyMonospaced(Qt::CheckState state);

private:
    Q_DISABLE_COPY(FontPage)

    Ui::FontPage m_ui;
}; // class FontPage

} /* namespace MdEditor */

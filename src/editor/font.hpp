/*
	SPDX-FileCopyrightText: 2024 Igor Mironchik <igor.mironchik@gmail.com>
	SPDX-License-Identifier: GPL-3.0-or-later
*/

#pragma once

// Qt include.
#include <QWidget>

// md-editor include.
#include "ui_font.h"


namespace MdEditor {

//
// FontPage
//

//! Page with font settings.
class FontPage
	:	public QWidget
{
	Q_OBJECT

public:
	explicit FontPage( QWidget * parent = nullptr );
	~FontPage() override = default;

	Ui::FontPage & ui();
	QFont currentFont() const;
	void initWithFont( const QFont & f );

private slots:
	void onShowOnlyMonospaced( int state );

private:
	Q_DISABLE_COPY( FontPage )

	Ui::FontPage m_ui;
}; // class FontPage

} /* namespace MdEditor */

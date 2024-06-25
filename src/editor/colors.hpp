/*
	SPDX-FileCopyrightText: 2024 Igor Mironchik <igor.mironchik@gmail.com>
	SPDX-License-Identifier: GPL-3.0-or-later
*/

#pragma once

// Qt include.
#include <QWidget>

// md-editor include.
#include "ui_colors.h"


namespace MdEditor {

//
// Colors
//

//! Color scheme.
struct Colors {
	QColor textColor = QColor( 0, 0, 128 );
	QColor linkColor = QColor( 0, 128, 0 );
	QColor inlineColor = Qt::black;
	QColor htmlColor = QColor( 128, 0, 0 );
	QColor tableColor = Qt::black;
	QColor codeColor = Qt::black;
	QColor mathColor = QColor( 128, 0, 0 );
	QColor referenceColor = QColor( 128, 0, 0 );
	QColor specialColor = QColor( 128, 0, 0 );
	bool enabled = true;
}; // struct Colors

bool operator != ( const Colors & c1, const Colors & c2 );

//
// ColorsPage
//

//! Page with colors.
class ColorsPage
	:	public QWidget
{
	Q_OBJECT

public:
	explicit ColorsPage( QWidget * parent = nullptr );
	~ColorsPage() override = default;

	Ui::ColorsPage & ui();
	Colors & colors();

public slots:
	void resetDefaults();
	void applyColors();
	void chooseLinkColor();
	void chooseTextColor();
	void chooseInlineColor();
	void chooseHtmlColor();
	void chooseTableColor();
	void chooseCodeColor();
	void chooseMathColor();
	void chooseReferenceColor();
	void chooseSpecialColor();
	void colorsToggled( bool on );

private:
	void chooseColor( MdShared::ColorWidget * w, QColor & c );

private:
	Q_DISABLE_COPY( ColorsPage )

	Ui::ColorsPage m_ui;
	Colors m_colors;
}; // class ColorsPage

} /* namespace MdEditor */

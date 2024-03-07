
/*!
	\file

	\author Igor Mironchik (igor.mironchik at gmail dot com).

	Copyright (c) 2023-2024 Igor Mironchik

	This program is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once

// Qt include.
#include <QDialog>
#include <QColor>
#include <QScopedPointer>
#include <QAbstractButton>

class ColorWidget;

namespace MdEditor {

//
// Colors
//

//! Color scheme.
struct Colors {
	QColor textColor = QColor( 0, 0, 128 );
	QColor linkColor = QColor( 0, 128, 0 );
	QColor listColor = QColor( 0, 128, 0 );
	QColor inlineColor = Qt::black;
	QColor htmlColor = QColor( 128, 0, 0 );
	QColor tableColor = Qt::black;
	QColor blockquoteColor = QColor( 0, 0, 128 );
	QColor codeColor = Qt::black;
	QColor headingColor = QColor( 0, 0, 128 );
	QColor mathColor = QColor( 128, 0, 0 );
	QColor footnoteColor = QColor( 128, 0, 0 );
	bool enabled = true;
}; // struct Colors

bool operator != ( const Colors & c1, const Colors & c2 );


//
// ColorsDialog
//

struct ColorsDialogPrivate;

//! Colors dialog.
class ColorsDialog
	:	public QDialog
{
	Q_OBJECT

public:
	explicit ColorsDialog( const Colors & cols, QWidget * parent = nullptr );
	~ColorsDialog() override;

	const Colors & colors() const;

private slots:
	void clicked( QAbstractButton * btn );
	void resetDefaults();
	void applyColors();
	void chooseLinkColor();
	void chooseListColor();
	void chooseTextColor();
	void chooseInlineColor();
	void chooseHtmlColor();
	void chooseTableColor();
	void chooseBlockquoteColor();
	void chooseCodeColor();
	void chooseHeadingColor();
	void chooseMathColor();
	void chooseFootnoteColor();
	void colorsToggled( bool on );

private:
	void chooseColor( ColorWidget * w, QColor & c );

private:
	Q_DISABLE_COPY( ColorsDialog )

	QScopedPointer< ColorsDialogPrivate > d;
}; // class ColorsDialog

} /* namespace MdEditor */

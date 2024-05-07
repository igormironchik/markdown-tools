/*
	SPDX-FileCopyrightText: 2024 Igor Mironchik <igor.mironchik@gmail.com>
	SPDX-License-Identifier: GPL-3.0-or-later
*/

#pragma once

// Qt include.
#include <QDialog>
#include <QColor>
#include <QScopedPointer>
#include <QAbstractButton>

namespace MdShared {

class ColorWidget;

} /* namespace MdShared */

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
	QColor specialColor = QColor( 128, 0, 0 );
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
	void chooseSpecialColor();
	void colorsToggled( bool on );

private:
	void chooseColor( MdShared::ColorWidget * w, QColor & c );

private:
	Q_DISABLE_COPY( ColorsDialog )

	QScopedPointer< ColorsDialogPrivate > d;
}; // class ColorsDialog

} /* namespace MdEditor */

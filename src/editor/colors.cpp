/*
	SPDX-FileCopyrightText: 2024 Igor Mironchik <igor.mironchik@gmail.com>
	SPDX-License-Identifier: GPL-3.0-or-later
*/

// md-editor include.
#include "colors.hpp"
#include "ui_colors.h"

// Qt include.
#include <QDialogButtonBox>
#include <QColorDialog>
#include <QPushButton>
#include <QGroupBox>

// shared include.
#include "color_widget.hpp"


namespace MdEditor {

bool operator != ( const Colors & c1, const Colors & c2 )
{
	return ( c1.enabled != c2.enabled || c1.inlineColor != c2.inlineColor ||
		c1.linkColor != c2.linkColor || c1.listColor != c2.listColor ||
		c1.textColor != c2.textColor || c1.htmlColor != c2.htmlColor ||
		c1.tableColor != c2.tableColor || c1.blockquoteColor != c2.blockquoteColor ||
		c1.codeColor != c2.codeColor || c1.enabled != c2.enabled ||
		c1.headingColor != c2.headingColor || c1.mathColor != c2.mathColor ||
		c1.footnoteColor != c2.footnoteColor );
}

//
// ColorsDialogPrivate
//

struct ColorsDialogPrivate {
	Colors colors;
	Ui::ColorsDialog ui;
}; // struct ColorsDialogPrivate

//
// ColorsDialog
//

ColorsDialog::ColorsDialog( const Colors & cols, QWidget * parent )
	:	QDialog( parent )
	,	d( new ColorsDialogPrivate )
{
	d->colors = cols;

	d->ui.setupUi( this );

	applyColors();

	connect( d->ui.buttonBox, &QDialogButtonBox::clicked,
		this, &ColorsDialog::clicked );
	connect( d->ui.linkColor, &MdShared::ColorWidget::clicked,
		this, &ColorsDialog::chooseLinkColor );
	connect( d->ui.listColor, &MdShared::ColorWidget::clicked,
		this, &ColorsDialog::chooseListColor );
	connect( d->ui.textColor, &MdShared::ColorWidget::clicked,
		this, &ColorsDialog::chooseTextColor );
	connect( d->ui.inlineColor, &MdShared::ColorWidget::clicked,
		this, &ColorsDialog::chooseInlineColor );
	connect( d->ui.htmlColor, &MdShared::ColorWidget::clicked,
		this, &ColorsDialog::chooseHtmlColor );
	connect( d->ui.tableColor, &MdShared::ColorWidget::clicked,
		this, &ColorsDialog::chooseTableColor );
	connect( d->ui.blockquoteColor, &MdShared::ColorWidget::clicked,
		this, &ColorsDialog::chooseBlockquoteColor );
	connect( d->ui.codeColor, &MdShared::ColorWidget::clicked,
		this, &ColorsDialog::chooseCodeColor );
	connect( d->ui.headingColor, &MdShared::ColorWidget::clicked,
		this, &ColorsDialog::chooseHeadingColor );
	connect( d->ui.colors, &QGroupBox::toggled,
		this, &ColorsDialog::colorsToggled );
}

ColorsDialog::~ColorsDialog()
{
}

const Colors &
ColorsDialog::colors() const
{
	return d->colors;
}

void
ColorsDialog::clicked( QAbstractButton * btn )
{
	if( static_cast< QAbstractButton* > ( d->ui.buttonBox->button(
		QDialogButtonBox::RestoreDefaults ) ) == btn )
			resetDefaults();
}

void
ColorsDialog::resetDefaults()
{
	d->colors = {};

	applyColors();
}

void
ColorsDialog::applyColors()
{
	d->ui.colors->setChecked( d->colors.enabled );

	d->ui.inlineColor->setColor( d->colors.inlineColor );
	d->ui.linkColor->setColor( d->colors.linkColor );
	d->ui.listColor->setColor( d->colors.listColor );
	d->ui.textColor->setColor( d->colors.textColor );
	d->ui.htmlColor->setColor( d->colors.htmlColor );
	d->ui.tableColor->setColor( d->colors.tableColor );
	d->ui.blockquoteColor->setColor( d->colors.blockquoteColor );
	d->ui.codeColor->setColor( d->colors.codeColor );
	d->ui.headingColor->setColor( d->colors.headingColor );
	d->ui.mathColor->setColor( d->colors.mathColor );
	d->ui.footnoteColor->setColor( d->colors.footnoteColor );
}

void
ColorsDialog::chooseColor( MdShared::ColorWidget * w, QColor & c )
{
	QColorDialog dlg( c, this );

	if( dlg.exec() == QDialog::Accepted )
	{
		w->setColor( dlg.currentColor() );
		c = dlg.currentColor();
	}
}

void
ColorsDialog::chooseLinkColor()
{
	chooseColor( d->ui.linkColor, d->colors.linkColor );
}

void
ColorsDialog::chooseListColor()
{
	chooseColor( d->ui.listColor, d->colors.listColor );
}

void
ColorsDialog::chooseTextColor()
{
	chooseColor( d->ui.textColor, d->colors.textColor );
}

void
ColorsDialog::chooseInlineColor()
{
	chooseColor( d->ui.inlineColor, d->colors.inlineColor );
}

void
ColorsDialog::chooseHtmlColor()
{
	chooseColor( d->ui.htmlColor, d->colors.htmlColor );
}

void
ColorsDialog::chooseTableColor()
{
	chooseColor( d->ui.tableColor, d->colors.tableColor );
}

void
ColorsDialog::chooseBlockquoteColor()
{
	chooseColor( d->ui.blockquoteColor, d->colors.blockquoteColor );
}

void
ColorsDialog::chooseCodeColor()
{
	chooseColor( d->ui.codeColor, d->colors.codeColor );
}

void
ColorsDialog::chooseHeadingColor()
{
	chooseColor( d->ui.headingColor, d->colors.headingColor );
}

void
ColorsDialog::chooseMathColor()
{
	chooseColor( d->ui.mathColor, d->colors.mathColor );
}

void
ColorsDialog::chooseFootnoteColor()
{
	chooseColor( d->ui.footnoteColor, d->colors.footnoteColor );
}

void
ColorsDialog::colorsToggled( bool on )
{
	d->colors.enabled = on;
}

} /* namespace MdEditor */

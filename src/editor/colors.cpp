/*
	SPDX-FileCopyrightText: 2024 Igor Mironchik <igor.mironchik@gmail.com>
	SPDX-License-Identifier: GPL-3.0-or-later
*/

// md-editor include.
#include "colors.hpp"

// Qt include.
#include <QColorDialog>


namespace MdEditor {

bool operator != ( const Colors & c1, const Colors & c2 )
{
	return ( c1.enabled != c2.enabled || c1.inlineColor != c2.inlineColor ||
		c1.linkColor != c2.linkColor || c1.textColor != c2.textColor ||
		c1.htmlColor != c2.htmlColor || c1.tableColor != c2.tableColor ||
		c1.codeColor != c2.codeColor || c1.enabled != c2.enabled ||
		c1.mathColor != c2.mathColor || c1.referenceColor != c2.referenceColor ||
		c1.specialColor != c2.specialColor );
}

//
// ColorsPage
//

ColorsPage::ColorsPage( QWidget * parent )
	:	QWidget( parent )
{
	m_ui.setupUi( this );

	connect( m_ui.linkColor, &MdShared::ColorWidget::clicked,
		this, &ColorsPage::chooseLinkColor );
	connect( m_ui.textColor, &MdShared::ColorWidget::clicked,
		this, &ColorsPage::chooseTextColor );
	connect( m_ui.inlineColor, &MdShared::ColorWidget::clicked,
		this, &ColorsPage::chooseInlineColor );
	connect( m_ui.htmlColor, &MdShared::ColorWidget::clicked,
		this, &ColorsPage::chooseHtmlColor );
	connect( m_ui.tableColor, &MdShared::ColorWidget::clicked,
		this, &ColorsPage::chooseTableColor );
	connect( m_ui.codeColor, &MdShared::ColorWidget::clicked,
		this, &ColorsPage::chooseCodeColor );
	connect( m_ui.colors, &QGroupBox::toggled,
		this, &ColorsPage::colorsToggled );
	connect( m_ui.specialColor, &MdShared::ColorWidget::clicked,
		this, &ColorsPage::chooseSpecialColor );
	connect( m_ui.referenceColor, &MdShared::ColorWidget::clicked,
		this, &ColorsPage::chooseReferenceColor );
	connect( m_ui.mathColor, &MdShared::ColorWidget::clicked,
		this, &ColorsPage::chooseMathColor );
}

Ui::ColorsPage &
ColorsPage::ui()
{
	return m_ui;
}

Colors &
ColorsPage::colors()
{
	return m_colors;
}

void
ColorsPage::resetDefaults()
{
	m_colors = {};

	applyColors();
}

void
ColorsPage::applyColors()
{
	m_ui.colors->setChecked( m_colors.enabled );

	m_ui.inlineColor->setColor( m_colors.inlineColor );
	m_ui.linkColor->setColor( m_colors.linkColor );
	m_ui.textColor->setColor( m_colors.textColor );
	m_ui.htmlColor->setColor( m_colors.htmlColor );
	m_ui.tableColor->setColor( m_colors.tableColor );
	m_ui.codeColor->setColor( m_colors.codeColor );
	m_ui.mathColor->setColor( m_colors.mathColor );
	m_ui.referenceColor->setColor( m_colors.referenceColor );
	m_ui.specialColor->setColor( m_colors.specialColor );
}

void
ColorsPage::chooseColor( MdShared::ColorWidget * w, QColor & c )
{
	QColorDialog dlg( c, this );

	if( dlg.exec() == QDialog::Accepted )
	{
		w->setColor( dlg.currentColor() );
		c = dlg.currentColor();
	}
}

void
ColorsPage::chooseLinkColor()
{
	chooseColor( m_ui.linkColor, m_colors.linkColor );
}

void
ColorsPage::chooseTextColor()
{
	chooseColor( m_ui.textColor, m_colors.textColor );
}

void
ColorsPage::chooseInlineColor()
{
	chooseColor( m_ui.inlineColor, m_colors.inlineColor );
}

void
ColorsPage::chooseHtmlColor()
{
	chooseColor( m_ui.htmlColor, m_colors.htmlColor );
}

void
ColorsPage::chooseTableColor()
{
	chooseColor( m_ui.tableColor, m_colors.tableColor );
}

void
ColorsPage::chooseCodeColor()
{
	chooseColor( m_ui.codeColor, m_colors.codeColor );
}

void
ColorsPage::chooseMathColor()
{
	chooseColor( m_ui.mathColor, m_colors.mathColor );
}

void
ColorsPage::chooseReferenceColor()
{
	chooseColor( m_ui.referenceColor, m_colors.referenceColor );
}

void
ColorsPage::chooseSpecialColor()
{
	chooseColor( m_ui.specialColor, m_colors.specialColor );
}

void
ColorsPage::colorsToggled( bool on )
{
	m_colors.enabled = on;
}

} /* namespace MdEditor */

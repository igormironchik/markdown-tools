/*
	SPDX-FileCopyrightText: 2024 Igor Mironchik <igor.mironchik@gmail.com>
	SPDX-License-Identifier: GPL-3.0-or-later
*/

// md-editor include.
#include "htmldocument.hpp"


namespace MdEditor {

HtmlDocument::HtmlDocument( QObject * parent )
	:	QObject( parent )
{
}

void
HtmlDocument::setText( const QString & text )
{
    if( text == m_text )
        return;

    m_text = text;

    emit textChanged( m_text );
}

} /* namespace MdEditor */

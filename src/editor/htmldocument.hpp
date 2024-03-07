
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
#include <QObject>
#include <QString>


namespace MdEditor {

//
// HtmlDocument
//

class HtmlDocument
	:	public QObject
{
    Q_OBJECT
    Q_PROPERTY( QString text MEMBER m_text NOTIFY textChanged FINAL )

signals:
    void textChanged( const QString & text );

public:
    explicit HtmlDocument( QObject * parent );
	~HtmlDocument() override = default;

    void setText( const QString & text );

private:
    QString m_text;
}; // class HtmlDocument

} /* namespace MdEditor */

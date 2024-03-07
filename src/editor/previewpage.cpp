
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

// md-editor include.
#include "previewpage.hpp"

// Qt include.
#include <QDesktopServices>
#include <QWebEngineSettings>


namespace MdEditor {

PreviewPage::PreviewPage( QObject * parent )
	:	QWebEnginePage( parent )
{
	settings()->setAttribute( QWebEngineSettings::LocalContentCanAccessRemoteUrls, true );
	settings()->setAttribute( QWebEngineSettings::LinksIncludedInFocusChain, false );
}

bool
PreviewPage::acceptNavigationRequest( const QUrl & url,
	QWebEnginePage::NavigationType /*type*/, bool /*isMainFrame*/ )
{
	if( url.scheme() == QStringLiteral( "qrc" ) || url.scheme() == QStringLiteral( "data" ) )
        return true;

	QDesktopServices::openUrl( url );

	return false;
}

} /* namespace MdEditor */

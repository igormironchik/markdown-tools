
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

// Qt include.
#include <QString>
#include <QApplication>
#include <QScreen>
#include <QCommandLineParser>
#include <QWebEngineUrlScheme>

// md-editor include.
#include "mainwindow.hpp"


int main( int argc, char ** argv )
{
	QWebEngineUrlScheme qrc( "qrc" );
	qrc.setFlags( QWebEngineUrlScheme::CorsEnabled );
	qrc.setSyntax( QWebEngineUrlScheme::Syntax::Path );
	QWebEngineUrlScheme::registerScheme( qrc );

	QApplication app( argc, argv );

	QCommandLineParser parser;
	parser.setApplicationDescription( QStringLiteral( "Markdown editor and viewer." ) );
	parser.addHelpOption();
	parser.addPositionalArgument( QStringLiteral( "markdown" ),
		QStringLiteral( "Markdown file to open." ) );
	QCommandLineOption view( QStringList() << "v" << "view",
		QStringLiteral( "Open Markdown file in view (HTML preview) mode." ) );
	QCommandLineOption all( QStringList() << "a" << "all",
		QStringLiteral( "Load all linked Markdown files." ) );
	parser.addOption( view );
	parser.addOption( all );

	parser.process( app );

	const auto args = parser.positionalArguments();

	const auto fileName = ( args.isEmpty() ? QString() : args.at( 0 ) );

	QIcon appIcon( QStringLiteral( ":/res/img/icon_256x256.png" ) );
	appIcon.addFile( QStringLiteral( ":/res/img/icon_128x128.png" ) );
	appIcon.addFile( QStringLiteral( ":/res/img/icon_64x64.png" ) );
	appIcon.addFile( QStringLiteral( ":/res/img/icon_48x48.png" ) );
	appIcon.addFile( QStringLiteral( ":/res/img/icon_32x32.png" ) );
	appIcon.addFile( QStringLiteral( ":/res/img/icon_24x24.png" ) );
	appIcon.addFile( QStringLiteral( ":/res/img/icon_16x16.png" ) );
	app.setWindowIcon( appIcon );

	MdEditor::MainWindow w;
	const auto screenSize = app.primaryScreen()->availableGeometry().size();
	w.resize( qRound( (double) screenSize.width() * 0.85 ),
		qRound( (double) screenSize.height() * 0.85 ) );

	if( !fileName.isEmpty() )
		w.openFile( fileName );

	if( parser.isSet( view ) )
		w.openInPreviewMode( parser.isSet( all ) );
	else if( parser.isSet( all ) )
		w.loadAllLinkedFiles();

	if( parser.isSet( view ) && fileName.isEmpty() )
		return 0;

	w.show();

	return QApplication::exec();
}

/*
	SPDX-FileCopyrightText: 2024 Igor Mironchik <igor.mironchik@gmail.com>
	SPDX-License-Identifier: GPL-3.0-or-later
*/

// Qt include.
#include <QProcess>
#include <QDir>
#include <QStringList>
#include <QDebug>
#include <QCommandLineParser>
#include <QCoreApplication>
#include <QFileInfo>

#ifdef Q_OS_WIN
// OpenSSL include.
#include <openssl/opensslconf.h>
#endif // Q_OS_WIN


int main( int argc, char ** argv )
{
	QCoreApplication app( argc, argv );

#ifdef Q_OS_WIN
  #if OPENSSL_VERSION_MAJOR >= 3
	QByteArray envVal = qgetenv( "OPENSSL_MODULES" );

	if ( envVal.isEmpty() )
	{
		qputenv( "OPENSSL_MODULES", "./lib/ossl-modules" );
		qputenv( "OPENSSL_ENGINES", "./lib/engines-3" );
	}
  #endif
#endif // Q_OS_WIN

	QCommandLineParser parser;
	parser.setApplicationDescription( QStringLiteral( "Launcher of application required OpenSSL." ) );
	parser.addHelpOption();

	QCommandLineOption executable( QStringLiteral( "exe" ),
		QStringLiteral( "Executable to launch." ),
		QStringLiteral( "exe" ) );
	parser.addOption( executable );

	QCommandLineOption mode( QStringLiteral( "mode" ),
		QStringLiteral( "Launch mode: detached | notdetached " ),
		QStringLiteral( "mode" ), QStringLiteral( "notdetached" ) );
	parser.addOption( mode );

	QCommandLineOption arg( QStringLiteral( "arg" ),
		QStringLiteral( "Argument to pass to launced process." ),
		QStringLiteral( "arg" ) );
	parser.addOption( arg );

	parser.process( app );

	const auto modeValue = ( parser.isSet( mode ) ? parser.value( mode ) :
		QStringLiteral( "notdetached" ) );
	const auto argValue = ( parser.isSet( arg ) ? parser.value( arg ) : QString() );

	if( parser.isSet( executable ) )
	{
		const auto exeValue = parser.value( executable );
		const QFileInfo info( exeValue );

		if( !info.exists() )
			return 1;

		QProcess p;
		p.setWorkingDirectory( info.absolutePath() );
		p.setProgram( info.absoluteFilePath() );

		if( !argValue.isEmpty() )
			p.setArguments( QStringList() << argValue );

		if( modeValue == QStringLiteral( "detached" ) )
		{
			if( p.startDetached() )
				return 0;
			else
				return 1;
		}
		else
		{
			p.start();

			if( p.waitForFinished( 15 * 60 * 1000 ) )
				return 0;
			else
				return 1;
		}
	}
	else
		return 1;
}

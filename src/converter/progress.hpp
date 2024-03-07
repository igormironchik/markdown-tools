
/*!
	\file

	\author Igor Mironchik (igor.mironchik at gmail dot com).

	Copyright (c) 2019-2024 Igor Mironchik

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

#ifndef MD_PDF_PROGRESS_HPP_INCLUDED
#define MD_PDF_PROGRESS_HPP_INCLUDED

// Qt include.
#include <QDialog>

// md-pdf include.
#include "renderer.hpp"
#include "ui_progress.h"


//
// ProgressDlg
//

//! Progress dialog.
class ProgressDlg final
	:	public QDialog
{
	Q_OBJECT

public:
	ProgressDlg( PdfRenderer * render, QWidget * parent );
	~ProgressDlg() override = default;

	const QString & errorMsg() const;

protected:
	void closeEvent( QCloseEvent * e ) override;

private slots:
	void progress( int value );
	void finished( bool terminated );
	void cancel();
	void error( const QString & msg );
	void status( const QString & msg );

private:
	PdfRenderer * m_render;
	QScopedPointer< Ui::ProgressDlg > m_ui;
	QString m_error;

	Q_DISABLE_COPY( ProgressDlg )
}; // class ProgressDlg

#endif // MD_PDF_PROGRESS_HPP_INCLUDED

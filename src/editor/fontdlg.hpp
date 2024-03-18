/*
	SPDX-FileCopyrightText: 2024 Igor Mironchik <igor.mironchik@gmail.com>
	SPDX-License-Identifier: GPL-3.0-or-later
*/

#pragma once

// Qt include.
#include <QDialog>
#include <QScopedPointer>
#include <QFont>


namespace MdEditor {

//
// FontDlg
//

struct FontDlgPrivate;

//! Font dialog.
class FontDlg
	:	public QDialog
{
	Q_OBJECT

public:
	FontDlg( const QFont & f, QWidget * parent );
	~FontDlg() override;

	QFont font() const;

private slots:
	void onShowOnlyMonospaced( int state );

private:
	friend struct FontDlgPrivate;

	Q_DISABLE_COPY( FontDlg )

	QScopedPointer< FontDlgPrivate > d;
}; // class FontDlg

} /* namespace MdEditor */

/*
	SPDX-FileCopyrightText: 2024 Igor Mironchik <igor.mironchik@gmail.com>
	SPDX-License-Identifier: GPL-3.0-or-later
*/

#pragma once

// Qt include.
#include <QFrame>
#include <QScopedPointer>


QT_BEGIN_NAMESPACE
class QLineEdit;
QT_END_NAMESPACE


namespace MdEditor {

//
// Find
//

struct FindPrivate;
class Editor;
class MainWindow;

//! Find/replace widget.
class Find
	:	public QFrame
{
	Q_OBJECT

public:
	Find( MainWindow * window, Editor * editor, QWidget * parent );
	~Find() override;

	QLineEdit * editLine() const;
	QLineEdit * replaceLine() const;

public slots:
	void setFindText( const QString & text );
	void setFocusOnFind();

private slots:
	void onFindTextChanged( const QString & str );
	void onReplaceTextChanged( const QString & str );
	void onReplace();
	void onReplaceAll();
	void onSelectionChanged();
	void onClose();

private:
	friend struct FindPrivate;

	Q_DISABLE_COPY( Find )

	QScopedPointer< FindPrivate > d;
}; // class Find

} /* namespace MdEditor */

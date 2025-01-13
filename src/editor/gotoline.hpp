/*
    SPDX-FileCopyrightText: 2024-2025 Igor Mironchik <igor.mironchik@gmail.com>
    SPDX-License-Identifier: GPL-3.0-or-later
*/

#pragma once

// Qt include.
#include <QFrame>
#include <QScopedPointer>

QT_BEGIN_NAMESPACE
class QLineEdit;
QT_END_NAMESPACE

namespace MdEditor
{

//
// GoToLine
//

struct GoToLinePrivate;
class Editor;
class MainWindow;

//! Go to line widget.
class GoToLine : public QFrame
{
    Q_OBJECT

public:
    GoToLine(MainWindow *window, Editor *editor, QWidget *parent);
    ~GoToLine() override;

    QLineEdit *line() const;

public slots:
    void setFocusOnLine();

private slots:
    void onEditingFinished();
    void onClose();

private:
    friend struct GoToLinePrivate;

    Q_DISABLE_COPY(GoToLine)

    QScopedPointer<GoToLinePrivate> m_d;
}; // class GoToLine

} /* namespace MdEditor */

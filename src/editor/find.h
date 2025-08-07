/*
    SPDX-FileCopyrightText: 2024-2025 Igor Mironchik <igor.mironchik@gmail.com>
    SPDX-License-Identifier: GPL-3.0-or-later
*/

#pragma once

// Qt include.
#include <QFrame>
#include <QScopedPointer>
#include <QTextDocument>

QT_BEGIN_NAMESPACE
class QLineEdit;
QT_END_NAMESPACE

namespace MdEditor
{

//
// Find
//

struct FindPrivate;
class Editor;
class MainWindow;

//! Find/replace widget.
class Find : public QFrame
{
    Q_OBJECT

public:
    Find(MainWindow *window,
         Editor *editor,
         QWidget *parent);
    ~Find() override;

    //! \return Line edit for "find".
    QLineEdit *editLine() const;
    //! \return Line edit for "replace".
    QLineEdit *replaceLine() const;
    //! \return Is search should be case sensitive?
    bool isCaseSensitive() const;
    //! \return Is search should be for whole word only?
    bool isWholeWord() const;
    //! \return QTextDocument::FindFlags of search.
    QTextDocument::FindFlags findFlags() const;

public slots:
    //! Set "find" text.
    void setFindText(const QString &text);
    //! Set focus on "find" line edit.
    void setFocusOnFind();
    //! Set whether search should be case sensitive.
    void setCaseSensitive(bool on = true);
    //! Set whether search shoulb be for whole word only.
    void setWholeWord(bool on = true);

private slots:
    //! On "Find" text was changed.
    void onFindTextChanged(const QString &str);
    //! On "Replace" button clicked.
    void onReplace();
    //! On "Replace all" button clicked.
    void onReplaceAll();
    //! On selection in editor was changed.
    void onSelectionChanged();
    //! On "Close" button was clicked.
    void onClose();
    //! On editor ready signal.
    void onEditorReady();
    //! On search flags change.
    void onFindFlagsChanged(Qt::CheckState state);

private:
    friend struct FindPrivate;

    Q_DISABLE_COPY(Find)

    QScopedPointer<FindPrivate> m_d;
}; // class Find

} /* namespace MdEditor */

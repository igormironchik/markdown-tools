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
// FindWeb
//

struct FindWebPrivate;
class MainWindow;
class WebView;

//! Find on Web preview widget.
class FindWeb : public QFrame
{
    Q_OBJECT

public:
    FindWeb(MainWindow *window,
            WebView *web,
            QWidget *parent);
    ~FindWeb() override;

    //! \return Line edit of "find" text.
    QLineEdit *line() const;

public slots:
    //! Set "find" text.
    void setFindWebText(const QString &text);
    //! Set focus on "find" line edit.
    void setFocusOnFindWeb();

private slots:
    //! On "find" text changed.
    void onFindWebTextChanged(const QString &str);
    //! On "Close" button clicked.
    void onClose();
    //! On find previous.
    void onFindPrev();
    //! ON find next.
    void onFindNext();

protected:
    void hideEvent(QHideEvent *event) override;

private:
    friend struct FindWebPrivate;

    Q_DISABLE_COPY(FindWeb)

    QScopedPointer<FindWebPrivate> m_d;
}; // class FindWeb

} /* namespace MdEditor */

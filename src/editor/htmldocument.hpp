/*
    SPDX-FileCopyrightText: 2024 Igor Mironchik <igor.mironchik@gmail.com>
    SPDX-License-Identifier: GPL-3.0-or-later
*/

#pragma once

// Qt include.
#include <QObject>
#include <QString>

namespace MdEditor
{

//
// HtmlDocument
//

class HtmlDocument : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString text MEMBER m_text NOTIFY textChanged FINAL)

signals:
    void textChanged(const QString &text);

public:
    explicit HtmlDocument(QObject *parent);
    ~HtmlDocument() override = default;

    void setText(const QString &text);

private:
    QString m_text;
}; // class HtmlDocument

} /* namespace MdEditor */

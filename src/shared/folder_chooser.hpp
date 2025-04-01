/*
    SPDX-FileCopyrightText: 2024-2025 Igor Mironchik <igor.mironchik@gmail.com>
    SPDX-License-Identifier: GPL-3.0-or-later
*/

#pragma once

// Qt include.
#include <QWidget>

namespace MdShared
{

//
// FolderChooser
//

class FolderChooserPrivate;

//! Dialog with chooser of any parent directory.
class FolderChooser : public QWidget
{
    Q_OBJECT

signals:
    void pathSelected(const QString &);

public:
    explicit FolderChooser(QWidget *parent = nullptr);
    ~FolderChooser() override;

    QSize sizeHint() const override;

    static QStringList splitPath(const QString &path);

    QString currentPath() const;

public slots:
    void setPath(const QString &path);
    void setPopup(bool on = true);

private slots:
    void onClicked(int idx);

private:
    friend class FolderChooserPrivate;

    QScopedPointer<FolderChooserPrivate> m_d;

    Q_DISABLE_COPY(FolderChooser)
}; // class FolderChooser

} /* namespace MdShared */

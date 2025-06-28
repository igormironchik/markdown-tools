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

/*!
 * \brief Folder chooser.
 *
 * Dialog with chooser of any parent directory.
 */
class FolderChooser : public QWidget
{
    Q_OBJECT

signals:
    /*!
     * Path choosed.
     */
    void pathSelected(const QString &);

public:
    /*!
     * Constructor.
     *
     * \a parent Parent widget.
     */
    explicit FolderChooser(QWidget *parent = nullptr);
    ~FolderChooser() override;

    QSize sizeHint() const override;

    /*!
     * Returns splitted path.
     *
     * \a path Path.
     */
    static QStringList splitPath(const QString &path);

    /*!
     * Returns current selected path.
     */
    QString currentPath() const;

public slots:
    /*!
     * Set new path.
     *
     * \a path Path.
     */
    void setPath(const QString &path);
    /*!
     * Set Qt::Popup flag for this widget.
     *
     * \a on Flag value.
     */
    void setPopup(bool on = true);
    /*!
     * Emulate click on folder with the given index.
     *
     * \a idx Index of folder.
     */
    void emulateClick(int idx);

private slots:
    void onClicked(int idx);
    void updateFolders();

private:
    friend class FolderChooserPrivate;

    QScopedPointer<FolderChooserPrivate> m_d;

    Q_DISABLE_COPY(FolderChooser)
}; // class FolderChooser

} /* namespace MdShared */

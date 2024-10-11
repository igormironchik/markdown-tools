/*
    SPDX-FileCopyrightText: 2024 Igor Mironchik <igor.mironchik@gmail.com>
    SPDX-License-Identifier: GPL-3.0-or-later
*/

#pragma once

// Qt include.
#include <QDialog>
#include <QVBoxLayout>
#include <QWidget>

namespace MdShared
{

//
// LicenseDialog
//

class LicenseDialogPrivate;

//! Dialog with list of licenses.
class LicenseDialog : public QDialog
{
    Q_OBJECT

public:
    explicit LicenseDialog(QWidget *parent = nullptr);
    ~LicenseDialog() override;

    //! Add license.
    void addLicense(const QString &title, const QString &license);

protected:
    void showEvent(QShowEvent *event) override;

private:
    QScopedPointer<LicenseDialogPrivate> d;

    Q_DISABLE_COPY(LicenseDialog)
}; // class LicenseDialog

} /* namespace MdShared */

/*
    SPDX-FileCopyrightText: 2025 Igor Mironchik <igor.mironchik@gmail.com>
    SPDX-License-Identifier: GPL-3.0-or-later
*/

#pragma once

// md-pdf include.
#include "renderer.h"
#include "ui_progress.h"

// Qt include.
#include <QDialog>

namespace MdPdf
{

//
// ProgressDlg
//

//! Progress dialog.
class ProgressDlg final : public QDialog
{
    Q_OBJECT

public:
    ProgressDlg(Render::PdfRenderer *render,
                QWidget *parent);
    ~ProgressDlg() override = default;

    const QString &errorMsg() const;

protected:
    void closeEvent(QCloseEvent *e) override;

private slots:
    void progress(int value);
    void finished(bool terminated);
    void cancel();
    void error(const QString &msg);
    void status(const QString &msg);

private:
    Render::PdfRenderer *m_render;
    QScopedPointer<Ui::ProgressDlg> m_ui;
    QString m_error;

    Q_DISABLE_COPY(ProgressDlg)
}; // class ProgressDlg

} /* namespace MdPdf */

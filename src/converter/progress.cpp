/*
    SPDX-FileCopyrightText: 2024-2025 Igor Mironchik <igor.mironchik@gmail.com>
    SPDX-License-Identifier: GPL-3.0-or-later
*/

// md-pdf include.
#include "progress.h"

// Qt include.
#include <QCloseEvent>
#include <QMessageBox>

namespace MdPdf
{

//
// ProgressDlg
//

ProgressDlg::ProgressDlg(Render::PdfRenderer *render,
                         QWidget *parent)
    : QDialog(parent)
    , m_render(render)
    , m_ui(new Ui::ProgressDlg())
{
    m_ui->setupUi(this);

    connect(m_render, &Render::PdfRenderer::progress, this, &ProgressDlg::progress);
    connect(m_render, &Render::PdfRenderer::done, this, &ProgressDlg::finished);
    connect(m_render, &Render::PdfRenderer::error, this, &ProgressDlg::error);
    connect(m_render, &Render::PdfRenderer::status, this, &ProgressDlg::status);

    connect(m_ui->m_cancel, &QPushButton::clicked, this, &ProgressDlg::cancel);
}

void ProgressDlg::closeEvent(QCloseEvent *e)
{
    cancel();

    e->ignore();
}

const QString &ProgressDlg::errorMsg() const
{
    return m_error;
}

void ProgressDlg::progress(int value)
{
    m_ui->m_progress->setValue(value);
}

void ProgressDlg::finished(bool terminated)
{
    if (terminated) {
        reject();
    } else {
        accept();
    }
}

void ProgressDlg::cancel()
{
    const auto answer =
        QMessageBox::question(this, tr("Cancel rendering?"), tr("Do you really want to terminate rendering?"));

    if (answer == QMessageBox::Yes) {
        m_render->terminate();
    }
}

void ProgressDlg::error(const QString &msg)
{
    m_error = msg;

    reject();
}

void ProgressDlg::status(const QString &msg)
{
    m_ui->m_status->setText(msg);
}

} /* namespace MdPdf */

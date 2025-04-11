/*
    SPDX-FileCopyrightText: 2024-2025 Igor Mironchik <igor.mironchik@gmail.com>
    SPDX-License-Identifier: GPL-3.0-or-later
*/

// md4qt include.
#define MD4QT_QT_SUPPORT
#include <md4qt/parser.h>

// md-pdf include.
#include "const.hpp"
#include "main_window.hpp"
#include "progress.hpp"
#include "renderer.hpp"
#include "version.hpp"

// Qt include.
#include <QApplication>
#include <QCloseEvent>
#include <QColorDialog>
#include <QComboBox>
#include <QFileDialog>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QPushButton>
#include <QShowEvent>
#include <QStandardPaths>
#include <QTextStream>
#include <QThread>
#include <QToolButton>
#include <QWindow>

// podofo include.
#include <podofo/podofo.h>

// shared include.
#include "license_dialog.hpp"

namespace MdPdf
{

//
// MainWindow
//

MainWidget::MainWidget(QWidget *parent)
    : QWidget(parent)
    , m_ui(new Ui::MainWindow())
    , m_thread(new QThread(this))
    , m_textFontOk(false)
    , m_codeFontOk(false)
    , m_syntax(new Syntax)
{
    m_ui->setupUi(this);

    connect(m_ui->m_linkColor, &MdShared::ColorWidget::clicked, this, &MainWidget::changeLinkColor);
    connect(m_ui->m_borderColor, &MdShared::ColorWidget::clicked, this, &MainWidget::changeBorderColor);
    connect(m_ui->m_fileNameBtn, &QToolButton::clicked, this, &MainWidget::selectMarkdown);
    connect(m_ui->m_startBtn, &QPushButton::clicked, this, &MainWidget::process);
    connect(m_ui->m_textFont, &QFontComboBox::currentFontChanged, this, &MainWidget::textFontChanged);
    connect(m_ui->m_codeFont, &QFontComboBox::currentFontChanged, this, &MainWidget::codeFontChanged);

    void (QSpinBox::*signal)(int) = &QSpinBox::valueChanged;

    connect(m_ui->m_codeFontSize, signal, this, &MainWidget::codeFontSizeChanged);
    connect(m_ui->m_textFontSize, signal, this, &MainWidget::textFontSizeChanged);

    connect(m_ui->m_mm, &QToolButton::toggled, this, &MainWidget::mmButtonToggled);

    m_ui->m_left->setMaximum(50);
    m_ui->m_right->setMaximum(50);
    m_ui->m_top->setMaximum(50);
    m_ui->m_bottom->setMaximum(50);

    adjustSize();

    m_thread->start();

    if (!Render::PdfRenderer::isFontCreatable(m_ui->m_textFont->currentText(), false)) {
        for (int i = 0; i < m_ui->m_textFont->count(); ++i) {
            if (Render::PdfRenderer::isFontCreatable(m_ui->m_textFont->itemText(i), false)) {
                m_ui->m_textFont->setCurrentIndex(i);

                break;
            }
        }
    }

    if (!Render::PdfRenderer::isFontCreatable(m_ui->m_codeFont->currentText(), true)) {
        for (int i = 0; i < m_ui->m_codeFont->count(); ++i) {
            if (Render::PdfRenderer::isFontCreatable(m_ui->m_codeFont->itemText(i), true)) {
                m_ui->m_codeFont->setCurrentIndex(i);

                break;
            }
        }
    }

    textFontChanged(m_ui->m_textFont->currentFont());
    codeFontChanged(m_ui->m_codeFont->currentFont());

    QStringList themeNames;
    const auto themes = m_syntax->repository().themes();

    for (const auto &t : themes) {
        themeNames.push_back(t.name());
    }

    themeNames.sort(Qt::CaseInsensitive);

    m_ui->m_codeTheme->addItems(themeNames);
    m_ui->m_codeTheme->setCurrentText(QStringLiteral("GitHub Light"));
}

MainWidget::~MainWidget()
{
    m_thread->quit();
    m_thread->wait();
}

void MainWidget::showEvent(QShowEvent *event)
{
    if (!m_alreadyShown) {
        m_alreadyShown = true;

        const auto w = std::max(m_ui->m_mm->width(), m_ui->m_pt->width());

        m_ui->m_mm->setMinimumWidth(w);
        m_ui->m_pt->setMinimumWidth(w);
    }

    event->accept();
}

static const QString s_leftAlignment = QStringLiteral("left");
static const QString s_centerAlignment = QStringLiteral("center");
static const QString s_rightAlignment = QStringLiteral("right");

inline QString imageAlignmentToString(Render::ImageAlignment a)
{
    switch (a) {
    case Render::ImageAlignment::Left:
        return s_leftAlignment;

    case Render::ImageAlignment::Center:
        return s_centerAlignment;

    case Render::ImageAlignment::Right:
        return s_rightAlignment;

    default:
        return {};
    }
}

inline Render::ImageAlignment stringToImageAlignment(const QString &a)
{
    if (a == s_leftAlignment) {
        return Render::ImageAlignment::Left;
    } else if (a == s_centerAlignment) {
        return Render::ImageAlignment::Center;
    } else if (a == s_rightAlignment) {
        return Render::ImageAlignment::Right;
    } else {
        return Render::ImageAlignment::Center;
    }
}

inline Render::ImageAlignment imageAlignmentFromInt(int v)
{
    switch (v) {
    case 0:
        return Render::ImageAlignment::Left;

    case 1:
        return Render::ImageAlignment::Center;

    case 2:
        return Render::ImageAlignment::Right;

    default:
        return Render::ImageAlignment::Center;
    }
}

inline int imageAlignmentToInt(Render::ImageAlignment a)
{
    switch (a) {
    case Render::ImageAlignment::Left:
        return 0;

    case Render::ImageAlignment::Center:
        return 1;

    case Render::ImageAlignment::Right:
        return 2;

    default:
        return 1;
    }
}

void MainWidget::saveCfg(QSettings &cfg) const
{
    cfg.beginGroup(QStringLiteral("ui"));

    cfg.beginGroup(QStringLiteral("textFont"));
    cfg.setValue(QStringLiteral("family"), m_ui->m_textFont->currentFont().family());
    cfg.setValue(QStringLiteral("size"), m_ui->m_textFontSize->value());
    cfg.endGroup();

    cfg.beginGroup(QStringLiteral("codeFont"));
    cfg.setValue(QStringLiteral("family"), m_ui->m_codeFont->currentFont().family());
    cfg.setValue(QStringLiteral("size"), m_ui->m_codeFontSize->value());
    cfg.endGroup();

    cfg.setValue(QStringLiteral("linkColor"), m_ui->m_linkColor->color());
    cfg.setValue(QStringLiteral("borderColor"), m_ui->m_borderColor->color());
    cfg.setValue(QStringLiteral("codeTheme"), m_ui->m_codeTheme->currentText());
    cfg.setValue(QStringLiteral("dpi"), m_ui->m_dpi->value());
    cfg.setValue(QStringLiteral("imageAlignment"), imageAlignmentToString(imageAlignmentFromInt(
        m_ui->m_imageAlignment->currentIndex())));

    cfg.beginGroup(QStringLiteral("margins"));

    if (m_ui->m_mm->isChecked()) {
        cfg.setValue(QStringLiteral("units"), QStringLiteral("mm"));
    } else {
        cfg.setValue(QStringLiteral("units"), QStringLiteral("pt"));
    }

    cfg.setValue(QStringLiteral("left"), m_ui->m_left->value());
    cfg.setValue(QStringLiteral("right"), m_ui->m_right->value());
    cfg.setValue(QStringLiteral("top"), m_ui->m_top->value());
    cfg.setValue(QStringLiteral("bottom"), m_ui->m_bottom->value());

    cfg.endGroup();
    cfg.endGroup();
}

inline int limitFontSize(int s)
{
    if (s < 6) {
        s = 6;
    }

    if (s > 16) {
        s = 16;
    }

    return s;
}

void MainWidget::applyCfg(QSettings &cfg)
{
    cfg.beginGroup(QStringLiteral("ui"));

    cfg.beginGroup(QStringLiteral("textFont"));

    {
        const auto fontName = cfg.value(QStringLiteral("family")).toString();

        if (!fontName.isEmpty()) {
            auto fs = limitFontSize(cfg.value(QStringLiteral("size")).toInt());

            const QFont f(fontName, fs);

            m_ui->m_textFont->setCurrentFont(f);
            m_ui->m_textFontSize->setValue(fs);
        }
    }

    cfg.endGroup();

    cfg.beginGroup(QStringLiteral("codeFont"));

    {
        const auto fontName = cfg.value(QStringLiteral("family")).toString();

        if (!fontName.isEmpty()) {
            auto fs = limitFontSize(cfg.value(QStringLiteral("size")).toInt());

            const QFont f(fontName, fs);

            m_ui->m_codeFont->setCurrentFont(f);
            m_ui->m_codeFontSize->setValue(fs);
        }
    }

    cfg.endGroup();

    const auto linkColor = cfg.value(QStringLiteral("linkColor"), QColor(33, 122, 255)).value<QColor>();
    if (linkColor.isValid()) {
        m_ui->m_linkColor->setColor(linkColor);
    }

    const auto borderColor = cfg.value(QStringLiteral("borderColor"), QColor(81, 81, 81)).value<QColor>();
    if (borderColor.isValid()) {
        m_ui->m_borderColor->setColor(borderColor);
    }

    const auto codeTheme = cfg.value(QStringLiteral("codeTheme")).toString();
    if (!codeTheme.isEmpty()) {
        m_ui->m_codeTheme->setCurrentText(codeTheme);
    }

    const auto dpi = cfg.value(QStringLiteral("dpi")).toInt();
    if (dpi > 0) {
        m_ui->m_dpi->setValue(dpi);
    }

    const auto imageAlignment = cfg.value(QStringLiteral("imageAlignment")).toString();
    if (!imageAlignment.isEmpty()) {
        m_ui->m_imageAlignment->setCurrentIndex(imageAlignmentToInt(stringToImageAlignment(imageAlignment)));
    }

    cfg.beginGroup(QStringLiteral("margins"));

    const auto units = cfg.value(QStringLiteral("units")).toString();
    if (!units.isEmpty()) {
        if (units == QStringLiteral("mm")) {
            m_ui->m_mm->setChecked(true);
        } else {
            m_ui->m_pt->setChecked(true);
        }
    }

    const auto left = cfg.value(QStringLiteral("left")).toInt();
    if (left >= 0) {
        m_ui->m_left->setValue(left);
    }

    const auto right = cfg.value(QStringLiteral("right")).toInt();
    if (right >= 0) {
        m_ui->m_right->setValue(right);
    }

    const auto bottom = cfg.value(QStringLiteral("bottom")).toInt();
    if (bottom >= 0) {
        m_ui->m_bottom->setValue(bottom);
    }

    const auto top = cfg.value(QStringLiteral("top")).toInt();
    if (top >= 0) {
        m_ui->m_top->setValue(top);
    }

    cfg.endGroup();
    cfg.endGroup();
}

void MainWidget::setMarkdownFile(const QString &fileName)
{
    m_ui->m_fileName->setText(fileName);
    m_ui->m_workingDirectory->setPath(QFileInfo(fileName).absolutePath());
    m_ui->m_workingDirBox->setChecked(false);

    changeStateOfStartButton();
}

void MainWidget::changeLinkColor()
{
    QColorDialog dlg(m_ui->m_linkColor->color(), this);

    if (QDialog::Accepted == dlg.exec()) {
        m_ui->m_linkColor->setColor(dlg.currentColor());
    }
}

void MainWidget::changeBorderColor()
{
    QColorDialog dlg(m_ui->m_borderColor->color(), this);

    if (QDialog::Accepted == dlg.exec()) {
        m_ui->m_borderColor->setColor(dlg.currentColor());
    }
}

void MainWidget::changeStateOfStartButton()
{
    if (m_textFontOk && m_codeFontOk) {
        m_ui->m_startBtn->setEnabled(true);
    } else {
        m_ui->m_startBtn->setEnabled(false);
    }
}

void MainWidget::selectMarkdown()
{
    const auto folder = m_ui->m_fileName->text().isEmpty() ?
                QDir::homePath() : QFileInfo(m_ui->m_fileName->text()).absolutePath();

    const auto fileName = QFileDialog::getOpenFileName(this, tr("Select Markdown"), folder,
                                                       tr("Markdown (*.md *.markdown)"));

    if (!fileName.isEmpty()) {
        setMarkdownFile(fileName);
    }
}

void MainWidget::process()
{
    auto fileName = QFileDialog::getSaveFileName(this, tr("Save as"), QDir::homePath(), tr("PDF (*.pdf)"));

    if (!fileName.isEmpty()) {
        if (!fileName.endsWith(QLatin1String(".pdf"), Qt::CaseInsensitive)) {
            fileName.append(QLatin1String(".pdf"));
        }

        MD::Parser<MD::QStringTrait> parser;

        std::shared_ptr<MD::Document<MD::QStringTrait>> doc;

        if (m_ui->m_workingDirBox->isChecked()) {
            doc = parser.parse(m_ui->m_fileName->text(), m_ui->m_workingDirectory->currentPath(),
                               m_ui->m_recursive->isChecked());
        } else {
            doc = parser.parse(m_ui->m_fileName->text(), m_ui->m_recursive->isChecked());
        }

        if (!doc->isEmpty()) {
            auto *pdf = new Render::PdfRenderer();
            pdf->moveToThread(m_thread);

            Render::RenderOpts opts;

            opts.m_textFont = m_ui->m_textFont->currentFont().family();
            opts.m_textFontSize = m_ui->m_textFontSize->value();
            opts.m_codeFont = m_ui->m_codeFont->currentFont().family();
            opts.m_codeFontSize = m_ui->m_codeFontSize->value();
            opts.m_linkColor = m_ui->m_linkColor->color();
            opts.m_borderColor = m_ui->m_borderColor->color();
            opts.m_left = (m_ui->m_pt->isChecked() ? m_ui->m_left->value() : m_ui->m_left->value() / s_mmInPt);
            opts.m_right = (m_ui->m_pt->isChecked() ? m_ui->m_right->value() : m_ui->m_right->value() / s_mmInPt);
            opts.m_top = (m_ui->m_pt->isChecked() ? m_ui->m_top->value() : m_ui->m_top->value() / s_mmInPt);
            opts.m_bottom = (m_ui->m_pt->isChecked() ? m_ui->m_bottom->value() : m_ui->m_bottom->value() / s_mmInPt);
            opts.m_dpi = m_ui->m_dpi->value();
            opts.m_syntax = m_syntax;
            m_syntax->setTheme(m_syntax->themeForName(m_ui->m_codeTheme->currentText()));
            opts.m_imageAlignment = imageAlignmentFromInt(m_ui->m_imageAlignment->currentIndex());

            ProgressDlg progress(pdf, this);

            pdf->render(fileName, doc, opts);

            if (progress.exec() == QDialog::Accepted) {
                QMessageBox::information(this, tr("Markdown processed..."),
                                         tr("PDF generated. Have a look at the result. Thank you."));
            } else {
                if (!progress.errorMsg().isEmpty()) {
                    QMessageBox::critical(this, tr("Error during rendering PDF..."),
                                          tr("%1\n\nOutput PDF is broken. Sorry.").arg(progress.errorMsg()));
                } else {
                    QMessageBox::information(this, tr("Canceled..."), tr("PDF generation is canceled."));
                }
            }
        } else {
            QMessageBox::warning(this, tr("Markdown is empty..."), tr("Input Markdown file is empty. Nothing saved."));
        }
    }
}

void MainWidget::codeFontSizeChanged(int i)
{
    if (i > m_ui->m_textFontSize->value()) {
        m_ui->m_codeFontSize->setValue(m_ui->m_textFontSize->value());
    }
}

void MainWidget::textFontSizeChanged(int i)
{
    if (i < m_ui->m_codeFontSize->value()) {
        m_ui->m_codeFontSize->setValue(m_ui->m_textFontSize->value());
    }
}

void MainWidget::mmButtonToggled(bool on)
{
    if (on) {
        m_ui->m_left->setValue(qRound(m_ui->m_left->value() * s_mmInPt));
        m_ui->m_right->setValue(qRound(m_ui->m_right->value() * s_mmInPt));
        m_ui->m_top->setValue(qRound(m_ui->m_top->value() * s_mmInPt));
        m_ui->m_bottom->setValue(qRound(m_ui->m_bottom->value() * s_mmInPt));

        m_ui->m_left->setMaximum(50);
        m_ui->m_right->setMaximum(50);
        m_ui->m_top->setMaximum(50);
        m_ui->m_bottom->setMaximum(50);
    } else {
        m_ui->m_left->setMaximum(qRound(50 / s_mmInPt));
        m_ui->m_right->setMaximum(qRound(50 / s_mmInPt));
        m_ui->m_top->setMaximum(qRound(50 / s_mmInPt));
        m_ui->m_bottom->setMaximum(qRound(50 / s_mmInPt));

        m_ui->m_left->setValue(qRound(m_ui->m_left->value() / s_mmInPt));
        m_ui->m_right->setValue(qRound(m_ui->m_right->value() / s_mmInPt));
        m_ui->m_top->setValue(qRound(m_ui->m_top->value() / s_mmInPt));
        m_ui->m_bottom->setValue(qRound(m_ui->m_bottom->value() / s_mmInPt));
    }
}

void MainWidget::textFontChanged(const QFont &f)
{
    static const QString defaultColor = m_ui->m_textFont->palette().color(QPalette::Text).name();

    if (!Render::PdfRenderer::isFontCreatable(f.family(), false)) {
        m_ui->m_textFont->setStyleSheet(QStringLiteral("QFontComboBox { color: red }"));
        m_textFontOk = false;

        m_ui->m_startBtn->setEnabled(false);
    } else {
        m_ui->m_textFont->setStyleSheet(QStringLiteral("QFontComboBox { color: %1 }").arg(defaultColor));
        m_textFontOk = true;

        if (!m_ui->m_fileName->text().isEmpty() && m_codeFontOk) {
            m_ui->m_startBtn->setEnabled(true);
        } else {
            m_ui->m_startBtn->setEnabled(false);
        }
    }
}

void MainWidget::codeFontChanged(const QFont &f)
{
    static const QString defaultColor = m_ui->m_codeFont->palette().color(QPalette::Text).name();

    if (!Render::PdfRenderer::isFontCreatable(f.family(), true)) {
        m_ui->m_codeFont->setStyleSheet(QStringLiteral("QFontComboBox { color: red }"));
        m_codeFontOk = false;

        m_ui->m_startBtn->setEnabled(false);
    } else {
        m_ui->m_codeFont->setStyleSheet(QStringLiteral("QFontComboBox { color: %1 }").arg(defaultColor));
        m_codeFontOk = true;

        if (!m_ui->m_fileName->text().isEmpty() && m_textFontOk) {
            m_ui->m_startBtn->setEnabled(true);
        } else {
            m_ui->m_startBtn->setEnabled(false);
        }
    }
}

//
// MainWindow
//

MainWindow::MainWindow()
{
    setWindowTitle(tr("MD-PDF Converter"));

    auto file = menuBar()->addMenu(tr("&File"));
    file->addAction(QIcon::fromTheme(QStringLiteral("application-exit"),
                                     QIcon(QStringLiteral(":/img/application-exit.png"))),
                    tr("&Quit"),
                    this,
                    &MainWindow::quit);

    auto help = menuBar()->addMenu(tr("&Help"));
    help->addAction(QIcon(QStringLiteral(":/icon/icon_24x24.png")), tr("About"), this, &MainWindow::about);
    help->addAction(QIcon(QStringLiteral(":/img/Qt-logo-neon-transparent.png")),
                    tr("About Qt"), this, &MainWindow::aboutQt);
    help->addAction(QIcon::fromTheme(QStringLiteral("bookmarks-organize"),
                                     QIcon(QStringLiteral(":/img/bookmarks-organize.png"))),
                    tr("Licenses"),
                    this,
                    &MainWindow::licenses);

    ui = new MainWidget(this);

    setCentralWidget(ui);
}

void MainWindow::readCfg()
{
    QSettings s;

    ui->applyCfg(s);

    s.beginGroup(QStringLiteral("window"));

    const auto width = s.value(QStringLiteral("width")).toInt();
    const auto height = s.value(QStringLiteral("height")).toInt();

    if (width > 0 && height > 0) {
        resize(width, height);

        const auto x = s.value(QStringLiteral("x")).toInt();
        const auto y = s.value(QStringLiteral("y")).toInt();

        windowHandle()->setX(x);
        windowHandle()->setY(y);

        const auto maximized = s.value(QStringLiteral("maximized")).toBool();
        if (maximized) {
            showMaximized();
        }
    }

    s.endGroup();
}

void MainWindow::saveCfg()
{
    QSettings s;

    ui->saveCfg(s);

    s.beginGroup(QStringLiteral("window"));

    s.setValue(QStringLiteral("width"), width());
    s.setValue(QStringLiteral("height"), height());
    s.setValue(QStringLiteral("x"), windowHandle()->x());
    s.setValue(QStringLiteral("y"), windowHandle()->y());
    s.setValue(QStringLiteral("maximized"), isMaximized());

    s.endGroup();
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    event->accept();

    saveCfg();
}

void MainWindow::showEvent(QShowEvent *event)
{
    if (!m_alreadyShown) {
        m_alreadyShown = true;

        readCfg();
    }

    event->accept();
}

void MainWindow::setMarkdownFile(const QString &fileName)
{
    ui->setMarkdownFile(fileName);
}

void MainWindow::about()
{
    QMessageBox::about(this,
                       tr("About MD-PDF Converter"),
                       tr("MD-PDF Converter.\n\n"
                          "Version %1\n\n"
                          "md4qt version %2\n\n"
                          "Author - Igor Mironchik (igor.mironchik at gmail dot com).\n\n"
                          "Copyright (c) 2019-2025 Igor Mironchik.\n\n"
                          "Licensed under GNU GPL 3.0.")
                           .arg(c_version, c_md4qtVersion));
}

void MainWindow::aboutQt()
{
    QMessageBox::aboutQt(this);
}

void MainWindow::licenses()
{
    MdShared::LicenseDialog msg(this);
    msg.addLicense(QStringLiteral("The Oxygen Icon Theme"),
                   QStringLiteral("<p><b>The Oxygen Icon Theme</b>\n\n</p>"
                                  "<p>Copyright (C) 2007 Nuno Pinheiro &lt;nuno@oxygen-icons.org&gt;\n</p>"
                                  "<p>Copyright (C) 2007 David Vignoni &lt;david@icon-king.com&gt;\n</p>"
                                  "<p>Copyright (C) 2007 David Miller &lt;miller@oxygen-icons.org&gt;\n</p>"
                                  "<p>Copyright (C) 2007 Johann Ollivier Lapeyre &lt;johann@oxygen-icons.org&gt;\n</p>"
                                  "<p>Copyright (C) 2007 Kenneth Wimer &lt;kwwii@bootsplash.org&gt;\n</p>"
                                  "<p>Copyright (C) 2007 Riccardo Iaconelli &lt;riccardo@oxygen-icons.org&gt;\n</p>"
                                  "<p>\nand others\n</p>"
                                  "\n"
                                  "<p>This library is free software; you can redistribute it and/or "
                                  "modify it under the terms of the GNU Lesser General Public "
                                  "License as published by the Free Software Foundation; either "
                                  "version 3 of the License, or (at your option) any later version.\n</p>"
                                  "\n"
                                  "<p>This library is distributed in the hope that it will be useful, "
                                  "but WITHOUT ANY WARRANTY; without even the implied warranty of "
                                  "MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU "
                                  "Lesser General Public License for more details.\n</p>"
                                  "\n"
                                  "<p>You should have received a copy of the GNU Lesser General Public "
                                  "License along with this library. If not, see "
                                  "<a href=\"http://www.gnu.org/licenses/\">&lt;http://www.gnu.org/licenses/&gt;</a>.\n</p>"
                                  "\n"
                                  "<p>Clarification:\n</p>"
                                  "\n"
                                  "<p>The GNU Lesser General Public License or LGPL is written for "
                                  "software libraries in the first place. We expressly want the LGPL to "
                                  "be valid for this artwork library too.\n</p>"
                                  "\n"
                                  "<p>KDE Oxygen theme icons is a special kind of software library, it is an "
                                  "artwork library, it's elements can be used in a Graphical User Interface, or "
                                  "GUI.\n</p>"
                                  "\n"
                                  "<p>Source code, for this library means:\n</p>"
                                  "<p><ul> <li>where they exist, SVG;\n</li>"
                                  " <li>otherwise, if applicable, the multi-layered formats xcf or psd, or "
                                  "otherwise png.\n</li></ul></p>"
                                  "\n"
                                  "<p>The LGPL in some sections obliges you to make the files carry "
                                  "notices. With images this is in some cases impossible or hardly useful.\n</p>"
                                  "\n"
                                  "<p>With this library a notice is placed at a prominent place in the directory "
                                  "containing the elements. You may follow this practice.\n</p>"
                                  "\n"
                                  "<p>The exception in section 5 of the GNU Lesser General Public License covers "
                                  "the use of elements of this art library in a GUI.\n</p>"
                                  "\n"
                                  "<p>kde-artists [at] kde.org\n</p>"
                                  "\n"
                                  "<p><b>GNU LESSER GENERAL PUBLIC LICENSE</b>\n</p>"
                                  "<p>Version 3, 29 June 2007\n</p>"
                                  "\n"
                                  "<p>Copyright (C) 2007 Free Software Foundation, Inc. <a href=\"http://fsf.org/\">&lt;http://fsf.org/&gt;</a> "
                                  "Everyone is permitted to copy and distribute verbatim copies "
                                  "of this license document, but changing it is not allowed.\n</p>"
                                  "\n"
                                  "\n"
                                  "<p>This version of the GNU Lesser General Public License incorporates "
                                  "the terms and conditions of version 3 of the GNU General Public "
                                  "License, supplemented by the additional permissions listed below.\n</p>"
                                  "\n"
                                  "<p><b>0.</b> Additional Definitions.\n</p>"
                                  "\n"
                                  "<p>As used herein, \"this License\" refers to version 3 of the GNU Lesser "
                                  "General Public License, and the \"GNU GPL\" refers to version 3 of the GNU "
                                  "General Public License.\n</p>"
                                  "\n"
                                  "<p>\"The Library\" refers to a covered work governed by this License, "
                                  "other than an Application or a Combined Work as defined below.\n</p>"
                                  "\n"
                                  "<p>An \"Application\" is any work that makes use of an interface provided "
                                  "by the Library, but which is not otherwise based on the Library. "
                                  "Defining a subclass of a class defined by the Library is deemed a mode "
                                  "of using an interface provided by the Library.\n</p>"
                                  "\n"
                                  "<p>A \"Combined Work\" is a work produced by combining or linking an "
                                  "Application with the Library.  The particular version of the Library "
                                  "with which the Combined Work was made is also called the \"Linked "
                                  "Version\".\n</p>"
                                  "\n"
                                  "<p>The \"Minimal Corresponding Source\" for a Combined Work means the "
                                  "Corresponding Source for the Combined Work, excluding any source code "
                                  "for portions of the Combined Work that, considered in isolation, are "
                                  "based on the Application, and not on the Linked Version.\n</p>"
                                  "\n"
                                  "<p>The \"Corresponding Application Code\" for a Combined Work means the "
                                  "object code and/or source code for the Application, including any data "
                                  "and utility programs needed for reproducing the Combined Work from the "
                                  "Application, but excluding the System Libraries of the Combined Work.\n</p>"
                                  "\n"
                                  "<p><b>1.</b> Exception to Section 3 of the GNU GPL.\n</p>"
                                  "\n"
                                  "<p>You may convey a covered work under sections 3 and 4 of this License "
                                  "without being bound by section 3 of the GNU GPL.\n</p>"
                                  "\n"
                                  "<p><b>2.</b> Conveying Modified Versions.\n</p>"
                                  "\n"
                                  "<p>If you modify a copy of the Library, and, in your modifications, a "
                                  "facility refers to a function or data to be supplied by an Application "
                                  "that uses the facility (other than as an argument passed when the "
                                  "facility is invoked), then you may convey a copy of the modified "
                                  "version:\n</p>"
                                  "\n"
                                  "<p><b>a)</b> under this License, provided that you make a good faith effort to "
                                  "ensure that, in the event an Application does not supply the "
                                  "function or data, the facility still operates, and performs "
                                  "whatever part of its purpose remains meaningful, or\n</p>"
                                  "\n"
                                  "<p><b>b)</b> under the GNU GPL, with none of the additional permissions of "
                                  "this License applicable to that copy.\n</p>"
                                  "\n"
                                  "<p><b>3.</b> Object Code Incorporating Material from Library Header Files.\n</p>"
                                  "\n"
                                  "<p>The object code form of an Application may incorporate material from "
                                  "a header file that is part of the Library.  You may convey such object "
                                  "code under terms of your choice, provided that, if the incorporated "
                                  "material is not limited to numerical parameters, data structure "
                                  "layouts and accessors, or small macros, inline functions and templates "
                                  "(ten or fewer lines in length), you do both of the following:\n</p>"
                                  "\n"
                                  "<p><b>a)</b> Give prominent notice with each copy of the object code that the "
                                  "Library is used in it and that the Library and its use are "
                                  "covered by this License.\n</p>"
                                  "\n"
                                  "<p><b>b)</b> Accompany the object code with a copy of the GNU GPL and this license "
                                  "document.\n</p>"
                                  "\n"
                                  "<p><b>4.</b> Combined Works.\n</p>"
                                  "\n"
                                  "<p>You may convey a Combined Work under terms of your choice that, "
                                  "taken together, effectively do not restrict modification of the "
                                  "portions of the Library contained in the Combined Work and reverse "
                                  "engineering for debugging such modifications, if you also do each of "
                                  "the following:\n</p>"
                                  "\n"
                                  "<p><b>a)</b> Give prominent notice with each copy of the Combined Work that "
                                  "the Library is used in it and that the Library and its use are "
                                  "covered by this License.\n</p>"
                                  "\n"
                                  "<p><b>b)</b> Accompany the Combined Work with a copy of the GNU GPL and this license "
                                  "document.\n</p>"
                                  "\n"
                                  "<p><b>c)</b> For a Combined Work that displays copyright notices during "
                                  "execution, include the copyright notice for the Library among "
                                  "these notices, as well as a reference directing the user to the "
                                  "copies of the GNU GPL and this license document.\n</p>"
                                  "\n"
                                  "<p><b>d)</b> Do one of the following:\n</p>"
                                  "\n"
                                  "<p>    <b>0)</b> Convey the Minimal Corresponding Source under the terms of this "
                                  "License, and the Corresponding Application Code in a form "
                                  "suitable for, and under terms that permit, the user to "
                                  "recombine or relink the Application with a modified version of "
                                  "the Linked Version to produce a modified Combined Work, in the "
                                  "manner specified by section 6 of the GNU GPL for conveying "
                                  "Corresponding Source.\n</p>"
                                  "\n"
                                  "<p>    <b>1)</b> Use a suitable shared library mechanism for linking with the "
                                  "Library.  A suitable mechanism is one that (a) uses at run time "
                                  "a copy of the Library already present on the user's computer "
                                  "system, and (b) will operate properly with a modified version "
                                  "of the Library that is interface-compatible with the Linked "
                                  "Version.\n</p>"
                                  "\n"
                                  "<p><b>e)</b> Provide Installation Information, but only if you would otherwise "
                                  "be required to provide such information under section 6 of the "
                                  "GNU GPL, and only to the extent that such information is "
                                  "necessary to install and execute a modified version of the "
                                  "Combined Work produced by recombining or relinking the "
                                  "Application with a modified version of the Linked Version. (If "
                                  "you use option 4d0, the Installation Information must accompany "
                                  "the Minimal Corresponding Source and Corresponding Application "
                                  "Code. If you use option 4d1, you must provide the Installation "
                                  "Information in the manner specified by section 6 of the GNU GPL "
                                  "for conveying Corresponding Source.)\n</p>"
                                  "\n"
                                  "<p><b>5.</b> Combined Libraries.\n</p>"
                                  "\n"
                                  "<p>You may place library facilities that are a work based on the "
                                  "Library side by side in a single library together with other library "
                                  "facilities that are not Applications and are not covered by this "
                                  "License, and convey such a combined library under terms of your "
                                  "choice, if you do both of the following:\n</p>"
                                  "\n"
                                  "<p><b>a)</b> Accompany the combined library with a copy of the same work based "
                                  "on the Library, uncombined with any other library facilities, "
                                  "conveyed under the terms of this License.\n</p>"
                                  "\n"
                                  "<p><b>b)</b> Give prominent notice with the combined library that part of it "
                                  "is a work based on the Library, and explaining where to find the "
                                  "accompanying uncombined form of the same work.\n</p>"
                                  "\n"
                                  "<p><b>6.</b> Revised Versions of the GNU Lesser General Public License.\n</p>"
                                  "\n"
                                  "<p>The Free Software Foundation may publish revised and/or new versions "
                                  "of the GNU Lesser General Public License from time to time. Such new "
                                  "versions will be similar in spirit to the present version, but may "
                                  "differ in detail to address new problems or concerns.\n</p>"
                                  "\n"
                                  "<p>Each version is given a distinguishing version number. If the "
                                  "Library as you received it specifies that a certain numbered version "
                                  "of the GNU Lesser General Public License \"or any later version\" "
                                  "applies to it, you have the option of following the terms and "
                                  "conditions either of that published version or of any later version "
                                  "published by the Free Software Foundation. If the Library as you "
                                  "received it does not specify a version number of the GNU Lesser "
                                  "General Public License, you may choose any version of the GNU Lesser "
                                  "General Public License ever published by the Free Software Foundation.\n</p>"
                                  "\n"
                                  "<p>If the Library as you received it specifies that a proxy can decide "
                                  "whether future versions of the GNU Lesser General Public License shall "
                                  "apply, that proxy's public statement of acceptance of any version is "
                                  "permanent authorization for you to choose that version for the "
                                  "Library.</p>"));

    msg.addLicense(QStringLiteral("PoDoFo"),
                   QStringLiteral("<p><b>PoDoFo License\n\n</b></p>"
                                  "<p>GNU LIBRARY GENERAL PUBLIC LICENSE\n</p>"
                                  "<p>Version 2, June 1991\n</p>"
                                  "\n"
                                  "<p>Copyright (C) 1991 Free Software Foundation, Inc.\n</p>"
                                  "<p>59 Temple Place - Suite 330\n</p>"
                                  "<p>Boston, MA 02111-1307, USA.\n</p>"
                                  "<p>Everyone is permitted to copy and distribute verbatim copies "
                                  "of this license document, but changing it is not allowed.\n</p>"
                                  "\n"
                                  "<p>[This is the first released version of the library GPL.  It is "
                                  "numbered 2 because it goes with version 2 of the ordinary GPL.]\n</p>"
                                  "\n"
                                  "<p>Preamble\n</p>"
                                  "\n"
                                  "<p>The licenses for most software are designed to take away your "
                                  "freedom to share and change it.  By contrast, the GNU General Public "
                                  "Licenses are intended to guarantee your freedom to share and change "
                                  "free software--to make sure the software is free for all its users.\n</p>"
                                  "\n"
                                  "<p>This license, the Library General Public License, applies to some "
                                  "specially designated Free Software Foundation software, and to any "
                                  "other libraries whose authors decide to use it.  You can use it for "
                                  "your libraries, too.\n</p>"
                                  "\n"
                                  "<p>When we speak of free software, we are referring to freedom, not "
                                  "price.  Our General Public Licenses are designed to make sure that you "
                                  "have the freedom to distribute copies of free software (and charge for "
                                  "this service if you wish), that you receive source code or can get it "
                                  "if you want it, that you can change the software or use pieces of it "
                                  "in new free programs; and that you know you can do these things.\n</p>"
                                  "\n"
                                  "<p>To protect your rights, we need to make restrictions that forbid "
                                  "anyone to deny you these rights or to ask you to surrender the rights. "
                                  "These restrictions translate to certain responsibilities for you if "
                                  "you distribute copies of the library, or if you modify it.\n</p>"
                                  "\n"
                                  "<p>For example, if you distribute copies of the library, whether gratis "
                                  "or for a fee, you must give the recipients all the rights that we gave "
                                  "you.  You must make sure that they, too, receive or can get the source "
                                  "code.  If you link a program with the library, you must provide "
                                  "complete object files to the recipients so that they can relink them "
                                  "with the library, after making changes to the library and recompiling "
                                  "it.  And you must show them these terms so they know their rights.\n</p>"
                                  "\n"
                                  "<p>Our method of protecting your rights has two steps: (1) copyright "
                                  "the library, and (2) offer you this license which gives you legal "
                                  "permission to copy, distribute and/or modify the library.\n</p>"
                                  "\n"
                                  "<p>Also, for each distributor's protection, we want to make certain "
                                  "that everyone understands that there is no warranty for this free "
                                  "library.  If the library is modified by someone else and passed on, we "
                                  "want its recipients to know that what they have is not the original "
                                  "version, so that any problems introduced by others will not reflect on "
                                  "the original authors' reputations.\n</p>"
                                  "\n"
                                  "<p>Finally, any free program is threatened constantly by software "
                                  "patents.  We wish to avoid the danger that companies distributing free "
                                  "software will individually obtain patent licenses, thus in effect "
                                  "transforming the program into proprietary software.  To prevent this, "
                                  "we have made it clear that any patent must be licensed for everyone's "
                                  "free use or not licensed at all.\n</p>"
                                  "\n"
                                  "<p>Most GNU software, including some libraries, is covered by the ordinary "
                                  "GNU General Public License, which was designed for utility programs.  This "
                                  "license, the GNU Library General Public License, applies to certain "
                                  "designated libraries.  This license is quite different from the ordinary "
                                  "one; be sure to read it in full, and don't assume that anything in it is "
                                  "the same as in the ordinary license.\n</p>"
                                  "\n"
                                  "<p>The reason we have a separate public license for some libraries is that "
                                  "they blur the distinction we usually make between modifying or adding to a "
                                  "program and simply using it.  Linking a program with a library, without "
                                  "changing the library, is in some sense simply using the library, and is "
                                  "analogous to running a utility program or application program.  However, in "
                                  "a textual and legal sense, the linked executable is a combined work, a "
                                  "derivative of the original library, and the ordinary General Public License "
                                  "treats it as such.\n</p>"
                                  "\n"
                                  "<p>Because of this blurred distinction, using the ordinary General "
                                  "Public License for libraries did not effectively promote software "
                                  "sharing, because most developers did not use the libraries.  We "
                                  "concluded that weaker conditions might promote sharing better.\n</p>"
                                  "\n"
                                  "</p>However, unrestricted linking of non-free programs would deprive the "
                                  "users of those programs of all benefit from the free status of the "
                                  "libraries themselves.  This Library General Public License is intended to "
                                  "permit developers of non-free programs to use free libraries, while "
                                  "preserving your freedom as a user of such programs to change the free "
                                  "libraries that are incorporated in them.  (We have not seen how to achieve "
                                  "this as regards changes in header files, but we have achieved it as regards "
                                  "changes in the actual functions of the Library.)  The hope is that this "
                                  "will lead to faster development of free libraries.\n</p>"
                                  "\n"
                                  "<p>The precise terms and conditions for copying, distribution and "
                                  "modification follow.  Pay close attention to the difference between a "
                                  "\"work based on the library\" and a \"work that uses the library\".  The "
                                  "former contains code derived from the library, while the latter only "
                                  "works together with the library.\n</p>"
                                  "\n"
                                  "<p>Note that it is possible for a library to be covered by the ordinary "
                                  "General Public License rather than by this special one.\n</p>"
                                  "\n"
                                  "<p><b>GNU LIBRARY GENERAL PUBLIC LICENSE "
                                  "TERMS AND CONDITIONS FOR COPYING, DISTRIBUTION AND MODIFICATION\n</b></p>"
                                  "\n"
                                  "<p><b>0.</b> This License Agreement applies to any software library which "
                                  "contains a notice placed by the copyright holder or other authorized "
                                  "party saying it may be distributed under the terms of this Library "
                                  "General Public License (also called \"this License\").  Each licensee is "
                                  "addressed as \"you\".\n</p>"
                                  "\n"
                                  "<p>A \"library\" means a collection of software functions and/or data "
                                  "prepared so as to be conveniently linked with application programs "
                                  "(which use some of those functions and data) to form executables.\n</p>"
                                  "\n"
                                  "<p>The \"Library\", below, refers to any such software library or work "
                                  "which has been distributed under these terms.  A \"work based on the "
                                  "Library\" means either the Library or any derivative work under "
                                  "copyright law: that is to say, a work containing the Library or a "
                                  "portion of it, either verbatim or with modifications and/or translated "
                                  "straightforwardly into another language.  (Hereinafter, translation is "
                                  "included without limitation in the term \"modification\".)\n</p>"
                                  "\n"
                                  "<p>\"Source code\" for a work means the preferred form of the work for "
                                  "making modifications to it.  For a library, complete source code means "
                                  "all the source code for all modules it contains, plus any associated "
                                  "interface definition files, plus the scripts used to control compilation "
                                  "and installation of the library.\n</p>"
                                  "\n"
                                  "<p>Activities other than copying, distribution and modification are not "
                                  "covered by this License; they are outside its scope.  The act of "
                                  "running a program using the Library is not restricted, and output from "
                                  "such a program is covered only if its contents constitute a work based "
                                  "on the Library (independent of the use of the Library in a tool for "
                                  "writing it).  Whether that is true depends on what the Library does "
                                  "and what the program that uses the Library does.\n</p>"
                                  "\n"
                                  "<p><b>1.</b> You may copy and distribute verbatim copies of the Library's "
                                  "complete source code as you receive it, in any medium, provided that "
                                  "you conspicuously and appropriately publish on each copy an "
                                  "appropriate copyright notice and disclaimer of warranty; keep intact "
                                  "all the notices that refer to this License and to the absence of any "
                                  "warranty; and distribute a copy of this License along with the "
                                  "Library.\n</p>"
                                  "\n"
                                  "<p>You may charge a fee for the physical act of transferring a copy, "
                                  "and you may at your option offer warranty protection in exchange for a "
                                  "fee.\n</p>"
                                  "\n"
                                  "<p><b>2.</b> You may modify your copy or copies of the Library or any portion "
                                  "of it, thus forming a work based on the Library, and copy and "
                                  "distribute such modifications or work under the terms of Section 1 "
                                  "above, provided that you also meet all of these conditions:\n</p>"
                                  "\n"
                                  "<p>  <b>a)</b> The modified work must itself be a software library.\n</p>"
                                  "\n"
                                  "<p>  <b>b)</b> You must cause the files modified to carry prominent notices "
                                  "stating that you changed the files and the date of any change.\n</p>"
                                  "\n"
                                  "<p>  <b>c)</b> You must cause the whole of the work to be licensed at no "
                                  "charge to all third parties under the terms of this License.\n</p>"
                                  "\n"
                                  "<p>  <b>d)</b> If a facility in the modified Library refers to a function or a "
                                  "table of data to be supplied by an application program that uses "
                                  "the facility, other than as an argument passed when the facility "
                                  "is invoked, then you must make a good faith effort to ensure that, "
                                  "in the event an application does not supply such function or "
                                  "table, the facility still operates, and performs whatever part of "
                                  "its purpose remains meaningful.\n</p>"
                                  "\n"
                                  "<p>(For example, a function in a library to compute square roots has "
                                  "a purpose that is entirely well-defined independent of the "
                                  "application.  Therefore, Subsection 2d requires that any "
                                  "application-supplied function or table used by this function must "
                                  "be optional: if the application does not supply it, the square "
                                  "root function must still compute square roots.)\n</p>"
                                  "\n"
                                  "<p>These requirements apply to the modified work as a whole.  If "
                                  "identifiable sections of that work are not derived from the Library, "
                                  "and can be reasonably considered independent and separate works in "
                                  "themselves, then this License, and its terms, do not apply to those "
                                  "sections when you distribute them as separate works.  But when you "
                                  "distribute the same sections as part of a whole which is a work based "
                                  "on the Library, the distribution of the whole must be on the terms of "
                                  "this License, whose permissions for other licensees extend to the "
                                  "entire whole, and thus to each and every part regardless of who wrote "
                                  "it.\n</p>"
                                  "\n"
                                  "<p>Thus, it is not the intent of this section to claim rights or contest "
                                  "your rights to work written entirely by you; rather, the intent is to "
                                  "exercise the right to control the distribution of derivative or "
                                  "collective works based on the Library.\n</p>"
                                  "\n"
                                  "<p>In addition, mere aggregation of another work not based on the Library "
                                  "with the Library (or with a work based on the Library) on a volume of "
                                  "a storage or distribution medium does not bring the other work under "
                                  "the scope of this License.\n</p>"
                                  "\n"
                                  "<p><b>3.</b> You may opt to apply the terms of the ordinary GNU General Public "
                                  "License instead of this License to a given copy of the Library.  To do "
                                  "this, you must alter all the notices that refer to this License, so "
                                  "that they refer to the ordinary GNU General Public License, version 2, "
                                  "instead of to this License.  (If a newer version than version 2 of the "
                                  "ordinary GNU General Public License has appeared, then you can specify "
                                  "that version instead if you wish.)  Do not make any other change in "
                                  "these notices.\n</p>"
                                  "\n"
                                  "<p>Once this change is made in a given copy, it is irreversible for "
                                  "that copy, so the ordinary GNU General Public License applies to all "
                                  "subsequent copies and derivative works made from that copy.\n</p>"
                                  "\n"
                                  "<p>This option is useful when you wish to copy part of the code of "
                                  "the Library into a program that is not a library.\n</p>"
                                  "\n"
                                  "<p><b>4.</b> You may copy and distribute the Library (or a portion or "
                                  "derivative of it, under Section 2) in object code or executable form "
                                  "under the terms of Sections 1 and 2 above provided that you accompany "
                                  "it with the complete corresponding machine-readable source code, which "
                                  "must be distributed under the terms of Sections 1 and 2 above on a "
                                  "medium customarily used for software interchange.\n</p>"
                                  "\n"
                                  "<p>If distribution of object code is made by offering access to copy "
                                  "from a designated place, then offering equivalent access to copy the "
                                  "source code from the same place satisfies the requirement to "
                                  "distribute the source code, even though third parties are not "
                                  "compelled to copy the source along with the object code.\n</p>"
                                  "\n"
                                  "<p><b>5.</b> A program that contains no derivative of any portion of the "
                                  "Library, but is designed to work with the Library by being compiled or "
                                  "linked with it, is called a \"work that uses the Library\".  Such a "
                                  "work, in isolation, is not a derivative work of the Library, and "
                                  "therefore falls outside the scope of this License.\n</p>"
                                  "\n"
                                  "<p>However, linking a \"work that uses the Library\" with the Library "
                                  "creates an executable that is a derivative of the Library (because it "
                                  "contains portions of the Library), rather than a \"work that uses the "
                                  "library\".  The executable is therefore covered by this License. "
                                  "Section 6 states terms for distribution of such executables.\n</p>"
                                  "\n"
                                  "<p>When a \"work that uses the Library\" uses material from a header file "
                                  "that is part of the Library, the object code for the work may be a "
                                  "derivative work of the Library even though the source code is not. "
                                  "Whether this is true is especially significant if the work can be "
                                  "linked without the Library, or if the work is itself a library.  The "
                                  "threshold for this to be true is not precisely defined by law.\n</p>"
                                  "\n"
                                  "<p>If such an object file uses only numerical parameters, data "
                                  "structure layouts and accessors, and small macros and small inline "
                                  "functions (ten lines or less in length), then the use of the object "
                                  "file is unrestricted, regardless of whether it is legally a derivative "
                                  "work.  (Executables containing this object code plus portions of the "
                                  "Library will still fall under Section 6.)\n</p>"
                                  "\n"
                                  "<p>Otherwise, if the work is a derivative of the Library, you may "
                                  "distribute the object code for the work under the terms of Section 6. "
                                  "Any executables containing that work also fall under Section 6, "
                                  "whether or not they are linked directly with the Library itself.\n</p>"
                                  "\n"
                                  "<p><b>6.</b> As an exception to the Sections above, you may also compile or "
                                  "link a \"work that uses the Library\" with the Library to produce a "
                                  "work containing portions of the Library, and distribute that work "
                                  "under terms of your choice, provided that the terms permit "
                                  "modification of the work for the customer's own use and reverse "
                                  "engineering for debugging such modifications.\n</p>"
                                  "\n"
                                  "<p>You must give prominent notice with each copy of the work that the "
                                  "Library is used in it and that the Library and its use are covered by "
                                  "this License.  You must supply a copy of this License.  If the work "
                                  "during execution displays copyright notices, you must include the "
                                  "copyright notice for the Library among them, as well as a reference "
                                  "directing the user to the copy of this License.  Also, you must do one "
                                  "of these things:\n</p>"
                                  "\n"
                                  "<p>  <b>a)</b> Accompany the work with the complete corresponding "
                                  "machine-readable source code for the Library including whatever "
                                  "changes were used in the work (which must be distributed under "
                                  "Sections 1 and 2 above); and, if the work is an executable linked "
                                  "with the Library, with the complete machine-readable \"work that "
                                  "uses the Library\", as object code and/or source code, so that the "
                                  "user can modify the Library and then relink to produce a modified "
                                  "executable containing the modified Library.  (It is understood "
                                  "that the user who changes the contents of definitions files in the "
                                  "Library will not necessarily be able to recompile the application "
                                  "to use the modified definitions.)\n</p>"
                                  "\n"
                                  "<p>  <b>b)</b> Accompany the work with a written offer, valid for at "
                                  "least three years, to give the same user the materials "
                                  "specified in Subsection 6a, above, for a charge no more "
                                  "than the cost of performing this distribution.\n</p>"
                                  "\n"
                                  "<p>  <b>c)</b> If distribution of the work is made by offering access to copy "
                                  "from a designated place, offer equivalent access to copy the above "
                                  "specified materials from the same place.\n</p>"
                                  "\n"
                                  "<p>  <b>d)</b> Verify that the user has already received a copy of these "
                                  "materials or that you have already sent this user a copy.\n</p>"
                                  "\n"
                                  "<p>For an executable, the required form of the \"work that uses the "
                                  "Library\" must include any data and utility programs needed for "
                                  "reproducing the executable from it.  However, as a special exception, "
                                  "the source code distributed need not include anything that is normally "
                                  "distributed (in either source or binary form) with the major "
                                  "components (compiler, kernel, and so on) of the operating system on "
                                  "which the executable runs, unless that component itself accompanies "
                                  "the executable.\n</p>"
                                  "\n"
                                  "<p>It may happen that this requirement contradicts the license "
                                  "restrictions of other proprietary libraries that do not normally "
                                  "accompany the operating system.  Such a contradiction means you cannot "
                                  "use both them and the Library together in an executable that you "
                                  "distribute.\n</p>"
                                  "\n"
                                  "<p><b>7.</b> You may place library facilities that are a work based on the "
                                  "Library side-by-side in a single library together with other library "
                                  "facilities not covered by this License, and distribute such a combined "
                                  "library, provided that the separate distribution of the work based on "
                                  "the Library and of the other library facilities is otherwise "
                                  "permitted, and provided that you do these two things:\n</p>"
                                  "\n"
                                  "<p>  <b>a)</b> Accompany the combined library with a copy of the same work "
                                  "based on the Library, uncombined with any other library "
                                  "facilities.  This must be distributed under the terms of the "
                                  "Sections above.\n</p>"
                                  "\n"
                                  "<p>  <b>b)</b> Give prominent notice with the combined library of the fact "
                                  "that part of it is a work based on the Library, and explaining "
                                  "where to find the accompanying uncombined form of the same work.\n</p>"
                                  "\n"
                                  "<p><b>8.</b> You may not copy, modify, sublicense, link with, or distribute "
                                  "the Library except as expressly provided under this License.  Any "
                                  "attempt otherwise to copy, modify, sublicense, link with, or "
                                  "distribute the Library is void, and will automatically terminate your "
                                  "rights under this License.  However, parties who have received copies, "
                                  "or rights, from you under this License will not have their licenses "
                                  "terminated so long as such parties remain in full compliance.\n</p>"
                                  "\n"
                                  "<p><b>9.</b> You are not required to accept this License, since you have not "
                                  "signed it.  However, nothing else grants you permission to modify or "
                                  "distribute the Library or its derivative works.  These actions are "
                                  "prohibited by law if you do not accept this License.  Therefore, by "
                                  "modifying or distributing the Library (or any work based on the "
                                  "Library), you indicate your acceptance of this License to do so, and "
                                  "all its terms and conditions for copying, distributing or modifying "
                                  "the Library or works based on it.\n</p>"
                                  "\n"
                                  "<p><b>10.</b> Each time you redistribute the Library (or any work based on the "
                                  "Library), the recipient automatically receives a license from the "
                                  "original licensor to copy, distribute, link with or modify the Library "
                                  "subject to these terms and conditions.  You may not impose any further "
                                  "restrictions on the recipients' exercise of the rights granted herein. "
                                  "You are not responsible for enforcing compliance by third parties to "
                                  "this License.\n</p>"
                                  "\n"
                                  "<p><b>11.</b> If, as a consequence of a court judgment or allegation of patent "
                                  "infringement or for any other reason (not limited to patent issues), "
                                  "conditions are imposed on you (whether by court order, agreement or "
                                  "otherwise) that contradict the conditions of this License, they do not "
                                  "excuse you from the conditions of this License.  If you cannot "
                                  "distribute so as to satisfy simultaneously your obligations under this "
                                  "License and any other pertinent obligations, then as a consequence you "
                                  "may not distribute the Library at all.  For example, if a patent "
                                  "license would not permit royalty-free redistribution of the Library by "
                                  "all those who receive copies directly or indirectly through you, then "
                                  "the only way you could satisfy both it and this License would be to "
                                  "refrain entirely from distribution of the Library.\n</p>"
                                  "\n"
                                  "<p>If any portion of this section is held invalid or unenforceable under any "
                                  "particular circumstance, the balance of the section is intended to apply, "
                                  "and the section as a whole is intended to apply in other circumstances.\n</p>"
                                  "\n"
                                  "<p>It is not the purpose of this section to induce you to infringe any "
                                  "patents or other property right claims or to contest validity of any "
                                  "such claims; this section has the sole purpose of protecting the "
                                  "integrity of the free software distribution system which is "
                                  "implemented by public license practices.  Many people have made "
                                  "generous contributions to the wide range of software distributed "
                                  "through that system in reliance on consistent application of that "
                                  "system; it is up to the author/donor to decide if he or she is willing "
                                  "to distribute software through any other system and a licensee cannot "
                                  "impose that choice.\n</p>"
                                  "\n"
                                  "<p>This section is intended to make thoroughly clear what is believed to "
                                  "be a consequence of the rest of this License.\n</p>"
                                  "\n"
                                  "<p><b>12.</b> If the distribution and/or use of the Library is restricted in "
                                  "certain countries either by patents or by copyrighted interfaces, the "
                                  "original copyright holder who places the Library under this License may add "
                                  "an explicit geographical distribution limitation excluding those countries, "
                                  "so that distribution is permitted only in or among countries not thus "
                                  "excluded.  In such case, this License incorporates the limitation as if "
                                  "written in the body of this License.\n</p>"
                                  "\n"
                                  "<p><b>13.</b> The Free Software Foundation may publish revised and/or new "
                                  "versions of the Library General Public License from time to time. "
                                  "Such new versions will be similar in spirit to the present version, "
                                  "but may differ in detail to address new problems or concerns.\n</p>"
                                  "\n"
                                  "<p>Each version is given a distinguishing version number.  If the Library "
                                  "specifies a version number of this License which applies to it and "
                                  "\"any later version\", you have the option of following the terms and "
                                  "conditions either of that version or of any later version published by "
                                  "the Free Software Foundation.  If the Library does not specify a "
                                  "license version number, you may choose any version ever published by "
                                  "the Free Software Foundation.\n</p>"
                                  "\n"
                                  "<p><b>14.</b> If you wish to incorporate parts of the Library into other free "
                                  "programs whose distribution conditions are incompatible with these, "
                                  "write to the author to ask for permission.  For software which is "
                                  "copyrighted by the Free Software Foundation, write to the Free "
                                  "Software Foundation; we sometimes make exceptions for this.  Our "
                                  "decision will be guided by the two goals of preserving the free status "
                                  "of all derivatives of our free software and of promoting the sharing "
                                  "and reuse of software generally.\n</p>"
                                  "\n"
                                  "<p>NO WARRANTY\n</p>"
                                  "\n"
                                  "<p><b>15.</b> BECAUSE THE LIBRARY IS LICENSED FREE OF CHARGE, THERE IS NO "
                                  "WARRANTY FOR THE LIBRARY, TO THE EXTENT PERMITTED BY APPLICABLE LAW. "
                                  "EXCEPT WHEN OTHERWISE STATED IN WRITING THE COPYRIGHT HOLDERS AND/OR "
                                  "OTHER PARTIES PROVIDE THE LIBRARY \"AS IS\" WITHOUT WARRANTY OF ANY "
                                  "KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, THE "
                                  "IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR "
                                  "PURPOSE.  THE ENTIRE RISK AS TO THE QUALITY AND PERFORMANCE OF THE "
                                  "LIBRARY IS WITH YOU.  SHOULD THE LIBRARY PROVE DEFECTIVE, YOU ASSUME "
                                  "THE COST OF ALL NECESSARY SERVICING, REPAIR OR CORRECTION.\n</p>"
                                  "\n"
                                  "<p><b>16.</b> IN NO EVENT UNLESS REQUIRED BY APPLICABLE LAW OR AGREED TO IN "
                                  "WRITING WILL ANY COPYRIGHT HOLDER, OR ANY OTHER PARTY WHO MAY MODIFY "
                                  "AND/OR REDISTRIBUTE THE LIBRARY AS PERMITTED ABOVE, BE LIABLE TO YOU "
                                  "FOR DAMAGES, INCLUDING ANY GENERAL, SPECIAL, INCIDENTAL OR "
                                  "CONSEQUENTIAL DAMAGES ARISING OUT OF THE USE OR INABILITY TO USE THE "
                                  "LIBRARY (INCLUDING BUT NOT LIMITED TO LOSS OF DATA OR DATA BEING "
                                  "RENDERED INACCURATE OR LOSSES SUSTAINED BY YOU OR THIRD PARTIES OR A "
                                  "FAILURE OF THE LIBRARY TO OPERATE WITH ANY OTHER SOFTWARE), EVEN IF "
                                  "SUCH HOLDER OR OTHER PARTY HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH "
                                  "DAMAGES.\n</p>"
                                  "\n"
                                  "<p>END OF TERMS AND CONDITIONS\n\n</p>"
                                  "<p>How to Apply These Terms to Your New Libraries\n</p>"
                                  "\n"
                                  "<p>If you develop a new library, and you want it to be of the greatest "
                                  "possible use to the public, we recommend making it free software that "
                                  "everyone can redistribute and change.  You can do so by permitting "
                                  "redistribution under these terms (or, alternatively, under the terms of the "
                                  "ordinary General Public License).\n</p>"
                                  "\n"
                                  "<p>To apply these terms, attach the following notices to the library.  It is "
                                  "safest to attach them to the start of each source file to most effectively "
                                  "convey the exclusion of warranty; and each file should have at least the "
                                  "\"copyright\" line and a pointer to where the full notice is found.\n</p>"
                                  "\n"
                                  "<p>&lt;one line to give the library's name and a brief idea of what it does.&gt;\n</p>"
                                  "<p>Copyright (C) &lt;year&gt;  &lt;name of author&gt;\n</p>"
                                  "\n"
                                  "<p>This library is free software; you can redistribute it and/or "
                                  "modify it under the terms of the GNU Lesser General Public "
                                  "License as published by the Free Software Foundation; either "
                                  "version 2 of the License, or (at your option) any later version.\n</p>"
                                  "\n"
                                  "<p>This library is distributed in the hope that it will be useful, "
                                  "but WITHOUT ANY WARRANTY; without even the implied warranty of "
                                  "MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU "
                                  "Lesser General Public License for more details.\n</p>"
                                  "\n"
                                  "<p>You should have received a copy of the GNU Lesser General Public "
                                  "License along with this library; if not, write to the Free Software "
                                  "Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA\n</p>"
                                  "\n"
                                  "<p>Also add information on how to contact you by electronic and paper mail.\n</p>"
                                  "\n"
                                  "<p>You should also get your employer (if you work as a programmer) or your "
                                  "school, if any, to sign a \"copyright disclaimer\" for the library, if "
                                  "necessary.  Here is a sample; alter the names:\n</p>"
                                  "\n"
                                  "<p>Yoyodyne, Inc., hereby disclaims all copyright interest in the "
                                  "library `Frob' (a library for tweaking knobs) written by James Random Hacker.\n</p>"
                                  "\n"
                                  "<p>&lt;signature of Ty Coon&gt;, 1 April 1990\n</p>"
                                  "<p>Ty Coon, President of Vice\n</p>"
                                  "\n"
                                  "<p>That's all there is to it!</p>"));

    msg.addLicense(QStringLiteral("KSyntaxHighlighting"),
                   QStringLiteral("<p><b>KSyntaxHighlighting License</b>\n\n</p>"
                                  "<p>MIT License Copyright (c) &lt;year&gt; &lt;copyright holders&gt;</p>\n"
                                  "<p>Permission is hereby granted, free of charge, to any person obtaining a copy "
                                  "of this software and associated documentation files (the \"Software\"), to deal "
                                  "in the Software without restriction, including without limitation the rights "
                                  "to use, copy, modify, merge, publish, distribute, sublicense, and/or sell "
                                  "copies of the Software, and to permit persons to whom the Software is furnished "
                                  "to do so, subject to the following conditions:</p>"
                                  "<p>The above copyright notice and this permission notice (including the next "
                                  "paragraph) shall be included in all copies or substantial portions of the "
                                  "Software.</p>"
                                  "<p>THE SOFTWARE IS PROVIDED \"AS IS\", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR "
                                  "IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS "
                                  "FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS "
                                  "OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, "
                                  "WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF "
                                  "OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.</p>"));

    msg.addLicense(QStringLiteral("resvg"),
                   QStringLiteral("<p><b>resvg License</b>\n\n</p>"
                                  "<p>Mozilla Public License Version 2.0</p>\n"
                                  "\n"
                                  "<h4>1. Definitions</h4>\n"
                                  "\n"
                                  "<h5>1.1. \"Contributor\"</h5>\n"
                                  "<p>means each individual or legal entity that creates, contributes to "
                                  "the creation of, or owns Covered Software.</p>\n"
                                  "\n"
                                  "<h5>1.2. \"Contributor Version\"</h5>\n"
                                  "<p>means the combination of the Contributions of others (if any) used "
                                  "by a Contributor and that particular Contributor's Contribution.</p>\n"
                                  "\n"
                                  "<h5>1.3. \"Contribution\"</h5>\n"
                                  "<p>means Covered Software of a particular Contributor.</p>\n"
                                  "\n"
                                  "<h5>1.4. \"Covered Software\"</h5>\n"
                                  "<p>means Source Code Form to which the initial Contributor has attached "
                                  "the notice in Exhibit A, the Executable Form of such Source Code "
                                  "Form, and Modifications of such Source Code Form, in each case "
                                  "including portions thereof.</p>\n"
                                  "\n"
                                  "<h5>1.5. \"Incompatible With Secondary Licenses\"</h5>\n"
                                  "<p>means</p>\n"
                                  "\n"
                                  "<p><b>(a)</b> that the initial Contributor has attached the notice described "
                                  "in Exhibit B to the Covered Software; or</p>\n"
                                  "\n"
                                  "<p><b>(b)</b> that the Covered Software was made available under the terms of "
                                  "version 1.1 or earlier of the License, but not also under the "
                                  "terms of a Secondary License.</p>\n"
                                  "\n"
                                  "<h5>1.6. \"Executable Form\"</h5>\n"
                                  "<p>means any form of the work other than Source Code Form.</p>\n"
                                  "\n"
                                  "<h5>1.7. \"Larger Work\"</h5>\n"
                                  "<p>means a work that combines Covered Software with other material, in "
                                  "a separate file or files, that is not Covered Software.</p>\n"
                                  "\n"
                                  "<h5>1.8. \"License\"</h5>\n"
                                  "<p>means this document.</p>\n"
                                  "\n"
                                  "<h5>1.9. \"Licensable\"</h5>\n"
                                  "<p>means having the right to grant, to the maximum extent possible, "
                                  "whether at the time of the initial grant or subsequently, any and "
                                  "all of the rights conveyed by this License.</p>\n"
                                  "\n"
                                  "<h5>1.10. \"Modifications\"</h5>\n"
                                  "<p>means any of the following:</p>\n"
                                  "\n"
                                  "<p><b>(a)</b> any file in Source Code Form that results from an addition to, "
                                  "deletion from, or modification of the contents of Covered "
                                  "Software; or</p>\n"
                                  "\n"
                                  "<p><b>(b)</b> any new file in Source Code Form that contains any Covered "
                                  "Software.</p>\n"
                                  "\n"
                                  "<h5>1.11. \"Patent Claims\" of a Contributor</h5>\n"
                                  "<p>means any patent claim(s), including without limitation, method, "
                                  "process, and apparatus claims, in any patent Licensable by such "
                                  "Contributor that would be infringed, but for the grant of the "
                                  "License, by the making, using, selling, offering for sale, having "
                                  "made, import, or transfer of either its Contributions or its "
                                  "Contributor Version.</p>\n"
                                  "\n"
                                  "<h5>1.12. \"Secondary License\"</h5>\n"
                                  "<p>means either the GNU General Public License, Version 2.0, the GNU "
                                  "Lesser General Public License, Version 2.1, the GNU Affero General "
                                  "Public License, Version 3.0, or any later versions of those "
                                  "licenses.</p>\n"
                                  "\n"
                                  "<h5>1.13. \"Source Code Form\"</h5>\n"
                                  "<p>means the form of the work preferred for making modifications.</p>\n"
                                  "\n"
                                  "<h5>1.14. \"You\" (or \"Your\")</h5>\n"
                                  "<p>means an individual or a legal entity exercising rights under this "
                                  "License. For legal entities, \"You\" includes any entity that "
                                  "controls, is controlled by, or is under common control with You. For "
                                  "purposes of this definition, \"control\" means <b>(a)</b> the power, direct "
                                  "or indirect, to cause the direction or management of such entity, "
                                  "whether by contract or otherwise, or <b>(b)</b> ownership of more than "
                                  "fifty percent (50%) of the outstanding shares or beneficial "
                                  "ownership of such entity.</p>\n"
                                  "\n"
                                  "<h4>2. License Grants and Conditions</h4>\n"
                                  "\n"
                                  "<h5>2.1. Grants</h5>\n"
                                  "\n"
                                  "<p>Each Contributor hereby grants You a world-wide, royalty-free, "
                                  "non-exclusive license:</p>\n"
                                  "\n"
                                  "<b>(a)</b> under intellectual property rights (other than patent or trademark) "
                                  "Licensable by such Contributor to use, reproduce, make available, "
                                  "modify, display, perform, distribute, and otherwise exploit its "
                                  "Contributions, either on an unmodified basis, with Modifications, or "
                                  "as part of a Larger Work; and</p>\n"
                                  "\n"
                                  "<p><b>(b)</b> under Patent Claims of such Contributor to make, use, sell, offer "
                                  "for sale, have made, import, and otherwise transfer either its "
                                  "Contributions or its Contributor Version.</p>\n"
                                  "\n"
                                  "<h5>2.2. Effective Date</h5>\n"
                                  "\n"
                                  "<p>The licenses granted in Section 2.1 with respect to any Contribution "
                                  "become effective for each Contribution on the date the Contributor first "
                                  "distributes such Contribution.</p>\n"
                                  "\n"
                                  "<h5>2.3. Limitations on Grant Scope</h5>\n"
                                  "\n"
                                  "<p>The licenses granted in this Section 2 are the only rights granted under "
                                  "this License. No additional rights or licenses will be implied from the "
                                  "distribution or licensing of Covered Software under this License. "
                                  "Notwithstanding Section 2.1(b) above, no patent license is granted by a "
                                  "Contributor:</p>\n"
                                  "\n"
                                  "<p><b>(a)</b> for any code that a Contributor has removed from Covered Software; "
                                  "or</p>\n"
                                  "\n"
                                  "<p><b>(b)</b> for infringements caused by: (i) Your and any other third party's "
                                  "modifications of Covered Software, or (ii) the combination of its "
                                  "Contributions with other software (except as part of its Contributor "
                                  "Version); or</p>\n"
                                  "\n"
                                  "<p><b>(c)</b> under Patent Claims infringed by Covered Software in the absence of "
                                  "its Contributions.</p>\n"
                                  "\n"
                                  "<p>This License does not grant any rights in the trademarks, service marks, "
                                  "or logos of any Contributor (except as may be necessary to comply with "
                                  "the notice requirements in Section 3.4).</p>\n"
                                  "\n"
                                  "<h5>2.4. Subsequent Licenses</h5>\n"
                                  "\n"
                                  "<p>No Contributor makes additional grants as a result of Your choice to "
                                  "distribute the Covered Software under a subsequent version of this "
                                  "License (see Section 10.2) or under the terms of a Secondary License (if "
                                  "permitted under the terms of Section 3.3).</p>\n"
                                  "\n"
                                  "<h5>2.5. Representation</h5>\n"
                                  "\n"
                                  "<p>Each Contributor represents that the Contributor believes its "
                                  "Contributions are its original creation(s) or it has sufficient rights "
                                  "to grant the rights to its Contributions conveyed by this License.</p>\n"
                                  "\n"
                                  "<h5>2.6. Fair Use</h5>\n"
                                  "\n"
                                  "<p>This License is not intended to limit any rights You have under "
                                  "applicable copyright doctrines of fair use, fair dealing, or other "
                                  "equivalents.</p>\n"
                                  "\n"
                                  "<h5>2.7. Conditions</h5>\n"
                                  "\n"
                                  "<p>Sections 3.1, 3.2, 3.3, and 3.4 are conditions of the licenses granted "
                                  "in Section 2.1.</p>\n"
                                  "\n"
                                  "<h4>3. Responsibilities</h4>\n"
                                  "\n"
                                  "<h5>3.1. Distribution of Source Form</h5>\n"
                                  "\n"
                                  "<p>All distribution of Covered Software in Source Code Form, including any "
                                  "Modifications that You create or to which You contribute, must be under "
                                  "the terms of this License. You must inform recipients that the Source "
                                  "Code Form of the Covered Software is governed by the terms of this "
                                  "License, and how they can obtain a copy of this License. You may not "
                                  "attempt to alter or restrict the recipients' rights in the Source Code "
                                  "Form.</p>\n"
                                  "\n"
                                  "<h5>3.2. Distribution of Executable Form</h5>\n"
                                  "\n"
                                  "<p>If You distribute Covered Software in Executable Form then:</p>\n"
                                  "\n"
                                  "<p><b>(a)</b> such Covered Software must also be made available in Source Code "
                                  "Form, as described in Section 3.1, and You must inform recipients of "
                                  "the Executable Form how they can obtain a copy of such Source Code "
                                  "Form by reasonable means in a timely manner, at a charge no more "
                                  "than the cost of distribution to the recipient; and</p>\n"
                                  "\n"
                                  "<p><b>(b)</b> You may distribute such Executable Form under the terms of this "
                                  "License, or sublicense it under different terms, provided that the "
                                  "license for the Executable Form does not attempt to limit or alter "
                                  "the recipients' rights in the Source Code Form under this License.</p>\n"
                                  "\n"
                                  "<h5>3.3. Distribution of a Larger Work</h5>\n"
                                  "\n"
                                  "<p>You may create and distribute a Larger Work under terms of Your choice, "
                                  "provided that You also comply with the requirements of this License for "
                                  "the Covered Software. If the Larger Work is a combination of Covered "
                                  "Software with a work governed by one or more Secondary Licenses, and the "
                                  "Covered Software is not Incompatible With Secondary Licenses, this "
                                  "License permits You to additionally distribute such Covered Software "
                                  "under the terms of such Secondary License(s), so that the recipient of "
                                  "the Larger Work may, at their option, further distribute the Covered "
                                  "Software under the terms of either this License or such Secondary "
                                  "License(s).</p>\n"
                                  "\n"
                                  "<h5>3.4. Notices</h5>\n"
                                  "\n"
                                  "<p>You may not remove or alter the substance of any license notices "
                                  "(including copyright notices, patent notices, disclaimers of warranty, "
                                  "or limitations of liability) contained within the Source Code Form of "
                                  "the Covered Software, except that You may alter any license notices to "
                                  "the extent required to remedy known factual inaccuracies.</p>\n"
                                  "\n"
                                  "<h5>3.5. Application of Additional Terms</h5>\n"
                                  "\n"
                                  "<p>You may choose to offer, and to charge a fee for, warranty, support, "
                                  "indemnity or liability obligations to one or more recipients of Covered "
                                  "Software. However, You may do so only on Your own behalf, and not on "
                                  "behalf of any Contributor. You must make it absolutely clear that any "
                                  "such warranty, support, indemnity, or liability obligation is offered by "
                                  "You alone, and You hereby agree to indemnify every Contributor for any "
                                  "liability incurred by such Contributor as a result of warranty, support, "
                                  "indemnity or liability terms You offer. You may include additional "
                                  "disclaimers of warranty and limitations of liability specific to any "
                                  "jurisdiction.</p>\n"
                                  "\n"
                                  "<h4>4. Inability to Comply Due to Statute or Regulation</h4>\n"
                                  "\n"
                                  "<p>If it is impossible for You to comply with any of the terms of this "
                                  "License with respect to some or all of the Covered Software due to "
                                  "statute, judicial order, or regulation then You must: <b>(a)</b> comply with "
                                  "the terms of this License to the maximum extent possible; and <b>(b)</b> "
                                  "describe the limitations and the code they affect. Such description must "
                                  "be placed in a text file included with all distributions of the Covered "
                                  "Software under this License. Except to the extent prohibited by statute "
                                  "or regulation, such description must be sufficiently detailed for a "
                                  "recipient of ordinary skill to be able to understand it.</p>\n"
                                  "\n"
                                  "<h4>5. Termination</h4>\n"
                                  "\n"
                                  "<h5>5.1.</h5> <p>The rights granted under this License will terminate automatically "
                                  "if You fail to comply with any of its terms. However, if You become "
                                  "compliant, then the rights granted under this License from a particular "
                                  "Contributor are reinstated <b>(a)</b> provisionally, unless and until such "
                                  "Contributor explicitly and finally terminates Your grants, and <b>(b)</b> on an "
                                  "ongoing basis, if such Contributor fails to notify You of the "
                                  "non-compliance by some reasonable means prior to 60 days after You have "
                                  "come back into compliance. Moreover, Your grants from a particular "
                                  "Contributor are reinstated on an ongoing basis if such Contributor "
                                  "notifies You of the non-compliance by some reasonable means, this is the "
                                  "first time You have received notice of non-compliance with this License "
                                  "from such Contributor, and You become compliant prior to 30 days after "
                                  "Your receipt of the notice.</p>\n"
                                  "\n"
                                  "<h5>5.2.</h5> <p>If You initiate litigation against any entity by asserting a patent "
                                  "infringement claim (excluding declaratory judgment actions, "
                                  "counter-claims, and cross-claims) alleging that a Contributor Version "
                                  "directly or indirectly infringes any patent, then the rights granted to "
                                  "You by any and all Contributors for the Covered Software under Section "
                                  "2.1 of this License shall terminate.</p>\n"
                                  "\n"
                                  "<h5>5.3.</h5> <p>In the event of termination under Sections 5.1 or 5.2 above, all "
                                  "end user license agreements (excluding distributors and resellers) which "
                                  "have been validly granted by You or Your distributors under this License "
                                  "prior to termination shall survive termination.</p>\n"
                                  "\n"
                                  "<h4>6. Disclaimer of Warranty</h4>\n"
                                  "\n"
                                  "<p>Covered Software is provided under this License on an \"as is\" "
                                  "basis, without warranty of any kind, either expressed, implied, or "
                                  "statutory, including, without limitation, warranties that the "
                                  "Covered Software is free of defects, merchantable, fit for a "
                                  "particular purpose or non-infringing. The entire risk as to the "
                                  "quality and performance of the Covered Software is with You. "
                                  "Should any Covered Software prove defective in any respect, You "
                                  "(not any Contributor) assume the cost of any necessary servicing, "
                                  "repair, or correction. This disclaimer of warranty constitutes an "
                                  "essential part of this License. No use of any Covered Software is "
                                  "authorized under this License except under this disclaimer.</p>\n"
                                  "\n"
                                  "<h4>7. Limitation of Liability</h4>\n"
                                  "\n"
                                  "<p>Under no circumstances and under no legal theory, whether tort "
                                  "(including negligence), contract, or otherwise, shall any "
                                  "Contributor, or anyone who distributes Covered Software as "
                                  "permitted above, be liable to You for any direct, indirect, "
                                  "special, incidental, or consequential damages of any character "
                                  "including, without limitation, damages for lost profits, loss of "
                                  "goodwill, work stoppage, computer failure or malfunction, or any "
                                  "and all other commercial damages or losses, even if such party "
                                  "shall have been informed of the possibility of such damages. This "
                                  "limitation of liability shall not apply to liability for death or "
                                  "personal injury resulting from such party's negligence to the "
                                  "extent applicable law prohibits such limitation. Some "
                                  "jurisdictions do not allow the exclusion or limitation of "
                                  "incidental or consequential damages, so this exclusion and "
                                  "limitation may not apply to You.</p>\n"
                                  "\n"
                                  "<h4>8. Litigation</h4>\n"
                                  "\n"
                                  "<p>Any litigation relating to this License may be brought only in the "
                                  "courts of a jurisdiction where the defendant maintains its principal "
                                  "place of business and such litigation shall be governed by laws of that "
                                  "jurisdiction, without reference to its conflict-of-law provisions. "
                                  "Nothing in this Section shall prevent a party's ability to bring "
                                  "cross-claims or counter-claims.</p>\n"
                                  "\n"
                                  "<h4>9. Miscellaneous</h4>\n"
                                  "\n"
                                  "<p>This License represents the complete agreement concerning the subject "
                                  "matter hereof. If any provision of this License is held to be "
                                  "unenforceable, such provision shall be reformed only to the extent "
                                  "necessary to make it enforceable. Any law or regulation which provides "
                                  "that the language of a contract shall be construed against the drafter "
                                  "shall not be used to construe this License against a Contributor.</p>\n"
                                  "\n"
                                  "<h4>10. Versions of the License</h4>\n"
                                  "\n"
                                  "<h5>10.1. New Versions</h5>\n"
                                  "\n"
                                  "<p>Mozilla Foundation is the license steward. Except as provided in Section "
                                  "10.3, no one other than the license steward has the right to modify or "
                                  "publish new versions of this License. Each version will be given a "
                                  "distinguishing version number.</p>\n"
                                  "\n"
                                  "<h5>10.2. Effect of New Versions</h5>\n"
                                  "\n"
                                  "<p>You may distribute the Covered Software under the terms of the version "
                                  "of the License under which You originally received the Covered Software, "
                                  "or under the terms of any subsequent version published by the license "
                                  "steward.</p>\n"
                                  "\n"
                                  "<h5>10.3. Modified Versions</h5>\n"
                                  "\n"
                                  "<p>If you create software not governed by this License, and you want to "
                                  "create a new license for such software, you may create and use a "
                                  "modified version of this License if you rename the license and remove "
                                  "any references to the name of the license steward (except to note that "
                                  "such modified license differs from this License).</p>\n"
                                  "\n"
                                  "<h5>10.4. Distributing Source Code Form that is Incompatible With Secondary "
                                  "Licenses</h5>\n"
                                  "\n"
                                  "<p>If You choose to distribute Source Code Form that is Incompatible With "
                                  "Secondary Licenses under the terms of this version of the License, the "
                                  "notice described in Exhibit B of this License must be attached.</p>\n"
                                  "\n"
                                  "<h4>Exhibit A - Source Code Form License Notice</h4>\n"
                                  "\n"
                                  "<p>This Source Code Form is subject to the terms of the Mozilla Public "
                                  "License, v. 2.0. If a copy of the MPL was not distributed with this "
                                  "file, You can obtain one at <a href=\"http://mozilla.org/MPL/2.0/\">http://mozilla.org/MPL/2.0/</a>.</p>\n"
                                  "\n"
                                  "<p>If it is not possible or desirable to put the notice in a particular "
                                  "file, then You may include the notice in a location (such as a LICENSE "
                                  "file in a relevant directory) where a recipient would be likely to look "
                                  "for such a notice.</p>\n"
                                  "\n"
                                  "<p>You may add additional accurate notices of copyright ownership.</p>\n"
                                  "\n"
                                  "<h4>Exhibit B - \"Incompatible With Secondary Licenses\" Notice</h4>\n"
                                  "\n"
                                  "<p>This Source Code Form is \"Incompatible With Secondary Licenses\", as "
                                  "defined by the Mozilla Public License, v. 2.0.</p>"));

    msg.addLicense(QStringLiteral("MicroTeX"),
                   QStringLiteral("<p><b>MicroTeX license</b></p>\n"
                                  "<p>The MIT License (MIT)<</p>\n"
                                  "\n"
                                  "<p>Copyright (c) 2020 Nano Michael</p>\n"
                                  "\n"
                                  "<p>Permission is hereby granted, free of charge, to any person obtaining a copy "
                                  "of this software and associated documentation files (the \"Software\"), to deal "
                                  "in the Software without restriction, including without limitation the rights "
                                  "to use, copy, modify, merge, publish, distribute, sublicense, and/or sell "
                                  "copies of the Software, and to permit persons to whom the Software is "
                                  "furnished to do so, subject to the following conditions:</p>\n"
                                  "\n"
                                  "<p>The above copyright notice and this permission notice shall be included in all "
                                  "copies or substantial portions of the Software.</p>\n"
                                  "\n"
                                  "<p>THE SOFTWARE IS PROVIDED \"AS IS\", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR "
                                  "IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, "
                                  "FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE "
                                  "AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER "
                                  "LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, "
                                  "OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE "
                                  "SOFTWARE.</p>"));

    msg.addLicense(QStringLiteral("md4qt"),
                   QStringLiteral("<p><b>md4qt</b>\n\n</p>"
                                  "<p>The MIT License (MIT)</p>\n"
                                  "\n"
                                  "<p>Copyright © 2022-2024 Igor Mironchik</p>\n"
                                  "\n"
                                  "<p>Permission is hereby granted, free of charge, to any person obtaining a copy of this "
                                  "software and associated documentation files (the \"Software\"), to deal in the Software "
                                  "without restriction, including without limitation the rights to use, copy, modify, merge, "
                                  "publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons "
                                  "to whom the Software is furnished to do so, subject to the following conditions:</p>\n"
                                  "\n"
                                  "<p>The above copyright notice and this permission notice shall be included in all copies or "
                                  "substantial portions of the Software.</p>\n"
                                  "\n"
                                  "<p>THE SOFTWARE IS PROVIDED \"AS IS\", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, "
                                  "INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR "
                                  "PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE "
                                  "FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, "
                                  "ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE "
                                  "SOFTWARE.</p>"));


    msg.exec();
}

void MainWindow::quit()
{
    QApplication::quit();
}

} /* namespace MdPdf */

/*
    SPDX-FileCopyrightText: 2026 Igor Mironchik <igor.mironchik@gmail.com>
    SPDX-License-Identifier: GPL-3.0-or-later
*/

// md4qt include.
#include <md4qt/src/parser.h>

// md-pdf include.
#include "const.h"
#include "main_window.h"
#include "progress.h"
#include "renderer.h"
#include "settings.h"
#include "version.h"

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
#include "license_dialog.h"
#include "utils.h"

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
    , m_syntax(new MdShared::Syntax)
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
    cfg.setValue(QStringLiteral("markColor"), m_markColor);
    cfg.setValue(QStringLiteral("borderColor"), m_ui->m_borderColor->color());
    cfg.setValue(QStringLiteral("codeTheme"), m_ui->m_codeTheme->currentText());
    cfg.setValue(QStringLiteral("dpi"), m_ui->m_dpi->value());
    cfg.setValue(QStringLiteral("imageAlignment"),
                 imageAlignmentToString(imageAlignmentFromInt(m_ui->m_imageAlignment->currentIndex())));

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

    cfg.beginGroup(QStringLiteral("plugins"));

    cfg.beginGroup(QStringLiteral("superscript"));
    cfg.setValue(QStringLiteral("delimiter"), m_pluginsCfg.m_sup.m_delimiter);
    cfg.setValue(QStringLiteral("enabled"), m_pluginsCfg.m_sup.m_on);
    cfg.endGroup();

    cfg.beginGroup(QStringLiteral("subscript"));
    cfg.setValue(QStringLiteral("delimiter"), m_pluginsCfg.m_sub.m_delimiter);
    cfg.setValue(QStringLiteral("enabled"), m_pluginsCfg.m_sub.m_on);
    cfg.endGroup();

    cfg.beginGroup(QStringLiteral("mark"));
    cfg.setValue(QStringLiteral("delimiter"), m_pluginsCfg.m_mark.m_delimiter);
    cfg.setValue(QStringLiteral("enabled"), m_pluginsCfg.m_mark.m_on);
    cfg.endGroup();

    cfg.setValue(QStringLiteral("yaml"), m_pluginsCfg.m_yamlEnabled);

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

    const auto markColor = cfg.value(QStringLiteral("markColor"), QColor(255, 255, 0, 75)).value<QColor>();
    if (markColor.isValid()) {
        m_markColor = markColor;
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

    cfg.beginGroup(QStringLiteral("plugins"));

    cfg.beginGroup(QStringLiteral("superscript"));
    m_pluginsCfg.m_sup.m_delimiter = cfg.value(QStringLiteral("delimiter"), QChar()).toChar();
    m_pluginsCfg.m_sup.m_on = cfg.value(QStringLiteral("enabled"), false).toBool();
    cfg.endGroup();

    cfg.beginGroup(QStringLiteral("subscript"));
    m_pluginsCfg.m_sub.m_delimiter = cfg.value(QStringLiteral("delimiter"), QChar()).toChar();
    m_pluginsCfg.m_sub.m_on = cfg.value(QStringLiteral("enabled"), false).toBool();
    cfg.endGroup();

    cfg.beginGroup(QStringLiteral("mark"));
    m_pluginsCfg.m_mark.m_delimiter = cfg.value(QStringLiteral("delimiter"), QChar()).toChar();
    m_pluginsCfg.m_mark.m_on = cfg.value(QStringLiteral("enabled"), false).toBool();
    cfg.endGroup();

    m_pluginsCfg.m_yamlEnabled = cfg.value(QStringLiteral("yaml"), false).toBool();

    cfg.endGroup();
}

const MdShared::PluginsCfg &MainWidget::pluginsCfg() const
{
    return m_pluginsCfg;
}

void MainWidget::setPluginsCfg(const MdShared::PluginsCfg &cfg)
{
    m_pluginsCfg = cfg;
}

const QColor &MainWidget::markColor() const
{
    return m_markColor;
}

void MainWidget::setMarkColor(const QColor &c)
{
    m_markColor = c;
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
    const auto folder =
        m_ui->m_fileName->text().isEmpty() ? QDir::homePath() : QFileInfo(m_ui->m_fileName->text()).absolutePath();

    const auto fileName =
        QFileDialog::getOpenFileName(this, tr("Select Markdown"), folder, tr("Markdown (*.md *.markdown)"));

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

        MD::Parser parser;
        setPlugins(parser, m_pluginsCfg);

        QSharedPointer<MD::Document> doc;

        if (m_ui->m_workingDirBox->isChecked()) {
            doc = parser.parse(m_ui->m_fileName->text(),
                               m_ui->m_workingDirectory->currentPath(),
                               m_ui->m_recursive->isChecked(),
                               QStringList() << QStringLiteral("md") << QStringLiteral("markdown"));
        } else {
            doc = parser.parse(m_ui->m_fileName->text(),
                               m_ui->m_recursive->isChecked(),
                               QStringList() << QStringLiteral("md") << QStringLiteral("markdown"));
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
            opts.m_markColor = m_markColor;
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
                QMessageBox::information(this,
                                         tr("Markdown processed..."),
                                         tr("PDF generated. Have a look at the result. Thank you."));
            } else {
                if (!progress.errorMsg().isEmpty()) {
                    QMessageBox::critical(this,
                                          tr("Error during rendering PDF..."),
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
    file->addAction(
        QIcon::fromTheme(QStringLiteral("application-exit"), QIcon(QStringLiteral(":/img/application-exit.png"))),
        tr("&Quit"),
        this,
        &MainWindow::quit);

    auto settings = menuBar()->addMenu(tr("&Settings"));
    settings->addAction(QIcon::fromTheme(QStringLiteral("configure"), QIcon(QStringLiteral(":/img/configure.png"))),
                        tr("Settings"),
                        this,
                        &MainWindow::settings);

    auto help = menuBar()->addMenu(tr("&Help"));
    help->addAction(QIcon(QStringLiteral(":/icon/icon_24x24.png")), tr("About"), this, &MainWindow::about);
    help->addAction(QIcon(QStringLiteral(":/img/Qt-logo-neon-transparent.png")),
                    tr("About Qt"),
                    this,
                    &MainWindow::aboutQt);
    help->addAction(
        QIcon::fromTheme(QStringLiteral("bookmarks-organize"), QIcon(QStringLiteral(":/img/bookmarks-organize.png"))),
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
    QMessageBox dlg(
        QMessageBox::Information,
        tr("About MD-PDF Converter"),
        tr("MD-PDF Converter.<br /><br />"
           "Version <a href=\"https://github.com/igormironchik/markdown-tools/commit/%3\">%1</a><br /><br />"
           "md4qt version %2<br /><br />"
           "Author - Igor Mironchik (<a href=\"mailto:igor.mironchik@gmail.com\">igor.mironchik at gmail dot "
           "com</a>).<br /><br />"
           "Copyright (c) 2026 Igor Mironchik.<br /><br />"
           "Licensed under GNU GPL 3.0.")
            .arg(c_version, c_md4qtVersion, c_commit),
        QMessageBox::NoButton,
        this);
    QIcon icon = dlg.windowIcon();
    dlg.setIconPixmap(icon.pixmap(QSize(64, 64), dlg.devicePixelRatio()));
    dlg.setTextFormat(Qt::RichText);

    dlg.exec();
}

void MainWindow::aboutQt()
{
    QMessageBox::aboutQt(this);
}

void MainWindow::licenses()
{
    MdShared::LicenseDialog msg(this);
    msg.addLicense(s_oxygenName, s_oxygenLicense);
    msg.addLicense(s_podofoName, s_podofoLicense);
    msg.addLicense(s_ksyntaxHighlightingName, s_ksyntaxHighlightingLicense);
    msg.addLicense(s_resvgName, s_resvgLicense);
    msg.addLicense(s_microtexName, s_microtexLicense);
    msg.addLicense(s_md4qtName, s_md4qtLicense);

    msg.exec();
}

void MainWindow::quit()
{
    QApplication::quit();
}

void MainWindow::settings()
{
    SettingsDlg dlg(ui->pluginsCfg(), ui->markColor(), this);

    if (dlg.exec() == QDialog::Accepted) {
        if (dlg.pluginsCfg() != ui->pluginsCfg()) {
            ui->setPluginsCfg(dlg.pluginsCfg());
        }

        if (dlg.markColor() != ui->markColor()) {
            ui->setMarkColor(dlg.markColor());
        }
    }
}

} /* namespace MdPdf */

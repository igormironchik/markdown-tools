/*
    SPDX-FileCopyrightText: 2026 Igor Mironchik <igor.mironchik@gmail.com>
    SPDX-License-Identifier: GPL-3.0-or-later
*/

// md-editor include.
#include "mainwindow.h"
#include "colorsdlg.h"
#include "const.h"
#include "find.h"
#include "findweb.h"
#include "fontdlg.h"
#include "gotoline.h"
#include "html.h"
#include "htmldocument.h"
#include "previewpage.h"
#include "settings.h"
#include "webview.h"
#include "widgets.h"

// Qt include.
#include <QDir>
#include <QFileDialog>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QMimeData>
#include <QMimeDatabase>
#include <QProcess>
#include <QScrollBar>
#include <QSettings>
#include <QSortFilterProxyModel>
#include <QSplitter>
#include <QStandardPaths>
#include <QStatusBar>
#include <QStyleFactory>
#include <QStyleHints>
#include <QTextDocument>
#include <QTextDocumentFragment>
#include <QTimer>
#include <QToolButton>
#include <QToolTip>
#include <QTreeWidgetItem>
#include <QWebEngineSettings>
#include <QWindow>
#include <QtConcurrent>

// Sonnet include.
#include <Sonnet/ConfigWidget>
#include <Sonnet/Settings>

// md4qt include.
#include <md4qt/src/algo.h>
#include <md4qt/src/parser.h>

// shared include.
#include "folder_chooser.h"
#include "license_dialog.h"
#include "md.h"
#include "plugins_page.h"
#include "utils.h"
#include "version.h"

// github-release include.
#include <github.h>

namespace MdEditor
{

//
// MainWindow
//

MainWindow::MainWindow()
    : m_d(new MainWindowPrivate(this))
{
    m_d->initUi();
}

MainWindow::~MainWindow()
{
    if (m_d->m_standardEditMenu) {
        m_d->m_standardEditMenu->deleteLater();
    }

    m_d->m_standardEditMenu = nullptr;
}

bool MainWindow::tryToNavigate(const QString &fileName)
{
    if (m_d->m_loadAllFlag) {
        if (m_d->m_fullFileNames.contains(fileName)) {
            openFileFromNavigationToolbar(fileName);

            return true;
        } else if (fileName.startsWith(QLatin1Char('#'))) {
            const auto idx = fileName.indexOf(QLatin1Char('/'), 0);

            if (idx != -1 && idx + 1 < fileName.size()) {
                m_d->m_requestedRef = fileName.sliced(0, idx);
                const auto fn = fileName.sliced(idx + 1, fileName.size() - idx - 1);

                if (m_d->m_fullFileNames.contains(fn)) {
                    openFileFromNavigationToolbar(fn);

                    m_d->runWhenEditorReady(std::bind(&MainWindow::onOpenRequestedRef, this));

                    return true;
                }
            }
        }
    }

    return false;
}

void MainWindow::onTabActivated()
{
    if (!m_d->m_tabsVisible || m_d->m_currentTab == m_d->m_tabs->currentIndex()) {
        showOrHideTabs();
    }

    m_d->handleCurrentTab();
}

void MainWindow::onWorkingDirectoryChange(const QString &)
{
    QDir::setCurrent(m_d->m_workingDirectoryWidget->workingDirectory());

    m_d->m_baseUrl =
        QString("file:%1/")
            .arg(QString(QUrl::toPercentEncoding(m_d->m_workingDirectoryWidget->workingDirectory(), "/\\:", {})));
    m_d->m_page->setHtml(htmlContent(), m_d->m_baseUrl);
    m_d->m_editor->onWorkingDirectoryChange(m_d->m_workingDirectoryWidget->workingDirectory(),
                                            !m_d->m_workingDirectoryWidget->isRelative());

    m_d->runWhenEditorReady(std::bind(&MainWindow::onEditorReady, this));
}

void MainWindow::onScrollWebViewTo(const QString &id)
{
    m_d->m_page->runJavaScript(QStringLiteral("scrollToId('%1')").arg(id));
}

void MainWindow::onConvertToPdf()
{
    QDir workingDir(QApplication::applicationDirPath());
    const auto mdPdfExeFiles = workingDir.entryInfoList({m_d->m_mdPdfExe}, QDir::Executable | QDir::Files);
    const auto starterExeFiles = workingDir.entryInfoList({m_d->m_launcherExe}, QDir::Executable | QDir::Files);

    if (!mdPdfExeFiles.isEmpty() && !starterExeFiles.isEmpty()) {
        QProcess::startDetached(starterExeFiles.at(0).absoluteFilePath(),
                                {QStringLiteral("--exe"),
                                 mdPdfExeFiles.at(0).fileName(),
                                 QStringLiteral("--mode"),
                                 QStringLiteral("detached"),
                                 QStringLiteral("--arg"),
                                 m_d->m_rootFilePath},
                                workingDir.absolutePath());
    }
}

void MainWindow::resizeEvent(QResizeEvent *e)
{
    if (!m_d->m_sizesInitialized) {
        m_d->m_sizesInitialized = true;

        QStyleOptionTab opt;
        opt.initFrom(m_d->m_tabs);

        m_d->m_minTabWidth = opt.rect.height();
        m_d->m_sidebarPanel->setFixedWidth(m_d->m_minTabWidth);
        m_d->m_sidebarPanel->setMinimumWidth(m_d->m_minTabWidth);

        auto w = (centralWidget()->width() - m_d->m_minTabWidth);

        m_d->m_tabEditorSplitter->setSizes({m_d->m_minTabWidth, w});
        m_d->m_editorPreviewSplitter->setSizes({w / 2, w / 2});
    }

    e->accept();
}

void MainWindow::showEvent(QShowEvent *e)
{
    if (!m_d->m_shownAlready) {
        m_d->m_shownAlready = true;

        QTimer::singleShot(0, this, &MainWindow::onFirstTimeShown);
    }

    e->accept();
}

QPair<QSharedPointer<QFile>,
      bool>
MainWindow::getFile(const QString &path)
{
    QString fileName = path;
    bool autosavedLoaded = false;

    if (QFileInfo::exists(path + s_autosaveExtension)) {
        const auto ret = QMessageBox::question(
            this,
            windowTitle(),
            tr("Would you like to open auto saved content of this file?\n\nFile: \"%1\"").arg(path));

        if (ret == QMessageBox::Yes) {
            autosavedLoaded = true;
            fileName.append(s_autosaveExtension);
        } else {
            QFile::remove(path + s_autosaveExtension);
        }
    }

    QSharedPointer<QFile> f(new QFile(fileName));
    if (!f->open(QIODevice::ReadOnly)) {
        QMessageBox::warning(this,
                             windowTitle(),
                             tr("Could not open file %1: %2").arg(QDir::toNativeSeparators(path), f->errorString()));
        return {};
    }

    return {f, autosavedLoaded};
}

void MainWindow::openFile(const QString &path)
{
    if (m_d->m_loadAllFlag) {
        loadAllLinkedFilesImpl();
    }

    QSharedPointer<QFile> f;
    bool autosavedLoaded = false;

    std::tie(f, autosavedLoaded) = getFile(path);

    if (!f) {
        return;
    }

    m_d->m_preview->stop();
    m_d->m_isDefaultFile = false;
    m_d->m_editor->enableAutoSave(false);
    m_d->m_editor->setDocName(path);
    const auto wd = QFileInfo(path).absoluteDir().absolutePath();
    m_d->m_workingDirectoryWidget->setWorkingDirectory(wd);
    onWorkingDirectoryChange(wd);

    m_d->m_editor->setText(f->readAll());
    m_d->m_editor->enableAutoSave(true);
    f->close();
    updateWindowTitle();
    m_d->m_editor->setFocus();
    m_d->m_editor->document()->clearUndoRedoStacks();
    m_d->m_editor->document()->setModified(autosavedLoaded);
    onCursorPositionChanged();
    m_d->m_loadAllAction->setEnabled(true);
    m_d->m_rootFilePath = path;

    if (m_d->m_convertToPdfAction) {
        m_d->m_convertToPdfAction->setEnabled(true);
    }

    closeAllLinkedFiles();
    updateLoadAllLinkedFilesMenuText();
    m_d->initMarkdownMenu();

    setWindowModified(autosavedLoaded);
}

void MainWindow::openInPreviewMode()
{
    m_d->m_viewAction->setChecked(true);
}

bool MainWindow::isModified() const
{
    return m_d->m_editor->document()->isModified() || isWindowModified();
}

void MainWindow::onFileNew()
{
    if (isModified()) {
        QMessageBox::StandardButton button =
            QMessageBox::question(this,
                                  windowTitle(),
                                  tr("You have unsaved changes. Do you want to create a new document anyway?"),
                                  QMessageBox::Yes | QMessageBox::No,
                                  QMessageBox::No);

        if (button != QMessageBox::Yes) {
            return;
        } else {
            m_d->m_editor->fileWasSaved();
        }
    }

    m_d->m_preview->stop();
    m_d->m_isDefaultFile = true;
    m_d->m_editor->setDocName(s_defaultFileName);
    m_d->m_editor->setText({});
    m_d->m_editor->document()->setModified(false);
    m_d->m_editor->document()->clearUndoRedoStacks();
    updateWindowTitle();

    const auto wd = QStandardPaths::standardLocations(QStandardPaths::HomeLocation).first();
    m_d->m_workingDirectoryWidget->setWorkingDirectory(wd);
    onWorkingDirectoryChange(wd);
    onCursorPositionChanged();
    m_d->m_loadAllAction->setEnabled(false);
    m_d->m_rootFilePath.clear();

    if (m_d->m_convertToPdfAction) {
        m_d->m_convertToPdfAction->setEnabled(false);
    }

    closeAllLinkedFiles();
    updateLoadAllLinkedFilesMenuText();
    m_d->initMarkdownMenu();

    setWindowModified(false);
}

bool MainWindow::askAboutUnsavedFile()
{
    if (isModified()) {
        QMessageBox::StandardButton button =
            QMessageBox::question(this,
                                  windowTitle(),
                                  tr("You have unsaved changes. Do you want to open a new document anyway?"),
                                  QMessageBox::Yes | QMessageBox::No,
                                  QMessageBox::No);

        if (button != QMessageBox::Yes) {
            return false;
        } else {
            m_d->m_editor->fileWasSaved();
        }
    }

    return true;
}

void MainWindow::onFileOpen()
{
    if (!askAboutUnsavedFile()) {
        return;
    }

    const auto folder = m_d->m_isDefaultFile ? QStandardPaths::standardLocations(QStandardPaths::HomeLocation).first()
                                             : QFileInfo(m_d->m_editor->docName()).absolutePath();

    QFileDialog dialog(this, tr("Open Markdown File"), folder);
    dialog.setMimeTypeFilters({"text/markdown"});
    dialog.setAcceptMode(QFileDialog::AcceptOpen);

    if (dialog.exec() == QDialog::Accepted) {
        openFile(dialog.selectedFiles().constFirst());
    }
}

void MainWindow::onFileSave()
{
    if (m_d->m_isDefaultFile) {
        onFileSaveAs();
        return;
    }

    QFile f(m_d->m_editor->docName());
    if (!f.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::warning(this,
                             windowTitle(),
                             tr("Could not write to file %1: %2")
                                 .arg(QDir::toNativeSeparators(m_d->m_editor->docName()), f.errorString()));

        return;
    }

    QTextStream str(&f);
    str << m_d->m_editor->toPlainText();
    f.close();

    m_d->m_editor->document()->setModified(false);
    m_d->m_editor->fileWasSaved();
    m_d->m_editor->clearAutoListStateOnAllBlocks();

    updateWindowTitle();

    m_d->runWhenEditorReady(std::bind(&MainWindow::onEditorReady, this));
}

void MainWindow::onFileSaveAs()
{
    QFileDialog dialog(this,
                       tr("Save Markdown File"),
                       QStandardPaths::standardLocations(QStandardPaths::HomeLocation).first());
    dialog.setMimeTypeFilters({"text/markdown"});
    dialog.setAcceptMode(QFileDialog::AcceptSave);
    dialog.setDefaultSuffix("md");

    if (dialog.exec() != QDialog::Accepted) {
        return;
    }

    m_d->m_editor->fileWasSaved();
    m_d->m_isDefaultFile = false;
    m_d->m_editor->setDocName(dialog.selectedFiles().constFirst());
    const auto wd = QFileInfo(m_d->m_editor->docName()).absoluteDir().absolutePath();
    m_d->m_workingDirectoryWidget->setWorkingDirectory(wd);
    m_d->m_rootFilePath = m_d->m_editor->docName();

    if (m_d->m_convertToPdfAction) {
        m_d->m_convertToPdfAction->setEnabled(true);
    }

    onFileSave();

    onWorkingDirectoryChange(wd);

    closeAllLinkedFiles();

    updateLoadAllLinkedFilesMenuText();

    m_d->m_editor->fileWasSaved();
}

void MainWindow::closeEvent(QCloseEvent *e)
{
    auto stopPreview = true;

    if (isModified()) {
        QMessageBox::StandardButton button =
            QMessageBox::question(this,
                                  windowTitle(),
                                  tr("You have unsaved changes. Do you want to exit anyway?"),
                                  QMessageBox::Yes | QMessageBox::No,
                                  QMessageBox::No);

        if (button != QMessageBox::Yes) {
            e->ignore();
            stopPreview = false;
        } else {
            m_d->m_editor->fileWasSaved();
        }
    }

    saveCfg();

    if (stopPreview) {
        m_d->m_preview->stop();
    }
}

void refreshStyleRecursively(QWidget *widget,
                             const QPalette &p)
{
    if (!widget) {
        return;
    }

    widget->setPalette(p);
    widget->style()->unpolish(widget);
    widget->style()->polish(widget);

    for (QObject *child : std::as_const(widget->children())) {
        if (QWidget *childWidget = qobject_cast<QWidget *>(child)) {
            refreshStyleRecursively(childWidget, p);
        }
    }

    widget->repaint();
}

void MainWindow::updateStyle(bool updateHtml,
                             bool switchToDefaultColors)
{
    qApp->setStyle(QStyleFactory::create(qApp->style()->objectName()));

    refreshStyleRecursively(this, qApp->palette());

    const auto isDark = (qApp->styleHints()->colorScheme() == Qt::ColorScheme::Dark);

    m_d->m_preview->settings()->setAttribute(QWebEngineSettings::WebAttribute::ForceDarkMode, isDark);

    if (updateHtml) {
        m_d->m_page->runJavaScript(QStringLiteral("changeCodeTheme('%1')")
                                       .arg(isDark ? QStringLiteral("github-dark") : QStringLiteral("github")));
    }

    if (switchToDefaultColors) {
        if (m_d->m_editor->settings().m_colors == Colors(true) || m_d->m_editor->settings().m_colors == Colors(false)) {
            m_d->m_editor->applyColors(Colors(isDark));

            m_d->m_editor->doUpdate();
        }
    }
}

bool MainWindow::event(QEvent *event)
{
    switch (event->type()) {
    case QEvent::ShortcutOverride: {
        if (static_cast<QKeyEvent *>(event)->key() == Qt::Key_Escape) {
            event->accept();

            if (m_d->m_findWeb->isVisible()) {
                m_d->m_findWeb->hide();
            } else if (m_d->m_gotoline->isVisible()) {
                m_d->m_gotoline->hide();
            } else if (m_d->m_find->isVisible()) {
                m_d->m_find->hide();
            }

            onToolHide();

            return true;
        }
    } break;

    case QEvent::ApplicationPaletteChange: {
        updateStyle(true, true);
    } break;

    default:
        break;
    }

    return QMainWindow::event(event);
}

void MainWindow::onToolHide()
{
    if (!m_d->m_find->isVisible() && !m_d->m_gotoline->isVisible()) {
        m_d->m_editor->setFocus();
        m_d->m_editor->clearHighlighting();
    } else if (m_d->m_find->isVisible() && !m_d->m_gotoline->isVisible()) {
        m_d->m_find->setFocusOnFind();
    } else if (m_d->m_gotoline->isVisible() && !m_d->m_find->isVisible()) {
        m_d->m_gotoline->setFocusOnLine();
        m_d->m_editor->clearHighlighting();
    }
}

const QString &MainWindow::htmlContent() const
{
    return m_d->m_htmlContent;
}

void MainWindow::onTextChanged()
{
    if (!m_d->m_loadAllFlag) {
        m_d->m_mdDoc = m_d->m_editor->currentDoc();

        if (m_d->m_livePreviewVisible && m_d->m_mdDoc) {
            m_d->m_html->setText(MD::toHtml<HtmlConv>(m_d->m_mdDoc,
                                                      false,
                                                      QStringLiteral("<img src=\"qrc:/res/img/go-jump.png\" />"),
                                                      true,
                                                      &m_d->m_editor->idsMap()));

            if (m_d->m_editor->settings().m_previewFollowEditor) {
                scrollToCursor();
            }
        }
    }

    m_d->initMarkdownMenu();
}

void MainWindow::onAbout()
{
    QMessageBox dlg(
        QMessageBox::Information,
        tr("About Markdown Editor"),
        tr("Markdown Editor.<br /><br />"
           "Version <a href=\"https://github.com/igormironchik/markdown-tools/commit/%3\">%1</a><br /><br />"
           "md4qt version %2<br /><br />"
           "Author - Igor Mironchik (<a href=\"mailto:igor.mironchik@gmail.com\">igor.mironchik at gmail dot "
           "com</a>).<br /><br />"
           "Copyright (c) 2026 Igor Mironchik.<br /><br />"
           "Licensed under GNU GPL 3.0.")
            .arg(MdShared::c_version, MdShared::c_md4qtVersion, MdShared::c_commit),
        QMessageBox::NoButton,
        this);
    QIcon icon = dlg.windowIcon();
    dlg.setIconPixmap(icon.pixmap(QSize(64, 64), dlg.devicePixelRatio()));
    dlg.setTextFormat(Qt::RichText);

    dlg.exec();
}

void MainWindow::onAboutQt()
{
    QMessageBox::aboutQt(this);
}

namespace /* anonymous */
{

inline QString itemType(MD::Item *item,
                        bool alone)
{
    switch (item->type()) {
    case MD::ItemType::Heading:
        return MainWindow::tr("Heading");
    case MD::ItemType::Text:
        return MainWindow::tr("Text");
    case MD::ItemType::Paragraph: {
        auto p = static_cast<MD::Paragraph *>(item);

        if (p->items().size() == 1
            && p->items().at(0)->type() == MD::ItemType::Math
            && !static_cast<MD::Math *>(p->items().at(0).get())->isInline()) {
            return MainWindow::tr("LaTeX Math Expression");
        } else {
            return MainWindow::tr("Paragraph");
        }
    }
    case MD::ItemType::LineBreak:
        return MainWindow::tr("Line Break");
    case MD::ItemType::Blockquote:
        return MainWindow::tr("Blockquote");
    case MD::ItemType::ListItem:
        return MainWindow::tr("List Item");
    case MD::ItemType::List:
        return MainWindow::tr("List");
    case MD::ItemType::Link: {
        if (alone) {
            return MainWindow::tr("Reference Link");
        } else {
            return MainWindow::tr("Link");
        }
    }
    case MD::ItemType::Image:
        return MainWindow::tr("Image");
    case MD::ItemType::Code:
        return MainWindow::tr("Code");
    case MD::ItemType::TableCell:
        return MainWindow::tr("Table Cell");
    case MD::ItemType::TableRow:
        return MainWindow::tr("Table Row");
    case MD::ItemType::Table:
        return MainWindow::tr("Table");
    case MD::ItemType::FootnoteRef:
        return MainWindow::tr("Footnote Reference");
    case MD::ItemType::Footnote:
        return MainWindow::tr("Footnote");
    case MD::ItemType::Document:
        return MainWindow::tr("Document");
    case MD::ItemType::PageBreak:
        return MainWindow::tr("Page Break");
    case MD::ItemType::Anchor:
        return MainWindow::tr("Anchor");
    case MD::ItemType::HorizontalLine:
        return MainWindow::tr("Horizontal Line");
    case MD::ItemType::RawHtml:
        return MainWindow::tr("Raw HTML");
    case MD::ItemType::Math:
        return MainWindow::tr("LaTeX Math Expression");

    case static_cast<MD::ItemType>(static_cast<int>(MD::ItemType::UserDefined) + 1):
        return MainWindow::tr("YAML Header");

    default:
        return {};
    }
}

} /* namespace anonymous */

void MainWindow::onLineHovered(int lineNumber,
                               const QPoint &pos)
{
    const auto items = m_d->m_editor->syntaxHighlighter().findFirstInCache(
        {0, lineNumber, m_d->m_editor->document()->findBlockByLineNumber(lineNumber).length(), lineNumber});

    if (!items.empty()) {
        if ((items.front()->type() != MD::ItemType::List && items.front()->type() != MD::ItemType::Footnote)
            || items.size() == 1) {
            QToolTip::showText(pos, itemType(items.front(), items.size() == 1));
        } else {
            QToolTip::showText(
                pos,
                tr("%1 in %2")
                    .arg(itemType(items.at(1), items.size() == 1), itemType(items.front(), items.size() == 1)));
        }
    }
}

void MainWindow::scrollPreview(const QString &id,
                               qsizetype count,
                               bool code,
                               qsizetype fromTop)
{
    if (code) {
        if (fromTop == 0) {
            m_d->m_page->runJavaScript(
                QStringLiteral("scrollToLineInCode('%1', '%2')").arg(id, QString::number(count)));
        } else {
            m_d->m_page->runJavaScript(QStringLiteral("scrollToLineInCodeFromTopElem('%1', '%2', '%3')")
                                           .arg(id, QString::number(count), QString::number(fromTop)));
        }
    } else {
        if (count > 0) {
            onScrollWebViewTo(QStringLiteral("%1-%2").arg(id, QString::number(count)));
        } else {
            onScrollWebViewTo(id);
        }
    }
}

void MainWindow::onEditorScrolled(int)
{
    const auto centerY = m_d->m_editor->viewport()->height() / 2;
    const QPoint centerPoint(m_d->m_editor->viewport()->width() / 2, centerY);

    const auto c = m_d->m_editor->cursorForPosition(centerPoint);

    scrollPreview(
        c.blockNumber(),
        [this](const QString &id, qsizetype count, bool code, const QPoint &, qsizetype fromTop) {
            this->scrollPreview(id, count, code, fromTop);
        },
        QPoint());
}

void MainWindow::scrollToCursor()
{
    scrollPreview(
        m_d->m_editor->textCursor().blockNumber(),
        [this](const QString &id, qsizetype count, bool code, const QPoint &, qsizetype fromTop) {
            this->scrollPreview(id, count, code, fromTop);
        },
        QPoint());
}

void MainWindow::onPinPreviewEditor(bool checked)
{
    if (checked) {
        m_d->m_pinPreviewEditor->setIcon(
            QIcon::fromTheme(QStringLiteral("window-unpin"), QIcon(QStringLiteral(":/res/img/window-unpin.png"))));
        m_d->m_pinPreviewEditor->setToolTip(MainWindow::tr("Unpin Web preview scrolling to editor"));
        connect(m_d->m_editor->verticalScrollBar(), &QScrollBar::valueChanged, this, &MainWindow::onEditorScrolled);
    } else {
        m_d->m_pinPreviewEditor->setIcon(
            QIcon::fromTheme(QStringLiteral("window-pin"), QIcon(QStringLiteral(":/res/img/window-pin.png"))));
        m_d->m_pinPreviewEditor->setToolTip(MainWindow::tr("Pin Web preview scrolling to editor"));
        disconnect(m_d->m_editor->verticalScrollBar(), &QScrollBar::valueChanged, this, &MainWindow::onEditorScrolled);
    }

    m_d->m_editor->setPreviewFollowEditor(checked);
}

void MainWindow::showMarkdownStandard(QTextCursor c)
{
    auto b = c.block();

    const auto items = m_d->m_editor->syntaxHighlighter().findFirstInCache(
        {c.positionInBlock(), b.blockNumber(), c.positionInBlock(), b.blockNumber()});

    bool inlineSpan = false;

    auto handleEmphasis = [](MD::ItemWithOpts *i, MdShared::LicenseDialog &dlg) {
        if (i->opts() & MD::TextOption::BoldText || i->opts() & MD::TextOption::ItalicText) {
            dlg.addLicense(tr("Emphasis and strong emphasis"),
                           QApplication::translate("Markdown", MdSyntax::s_emphasisAndStrongEmphasis));
        }

        if (i->opts() & MD::TextOption::StrikethroughText) {
            dlg.addLicense(tr("Strikethrough"), QApplication::translate("Markdown", MdSyntax::s_strikethrough));
        }
    };

    MdShared::LicenseDialog dlg(this);
    dlg.setWindowTitle(tr("Extract from the Markdown Standard"));

    if (!items.empty()) {
        for (const auto &i : items) {
            switch (i->type()) {
            case MD::ItemType::Heading: {
                auto h = static_cast<MD::Heading *>(i);

                b = m_d->m_editor->document()->findBlockByNumber(h->delims().front().startLine());
                c = QTextCursor(b);
                c.setPosition(c.position() + h->delims().front().startColumn());
                c.setPosition(c.position() + 1, QTextCursor::KeepAnchor);

                if (c.selectedText() == QStringLiteral("#")) {
                    dlg.addLicense(tr("ATX headings"), QApplication::translate("Markdown", MdSyntax::s_atxHeadings));
                } else {
                    dlg.addLicense(tr("Setext headings"),
                                   QApplication::translate("Markdown", MdSyntax::s_setextHeadings));
                }
            } break;

            case MD::ItemType::Text: {
                auto t = static_cast<MD::Text *>(i);

                handleEmphasis(t, dlg);
            } break;

            case MD::ItemType::Paragraph: {
                dlg.addLicense(tr("Paragraphs"), QApplication::translate("Markdown", MdSyntax::s_paragraphs));
                inlineSpan = true;
            } break;

            case MD::ItemType::LineBreak: {
                dlg.addLicense(tr("Hard line breaks"), QApplication::translate("Markdown", MdSyntax::s_hardLineBreaks));
            } break;

            case MD::ItemType::Blockquote: {
                dlg.addLicense(tr("Block quotes"), QApplication::translate("Markdown", MdSyntax::s_blockQuotes));
            } break;

            case MD::ItemType::ListItem: {
                auto li = static_cast<MD::ListItem *>(i);

                dlg.addLicense(tr("List items"), QApplication::translate("Markdown", MdSyntax::s_listItems));

                if (li->isTaskList()) {
                    dlg.addLicense(tr("Task list items"),
                                   QApplication::translate("Markdown", MdSyntax::s_taskListItems));
                }
            } break;

            case MD::ItemType::List: {
                dlg.addLicense(tr("Lists"), QApplication::translate("Markdown", MdSyntax::s_lists));
            } break;

            case MD::ItemType::Link: {
                dlg.addLicense(tr("Links"), QApplication::translate("Markdown", MdSyntax::s_links));
                dlg.addLicense(tr("Reference links"),
                               QApplication::translate("Markdown", MdSyntax::s_linkReferenceDefinitions));
                dlg.addLicense(tr("Autolinks"), QApplication::translate("Markdown", MdSyntax::s_autolinks));

                auto l = static_cast<MD::Link *>(i);

                handleEmphasis(l, dlg);
            } break;

            case MD::ItemType::Image: {
                dlg.addLicense(tr("Links"), QApplication::translate("Markdown", MdSyntax::s_links));
                dlg.addLicense(tr("Reference links"),
                               QApplication::translate("Markdown", MdSyntax::s_linkReferenceDefinitions));
                dlg.addLicense(tr("Autolinks"), QApplication::translate("Markdown", MdSyntax::s_autolinks));
                dlg.addLicense(tr("Images"), QApplication::translate("Markdown", MdSyntax::s_images));

                auto img = static_cast<MD::Image *>(i);

                handleEmphasis(img, dlg);
            } break;

            case MD::ItemType::Code: {
                auto c = static_cast<MD::Code *>(i);

                if (c->isInline()) {
                    dlg.addLicense(tr("Code spans"), QApplication::translate("Markdown", MdSyntax::s_codeSpans));

                    handleEmphasis(c, dlg);
                } else {
                    if (c->isFensedCode()) {
                        dlg.addLicense(tr("Fenced code blocks"),
                                       QApplication::translate("Markdown", MdSyntax::s_fencedCodeBlocks));
                    } else {
                        dlg.addLicense(tr("Indented code blocks"),
                                       QApplication::translate("Markdown", MdSyntax::s_indentedCodeBlocks));
                    }
                }
            } break;

            case MD::ItemType::FootnoteRef:
            case MD::ItemType::Footnote:
                break;

            case MD::ItemType::HorizontalLine: {
                dlg.addLicense(tr("Thematic breaks"), QApplication::translate("Markdown", MdSyntax::s_thematicBreaks));
            } break;

            case MD::ItemType::RawHtml: {
                if (inlineSpan) {
                    dlg.addLicense(tr("Raw HTML"), QApplication::translate("Markdown", MdSyntax::s_rawHtml));

                    auto h = static_cast<MD::RawHtml *>(i);

                    handleEmphasis(h, dlg);
                } else {
                    dlg.addLicense(tr("HTML blocks"), QApplication::translate("Markdown", MdSyntax::s_htmlBlocks));
                }
            } break;

            case MD::ItemType::Math:
                break;

            case MD::ItemType::Table: {
                dlg.addLicense(tr("Tables"), QApplication::translate("Markdown", MdSyntax::s_tables));
            } break;

            default:
                break;
            }
        }
    } else {
        dlg.addLicense(tr("Blank lines"), QApplication::translate("Markdown", MdSyntax::s_blankLines));
    }

    dlg.exec();
}

void MainWindow::onMarkdownStandardHelp()
{
    showMarkdownStandard(m_d->m_editor->textCursor());
}

void checkForUpdates(QPromise<Update> &promise,
                     const QString &currentVersion)
{
    Update info;
    info.m_available = GHRelease::isUpdateAvailable(QStringLiteral("igormironchik"),
                                                    QStringLiteral("markdown-tools"),
                                                    currentVersion,
                                                    GHRelease::majorMinorPatchCompare,
                                                    info.m_url,
                                                    info.m_tag);

    promise.addResult(info);
}

void MainWindow::onCheckForUpdates()
{
    if (QDateTime::currentDateTime() - m_d->m_lastCheckForUpdates
            > std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::hours(1))
        && !GHRelease::majorMinorPatchCompare(MdShared::c_versionNumbers, m_d->m_updatesAbailable)) {
        auto future = QtConcurrent::run(checkForUpdates, MdShared::c_versionNumbers);
        m_d->m_updateWatcher->setFuture(future);
    } else {
        onAddUpdatesButton();
    }
}

void MainWindow::onCheckForUpdatesFinished()
{
    const auto update = m_d->m_updateWatcher->result();
    m_d->m_lastCheckForUpdates = QDateTime::currentDateTime();

    if (update.m_available) {
        m_d->m_updatesAbailable = update.m_tag;
        m_d->m_updatesUrl = update.m_url;
    } else {
        m_d->m_updatesAbailable.clear();
        m_d->m_updatesUrl.clear();
    }

    saveCfg();

    onAddUpdatesButton();
}

void MainWindow::onAddUpdatesButton()
{
    if (!m_d->m_updatesAbailable.isEmpty()
        && !m_d->m_updatesUrl.isEmpty()
        && GHRelease::majorMinorPatchCompare(MdShared::c_versionNumbers, m_d->m_updatesAbailable)) {
        auto btn = new GHRelease::NewVersionAvailableButton(m_d->m_updatesUrl,
                                                            GHRelease::NewVersionAvailableButton::OpenUrlOnClick,
                                                            statusBar());
        btn->setMinimumHeight(m_d->m_workingDirectoryWidget->labelHeight());
        btn->setMaximumHeight(m_d->m_workingDirectoryWidget->labelHeight());
        statusBar()->addPermanentWidget(m_d->makeSeparator());
        statusBar()->addPermanentWidget(btn);
    }
}

void MainWindow::onLineNumberContextMenuRequested(int lineNumber,
                                                  const QPoint &pos)
{
    scrollPreview(
        lineNumber,
        [this](const QString &id, qsizetype count, bool code, const QPoint &pos, qsizetype fromTop) {
            QMenu menu;

            menu.addAction(tr("Scroll Web Preview To"), [id, this, count, code, fromTop]() {
                this->scrollPreview(id, count, code, fromTop);
            });

            menu.exec(pos);
        },
        pos);
}

void MainWindow::onFind(bool)
{
    if (!m_d->m_find->isVisible()) {
        m_d->m_find->show();
    }

    if (!m_d->m_editor->textCursor().selection().isEmpty()) {
        m_d->m_find->setFindText(m_d->m_editor->textCursor().selection().toPlainText());
    } else {
        m_d->m_find->setFocusOnFind();
    }
}

void MainWindow::onFindWeb(bool)
{
    if (!m_d->m_findWeb->isVisible()) {
        m_d->m_findWeb->show();
    }

    m_d->m_findWeb->setFocusOnFindWeb();
}

void MainWindow::onGoToLine(bool)
{
    if (!m_d->m_gotoline->isVisible()) {
        m_d->m_gotoline->show();
    }

    m_d->m_gotoline->setFocusOnLine();
}

void MainWindow::onNextMisspelled()
{
    m_d->m_editor->onNextMisspelled();
}

void MainWindow::onMisspelledFount(bool found)
{
    m_d->m_nextMisspelled->setEnabled(found);
}

void MainWindow::onChooseFont()
{
    FontDlg dlg(m_d->m_editor->font(), this);

    if (dlg.exec() == QDialog::Accepted) {
        m_d->m_editor->applyFont(dlg.currentFont());

        m_d->m_editor->doUpdate();

        saveCfg();
    }
}

static const QString s_ui = QStringLiteral("ui");
static const QString s_font = QStringLiteral("font");
static const QString s_family = QStringLiteral("family");
static const QString s_size = QStringLiteral("size");
static const QString s_linkColor = QStringLiteral("linkColor");
static const QString s_textColor = QStringLiteral("textColor");
static const QString s_inlineColor = QStringLiteral("inlineColor");
static const QString s_htmlColor = QStringLiteral("htmlColor");
static const QString s_tableColor = QStringLiteral("tableColor");
static const QString s_codeColor = QStringLiteral("codeColor");
static const QString s_mathColor = QStringLiteral("mathColor");
static const QString s_referenceColor = QStringLiteral("referenceColor");
static const QString s_specialColor = QStringLiteral("specialColor");
static const QString s_enableMargin = QStringLiteral("enableMargin");
static const QString s_margin = QStringLiteral("margin");
static const QString s_enableColors = QStringLiteral("enableColors");
static const QString s_sidebarWidth = QStringLiteral("sidebarWidth");
static const QString s_indent = QStringLiteral("indent");
static const QString s_mode = QStringLiteral("mode");
static const QString s_tabs = QStringLiteral("tabs");
static const QString s_spaces = QStringLiteral("spaces");
static const QString s_spacesCount = QStringLiteral("spacesCount");
static const QString s_findCaseSensitive = QStringLiteral("findCaseSensitive");
static const QString s_findWholeWord = QStringLiteral("findWholeWord");
static const QString s_window = QStringLiteral("window");
static const QString s_width = QStringLiteral("width");
static const QString s_height = QStringLiteral("height");
static const QString s_x = QStringLiteral("x");
static const QString s_y = QStringLiteral("y");
static const QString s_maximized = QStringLiteral("maximized");
static const QString s_settingsWindow = QStringLiteral("settings_window");
static const QString s_spelling = QStringLiteral("spelling");
static const QString s_enabled = QStringLiteral("enabled");
static const QString s_plugins = QStringLiteral("plugins");
static const QString s_superscript = QStringLiteral("superscript");
static const QString s_delimiter = QStringLiteral("delimiter");
static const QString s_subscript = QStringLiteral("subscript");
static const QString s_mark = QStringLiteral("mark");
static const QString s_yaml = QStringLiteral("yaml");
static const QString s_autoLists = QStringLiteral("autoLists");
static const QString s_autoLinks = QStringLiteral("autoLinks");
static const QString s_autoEmojies = QStringLiteral("autoEmojies");
static const QString s_enableCodeBlockTheme = QStringLiteral("enableCodeBlockTheme");
static const QString s_codeBlockTheme = QStringLiteral("codeBlockTheme");
static const QString s_drawCodeBackground = QStringLiteral("drawCodeBackground");
static const QString s_githubBehaviour = QStringLiteral("githubBehaviour");
static const QString s_dontUseAutoListInCodeBlock = QStringLiteral("dontUseAutoListInCodeBlock");
static const QString s_autoCodeBlocks = QStringLiteral("autoCodeBlocks");
static const QString s_horizontalOrient = QStringLiteral("horizontalUi");
static const QString s_previewFollowEditor = QStringLiteral("previewFollowEditor");
static const QString s_lastCheckForUpdates = QStringLiteral("lastCheckForUpdates");
static const QString s_updatesAvailable = QStringLiteral("updatesAvailable");
static const QString s_updates = QStringLiteral("updates");
static const QString s_updatesUrl = QStringLiteral("updatesUrl");

void MainWindow::saveCfg() const
{
    const auto f = m_d->m_editor->font();

    QSettings s;

    s.beginGroup(s_ui);

    s.beginGroup(s_font);
    s.setValue(s_family, f.family());
    s.setValue(s_size, f.pointSize());
    s.endGroup();

    s.setValue(s_horizontalOrient, m_d->m_horizontalOrient);
    s.setValue(s_linkColor, m_d->m_editor->settings().m_colors.m_linkColor);
    s.setValue(s_textColor, m_d->m_editor->settings().m_colors.m_textColor);
    s.setValue(s_inlineColor, m_d->m_editor->settings().m_colors.m_inlineColor);
    s.setValue(s_htmlColor, m_d->m_editor->settings().m_colors.m_htmlColor);
    s.setValue(s_tableColor, m_d->m_editor->settings().m_colors.m_tableColor);
    s.setValue(s_codeColor, m_d->m_editor->settings().m_colors.m_codeColor);
    s.setValue(s_mathColor, m_d->m_editor->settings().m_colors.m_mathColor);
    s.setValue(s_referenceColor, m_d->m_editor->settings().m_colors.m_referenceColor);
    s.setValue(s_specialColor, m_d->m_editor->settings().m_colors.m_specialColor);
    s.setValue(s_enableMargin, m_d->m_editor->settings().m_margins.m_enable);
    s.setValue(s_margin, m_d->m_editor->settings().m_margins.m_length);
    s.setValue(s_enableColors, m_d->m_editor->settings().m_colors.m_enabled);
    s.setValue(s_enableCodeBlockTheme, m_d->m_editor->settings().m_colors.m_codeThemeEnabled);
    s.setValue(s_codeBlockTheme, m_d->m_editor->settings().m_colors.m_codeTheme);
    s.setValue(s_drawCodeBackground, m_d->m_editor->settings().m_colors.m_drawCodeBackground);
    s.setValue(s_sidebarWidth, m_d->m_tabWidth);
    s.setValue(s_previewFollowEditor, m_d->m_editor->settings().m_previewFollowEditor);

    s.beginGroup(s_indent);
    s.setValue(s_mode, m_d->m_editor->settings().m_indentMode == Editor::IndentMode::Tabs ? s_tabs : s_spaces);
    s.setValue(s_spacesCount, m_d->m_editor->settings().m_indentSpacesCount);
    s.endGroup();

    s.setValue(s_autoLists, m_d->m_editor->settings().m_isAutoListsEnabled);
    s.setValue(s_githubBehaviour, m_d->m_editor->settings().m_githubBehaviour);
    s.setValue(s_dontUseAutoListInCodeBlock, m_d->m_editor->settings().m_dontUseAutoListInCodeBlock);

    s.setValue(s_autoCodeBlocks, m_d->m_editor->settings().m_isAutoCodeBlocksEnabled);

    s.setValue(s_autoLinks, m_d->m_editor->settings().m_isLinksAutoCompletionEnabled);
    s.setValue(s_autoEmojies, m_d->m_editor->settings().m_isEmojiAutoCompletionEnabled);

    s.setValue(s_findCaseSensitive, m_d->m_find->isCaseSensitive());
    s.setValue(s_findWholeWord, m_d->m_find->isWholeWord());
    s.endGroup();

    s.beginGroup(s_window);

    s.setValue(s_width, width());
    s.setValue(s_height, height());
    s.setValue(s_x, windowHandle()->x());
    s.setValue(s_y, windowHandle()->y());
    s.setValue(s_maximized, isMaximized());

    s.endGroup();

    s.beginGroup(s_settingsWindow);
    s.setValue(s_width, m_d->m_settingsWindowWidth);
    s.setValue(s_height, m_d->m_settingsWindowHeight);
    s.setValue(s_maximized, m_d->m_settingsWindowMaximized);
    s.endGroup();

    s.beginGroup(s_spelling);
    s.setValue(s_enabled, m_d->m_editor->settings().m_enableSpelling);
    s.endGroup();

    Sonnet::Settings sonnet;
    sonnet.save();

    s.beginGroup(s_plugins);

    s.beginGroup(s_superscript);
    s.setValue(s_delimiter, m_d->m_editor->settings().m_pluginsCfg.m_sup.m_delimiter);
    s.setValue(s_enabled, m_d->m_editor->settings().m_pluginsCfg.m_sup.m_on);
    s.endGroup();

    s.beginGroup(s_subscript);
    s.setValue(s_delimiter, m_d->m_editor->settings().m_pluginsCfg.m_sub.m_delimiter);
    s.setValue(s_enabled, m_d->m_editor->settings().m_pluginsCfg.m_sub.m_on);
    s.endGroup();

    s.beginGroup(s_mark);
    s.setValue(s_delimiter, m_d->m_editor->settings().m_pluginsCfg.m_mark.m_delimiter);
    s.setValue(s_enabled, m_d->m_editor->settings().m_pluginsCfg.m_mark.m_on);
    s.endGroup();

    s.setValue(s_yaml, m_d->m_editor->settings().m_pluginsCfg.m_yamlEnabled);

    s.endGroup();

    s.beginGroup(s_updates);
    s.setValue(s_lastCheckForUpdates, m_d->m_lastCheckForUpdates);
    s.setValue(s_updatesAvailable, m_d->m_updatesAbailable);
    s.setValue(s_updatesUrl, m_d->m_updatesUrl);
    s.endGroup();
}

void MainWindow::readCfg()
{
    QSettings s;

    s.beginGroup(s_ui);

    s.beginGroup(s_font);

    {
        const auto fontName = s.value(s_family).toString();

        if (!fontName.isEmpty()) {
            auto fs = s.value(s_size).toInt();

            const QFont f(fontName, fs);

            m_d->m_editor->applyFont(f);
        }
    }

    s.endGroup();

    m_d->m_horizontalOrient = s.value(s_horizontalOrient, m_d->m_horizontalOrient).toBool();

    const auto isFindCaseSensitive = s.value(s_findCaseSensitive, true).toBool();
    m_d->m_find->setCaseSensitive(isFindCaseSensitive);

    const auto isFindWholeWord = s.value(s_findWholeWord, true).toBool();
    m_d->m_find->setWholeWord(isFindWholeWord);

    Colors colors = m_d->m_editor->settings().m_colors;

    const auto linkColor = s.value(s_linkColor, m_d->m_editor->settings().m_colors.m_linkColor).value<QColor>();
    if (linkColor.isValid()) {
        colors.m_linkColor = linkColor;
    }

    const auto textColor = s.value(s_textColor, m_d->m_editor->settings().m_colors.m_textColor).value<QColor>();
    if (textColor.isValid()) {
        colors.m_textColor = textColor;
    }

    const auto inlineColor = s.value(s_inlineColor, m_d->m_editor->settings().m_colors.m_inlineColor).value<QColor>();
    if (inlineColor.isValid()) {
        colors.m_inlineColor = inlineColor;
    }

    const auto htmlColor = s.value(s_htmlColor, m_d->m_editor->settings().m_colors.m_htmlColor).value<QColor>();
    if (htmlColor.isValid()) {
        colors.m_htmlColor = htmlColor;
    }

    const auto tableColor = s.value(s_tableColor, m_d->m_editor->settings().m_colors.m_tableColor).value<QColor>();
    if (tableColor.isValid()) {
        colors.m_tableColor = tableColor;
    }

    const auto codeColor = s.value(s_codeColor, m_d->m_editor->settings().m_colors.m_codeColor).value<QColor>();
    if (codeColor.isValid()) {
        colors.m_codeColor = codeColor;
    }

    const auto mathColor = s.value(s_mathColor, m_d->m_editor->settings().m_colors.m_mathColor).value<QColor>();
    if (mathColor.isValid()) {
        colors.m_mathColor = mathColor;
    }

    const auto refColor =
        s.value(s_referenceColor, m_d->m_editor->settings().m_colors.m_referenceColor).value<QColor>();
    if (refColor.isValid()) {
        colors.m_referenceColor = refColor;
    }

    const auto specialColor =
        s.value(s_specialColor, m_d->m_editor->settings().m_colors.m_specialColor).value<QColor>();
    if (specialColor.isValid()) {
        colors.m_specialColor = specialColor;
    }

    colors.m_codeThemeEnabled =
        s.value(s_enableCodeBlockTheme, m_d->m_editor->settings().m_colors.m_codeThemeEnabled).toBool();
    colors.m_codeTheme = s.value(s_codeBlockTheme, QStringLiteral("GitHub Light")).toString();
    colors.m_drawCodeBackground =
        s.value(s_drawCodeBackground, m_d->m_editor->settings().m_colors.m_drawCodeBackground).toBool();

    m_d->m_editor->enableAutoLists(s.value(s_autoLists, m_d->m_editor->settings().m_isAutoListsEnabled).toBool());
    m_d->m_editor->enableGithubBehaviour(
        s.value(s_githubBehaviour, m_d->m_editor->settings().m_githubBehaviour).toBool());
    m_d->m_editor->enableAutoListInCodeBlock(
        !s.value(s_dontUseAutoListInCodeBlock, !m_d->m_editor->settings().m_dontUseAutoListInCodeBlock).toBool());
    m_d->m_editor->enableAutoCodeBlocks(
        s.value(s_autoCodeBlocks, m_d->m_editor->settings().m_isAutoCodeBlocksEnabled).toBool());
    m_d->m_editor->enableAutoCompletionOfLinks(
        s.value(s_autoLinks, m_d->m_editor->settings().m_isLinksAutoCompletionEnabled).toBool());
    m_d->m_editor->enableAutoCompletionOfEmojies(
        s.value(s_autoEmojies, m_d->m_editor->settings().m_isEmojiAutoCompletionEnabled).toBool());
    m_d->m_pinPreviewEditor->setChecked(
        s.value(s_previewFollowEditor, m_d->m_editor->settings().m_previewFollowEditor).toBool());
    onPinPreviewEditor(m_d->m_pinPreviewEditor->isChecked());

    Margins margins = m_d->m_editor->settings().m_margins;

    const auto enableMargin = s.value(s_enableMargin, margins.m_enable).toBool();
    margins.m_enable = enableMargin;

    const auto margin = s.value(s_margin, margins.m_length).toInt();
    margins.m_length = margin;

    m_d->m_editor->applyMargins(margins);

    const auto enableColors = s.value(s_enableColors, m_d->m_editor->settings().m_colors.m_enabled).toBool();
    colors.m_enabled = enableColors;

    m_d->m_editor->applyColors(colors);

    const auto sidebarWidth = s.value(s_sidebarWidth, 0).toInt();
    if (sidebarWidth > 0) {
        m_d->m_tabWidth = sidebarWidth;
    }

    s.beginGroup(s_indent);

    const auto mode = s.value(s_mode, s_tabs).toString();
    if (mode == s_tabs) {
        m_d->m_editor->setIndentMode(Editor::IndentMode::Tabs);
    } else {
        m_d->m_editor->setIndentMode(Editor::IndentMode::Spaces);
    }

    const auto spacesCount = s.value(s_spacesCount, m_d->m_editor->settings().m_indentSpacesCount).toInt();

    if (spacesCount > 0) {
        m_d->m_editor->setIndentSpacesCount(spacesCount);
    }

    s.endGroup();

    s.endGroup();

    s.beginGroup(s_window);

    const auto width = s.value(s_width, -1).toInt();
    const auto height = s.value(s_height, -1).toInt();

    if (width > 0 && height > 0) {
        resize(width, height);

        const auto x = s.value(s_x, 0).toInt();
        const auto y = s.value(s_y, 0).toInt();

        windowHandle()->setX(x);
        windowHandle()->setY(y);

        const auto maximized = s.value(s_maximized, false).toBool();
        if (maximized) {
            showMaximized();
        }
    }

    s.endGroup();

    s.beginGroup(s_settingsWindow);
    m_d->m_settingsWindowWidth = s.value(s_width, -1).toInt();
    m_d->m_settingsWindowHeight = s.value(s_height, -1).toInt();
    m_d->m_settingsWindowMaximized = s.value(s_maximized, false).toBool();
    s.endGroup();

    s.beginGroup(s_spelling);
    m_d->m_editor->enableSpellingCheck(s.value(s_enabled, m_d->m_editor->settings().m_enableSpelling).toBool());
    s.endGroup();

    MdShared::PluginsCfg pluginsCfg;

    s.beginGroup(s_plugins);

    s.beginGroup(s_superscript);
    pluginsCfg.m_sup.m_delimiter = s.value(s_delimiter, QChar()).toChar();
    pluginsCfg.m_sup.m_on = s.value(s_enabled, m_d->m_editor->settings().m_pluginsCfg.m_sup.m_on).toBool();
    s.endGroup();

    s.beginGroup(s_subscript);
    pluginsCfg.m_sub.m_delimiter = s.value(s_delimiter, QChar()).toChar();
    pluginsCfg.m_sub.m_on = s.value(s_enabled, m_d->m_editor->settings().m_pluginsCfg.m_sub.m_on).toBool();
    s.endGroup();

    s.beginGroup(s_mark);
    pluginsCfg.m_mark.m_delimiter = s.value(s_delimiter, QChar()).toChar();
    pluginsCfg.m_mark.m_on = s.value(s_enabled, m_d->m_editor->settings().m_pluginsCfg.m_mark.m_on).toBool();
    s.endGroup();

    pluginsCfg.m_yamlEnabled = s.value(s_yaml, m_d->m_editor->settings().m_pluginsCfg.m_yamlEnabled).toBool();

    s.endGroup();

    s.beginGroup(s_updates);
    m_d->m_lastCheckForUpdates =
        s.value(s_lastCheckForUpdates,
                QDateTime::currentDateTime()
                    - std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::hours(2)))
            .toDateTime();
    m_d->m_updatesAbailable = s.value(s_updatesAvailable, QString()).toString();
    m_d->m_updatesUrl = s.value(s_updatesUrl, QString()).toString();
    s.endGroup();

    onCheckForUpdates();
    m_d->m_editor->setPluginsCfg(pluginsCfg);
    m_d->m_editor->doUpdate();
}

void MainWindow::onLessFontSize()
{
    auto f = m_d->m_editor->font();

    if (f.pointSize() > 5) {
        f.setPointSize(f.pointSize() - 1);

        m_d->m_editor->applyFont(f);

        m_d->m_editor->doUpdate();

        saveCfg();
    }
}

void MainWindow::onMoreFontSize()
{
    auto f = m_d->m_editor->font();

    if (f.pointSize() < 66) {
        f.setPointSize(f.pointSize() + 1);

        m_d->m_editor->applyFont(f);

        m_d->m_editor->doUpdate();

        saveCfg();
    }
}

void MainWindow::onCursorPositionChanged()
{
    if (m_d->m_standardEditMenu) {
        m_d->m_standardEditMenu->deleteLater();
        m_d->m_standardEditMenu = nullptr;
    }

    m_d->m_standardEditMenu = m_d->m_editor->createStandardContextMenu(m_d->m_editor->cursorRect().center());

    m_d->m_standardEditMenu->addSeparator();

    m_d->m_standardEditMenu->addAction(m_d->m_toggleFindAction);
    m_d->m_standardEditMenu->addAction(m_d->m_toggleGoToLineAction);
    m_d->m_standardEditMenu->addSeparator();
    m_d->m_standardEditMenu->addAction(m_d->m_toggleFindWebAction);
    m_d->m_standardEditMenu->addSeparator();
    m_d->m_standardEditMenu->addAction(m_d->m_nextMisspelled);
    m_d->m_standardEditMenu->addSeparator();
    m_d->m_standardEditMenu->addAction(m_d->m_addTOCAction);

    m_d->m_editMenuAction->setMenu(m_d->m_standardEditMenu);

    connect(m_d->m_standardEditMenu, &QMenu::triggered, this, &MainWindow::onEditMenuActionTriggered);

    const auto c = m_d->m_editor->textCursor();

    m_d->m_tabAction->setEnabled(c.hasSelection());
    m_d->m_backtabAction->setEnabled(c.hasSelection());

    m_d->m_cursorPosLabel->setText(
        tr("<b>Line:</b> %1, <b>Col:</b> %2").arg(c.blockNumber() + 1).arg(c.positionInBlock() + 1));

    if (m_d->m_editor->settings().m_previewFollowEditor) {
        scrollToCursor();
    }
}

void MainWindow::onEditMenuActionTriggered(QAction *action)
{
    if (action != m_d->m_toggleFindAction
        && action != m_d->m_toggleGoToLineAction
        && action != m_d->m_toggleFindWebAction) {
        m_d->m_editor->setFocus();
    }
}

namespace
{

struct Node {
    QVector<QString> m_keys;
    QVector<QPair<QSharedPointer<Node>, QTreeWidgetItem *>> m_children;
    QTreeWidgetItem *m_self = nullptr;
};

qsizetype countOfFiles(const Node &root)
{
    qsizetype count = 0;

    for (qsizetype i = 0; i < root.m_children.size(); ++i) {
        if (!root.m_children.at(i).second->data(0, Qt::UserRole).toString().isEmpty()) {
            ++count;
        } else {
            count += countOfFiles(*root.m_children.at(i).first.get());
        }
    }

    return count;
}

}

void MainWindow::loadAllLinkedFilesImpl()
{
    if (m_d->m_loadAllFlag) {
        m_d->m_loadAllFlag = false;

        m_d->m_tabs->removeTab(1);
        m_d->m_filePanel->hide();
        m_d->m_actionMenu->menuAction()->setVisible(false);
        m_d->m_saveAsAction->setEnabled(true);

        closeAllLinkedFiles();

        updateLoadAllLinkedFilesMenuText();

        onTextChanged();

        return;
    } else {
        if (isModified()) {
            QMessageBox::information(this, windowTitle(), tr("You have unsaved changes. Please save document first."));

            m_d->m_editor->setFocus();

            return;
        }

        m_d->m_loadAllFlag = true;

        m_d->m_saveAsAction->setEnabled(false);
    }

    readAllLinked(true);

    m_d->m_fileTree->clear();
    m_d->m_filePanel->show();
    m_d->m_tabs->addTab(m_d->m_filePanel, tr("&Navigation"));

    const auto rootFolder = m_d->m_workingDirectoryWidget->workingDirectory() + QStringLiteral("/");

    Node root;

    if (m_d->m_mdDoc) {
        for (auto it = m_d->m_mdDoc->items().cbegin(), last = m_d->m_mdDoc->items().cend(); it != last; ++it) {
            if ((*it)->type() == MD::ItemType::Anchor) {
                const auto fullFileName = static_cast<MD::Anchor *>(it->get())->label();

                const auto fileName =
                    fullFileName.startsWith(rootFolder) ? fullFileName.sliced(rootFolder.size()) : fullFileName;

                const auto parts = fileName.split(QStringLiteral("/"));

                Node *current = &root;

                for (qsizetype i = 0; i < parts.size(); ++i) {
                    const QString f = parts.at(i).isEmpty() ? QStringLiteral("/") : parts.at(i);

                    if (i == parts.size() - 1) {
                        if (!current->m_keys.contains(f)) {
                            auto tmp = QSharedPointer<Node>::create();
                            auto item = new QTreeWidgetItem(current->m_self);
                            item->setIcon(0, QIcon(":/icon/icon_16x16.png"));
                            item->setData(0, Qt::UserRole, fullFileName);
                            m_d->m_fullFileNames.insert(fullFileName);
                            tmp->m_self = item;
                            item->setText(0, f);
                            current->m_children.push_back({tmp, item});
                            current->m_keys.push_back(f);
                            current = tmp.get();
                        }
                    } else {
                        if (!current->m_keys.contains(f)) {
                            auto tmp = QSharedPointer<Node>::create();
                            auto item = new QTreeWidgetItem(current->m_self);
                            item->setIcon(0,
                                          QIcon::fromTheme(QStringLiteral("folder-yellow"),
                                                           QIcon(":/res/img/folder-yellow.png")));
                            tmp->m_self = item;
                            item->setText(0, f);
                            current->m_children.push_back({tmp, item});
                            current->m_keys.push_back(f);
                            current = tmp.get();
                        } else {
                            current = current->m_children.at(current->m_keys.indexOf(f)).first.get();
                        }
                    }
                }
            }
        }
    }

    if (countOfFiles(root) > 1) {
        for (auto it = root.m_children.cbegin(), last = root.m_children.cend(); it != last; ++it) {
            m_d->m_fileTree->addTopLevelItem(it->second);
        }

        m_d->m_navigationStack.clear();
        m_d->m_navigationStack.push_back(m_d->m_rootFilePath);
        m_d->m_navigationStackIdx = 0;
        m_d->m_backBtn->setEnabled(false);
        m_d->m_fwdBtn->setEnabled(false);
        m_d->m_goBackAction->setEnabled(false);
        m_d->m_goFwdAction->setEnabled(false);

        if (!m_d->m_previewMode) {
            m_d->m_actionMenu->menuAction()->setVisible(true);

            QMessageBox::information(this,
                                     windowTitle(),
                                     tr("HTML preview is ready. Modifications in files will not update "
                                        "HTML preview till you save changes."));
        } else {
            m_d->m_actionMenu->menuAction()->setVisible(false);
        }
    } else {
        closeAllLinkedFiles();

        m_d->m_tabs->removeTab(1);
        m_d->m_filePanel->hide();
        m_d->m_actionMenu->menuAction()->setVisible(false);

        QMessageBox::information(this, windowTitle(), tr("This document doesn't have linked documents."));
    }

    if (!m_d->m_previewMode) {
        m_d->m_editor->setFocus();
    }

    updateLoadAllLinkedFilesMenuText();
}

void MainWindow::loadAllLinkedFiles()
{
    m_d->runWhenEditorReady(std::bind(&MainWindow::loadAllLinkedFilesImpl, this));
}

void MainWindow::setWorkingDirectory(const QString &path)
{
    m_d->m_tmpWorkingDir.clear();

    QDir dir(path);

    if (dir.exists()) {
        m_d->m_tmpWorkingDir = dir.absolutePath();
    }

    if (!m_d->m_tmpWorkingDir.isEmpty()) {
        m_d->runWhenEditorReady(std::bind(&MainWindow::onSetWorkingDirectory, this));
    }
}

void MainWindow::setStartupState(const StartupState &st)
{
    m_d->m_startupState = st;
}

void MainWindow::onSetWorkingDirectory()
{
    if (!m_d->m_tmpWorkingDir.isEmpty()) {
        if (m_d->m_tmpWorkingDir != m_d->m_workingDirectoryWidget->fullPath()
            && m_d->m_workingDirectoryWidget->fullPath().contains(m_d->m_tmpWorkingDir)) {
            const auto idx = MdShared::FolderChooser::splitPath(m_d->m_tmpWorkingDir).size() - 1;

            m_d->m_workingDirectoryWidget->folderChooser()->emulateClick(idx);
        }
    }
}

void MainWindow::onProcessQueue()
{
    while (!m_d->m_funcsQueue.isEmpty()) {
        const auto f = m_d->m_funcsQueue.front();
        m_d->m_funcsQueue.pop_front();

        f();

        if (!m_d->m_editor->isReady()) {
            break;
        }
    }
}

void MainWindow::onFirstTimeShown()
{
    readCfg();

    updateStyle(false, true);

    if (!m_d->m_startupState.m_fileName.isEmpty()) {
        openFile(m_d->m_startupState.m_fileName);
    } else {
        onFileNew();
    }

    if (!m_d->m_startupState.m_workingDir.isEmpty()) {
        setWorkingDirectory(m_d->m_startupState.m_workingDir);
    }

    if (m_d->m_startupState.m_loadAllLinked) {
        loadAllLinkedFiles();
    }

    if (!m_d->m_horizontalOrient) {
        m_d->m_horizontalOrient = true;
        onChangeOrient();
    }

    if (m_d->m_startupState.m_previewMode) {
        openInPreviewMode();
    }

    connect(m_d->m_editor->document(), &QTextDocument::modificationChanged, this, &MainWindow::setWindowModified);
}

void MainWindow::onGoBack()
{
    if (m_d->m_navigationStackIdx > 0) {
        --m_d->m_navigationStackIdx;

        openFileFromNavigationToolbar(m_d->m_navigationStack[m_d->m_navigationStackIdx], false);
    }
}

void MainWindow::onGoForward()
{
    if (m_d->m_navigationStackIdx + 1 < m_d->m_navigationStack.size()) {
        ++m_d->m_navigationStackIdx;

        openFileFromNavigationToolbar(m_d->m_navigationStack[m_d->m_navigationStackIdx], false);
    }
}

void MainWindow::closeAllLinkedFiles()
{
    m_d->m_loadAllFlag = false;

    m_d->m_fullFileNames.clear();

    m_d->m_editor->setFocus();

    onTextChanged();
}

void MainWindow::readAllLinked(bool updateRootFileName)
{
    if (m_d->m_loadAllFlag) {
        if (updateRootFileName) {
            m_d->m_rootFilePath = m_d->m_editor->docName();
        }

        MD::Parser parser;
        setPlugins(parser, m_d->m_editor->settings().m_pluginsCfg, true);

        if (m_d->m_workingDirectoryWidget->isRelative()) {
            m_d->m_mdDoc = parser.parse(m_d->m_rootFilePath,
                                        true,
                                        {QStringLiteral("md"), QStringLiteral("mkd"), QStringLiteral("markdown")});
        } else {
            m_d->m_mdDoc = parser.parse(m_d->m_rootFilePath,
                                        m_d->m_workingDirectoryWidget->workingDirectory(),
                                        true,
                                        {QStringLiteral("md"), QStringLiteral("mkd"), QStringLiteral("markdown")});
        }

        MD::details::IdsMap idsMap;

        for (auto it = m_d->m_mdDoc->items().cbegin(), last = m_d->m_mdDoc->items().cend(); it != last; ++it) {
            if ((*it)->type() == MD::ItemType::Anchor) {
                auto a = static_cast<MD::Anchor *>(it->get());

                if (a->label() == m_d->m_editor->docName()) {
                    const auto doc = m_d->m_editor->currentDoc();

                    for (auto eIt = doc->items().cbegin(), eLast = doc->items().cend(); eIt != eLast; ++eIt, ++it) {
                        const auto idIt = m_d->m_editor->idsMap().find(eIt->get());

                        if (idIt != m_d->m_editor->idsMap().cend()) {
                            idsMap.insert(it->get(), idIt.value());
                        }
                    }

                    break;
                }
            }
        }

        if (m_d->m_livePreviewVisible) {
            m_d->m_html->setText(MD::toHtml<HtmlConv>(m_d->m_mdDoc,
                                                      false,
                                                      QStringLiteral("<img src=\"qrc:/res/img/go-jump.png\" />"),
                                                      true,
                                                      &idsMap));
        }
    }
}

void MainWindow::onNavigationDoubleClicked(QTreeWidgetItem *item,
                                           int)
{
    openFileFromNavigationToolbar(item->data(0, Qt::UserRole).toString());
}

void MainWindow::openFileFromNavigationToolbar(const QString &path,
                                               bool modifyStack)
{
    if (!path.isEmpty()) {
        if (isModified()) {
            QMessageBox::information(this, windowTitle(), tr("You have unsaved changes. Please save document first."));

            m_d->m_editor->setFocus();

            return;
        }

        QSharedPointer<QFile> f;
        bool autosavedLoaded = false;

        std::tie(f, autosavedLoaded) = getFile(path);

        if (!f) {
            return;
        }

        m_d->m_editor->setDocName(path);
        m_d->m_editor->enableAutoSave(false);
        m_d->m_editor->setText(f->readAll());
        m_d->m_editor->enableAutoSave(true);
        f->close();

        updateWindowTitle();

        m_d->m_editor->document()->clearUndoRedoStacks();
        m_d->m_editor->document()->setModified(autosavedLoaded);
        m_d->m_editor->setFocus();
        setWindowModified(autosavedLoaded);

        if (modifyStack) {
            if (m_d->m_navigationStackIdx >= 0) {
                m_d->m_navigationStack.remove(m_d->m_navigationStackIdx + 1,
                                              m_d->m_navigationStack.size() - m_d->m_navigationStackIdx - 1);
            }

            m_d->m_navigationStack.push_back(path);
            m_d->m_navigationStackIdx = m_d->m_navigationStack.size() - 1;
        }

        m_d->m_backBtn->setEnabled(m_d->m_navigationStackIdx > 0);
        m_d->m_fwdBtn->setEnabled(m_d->m_navigationStackIdx + 1 < m_d->m_navigationStack.size());
        m_d->m_goBackAction->setEnabled(m_d->m_navigationStackIdx > 0);
        m_d->m_goFwdAction->setEnabled(m_d->m_navigationStackIdx + 1 < m_d->m_navigationStack.size());

        onCursorPositionChanged();

        m_d->runWhenEditorReady(std::bind(&MainWindow::onEditorReady, this));
    }
}

void MainWindow::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->mimeData()->hasUrls()) {
        for (const auto &url : event->mimeData()->urls()) {
            if (url.isLocalFile()) {
                const auto fileName = url.toLocalFile();

                QMimeDatabase db;
                QMimeType mime = db.mimeTypeForFile(fileName);

                if (mime.inherits(QStringLiteral("text/markdown"))
                    || fileName.endsWith(QStringLiteral(".md"), Qt::CaseInsensitive)
                    || fileName.endsWith(QStringLiteral(".markdown"), Qt::CaseInsensitive)) {
                    event->acceptProposedAction();

                    return;
                }
            }
        }
    }
}

void MainWindow::dropEvent(QDropEvent *event)
{
    for (const auto &url : event->mimeData()->urls()) {
        if (url.isLocalFile()) {
            const auto fileName = url.toLocalFile();

            if (fileName.endsWith(".md", Qt::CaseInsensitive) || fileName.endsWith(".markdown", Qt::CaseInsensitive)) {
                if (!askAboutUnsavedFile()) {
                    return;
                }

                openFile(fileName);

                event->acceptProposedAction();

                return;
            }
        }
    }
}

void MainWindow::onEditorReady()
{
    readAllLinked();

    m_d->initMarkdownMenu();
}

void MainWindow::onOpenRequestedRef()
{
    if (!m_d->m_requestedRef.isEmpty()) {
        m_d->m_editor->tryToNavigate(m_d->m_requestedRef);
    }
}

void MainWindow::onScrollEditor(const QString &id)
{
    if (id.isEmpty()) {
        return;
    }

    qsizetype i = id.length() - 1;

    for (; i >= 0; --i) {
        if (!id[i].isNumber()) {
            break;
        }
    }

    const auto number = (i < id.length() - 1 ? id.sliced(i + 1, id.length() - 1 - i) : QString());
    const auto withNum = (id[i] == QLatin1Char('-') && !number.isEmpty());

    auto tmp = id;

    if (withNum) {
        tmp = id.sliced(0, i);
    }

    const auto fit = m_d->m_mdDoc->footnotesMap().find(tmp);

    if (!tmp.isEmpty() && (fit != m_d->m_mdDoc->footnotesMap().cend() || m_d->m_editor->itemsMap().contains(tmp))) {
        auto item = (fit != m_d->m_mdDoc->footnotesMap().cend() ? fit->get() : m_d->m_editor->itemsMap()[tmp]);

        auto c = m_d->m_editor->textCursor();

        c.setPosition(m_d->m_editor->document()
                          ->findBlockByNumber(item->startLine() + (withNum ? number.toInt() : 0))
                          .position());

        m_d->m_editor->unfoldLine(c);

        m_d->m_editor->setTextCursor(c);

        m_d->m_editor->ensureCursorVisible();

        m_d->m_editor->setFocus();
    }
}

void MainWindow::updateWindowTitle()
{
    setWindowTitle(tr("%1[*] - Markdown Editor%2")
                       .arg(QFileInfo(m_d->m_editor->docName()).fileName(),
                            m_d->m_previewMode ? tr(" [Preview Mode]") : QString()));
}

void MainWindow::updateLoadAllLinkedFilesMenuText()
{
    if (m_d->m_loadAllFlag) {
        m_d->m_loadAllAction->setText(tr("Show Only Current File..."));
        m_d->m_addTOCAction->setEnabled(false);
        m_d->m_preview->enableScrollEditor(false);
    } else {
        m_d->m_loadAllAction->setText(tr("Load All Linked Files..."));
        m_d->m_addTOCAction->setEnabled(true);
        m_d->m_preview->enableScrollEditor(true);
    }
}

void MainWindow::onTogglePreviewAction(bool checked)
{
    m_d->m_previewMode = checked;

    if (checked) {
        m_d->m_settingsMenu->menuAction()->setVisible(false);
        m_d->m_editMenuAction->setVisible(false);
        m_d->m_saveAction->setVisible(false);
        m_d->m_saveAction->setEnabled(false);
        m_d->m_saveAsAction->setVisible(false);
        m_d->m_saveAsAction->setEnabled(false);
        m_d->m_toggleFindAction->setEnabled(false);
        m_d->m_toggleGoToLineAction->setEnabled(false);
        m_d->m_newAction->setVisible(false);
        m_d->m_newAction->setEnabled(false);
        m_d->m_editor->setVisible(false);
        m_d->m_livePreviewAction->setVisible(false);
        m_d->m_livePreviewAction->setEnabled(false);
        m_d->m_orientAction->setVisible(false);
        m_d->m_orientAction->setEnabled(false);
        m_d->m_formatMenu->menuAction()->setEnabled(false);
        m_d->m_formatMenu->menuAction()->setVisible(false);
        m_d->m_sidebarPanel->hide();
        m_d->m_mdStandardAction->setEnabled(false);
        m_d->m_tabEditorSplitter->handle(1)->setCursor(Qt::ArrowCursor);
        m_d->m_editorPreviewSplitter->handle(1)->setCursor(Qt::ArrowCursor);
        m_d->m_cursorPosLabel->hide();

        if (m_d->m_tabsVisible) {
            showOrHideTabs();
        }

        m_d->m_tabEditorSplitter->setSizes({0, centralWidget()->width()});
        m_d->m_editorPreviewSplitter->setSizes(m_d->m_horizontalOrient ? QList<int>{0, centralWidget()->width()}
                                                                       : QList<int>{centralWidget()->height(), 0});
        m_d->m_editorPreviewSplitter->handle(1)->setEnabled(false);
        m_d->m_editorPreviewSplitter->handle(1)->setVisible(false);
        m_d->m_tabEditorSplitter->handle(1)->setEnabled(false);
        m_d->m_tabEditorSplitter->handle(1)->setVisible(false);

        m_d->m_actionMenu->menuAction()->setVisible(false);
        m_d->m_editor->setEnabled(false);
    } else {
        m_d->m_settingsMenu->menuAction()->setVisible(true);
        m_d->m_editMenuAction->setVisible(true);
        m_d->m_saveAction->setVisible(true);
        m_d->m_saveAction->setEnabled(true);
        m_d->m_saveAsAction->setVisible(true);
        m_d->m_saveAsAction->setEnabled(true);
        m_d->m_toggleFindAction->setEnabled(true);
        m_d->m_toggleGoToLineAction->setEnabled(true);
        m_d->m_newAction->setVisible(true);
        m_d->m_newAction->setEnabled(true);
        m_d->m_orientAction->setVisible(true);
        m_d->m_orientAction->setEnabled(true);
        m_d->m_editor->setVisible(true);
        m_d->m_sidebarPanel->show();
        m_d->m_livePreviewAction->setVisible(true);
        m_d->m_livePreviewAction->setEnabled(true);
        m_d->m_formatMenu->menuAction()->setEnabled(true);
        m_d->m_formatMenu->menuAction()->setVisible(true);
        m_d->m_mdStandardAction->setEnabled(true);

        m_d->m_cursorPosLabel->show();

        if (m_d->m_loadAllFlag) {
            m_d->m_actionMenu->menuAction()->setVisible(true);
        }

        m_d->m_tabEditorSplitter->setSizes({m_d->m_minTabWidth, centralWidget()->width() - m_d->m_minTabWidth});
        m_d->m_tabEditorSplitter->handle(1)->setEnabled(true);
        m_d->m_tabEditorSplitter->handle(1)->setVisible(true);

        setStateOfEditorPreviewSplitter(false);

        m_d->m_editor->setEnabled(true);
        m_d->m_editor->setFocus();
    }

    updateLoadAllLinkedFilesMenuText();

    updateWindowTitle();
}

void MainWindow::setStateOfEditorPreviewSplitter(bool updateHtml)
{
    if (!m_d->m_livePreviewVisible) {
        m_d->m_editorPreviewSplitter->setSizes(m_d->m_horizontalOrient
                                                   ? QList<int>{m_d->m_editorPreviewWidget->width(), 0}
                                                   : QList<int>{0, m_d->m_editorPreviewWidget->height()});
        m_d->m_editorPreviewSplitter->handle(1)->setCursor(Qt::ArrowCursor);
        m_d->m_editorPreviewSplitter->handle(1)->setEnabled(false);
        m_d->m_editorPreviewSplitter->handle(1)->setVisible(false);
    } else {
        const auto s =
            m_d->m_horizontalOrient ? m_d->m_editorPreviewWidget->width() : m_d->m_editorPreviewWidget->height();
        m_d->m_editorPreviewSplitter->setSizes({s / 2, s / 2});
        m_d->m_editorPreviewSplitter->handle(1)->setCursor(m_d->m_horizontalOrient ? Qt::SplitHCursor
                                                                                   : Qt::SplitVCursor);
        m_d->m_editorPreviewSplitter->handle(1)->setEnabled(true);
        m_d->m_editorPreviewSplitter->handle(1)->setVisible(true);

        if (updateHtml) {
            if (m_d->m_mdDoc) {
                m_d->m_html->setText(MD::toHtml<HtmlConv>(m_d->m_mdDoc,
                                                          false,
                                                          QStringLiteral("<img src=\"qrc:/res/img/go-jump.png\" />"),
                                                          true,
                                                          &m_d->m_editor->idsMap()));
            } else {
                m_d->m_html->setText({});
            }
        }
    }
}

void MainWindow::onToggleLivePreviewAction(bool checked)
{
    m_d->m_livePreviewVisible = checked;

    setStateOfEditorPreviewSplitter(true);
}

void MainWindow::onChangeOrient()
{
    m_d->m_horizontalOrient = !m_d->m_horizontalOrient;

    if (m_d->m_horizontalOrient) {
        m_d->m_orientAction->setIcon(QIcon::fromTheme(QStringLiteral("view-split-top-bottom"),
                                                      QIcon(QStringLiteral(":/res/img/view-split-top-bottom.png"))));
        m_d->m_orientAction->setText(tr("Split Vertically"));
        m_d->m_editorPreviewSplitter->setOrientation(Qt::Orientation::Horizontal);
        m_d->m_editorPreviewSplitter->addWidget(m_d->m_previewPanel);
    } else {
        m_d->m_orientAction->setIcon(QIcon::fromTheme(QStringLiteral("view-split-left-right"),
                                                      QIcon(QStringLiteral(":/res/img/view-split-left-right.png"))));
        m_d->m_orientAction->setText(tr("Split Horizontally"));
        m_d->m_editorPreviewSplitter->setOrientation(Qt::Orientation::Vertical);
        m_d->m_editorPreviewSplitter->addWidget(m_d->m_editorPanel);
    }

    setStateOfEditorPreviewSplitter(false);
}

namespace /* anonymous */
{

inline QString paragraphToMD(MD::Paragraph *p,
                             QPlainTextEdit *editor)
{
    QTextCursor c = editor->textCursor();

    c.movePosition(QTextCursor::Start);

    for (long long int i = 0; i < p->startLine(); ++i) {
        c.movePosition(QTextCursor::NextBlock);
    }

    for (long long int i = 0; i < p->startColumn(); ++i) {
        c.movePosition(QTextCursor::Right);
    }

    for (long long int i = p->startLine(); i < p->endLine(); ++i) {
        c.movePosition(QTextCursor::NextBlock, QTextCursor::KeepAnchor);
    }

    for (long long int i = (p->startLine() == p->endLine() ? p->startColumn() : 0); i <= p->endColumn(); ++i) {
        c.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor);
    }

    auto res = c.selectedText();
    res.replace(QChar('\n'), QChar(' '));
    res.replace(QChar(0x2029), QChar(' '));

    return res;
}

inline QString simplifyLabel(const QString &label,
                             const QString &fileName)
{
    return label.sliced(0, label.lastIndexOf(fileName) - 1).toLower();
}

} /* namespace anonymous */

void MainWindow::onAddTOC()
{
    QString toc;
    int offset = 0;
    QString fileName;
    std::vector<int> current;

    MD::forEach(
        {MD::ItemType::Anchor, MD::ItemType::Heading},
        m_d->m_editor->currentDoc(),
        [&](MD::Item *item) {
            if (item->type() == MD::ItemType::Anchor) {
                auto a = static_cast<MD::Anchor *>(item);
                fileName = a->label();
            } else if (item->type() == MD::ItemType::Heading) {
                auto h = static_cast<MD::Heading *>(item);

                if (current.size()) {
                    if (h->level() < current.front()) {
                        current.clear();
                    } else {
                        current.erase(std::find_if(current.cbegin(),
                                                   current.cend(),
                                                   [h](const auto &i) {
                                                       return i >= h->level();
                                                   }),
                                      current.cend());
                    }
                }

                current.push_back(h->level());
                offset = (current.size() - 1) * 2;

                toc.append(QString(offset, QChar(' ')));
                toc.append(QStringLiteral("* ["));
                toc.append(paragraphToMD(h->text().get(), m_d->m_editor));
                toc.append(QStringLiteral("]("));
                toc.append(simplifyLabel(h->label(), fileName));
                toc.append(QStringLiteral(")\n"));
            }
        },
        1);

    m_d->m_editor->insertPlainText(toc);
}

void MainWindow::onShowLicenses()
{
    MdShared::LicenseDialog msg(this);
    msg.addLicense(s_oxygenName, s_oxygenLicense);
    msg.addLicense(s_katexName, s_katexLicense);
    msg.addLicense(s_githubMarkdownCssName, s_githubMarkdownCssLicense);
    msg.addLicense(s_highlightJsName, s_highlightJsLicense);
    msg.addLicense(s_highlightBlockquoteName, s_highlightBlockquoteLicense);
    msg.addLicense(s_sonnetName, s_sonnetLicense);
    msg.addLicense(s_md4qtName, s_md4qtLicense);
    msg.addLicense(s_kwidgetsaddonsName, s_kwidgetsaddonsLicense);

#ifdef Q_OS_WIN
    msg.addLicense(s_breezeName, s_breezeLicense);
#endif

    msg.exec();
}

void MainWindow::onChangeColors()
{
    ColorsDialog dlg(m_d->m_editor->settings().m_colors,
                     m_d->m_editor->syntaxHighlighter().codeBlockSyntaxHighlighter(),
                     this);

    if (dlg.exec() == QDialog::Accepted) {
        if (m_d->m_editor->settings().m_colors != dlg.colors()) {
            m_d->m_editor->applyColors(dlg.colors());

            m_d->m_editor->doUpdate();

            saveCfg();
        }
    }
}

void MainWindow::onTocClicked(const QModelIndex &index)
{
    auto c = m_d->m_editor->textCursor();

    c.setPosition(m_d->m_editor->document()
                      ->findBlockByNumber(m_d->m_tocModel->lineNumber(m_d->m_filterTocModel->mapToSource(index)))
                      .position());

    m_d->m_editor->setTextCursor(c);

    m_d->m_editor->ensureCursorVisible();

    m_d->m_editor->setFocus();
}

void MainWindow::showOrHideTabs()
{
    m_d->m_tocPanel->setVisible(!m_d->m_tabsVisible);

    auto s = m_d->m_tabEditorSplitter->sizes();

    if (!m_d->m_tabsVisible) {
        if (m_d->m_tabWidth == -1) {
            m_d->m_tabWidth = 250;
        }

        s[0] = m_d->m_tabWidth;
        s[1] = s[1] - m_d->m_tabWidth + m_d->m_minTabWidth;
        m_d->m_sidebarPanel->setMaximumWidth(QWIDGETSIZE_MAX);
        m_d->m_tabEditorSplitter->handle(1)->setCursor(Qt::SplitHCursor);
    } else {
        m_d->m_tabWidth = s[0];
        const auto w = s[0] - m_d->m_minTabWidth;
        s[0] = m_d->m_minTabWidth;
        s[1] = s[1] + w;
        m_d->m_sidebarPanel->setFixedWidth(m_d->m_minTabWidth);
        m_d->m_tabEditorSplitter->handle(1)->setCursor(Qt::ArrowCursor);
        m_d->m_editor->setFocus();
    }

    m_d->m_tabsVisible = !m_d->m_tabsVisible;

    m_d->m_tabEditorSplitter->setSizes(s);
}

void MainWindow::onTabClicked(int index)
{
    if (m_d->m_tabs->currentIndex() == index || !m_d->m_tabsVisible) {
        showOrHideTabs();
    }

    m_d->m_currentTab = index;

    if (index == 0) {
        m_d->initMarkdownMenu();
    }
}

void MainWindow::onSettings()
{
    SettingsDlg dlg(m_d->m_editor->settings(), m_d->m_editor->syntaxHighlighter().codeBlockSyntaxHighlighter(), this);

    if (m_d->m_settingsWindowWidth != -1 && m_d->m_settingsWindowHeight != -1) {
        dlg.resize(m_d->m_settingsWindowWidth, m_d->m_settingsWindowHeight);
    }

    if (m_d->m_settingsWindowMaximized) {
        dlg.showMaximized();
    }

    bool spellingSettingsChanged = false;

    connect(dlg.sonnetConfigWidget(), &Sonnet::ConfigWidget::configChanged, [&spellingSettingsChanged]() {
        spellingSettingsChanged = true;
    });

    if (dlg.exec() == QDialog::Accepted) {
        if (dlg.settings() != m_d->m_editor->settings()) {
            const auto settings = dlg.settings();

            m_d->m_editor->applyColors(settings.m_colors);
            m_d->m_editor->applyFont(settings.m_font);
            m_d->m_editor->applyMargins(settings.m_margins);

            if (m_d->m_editor->settings().m_enableSpelling != settings.m_enableSpelling || spellingSettingsChanged) {
                m_d->m_editor->enableSpellingCheck(settings.m_enableSpelling);
            }

            m_d->m_editor->enableAutoLists(settings.m_isAutoListsEnabled);
            m_d->m_editor->enableGithubBehaviour(settings.m_githubBehaviour);
            m_d->m_editor->enableAutoListInCodeBlock(!settings.m_dontUseAutoListInCodeBlock);
            m_d->m_editor->enableAutoCodeBlocks(settings.m_isAutoCodeBlocksEnabled);
            m_d->m_editor->enableAutoCompletionOfLinks(settings.m_isLinksAutoCompletionEnabled);
            m_d->m_editor->enableAutoCompletionOfEmojies(settings.m_isEmojiAutoCompletionEnabled);
            m_d->m_pinPreviewEditor->setChecked(settings.m_previewFollowEditor);
            onPinPreviewEditor(m_d->m_pinPreviewEditor->isChecked());

            m_d->m_editor->doUpdate();

            if (m_d->m_editor->settings().m_pluginsCfg != settings.m_pluginsCfg) {
                m_d->m_editor->setPluginsCfg(settings.m_pluginsCfg);

                onEditorReady();
            }

            m_d->m_editor->setIndentMode(settings.m_indentMode);
            m_d->m_editor->setIndentSpacesCount(settings.m_indentSpacesCount);
        }
    }

    m_d->m_settingsWindowWidth = dlg.width();
    m_d->m_settingsWindowHeight = dlg.height();
    m_d->m_settingsWindowMaximized = dlg.isMaximized();

    saveCfg();
}

} /* namespace MdEditor */

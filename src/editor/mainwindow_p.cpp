/*
    SPDX-FileCopyrightText: 2026 Igor Mironchik <igor.mironchik@gmail.com>
    SPDX-License-Identifier: GPL-3.0-or-later
*/

// md-editor include.
#include "find.h"
#include "findweb.h"
#include "gotoline.h"
#include "htmldocument.h"
#include "mainwindow.h"
#include "previewpage.h"
#include "settings.h"
#include "webview.h"
#include "widgets.h"
#include "wordwrapdelegate.h"

// md4qt include.
#include <md4qt/src/algo.h>

// Qt include.
#include <QDesktopServices>
#include <QDir>
#include <QFile>
#include <QFutureWatcher>
#include <QHeaderView>
#include <QMenuBar>
#include <QSortFilterProxyModel>
#include <QSplitter>
#include <QStatusBar>
#include <QStringListModel>
#include <QStyleHints>
#include <QToolButton>
#include <QTreeWidget>
#include <QWebChannel>

namespace MdEditor
{

//
// MainWindowPrivate
//

MainWindowPrivate::MainWindowPrivate(MainWindow *parent)
    : m_q(parent)
{
}

void MainWindowPrivate::notifyTocTree(QAbstractItemModel *model,
                                      WordWrapItemDelegate *delegate,
                                      const QModelIndex &parent)
{
    for (int i = 0; i < model->rowCount(parent); ++i) {
        const auto idx = model->index(i, 0, parent);

        emit delegate->sizeHintChanged(idx);

        if (model->rowCount(idx) > 0) {
            notifyTocTree(model, delegate, idx);
        }
    }
}

QFrame *MainWindowPrivate::makeSeparator() const
{
    auto separator = new QFrame(m_q->statusBar());
    separator->setFrameShape(QFrame::VLine);
    separator->setFrameShadow(QFrame::Sunken);

    return separator;
}

void MainWindowPrivate::initUi()
{
    {
        QFile wrapperHtmlFile(":/res/index.html");

        if (!wrapperHtmlFile.open(QFile::ReadOnly | QFile::Text)) {
            m_htmlContent = MainWindow::tr("Error loading res/index.html");
        } else {
            QTextStream stream(&wrapperHtmlFile);
            m_htmlContent = stream.readAll();
            wrapperHtmlFile.close();
            const auto isDark = (qApp->styleHints()->colorScheme() == Qt::ColorScheme::Dark);
            m_htmlContent = m_htmlContent.arg(isDark ? QStringLiteral("qrc:/res/css/github-dark.css")
                                                     : QStringLiteral("qrc:/res/css/github.css"));
        }
    }

    auto w = new QWidget(m_q);
    auto l = new QVBoxLayout(w);
    l->setContentsMargins(0, 0, 0, 0);
    l->setSpacing(0);

    m_tabEditorSplitter = new QSplitter(w);
    m_tabEditorSplitter->setOrientation(Qt::Orientation::Horizontal);

    // Sidebar.
    m_sidebarPanel = new QWidget(m_tabEditorSplitter);
    QVBoxLayout *sl = new QVBoxLayout(m_sidebarPanel);
    sl->setContentsMargins(0, 0, 0, 0);
    sl->setSpacing(0);
    m_tabs = new TabWidget(m_sidebarPanel);
    sl->addWidget(m_tabs);
    m_tabs->setTabPosition(QTabWidget::East);

    QObject::connect(m_tabs, &TabWidget::activated, m_q, &MainWindow::onTabActivated);
    QObject::connect(m_tabs, &TabWidget::removed, [this]() {
        this->handleCurrentTab();
    });

    m_tocPanel = new QWidget(m_tabs);
    auto tpv = new QVBoxLayout(m_tocPanel);
    tpv->setContentsMargins(3, 3, 3, 3);
    tpv->setSpacing(3);
    m_tocFilterLine = new QLineEdit(m_tocPanel);
    m_tocFilterLine->setPlaceholderText(MainWindow::tr("Filter ToC (Ctrl+Alt+F)"));
    tpv->addWidget(m_tocFilterLine);
    auto tocFilterAction = new QAction(m_q);
    tocFilterAction->setShortcut(MainWindow::tr("Ctrl+Alt+F"));
    tocFilterAction->setShortcutContext(Qt::ApplicationShortcut);
    m_q->addAction(tocFilterAction);

    QObject::connect(tocFilterAction, &QAction::triggered, [this]() {
        this->m_tocFilterLine->setFocus();
        this->m_tocFilterLine->selectAll();
    });

    m_tocTree = new TocTreeView(m_tocPanel);
    m_tocModel = new TocModel(m_tocTree);
    m_filterTocModel = new QSortFilterProxyModel(m_tocTree);
    m_filterTocModel->setSourceModel(m_tocModel);
    m_filterTocModel->setFilterCaseSensitivity(Qt::CaseInsensitive);
    m_filterTocModel->setRecursiveFilteringEnabled(true);
    m_tocTree->setModel(m_filterTocModel);
    m_tocTree->setHeaderHidden(true);
    m_delegate = new WordWrapItemDelegate(m_tocTree, m_tocModel, m_filterTocModel);
    m_tocTree->setItemDelegate(m_delegate);
    m_tocTree->setAlternatingRowColors(true);
    m_tocTree->setSortingEnabled(false);
    tpv->addWidget(m_tocTree);
    m_tabs->addTab(m_tocPanel, MainWindow::tr("To&C"));

    m_urlAutoCompleteModel = new QStringListModel(m_q);

    m_filePanel = new QWidget(m_tabs);
    auto fpv = new QVBoxLayout(m_filePanel);
    fpv->setContentsMargins(3, 3, 3, 3);
    fpv->setSpacing(3);
    m_backBtn = new QToolButton(m_filePanel);
    m_backBtn->setIcon(
        QIcon::fromTheme(QStringLiteral("go-previous"), QIcon(QStringLiteral(":/res/img/go-previous-16.png"))));
    m_backBtn->setToolTip(MainWindow::tr("Go Back"));
    m_fwdBtn = new QToolButton(m_filePanel);
    m_fwdBtn->setIcon(QIcon::fromTheme(QStringLiteral("go-next"), QIcon(QStringLiteral(":/res/img/go-next-16.png"))));
    m_fwdBtn->setToolTip(MainWindow::tr("Go Forward"));
    m_backBtn->setEnabled(false);
    m_fwdBtn->setEnabled(false);
    auto fph = new QHBoxLayout;
    fph->setContentsMargins(0, 0, 0, 0);
    fph->setSpacing(3);
    fph->addWidget(m_backBtn);
    fph->addWidget(m_fwdBtn);
    fph->addItem(new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Fixed));
    fpv->addLayout(fph);
    m_fileTree = new QTreeWidget(m_filePanel);
    m_fileTree->setHeaderHidden(true);
    fpv->addWidget(m_fileTree);
    m_filePanel->hide();

    QObject::connect(m_fileTree, &QTreeWidget::itemDoubleClicked, m_q, &MainWindow::onNavigationDoubleClicked);
    QObject::connect(m_tocTree->header(), &QHeaderView::sectionResized, [this](int, int, int) {
        notifyTocTree(this->m_filterTocModel, this->m_delegate, QModelIndex());
    });
    QObject::connect(m_tocTree, &TocTreeView::scrollWebViewToRequested, m_q, &MainWindow::onScrollWebViewTo);

    QObject::connect(m_tocFilterLine, &QLineEdit::textChanged, [this](const QString &text) {
        this->m_filterTocModel->setFilterFixedString(text);
    });
    QObject::connect(m_backBtn, &QToolButton::clicked, m_q, &MainWindow::onGoBack);
    QObject::connect(m_fwdBtn, &QToolButton::clicked, m_q, &MainWindow::onGoForward);

    m_tocPanel->hide();

    // Editor.
    m_editorPreviewWidget = new QWidget(m_tabEditorSplitter);
    auto editorPreviewLayout = new QVBoxLayout(m_editorPreviewWidget);
    m_editorPreviewSplitter = new QSplitter(m_editorPreviewWidget);
    m_editorPreviewSplitter->setOrientation(Qt::Orientation::Horizontal);
    editorPreviewLayout->addWidget(m_editorPreviewSplitter);

    m_editorPanel = new QWidget(m_editorPreviewSplitter);
    auto v = new QVBoxLayout(m_editorPanel);
    v->setContentsMargins(0, 0, 0, 0);
    v->setSpacing(0);
    m_editor = new Editor(m_editorPanel, m_q, m_urlAutoCompleteModel);
    m_find = new Find(m_q, m_editor, m_editorPanel);
    m_editor->setFindWidget(m_find);
    m_gotoline = new GoToLine(m_q, m_editor, m_editorPanel);
    v->addWidget(m_editor);
    v->addWidget(m_gotoline);
    v->addWidget(m_find);

    // Preview.
    m_previewPanel = new QWidget(m_editorPreviewSplitter);
    auto v1 = new QVBoxLayout(m_previewPanel);
    v1->setContentsMargins(0, 0, 0, 0);
    v1->setSpacing(0);
    m_preview = new WebView(m_previewPanel);
    m_findWeb = new FindWeb(m_q, m_preview, m_previewPanel);
    v1->addWidget(m_preview);
    v1->addWidget(m_findWeb);

    m_editorPreviewSplitter->addWidget(m_editorPanel);
    m_editorPreviewSplitter->addWidget(m_previewPanel);

    m_find->hide();
    m_gotoline->hide();
    m_findWeb->hide();

    m_tabEditorSplitter->addWidget(m_sidebarPanel);
    m_tabEditorSplitter->addWidget(m_editorPreviewWidget);

    this->m_tabEditorSplitter->handle(1)->setCursor(Qt::ArrowCursor);

    l->addWidget(m_tabEditorSplitter);

    m_q->setCentralWidget(w);

    m_page = new PreviewPage(m_preview);
    m_preview->setPage(m_page);

    m_html = new HtmlDocument(m_q);

    auto channel = new QWebChannel(m_q);
    channel->registerObject(QStringLiteral("content"), m_html);
    m_page->setWebChannel(channel);

    QObject::connect(m_preview, &WebView::scrollRequested, m_q, &MainWindow::onScrollEditor, Qt::QueuedConnection);

#ifdef Q_OS_WIN
    m_mdPdfExe = QStringLiteral("md-pdf-gui.exe");
    m_launcherExe = QStringLiteral("md-launcher.exe");
#else
    m_mdPdfExe = QStringLiteral("md-pdf-gui");
    m_launcherExe = QStringLiteral("md-launcher");
#endif

    QDir workingDir(QApplication::applicationDirPath());
    const auto mdPdfExeFiles = workingDir.entryInfoList({m_mdPdfExe}, QDir::Executable | QDir::Files);
    const auto starterExeFiles = workingDir.entryInfoList({m_launcherExe}, QDir::Executable | QDir::Files);

    auto fileMenu = m_q->menuBar()->addMenu(MainWindow::tr("&File"));
    m_newAction = fileMenu->addAction(
        QIcon::fromTheme(QStringLiteral("document-new"), QIcon(QStringLiteral(":/res/img/document-new.png"))),
        MainWindow::tr("New"),
        MainWindow::tr("Ctrl+N"),
        m_q,
        &MainWindow::onFileNew);
    m_openAction = fileMenu->addAction(
        QIcon::fromTheme(QStringLiteral("document-open"), QIcon(QStringLiteral(":/res/img/document-open.png"))),
        MainWindow::tr("Open"),
        MainWindow::tr("Ctrl+O"),
        m_q,
        &MainWindow::onFileOpen);
    fileMenu->addSeparator();
    m_saveAction = fileMenu->addAction(
        QIcon::fromTheme(QStringLiteral("document-save"), QIcon(QStringLiteral(":/res/img/document-save.png"))),
        MainWindow::tr("Save"),
        MainWindow::tr("Ctrl+S"),
        m_q,
        &MainWindow::onFileSave);
    m_saveAsAction = fileMenu->addAction(
        QIcon::fromTheme(QStringLiteral("document-save-as"), QIcon(QStringLiteral(":/res/img/document-save-as.png"))),
        MainWindow::tr("Save As"),
        m_q,
        &MainWindow::onFileSaveAs);
    fileMenu->addSeparator();
    m_loadAllAction = fileMenu->addAction(MainWindow::tr("Load All Linked Files..."),
                                          MainWindow::tr("Ctrl+R"),
                                          m_q,
                                          &MainWindow::loadAllLinkedFiles);
    m_loadAllAction->setEnabled(false);

    if (!mdPdfExeFiles.isEmpty() && !starterExeFiles.isEmpty()) {
        fileMenu->addSeparator();

        m_convertToPdfAction =
            fileMenu->addAction(MainWindow::tr("Convert To PDF..."), m_q, &MainWindow::onConvertToPdf);

        m_convertToPdfAction->setEnabled(false);
    }

    fileMenu->addSeparator();
    fileMenu->addAction(
        QIcon::fromTheme(QStringLiteral("application-exit"), QIcon(QStringLiteral(":/res/img/application-exit.png"))),
        MainWindow::tr("Quit"),
        MainWindow::tr("Ctrl+Q"),
        m_q,
        &QWidget::close);

    m_editMenuAction = m_q->menuBar()->addAction(MainWindow::tr("&Edit"));
    m_toggleFindAction =
        new QAction(QIcon::fromTheme(QStringLiteral("edit-find"), QIcon(QStringLiteral(":/res/img/edit-find.png"))),
                    MainWindow::tr("Find/Replace"),
                    m_q);
    m_toggleFindAction->setShortcut(MainWindow::tr("Ctrl+F"));
    m_q->addAction(m_toggleFindAction);

    m_toggleFindWebAction =
        new QAction(QIcon::fromTheme(QStringLiteral("edit-find"), QIcon(QStringLiteral(":/res/img/edit-find.png"))),
                    MainWindow::tr("Find In Preview"),
                    m_q);
    m_toggleFindWebAction->setShortcut(MainWindow::tr("Ctrl+W"));
    m_q->addAction(m_toggleFindWebAction);

    m_toggleGoToLineAction =
        new QAction(QIcon::fromTheme(QStringLiteral("go-next-use"), QIcon(QStringLiteral(":/res/img/go-next-use.png"))),
                    MainWindow::tr("Go to Line"),
                    m_q);
    m_toggleGoToLineAction->setShortcut(MainWindow::tr("Ctrl+L"));
    m_q->addAction(m_toggleGoToLineAction);

    m_addTOCAction = new QAction(MainWindow::tr("Add ToC"), m_q);

    m_nextMisspelled = new QAction(MainWindow::tr("Next Misspelled"), m_q);
    m_nextMisspelled->setShortcut(MainWindow::tr("F6"));
    m_nextMisspelled->setEnabled(false);
    m_q->addAction(m_nextMisspelled);

    m_formatMenu = m_q->menuBar()->addMenu(MainWindow::tr("F&ormat"));

    m_tabAction = new QAction(MainWindow::tr("Indent"), m_formatMenu);
    m_tabAction->setShortcut(MainWindow::tr("Tab"));
    m_tabAction->setShortcutContext(Qt::WidgetShortcut);
    QObject::connect(m_tabAction, &QAction::triggered, [this]() {
        qApp->postEvent(this->m_editor, new QKeyEvent(QEvent::KeyPress, Qt::Key_Tab, Qt::NoModifier));
    });
    m_formatMenu->addAction(m_tabAction);

    m_backtabAction = new QAction(MainWindow::tr("Unindent"), m_formatMenu);
    m_backtabAction->setShortcut(MainWindow::tr("Shift+Tab"));
    m_backtabAction->setShortcutContext(Qt::WidgetShortcut);
    QObject::connect(m_backtabAction, &QAction::triggered, [this]() {
        qApp->postEvent(this->m_editor, new QKeyEvent(QEvent::KeyPress, Qt::Key_Backtab, Qt::NoModifier));
    });
    m_formatMenu->addAction(m_backtabAction);

    m_actionMenu = m_q->menuBar()->addMenu(MainWindow::tr("&Action"));
    m_goBackAction =
        new QAction(QIcon::fromTheme(QStringLiteral("go-previous"), QIcon(QStringLiteral(":/res/img/go-previous.png"))),
                    MainWindow::tr("Go Back"),
                    m_q);
    m_goBackAction->setShortcut(QKeySequence(Qt::ControlModifier | Qt::AltModifier | Qt::Key_Left));
    m_goBackAction->setShortcutContext(Qt::ApplicationShortcut);
    m_actionMenu->addAction(m_goBackAction);
    m_goBackAction->setEnabled(false);
    m_goFwdAction =
        new QAction(QIcon::fromTheme(QStringLiteral("go-next"), QIcon(QStringLiteral(":/res/img/go-next.png"))),
                    MainWindow::tr("Go Forward"),
                    m_q);
    m_goFwdAction->setShortcut(QKeySequence(Qt::ControlModifier | Qt::AltModifier | Qt::Key_Right));
    m_goFwdAction->setShortcutContext(Qt::ApplicationShortcut);
    m_actionMenu->addAction(m_goFwdAction);
    m_goFwdAction->setEnabled(false);
    m_actionMenu->menuAction()->setVisible(false);

    QObject::connect(m_goBackAction, &QAction::triggered, m_q, &MainWindow::onGoBack);
    QObject::connect(m_goFwdAction, &QAction::triggered, m_q, &MainWindow::onGoForward);

    auto viewMenu = m_q->menuBar()->addMenu(MainWindow::tr("&View"));
    m_viewAction = new QAction(
        QIcon::fromTheme(QStringLiteral("view-preview"), QIcon(QStringLiteral(":/res/img/view-preview.png"))),
        MainWindow::tr("Toggle Preview Mode"));
    m_viewAction->setShortcut(MainWindow::tr("Ctrl+P"));
    m_viewAction->setCheckable(true);
    m_viewAction->setChecked(false);
    viewMenu->addAction(m_viewAction);
    m_livePreviewAction = new QAction(
        QIcon::fromTheme(QStringLiteral("layer-visible-on"), QIcon(QStringLiteral(":/res/img/layer-visible-on.png"))),
        MainWindow::tr("Live Preview"));
    m_livePreviewAction->setShortcut(MainWindow::tr("Ctrl+Alt+P"));
    m_livePreviewAction->setCheckable(true);
    m_livePreviewAction->setChecked(true);
    viewMenu->addAction(m_livePreviewAction);

    m_orientAction = new QAction(QIcon::fromTheme(QStringLiteral("view-split-top-bottom"),
                                                  QIcon(QStringLiteral(":/res/img/view-split-top-bottom.png"))),
                                 MainWindow::tr("Split Vertically"),
                                 m_q);
    m_orientAction->setShortcut(MainWindow::tr("Ctrl+Alt+S"));
    viewMenu->addAction(m_orientAction);

    QObject::connect(m_orientAction, &QAction::triggered, m_q, &MainWindow::onChangeOrient);

    m_settingsMenu = m_q->menuBar()->addMenu(MainWindow::tr("&Settings"));
    auto toggleLineNumbersAction =
        new QAction(QIcon::fromTheme(QStringLiteral("view-table-of-contents-ltr"),
                                     QIcon(QStringLiteral(":/res/img/view-table-of-contents-ltr.png"))),
                    MainWindow::tr("Show Line Numbers"),
                    m_q);
    toggleLineNumbersAction->setCheckable(true);
    toggleLineNumbersAction->setShortcut(MainWindow::tr("Alt+L"));
    toggleLineNumbersAction->setChecked(true);
    m_settingsMenu->addAction(toggleLineNumbersAction);

    auto toggleUnprintableCharacters = new QAction(
        QIcon::fromTheme(QStringLiteral("character-set"), QIcon(QStringLiteral(":/res/img/character-set.png"))),
        MainWindow::tr("Show Tabs/Spaces"),
        m_q);
    toggleUnprintableCharacters->setCheckable(true);
    toggleUnprintableCharacters->setShortcut(MainWindow::tr("Alt+T"));
    toggleUnprintableCharacters->setChecked(true);
    m_settingsMenu->addAction(toggleUnprintableCharacters);

    m_settingsMenu->addSeparator();

    m_settingsMenu->addAction(QIcon::fromTheme(QStringLiteral("format-font-size-less"),
                                               QIcon(QStringLiteral(":/res/img/format-font-size-less.png"))),
                              MainWindow::tr("Decrease Font Size"),
                              MainWindow::tr("Ctrl+-"),
                              m_q,
                              &MainWindow::onLessFontSize);
    m_settingsMenu->addAction(QIcon::fromTheme(QStringLiteral("format-font-size-more"),
                                               QIcon(QStringLiteral(":/res/img/format-font-size-more.png"))),
                              MainWindow::tr("Increase Font Size"),
                              MainWindow::tr("Ctrl+="),
                              m_q,
                              &MainWindow::onMoreFontSize);

    m_settingsMenu->addSeparator();

    m_settingsMenu->addAction(QIcon::fromTheme(QStringLiteral("preferences-desktop-font"),
                                               QIcon(QStringLiteral(":/res/img/preferences-desktop-font.png"))),
                              MainWindow::tr("Font..."),
                              m_q,
                              &MainWindow::onChooseFont);

    m_settingsMenu->addAction(
        QIcon::fromTheme(QStringLiteral("fill-color"), QIcon(QStringLiteral(":/res/img/fill-color.png"))),
        MainWindow::tr("Colors..."),
        m_q,
        &MainWindow::onChangeColors);

#if defined(Q_OS_WIN) && defined(MD_BREEZE)
    m_settingsMenu->addSeparator();

    m_themeAction = m_settingsMenu->addAction(MainWindow::tr("Dark Mode"), m_q, &MainWindow::onChangeTheme);
#endif

    m_settingsMenu->addSeparator();

    m_settingsMenu->addAction(
        QIcon::fromTheme(QStringLiteral("configure"), QIcon(QStringLiteral(":/res/img/configure.png"))),
        MainWindow::tr("Settings"),
        m_q,
        &MainWindow::onSettings);

    auto helpMenu = m_q->menuBar()->addMenu(MainWindow::tr("&Help"));
    helpMenu->addAction(QIcon(QStringLiteral(":/icon/icon_24x24.png")),
                        MainWindow::tr("About"),
                        m_q,
                        &MainWindow::onAbout);
    helpMenu->addAction(QIcon(QStringLiteral(":/img/Qt-logo-neon-transparent.png")),
                        MainWindow::tr("About Qt"),
                        m_q,
                        &MainWindow::onAboutQt);
    helpMenu->addAction(QIcon(QStringLiteral(":/icon/icon_24x24.png")), MainWindow::tr("About Markdown"), []() {
        QDesktopServices::openUrl(QUrl(QStringLiteral("https://spec.commonmark.org/0.31.2/")));
    });
    m_mdStandardAction = helpMenu->addAction(QIcon(QStringLiteral(":/icon/icon_24x24.png")),
                                             MainWindow::tr("Extract from the standard"),
                                             m_q,
                                             &MainWindow::onMarkdownStandardHelp);
    m_mdStandardAction->setShortcut(MainWindow::tr("F1"));
    helpMenu->addAction(QIcon::fromTheme(QStringLiteral("bookmarks-organize"),
                                         QIcon(QStringLiteral(":/res/img/bookmarks-organize.png"))),
                        MainWindow::tr("Licenses"),
                        m_q,
                        &MainWindow::onShowLicenses);
    helpMenu->addSeparator();
    helpMenu->addAction(QIcon::fromTheme(QStringLiteral("help-hint"), QIcon(QStringLiteral(":/res/img/help-hint.png"))),
                        MainWindow::tr("Tips && Tricks"),
                        []() {
                            QDesktopServices::openUrl(
                                QUrl(QStringLiteral("https://igormironchik.github.io/markdown-tools/")));
                        });
    helpMenu->addSeparator();
    helpMenu->addAction(
        QIcon::fromTheme(QStringLiteral("tools-report-bug"), QIcon(QStringLiteral(":/res/img/tools-report-bug.png"))),
        MainWindow::tr("Report Bug"),
        []() {
            QDesktopServices::openUrl(QUrl(QStringLiteral("https://github.com/igormironchik/markdown-tools/issues")));
        });

    m_cursorPosLabel = new QLabel(m_q);
    m_workingDirectoryWidget = new WorkingDirectoryWidget(m_q);

    m_pinPreviewEditor = new QToolButton(m_q->statusBar());
    m_pinPreviewEditor->setCheckable(true);
    m_pinPreviewEditor->setIconSize(QSize(16, 16));
    m_pinPreviewEditor->setMinimumHeight(m_workingDirectoryWidget->labelHeight());
    m_pinPreviewEditor->setMaximumHeight(m_workingDirectoryWidget->labelHeight());
    m_pinPreviewEditor->setMinimumWidth(m_workingDirectoryWidget->labelHeight());
    m_pinPreviewEditor->setMaximumWidth(m_workingDirectoryWidget->labelHeight());

    QObject::connect(m_pinPreviewEditor, &QToolButton::toggled, m_q, &MainWindow::onPinPreviewEditor);

    m_q->statusBar()->addPermanentWidget(m_pinPreviewEditor);
    m_q->statusBar()->addPermanentWidget(makeSeparator());
    m_q->statusBar()->addPermanentWidget(m_workingDirectoryWidget);
    m_q->statusBar()->addPermanentWidget(makeSeparator());
    m_q->statusBar()->addPermanentWidget(m_cursorPosLabel);
    m_workingDirectoryWidget->hide();

    QObject::connect(m_workingDirectoryWidget,
                     &WorkingDirectoryWidget::workingDirectoryChanged,
                     m_q,
                     &MainWindow::onWorkingDirectoryChange);
    QObject::connect(m_editor->document(), &QTextDocument::modificationChanged, m_saveAction, &QAction::setEnabled);
    QObject::connect(m_editor, &Editor::ready, m_q, &MainWindow::onTextChanged);
    QObject::connect(m_editor, &Editor::misspelled, m_q, &MainWindow::onMisspelledFount);
    QObject::connect(m_editor, &Editor::lineHovered, m_q, &MainWindow::onLineHovered);
    QObject::connect(m_editor,
                     &Editor::lineNumberContextMenuRequested,
                     m_q,
                     &MainWindow::onLineNumberContextMenuRequested);
    QObject::connect(toggleLineNumbersAction, &QAction::toggled, m_editor, &Editor::showLineNumbers);
    QObject::connect(toggleUnprintableCharacters, &QAction::toggled, m_editor, &Editor::showUnprintableCharacters);
    QObject::connect(m_toggleFindAction, &QAction::triggered, m_q, &MainWindow::onFind);
    QObject::connect(m_toggleFindWebAction, &QAction::triggered, m_q, &MainWindow::onFindWeb);
    QObject::connect(m_toggleGoToLineAction, &QAction::triggered, m_q, &MainWindow::onGoToLine);
    QObject::connect(m_nextMisspelled, &QAction::triggered, m_q, &MainWindow::onNextMisspelled);
    QObject::connect(m_page, &QWebEnginePage::linkHovered, [this](const QString &url) {
        if (!url.isEmpty())
            this->m_q->statusBar()->showMessage(url);
        else
            this->m_q->statusBar()->clearMessage();
    });
    QObject::connect(m_editor, &QPlainTextEdit::cursorPositionChanged, m_q, &MainWindow::onCursorPositionChanged);
    QObject::connect(m_viewAction, &QAction::toggled, m_q, &MainWindow::onTogglePreviewAction);
    QObject::connect(m_livePreviewAction, &QAction::toggled, m_q, &MainWindow::onToggleLivePreviewAction);
    QObject::connect(m_addTOCAction, &QAction::triggered, m_q, &MainWindow::onAddTOC);
    QObject::connect(m_tabs, &QTabWidget::tabBarClicked, m_q, &MainWindow::onTabClicked);
    QObject::connect(m_tocTree, &QTreeView::activated, m_q, &MainWindow::onTocClicked);
    QObject::connect(m_tocTree, &QTreeView::clicked, m_q, &MainWindow::onTocClicked);
    QObject::connect(m_editor, &Editor::ready, m_q, &MainWindow::onProcessQueue);

    m_editor->setFocus();
    m_preview->setFocusPolicy(Qt::ClickFocus);
    m_editor->applyColors(m_editor->settings().m_colors);

    m_q->setTabOrder(m_gotoline->line(), m_find->editLine());
    m_q->setTabOrder(m_find->editLine(), m_find->replaceLine());
    m_q->setTabOrder(m_find->replaceLine(), m_findWeb->line());

    m_q->setAcceptDrops(true);

    m_updateWatcher = new QFutureWatcher<Update>(m_q);

    QObject::connect(m_updateWatcher, &QFutureWatcher<Update>::finished, m_q, &MainWindow::onCheckForUpdatesFinished);
}

void MainWindowPrivate::handleCurrentTab()
{
    if (m_tabs->currentIndex() == 0) {
        initMarkdownMenu();
    }

    m_currentTab = m_tabs->currentIndex();
}

StringDataVec MainWindowPrivate::paragraphToMenuText(MD::Paragraph *p,
                                                     bool skipRtl)
{
    StringDataVec res;
    bool rtl = false;
    bool first = !skipRtl;

    for (auto it = p->items().cbegin(), last = p->items().cend(); it != last; ++it) {
        switch ((*it)->type()) {
        case MD::ItemType::Text: {
            auto t = static_cast<MD::Text *>(it->get());

            if (first) {
                first = false;
                rtl = t->text().isRightToLeft();
            }

            res.append({t->text(), false, rtl});
        } break;

        case MD::ItemType::Code: {
            auto c = static_cast<MD::Code *>(it->get());

            if (first) {
                first = false;
                rtl = c->text().isRightToLeft();
            }

            res.append({c->text(), true, rtl});
        } break;

        case MD::ItemType::Link: {
            auto l = static_cast<MD::Link *>(it->get());

            first = false;

            if (!l->p()->isEmpty())
                res.append(paragraphToMenuText(l->p().get(), true));
        } break;

        default:
            break;
        }
    }

    return res;
}

void MainWindowPrivate::initMarkdownMenu()
{
    if (m_tocDoc != m_editor->currentDoc()) {
        m_tocDoc = m_editor->currentDoc();

        TocStringLevelVec newToc;

        MD::forEach(
            {MD::ItemType::Heading},
            m_tocDoc,
            [this, &newToc](MD::Item *item) {
                auto h = static_cast<MD::Heading *>(item);

                newToc.push_back({this->paragraphToMenuText(h->text().get()), h->level(), h->startLine()});
            },
            1);

        if (newToc != m_tocModel->tocStrings()) {
            QStringList urls;
            m_tocModel->clear();

            std::vector<QModelIndex> current;

            MD::forEach(
                {MD::ItemType::Heading},
                m_tocDoc,
                [this, &current, &urls](MD::Item *item) {
                    auto h = static_cast<MD::Heading *>(item);

                    if (h->text()) {
                        if (current.size()) {
                            if (h->level() < this->m_tocModel->level(current.front())) {
                                current.clear();
                            } else {
                                current.erase(std::find_if(current.cbegin(),
                                                           current.cend(),
                                                           [h, this](const auto &i) {
                                                               return this->m_tocModel->level(i) >= h->level();
                                                           }),
                                              current.cend());
                            }

                            if (current.empty()) {
                                this->m_tocModel->addTopLevelItem(this->paragraphToMenuText(h->text().get()),
                                                                  h->startLine(),
                                                                  h->level(),
                                                                  h->label());
                                const auto idx = this->m_tocModel->index(this->m_tocModel->rowCount() - 1, 0);

                                current.push_back(idx);
                                urls.append(QLatin1Char('#') + this->m_tocModel->shortId(idx));
                            } else {
                                this->m_tocModel->addChildItem(current.back(),
                                                               this->paragraphToMenuText(h->text().get()),
                                                               h->startLine(),
                                                               h->level(),
                                                               h->label());

                                const auto idx = this->m_tocModel->index(this->m_tocModel->rowCount(current.back()) - 1,
                                                                         0,
                                                                         current.back());
                                current.push_back(idx);
                                urls.append(QLatin1Char('#') + this->m_tocModel->shortId(idx));
                            }
                        } else {
                            this->m_tocModel->addTopLevelItem(this->paragraphToMenuText(h->text().get()),
                                                              h->startLine(),
                                                              h->level(),
                                                              h->label());

                            const auto idx = this->m_tocModel->index(this->m_tocModel->rowCount() - 1, 0);

                            current.push_back(idx);
                            urls.append(QLatin1Char('#') + this->m_tocModel->shortId(idx));
                        }
                    }
                },
                1);

            std::sort(urls.begin(), urls.end());
            m_urlAutoCompleteModel->setStringList(urls);
        } else {
            m_tocModel->updateTocLines(newToc);
        }
    }
}

} /* namespace MdEditor */

/*
    SPDX-FileCopyrightText: 2024-2025 Igor Mironchik <igor.mironchik@gmail.com>
    SPDX-License-Identifier: GPL-3.0-or-later
*/

// md-editor include.
#include "mainwindow.hpp"
#include "colorsdlg.hpp"
#include "editor.hpp"
#include "find.hpp"
#include "findweb.hpp"
#include "fontdlg.hpp"
#include "gotoline.hpp"
#include "htmldocument.hpp"
#include "previewpage.hpp"
#include "settings.hpp"
#include "syntaxvisitor.hpp"
#include "toc.hpp"
#include "version.hpp"
#include "webview.hpp"
#include "wordwrapdelegate.hpp"

// Sonnet include.
#include <Sonnet/Settings>

// Qt include.
#include <QAction>
#include <QApplication>
#include <QCloseEvent>
#include <QDir>
#include <QFileDialog>
#include <QFileInfo>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QKeyEvent>
#include <QLabel>
#include <QLineEdit>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QProcess>
#include <QResizeEvent>
#include <QSortFilterProxyModel>
#include <QSplitter>
#include <QStandardPaths>
#include <QStatusBar>
#include <QStyleOptionTab>
#include <QTabBar>
#include <QTabWidget>
#include <QTextBlock>
#include <QTextDocumentFragment>
#include <QToolTip>
#include <QToolButton>
#include <QTreeView>
#include <QTreeWidget>
#include <QVBoxLayout>
#include <QWebChannel>
#include <QWidget>
#include <QWindow>
#include <QSettings>

// md4qt include.
#define MD4QT_QT_SUPPORT
#include <md4qt/algo.h>
#include <md4qt/html.h>
#include <md4qt/parser.h>

// shared include.
#include "license_dialog.hpp"
#include "folder_chooser.hpp"

namespace MdEditor
{

//
// HtmlVisitor
//

//! Converter into HTML.
class HtmlVisitor : public MD::details::HtmlVisitor<MD::QStringTrait>
{
protected:
    //! Prepare text to insert into HTML content.
    QString prepareTextForHtml(const QString &t) override
    {
        auto tmp = MD::details::HtmlVisitor<MD::QStringTrait>::prepareTextForHtml(t);
        tmp.replace(QLatin1Char('$'), QStringLiteral("<span>$</span>"));

        return tmp;
    }
};

//
// TabBar
//

//! Tab bar for tabs.
class TabBar : public QTabBar
{
    Q_OBJECT

signals:
    void activated();

public:
    explicit TabBar(QWidget *parent)
        : QTabBar(parent)
    {
    }

    ~TabBar() override = default;

protected:
    bool event(QEvent *e) override
    {
        if (e->type() == QEvent::Shortcut) {
            const auto res = QTabBar::event(e);

            if (res) {
                emit activated();
            }

            return res;
        } else {
            return QTabBar::event(e);
        }
    }
}; // class TabBar

//
// TabWidget
//

//! Tabs in main window.
class TabWidget : public QTabWidget
{
    Q_OBJECT

signals:
    //! Return key was pressed on tab or tab was activated by shortcut.
    void activated();
    //! Tab removed.
    void removed();

public:
    explicit TabWidget(QWidget *parent)
        : QTabWidget(parent)
    {
        auto bar = new TabBar(this);

        setTabBar(bar);

        connect(bar, &TabBar::activated, this, &TabWidget::activated);
    }

    ~TabWidget() override = default;

protected:
    void keyPressEvent(QKeyEvent *e) override
    {
        if (e->key() == Qt::Key_Return) {
            emit activated();
        }

        QTabWidget::keyPressEvent(e);
    }

    void tabRemoved(int index) override
    {
        QTabWidget::tabRemoved(index);

        emit removed();
    }
}; // class TabWidget

//
// TocTreeView
//

//! Tree view for ToC.
class TocTreeView : public QTreeView
{
    Q_OBJECT

signals:
    void scrollWebViewToRequested(const QString &id);

public:
    explicit TocTreeView(QWidget *parent)
        : QTreeView(parent)
    {
    }

    ~TocTreeView() override = default;

protected:
    void keyPressEvent(QKeyEvent *e) override
    {
        QTreeView::keyPressEvent(e);

        if (e->key() == Qt::Key_Return) {
            e->accept();
        }
    }

    void contextMenuEvent(QContextMenuEvent *e) override
    {
        QMenu menu;

        const auto id = static_cast<TocData *>(static_cast<QSortFilterProxyModel*>(model())->
                                               mapToSource(indexAt(e->pos())).internalPointer())->m_id;

        menu.addAction(tr("Scroll Web Preview To"), [id, this](){ emit this->scrollWebViewToRequested(id); });

        menu.exec(e->globalPos());

        e->accept();
    }
}; // class TocTreeView

//
// WorkingDirectoryWidget
//

class WorkingDirectoryWidget : public QWidget
{
    Q_OBJECT

signals:
    void workingDirectoryChanged(const QString &);

public:
    WorkingDirectoryWidget(QWidget *parent)
        : QWidget(parent)
        , m_label(new QLabel(this))
        , m_btn(new QToolButton(this))
        , m_useWorkingDir(new QCheckBox(this))
    {
        auto layout = new QHBoxLayout(this);
        layout->setContentsMargins(0, 0, 0, 0);
        layout->addWidget(m_label);
        layout->addWidget(m_btn);
        layout->addWidget(m_useWorkingDir);

        m_label->setText(QStringLiteral("T"));
        const auto h = m_label->sizeHint().height();
        m_label->setText({});

        m_btn->setText(tr("..."));
        m_btn->setToolTip(tr("Change Working Directory."));
        m_btn->setMinimumHeight(h);
        m_btn->setMaximumHeight(h);

        m_useWorkingDir->setToolTip(tr("Use Working Directory."));
        m_useWorkingDir->setChecked(false);

        m_folderChooser = new MdShared::FolderChooser(this);
        m_folderChooser->setPopup();
        m_folderChooser->hide();

        connect(m_btn, &QToolButton::clicked, this, &WorkingDirectoryWidget::onChangeButtonClicked);
        connect(m_folderChooser, &MdShared::FolderChooser::pathSelected, this, &WorkingDirectoryWidget::onPathChanged);
        connect(m_useWorkingDir, &QCheckBox::checkStateChanged, this, &WorkingDirectoryWidget::onUseWorkingDirChanged);
    }

    ~WorkingDirectoryWidget() override = default;

    const QString &workingDirectory() const
    {
        return (isRelative() ? m_fullPath : m_currentPath);
    }

    bool isRelative() const
    {
        return !m_useWorkingDir->isChecked();
    }

public slots:
    void setWorkingDirectory(const QString &wd, bool notify = true)
    {
        m_label->setText(tr("<b>Working Directory:</b> ") + wd);
        m_currentPath = wd;

        if (notify) {
            m_folderChooser->setPath(wd);
            m_fullPath = wd;
        }

        show();
    }

private slots:
    void onChangeButtonClicked()
    {
        const auto p1 = mapToGlobal(m_btn->pos());
        auto s = static_cast<QWidget*>(parent());
        const auto p2 = s->mapToGlobal(QPoint{0, 0});

        m_folderChooser->move(p1.x() -
                (layoutDirection() == Qt::LeftToRight ? m_folderChooser->sizeHint().width() : 0),
            p2.y() - m_folderChooser->sizeHint().height() - 3);
        m_folderChooser->show();
    }

    void onPathChanged(const QString &path)
    {
        if (!path.isEmpty() && m_currentPath != path) {
            disconnect(m_useWorkingDir, &QCheckBox::checkStateChanged,
                       this, &WorkingDirectoryWidget::onUseWorkingDirChanged);
            m_useWorkingDir->setChecked(true);
            connect(m_useWorkingDir, &QCheckBox::checkStateChanged,
                    this, &WorkingDirectoryWidget::onUseWorkingDirChanged);

            setWorkingDirectory(path, false);

            emit workingDirectoryChanged(path);
        }
    }

    void onUseWorkingDirChanged(Qt::CheckState)
    {
        emit workingDirectoryChanged(m_currentPath);
    }

private:
    QLabel *m_label = nullptr;
    QToolButton *m_btn = nullptr;
    QCheckBox *m_useWorkingDir = nullptr;
    MdShared::FolderChooser *m_folderChooser = nullptr;
    QString m_currentPath;
    QString m_fullPath;
}; // class WorkingDirectoryWidget

//
// MainWindowPrivate
//

struct MainWindowPrivate {
    MainWindowPrivate(MainWindow *parent)
        : m_q(parent)
    {
    }

    void notifyTocTree(QAbstractItemModel *model, WordWrapItemDelegate *delegate, const QModelIndex &parent)
    {
        for (int i = 0; i < model->rowCount(parent); ++i) {
            const auto idx = model->index(i, 0, parent);

            emit delegate->sizeHintChanged(idx);

            if (model->rowCount(idx) > 0) {
                notifyTocTree(model, delegate, idx);
            }
        }
    }

    void initUi()
    {
        {
            QFile wrapperHtmlFile(":/res/index.html");

            if (!wrapperHtmlFile.open(QFile::ReadOnly | QFile::Text)) {
                m_htmlContent = MainWindow::tr("Error loading res/index.html");
            } else {
                QTextStream stream(&wrapperHtmlFile);
                m_htmlContent = stream.readAll();
                wrapperHtmlFile.close();
            }
        }

        auto w = new QWidget(m_q);
        auto l = new QVBoxLayout(w);
        l->setContentsMargins(0, 0, 0, 0);
        l->setSpacing(0);

        m_splitter = new QSplitter(w);
        m_splitter->setOrientation(Qt::Orientation::Horizontal);

        // Sidebar.
        m_sidebarPanel = new QWidget(m_splitter);
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

        m_fileTree = new QTreeWidget(m_tabs);
        m_fileTree->setHeaderHidden(true);
        m_fileTree->hide();

        QObject::connect(m_tocTree->header(), &QHeaderView::sectionResized, [this](int, int, int) {
            notifyTocTree(this->m_filterTocModel, this->m_delegate, QModelIndex());
        });
        QObject::connect(m_tocTree, &TocTreeView::scrollWebViewToRequested, m_q, &MainWindow::onScrollWebViewTo);

        QObject::connect(m_tocFilterLine, &QLineEdit::textChanged, [this](const QString &text) {
            this->m_filterTocModel->setFilterFixedString(text);
        });

        m_tocPanel->hide();

        // Editor.
        auto editorPanel = new QWidget(m_splitter);
        auto v = new QVBoxLayout(editorPanel);
        v->setContentsMargins(0, 0, 0, 0);
        v->setSpacing(0);
        m_editor = new Editor(editorPanel);
        m_find = new Find(m_q, m_editor, editorPanel);
        m_editor->setFindWidget(m_find);
        m_gotoline = new GoToLine(m_q, m_editor, editorPanel);
        v->addWidget(m_editor);
        v->addWidget(m_gotoline);
        v->addWidget(m_find);

        // Preview.
        auto previewPanel = new QWidget(m_splitter);
        auto v1 = new QVBoxLayout(previewPanel);
        v1->setContentsMargins(0, 0, 0, 0);
        v1->setSpacing(0);
        m_preview = new WebView(previewPanel);
        m_findWeb = new FindWeb(m_q, m_preview, previewPanel);
        v1->addWidget(m_preview);
        v1->addWidget(m_findWeb);

        m_find->hide();
        m_gotoline->hide();
        m_findWeb->hide();

        m_splitter->addWidget(m_sidebarPanel);
        m_splitter->addWidget(editorPanel);
        m_splitter->addWidget(previewPanel);

        m_splitterCursor = m_splitter->handle(1)->cursor();
        this->m_splitter->handle(1)->setCursor(Qt::ArrowCursor);

        l->addWidget(m_splitter);

        m_q->setCentralWidget(w);

        m_page = new PreviewPage(m_preview);
        m_preview->setPage(m_page);

        m_html = new HtmlDocument(m_q);

        auto channel = new QWebChannel(m_q);
        channel->registerObject(QStringLiteral("content"), m_html);
        m_page->setWebChannel(channel);

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
        m_newAction = fileMenu->addAction(QIcon::fromTheme(QStringLiteral("document-new"), QIcon(QStringLiteral(":/res/img/document-new.png"))),
                                        MainWindow::tr("New"),
                                        MainWindow::tr("Ctrl+N"),
                                        m_q,
                                        &MainWindow::onFileNew);
        m_openAction = fileMenu->addAction(QIcon::fromTheme(QStringLiteral("document-open"), QIcon(QStringLiteral(":/res/img/document-open.png"))),
                                         MainWindow::tr("Open"),
                                         MainWindow::tr("Ctrl+O"),
                                         m_q,
                                         &MainWindow::onFileOpen);
        fileMenu->addSeparator();
        m_saveAction = fileMenu->addAction(QIcon::fromTheme(QStringLiteral("document-save"), QIcon(QStringLiteral(":/res/img/document-save.png"))),
                                         MainWindow::tr("Save"),
                                         MainWindow::tr("Ctrl+S"),
                                         m_q,
                                         &MainWindow::onFileSave);
        m_saveAsAction = fileMenu->addAction(QIcon::fromTheme(QStringLiteral("document-save-as"), QIcon(QStringLiteral(":/res/img/document-save-as.png"))),
                                           MainWindow::tr("Save As"),
                                           m_q,
                                           &MainWindow::onFileSaveAs);
        fileMenu->addSeparator();
        m_loadAllAction = fileMenu->addAction(MainWindow::tr("Load All Linked Files..."), MainWindow::tr("Ctrl+R"), m_q, &MainWindow::loadAllLinkedFiles);
        m_loadAllAction->setEnabled(false);

        if (!mdPdfExeFiles.isEmpty() && !starterExeFiles.isEmpty()) {
            fileMenu->addSeparator();

            m_convertToPdfAction = fileMenu->addAction(MainWindow::tr("Convert To PDF..."), m_q, &MainWindow::onConvertToPdf);

            m_convertToPdfAction->setEnabled(false);
        }

        fileMenu->addSeparator();
        fileMenu->addAction(QIcon::fromTheme(QStringLiteral("application-exit"), QIcon(QStringLiteral(":/res/img/application-exit.png"))),
                            MainWindow::tr("Quit"),
                            MainWindow::tr("Ctrl+Q"),
                            m_q,
                            &QWidget::close);

        m_editMenuAction = m_q->menuBar()->addAction(MainWindow::tr("&Edit"));
        m_toggleFindAction =
            new QAction(QIcon::fromTheme(QStringLiteral("edit-find"), QIcon(QStringLiteral(":/res/img/edit-find.png"))), MainWindow::tr("Find/Replace"), m_q);
        m_toggleFindAction->setShortcut(MainWindow::tr("Ctrl+F"));
        m_q->addAction(m_toggleFindAction);

        m_toggleFindWebAction =
            new QAction(QIcon::fromTheme(QStringLiteral("edit-find"), QIcon(QStringLiteral(":/res/img/edit-find.png"))), MainWindow::tr("Find In Preview"), m_q);
        m_toggleFindWebAction->setShortcut(MainWindow::tr("Ctrl+W"));
        m_q->addAction(m_toggleFindWebAction);

        m_toggleGoToLineAction =
            new QAction(QIcon::fromTheme(QStringLiteral("go-next-use"), QIcon(QStringLiteral(":/res/img/go-next-use.png"))), MainWindow::tr("Go to Line"), m_q);
        m_toggleGoToLineAction->setShortcut(MainWindow::tr("Ctrl+L"));
        m_q->addAction(m_toggleGoToLineAction);

        m_addTOCAction = new QAction(MainWindow::tr("Add ToC"), m_q);

        m_nextMisspelled = new QAction(MainWindow::tr("Next Misspelled"), m_q);
        m_nextMisspelled->setShortcut(MainWindow::tr("F6"));
        m_nextMisspelled->setEnabled(false);
        m_q->addAction(m_nextMisspelled);

        auto formatMenu = m_q->menuBar()->addMenu(MainWindow::tr("F&ormat"));

        m_tabAction = new QAction(MainWindow::tr("Indent"), formatMenu);
        m_tabAction->setShortcut(MainWindow::tr("Tab"));
        m_tabAction->setShortcutContext(Qt::WidgetShortcut);
        QObject::connect(m_tabAction, &QAction::triggered, [this]() {
            qApp->postEvent(this->m_editor, new QKeyEvent(QEvent::KeyPress, Qt::Key_Tab, Qt::NoModifier));
        });
        formatMenu->addAction(m_tabAction);

        m_backtabAction = new QAction(MainWindow::tr("Unindent"), formatMenu);
        m_backtabAction->setShortcut(MainWindow::tr("Shift+Tab"));
        m_backtabAction->setShortcutContext(Qt::WidgetShortcut);
        QObject::connect(m_backtabAction, &QAction::triggered, [this]() {
            qApp->postEvent(this->m_editor, new QKeyEvent(QEvent::KeyPress, Qt::Key_Backtab, Qt::NoModifier));
        });
        formatMenu->addAction(m_backtabAction);

        auto viewMenu = m_q->menuBar()->addMenu(MainWindow::tr("&View"));
        m_viewAction = new QAction(QIcon::fromTheme(QStringLiteral("view-preview"), QIcon(QStringLiteral(":/res/img/view-preview.png"))),
                                 MainWindow::tr("Toggle Preview Mode"));
        m_viewAction->setShortcut(MainWindow::tr("Ctrl+P"));
        m_viewAction->setCheckable(true);
        m_viewAction->setChecked(false);
        viewMenu->addAction(m_viewAction);
        m_livePreviewAction = new QAction(QIcon::fromTheme(QStringLiteral("layer-visible-on"), QIcon(QStringLiteral(":/res/img/layer-visible-on.png"))),
                                        MainWindow::tr("Live Preview"));
        m_livePreviewAction->setShortcut(MainWindow::tr("Ctrl+Alt+P"));
        m_livePreviewAction->setCheckable(true);
        m_livePreviewAction->setChecked(true);
        viewMenu->addAction(m_livePreviewAction);

        m_settingsMenu = m_q->menuBar()->addMenu(MainWindow::tr("&Settings"));
        auto toggleLineNumbersAction =
            new QAction(QIcon::fromTheme(QStringLiteral("view-table-of-contents-ltr"), QIcon(QStringLiteral(":/res/img/view-table-of-contents-ltr.png"))),
                        MainWindow::tr("Show Line Numbers"),
                        m_q);
        toggleLineNumbersAction->setCheckable(true);
        toggleLineNumbersAction->setShortcut(MainWindow::tr("Alt+L"));
        toggleLineNumbersAction->setChecked(true);
        m_settingsMenu->addAction(toggleLineNumbersAction);

        auto toggleUnprintableCharacters = new QAction(QIcon::fromTheme(QStringLiteral("character-set"), QIcon(QStringLiteral(":/res/img/character-set.png"))),
                                                       MainWindow::tr("Show Tabs/Spaces"),
                                                       m_q);
        toggleUnprintableCharacters->setCheckable(true);
        toggleUnprintableCharacters->setShortcut(MainWindow::tr("Alt+T"));
        toggleUnprintableCharacters->setChecked(true);
        m_settingsMenu->addAction(toggleUnprintableCharacters);

        m_settingsMenu->addSeparator();

        m_settingsMenu->addAction(QIcon::fromTheme(QStringLiteral("format-font-size-less"), QIcon(QStringLiteral(":/res/img/format-font-size-less.png"))),
                                MainWindow::tr("Decrease Font Size"),
                                MainWindow::tr("Ctrl+-"),
                                m_q,
                                &MainWindow::onLessFontSize);
        m_settingsMenu->addAction(QIcon::fromTheme(QStringLiteral("format-font-size-more"), QIcon(QStringLiteral(":/res/img/format-font-size-more.png"))),
                                MainWindow::tr("Increase Font Size"),
                                MainWindow::tr("Ctrl+="),
                                m_q,
                                &MainWindow::onMoreFontSize);

        m_settingsMenu->addSeparator();

        m_settingsMenu->addAction(QIcon::fromTheme(QStringLiteral("preferences-desktop-font"), QIcon(QStringLiteral(":/res/img/preferences-desktop-font.png"))),
                                MainWindow::tr("Font..."),
                                m_q,
                                &MainWindow::onChooseFont);

        m_settingsMenu->addAction(QIcon::fromTheme(QStringLiteral("fill-color"), QIcon(QStringLiteral(":/res/img/fill-color.png"))),
                                MainWindow::tr("Colors..."),
                                m_q,
                                &MainWindow::onChangeColors);

        m_settingsMenu->addSeparator();

        m_settingsMenu->addAction(QIcon::fromTheme(QStringLiteral("configure"), QIcon(QStringLiteral(":/res/img/configure.png"))),
                                MainWindow::tr("Settings"),
                                m_q,
                                &MainWindow::onSettings);

        auto helpMenu = m_q->menuBar()->addMenu(MainWindow::tr("&Help"));
        helpMenu->addAction(QIcon(QStringLiteral(":/icon/icon_24x24.png")), MainWindow::tr("About"), m_q, &MainWindow::onAbout);
        helpMenu->addAction(QIcon(QStringLiteral(":/img/Qt-logo-neon-transparent.png")), MainWindow::tr("About Qt"), m_q, &MainWindow::onAboutQt);
        helpMenu->addAction(QIcon::fromTheme(QStringLiteral("bookmarks-organize"), QIcon(QStringLiteral(":/res/img/bookmarks-organize.png"))),
                            MainWindow::tr("Licenses"),
                            m_q,
                            &MainWindow::onShowLicenses);

        m_cursorPosLabel = new QLabel(m_q);
        m_workingDirectoryWidget = new WorkingDirectoryWidget(m_q);
        m_q->statusBar()->addPermanentWidget(m_workingDirectoryWidget);
        m_q->statusBar()->addPermanentWidget(m_cursorPosLabel);
        m_workingDirectoryWidget->hide();

        QObject::connect(m_workingDirectoryWidget, &WorkingDirectoryWidget::workingDirectoryChanged,
                         m_q, &MainWindow::onWorkingDirectoryChange);
        QObject::connect(m_editor->document(), &QTextDocument::modificationChanged, m_saveAction, &QAction::setEnabled);
        QObject::connect(m_editor->document(), &QTextDocument::modificationChanged, m_q, &MainWindow::setWindowModified);
        QObject::connect(m_editor, &Editor::ready, m_q, &MainWindow::onTextChanged);
        QObject::connect(m_editor, &Editor::misspelled, m_q, &MainWindow::onMisspelledFount);
        QObject::connect(m_editor, &Editor::lineHovered, m_q, &MainWindow::onLineHovered);
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

        m_editor->setFocus();
        m_preview->setFocusPolicy(Qt::ClickFocus);
        m_editor->applyColors(m_mdColors);

        m_q->setTabOrder(m_gotoline->line(), m_find->editLine());
        m_q->setTabOrder(m_find->editLine(), m_find->replaceLine());
        m_q->setTabOrder(m_find->replaceLine(), m_findWeb->line());

        m_q->onFileNew();
    }

    void handleCurrentTab()
    {
        if (m_tabs->currentIndex() == 0) {
            initMarkdownMenu();
        }

        m_currentTab = m_tabs->currentIndex();
    }

    StringDataVec paragraphToMenuText(MD::Paragraph<MD::QStringTrait> *p, bool skipRtl = false)
    {
        StringDataVec res;
        bool rtl = false;
        bool first = !skipRtl;

        for (auto it = p->items().cbegin(), last = p->items().cend(); it != last; ++it) {
            switch ((*it)->type()) {
            case MD::ItemType::Text: {
                auto t = static_cast<MD::Text<MD::QStringTrait> *>(it->get());

                if (first) {
                    first = false;
                    rtl = t->text().isRightToLeft();
                }

                res.append({t->text(), false, rtl});
            } break;

            case MD::ItemType::Code: {
                auto c = static_cast<MD::Code<MD::QStringTrait> *>(it->get());

                if (first) {
                    first = false;
                    rtl = c->text().isRightToLeft();
                }

                res.append({c->text(), true, rtl});
            } break;

            case MD::ItemType::Link: {
                auto l = static_cast<MD::Link<MD::QStringTrait> *>(it->get());

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

    void initMarkdownMenu()
    {
        m_initMarkdownMenuRequested = true;

        if (m_tabsVisible && m_tocDoc != m_editor->currentDoc()) {
            m_initMarkdownMenuRequested = false;
            m_tocModel->clear();

            m_tocDoc = m_editor->currentDoc();

            std::vector<QModelIndex> current;

            MD::forEach<MD::QStringTrait>(
                {MD::ItemType::Heading},
                m_tocDoc,
                [this, &current](MD::Item<MD::QStringTrait> *item) {
                    auto h = static_cast<MD::Heading<MD::QStringTrait> *>(item);

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
                                                                  h->startLine(), h->level(), h->label());
                                current.push_back(this->m_tocModel->index(this->m_tocModel->rowCount() - 1, 0));
                            } else {
                                this->m_tocModel->addChildItem(current.back(), this->paragraphToMenuText(h->text().get()),
                                                               h->startLine(), h->level(), h->label());
                                current.push_back(this->m_tocModel->index(this->m_tocModel->rowCount(current.back()) - 1,
                                                                          0, current.back()));
                            }
                        } else {
                            this->m_tocModel->addTopLevelItem(this->paragraphToMenuText(h->text().get()),
                                                              h->startLine(), h->level(), h->label());
                            current.push_back(this->m_tocModel->index(this->m_tocModel->rowCount() - 1, 0));
                        }
                    }
                },
                1);
        }
    }

    MainWindow *m_q = nullptr;
    Editor *m_editor = nullptr;
    WebView *m_preview = nullptr;
    PreviewPage *m_page = nullptr;
    QSplitter *m_splitter = nullptr;
    QWidget *m_sidebarPanel = nullptr;
    HtmlDocument *m_html = nullptr;
    WordWrapItemDelegate *m_delegate = nullptr;
    Find *m_find = nullptr;
    FindWeb *m_findWeb = nullptr;
    GoToLine *m_gotoline = nullptr;
    TabWidget *m_tabs = nullptr;
    QAction *m_newAction = nullptr;
    QAction *m_openAction = nullptr;
    QAction *m_saveAction = nullptr;
    QAction *m_saveAsAction = nullptr;
    QAction *m_toggleFindAction = nullptr;
    QAction *m_toggleFindWebAction = nullptr;
    QAction *m_toggleGoToLineAction = nullptr;
    QAction *m_editMenuAction = nullptr;
    QAction *m_loadAllAction = nullptr;
    QAction *m_viewAction = nullptr;
    QAction *m_livePreviewAction = nullptr;
    QAction *m_convertToPdfAction = nullptr;
    QAction *m_addTOCAction = nullptr;
    QAction *m_nextMisspelled = nullptr;
    QAction *m_tabAction = nullptr;
    QAction *m_backtabAction = nullptr;
    QMenu *m_standardEditMenu = nullptr;
    QMenu *m_settingsMenu = nullptr;
    QTreeWidget *m_fileTree = nullptr;
    QWidget *m_tocPanel = nullptr;
    WorkingDirectoryWidget *m_workingDirectoryWidget = nullptr;
    TocTreeView *m_tocTree = nullptr;
    TocModel *m_tocModel = nullptr;
    QSortFilterProxyModel *m_filterTocModel = nullptr;
    QLabel *m_cursorPosLabel = nullptr;
    QLineEdit *m_tocFilterLine = nullptr;
    bool m_sizesInitialized = false;
    bool m_shownAlready = false;
    bool m_loadAllFlag = false;
    bool m_previewMode = false;
    bool m_tabsVisible = false;
    bool m_livePreviewVisible = true;
    bool m_initMarkdownMenuRequested = false;
    QCursor m_splitterCursor;
    std::shared_ptr<MD::Document<MD::QStringTrait>> m_mdDoc;
    std::shared_ptr<MD::Document<MD::QStringTrait>> m_tocDoc;
    QString m_baseUrl;
    QString m_rootFilePath;
    QString m_mdPdfExe;
    QString m_launcherExe;
    QString m_htmlContent;
    Colors m_mdColors;
    bool m_spellingEnabled = false;
    int m_tabWidth = -1;
    int m_minTabWidth = -1;
    int m_currentTab = 0;
    int m_settingsWindowWidth = -1;
    int m_settingsWindowHeight = -1;
    bool m_settingsWindowMaximized = false;
}; // struct MainWindowPrivate

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

void MainWindow::onTabActivated()
{
    if (!m_d->m_tabsVisible || m_d->m_currentTab == m_d->m_tabs->currentIndex()) {
        showOrHideTabs();
    }

    m_d->handleCurrentTab();
}

void MainWindow::onWorkingDirectoryChange(const QString &)
{
    m_d->m_baseUrl = QString("file:%1/").arg(QString(QUrl::toPercentEncoding(
        m_d->m_workingDirectoryWidget->workingDirectory(), "/\\:", {})));
    m_d->m_page->setHtml(htmlContent(), m_d->m_baseUrl);
    m_d->m_editor->onWorkingDirectoryChange(m_d->m_workingDirectoryWidget->workingDirectory(),
                                            !m_d->m_workingDirectoryWidget->isRelative());

    readAllLinked();

    m_d->initMarkdownMenu();
}

void MainWindow::onScrollWebViewTo(const QString &id)
{
    m_d->m_page->runJavaScript(QStringLiteral("scrollToHeading('%1')").arg(id));
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

        auto w = (centralWidget()->width() - m_d->m_minTabWidth) / 2;

        if (!m_d->m_previewMode) {
            m_d->m_splitter->setSizes({m_d->m_minTabWidth, w, w});
        } else {
            m_d->m_splitter->setSizes({0, 0, centralWidget()->width()});
            m_d->m_splitter->handle(1)->setCursor(Qt::ArrowCursor);
            m_d->m_splitter->handle(2)->setCursor(Qt::ArrowCursor);
        }
    }

    e->accept();
}

void MainWindow::showEvent(QShowEvent *e)
{
    if (!m_d->m_shownAlready) {
        m_d->m_shownAlready = true;

        readCfg();
    }

    e->accept();
}

void MainWindow::openFile(const QString &path)
{
    QFile f(path);
    if (!f.open(QIODevice::ReadOnly)) {
        QMessageBox::warning(this, windowTitle(), tr("Could not open file %1: %2").arg(
                                 QDir::toNativeSeparators(path), f.errorString()));
        return;
    }

    m_d->m_editor->setDocName(path);
    const auto wd = QFileInfo(path).absoluteDir().absolutePath();
    m_d->m_workingDirectoryWidget->setWorkingDirectory(wd);
    onWorkingDirectoryChange(wd);

    m_d->m_editor->setText(f.readAll());
    f.close();
    updateWindowTitle();
    m_d->m_editor->setFocus();
    m_d->m_editor->document()->clearUndoRedoStacks();
    onCursorPositionChanged();
    m_d->m_loadAllAction->setEnabled(true);
    m_d->m_rootFilePath = path;

    if (m_d->m_convertToPdfAction) {
        m_d->m_convertToPdfAction->setEnabled(true);
    }

    closeAllLinkedFiles();
    updateLoadAllLinkedFilesMenuText();
    m_d->initMarkdownMenu();

    setWindowModified(false);
}

void MainWindow::openInPreviewMode(bool loadAllLinked)
{
    m_d->m_viewAction->setChecked(true);

    if (loadAllLinked) {
        loadAllLinkedFiles();
    }
}

bool MainWindow::isModified() const
{
    return m_d->m_editor->document()->isModified();
}

void MainWindow::onFileNew()
{
    if (isModified()) {
        QMessageBox::StandardButton button = QMessageBox::question(this,
                                                                   windowTitle(),
                                                                   tr("You have unsaved changes. Do you want to create a new document anyway?"),
                                                                   QMessageBox::Yes | QMessageBox::No,
                                                                   QMessageBox::No);

        if (button != QMessageBox::Yes) {
            return;
        }
    }

    m_d->m_editor->setDocName(QStringLiteral("default.md"));
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

void MainWindow::onFileOpen()
{
    if (isModified()) {
        QMessageBox::StandardButton button = QMessageBox::question(this,
                                                                   windowTitle(),
                                                                   tr("You have unsaved changes. Do you want to open a new document anyway?"),
                                                                   QMessageBox::Yes | QMessageBox::No,
                                                                   QMessageBox::No);

        if (button != QMessageBox::Yes) {
            return;
        }
    }

    const auto folder = m_d->m_editor->docName() == QStringLiteral("default.md") ?
                QStandardPaths::standardLocations(QStandardPaths::HomeLocation).first()
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
    if (m_d->m_editor->docName() == QStringLiteral("default.md")) {
        onFileSaveAs();
        return;
    }

    QFile f(m_d->m_editor->docName());
    if (!f.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::warning(this, windowTitle(), tr("Could not write to file %1: %2").arg(
                                 QDir::toNativeSeparators(m_d->m_editor->docName()), f.errorString()));

        return;
    }

    QTextStream str(&f);
    str << m_d->m_editor->toPlainText();
    f.close();

    m_d->m_editor->document()->setModified(false);

    updateWindowTitle();

    readAllLinked();

    m_d->initMarkdownMenu();
}

void MainWindow::onFileSaveAs()
{
    QFileDialog dialog(this, tr("Save Markdown File"), QStandardPaths::standardLocations(QStandardPaths::HomeLocation).first());
    dialog.setMimeTypeFilters({"text/markdown"});
    dialog.setAcceptMode(QFileDialog::AcceptSave);
    dialog.setDefaultSuffix("md");

    if (dialog.exec() != QDialog::Accepted) {
        return;
    }

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
}

void MainWindow::closeEvent(QCloseEvent *e)
{
    if (isModified()) {
        QMessageBox::StandardButton button = QMessageBox::question(this,
                                                                   windowTitle(),
                                                                   tr("You have unsaved changes. Do you want to exit anyway?"),
                                                                   QMessageBox::Yes | QMessageBox::No,
                                                                   QMessageBox::No);

        if (button != QMessageBox::Yes) {
            e->ignore();
        }
    }

    saveCfg();
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
            m_d->m_html->setText(MD::toHtml<MD::QStringTrait, HtmlVisitor>(
                                     m_d->m_mdDoc, false, QStringLiteral("qrc:/res/img/go-jump.png")));
        }
    }

    const auto lineNumber = m_d->m_editor->textCursor().block().blockNumber();
    const auto lineLength = m_d->m_editor->textCursor().block().length();

    const auto items = m_d->m_editor->syntaxHighlighter().findFirstInCache({0, lineNumber, lineLength, lineNumber});

    if ((!items.empty() && items[0]->type() == MD::ItemType::Heading) || m_d->m_initMarkdownMenuRequested) {
        m_d->initMarkdownMenu();
    }
}

void MainWindow::onAbout()
{
    QMessageBox::about(this,
                       tr("About Markdown Editor"),
                       tr("Markdown Editor.\n\n"
                          "Version %1\n\n"
                          "md4qt version %2\n\n"
                          "Author - Igor Mironchik (igor.mironchik at gmail dot com).\n\n"
                          "Copyright (c) 2023-2025 Igor Mironchik.\n\n"
                          "Licensed under GNU GPL 3.0.")
                           .arg(c_version, c_md4qtVersion));
}

void MainWindow::onAboutQt()
{
    QMessageBox::aboutQt(this);
}

namespace /* anonymous */
{

inline QString itemType(MD::ItemType t, bool alone)
{
    switch (t) {
    case MD::ItemType::Heading:
        return MainWindow::tr("Heading");
    case MD::ItemType::Text:
        return MainWindow::tr("Text");
    case MD::ItemType::Paragraph:
        return MainWindow::tr("Paragraph");
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

    default:
        return {};
    }
}

} /* namespace anonymous */

void MainWindow::onLineHovered(int lineNumber, const QPoint &pos)
{
    const auto items =
        m_d->m_editor->syntaxHighlighter().findFirstInCache(
                {0, lineNumber, m_d->m_editor->document()->findBlockByLineNumber(lineNumber).length(), lineNumber});

    if (!items.empty()) {
        if ((items.front()->type() != MD::ItemType::List && items.front()->type() != MD::ItemType::Footnote)
            || items.size() == 1) {
            QToolTip::showText(pos, itemType(items.front()->type(), items.size() == 1));
        } else {
            QToolTip::showText(pos, tr("%1 in %2").arg(itemType(items.at(1)->type(), items.size() == 1),
                                                       itemType(items.front()->type(), items.size() == 1)));
        }
    }
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

        saveCfg();
    }
}

void MainWindow::saveCfg() const
{
    const auto f = m_d->m_editor->font();

    QSettings s;

    s.beginGroup(QStringLiteral("ui"));

    s.beginGroup(QStringLiteral("font"));
    s.setValue(QStringLiteral("family"), f.family());
    s.setValue(QStringLiteral("size"), f.pointSize());
    s.endGroup();

    s.setValue(QStringLiteral("linkColor"), m_d->m_mdColors.m_linkColor);
    s.setValue(QStringLiteral("textColor"), m_d->m_mdColors.m_textColor);
    s.setValue(QStringLiteral("inlineColor"), m_d->m_mdColors.m_inlineColor);
    s.setValue(QStringLiteral("htmlColor"), m_d->m_mdColors.m_htmlColor);
    s.setValue(QStringLiteral("tableColor"), m_d->m_mdColors.m_tableColor);
    s.setValue(QStringLiteral("codeColor"), m_d->m_mdColors.m_codeColor);
    s.setValue(QStringLiteral("mathColor"), m_d->m_mdColors.m_mathColor);
    s.setValue(QStringLiteral("referenceColor"), m_d->m_mdColors.m_referenceColor);
    s.setValue(QStringLiteral("specialColor"), m_d->m_mdColors.m_specialColor);
    s.setValue(QStringLiteral("enableMargin"), m_d->m_editor->margins().m_enable);
    s.setValue(QStringLiteral("margin"), m_d->m_editor->margins().m_length);
    s.setValue(QStringLiteral("enableColors"), m_d->m_mdColors.m_enabled);
    s.setValue(QStringLiteral("sidebarWidth"), m_d->m_tabWidth);

    s.setValue(QStringLiteral("findCaseSensitive"), m_d->m_find->isCaseSensitive());
    s.setValue(QStringLiteral("findWholeWord"), m_d->m_find->isWholeWord());
    s.endGroup();

    s.beginGroup(QStringLiteral("window"));

    s.setValue(QStringLiteral("width"), width());
    s.setValue(QStringLiteral("height"), height());
    s.setValue(QStringLiteral("x"), windowHandle()->x());
    s.setValue(QStringLiteral("y"), windowHandle()->y());
    s.setValue(QStringLiteral("maximized"), isMaximized());

    s.endGroup();

    s.beginGroup(QStringLiteral("settings_window"));
    s.setValue(QStringLiteral("width"), m_d->m_settingsWindowWidth);
    s.setValue(QStringLiteral("height"), m_d->m_settingsWindowHeight);
    s.setValue(QStringLiteral("maximized"), m_d->m_settingsWindowMaximized);
    s.endGroup();

    s.beginGroup(QStringLiteral("spelling"));
    s.setValue(QStringLiteral("enabled"), m_d->m_spellingEnabled);
    s.endGroup();

    Sonnet::Settings sonnet;
    sonnet.save();
}

void MainWindow::readCfg()
{
    QSettings s;

    s.beginGroup(QStringLiteral("ui"));

    s.beginGroup(QStringLiteral("font"));

    {
        const auto fontName = s.value(QStringLiteral("family")).toString();

        if (!fontName.isEmpty()) {
            auto fs = s.value(QStringLiteral("size")).toInt();

            const QFont f(fontName, fs);

            m_d->m_editor->applyFont(f);
        }
    }

    s.endGroup();

    const auto isFindCaseSensitive = s.value(QStringLiteral("findCaseSensitive"), true).toBool();
    m_d->m_find->setCaseSensitive(isFindCaseSensitive);

    const auto isFindWholeWord = s.value(QStringLiteral("findWholeWord"), true).toBool();
    m_d->m_find->setWholeWord(isFindWholeWord);

    const auto linkColor = s.value(QStringLiteral("linkColor"), m_d->m_mdColors.m_linkColor).value<QColor>();
    if (linkColor.isValid()) {
        m_d->m_mdColors.m_linkColor = linkColor;
    }

    const auto textColor = s.value(QStringLiteral("textColor"), m_d->m_mdColors.m_textColor).value<QColor>();
    if (textColor.isValid()) {
        m_d->m_mdColors.m_textColor = textColor;
    }

    const auto inlineColor = s.value(QStringLiteral("inlineColor"), m_d->m_mdColors.m_inlineColor).value<QColor>();
    if (inlineColor.isValid()) {
        m_d->m_mdColors.m_inlineColor = inlineColor;
    }

    const auto htmlColor = s.value(QStringLiteral("htmlColor"), m_d->m_mdColors.m_htmlColor).value<QColor>();
    if (htmlColor.isValid()) {
        m_d->m_mdColors.m_htmlColor = htmlColor;
    }

    const auto tableColor = s.value(QStringLiteral("tableColor"), m_d->m_mdColors.m_tableColor).value<QColor>();
    if (tableColor.isValid()) {
        m_d->m_mdColors.m_tableColor = tableColor;
    }

    const auto codeColor = s.value(QStringLiteral("codeColor"), m_d->m_mdColors.m_codeColor).value<QColor>();
    if (codeColor.isValid()) {
        m_d->m_mdColors.m_codeColor = codeColor;
    }

    const auto mathColor = s.value(QStringLiteral("mathColor"), m_d->m_mdColors.m_mathColor).value<QColor>();
    if (mathColor.isValid()) {
        m_d->m_mdColors.m_mathColor = mathColor;
    }

    const auto refColor = s.value(QStringLiteral("referenceColor"), m_d->m_mdColors.m_referenceColor).value<QColor>();
    if (refColor.isValid()) {
        m_d->m_mdColors.m_referenceColor = refColor;
    }

    const auto specialColor = s.value(QStringLiteral("specialColor"), m_d->m_mdColors.m_specialColor).value<QColor>();
    if (specialColor.isValid()) {
        m_d->m_mdColors.m_specialColor = specialColor;
    }

    const auto enableMargin = s.value(QStringLiteral("enableMargin")).toBool();
    m_d->m_editor->margins().m_enable = enableMargin;

    const auto margin = s.value(QStringLiteral("margin")).toInt();
    m_d->m_editor->margins().m_length = margin;

    const auto enableColors = s.value(QStringLiteral("enableColors")).toBool();
    m_d->m_mdColors.m_enabled = enableColors;

    m_d->m_editor->applyColors(m_d->m_mdColors);

    const auto sidebarWidth = s.value(QStringLiteral("sidebarWidth")).toInt();
    if (sidebarWidth > 0) {
        m_d->m_tabWidth = sidebarWidth;
    }

    s.endGroup();

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

    s.beginGroup(QStringLiteral("settings_window"));
    m_d->m_settingsWindowWidth = s.value(QStringLiteral("width"), -1).toInt();
    m_d->m_settingsWindowHeight = s.value(QStringLiteral("height"), -1).toInt();
    m_d->m_settingsWindowMaximized = s.value(QStringLiteral("maximized")).toBool();
    s.endGroup();

    s.beginGroup(QStringLiteral("spelling"));
    m_d->m_spellingEnabled = s.value(QStringLiteral("enabled")).toBool();
    s.endGroup();

    m_d->m_editor->enableSpellingCheck(m_d->m_spellingEnabled);
}

void MainWindow::onLessFontSize()
{
    auto f = m_d->m_editor->font();

    if (f.pointSize() > 5) {
        f.setPointSize(f.pointSize() - 1);

        m_d->m_editor->applyFont(f);

        saveCfg();
    }
}

void MainWindow::onMoreFontSize()
{
    auto f = m_d->m_editor->font();

    if (f.pointSize() < 66) {
        f.setPointSize(f.pointSize() + 1);

        m_d->m_editor->applyFont(f);

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

    m_d->m_cursorPosLabel->setText(tr("<b>Line:</b> %1, <b>Col:</b> %2").arg(c.block().blockNumber() + 1).arg(c.positionInBlock() + 1));
}

void MainWindow::onEditMenuActionTriggered(QAction *action)
{
    if (action != m_d->m_toggleFindAction && action != m_d->m_toggleGoToLineAction
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

void MainWindow::loadAllLinkedFiles()
{
    if (m_d->m_loadAllFlag) {
        m_d->m_loadAllFlag = false;

        m_d->m_tabs->removeTab(1);
        m_d->m_fileTree->hide();

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
    }

    readAllLinked();

    m_d->m_fileTree->clear();
    m_d->m_fileTree->show();
    m_d->m_tabs->addTab(m_d->m_fileTree, tr("&Navigation"));

    const auto rootFolder = m_d->m_workingDirectoryWidget->workingDirectory() + QStringLiteral("/");

    Node root;

    if (m_d->m_mdDoc) {
        for (auto it = m_d->m_mdDoc->items().cbegin(), last = m_d->m_mdDoc->items().cend(); it != last; ++it) {
            if ((*it)->type() == MD::ItemType::Anchor) {
                const auto fullFileName = static_cast<MD::Anchor<MD::QStringTrait> *>(it->get())->label();

                const auto fileName = fullFileName.startsWith(rootFolder) ? fullFileName.sliced(rootFolder.size()) : fullFileName;

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
                            item->setIcon(0, QIcon::fromTheme(QStringLiteral("folder-yellow"), QIcon(":/res/img/folder-yellow.png")));
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

        connect(m_d->m_fileTree, &QTreeWidget::itemDoubleClicked, this, &MainWindow::onNavigationDoubleClicked);

        if (!m_d->m_previewMode) {
            QMessageBox::information(this,
                                     windowTitle(),
                                     tr("HTML preview is ready. Modifications in files will not update "
                                        "HTML preview till you save changes."));
        }
    } else {
        closeAllLinkedFiles();

        m_d->m_tabs->removeTab(1);
        m_d->m_fileTree->hide();

        QMessageBox::information(this, windowTitle(), tr("This document doesn't have linked documents."));
    }

    if (!m_d->m_previewMode) {
        m_d->m_editor->setFocus();
    }

    updateLoadAllLinkedFilesMenuText();
}

void MainWindow::closeAllLinkedFiles()
{
    m_d->m_loadAllFlag = false;

    m_d->m_editor->setFocus();

    onTextChanged();
}

void MainWindow::readAllLinked()
{
    if (m_d->m_loadAllFlag) {
        MD::Parser<MD::QStringTrait> parser;

        if (m_d->m_workingDirectoryWidget->isRelative()) {
            m_d->m_mdDoc = parser.parse(m_d->m_rootFilePath,
                                        true, {QStringLiteral("md"), QStringLiteral("mkd"), QStringLiteral("markdown")});
        } else {
            m_d->m_mdDoc = parser.parse(m_d->m_rootFilePath, m_d->m_workingDirectoryWidget->workingDirectory(),
                                        true, {QStringLiteral("md"), QStringLiteral("mkd"), QStringLiteral("markdown")});
        }

        if (m_d->m_livePreviewVisible) {
            m_d->m_html->setText(MD::toHtml<MD::QStringTrait, HtmlVisitor>(
                                     m_d->m_mdDoc, false, QStringLiteral("qrc:/res/img/go-jump.png")));
        }
    }
}

void MainWindow::onNavigationDoubleClicked(QTreeWidgetItem *item, int)
{
    const auto path = item->data(0, Qt::UserRole).toString();

    if (!path.isEmpty()) {
        if (isModified()) {
            QMessageBox::information(this, windowTitle(), tr("You have unsaved changes. Please save document first."));

            m_d->m_editor->setFocus();

            return;
        }

        QFile f(path);
        if (!f.open(QIODevice::ReadOnly)) {
            QMessageBox::warning(this, windowTitle(), tr("Could not open file %1: %2").arg(
                                     QDir::toNativeSeparators(path), f.errorString()));
            return;
        }

        m_d->m_editor->setDocName(path);
        m_d->m_editor->setText(f.readAll());
        f.close();

        updateWindowTitle();

        m_d->m_editor->document()->clearUndoRedoStacks();
        m_d->m_editor->setFocus();

        onCursorPositionChanged();

        m_d->initMarkdownMenu();
    }
}

void MainWindow::updateWindowTitle()
{
    setWindowTitle(tr("%1[*] - Markdown Editor%2").arg(
                       QFileInfo(m_d->m_editor->docName()).fileName(), m_d->m_previewMode ? tr(" [Preview Mode]") : QString()));
}

void MainWindow::updateLoadAllLinkedFilesMenuText()
{
    if (m_d->m_loadAllFlag) {
        m_d->m_loadAllAction->setText(tr("Show Only Current File..."));
        m_d->m_addTOCAction->setEnabled(false);
    } else {
        m_d->m_loadAllAction->setText(tr("Load All Linked Files..."));
        m_d->m_addTOCAction->setEnabled(true);
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
        m_d->m_sidebarPanel->hide();
        m_d->m_splitter->handle(1)->setCursor(Qt::ArrowCursor);
        m_d->m_splitter->handle(2)->setCursor(Qt::ArrowCursor);
        m_d->m_cursorPosLabel->hide();

        if (m_d->m_tabsVisible) {
            showOrHideTabs();
        }

        m_d->m_splitter->setSizes({0, 0, centralWidget()->width()});
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
        m_d->m_editor->setVisible(true);
        m_d->m_sidebarPanel->show();
        m_d->m_livePreviewAction->setVisible(true);
        m_d->m_livePreviewAction->setEnabled(true);

        if (m_d->m_livePreviewVisible) {
            m_d->m_splitter->handle(2)->setCursor(m_d->m_splitterCursor);
        }

        m_d->m_cursorPosLabel->show();

        const auto w = (centralWidget()->width() - m_d->m_minTabWidth) / 2;

        QList<int> s = {m_d->m_minTabWidth, w, w};

        if (!m_d->m_livePreviewVisible) {
            s[1] += s[2];
            s[2] = 0;
        }

        m_d->m_splitter->setSizes(s);

        m_d->m_editor->setFocus();
    }

    updateLoadAllLinkedFilesMenuText();

    updateWindowTitle();
}

void MainWindow::onToggleLivePreviewAction(bool checked)
{
    m_d->m_livePreviewVisible = checked;

    auto s = m_d->m_splitter->sizes();

    if (!m_d->m_livePreviewVisible) {
        s[1] += s[2];
        s[2] = 0;

        m_d->m_splitter->handle(2)->setCursor(Qt::ArrowCursor);
    } else {
        s[2] = s[1] / 2;
        s[1] = s[2];

        m_d->m_splitter->handle(2)->setCursor(m_d->m_splitterCursor);

        if (m_d->m_mdDoc) {
            m_d->m_html->setText(MD::toHtml<MD::QStringTrait, HtmlVisitor>(
                                     m_d->m_mdDoc, false, QStringLiteral("qrc:/res/img/go-jump.png")));
        } else {
            m_d->m_html->setText({});
        }
    }

    m_d->m_splitter->setSizes(s);
}

namespace /* anonymous */
{

inline QString paragraphToMD(MD::Paragraph<MD::QStringTrait> *p, QPlainTextEdit *editor)
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

inline QString simplifyLabel(const QString &label, const QString &fileName)
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

    MD::forEach<MD::QStringTrait>(
        {MD::ItemType::Anchor, MD::ItemType::Heading},
        m_d->m_editor->currentDoc(),
        [&](MD::Item<MD::QStringTrait> *item) {
            if (item->type() == MD::ItemType::Anchor) {
                auto a = static_cast<MD::Anchor<MD::QStringTrait> *>(item);
                fileName = a->label();
            } else if (item->type() == MD::ItemType::Heading) {
                auto h = static_cast<MD::Heading<MD::QStringTrait> *>(item);

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

    msg.addLicense(QStringLiteral("KaTeX"),
                   QStringLiteral("<p><b>KaTeX</b></p>\n\n"
                                  "<p>The MIT License (MIT)</p>\n"
                                  "\n"
                                  "<p>Copyright (c) 2013-2020 Khan Academy and other contributors</p>\n"
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

    msg.addLicense(QStringLiteral("github-markdown-css"),
                   QStringLiteral("<p><b>github-markdown-css</b></p>\n\n"
                                  "<p>The MIT License (MIT)</p>\n"
                                  "\n"
                                  "<p>Copyright (c) Sindre Sorhus "
                                  "<a href=\"mailto:sindresorhus@gmail.com\">&lt;sindresorhus@gmail.com&gt;</a> "
                                  "<a href=\"https://sindresorhus.com\">(https://sindresorhus.com)</a></p>\n"
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

    msg.addLicense(QStringLiteral("Highlight.js"),
                   QStringLiteral("<p><b>Highlight.js</b>\n\n</p>"
                                  "<p>BSD 3-Clause License</p>\n"
                                  "\n"
                                  "<p>Copyright (c) 2006, Ivan Sagalaev.\n"
                                  "All rights reserved.</p>\n"
                                  "\n"
                                  "<p>Redistribution and use in source and binary forms, with or without "
                                  "modification, are permitted provided that the following conditions are met:</p>\n"
                                  "\n"
                                  "<ul><li>Redistributions of source code must retain the above copyright notice, this "
                                  "list of conditions and the following disclaimer.</li>\n"
                                  "\n"
                                  "<li>Redistributions in binary form must reproduce the above copyright notice, "
                                  "this list of conditions and the following disclaimer in the documentation "
                                  "and/or other materials provided with the distribution.</li>\n"
                                  "\n"
                                  "<li>Neither the name of the copyright holder nor the names of its "
                                  "contributors may be used to endorse or promote products derived from "
                                  "this software without specific prior written permission.</li></ul>\n"
                                  "\n"
                                  "<p>THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS \"AS IS\" "
                                  "AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE "
                                  "IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE "
                                  "DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE "
                                  "FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL "
                                  "DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR "
                                  "SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER "
                                  "CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, "
                                  "OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE "
                                  "OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.</p>"));

    msg.addLicense(QStringLiteral("js-emoji"),
                   QStringLiteral("<p><b>js-emoji</b>\n\n</p>"
                                  "<p>The MIT License (MIT)</p>\n"
                                  "\n"
                                  "<p>Copyright (c) 2015 Cal Henderson</p>\n"
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

    msg.addLicense(QStringLiteral("hightlight-blockquote"),
                   QStringLiteral("<p><b>hightlight-blockquote</b>\n\n</p>"
                                  "<p>The MIT License (MIT)</p>\n"
                                  "\n"
                                  "<p>Copyright © 2024 Ivan Stanevich</p>\n"
                                  "<p>Copyright © 2024 Igor Mironchik</p>\n"
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

    msg.addLicense(QStringLiteral("Sonnet"),
                   QStringLiteral("<p><b>Sonnet</b>\n\n</p>"
                                  "<p>Copyright © 2004 Zack Rusin</p>\n"
                                  "<p>Copyright © 2009 Jakub Stachowski</p>\n"
                                  "\n"
                                  "<p>GNU LESSER GENERAL PUBLIC LICENSE</p>\n"
                                  "\n"
                                  "<p>Version 2.1, February 1999</p>\n"
                                  "\n"
                                  "<p>Copyright (C) 1991, 1999 Free Software Foundation, Inc.</p>\n"
                                  "\n"
                                  "<p>51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA</p>"
                                  "\n"
                                  "<p>Everyone is permitted to copy and distribute verbatim copies of this license "
                                  "document, but changing it is not allowed.</p>"
                                  "\n"
                                  "<p>[This is the first released version of the Lesser GPL. It also counts as the "
                                  "successor of the GNU Library Public License, version 2, hence the version "
                                  "number 2.1.]</p>\n"
                                  "\n"
                                  "<p>Preamble</p>\n"
                                  "\n"
                                  "<p>The licenses for most software are designed to take away your freedom to share "
                                  "and change it. By contrast, the GNU General Public Licenses are intended to "
                                  "guarantee your freedom to share and change free software--to make sure the "
                                  "software is free for all its users.</p>\n"
                                  "\n"
                                  "<p>This license, the Lesser General Public License, applies to some specially "
                                  "designated software packages--typically libraries--of the Free Software Foundation "
                                  "and other authors who decide to use it. You can use it too, but we suggest "
                                  "you first think carefully about whether this license or the ordinary General "
                                  "Public License is the better strategy to use in any particular case, based "
                                  "on the explanations below.</p>\n"
                                  "\n"
                                  "<p>When we speak of free software, we are referring to freedom of use, not price. "
                                  "Our General Public Licenses are designed to make sure that you have the freedom "
                                  "to distribute copies of free software (and charge for this service if you "
                                  "wish); that you receive source code or can get it if you want it; that you "
                                  "can change the software and use pieces of it in new free programs; and that "
                                  "you are informed that you can do these things.</p>\n"
                                  "\n"
                                  "<p>To protect your rights, we need to make restrictions that forbid distributors "
                                  "to deny you these rights or to ask you to surrender these rights. These restrictions "
                                  "translate to certain responsibilities for you if you distribute copies of "
                                  "the library or if you modify it.</p>\n"
                                  "\n"
                                  "<p>For example, if you distribute copies of the library, whether gratis or for "
                                  "a fee, you must give the recipients all the rights that we gave you. You must "
                                  "make sure that they, too, receive or can get the source code. If you link "
                                  "other code with the library, you must provide complete object files to the "
                                  "recipients, so that they can relink them with the library after making changes "
                                  "to the library and recompiling it. And you must show them these terms so they "
                                  "know their rights.</p>\n"
                                  "\n"
                                  "<p>We protect your rights with a two-step method: (1) we copyright the library, "
                                  "and (2) we offer you this license, which gives you legal permission to copy, "
                                  "distribute and/or modify the library.</p>\n"
                                  "\n"
                                  "<p>To protect each distributor, we want to make it very clear that there is no "
                                  "warranty for the free library. Also, if the library is modified by someone "
                                  "else and passed on, the recipients should know that what they have is not "
                                  "the original version, so that the original author's reputation will not be "
                                  "affected by problems that might be introduced by others.</p>\n"
                                  "\n"
                                  "<p>Finally, software patents pose a constant threat to the existence of any free "
                                  "program. We wish to make sure that a company cannot effectively restrict the "
                                  "users of a free program by obtaining a restrictive license from a patent holder. "
                                  "Therefore, we insist that any patent license obtained for a version of the "
                                  "library must be consistent with the full freedom of use specified in this "
                                  "license.</p>\n"
                                  "\n"
                                  "<p>Most GNU software, including some libraries, is covered by the ordinary GNU "
                                  "General Public License. This license, the GNU Lesser General Public License, "
                                  "applies to certain designated libraries, and is quite different from the ordinary "
                                  "General Public License. We use this license for certain libraries in order "
                                  "to permit linking those libraries into non-free programs.</p>\n"
                                  "\n"
                                  "<p>When a program is linked with a library, whether statically or using a shared "
                                  "library, the combination of the two is legally speaking a combined work, a "
                                  "derivative of the original library. The ordinary General Public License therefore "
                                  "permits such linking only if the entire combination fits its criteria of freedom. "
                                  "The Lesser General Public License permits more lax criteria for linking other "
                                  "code with the library.</p>\n"
                                  "\n"
                                  "<p>We call this license the \"Lesser\" General Public License because it does Less "
                                  "to protect the user's freedom than the ordinary General Public License. It "
                                  "also provides other free software developers Less of an advantage over competing "
                                  "non-free programs. These disadvantages are the reason we use the ordinary "
                                  "General Public License for many libraries. However, the Lesser license provides "
                                  "advantages in certain special circumstances.</p>\n"
                                  "\n"
                                  "<p>For example, on rare occasions, there may be a special need to encourage the "
                                  "widest possible use of a certain library, so that it becomes a de-facto standard. "
                                  "To achieve this, non-free programs must be allowed to use the library. A more "
                                  "frequent case is that a free library does the same job as widely used non-free "
                                  "libraries. In this case, there is little to gain by limiting the free library "
                                  "to free software only, so we use the Lesser General Public License.</p>\n"
                                  "\n"
                                  "<p>In other cases, permission to use a particular library in non-free programs "
                                  "enables a greater number of people to use a large body of free software. For "
                                  "example, permission to use the GNU C Library in non-free programs enables "
                                  "many more people to use the whole GNU operating system, as well as its variant, "
                                  "the GNU/Linux operating system.</p>\n"
                                  "\n"
                                  "<p>Although the Lesser General Public License is Less protective of the users' "
                                  "freedom, it does ensure that the user of a program that is linked with the "
                                  "Library has the freedom and the wherewithal to run that program using a modified "
                                  "version of the Library.</p>\n"
                                  "\n"
                                  "<p>The precise terms and conditions for copying, distribution and modification "
                                  "follow. Pay close attention to the difference between a \"work based on the "
                                  "library\" and a \"work that uses the library\". The former contains code derived "
                                  "from the library, whereas the latter must be combined with the library in "
                                  "order to run.</p>\n"
                                  "\n"
                                  "<p><b>TERMS AND CONDITIONS FOR COPYING, DISTRIBUTION AND MODIFICATION</b></p>\n"
                                  "\n"
                                  "<p><b>0.</b> This License Agreement applies to any software library or other program "
                                  "which contains a notice placed by the copyright holder or other authorized "
                                  "party saying it may be distributed under the terms of this Lesser General "
                                  "Public License (also called \"this License\"). Each licensee is addressed as "
                                  "\"you\".</p>\n"
                                  "\n"
                                  "<p>A \"library\" means a collection of software functions and/or data prepared "
                                  "so as to be conveniently linked with application programs (which use some "
                                  "of those functions and data) to form executables.</p>\n"
                                  "\n"
                                  "<p>The \"Library\", below, refers to any such software library or work which has "
                                  "been distributed under these terms. A \"work based on the Library\" means either "
                                  "the Library or any derivative work under copyright law: that is to say, a "
                                  "work containing the Library or a portion of it, either verbatim or with modifications "
                                  "and/or translated straightforwardly into another language. (Hereinafter, translation "
                                  "is included without limitation in the term \"modification\".)</p>\n"
                                  "\n"
                                  "<p>\"Source code\" for a work means the preferred form of the work for making modifications "
                                  "to it. For a library, complete source code means all the source code for all "
                                  "modules it contains, plus any associated interface definition files, plus "
                                  "the scripts used to control compilation and installation of the library.</p>\n"
                                  "\n"
                                  "<p>Activities other than copying, distribution and modification are not covered "
                                  "by this License; they are outside its scope. The act of running a program "
                                  "using the Library is not restricted, and output from such a program is covered "
                                  "only if its contents constitute a work based on the Library (independent of "
                                  "the use of the Library in a tool for writing it). Whether that is true depends "
                                  "on what the Library does and what the program that uses the Library does.</p>\n"
                                  "\n"
                                  "<p><b>1.</b> You may copy and distribute verbatim copies of the Library's complete source "
                                  "code as you receive it, in any medium, provided that you conspicuously and "
                                  "appropriately publish on each copy an appropriate copyright notice and disclaimer "
                                  "of warranty; keep intact all the notices that refer to this License and to "
                                  "the absence of any warranty; and distribute a copy of this License along with "
                                  "the Library.</p>\n"
                                  "\n"
                                  "<p>You may charge a fee for the physical act of transferring a copy, and you "
                                  "may at your option offer warranty protection in exchange for a fee.</p>\n"
                                  "\n"
                                  "<p><b>2.</b> You may modify your copy or copies of the Library or any portion of it, "
                                  "thus forming a work based on the Library, and copy and distribute such modifications "
                                  "or work under the terms of Section 1 above, provided that you also meet all "
                                  "of these conditions:</p>\n"
                                  "\n"
                                  "<p><b>a)</b> The modified work must itself be a software library.</p>\n"
                                  "\n"
                                  "<p><b>b)</b> You must cause the files modified to carry prominent notices stating that "
                                  "you changed the files and the date of any change.</p>\n"
                                  "\n"
                                  "<p><b>c)</b> You must cause the whole of the work to be licensed at no charge to all "
                                  "third parties under the terms of this License.</p>\n"
                                  "\n"
                                  "<p><b>d)</b> If a facility in the modified Library refers to a function or a table of "
                                  "data to be supplied by an application program that uses the facility, other "
                                  "than as an argument passed when the facility is invoked, then you must make "
                                  "a good faith effort to ensure that, in the event an application does not supply "
                                  "such function or table, the facility still operates, and performs whatever "
                                  "part of its purpose remains meaningful.</p>\n"
                                  "\n"
                                  "<p>(For example, a function in a library to compute square roots has a purpose "
                                  "that is entirely well-defined independent of the application. Therefore, Subsection "
                                  "2d requires that any application-supplied function or table used by this function "
                                  "must be optional: if the application does not supply it, the square root function "
                                  "must still compute square roots.)</p>\n"
                                  "\n"
                                  "<p>These requirements apply to the modified work as a whole. If identifiable "
                                  "sections of that work are not derived from the Library, and can be reasonably "
                                  "considered independent and separate works in themselves, then this License, "
                                  "and its terms, do not apply to those sections when you distribute them as "
                                  "separate works. But when you distribute the same sections as part of a whole "
                                  "which is a work based on the Library, the distribution of the whole must be "
                                  "on the terms of this License, whose permissions for other licensees extend "
                                  "to the entire whole, and thus to each and every part regardless of who wrote "
                                  "it.</p>\n"
                                  "\n"
                                  "<p>Thus, it is not the intent of this section to claim rights or contest your "
                                  "rights to work written entirely by you; rather, the intent is to exercise "
                                  "the right to control the distribution of derivative or collective works based "
                                  "on the Library.</p>\n"
                                  "\n"
                                  "<p>In addition, mere aggregation of another work not based on the Library with "
                                  "the Library (or with a work based on the Library) on a volume of a storage "
                                  "or distribution medium does not bring the other work under the scope of this "
                                  "License.</p>\n"
                                  "\n"
                                  "<p><b>3.</b> You may opt to apply the terms of the ordinary GNU General Public License "
                                  "instead of this License to a given copy of the Library. To do this, you must "
                                  "alter all the notices that refer to this License, so that they refer to the "
                                  "ordinary GNU General Public License, version 2, instead of to this License. "
                                  "(If a newer version than version 2 of the ordinary GNU General Public License "
                                  "has appeared, then you can specify that version instead if you wish.) Do not "
                                  "make any other change in these notices.</p>\n"
                                  "\n"
                                  "<p>Once this change is made in a given copy, it is irreversible for that copy, "
                                  "so the ordinary GNU General Public License applies to all subsequent copies "
                                  "and derivative works made from that copy.</p>\n"
                                  "\n"
                                  "<p>This option is useful when you wish to copy part of the code of the Library "
                                  "into a program that is not a library.</p>\n"
                                  "\n"
                                  "<p><b>4.</b> You may copy and distribute the Library (or a portion or derivative of "
                                  "it, under Section 2) in object code or executable form under the terms of "
                                  "Sections 1 and 2 above provided that you accompany it with the complete corresponding "
                                  "machine-readable source code, which must be distributed under the terms of "
                                  "Sections 1 and 2 above on a medium customarily used for software interchange.</p>\n"
                                  "\n"
                                  "<p>If distribution of object code is made by offering access to copy from a designated "
                                  "place, then offering equivalent access to copy the source code from the same "
                                  "place satisfies the requirement to distribute the source code, even though "
                                  "third parties are not compelled to copy the source along with the object code.</p>\n"
                                  "\n"
                                  "<p><b>5.</b> A program that contains no derivative of any portion of the Library, but "
                                  "is designed to work with the Library by being compiled or linked with it, "
                                  "is called a \"work that uses the Library\". Such a work, in isolation, is not "
                                  "a derivative work of the Library, and therefore falls outside the scope of "
                                  "this License.</p>\n"
                                  "\n"
                                  "<p>However, linking a \"work that uses the Library\" with the Library creates an "
                                  "executable that is a derivative of the Library (because it contains portions "
                                  "of the Library), rather than a \"work that uses the library\". The executable "
                                  "is therefore covered by this License. Section 6 states terms for distribution "
                                  "of such executables.</p>\n"
                                  "\n"
                                  "<p>When a \"work that uses the Library\" uses material from a header file that "
                                  "is part of the Library, the object code for the work may be a derivative work "
                                  "of the Library even though the source code is not. Whether this is true is "
                                  "especially significant if the work can be linked without the Library, or if "
                                  "the work is itself a library. The threshold for this to be true is not precisely "
                                  "defined by law.</p>\n"
                                  "\n"
                                  "<p>If such an object file uses only numerical parameters, data structure layouts "
                                  "and accessors, and small macros and small inline functions (ten lines or less "
                                  "in length), then the use of the object file is unrestricted, regardless of "
                                  "whether it is legally a derivative work. (Executables containing this object "
                                  "code plus portions of the Library will still fall under Section 6.)</p>\n"
                                  "\n"
                                  "<p>Otherwise, if the work is a derivative of the Library, you may distribute "
                                  "the object code for the work under the terms of Section 6. Any executables "
                                  "containing that work also fall under Section 6, whether or not they are linked "
                                  "directly with the Library itself.</p>\n"
                                  "\n"
                                  "<p><b>6.</b> As an exception to the Sections above, you may also combine or link a \"work "
                                  "that uses the Library\" with the Library to produce a work containing portions "
                                  "of the Library, and distribute that work under terms of your choice, provided "
                                  "that the terms permit modification of the work for the customer's own use "
                                  "and reverse engineering for debugging such modifications.</p>\n"
                                  "\n"
                                  "<p>You must give prominent notice with each copy of the work that the Library "
                                  "is used in it and that the Library and its use are covered by this License. "
                                  "You must supply a copy of this License. If the work during execution displays "
                                  "copyright notices, you must include the copyright notice for the Library among "
                                  "them, as well as a reference directing the user to the copy of this License. "
                                  "Also, you must do one of these things:</p>\n"
                                  "\n"
                                  "<p><b>a)</b> Accompany the work with the complete corresponding machine-readable source "
                                  "code for the Library including whatever changes were used in the work (which "
                                  "must be distributed under Sections 1 and 2 above); and, if the work is an "
                                  "executable linked with the Library, with the complete machine-readable \"work "
                                  "that uses the Library\", as object code and/or source code, so that the user "
                                  "can modify the Library and then relink to produce a modified executable containing "
                                  "the modified Library. (It is understood that the user who changes the contents "
                                  "of definitions files in the Library will not necessarily be able to recompile "
                                  "the application to use the modified definitions.)</p>\n"
                                  "\n"
                                  "<p><b>b)</b> Use a suitable shared library mechanism for linking with the Library. A "
                                  "suitable mechanism is one that (1) uses at run time a copy of the library "
                                  "already present on the user's computer system, rather than copying library "
                                  "functions into the executable, and (2) will operate properly with a modified "
                                  "version of the library, if the user installs one, as long as the modified "
                                  "version is interface-compatible with the version that the work was made with.</p>\n"
                                  "\n"
                                  "<p><b>c)</b> Accompany the work with a written offer, valid for at least three years, "
                                  "to give the same user the materials specified in Subsection 6a, above, for "
                                  "a charge no more than the cost of performing this distribution.</p>\n"
                                  "\n"
                                  "<p><b>d)</b> If distribution of the work is made by offering access to copy from a designated "
                                  "place, offer equivalent access to copy the above specified materials from "
                                  "the same place.</p>\n"
                                  "\n"
                                  "<p><b>e)</b> Verify that the user has already received a copy of these materials or "
                                  "that you have already sent this user a copy.</p>\n"
                                  "\n"
                                  "<p>For an executable, the required form of the \"work that uses the Library\" must "
                                  "include any data and utility programs needed for reproducing the executable "
                                  "from it. However, as a special exception, the materials to be distributed "
                                  "need not include anything that is normally distributed (in either source or "
                                  "binary form) with the major components (compiler, kernel, and so on) of the "
                                  "operating system on which the executable runs, unless that component itself "
                                  "accompanies the executable.</p>\n"
                                  "\n"
                                  "<p>It may happen that this requirement contradicts the license restrictions of "
                                  "other proprietary libraries that do not normally accompany the operating system. "
                                  "Such a contradiction means you cannot use both them and the Library together "
                                  "in an executable that you distribute.</p>\n"
                                  "\n"
                                  "<p><b>7.</b> You may place library facilities that are a work based on the Library side-by-side "
                                  "in a single library together with other library facilities not covered by "
                                  "this License, and distribute such a combined library, provided that the separate "
                                  "distribution of the work based on the Library and of the other library facilities "
                                  "is otherwise permitted, and provided that you do these two things:</p>\n"
                                  "\n"
                                  "<p><b>a)</b> Accompany the combined library with a copy of the same work based on the "
                                  "Library, uncombined with any other library facilities. This must be distributed "
                                  "under the terms of the Sections above.</p>\n"
                                  "\n"
                                  "<p><b>b)</b> Give prominent notice with the combined library of the fact that part of "
                                  "it is a work based on the Library, and explaining where to find the accompanying "
                                  "uncombined form of the same work.</p>\n"
                                  "\n"
                                  "<p><b>8.</b> You may not copy, modify, sublicense, link with, or distribute the Library "
                                  "except as expressly provided under this License. Any attempt otherwise to "
                                  "copy, modify, sublicense, link with, or distribute the Library is void, and "
                                  "will automatically terminate your rights under this License. However, parties "
                                  "who have received copies, or rights, from you under this License will not "
                                  "have their licenses terminated so long as such parties remain in full compliance.</p>\n"
                                  "\n"
                                  "<p><b>9.</b> You are not required to accept this License, since you have not signed "
                                  "it. However, nothing else grants you permission to modify or distribute the "
                                  "Library or its derivative works. These actions are prohibited by law if you "
                                  "do not accept this License. Therefore, by modifying or distributing the Library "
                                  "(or any work based on the Library), you indicate your acceptance of this License "
                                  "to do so, and all its terms and conditions for copying, distributing or modifying "
                                  "the Library or works based on it.</p>\n"
                                  "\n"
                                  "<p><b>10.</b> Each time you redistribute the Library (or any work based on the Library), "
                                  "the recipient automatically receives a license from the original licensor "
                                  "to copy, distribute, link with or modify the Library subject to these terms "
                                  "and conditions. You may not impose any further restrictions on the recipients' "
                                  "exercise of the rights granted herein. You are not responsible for enforcing "
                                  "compliance by third parties with this License.</p>\n"
                                  "\n"
                                  "<p><b>11.</b> If, as a consequence of a court judgment or allegation of patent infringement "
                                  "or for any other reason (not limited to patent issues), conditions are imposed "
                                  "on you (whether by court order, agreement or otherwise) that contradict the "
                                  "conditions of this License, they do not excuse you from the conditions of "
                                  "this License. If you cannot distribute so as to satisfy simultaneously your "
                                  "obligations under this License and any other pertinent obligations, then as "
                                  "a consequence you may not distribute the Library at all. For example, if a "
                                  "patent license would not permit royalty-free redistribution of the Library "
                                  "by all those who receive copies directly or indirectly through you, then the "
                                  "only way you could satisfy both it and this License would be to refrain entirely "
                                  "from distribution of the Library.</p>\n"
                                  "\n"
                                  "<p>If any portion of this section is held invalid or unenforceable under any "
                                  "particular circumstance, the balance of the section is intended to apply, "
                                  "and the section as a whole is intended to apply in other circumstances.</p>\n"
                                  "\n"
                                  "<p>It is not the purpose of this section to induce you to infringe any patents "
                                  "or other property right claims or to contest validity of any such claims; "
                                  "this section has the sole purpose of protecting the integrity of the free "
                                  "software distribution system which is implemented by public license practices. "
                                  "Many people have made generous contributions to the wide range of software "
                                  "distributed through that system in reliance on consistent application of that "
                                  "system; it is up to the author/donor to decide if he or she is willing to "
                                  "distribute software through any other system and a licensee cannot impose "
                                  "that choice.</p>\n"
                                  "\n"
                                  "<p>This section is intended to make thoroughly clear what is believed to be a "
                                  "consequence of the rest of this License.</p>\n"
                                  "\n"
                                  "<p><b>12.</b> If the distribution and/or use of the Library is restricted in certain "
                                  "countries either by patents or by copyrighted interfaces, the original copyright "
                                  "holder who places the Library under this License may add an explicit geographical "
                                  "distribution limitation excluding those countries, so that distribution is "
                                  "permitted only in or among countries not thus excluded. In such case, this "
                                  "License incorporates the limitation as if written in the body of this License.</p>\n"
                                  "\n"
                                  "<p><b>13.</b> The Free Software Foundation may publish revised and/or new versions of "
                                  "the Lesser General Public License from time to time. Such new versions will "
                                  "be similar in spirit to the present version, but may differ in detail to address "
                                  "new problems or concerns.</p>\n"
                                  "\n"
                                  "<p>Each version is given a distinguishing version number. If the Library specifies "
                                  "a version number of this License which applies to it and \"any later version\", "
                                  "you have the option of following the terms and conditions either of that version "
                                  "or of any later version published by the Free Software Foundation. If the "
                                  "Library does not specify a license version number, you may choose any version "
                                  "ever published by the Free Software Foundation.</p>\n"
                                  "\n"
                                  "<p><b>14.</b> If you wish to incorporate parts of the Library into other free programs "
                                  "whose distribution conditions are incompatible with these, write to the author "
                                  "to ask for permission. For software which is copyrighted by the Free Software "
                                  "Foundation, write to the Free Software Foundation; we sometimes make exceptions "
                                  "for this. Our decision will be guided by the two goals of preserving the free "
                                  "status of all derivatives of our free software and of promoting the sharing "
                                  "and reuse of software generally.</p>\n"
                                  "\n"
                                  "<p>NO WARRANTY</p>\n"
                                  "\n"
                                  "<p><b>15.</b> BECAUSE THE LIBRARY IS LICENSED FREE OF CHARGE, THERE IS NO WARRANTY FOR "
                                  "THE LIBRARY, TO THE EXTENT PERMITTED BY APPLICABLE LAW. EXCEPT WHEN OTHERWISE "
                                  "STATED IN WRITING THE COPYRIGHT HOLDERS AND/OR OTHER PARTIES PROVIDE THE LIBRARY "
                                  "\"AS IS\" WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING, "
                                  "BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS "
                                  "FOR A PARTICULAR PURPOSE. THE ENTIRE RISK AS TO THE QUALITY AND PERFORMANCE "
                                  "OF THE LIBRARY IS WITH YOU. SHOULD THE LIBRARY PROVE DEFECTIVE, YOU ASSUME "
                                  "THE COST OF ALL NECESSARY SERVICING, REPAIR OR CORRECTION.</p>\n"
                                  "\n"
                                  "<p><b>16.</b> IN NO EVENT UNLESS REQUIRED BY APPLICABLE LAW OR AGREED TO IN WRITING "
                                  "WILL ANY COPYRIGHT HOLDER, OR ANY OTHER PARTY WHO MAY MODIFY AND/OR REDISTRIBUTE "
                                  "THE LIBRARY AS PERMITTED ABOVE, BE LIABLE TO YOU FOR DAMAGES, INCLUDING ANY "
                                  "GENERAL, SPECIAL, INCIDENTAL OR CONSEQUENTIAL DAMAGES ARISING OUT OF THE USE "
                                  "OR INABILITY TO USE THE LIBRARY (INCLUDING BUT NOT LIMITED TO LOSS OF DATA "
                                  "OR DATA BEING RENDERED INACCURATE OR LOSSES SUSTAINED BY YOU OR THIRD PARTIES "
                                  "OR A FAILURE OF THE LIBRARY TO OPERATE WITH ANY OTHER SOFTWARE), EVEN IF SUCH "
                                  "HOLDER OR OTHER PARTY HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES. "
                                  "END OF TERMS AND CONDITIONS</p>\n"
                                  "\n"
                                  "<p>How to Apply These Terms to Your New Libraries</p>\n"
                                  "\n"
                                  "<p>If you develop a new library, and you want it to be of the greatest possible "
                                  "use to the public, we recommend making it free software that everyone can "
                                  "redistribute and change. You can do so by permitting redistribution under "
                                  "these terms (or, alternatively, under the terms of the ordinary General Public "
                                  "License).</p>\n"
                                  "\n"
                                  "<p>To apply these terms, attach the following notices to the library. It is safest "
                                  "to attach them to the start of each source file to most effectively convey "
                                  "the exclusion of warranty; and each file should have at least the \"copyright\" "
                                  "line and a pointer to where the full notice is found.</p>\n"
                                  "\n"
                                  "<p>&lt;one line to give the library's name and an idea of what it does.&gt;</p>\n"
                                  "\n"
                                  "<p>Copyright (C) &lt;year&gt; &lt;name of author&gt;</p>\n"
                                  "\n"
                                  "<p>This library is free software; you can redistribute it and/or modify it under "
                                  "the terms of the GNU Lesser General Public License as published by the Free "
                                  "Software Foundation; either version 2.1 of the License, or (at your option) "
                                  "any later version.</p>\n"
                                  "\n"
                                  "<p>This library is distributed in the hope that it will be useful, but WITHOUT "
                                  "ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS "
                                  "FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more "
                                  "details.</p>\n"
                                  "\n"
                                  "<p>You should have received a copy of the GNU Lesser General Public License along "
                                  "with this library; if not, write to the Free Software Foundation, Inc., 51 "
                                  "Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA</p>\n"
                                  "\n"
                                  "<p>Also add information on how to contact you by electronic and paper mail.</p>\n"
                                  "\n"
                                  "<p>You should also get your employer (if you work as a programmer) or your school, "
                                  "if any, to sign a \"copyright disclaimer\" for the library, if necessary. Here "
                                  "is a sample; alter the names:</p>\n"
                                  "\n"
                                  "<p>Yoyodyne, Inc., hereby disclaims all copyright interest in</p>\n"
                                  "\n"
                                  "<p>the library `Frob' (a library for tweaking knobs) written</p>\n"
                                  "\n"
                                  "<p>by James Random Hacker.</p>\n"
                                  "\n"
                                  "<p>&lt; signature of Ty Coon &gt; , 1 April 1990</p>\n"
                                  "\n"
                                  "<p>Ty Coon, President of Vice</p>\n"
                                  "\n"
                                  "<p>That's all there is to it!</p>\n"));

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

void MainWindow::onChangeColors()
{
    ColorsDialog dlg(m_d->m_mdColors, this);

    if (dlg.exec() == QDialog::Accepted) {
        if (m_d->m_mdColors != dlg.colors()) {
            m_d->m_mdColors = dlg.colors();

            m_d->m_editor->applyColors(m_d->m_mdColors);

            saveCfg();
        }
    }
}

void MainWindow::onTocClicked(const QModelIndex &index)
{
    auto c = m_d->m_editor->textCursor();

    c.setPosition(m_d->m_editor->document()->findBlockByNumber(
                      m_d->m_tocModel->lineNumber(m_d->m_filterTocModel->mapToSource(index))).position());

    m_d->m_editor->setTextCursor(c);

    m_d->m_editor->ensureCursorVisible();

    m_d->m_editor->setFocus();
}

void MainWindow::showOrHideTabs()
{
    m_d->m_tocPanel->setVisible(!m_d->m_tabsVisible);

    auto s = m_d->m_splitter->sizes();

    if (!m_d->m_tabsVisible) {
        if (m_d->m_tabWidth == -1) {
            m_d->m_tabWidth = 250;
        }

        s[0] = m_d->m_tabWidth;
        s[1] = s[1] - m_d->m_tabWidth + m_d->m_minTabWidth;
        m_d->m_sidebarPanel->setMaximumWidth(QWIDGETSIZE_MAX);
        m_d->m_splitter->handle(1)->setCursor(m_d->m_splitterCursor);
    } else {
        m_d->m_tabWidth = s[0];
        const auto w = s[0] - m_d->m_minTabWidth;
        s[0] = m_d->m_minTabWidth;
        s[1] = s[1] + w;
        m_d->m_sidebarPanel->setFixedWidth(m_d->m_minTabWidth);
        m_d->m_splitter->handle(1)->setCursor(Qt::ArrowCursor);
        m_d->m_editor->setFocus();
    }

    m_d->m_tabsVisible = !m_d->m_tabsVisible;

    m_d->m_splitter->setSizes(s);
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
    SettingsDlg dlg(m_d->m_mdColors, m_d->m_editor->font(), m_d->m_editor->margins(),
                    m_d->m_spellingEnabled, this);

    if (m_d->m_settingsWindowWidth != -1 && m_d->m_settingsWindowHeight != -1) {
        dlg.resize(m_d->m_settingsWindowWidth, m_d->m_settingsWindowHeight);
    }

    if (m_d->m_settingsWindowMaximized) {
        dlg.showMaximized();
    }

    bool spellingSettingsChanged = false;

    connect(dlg.sonnetConfigWidget(), &Sonnet::ConfigWidget::configChanged,
        [&spellingSettingsChanged](){ spellingSettingsChanged = true; });

    if (dlg.exec() == QDialog::Accepted) {

        if (dlg.colors() != m_d->m_mdColors) {
            m_d->m_mdColors = dlg.colors();

            m_d->m_editor->applyColors(m_d->m_mdColors);
        }

        if (dlg.currentFont() != m_d->m_editor->font()) {
            m_d->m_editor->applyFont(dlg.currentFont());
        }

        if (dlg.editorMargins() != m_d->m_editor->margins()) {
            m_d->m_editor->margins() = dlg.editorMargins();
        }

        if (m_d->m_spellingEnabled != dlg.isSpellingEnabled()) {
            m_d->m_spellingEnabled = dlg.isSpellingEnabled();

            spellingSettingsChanged = true;
        }

        if (spellingSettingsChanged) {
            m_d->m_editor->enableSpellingCheck(m_d->m_spellingEnabled);
        }
    }

    m_d->m_settingsWindowWidth = dlg.width();
    m_d->m_settingsWindowHeight = dlg.height();
    m_d->m_settingsWindowMaximized = dlg.isMaximized();

    saveCfg();
}

} /* namespace MdEditor */

#include "mainwindow.moc"

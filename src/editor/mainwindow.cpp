/*
    SPDX-FileCopyrightText: 2026 Igor Mironchik <igor.mironchik@gmail.com>
    SPDX-License-Identifier: GPL-3.0-or-later
*/

// md-editor include.
#include "mainwindow.h"
#include "colorsdlg.h"
#include "editor.h"
#include "find.h"
#include "findweb.h"
#include "fontdlg.h"
#include "gotoline.h"
#include "htmldocument.h"
#include "previewpage.h"
#include "settings.h"
#include "syntaxvisitor.h"
#include "toc.h"
#include "version.h"
#include "webview.h"
#include "wordwrapdelegate.h"

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
#include <QMimeData>
#include <QMimeDatabase>
#include <QProcess>
#include <QResizeEvent>
#include <QSet>
#include <QSettings>
#include <QSortFilterProxyModel>
#include <QSplitter>
#include <QStandardPaths>
#include <QStatusBar>
#include <QStyleOptionTab>
#include <QTabBar>
#include <QTabWidget>
#include <QTextBlock>
#include <QTextDocumentFragment>
#include <QTimer>
#include <QToolButton>
#include <QToolTip>
#include <QTreeView>
#include <QTreeWidget>
#include <QUrl>
#include <QVBoxLayout>
#include <QWebChannel>
#include <QWidget>
#include <QWindow>

// md4qt include.
#include <md4qt/src/algo.h>
#include <md4qt/src/html.h>
#include <md4qt/src/parser.h>

// shared include.
#include "folder_chooser.h"
#include "license_dialog.h"
#include "plugins_page.h"
#include "utils.h"

namespace MdEditor
{

//
// HtmlVisitor
//

//! Converter into HTML.
class HtmlVisitor : public MD::details::HtmlVisitor
{
protected:
    //! Prepare text to insert into HTML content.
    QString prepareTextForHtml(const QString &t) override
    {
        auto tmp = MD::details::HtmlVisitor::prepareTextForHtml(t);
        tmp.replace(QLatin1Char('$'), QStringLiteral("<span>$</span>"));

        return tmp;
    }

    void openStyle(const typename MD::ItemWithOpts::Styles &styles) override
    {
        for (const auto &s : styles) {
            switch (s.style()) {
            case MD::TextOption::BoldText:
                m_html.push_back(QStringLiteral("<strong>"));
                break;

            case MD::TextOption::ItalicText:
                m_html.push_back(QStringLiteral("<em>"));
                break;

            case MD::TextOption::StrikethroughText:
                m_html.push_back(QStringLiteral("<del>"));
                break;

            case 8:
                m_html.push_back(QStringLiteral("<sup>"));
                break;

            case 16:
                m_html.push_back(QStringLiteral("<sub>"));
                break;

            case 32:
                m_html.push_back(QStringLiteral("<mark>"));
                break;

            default:
                break;
            }
        }
    }

    void closeStyle(const typename MD::ItemWithOpts::Styles &styles) override
    {
        for (const auto &s : styles) {
            switch (s.style()) {
            case MD::TextOption::BoldText:
                m_html.push_back(QStringLiteral("</strong>"));
                break;

            case MD::TextOption::ItalicText:
                m_html.push_back(QStringLiteral("</em>"));
                break;

            case MD::TextOption::StrikethroughText:
                m_html.push_back(QStringLiteral("</del>"));
                break;

            case 8:
                m_html.push_back(QStringLiteral("</sup>"));
                break;

            case 16:
                m_html.push_back(QStringLiteral("</sub>"));
                break;

            case 32:
                m_html.push_back(QStringLiteral("</mark>"));
                break;

            default:
                break;
            }
        }
    }
};

//
// TabBar
//

//! Tab bar for tabs in main window.
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
        const auto res = QTabBar::event(e);

        if (e->type() == QEvent::Shortcut && res) {
            emit activated();
        }

        return res;
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
    //! Scroll Web preview to a given ID.
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

        const auto id =
            static_cast<TocData *>(
                static_cast<QSortFilterProxyModel *>(model())->mapToSource(indexAt(e->pos())).internalPointer())
                ->m_id;

        menu.addAction(tr("Scroll Web Preview To"), [id, this]() {
            emit this->scrollWebViewToRequested(id);
        });

        menu.exec(e->globalPos());

        e->accept();
    }
}; // class TocTreeView

//
// WorkingDirectoryWidget
//

//! Widget for working directory in a status bar of main window.
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

    //! \return Current working directory.
    const QString &workingDirectory() const
    {
        return (isRelative() ? m_fullPath : m_currentPath);
    }

    //! \return Full path available for selection in this widget.
    const QString &fullPath() const
    {
        return m_fullPath;
    }

    //! Should relative path be used?
    bool isRelative() const
    {
        return !m_useWorkingDir->isChecked();
    }

    //! \return Folder chooser widget.
    MdShared::FolderChooser *folderChooser()
    {
        return m_folderChooser;
    }

public slots:
    //! Set working directory.
    void setWorkingDirectory(const QString &wd,
                             bool notify = true)
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
        auto s = static_cast<QWidget *>(parent());
        const auto p2 = s->mapToGlobal(QPoint{0, 0});

        m_folderChooser->move(p1.x() - (layoutDirection() == Qt::LeftToRight ? m_folderChooser->sizeHint().width() : 0),
                              p2.y() - m_folderChooser->sizeHint().height() - 3);
        m_folderChooser->show();
    }

    void onPathChanged(const QString &path)
    {
        if (!path.isEmpty() && m_currentPath != path) {
            disconnect(m_useWorkingDir,
                       &QCheckBox::checkStateChanged,
                       this,
                       &WorkingDirectoryWidget::onUseWorkingDirChanged);
            m_useWorkingDir->setChecked(true);
            connect(m_useWorkingDir,
                    &QCheckBox::checkStateChanged,
                    this,
                    &WorkingDirectoryWidget::onUseWorkingDirChanged);

            setWorkingDirectory(path, false);

            emit workingDirectoryChanged(path);
        }
    }

    void onUseWorkingDirChanged(Qt::CheckState)
    {
        emit workingDirectoryChanged(m_currentPath);
    }

private:
    //! Text label.
    QLabel *m_label = nullptr;
    //! Button to show folder chooser.
    QToolButton *m_btn = nullptr;
    //! Checkbox for enabling/disabling working directory usage.
    QCheckBox *m_useWorkingDir = nullptr;
    //! Folder chooser widget.
    MdShared::FolderChooser *m_folderChooser = nullptr;
    //! Currently selected path.
    QString m_currentPath;
    //! Full available path.
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

    //! Notify ToC tree to update their items' sizes.
    void notifyTocTree(QAbstractItemModel *model,
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

        m_filePanel = new QWidget(m_tabs);
        auto fpv = new QVBoxLayout(m_filePanel);
        fpv->setContentsMargins(3, 3, 3, 3);
        fpv->setSpacing(3);
        m_backBtn = new QToolButton(m_filePanel);
        m_backBtn->setIcon(
            QIcon::fromTheme(QStringLiteral("go-previous"), QIcon(QStringLiteral(":/res/img/go-previous-16.png"))));
        m_backBtn->setToolTip(MainWindow::tr("Go Back"));
        m_fwdBtn = new QToolButton(m_filePanel);
        m_fwdBtn->setIcon(
            QIcon::fromTheme(QStringLiteral("go-next"), QIcon(QStringLiteral(":/res/img/go-next-16.png"))));
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
        auto editorPanel = new QWidget(m_splitter);
        auto v = new QVBoxLayout(editorPanel);
        v->setContentsMargins(0, 0, 0, 0);
        v->setSpacing(0);
        m_editor = new Editor(editorPanel, m_q);
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
        m_saveAsAction = fileMenu->addAction(QIcon::fromTheme(QStringLiteral("document-save-as"),
                                                              QIcon(QStringLiteral(":/res/img/document-save-as.png"))),
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
        fileMenu->addAction(QIcon::fromTheme(QStringLiteral("application-exit"),
                                             QIcon(QStringLiteral(":/res/img/application-exit.png"))),
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

        m_toggleGoToLineAction = new QAction(
            QIcon::fromTheme(QStringLiteral("go-next-use"), QIcon(QStringLiteral(":/res/img/go-next-use.png"))),
            MainWindow::tr("Go to Line"),
            m_q);
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

        m_actionMenu = m_q->menuBar()->addMenu(MainWindow::tr("&Action"));
        m_goBackAction = new QAction(
            QIcon::fromTheme(QStringLiteral("go-previous"), QIcon(QStringLiteral(":/res/img/go-previous.png"))),
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
        m_livePreviewAction = new QAction(QIcon::fromTheme(QStringLiteral("layer-visible-on"),
                                                           QIcon(QStringLiteral(":/res/img/layer-visible-on.png"))),
                                          MainWindow::tr("Live Preview"));
        m_livePreviewAction->setShortcut(MainWindow::tr("Ctrl+Alt+P"));
        m_livePreviewAction->setCheckable(true);
        m_livePreviewAction->setChecked(true);
        viewMenu->addAction(m_livePreviewAction);

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
        helpMenu->addAction(QIcon::fromTheme(QStringLiteral("bookmarks-organize"),
                                             QIcon(QStringLiteral(":/res/img/bookmarks-organize.png"))),
                            MainWindow::tr("Licenses"),
                            m_q,
                            &MainWindow::onShowLicenses);

        m_cursorPosLabel = new QLabel(m_q);
        m_workingDirectoryWidget = new WorkingDirectoryWidget(m_q);
        m_q->statusBar()->addPermanentWidget(m_workingDirectoryWidget);
        m_q->statusBar()->addPermanentWidget(m_cursorPosLabel);
        m_workingDirectoryWidget->hide();

        QObject::connect(m_workingDirectoryWidget,
                         &WorkingDirectoryWidget::workingDirectoryChanged,
                         m_q,
                         &MainWindow::onWorkingDirectoryChange);
        QObject::connect(m_editor->document(), &QTextDocument::modificationChanged, m_saveAction, &QAction::setEnabled);
        QObject::connect(m_editor->document(),
                         &QTextDocument::modificationChanged,
                         m_q,
                         &MainWindow::setWindowModified);
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

        m_q->onFileNew();

        m_q->setAcceptDrops(true);
    }

    void handleCurrentTab()
    {
        if (m_tabs->currentIndex() == 0) {
            initMarkdownMenu();
        }

        m_currentTab = m_tabs->currentIndex();
    }

    //! \return Data for ToC item.
    StringDataVec paragraphToMenuText(MD::Paragraph *p,
                                      bool skipRtl = false)
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

    //! Populate ToC tree view.
    void initMarkdownMenu()
    {
        m_initMarkdownMenuRequested = true;

        if (m_tabsVisible && m_tocDoc != m_editor->currentDoc()) {
            m_initMarkdownMenuRequested = false;
            m_tocModel->clear();

            m_tocDoc = m_editor->currentDoc();

            std::vector<QModelIndex> current;

            MD::forEach(
                {MD::ItemType::Heading},
                m_tocDoc,
                [this, &current](MD::Item *item) {
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
                                current.push_back(this->m_tocModel->index(this->m_tocModel->rowCount() - 1, 0));
                            } else {
                                this->m_tocModel->addChildItem(current.back(),
                                                               this->paragraphToMenuText(h->text().get()),
                                                               h->startLine(),
                                                               h->level(),
                                                               h->label());
                                current.push_back(
                                    this->m_tocModel->index(this->m_tocModel->rowCount(current.back()) - 1,
                                                            0,
                                                            current.back()));
                            }
                        } else {
                            this->m_tocModel->addTopLevelItem(this->paragraphToMenuText(h->text().get()),
                                                              h->startLine(),
                                                              h->level(),
                                                              h->label());
                            current.push_back(this->m_tocModel->index(this->m_tocModel->rowCount() - 1, 0));
                        }
                    }
                },
                1);
        }
    }

    //! Run function when editor will be ready.
    template<class Func>
    void runWhenEditorReady(Func f)
    {
        if (m_editor->isReady()) {
            f();
        } else {
            m_funcsQueue.push_back(std::move(f));
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
    QAction *m_goBackAction = nullptr;
    QAction *m_goFwdAction = nullptr;
    QMenu *m_actionMenu = nullptr;
    QMenu *m_standardEditMenu = nullptr;
    QMenu *m_settingsMenu = nullptr;
    QToolButton *m_backBtn = nullptr;
    QToolButton *m_fwdBtn = nullptr;
    QWidget *m_filePanel = nullptr;
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
    QSharedPointer<MD::Document> m_mdDoc;
    QSharedPointer<MD::Document> m_tocDoc;
    QString m_baseUrl;
    QString m_rootFilePath;
    QString m_mdPdfExe;
    QString m_launcherExe;
    QString m_htmlContent;
    QString m_tmpWorkingDir;
    QString m_requestedRef;
    int m_tabWidth = -1;
    int m_minTabWidth = -1;
    int m_currentTab = 0;
    int m_settingsWindowWidth = -1;
    int m_settingsWindowHeight = -1;
    bool m_settingsWindowMaximized = false;
    bool m_isDefaultFile = true;
    QVector<std::function<void()>> m_funcsQueue;
    QStringList m_navigationStack;
    int m_navigationStackIdx = -1;
    StartupState m_startupState;
    //! Names of files available in navigation toolbar.
    QSet<QString> m_fullFileNames;
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

        QTimer::singleShot(0, this, &MainWindow::onFirstTimeShown);
    }

    e->accept();
}

void MainWindow::openFile(const QString &path)
{
    if (m_d->m_loadAllFlag) {
        loadAllLinkedFilesImpl();
    }

    QFile f(path);
    if (!f.open(QIODevice::ReadOnly)) {
        QMessageBox::warning(this,
                             windowTitle(),
                             tr("Could not open file %1: %2").arg(QDir::toNativeSeparators(path), f.errorString()));
        return;
    }

    m_d->m_isDefaultFile = false;
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

void MainWindow::openInPreviewMode()
{
    m_d->m_viewAction->setChecked(true);
}

bool MainWindow::isModified() const
{
    return m_d->m_editor->document()->isModified();
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
        }
    }

    m_d->m_isDefaultFile = true;
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
    m_d->m_editor->clearUserStateOnAllBlocks();

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
}

void MainWindow::closeEvent(QCloseEvent *e)
{
    if (isModified()) {
        QMessageBox::StandardButton button =
            QMessageBox::question(this,
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
            m_d->m_html->setText(MD::toHtml<HtmlVisitor>(m_d->m_mdDoc,
                                                         false,
                                                         QStringLiteral("<img src=\"qrc:/res/img/go-jump.png\" />"),
                                                         true,
                                                         &m_d->m_editor->idsMap()));
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
            .arg(c_version, c_md4qtVersion, c_commit),
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

inline QString itemType(MD::ItemType t,
                        bool alone)
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
            QToolTip::showText(pos, itemType(items.front()->type(), items.size() == 1));
        } else {
            QToolTip::showText(pos,
                               tr("%1 in %2")
                                   .arg(itemType(items.at(1)->type(), items.size() == 1),
                                        itemType(items.front()->type(), items.size() == 1)));
        }
    }
}

void MainWindow::onLineNumberContextMenuRequested(int lineNumber,
                                                  const QPoint &pos)
{
    const auto items = m_d->m_editor->syntaxHighlighter().findFirstInCache(
        {0, lineNumber, m_d->m_editor->document()->findBlockByLineNumber(lineNumber).length(), lineNumber});

    if (!items.isEmpty()) {
        const auto it = m_d->m_editor->idsMap().find(items.front());

        if (it != m_d->m_editor->idsMap().cend()) {
            QMenu menu;

            const auto id = it.value();

            menu.addAction(tr("Scroll Web Preview To"), [id, this]() {
                this->onScrollWebViewTo(id);
            });

            menu.exec(pos);
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

        m_d->m_editor->doUpdate();

        saveCfg();
    }
}

const QString s_ui = QStringLiteral("ui");
const QString s_font = QStringLiteral("font");
const QString s_family = QStringLiteral("family");
const QString s_size = QStringLiteral("size");
const QString s_linkColor = QStringLiteral("linkColor");
const QString s_textColor = QStringLiteral("textColor");
const QString s_inlineColor = QStringLiteral("inlineColor");
const QString s_htmlColor = QStringLiteral("htmlColor");
const QString s_tableColor = QStringLiteral("tableColor");
const QString s_codeColor = QStringLiteral("codeColor");
const QString s_mathColor = QStringLiteral("mathColor");
const QString s_referenceColor = QStringLiteral("referenceColor");
const QString s_specialColor = QStringLiteral("specialColor");
const QString s_enableMargin = QStringLiteral("enableMargin");
const QString s_margin = QStringLiteral("margin");
const QString s_enableColors = QStringLiteral("enableColors");
const QString s_sidebarWidth = QStringLiteral("sidebarWidth");
const QString s_indent = QStringLiteral("indent");
const QString s_mode = QStringLiteral("mode");
const QString s_tabs = QStringLiteral("tabs");
const QString s_spaces = QStringLiteral("spaces");
const QString s_spacesCount = QStringLiteral("spacesCount");
const QString s_findCaseSensitive = QStringLiteral("findCaseSensitive");
const QString s_findWholeWord = QStringLiteral("findWholeWord");
const QString s_window = QStringLiteral("window");
const QString s_width = QStringLiteral("width");
const QString s_height = QStringLiteral("height");
const QString s_x = QStringLiteral("x");
const QString s_y = QStringLiteral("y");
const QString s_maximized = QStringLiteral("maximized");
const QString s_settingsWindow = QStringLiteral("settings_window");
const QString s_spelling = QStringLiteral("spelling");
const QString s_enabled = QStringLiteral("enabled");
const QString s_plugins = QStringLiteral("plugins");
const QString s_superscript = QStringLiteral("superscript");
const QString s_delimiter = QStringLiteral("delimiter");
const QString s_subscript = QStringLiteral("subscript");
const QString s_mark = QStringLiteral("mark");
const QString s_yaml = QStringLiteral("yaml");
const QString s_autoLists = QStringLiteral("autoLists");
const QString s_enableCodeBlockTheme = QStringLiteral("enableCodeBlockTheme");
const QString s_codeBlockTheme = QStringLiteral("codeBlockTheme");
const QString s_drawCodeBackground = QStringLiteral("drawCodeBackground");
const QString s_githubBehaviour = QStringLiteral("githubBehaviour");
const QString s_dontUseAutoListInCodeBlock = QStringLiteral("dontUseAutoListInCodeBlock");
const QString s_autoCodeBlocks = QStringLiteral("autoCodeBlocks");

void MainWindow::saveCfg() const
{
    const auto f = m_d->m_editor->font();

    QSettings s;

    s.beginGroup(s_ui);

    s.beginGroup(s_font);
    s.setValue(s_family, f.family());
    s.setValue(s_size, f.pointSize());
    s.endGroup();

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

    s.beginGroup(s_indent);
    s.setValue(s_mode, m_d->m_editor->settings().m_indentMode == Editor::IndentMode::Tabs ? s_tabs : s_spaces);
    s.setValue(s_spacesCount, m_d->m_editor->settings().m_indentSpacesCount);
    s.endGroup();

    s.setValue(s_autoLists, m_d->m_editor->settings().m_isAutoListsEnabled);
    s.setValue(s_githubBehaviour, m_d->m_editor->settings().m_githubBehaviour);
    s.setValue(s_dontUseAutoListInCodeBlock, m_d->m_editor->settings().m_dontUseAutoListInCodeBlock);

    s.setValue(s_autoCodeBlocks, m_d->m_editor->settings().m_isAutoCodeBlocksEnabled);

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
        tr("<b>Line:</b> %1, <b>Col:</b> %2").arg(c.block().blockNumber() + 1).arg(c.positionInBlock() + 1));
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

    if (!m_d->m_startupState.m_fileName.isEmpty()) {
        openFile(m_d->m_startupState.m_fileName);
    }

    if (!m_d->m_startupState.m_workingDir.isEmpty()) {
        setWorkingDirectory(m_d->m_startupState.m_workingDir);
    }

    if (m_d->m_startupState.m_loadAllLinked) {
        loadAllLinkedFiles();
    }
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
        setPlugins(parser, m_d->m_editor->settings().m_pluginsCfg);

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
            m_d->m_html->setText(MD::toHtml<HtmlVisitor>(m_d->m_mdDoc,
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

        QFile f(path);
        if (!f.open(QIODevice::ReadOnly)) {
            QMessageBox::warning(this,
                                 windowTitle(),
                                 tr("Could not open file %1: %2").arg(QDir::toNativeSeparators(path), f.errorString()));
            return;
        }

        m_d->m_editor->setDocName(path);
        m_d->m_editor->setText(f.readAll());
        f.close();

        updateWindowTitle();

        m_d->m_editor->document()->clearUndoRedoStacks();
        m_d->m_editor->setFocus();

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

            if (fileName.endsWith(".md", Qt::CaseInsensitive)
                || fileName.endsWith(".markdown", Qt::CaseInsensitive)) {
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

        m_d->m_actionMenu->menuAction()->setVisible(false);
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

        if (m_d->m_loadAllFlag) {
            m_d->m_actionMenu->menuAction()->setVisible(true);
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
            m_d->m_html->setText(MD::toHtml<HtmlVisitor>(m_d->m_mdDoc,
                                                         false,
                                                         QStringLiteral("<img src=\"qrc:/res/img/go-jump.png\" />"),
                                                         true,
                                                         &m_d->m_editor->idsMap()));
        } else {
            m_d->m_html->setText({});
        }
    }

    m_d->m_splitter->setSizes(s);
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
    msg.addLicense(s_jsEmojiName, s_jsEmojiLicense);
    msg.addLicense(s_highlightBlockquoteName, s_highlightBlockquoteLicense);
    msg.addLicense(s_sonnetName, s_sonnetLicense);
    msg.addLicense(s_md4qtName, s_md4qtLicense);

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

#include "mainwindow.moc"

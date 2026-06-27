/*
    SPDX-FileCopyrightText: 2026 Igor Mironchik <igor.mironchik@gmail.com>
    SPDX-License-Identifier: GPL-3.0-or-later
*/

#pragma once

// md-editor include.
#include "editor.h"
#include "toc.h"

QT_BEGIN_NAMESPACE
class QSplitter;
class QTreeWidget;
class QSortFilterProxyModel;
QT_END_NAMESPACE

namespace MdEditor
{

class MainWindow;
class WebView;
class PreviewPage;
class HtmlDocument;
class WordWrapItemDelegate;
class FindWeb;
class GoToLine;
class TabWidget;
class WorkingDirectoryWidget;
class TocTreeView;
class TocModel;

//
// StartupState
//

//! Startup state of the application.
struct StartupState {
    //! File name to open on startup.
    QString m_fileName;
    //! Working directory to set on startup.
    QString m_workingDir;
    //! Should all linked files be loaded on startup?
    bool m_loadAllLinked;
    //! Should editor be opened in preview mode?
    bool m_previewMode;
}; // strucy StartupState

//
// MainWindowPrivate
//

struct MainWindowPrivate {
    MainWindowPrivate(MainWindow *parent);
    //! Notify ToC tree to update their items' sizes.
    void notifyTocTree(QAbstractItemModel *model,
                       WordWrapItemDelegate *delegate,
                       const QModelIndex &parent);

    void initUi();

    void handleCurrentTab();

    //! \return Data for ToC item.
    StringDataVec paragraphToMenuText(MD::Paragraph *p,
                                      bool skipRtl = false);

    //! Populate ToC tree view.
    void initMarkdownMenu();

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
    QSplitter *m_tabEditorSplitter = nullptr;
    QSplitter *m_editorPreviewSplitter = nullptr;
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
    QAction *m_orientAction = nullptr;
    QAction *m_mdStandardAction = nullptr;
    QMenu *m_actionMenu = nullptr;
    QMenu *m_standardEditMenu = nullptr;
    QMenu *m_settingsMenu = nullptr;
    QMenu *m_formatMenu = nullptr;
    QToolButton *m_backBtn = nullptr;
    QToolButton *m_fwdBtn = nullptr;
    QWidget *m_filePanel = nullptr;
    QTreeWidget *m_fileTree = nullptr;
    QWidget *m_tocPanel = nullptr;
    QWidget *m_editorPreviewWidget = nullptr;
    QWidget *m_editorPanel = nullptr;
    QWidget *m_previewPanel = nullptr;
    WorkingDirectoryWidget *m_workingDirectoryWidget = nullptr;
    TocTreeView *m_tocTree = nullptr;
    TocModel *m_tocModel = nullptr;
    QStringListModel *m_urlAutoCompleteModel = nullptr;
    QSortFilterProxyModel *m_filterTocModel = nullptr;
    QLabel *m_cursorPosLabel = nullptr;
    QLineEdit *m_tocFilterLine = nullptr;
    QToolButton *m_pinPreviewEditor = nullptr;
    bool m_sizesInitialized = false;
    bool m_shownAlready = false;
    bool m_loadAllFlag = false;
    bool m_previewMode = false;
    bool m_tabsVisible = false;
    bool m_livePreviewVisible = true;
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
    bool m_horizontalOrient = true;
    QVector<std::function<void()>> m_funcsQueue;
    QStringList m_navigationStack;
    int m_navigationStackIdx = -1;
    StartupState m_startupState;
    //! Names of files available in navigation toolbar.
    QSet<QString> m_fullFileNames;
}; // struct MainWindowPrivate

} /* namespace MdEditor */

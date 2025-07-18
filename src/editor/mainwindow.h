/*
    SPDX-FileCopyrightText: 2024-2025 Igor Mironchik <igor.mironchik@gmail.com>
    SPDX-License-Identifier: GPL-3.0-or-later
*/

#pragma once

// Qt include.
#include <QMainWindow>
#include <QScopedPointer>

QT_BEGIN_NAMESPACE
class QTreeWidgetItem;
QT_END_NAMESPACE

//! Namespace for Markdown editor.
namespace MdEditor
{

//
// MainWindow
//

struct MainWindowPrivate;

//! Main window.
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow();
    ~MainWindow() override;

public slots:
    //! Open file.
    void openFile(const QString &path);
    //! Reorganize UI to show only Web preview.
    void openInPreviewMode();
    //! Load all linked files.
    void loadAllLinkedFiles();
    //! Set working directory.
    void setWorkingDirectory(const QString &path);

protected:
    void resizeEvent(QResizeEvent *e) override;
    void closeEvent(QCloseEvent *e) override;
    bool event(QEvent *event) override;
    void showEvent(QShowEvent *e) override;

private slots:
    void onFileNew();
    void onFileOpen();
    void onFileSave();
    void onFileSaveAs();
    void onTextChanged();
    void onAbout();
    void onAboutQt();
    void onLineHovered(int lineNumber, const QPoint &pos);
    void onLineNumberContextMenuRequested(int lineNumber, const QPoint &pos);
    void onFind(bool on);
    void onFindWeb(bool on);
    void onGoToLine(bool on);
    void onMisspelledFount(bool found);
    void onNextMisspelled();
    void onChooseFont();
    void onLessFontSize();
    void onMoreFontSize();
    void onToolHide();
    void onCursorPositionChanged();
    void onEditMenuActionTriggered(QAction *action);
    void onNavigationDoubleClicked(QTreeWidgetItem *item, int column);
    void onTogglePreviewAction(bool checked);
    void onToggleLivePreviewAction(bool checked);
    void onShowLicenses();
    void onConvertToPdf();
    void onAddTOC();
    void onChangeColors();
    void onTocClicked(const QModelIndex &index);
    void onTabClicked(int index);
    void onSettings();
    void onTabActivated();
    void onWorkingDirectoryChange(const QString &);
    void onScrollWebViewTo(const QString &id);
    void onEditorReady();
    void onSetWorkingDirectory();
    void onProcessQueue();

private:
    //! \return Is document was changed?
    bool isModified() const;
    //! \return HTML content.
    const QString &htmlContent() const;
    //! Save configuration.
    void saveCfg() const;
    //! Read configuration.
    void readCfg();
    //! Read all linked files.
    void readAllLinked();
    //! Update window title.
    void updateWindowTitle();
    //! Update "Load All Linked Files" action's text.
    void updateLoadAllLinkedFilesMenuText();
    //! Switch to alone file mode.
    void closeAllLinkedFiles();
    //! Show/hide left tabbed sidebar.
    void showOrHideTabs();
    //! Real implementation of loading all linked files.
    void loadAllLinkedFilesImpl();

private:
    Q_DISABLE_COPY(MainWindow)

    friend struct MainWindowPrivate;
    friend class Find;
    friend class FindWeb;
    friend class GoToLine;

    QScopedPointer<MainWindowPrivate> m_d;
}; // class MainWindow

} /* namespace MdEditor */

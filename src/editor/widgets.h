/*
    SPDX-FileCopyrightText: 2026 Igor Mironchik <igor.mironchik@gmail.com>
    SPDX-License-Identifier: GPL-3.0-or-later
*/

#pragma once

// Qt include.
#include <QTabBar>
#include <QTabWidget>
#include <QTreeView>

QT_BEGIN_NAMESPACE
class QLabel;
class QToolButton;
class QCheckBox;
QT_END_NAMESPACE

namespace MdShared
{

class FolderChooser;

}

namespace MdEditor
{

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
    explicit TabBar(QWidget *parent);
    ~TabBar() override = default;

protected:
    bool event(QEvent *e) override;
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
    explicit TabWidget(QWidget *parent);
    ~TabWidget() override = default;

protected:
    void keyPressEvent(QKeyEvent *e) override;
    void tabRemoved(int index) override;
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
    explicit TocTreeView(QWidget *parent);
    ~TocTreeView() override = default;

protected:
    void keyPressEvent(QKeyEvent *e) override;
    void contextMenuEvent(QContextMenuEvent *e) override;
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
    WorkingDirectoryWidget(QWidget *parent);
    ~WorkingDirectoryWidget() override = default;

    //! \return Current working directory.
    const QString &workingDirectory() const;
    //! \return Full path available for selection in this widget.
    const QString &fullPath() const;
    //! Should relative path be used?
    bool isRelative() const;
    //! \return Folder chooser widget.
    MdShared::FolderChooser *folderChooser();

    int labelHeight() const;

public slots:
    //! Set working directory.
    void setWorkingDirectory(const QString &wd,
                             bool notify = true);

private slots:
    void onChangeButtonClicked();
    void onPathChanged(const QString &path);
    void onUseWorkingDirChanged(Qt::CheckState);

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

} /* namespace MdEditor */

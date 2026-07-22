/*
    SPDX-FileCopyrightText: 2026 Igor Mironchik <igor.mironchik@gmail.com>
    SPDX-License-Identifier: GPL-3.0-or-later
*/

// md-editor include.
#include "widgets.h"
#include "toc.h"

// md-shared include.
#include "folder_chooser.h"

// Qt include.
#include <QCheckBox>
#include <QEvent>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QLabel>
#include <QMenu>
#include <QSortFilterProxyModel>
#include <QToolButton>

namespace MdEditor
{

//
// TabBar
//

TabBar::TabBar(QWidget *parent)
    : QTabBar(parent)
{
}

bool TabBar::event(QEvent *e)
{
    const auto res = QTabBar::event(e);

    if (e->type() == QEvent::Shortcut && res) {
        Q_EMIT activated();
    }

    return res;
}

//
// TabWidget
//

TabWidget::TabWidget(QWidget *parent)
    : QTabWidget(parent)
{
    auto tabs = new TabBar(this);

    setTabBar(tabs);

    connect(tabs, &TabBar::activated, this, &TabWidget::activated);
}

void TabWidget::keyPressEvent(QKeyEvent *e)
{
    if (e->key() == Qt::Key_Return) {
        Q_EMIT activated();
    }

    QTabWidget::keyPressEvent(e);
}

void TabWidget::tabRemoved(int index)
{
    QTabWidget::tabRemoved(index);

    Q_EMIT removed();
}

//
// TocTreeView
//

TocTreeView::TocTreeView(QWidget *parent)
    : QTreeView(parent)
{
}

void TocTreeView::keyPressEvent(QKeyEvent *e)
{
    QTreeView::keyPressEvent(e);

    if (e->key() == Qt::Key_Return) {
        e->accept();
    }
}

void TocTreeView::contextMenuEvent(QContextMenuEvent *e)
{
    QMenu menu;

    const auto id = static_cast<TocData *>(
                        static_cast<QSortFilterProxyModel *>(model())->mapToSource(indexAt(e->pos())).internalPointer())
                        ->m_id;

    menu.addAction(tr("Scroll Web Preview To"), [id, this]() {
        Q_EMIT this->scrollWebViewToRequested(id);
    });

    menu.exec(e->globalPos());

    e->accept();
}

//
// WorkingDirectoryWidget
//

WorkingDirectoryWidget::WorkingDirectoryWidget(QWidget *parent)
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
    const auto h = labelHeight();
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

int WorkingDirectoryWidget::labelHeight() const
{
    return m_label->sizeHint().height();
}

const QString &WorkingDirectoryWidget::workingDirectory() const
{
    return (isRelative() ? m_fullPath : m_currentPath);
}

const QString &WorkingDirectoryWidget::fullPath() const
{
    return m_fullPath;
}

bool WorkingDirectoryWidget::isRelative() const
{
    return !m_useWorkingDir->isChecked();
}

MdShared::FolderChooser *WorkingDirectoryWidget::folderChooser()
{
    return m_folderChooser;
}

void WorkingDirectoryWidget::setWorkingDirectory(const QString &wd,
                                                 bool notify)
{
    m_label->setText(tr("<b>Working Directory:</b> ") + wd);
    m_currentPath = wd;

    if (notify) {
        m_folderChooser->setPath(wd);
        m_fullPath = wd;
    }

    show();
}

void WorkingDirectoryWidget::onChangeButtonClicked()
{
    const auto p1 = mapToGlobal(m_btn->pos());
    auto s = static_cast<QWidget *>(parent());
    const auto p2 = s->mapToGlobal(QPoint{0, 0});

    m_folderChooser->move(p1.x() - (layoutDirection() == Qt::LeftToRight ? m_folderChooser->sizeHint().width() : 0),
                          p2.y() - m_folderChooser->sizeHint().height() - 3);
    m_folderChooser->show();
}

void WorkingDirectoryWidget::onPathChanged(const QString &path)
{
    if (!path.isEmpty() && m_currentPath != path) {
        disconnect(m_useWorkingDir,
                   &QCheckBox::checkStateChanged,
                   this,
                   &WorkingDirectoryWidget::onUseWorkingDirChanged);
        m_useWorkingDir->setChecked(true);
        connect(m_useWorkingDir, &QCheckBox::checkStateChanged, this, &WorkingDirectoryWidget::onUseWorkingDirChanged);

        setWorkingDirectory(path, false);

        Q_EMIT workingDirectoryChanged(path);
    }
}

void WorkingDirectoryWidget::onUseWorkingDirChanged(Qt::CheckState)
{
    Q_EMIT workingDirectoryChanged(m_currentPath);
}

} /* namespace MdEditor */

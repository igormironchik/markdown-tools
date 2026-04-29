/*
 *    This file is part of the KDE libraries
 *    SPDX-FileCopyrightText: 2026 Nicolas Fella <nicolas.fella@gmx.de>
 *
 *    SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <kactionmenu.h>

#include <QMenu>
#include <QTest>
#include <QToolBar>

class KActionMenuTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase()
    {
    }

    void testActionMenu()
    {
        auto menu = new KActionMenu(nullptr);

        auto a1 = new QAction;

        menu->addAction(a1);

        QCOMPARE(menu->menu()->actions().size(), 1);
        QCOMPARE(menu->menu()->actions()[0], a1);

        auto sep = menu->addSeparator();
        QVERIFY(sep->isSeparator());
        QCOMPARE(menu->menu()->actions().size(), 2);
        QCOMPARE(menu->menu()->actions()[0], a1);
        QCOMPARE(menu->menu()->actions()[1], sep);

        auto a2 = new QAction;
        menu->insertAction(a1, a2);
        QCOMPARE(menu->menu()->actions().size(), 3);
        QCOMPARE(menu->menu()->actions()[0], a2);
        QCOMPARE(menu->menu()->actions()[1], a1);
        QCOMPARE(menu->menu()->actions()[2], sep);

        auto sep2 = menu->insertSeparator(a1);
        QVERIFY(sep2->isSeparator());
        QCOMPARE(menu->menu()->actions().size(), 4);
        QCOMPARE(menu->menu()->actions()[0], a2);
        QCOMPARE(menu->menu()->actions()[1], sep2);
        QCOMPARE(menu->menu()->actions()[2], a1);
        QCOMPARE(menu->menu()->actions()[3], sep);

        menu->removeAction(a1);
        QCOMPARE(menu->menu()->actions().size(), 3);
        QCOMPARE(menu->menu()->actions()[0], a2);
        QCOMPARE(menu->menu()->actions()[1], sep2);
        QCOMPARE(menu->menu()->actions()[2], sep);

        QCOMPARE(menu->popupMode(), QToolButton::DelayedPopup);
        menu->setPopupMode(QToolButton::MenuButtonPopup);
        QCOMPARE(menu->popupMode(), QToolButton::MenuButtonPopup);

        auto parent = new QWidget;
        auto w = menu->createWidget(parent);
        QCOMPARE(w, nullptr);

        auto toolbar = new QToolBar;
        auto toolbarButton = qobject_cast<QToolButton *>(menu->createWidget(toolbar));
        QVERIFY(toolbarButton);

        delete menu;
        delete a1;
        delete a2;
        delete w;
        delete toolbar;
    }

    void testSetMenu()
    {
        auto actionMenu = new KActionMenu(nullptr);

        auto menu = std::make_unique<QMenu>();
        // setMenu doesn't take ownership, make sure nothing crashes or leaks
        actionMenu->setMenu(menu.get());

        delete actionMenu;
    }
};

QTEST_MAIN(KActionMenuTest)

#include "kactionmenutest.moc"

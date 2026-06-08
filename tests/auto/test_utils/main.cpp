/*
    SPDX-FileCopyrightText: 2026 Igor Mironchik <igor.mironchik@gmail.com>
    SPDX-License-Identifier: GPL-3.0-or-later
*/

// md-utils include.
#include "diff.h"

bool operator==(const MdUtils::BlockLinesDiff &d1,
                const MdUtils::BlockLinesDiff &d2)
{
    return (d1.m_start == d2.m_start && d1.m_end == d2.m_end);
}

// Qt include.
#include <QtTest/QtTest>

using namespace MdUtils;

//
// TestUtils
//

class TestUtils final : public QObject
{
    Q_OBJECT

private slots:
    void test_simple_equal()
    {
        QVector<QSharedPointer<BlockLines>> bv1, bv2;

        bv1.append(QSharedPointer<BlockLines>::create(0, 10));

        bv2.append(QSharedPointer<BlockLines>::create(2, 12));

        BlockLinesDiff diff;

        QCOMPARE(compareBlocks(bv1, bv2), diff);
    }

    void test_two_equal()
    {
        QVector<QSharedPointer<BlockLines>> bv1, bv2;

        bv1.append(QSharedPointer<BlockLines>::create(0, 10));
        bv1.append(QSharedPointer<BlockLines>::create(11, 21));

        bv2.append(QSharedPointer<BlockLines>::create(2, 12));
        bv2.append(QSharedPointer<BlockLines>::create(14, 24));

        BlockLinesDiff diff;

        QCOMPARE(compareBlocks(bv1, bv2), diff);
    }

    void test_two_equal_with_child()
    {
        QVector<QSharedPointer<BlockLines>> bv1, bv2;

        {
            bv1.append(QSharedPointer<BlockLines>::create(0, 10));
            auto b1 = QSharedPointer<BlockLines>::create(11, 21);
            b1->m_children.append(QSharedPointer<BlockLines>::create(12, 14));
            bv1.append(b1);
        }

        {
            bv2.append(QSharedPointer<BlockLines>::create(2, 12));
            auto b1 = QSharedPointer<BlockLines>::create(14, 24);
            b1->m_children.append(QSharedPointer<BlockLines>::create(16, 18));
            bv2.append(b1);
        }

        BlockLinesDiff diff;

        QCOMPARE(compareBlocks(bv1, bv2), diff);
    }

    void test_two_not_equal_with_child()
    {
        QVector<QSharedPointer<BlockLines>> bv1, bv2;

        {
            bv1.append(QSharedPointer<BlockLines>::create(0, 10));
            auto b1 = QSharedPointer<BlockLines>::create(11, 21);
            b1->m_children.append(QSharedPointer<BlockLines>::create(12, 14));
            bv1.append(b1);
        }

        {
            bv2.append(QSharedPointer<BlockLines>::create(2, 12));
            auto b1 = QSharedPointer<BlockLines>::create(14, 24);
            bv2.append(b1);
        }

        BlockLinesDiff diff;
        diff.m_start = 14;
        diff.m_end = 24;

        QCOMPARE(compareBlocks(bv1, bv2), diff);
    }

    void test_two_not_equal_with_two_child()
    {
        QVector<QSharedPointer<BlockLines>> bv1, bv2;

        {
            bv1.append(QSharedPointer<BlockLines>::create(0, 10));
            auto b1 = QSharedPointer<BlockLines>::create(11, 21);
            b1->m_children.append(QSharedPointer<BlockLines>::create(12, 14));
            bv1.append(b1);
        }

        {
            bv2.append(QSharedPointer<BlockLines>::create(2, 12));
            auto b1 = QSharedPointer<BlockLines>::create(14, 24);
            b1->m_children.append(QSharedPointer<BlockLines>::create(16, 18));
            b1->m_children.append(QSharedPointer<BlockLines>::create(19, 24));
            bv2.append(b1);
        }

        BlockLinesDiff diff;
        diff.m_start = 14;
        diff.m_end = 24;

        QCOMPARE(compareBlocks(bv1, bv2), diff);
    }

    void test_with_empty_1()
    {
        QVector<QSharedPointer<BlockLines>> bv1, bv2;

        {
            bv1.append(QSharedPointer<BlockLines>::create(0, 10));
            auto b1 = QSharedPointer<BlockLines>::create(11, 21);
            b1->m_children.append(QSharedPointer<BlockLines>::create(12, 14));
            bv1.append(b1);
        }

        BlockLinesDiff diff;

        QCOMPARE(compareBlocks(bv1, bv2), diff);
    }

    void test_with_empty_2()
    {
        QVector<QSharedPointer<BlockLines>> bv1, bv2;

        {
            bv2.append(QSharedPointer<BlockLines>::create(0, 10));
            auto b1 = QSharedPointer<BlockLines>::create(11, 21);
            b1->m_children.append(QSharedPointer<BlockLines>::create(12, 14));
            bv2.append(b1);
        }

        BlockLinesDiff diff;

        QCOMPARE(compareBlocks(bv1, bv2), diff);
    }

    void test_delete_in_the_middle()
    {
        QVector<QSharedPointer<BlockLines>> bv1, bv2;

        bv1.append(QSharedPointer<BlockLines>::create(0, 10));
        bv1.append(QSharedPointer<BlockLines>::create(11, 22));
        bv1.append(QSharedPointer<BlockLines>::create(23, 25));

        bv2.append(QSharedPointer<BlockLines>::create(0, 10));
        bv2.append(QSharedPointer<BlockLines>::create(11, 13));

        BlockLinesDiff diff;

        QCOMPARE(compareBlocks(bv1, bv2), diff);
    }

    void test_insert_in_the_middle()
    {
        QVector<QSharedPointer<BlockLines>> bv1, bv2;

        bv1.append(QSharedPointer<BlockLines>::create(0, 10));
        bv1.append(QSharedPointer<BlockLines>::create(11, 13));

        bv2.append(QSharedPointer<BlockLines>::create(0, 10));
        bv2.append(QSharedPointer<BlockLines>::create(11, 22));
        bv2.append(QSharedPointer<BlockLines>::create(23, 25));

        BlockLinesDiff diff;

        QCOMPARE(compareBlocks(bv1, bv2), diff);
    }
}; // class TestRender

QTEST_MAIN(TestUtils)

#include "main.moc"

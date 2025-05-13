/*
    SPDX-FileCopyrightText: 2024-2025 Igor Mironchik <igor.mironchik@gmail.com>
    SPDX-License-Identifier: GPL-3.0-or-later
*/

// md-editor include.
#include "wordwrapdelegate.hpp"
#include "toc.hpp"

// Qt include.
#include <QApplication>
#include <QHeaderView>
#include <QPainter>
#include <QSortFilterProxyModel>
#include <QStyle>

#include <chrono>

namespace MdEditor
{

static const QColor s_codeColor = QColor(33, 122, 255);
static const QColor s_codeBackgroundColor = QColor(239, 239, 239);

//
// WordWrapItemDelegate
//

WordWrapItemDelegate::WordWrapItemDelegate(QTreeView *parent, TocModel *model, QSortFilterProxyModel *sortModel)
    : QStyledItemDelegate(parent)
    , m_parent(parent)
    , m_model(model)
    , m_sortModel(sortModel)
{
}

namespace /* anonymous */
{

struct LayoutDirectionHandler
{
    LayoutDirectionHandler(bool rightToLeft, int leftX, int width, int y, int lineHeight)
        : m_rightToLeft(rightToLeft)
        , m_leftX(leftX)
        , m_width(width)
        , m_y(y)
        , m_lineHeight(lineHeight)
        , m_x(startX())
    {
    }

    bool isRightToLeft() const { return m_rightToLeft; }

    bool isFit(int width) const
    {
        return (isRightToLeft() ? m_x - width >= m_leftX : m_x + width <= m_leftX + m_width );
    }

    QRect codeRect(int startCodeX, int startCodeY, int codeWidth) const
    {
        return (isRightToLeft() ? QRect{startCodeX - codeWidth, startCodeY, codeWidth, m_lineHeight} :
                                  QRect{startCodeX, startCodeY, codeWidth, m_lineHeight});
    }

    bool isWider(int width) const { return width > m_width; }

    void moveToNewLine()
    {
        m_y += m_lineHeight;
        ++m_linesCount;
        m_x = startX();
    }

    int height() const
    {
        return m_linesCount * m_lineHeight;
    }

    int width() const
    {
        return m_width;
    }

    void moveToNewLineIfSomethingThere()
    {
        if (isRightToLeft() ? m_x < startX() : m_x > startX()) {
            moveToNewLine();
        }
    }

    QRect rect(int width) const
    {
        return (isRightToLeft() ? QRect{m_x - width, m_y, width, m_lineHeight} :
                                  QRect{m_x, m_y, width, m_lineHeight});
    }

    int startX() const { return (isRightToLeft() ? m_leftX + m_width : m_leftX); }

    void moveXBy(int delta)
    {
        if (isRightToLeft()) {
            m_x -= delta;
        } else {
            m_x += delta;
        }
    }

    int x() const { return m_x; }

    int y() const { return m_y; }

    bool m_rightToLeft;
    int m_leftX;
    int m_width;
    int m_y;
    int m_lineHeight;
    int m_x;
    int m_linesCount = 1;
}; // struct LayoutDirectionHandler

} /* namespace anonymous */

QSize WordWrapItemDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    const auto w = QApplication::style()->pixelMetric(QStyle::PM_TreeViewIndentation);

    int level = 1;
    auto i = index;

    while (i.parent().isValid()) {
        ++level;
        i = i.parent();
    }

    auto &data = m_model->stringData(m_sortModel->mapToSource(index));

    if (data.isEmpty()) {
        return {};
    }

    const auto sw = option.fontMetrics.horizontalAdvance(QLatin1Char(' '));
    const auto width = m_parent->header()->sectionSize(index.column()) - w * level;

    int startCodeX, startCodeY, codeWidth;
    LayoutDirectionHandler layout(data.first().m_data.m_text.isRightToLeft(), 0, width, 0,
                                  option.fontMetrics.height());

    for (auto it = data.begin(), last = data.end(); it != last; ++it) {
        it->m_backgroundRects.clear();
        it->m_textRects.clear();

        if (it->m_data.m_code) {
            startCodeX = layout.x();
            startCodeY = layout.y();
        }

        codeWidth = 0;

        for (const auto &tt : std::as_const(it->m_splittedText)) {
            auto text = tt.first;

            auto w = option.fontMetrics.horizontalAdvance(text);

            if (!layout.isFit(w)) {
                if (it->m_data.m_code && codeWidth > 0) {
                    it->m_backgroundRects.append(layout.codeRect(startCodeX, startCodeY, codeWidth));
                }

                if (layout.isWider(w)) {
                    layout.moveToNewLineIfSomethingThere();

                    QString str, tmpStr = text;

                    while (!tmpStr.isEmpty()) {
                        while (layout.isWider(w)) {
                            str.prepend(tmpStr.back());
                            tmpStr.removeLast();

                            w = option.fontMetrics.horizontalAdvance(tmpStr);
                        }

                        it->m_textRects.append(std::make_pair(layout.rect(w),
                                                              UnitData{tmpStr, it->m_data.m_code, tt.second}));

                        if (it->m_data.m_code) {
                            it->m_backgroundRects.append(layout.rect(w));
                        }

                        layout.moveToNewLine();

                        tmpStr = str;
                        str.clear();
                        text = tmpStr;

                        w = option.fontMetrics.horizontalAdvance(tmpStr);

                        if (!layout.isWider(w)) {
                            break;
                        }
                    }
                }

                layout.moveToNewLineIfSomethingThere();

                startCodeX = layout.x();
                startCodeY = layout.y();
                codeWidth = 0;
            } else if (codeWidth > 0) {
                codeWidth += sw;
            }

            if (it->m_data.m_code) {
                codeWidth += w;
            }

            it->m_textRects.append(std::make_pair(layout.rect(w), UnitData{text, it->m_data.m_code, tt.second}));

            layout.moveXBy(w + sw);
        }

        if (it->m_data.m_code && codeWidth > 0) {
            it->m_backgroundRects.append(layout.codeRect(startCodeX, startCodeY, codeWidth));
        }
    }

    return {layout.width(), layout.height()};
}

void WordWrapItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QStyledItemDelegate::paint(painter, option, index);

    const auto &data = m_model->stringData(m_sortModel->mapToSource(index));

    if (data.isEmpty()) {
        return;
    }

    const auto x = option.rect.x();
    const auto y = option.rect.y();

    const auto descend = option.fontMetrics.descent();

    for (const auto &d : std::as_const(data)) {
        painter->setPen(Qt::NoPen);
        painter->setBrush(s_codeBackgroundColor);

        for (const auto &r : std::as_const(d.m_backgroundRects)) {
            auto tmp = r;
            tmp.moveTo(x + tmp.x(), y + tmp.y());

            painter->drawRoundedRect(tmp.adjusted(0, 1, 0, -1), 3, 3);
        }

        for (const auto &p : std::as_const(d.m_textRects)) {
            if (p.second.m_code) {
                painter->setPen(s_codeColor);
            } else {
                painter->setPen(option.palette.color(QPalette::Text));
            }

            if (p.second.m_isRightToLeft) {
                painter->setLayoutDirection(Qt::RightToLeft);
            } else {
                painter->setLayoutDirection(Qt::LeftToRight);
            }

            auto tmp = p.first;
            tmp.moveTo(x + tmp.x(), y + tmp.y());

            painter->drawText(tmp.bottomLeft() - QPoint(0, descend), p.second.m_text);
        }
    }
}

void WordWrapItemDelegate::initStyleOption(QStyleOptionViewItem *option, const QModelIndex &index) const
{
    QStyledItemDelegate::initStyleOption(option, index);
    option->text = QString();
}

} /* namespace MdEditor */

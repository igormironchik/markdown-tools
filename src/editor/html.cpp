/*
    SPDX-FileCopyrightText: 2026 Igor Mironchik <igor.mironchik@gmail.com>
    SPDX-License-Identifier: GPL-3.0-or-later
*/

// md-editor include.
#include "html.h"

// shared include.
#include "emoji.h"
#include "emoji_parser.h"

// Qt include.
#include <QScopedValueRollback>

namespace MdEditor
{

//
// HtmlConv
//

QString HtmlConv::getId(MD::Item *item) const
{
    const auto tmp = MD::details::HtmlVisitor::getId(item);

    if (!tmp.isEmpty()) {
        m_id = tmp;
        m_top = item;
        m_count = 0;
        m_lastCount = 0;
    }

    return tmp;
}

void HtmlConv::openStyle(const typename MD::ItemWithOpts::Styles &styles)
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

void HtmlConv::closeStyle(const typename MD::ItemWithOpts::Styles &styles)
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

void HtmlConv::onAddLineEnding()
{
    MD::details::HtmlVisitor::onAddLineEnding();

    if (!m_id.isEmpty() && !m_skipSpan) {
        printLineId(nullptr);
    }
}

void HtmlConv::onParagraph(MD::Paragraph *p,
                           bool wrap,
                           bool skipOpeningWrap)
{
    if (!m_justCollectFootnoteRefs) {
        printLineId(p);
    }

    MD::details::HtmlVisitor::onParagraph(p, wrap, skipOpeningWrap);

    clearTop(p);
}

void HtmlConv::onHeading(MD::Heading *h)
{
    if (!m_justCollectFootnoteRefs) {
        printLineId(h);
    }

    MD::details::HtmlVisitor::onHeading(h);

    clearTop(h);
}

void HtmlConv::printLineId(MD::Item *item)
{
    if (item != m_top) {
        if (item && m_top) {
            m_count = item->startLine() - m_top->startLine();
        } else {
            ++m_count;
        }
    }

    if (m_lastCount != m_count) {
        m_html.push_back(QStringLiteral("<span id=\"%1-%2\"></span>").arg(m_id, QString::number(m_count)));
        m_lastCount = m_count;
    }
}

void HtmlConv::clearTop(MD::Item *item)
{
    if (item == m_top) {
        m_top = nullptr;
    }
}

static const QChar s_newLine = QLatin1Char('\n');

void HtmlConv::onCode(MD::Code *c)
{
    if (!m_justCollectFootnoteRefs) {
        printLineId(c);

        const auto lines = c->text().split(s_newLine);

        m_html.push_back(QStringLiteral("\n"));
        m_html.push_back(QStringLiteral("<pre"));
        printId(c);
        m_html.push_back(QStringLiteral(" data-lines-count=\""));
        m_html.push_back(QString::number(lines.size()));
        m_html.push_back(QStringLiteral("\"><code"));

        if (!c->syntax().isEmpty()) {
            m_html.push_back(QStringLiteral(" class=\"language-"));
            m_html.push_back(c->syntax());
            m_html.push_back(QStringLiteral("\""));
        }

        m_html.push_back(QStringLiteral(">"));

        QScopedValueRollback rollback(m_skipSpan, true);

        for (const auto &line : lines) {
            m_html.push_back(prepareTextForHtml(line));

            onAddLineEnding();
        }

        m_html.push_back(QStringLiteral("</code></pre>"));
        m_html.push_back(QStringLiteral("\n"));

        clearTop(c);
    }
}

void HtmlConv::onBlockquote(MD::Blockquote *b)
{
    if (!m_justCollectFootnoteRefs) {
        printLineId(b);
    }

    MD::details::HtmlVisitor::onBlockquote(b);

    clearTop(b);
}

void HtmlConv::onList(MD::List *l)
{
    if (!m_justCollectFootnoteRefs) {
        printLineId(l);
    }

    MD::details::HtmlVisitor::onList(l);

    clearTop(l);
}

void HtmlConv::onTable(MD::Table *t)
{
    if (!m_justCollectFootnoteRefs) {
        m_html.push_back(QStringLiteral("\n"));
        printLineId(t);
    }

    if (!t->isEmpty()) {
        if (!m_justCollectFootnoteRefs) {
            m_html.push_back(QStringLiteral("<table"));
            printId(t);
            m_html.push_back(QStringLiteral("><thead><tr>\n"));
        }

        int columns = 0;

        for (auto th = (*t->rows().cbegin())->cells().cbegin(), last = (*t->rows().cbegin())->cells().cend();
             th != last;
             ++th) {
            if (!m_justCollectFootnoteRefs) {
                m_html.push_back(QStringLiteral("<th"));
                m_html.push_back(tableAlignmentToHtml(t->columnAlignment(columns)));
                m_html.push_back(QStringLiteral(" dir=\"auto\">\n"));
            }

            this->onTableCell(th->get());

            if (!m_justCollectFootnoteRefs) {
                m_html.push_back(QStringLiteral("\n</th>\n"));
            }

            ++columns;
        }

        if (!m_justCollectFootnoteRefs) {
            m_html.push_back(QStringLiteral("</tr></thead><tbody>\n"));
        }

        ++m_count;

        for (auto r = std::next(t->rows().cbegin()), rlast = t->rows().cend(); r != rlast; ++r) {
            if (!m_justCollectFootnoteRefs) {
                m_html.push_back(QStringLiteral("<tr>\n"));
            }

            int i = 0;

            bool first = true;

            for (auto c = (*r)->cells().cbegin(), clast = (*r)->cells().cend(); c != clast; ++c) {
                if (!m_justCollectFootnoteRefs) {
                    m_html.push_back(QStringLiteral("\n<td"));
                    m_html.push_back(tableAlignmentToHtml(t->columnAlignment(i)));
                    m_html.push_back(QStringLiteral(" dir=\"auto\">\n"));

                    if (first) {
                        printLineId(nullptr);
                        first = false;
                    }
                }

                QScopedValueRollback rollback(m_skipSpan, true);

                this->onTableCell(c->get());

                if (!m_justCollectFootnoteRefs) {
                    m_html.push_back(QStringLiteral("\n</td>\n"));
                }

                ++i;

                if (i == columns) {
                    break;
                }
            }

            if (!m_justCollectFootnoteRefs) {
                for (; i < columns; ++i) {
                    m_html.push_back(QStringLiteral("<td dir=\"auto\"></td>"));
                }

                m_html.push_back(QStringLiteral("\n</tr>\n"));
            }
        }

        if (!m_justCollectFootnoteRefs) {
            m_html.push_back(QStringLiteral("</tbody></table>"));
        }
    }

    if (!m_justCollectFootnoteRefs) {
        m_html.push_back(QStringLiteral("\n"));
    }

    clearTop(t);
}

void HtmlConv::onHorizontalLine(MD::HorizontalLine *l)
{
    if (!m_justCollectFootnoteRefs) {
        printLineId(l);
    }

    MD::details::HtmlVisitor::onHorizontalLine(l);

    clearTop(l);
}

void HtmlConv::onListItem(MD::ListItem *i,
                          bool first,
                          bool skipOpeningWrap)
{
    if (!m_justCollectFootnoteRefs) {
        printLineId(i);
    }

    MD::details::HtmlVisitor::onListItem(i, first, skipOpeningWrap);

    clearTop(i);
}
void HtmlConv::onUserDefined(MD::Item *item)
{
    if (!m_justCollectFootnoteRefs) {
        if (item->type() == MdShared::EmojiItem::emojiType()) {
            printLineId(item);

            auto emoji = static_cast<MdShared::EmojiItem *>(item);

            openStyle(emoji->openStyles());

            m_html.push_back(s_emojiMap[emoji->emojiName()]);

            closeStyle(emoji->closeStyles());

            clearTop(item);
        }
    }
}

QString HtmlConv::prepareTextForHtml(const QString &t)
{
    auto tmp = MD::details::HtmlVisitor::prepareTextForHtml(t);
    tmp.replace(QLatin1Char('$'), QStringLiteral("<span>$</span>"));

    return tmp;
}

} /* namespace MdEditor */

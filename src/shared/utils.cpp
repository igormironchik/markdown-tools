/*
    SPDX-FileCopyrightText: 2025 Igor Mironchik <igor.mironchik@gmail.com>
    SPDX-License-Identifier: GPL-3.0-or-later
*/

// shared include.
#include "utils.h"
#include "emphasis.h"
#include "syntax.h"

// Qt include.
#include <QtResource>

// md4qt inclide.
#include <md4qt/src/asterisk_emphasis_parser.h>
#include <md4qt/src/atx_heading_parser.h>
#include <md4qt/src/autolink_parser.h>
#include <md4qt/src/blockquote_parser.h>
#include <md4qt/src/emphasis_parser.h>
#include <md4qt/src/fenced_code_parser.h>
#include <md4qt/src/footnote_parser.h>
#include <md4qt/src/gfm_autolink_parser.h>
#include <md4qt/src/hard_line_break_parser.h>
#include <md4qt/src/html_parser.h>
#include <md4qt/src/indented_code_parser.h>
#include <md4qt/src/inline_code_parser.h>
#include <md4qt/src/inline_html_parser.h>
#include <md4qt/src/inline_math_parser.h>
#include <md4qt/src/link_image_parser.h>
#include <md4qt/src/list_parser.h>
#include <md4qt/src/paragraph_parser.h>
#include <md4qt/src/setext_heading_parser.h>
#include <md4qt/src/strikethrough_emphasis_parser.h>
#include <md4qt/src/table_parser.h>
#include <md4qt/src/thematic_break_parser.h>
#include <md4qt/src/underline_emphasis_parser.h>
#include <md4qt/src/yaml_parser.h>

void initSharedResources()
{
    Q_INIT_RESOURCE(qt);
    Q_INIT_RESOURCE(icon);
    Q_INIT_RESOURCE(tr);
    MdShared::Syntax::init();
}

bool isRightToLeft(const QChar &ch)
{
    switch (ch.direction()) {
    case QChar::DirAL:
    case QChar::DirAN:
    case QChar::DirR:
        return true;

    default:
        return false;
    }
}

QVector<QPair<QString,
              bool>>
splitString(const QString &str,
            bool skipSpaces)
{
    QVector<QPair<QString, bool>> res;
    qsizetype first = 0;
    bool space = false;
    bool previousRTL = false;

    for (qsizetype i = 0; i < str.length(); ++i) {
        if (str[i].isSpace() && !space) {
            if (first < i) {
                const auto word = str.sliced(first, i - first);
                bool rtl = false;

                if (word[0].direction() != QChar::DirON) {
                    previousRTL = isRightToLeft(word[0]);
                }

                rtl = previousRTL;

                res.append({word, rtl});
            }

            if (!skipSpaces) {
                res.append({QStringLiteral(" "), false});
            }

            space = true;
        } else {
            if (space) {
                first = i;
            }

            space = false;
        }
    }

    if (!space && first < str.length()) {
        const auto word = str.sliced(first);
        res.append({word, isRightToLeft(word[0])});
    }

    return res;
}

void orderWords(QVector<QPair<QString,
                              bool>> &text)
{
    qsizetype start = -1;
    qsizetype end = -1;

    auto reverseItems = [](qsizetype start, qsizetype end, QVector<QPair<QString, bool>> &data) {
        if (start > -1 && end > start) {
            while (end - start > 0) {
                data.swapItemsAt(start, end);
                ++start;
                --end;
            }
        }
    };

    for (qsizetype i = 0; i < text.size(); ++i) {
        if (text[i].first != QStringLiteral(" ")) {
            if (!text[i].second) {
                if (start == -1) {
                    start = i;
                    end = i;
                } else {
                    end = i;
                }
            } else {
                reverseItems(start, end, text);

                start = -1;
                end = -1;
            }
        }
    }

    reverseItems(start, end, text);
}

void setPlugins(MD::Parser &parser,
                const MdShared::PluginsCfg &cfg)
{
    MD::Parser::InlineParsers inlineParsers;

    MD::Parser::appendInlineParser<MD::InlineCodeParser>(inlineParsers);

    auto linkParser = MD::Parser::appendInlineParser<MD::LinkImageParser>(inlineParsers);

    MD::Parser::appendInlineParser<MD::AutolinkParser>(inlineParsers);
    MD::Parser::appendInlineParser<MD::InlineHtmlParser>(inlineParsers);
    MD::Parser::appendInlineParser<MD::InlineMathParser>(inlineParsers);
    MD::Parser::appendInlineParser<MD::AsteriskEmphasisParser>(inlineParsers);
    MD::Parser::appendInlineParser<MD::UnderlineEmphasisParser>(inlineParsers);
    MD::Parser::appendInlineParser<MD::StrikethroughEmphasisParser>(inlineParsers);

    if (cfg.m_sup.m_on) {
        inlineParsers.append(QSharedPointer<MdShared::SupEmphasisParser>::create(cfg.m_sup.m_delimiter));
    }

    if (cfg.m_sub.m_on) {
        inlineParsers.append(QSharedPointer<MdShared::SubEmphasisParser>::create(cfg.m_sub.m_delimiter));
    }

    if (cfg.m_mark.m_on) {
        inlineParsers.append(QSharedPointer<MdShared::HighlightEmphasisParser>::create(cfg.m_mark.m_delimiter));
    }

    inlineParsers.append(QSharedPointer<MD::GfmAutolinkParser>::create(linkParser));

    MD::Parser::appendInlineParser<MD::HardLineBreakParser>(inlineParsers);

    MD::Parser::BlockParsers blockParsers;

    if (cfg.m_yamlEnabled) {
        MD::Parser::appendBlockParser<MD::YAMLParser>(blockParsers, &parser);
    }

    MD::Parser::appendBlockParser<MD::BlockquoteParser>(blockParsers, &parser);
    MD::Parser::appendBlockParser<MD::SetextHeadingParser>(blockParsers, &parser);
    MD::Parser::appendBlockParser<MD::ThematicBreakParser>(blockParsers, &parser);
    MD::Parser::appendBlockParser<MD::ListParser>(blockParsers, &parser);
    MD::Parser::appendBlockParser<MD::ATXHeadingParser>(blockParsers, &parser);
    MD::Parser::appendBlockParser<MD::FencedCodeParser>(blockParsers, &parser);
    MD::Parser::appendBlockParser<MD::HTMLParser>(blockParsers, &parser);
    MD::Parser::appendBlockParser<MD::IndentedCodeParser>(blockParsers, &parser);
    MD::Parser::appendBlockParser<MD::FootnoteParser>(blockParsers, &parser);
    MD::Parser::appendBlockParser<MD::TableParser>(blockParsers, &parser);
    MD::Parser::appendBlockParser<MD::ParagraphParser>(blockParsers, &parser);

    parser.setBlockParsers(blockParsers);
    parser.setInlineParsers(inlineParsers);
}

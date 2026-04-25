/*
    SPDX-FileCopyrightText: 2026 Igor Mironchik <igor.mironchik@gmail.com>
    SPDX-License-Identifier: GPL-3.0-or-later
*/

#pragma once

// md4qt include.
#include <md4qt/src/doc.h>
#include <md4qt/src/inline_parser.h>

namespace MdShared
{

//
// EmojiItem
//

class EmojiItem : public MD::ItemWithOpts
{
public:
    EmojiItem() = default;
    ~EmojiItem() override = default;

    static constexpr MD::ItemType emojiType()
    {
        return static_cast<MD::ItemType>(static_cast<int>(MD::ItemType::UserDefined) + 2);
    }

    MD::ItemType type() const override;
    QSharedPointer<MD::Item> clone(MD::Document *doc = nullptr) const override;

    const QString &emojiName() const;
    void setEmojiName(const QString &name);

private:
    QString m_emojiName;
}; // class EmojiItem

//
// EmojiParser
//

//! Emoji inline parser.
class EmojiParser : public MD::InlineParser
{
public:
    EmojiParser() = default;
    ~EmojiParser() override = default;

    bool check(MD::Line &line,
               MD::ParagraphStream &stream,
               MD::InlineContext &ctx,
               QSharedPointer<MD::Document> doc,
               const QString &path,
               const QString &fileName,
               QStringList &linksToParse,
               MD::Parser &parser,
               const MD::ReverseSolidusHandler &rs) override;

    QString startDelimiterSymbols() const override;
}; // class EmojiParser

} /* namespace MdShared */

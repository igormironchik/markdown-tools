/*
    SPDX-FileCopyrightText: 2026 Igor Mironchik <igor.mironchik@gmail.com>
    SPDX-License-Identifier: GPL-3.0-or-later
*/

// Qt include.
#include <QCoreApplication>
#include <QDebug>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QObject>
#include <QTextStream>

// C++ include.
#include <algorithm>

inline QString toString(const QString &s)
{
    const auto chars = s.split(QLatin1Char('-'), Qt::SkipEmptyParts);

    QString res;

    for (const auto &ch : std::as_const(chars)) {
        res.append(QStringLiteral("\\U"));
        res.append(QString(8 - ch.length(), QLatin1Char('0')));
        res.append(ch);
    }

    return res;
}

int main(int argc,
         char **argv)
{
    QCoreApplication app(argc, argv);

    QFile json(QStringLiteral("./emoji.json"));

    if (json.open(QIODevice::ReadOnly)) {
        QJsonParseError er;
        const auto doc = QJsonDocument::fromJson(json.readAll(), &er);
        json.close();

        if (!doc.isNull()) {
            QFile out(QStringLiteral("./emoji.h"));

            if (out.open(QIODevice::WriteOnly)) {
                QTextStream stream(&out);
                QStringList emojiNames;

                // REUSE-IgnoreStart
                stream << QStringLiteral(
                    "/*\n"
                    "    SPDX-FileCopyrightText: 2026 Igor Mironchik <igor.mironchik@gmail.com>\n"
                    "    SPDX-License-Identifier: GPL-3.0-or-later\n"
                    "*/\n\n");
                // REUSE-IgnoreEnd

                stream << QStringLiteral("#include <QHash>\n#include <QStringList>\n\n");
                stream << QStringLiteral("static const QHash<QString, QString> s_emojiMap = {\n");

                auto first = true;

                const auto array = doc.array();

                for (auto it = array.cbegin(), last = array.cend(); it != last; ++it) {
                    const auto object = it->toObject();

                    const auto unicodeIt = object.constFind(QStringLiteral("unified"));
                    const auto namesIt = object.constFind(QStringLiteral("short_names"));

                    if (unicodeIt != object.constEnd() && namesIt != object.constEnd()) {
                        const auto names = namesIt->toArray();
                        const auto unicode = toString(unicodeIt.value().toString());

                        if (!names.isEmpty()) {
                            for (auto nit = names.cbegin(), nlast = names.cend(); nit != nlast; ++nit) {
                                if (!first) {
                                    stream << QStringLiteral(",\n");
                                } else {
                                    first = false;
                                }

                                emojiNames << nit->toString();

                                stream
                                    << QStringLiteral("    {QStringLiteral(\"")
                                    << nit->toString()
                                    << QStringLiteral("\"), QString::fromUcs4(U\"")
                                    << unicode
                                    << QStringLiteral("\")}");
                            }
                        }
                    }
                }

                emojiNames.append(QStringLiteral("robot"));
                emojiNames.append(QStringLiteral("metal"));
                emojiNames.append(QStringLiteral("fu"));

                stream << QStringLiteral(",\n");
                stream << QStringLiteral("    {QStringLiteral(\"robot\"), QString::fromUcs4(U\"\\U0001f916\")},\n");
                stream << QStringLiteral("    {QStringLiteral(\"metal\"), QString::fromUcs4(U\"\\U0001f918\")},\n");
                stream << QStringLiteral("    {QStringLiteral(\"fu\"), QString::fromUcs4(U\"\\U0001f595\")}");
                stream << QStringLiteral("};\n\n");

                std::sort(emojiNames.begin(), emojiNames.end());

                stream << QStringLiteral("static const QStringList s_emojiKeys = {\n");
                first = true;

                for (const auto &name : std::as_const(emojiNames)) {
                    if (!first) {
                        stream << QStringLiteral(",\n");
                    } else {
                        first = false;
                    }

                    stream << QStringLiteral("    {QStringLiteral(\"") << name << QStringLiteral("\")}");
                }

                stream << QStringLiteral("};\n");

                out.close();
            } else {
                qDebug() << "Unable to create emoji.h";

                return 1;
            }
        } else {
            qDebug() << er.errorString();

            return 1;
        }
    } else {
        qDebug() << "Unable to open emoji.json";

        return 1;
    }

    return 0;
}

/*
    SPDX-FileCopyrightText: 2020 Alexander Lohnau <alexander.lohnau@gmx.de>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include <QCoreApplication>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QString>

static QStringList jsonArrayToStringList(const QJsonArray &array)
{
    QStringList list;
    for (const QJsonValue &value : array) {
        list.append(value.toString());
    }
    return list;
}

static void writeKeyValue(QFile *file, const QString &key, const QString &value)
{
    file->write((key + u'=' + value + u'\n').toUtf8());
}

static void writeKeyValue(QFile *file, const QString &key, const QStringList &list)
{
    writeKeyValue(file, key, list.join(u';'));
}

int main(int argc, char **argv)
{
    Q_ASSERT(argc == 3);
    // Use QCoreApplication to parse arguments to handle encoding correctly
    QCoreApplication app(argc, argv);
    QString fileName = app.arguments().at(1);

    QFile file(fileName);
    bool isOpen = file.open(QIODevice::ReadOnly);
    if (!isOpen) {
        qCritical() << "Could not open file" << fileName;
        exit(1);
    }
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    Q_ASSERT(doc.isObject());

    const QJsonObject metadata = doc.object();
    const QJsonObject kplugin = metadata.value(QLatin1String("KPlugin")).toObject();
    const QLatin1String namePrefix("Name");

    QFile out(app.arguments().at(2));
    out.open(QIODevice::WriteOnly);
    out.write("[Desktop Entry]\n");
    out.write("Type=Application\n");
    out.write("NoDisplay=true\n");
    out.write("X-KDE-AliasFor=systemsettings\n");

    const QString showOnlyOnQtPlatformsKey = QStringLiteral("X-KDE-OnlyShowOnQtPlatforms");
    if (auto it = metadata.find(showOnlyOnQtPlatformsKey); it != metadata.end()) {
        writeKeyValue(&out, showOnlyOnQtPlatformsKey, jsonArrayToStringList(it->toArray()));
    }

    QString executableProgram = QStringLiteral("systemsettings ");
    if (!metadata.contains(QLatin1String("X-KDE-System-Settings-Parent-Category"))) {
        executableProgram = QStringLiteral("kcmshell6 ");
    }

    const QString exec = QLatin1String("Exec=") + executableProgram + QFileInfo(fileName).baseName() + QLatin1Char('\n');
    out.write(exec.toUtf8());
    const QString icon = QLatin1String("Icon=") + kplugin.value(QLatin1String("Icon")).toString() + QLatin1Char('\n');
    out.write(icon.toUtf8());

    for (auto it = kplugin.begin(), end = kplugin.end(); it != end; ++it) {
        const QString key = it.key();
        if (key.startsWith(namePrefix)) {
            writeKeyValue(&out, key, it.value().toString());
        }
    }
}

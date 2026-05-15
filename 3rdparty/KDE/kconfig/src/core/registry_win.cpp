/*
    SPDX-FileCopyrightText: 2022 g10 Code GmbH
    SPDX-FileContributor: Andre Heinecke <aheinecke@gnupg.com>
    SPDX-FileContributor: Tobias Fella <tobias.fella@gnupg.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "registry_win_p.h"

#include <windows.h>

#include <QSettings>

enum Registry {
    HKLM,
    HKCU,
};

static void parseRegValues(const QString &groupName, QSettings &settings, KEntryMap &entryMap, bool groupImmutable)
{
    for (auto &key : settings.childKeys()) {
        KEntryMap::EntryOptions entryOptions = KEntryMap::EntryDefault;
        const auto value = settings.value(key).toString().toStdWString();
        const auto requiredSize = ExpandEnvironmentStringsW(value.c_str(), nullptr, 0);
        std::vector<char16_t> expanded(requiredSize, char16_t(0));
        ExpandEnvironmentStringsW(value.c_str(), (LPWSTR)expanded.data(), expanded.size());
        if (key.endsWith(QStringLiteral("[$i]"))) {
            key.chop(4);
            entryOptions |= KEntryMap::EntryImmutable;
        }
        if (groupImmutable) {
            entryOptions |= KEntryMap::EntryImmutable;
        }
        key = key.replace(QLatin1Char('\\'), QLatin1Char('/'));
        if (entryMap.getEntryOption(groupName, key.toUtf8(), KEntryMap::SearchDefaults, KEntryMap::EntryImmutable)) {
            continue;
        }
        entryMap.setEntry(groupName, key.toUtf8(), QString::fromUtf16(&expanded[0]), entryOptions);
    }
}

static void parseRegSubkeys(const QString &baseGroup, QSettings &settings, KEntryMap &entryMap, bool immutable)
{
    parseRegValues(baseGroup, settings, entryMap, immutable);

    for (auto &group : settings.childGroups()) {
        bool groupImmutable = immutable;
        QString groupWithoutSuffix = group;
        if (group.endsWith(QStringLiteral("[$i]"))) {
            groupWithoutSuffix.chop(4);
            groupImmutable = true;
        }
        settings.beginGroup(group);
        QString groupName = (baseGroup == QStringLiteral("<default>") ? QString() : (baseGroup + QLatin1Char('\x1d'))) + groupWithoutSuffix;
        parseRegSubkeys(groupName, settings, entryMap, groupImmutable);
        settings.endGroup();
        entryMap.setEntry(groupName, {}, QByteArray(), groupImmutable ? KEntryMap::EntryImmutable : KEntryMap::EntryOptions());
    }
}

static void parseRegistry(const QString &regKey, KEntryMap &entryMap, Registry registry)
{
    QSettings settings((registry == HKCU ? QStringLiteral("HKEY_CURRENT_USER\\SOFTWARE") : QStringLiteral("HKEY_LOCAL_MACHINE\\SOFTWARE")),
                       QSettings::NativeFormat);
    settings.beginGroup(regKey);
    parseRegSubkeys(QStringLiteral("<default>"), settings, entryMap, false);
}

void WindowsRegistry::parse(const QString &regKey, KEntryMap &entryMap)
{
    parseRegistry(regKey, entryMap, HKLM);
    parseRegistry(regKey, entryMap, HKCU);
}

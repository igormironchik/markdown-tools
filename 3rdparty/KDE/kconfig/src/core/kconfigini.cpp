/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2006, 2007 Thomas Braxton <kde.braxton@gmail.com>
    SPDX-FileCopyrightText: 1999 Preston Brown <pbrown@kde.org>
    SPDX-FileCopyrightText: 1997-1999 Matthias Kalle Dalheimer <kalle@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "kconfigini_p.h"

#include "kconfig_core_log_settings.h"
#include "kconfigdata_p.h"
#include "kconfiginibackendreader_p.h"

using namespace Qt::StringLiterals;

KCONFIGCORE_EXPORT bool kde_kiosk_exception = false; // flag to disable kiosk restrictions

namespace
{
QString warningProlog(const KConfigIniBackendAbstractDevice *device, int line)
{
    // %2 then %1 i.e. int before QString, so that the QString is last
    // This avoids a wrong substitution if the fileName itself contains %1
    return u"KConfigIni: In file %2, line %1:"_s.arg(line).arg(device->id());
}
} // anonymous namespace

KConfigIniBackend::KConfigIniBackend(std::unique_ptr<KConfigIniBackendAbstractDevice> deviceInterface)
    : mDeviceInterface(std::move(deviceInterface)) { };

KConfigIniBackend::ParseInfo KConfigIniBackend::parseConfig(const QByteArray &currentLocale, KEntryMap &entryMap, ParseOptions options)
{
    return parseConfig(currentLocale, entryMap, options, false);
}

// merging==true is the merging that happens at the beginning of writeConfig:
// merge changes in the on-disk file with the changes in the KConfig object.
KConfigIniBackend::ParseInfo KConfigIniBackend::parseConfig(const QByteArray &currentLocale, KEntryMap &entryMap, ParseOptions options, bool merging)
{
    if (!mDeviceInterface->isDeviceReadable()) {
        return ParseOk;
    }

    auto openResult = mDeviceInterface->open();
    if (openResult.shouldHaveDevice && !openResult.device) {
        return ParseOpenError;
    }
    auto file = std::move(openResult.device);
    if (!file) {
        return ParseOk;
    }

    if (file->size() == 0) {
        return ParseOk;
    }

    QList<QString> immutableGroups;

    bool fileOptionImmutable = false;
    bool groupOptionImmutable = false;
    bool groupSkip = false;

    uint lineNo = 0;

    const int langIdx = currentLocale.indexOf('_');
    const QByteArray currentLanguage = langIdx >= 0 ? currentLocale.left(langIdx) : currentLocale;

    QString currentGroup = u"<default>"_s;
    bool bDefault = options & ParseDefaults;
    bool allowExecutableValues = options & ParseExpansions;

    const uint MAX_ERRORS = 100;
    uint errorCount = 0;

    const qint64 initialBufferSize = std::min(file->size(), qint64(128 * 1024) /* 128 KB */);
    const uint maximumSizeWithoutNewLine = 1.5 * 1024 * 1024; // 1.5 MB
    const uint defaultgroupNameBufferSize = 512; // should be a rather big fit for common group names

    QByteArray groupNameBuffer; // reused allocated buffer to read group names, each processed into QString afterwards
    groupNameBuffer.reserve(defaultgroupNameBufferSize);

    QByteArray buffer(initialBufferSize, Qt::Uninitialized);
    while (!file->atEnd() && errorCount < MAX_ERRORS) {
        auto res = file->readLineInto(&buffer, maximumSizeWithoutNewLine);
        if (!res) {
            qCWarning(KCONFIG_CORE_LOG) << "Couldn't find a single line in " << mDeviceInterface->id() << " after reading" << (maximumSizeWithoutNewLine)
                                        << "bytes.";
        }
        QByteArrayView line = buffer;
        line = line.trimmed();
        ++lineNo;

        // skip empty lines and lines beginning with '#'
        if (line.isEmpty() || line.at(0) == '#') {
            continue;
        }

        if (line.at(0) == '[') { // found a group
            groupOptionImmutable = fileOptionImmutable;

            groupNameBuffer.resize(0); // drop content, but keep allocated capacity
            int start = 1;
            int end = 0;
            do {
                end = start;
                for (;;) {
                    if (end == line.length()) {
                        qCWarning(KCONFIG_CORE_LOG) << warningProlog(mDeviceInterface.get(), lineNo) << "Invalid group header.";
                        // XXX maybe reset the current group here?
                        goto next_line;
                    }
                    if (line.at(end) == ']') {
                        break;
                    }
                    ++end;
                }
                /* clang-format off */
                if (end + 1 == line.length()
                    && start + 2 == end
                    && line.at(start) == '$'
                    && line.at(start + 1) == 'i') { /* clang-format on */
                    if (groupNameBuffer.isEmpty()) {
                        fileOptionImmutable = !kde_kiosk_exception;
                    } else {
                        groupOptionImmutable = !kde_kiosk_exception;
                    }
                } else {
                    if (!groupNameBuffer.isEmpty()) {
                        groupNameBuffer += '\x1d';
                    }
                    QByteArrayView namePart = line.mid(start, end - start);
                    printableToString(namePart, mDeviceInterface.get(), lineNo);
                    groupNameBuffer.append(namePart);
                }
            } while ((start = end + 2) <= line.length() && line.at(end + 1) == '[');
            currentGroup = QString::fromUtf8(groupNameBuffer);

            groupSkip = entryMap.getEntryOption(currentGroup, {}, {}, KEntryMap::EntryImmutable);

            if (groupSkip && !bDefault) {
                continue;
            }

            if (groupOptionImmutable)
            // Do not make the groups immutable until the entries from
            // this file have been added.
            {
                immutableGroups.append(currentGroup);
            }
        } else {
            if (groupSkip && !bDefault) {
                continue; // skip entry
            }

            QByteArrayView aKey;
            int eqpos = line.indexOf('=');
            if (eqpos < 0) {
                aKey = line;
                line = {};
            } else {
                QByteArrayView temp = line.left(eqpos);
                aKey = temp.trimmed();
                line = line.mid(eqpos + 1);
                line = line.trimmed();
            }
            if (aKey.isEmpty()) {
                qCWarning(KCONFIG_CORE_LOG) << warningProlog(mDeviceInterface.get(), lineNo) << "Invalid entry (empty key)";
                continue;
            }

            KEntryMap::EntryOptions entryOptions = {};
            if (groupOptionImmutable) {
                entryOptions |= KEntryMap::EntryImmutable;
            }

            QByteArrayView locale;
            int start;
            while ((start = aKey.lastIndexOf('[')) >= 0) {
                int end = aKey.indexOf(']', start);
                if (end < 0) {
                    errorCount++;
                    qCWarning(KCONFIG_CORE_LOG) << warningProlog(mDeviceInterface.get(), lineNo) << "Invalid entry (missing ']')";
                    goto next_line;
                } else if (end > start + 1 && aKey.at(start + 1) == '$') { // found option(s)
                    int i = start + 2;
                    while (i < end) {
                        switch (aKey.at(i)) {
                        case 'i':
                            if (!kde_kiosk_exception) {
                                entryOptions |= KEntryMap::EntryImmutable;
                            }
                            break;
                        case 'e':
                            if (allowExecutableValues) {
                                entryOptions |= KEntryMap::EntryExpansion;
                            }
                            break;
                        case 'd':
                            entryOptions |= KEntryMap::EntryDeleted;
                            aKey.truncate(start);
                            printableToString(aKey, mDeviceInterface.get(), lineNo);
                            entryMap.setEntry(currentGroup, aKey.toByteArray(), QByteArray(), entryOptions);
                            goto next_line;
                        default:
                            break;
                        }
                        ++i;
                    }
                } else { // found a locale
                    if (!locale.isNull()) {
                        errorCount++;
                        qCWarning(KCONFIG_CORE_LOG) << warningProlog(mDeviceInterface.get(), lineNo) << "Invalid entry (second locale!?)";
                        goto next_line;
                    }

                    locale = aKey.mid(start + 1, end - start - 1);
                }
                aKey.truncate(start);
            }
            if (eqpos < 0) { // Do this here after [$d] was checked
                errorCount++;
                qCWarning(KCONFIG_CORE_LOG) << warningProlog(mDeviceInterface.get(), lineNo) << "Invalid entry (missing '=')";
                continue;
            }
            printableToString(aKey, mDeviceInterface.get(), lineNo);
            if (!locale.isEmpty()) {
                if (locale != currentLocale && locale != currentLanguage) {
                    // backward compatibility. C == en_US
                    if (locale.at(0) != 'C' || currentLocale != "en_US") {
                        if (merging) {
                            entryOptions |= KEntryMap::EntryRawKey;
                        } else {
                            goto next_line; // skip this entry if we're not merging
                        }
                    }
                }
            }

            if (options & ParseGlobal) {
                entryOptions |= KEntryMap::EntryGlobal;
            }
            if (bDefault) {
                entryOptions |= KEntryMap::EntryDefault;
            }
            if (!locale.isNull()) {
                entryOptions |= KEntryMap::EntryLocalized;
                if (locale.indexOf('_') != -1) {
                    entryOptions |= KEntryMap::EntryLocalizedCountry;
                }
            }
            if (!printableToString(line, mDeviceInterface.get(), lineNo)) {
                errorCount++;
            }
            if (entryOptions & KEntryMap::EntryRawKey) {
                QByteArray rawKey;
                rawKey.reserve(aKey.length() + locale.length() + 2);
                rawKey.append(aKey);
                rawKey.append('[').append(locale).append(']');
                entryMap.setEntry(currentGroup, rawKey, line.toByteArray(), entryOptions);
            } else {
                entryMap.setEntry(currentGroup, aKey.toByteArray(), line.toByteArray(), entryOptions);
            }
        }
    next_line:
        continue;
    }

    if (errorCount > MAX_ERRORS) {
        qCWarning(KCONFIG_CORE_LOG) << "Too many errors in file" << mDeviceInterface->id();
        return ParseOpenError;
    }

    // now make sure immutable groups are marked immutable
    for (const QString &group : std::as_const(immutableGroups)) {
        entryMap.setEntry(group, QByteArray(), QByteArray(), KEntryMap::EntryImmutable);
    }

    return fileOptionImmutable ? ParseImmutable : ParseOk;
}

void KConfigIniBackend::writeEntries(const QByteArray &locale, QIODevice &file, const KEntryMap &map, bool defaultGroup, bool primaryGroup, bool &firstEntry)
{
    QString currentGroup;
    bool groupIsImmutable = false;
    for (const auto &[key, entry] : map) {
        // Either process the default group or all others
        if ((key.mGroup != u"<default>"_s) == defaultGroup) {
            continue; // skip
        }
        // Either process the primary group or all others
        if ((mPrimaryGroup.isNull() || key.mGroup != mPrimaryGroup) == primaryGroup) {
            continue; // skip
        }

        // the only thing we care about groups is, is it immutable?
        if (key.mKey.isNull()) {
            groupIsImmutable = entry.bImmutable;
            continue; // skip
        }

        const KEntry &currentEntry = entry;
        if (!defaultGroup && currentGroup != key.mGroup) {
            if (!firstEntry) {
                file.putChar('\n');
            }
            currentGroup = key.mGroup;
            for (int start = 0, end;; start = end + 1) {
                file.putChar('[');
                end = currentGroup.indexOf(QLatin1Char('\x1d'), start);
                if (end < 0) {
                    int cgl = currentGroup.length();
                    // Start has to be smaller than currentGroup length, or otherwise currentGroup.at()
                    // will assert: https://doc.qt.io/qt-6/qstring.html#at
                    if (cgl > start && cgl - start <= 10 && currentGroup.at(start) == QLatin1Char('$')) {
                        for (int i = start + 1; i < cgl; i++) {
                            const QChar c = currentGroup.at(i);
                            if (c < QLatin1Char('a') || c > QLatin1Char('z')) {
                                goto nope;
                            }
                        }
                        file.write("\\x24");
                        ++start;
                    }
                nope:
                    // TODO: make stringToPrintable also process QString, to save the conversion here and below
                    file.write(stringToPrintable(QStringView(currentGroup).mid(start).toUtf8(), GroupString));
                    file.putChar(']');
                    if (groupIsImmutable) {
                        file.write("[$i]", 4);
                    }
                    file.putChar('\n');
                    break;
                } else {
                    file.write(stringToPrintable(QStringView(currentGroup).mid(start, end - start).toUtf8(), GroupString));
                    file.putChar(']');
                }
            }
        }

        firstEntry = false;
        // it is data for a group

        if (key.bRaw) { // unprocessed key with attached locale from merge
            file.write(key.mKey);
        } else {
            file.write(stringToPrintable(key.mKey, KeyString)); // Key
            if (key.bLocal && locale != "C") { // 'C' locale == untranslated
                file.putChar('[');
                file.write(locale); // locale tag
                file.putChar(']');
            }
        }
        if (currentEntry.bDeleted) {
            if (currentEntry.bImmutable) {
                file.write("[$di]", 5); // Deleted + immutable
            } else {
                file.write("[$d]", 4); // Deleted
            }
        } else {
            if (currentEntry.bImmutable || currentEntry.bExpand) {
                file.write("[$", 2);
                if (currentEntry.bImmutable) {
                    file.putChar('i');
                }
                if (currentEntry.bExpand) {
                    file.putChar('e');
                }
                file.putChar(']');
            }
            file.putChar('=');
            file.write(stringToPrintable(currentEntry.mValue, ValueString));
        }
        file.putChar('\n');
    }
}

void KConfigIniBackend::writeEntries(const QByteArray &locale, QIODevice &file, const KEntryMap &map)
{
    bool firstEntry = true;

    // write default group
    writeEntries(locale, file, map, true, false, firstEntry);

    if (!mPrimaryGroup.isNull()) {
        // write the primary group - it needs to be written before all other groups
        writeEntries(locale, file, map, false, true, firstEntry);
    }

    // write all other groups
    writeEntries(locale, file, map, false, false, firstEntry);
}

bool KConfigIniBackend::writeConfig(const QByteArray &locale, KEntryMap &entryMap, WriteOptions options)
{
    Q_ASSERT(mDeviceInterface->isDeviceReadable());

    KEntryMap writeMap;
    const bool bGlobal = options & WriteGlobal;

    // First, reparse the file on disk, to merge our changes with the ones done by other apps
    // Store the result into writeMap.
    {
        ParseOptions opts = ParseExpansions;
        if (bGlobal) {
            opts |= ParseGlobal;
        }
        ParseInfo info = parseConfig(locale, writeMap, opts, true);
        if (info != ParseOk) { // either there was an error or the file became immutable
            return false;
        }
    }

    for (auto &[key, entry] : entryMap) {
        if (!key.mKey.isEmpty() && !entry.bDirty) { // not dirty, doesn't overwrite entry in writeMap. skips default entries, too.
            continue;
        }

        // only write entries that have the same "globality" as the file
        if (entry.bGlobal == bGlobal) {
            if (entry.bReverted && entry.bOverridesGlobal) {
                entry.bDeleted = true;
                writeMap[key] = entry;
            } else if (entry.bReverted) {
                writeMap.erase(key);
            } else if (!entry.bDeleted) {
                writeMap[key] = entry;
            } else {
                KEntryKey defaultKey = key;
                defaultKey.bDefault = true;
                if (entryMap.find(defaultKey) == entryMap.end() && !entry.bOverridesGlobal) {
                    writeMap.erase(key); // remove the deleted entry if there is no default
                    // qDebug() << "Detected as deleted=>removed:" << key.mGroup << key.mKey << "global=" << bGlobal;
                } else {
                    writeMap[key] = entry; // otherwise write an explicitly deleted entry
                    // qDebug() << "Detected as deleted=>[$d]:" << key.mGroup << key.mKey << "global=" << bGlobal;
                }
            }
            entry.bDirty = false;
        }
    }

    return mDeviceInterface->writeToDevice([this, &locale, &writeMap](auto &device) {
        writeEntries(locale, device, writeMap);
    });
}

bool KConfigIniBackend::isWritable() const
{
    return mDeviceInterface->canWriteToDevice();
}

QString KConfigIniBackend::nonWritableErrorMessage() const
{
    return tr("Configuration file \"%1\" not writable.\n").arg(mDeviceInterface->id());
}

void KConfigIniBackend::createEnclosing()
{
    mDeviceInterface->createEnclosingEntity();
}

KConfigBase::AccessMode KConfigIniBackend::accessMode() const
{
    if (!mDeviceInterface->isDeviceReadable()) {
        return KConfigBase::NoAccess;
    }

    if (isWritable()) {
        return KConfigBase::ReadWrite;
    }

    return KConfigBase::ReadOnly;
}

bool KConfigIniBackend::lock()
{
    Q_ASSERT(mDeviceInterface->isDeviceReadable());

    // Default staleLockTime is 30 seconds. Set it lower since KConfig is not expected to hold the
    // lock for long. The tryLockTimeout is set to staleLockTime*2+buffer, to cover the case when
    // the file modification date is in the future, and the fallback to prevent blocking forever.
    constexpr std::chrono::milliseconds tryLockTimeout = std::chrono::seconds{45};

    lockFile = mDeviceInterface->lockFile();

    if (!lockFile->tryLock(tryLockTimeout)) {
        qCWarning(KCONFIG_CORE_LOG) << "Failed to lock file" << lockFile->fileName() << "with error" << int(lockFile->error());
    }

    return lockFile->isLocked();
}

void KConfigIniBackend::unlock()
{
    lockFile.reset();
}

bool KConfigIniBackend::isLocked() const
{
    return lockFile && lockFile->isLocked();
}

namespace
{
// serialize an escaped byte at the end of @param data
// @param data should have room for 4 bytes
char *escapeByte(char *data, unsigned char s)
{
    static const char nibbleLookup[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'};
    *data++ = '\\';
    *data++ = 'x';
    *data++ = nibbleLookup[s >> 4];
    *data++ = nibbleLookup[s & 0x0f];
    return data;
}

// Struct that represents a multi-byte UTF-8 character.
// This struct is used to keep track of bytes that seem to be valid
// UTF-8.
struct Utf8Char {
public:
    unsigned char bytes[4];
    unsigned char count;
    unsigned char charLength;

    Utf8Char()
    {
        clear();
        charLength = 0;
    }
    void clear()
    {
        count = 0;
    }
    // Add a byte to the UTF8 character.
    // When an additional byte leads to an invalid character, return false.
    bool addByte(unsigned char b)
    {
        if (count == 0) {
            if (b > 0xc1 && (b & 0xe0) == 0xc0) {
                charLength = 2;
            } else if ((b & 0xf0) == 0xe0) {
                charLength = 3;
            } else if (b < 0xf5 && (b & 0xf8) == 0xf0) {
                charLength = 4;
            } else {
                return false;
            }
            bytes[0] = b;
            count = 1;
        } else if (count < 4 && (b & 0xc0) == 0x80) {
            if (count == 1) {
                if (charLength == 3 && bytes[0] == 0xe0 && b < 0xa0) {
                    return false; // overlong 3 byte sequence
                }
                if (charLength == 4) {
                    if (bytes[0] == 0xf0 && b < 0x90) {
                        return false; // overlong 4 byte sequence
                    }
                    if (bytes[0] == 0xf4 && b > 0x8f) {
                        return false; // Unicode value larger than U+10FFFF
                    }
                }
            }
            bytes[count++] = b;
        } else {
            return false;
        }
        return true;
    }
    // Return true if Utf8Char contains one valid character.
    bool isComplete() const
    {
        return count > 0 && count == charLength;
    }
    // Add the bytes in this UTF8 character in escaped form to data.
    char *escapeBytes(char *data)
    {
        for (unsigned char i = 0; i < count; ++i) {
            data = escapeByte(data, bytes[i]);
        }
        clear();
        return data;
    }
    // Add the bytes of the UTF8 character to a buffer.
    // Only call this if isComplete() returns true.
    char *writeUtf8(char *data)
    {
        for (unsigned char i = 0; i < count; ++i) {
            *data++ = bytes[i];
        }
        clear();
        return data;
    }
    // Write the bytes in the UTF8 character literally, or, if the
    // character is not complete, write the escaped bytes.
    // This is useful to handle the state that remains after handling
    // all bytes in a buffer.
    char *write(char *data)
    {
        if (isComplete()) {
            data = writeUtf8(data);
        } else {
            data = escapeBytes(data);
        }
        return data;
    }
};
}

QByteArray KConfigIniBackend::stringToPrintable(const QByteArray &aString, StringType type)
{
    const int len = aString.size();
    if (len == 0) {
        return aString;
    }

    QByteArray result; // Guesstimated that it's good to avoid data() initialization for a length of len*4
    result.resize(len * 4); // Maximum 4x as long as source string due to \x<ab> escape sequences
    const char *s = aString.constData();
    int i = 0;
    char *data = result.data();
    char *start = data;

    // Protect leading space
    if (s[0] == ' ' && type != GroupString) {
        *data++ = '\\';
        *data++ = 's';
        ++i;
    }
    Utf8Char utf8;

    for (; i < len; ++i) {
        switch (s[i]) {
        default:
            if (utf8.addByte(s[i])) {
                break;
            } else {
                data = utf8.escapeBytes(data);
            }
            // The \n, \t, \r cases (all < 32) are handled below; we can ignore them here
            if (((unsigned char)s[i]) < 32) {
                goto doEscape;
            }
            // GroupString and KeyString should be valid UTF-8, but ValueString
            // can be a bytearray with non-UTF-8 bytes that should be escaped.
            if (type == ValueString && ((unsigned char)s[i]) >= 127) {
                goto doEscape;
            }
            *data++ = s[i];
            break;
        case '\n':
            *data++ = '\\';
            *data++ = 'n';
            break;
        case '\t':
            *data++ = '\\';
            *data++ = 't';
            break;
        case '\r':
            *data++ = '\\';
            *data++ = 'r';
            break;
        case '\\':
            *data++ = '\\';
            *data++ = '\\';
            break;
        case '=':
            if (type != KeyString) {
                *data++ = s[i];
                break;
            }
            goto doEscape;
        case '[':
        case ']':
            // Above chars are OK to put in *value* strings as plaintext
            if (type == ValueString) {
                *data++ = s[i];
                break;
            }
        doEscape:
            data = escapeByte(data, s[i]);
            break;
        }
        if (utf8.isComplete()) {
            data = utf8.writeUtf8(data);
        }
    }
    data = utf8.write(data);
    *data = 0;
    result.resize(data - start);

    // Protect trailing space
    if (result.endsWith(' ') && type != GroupString) {
        result.replace(result.length() - 1, 1, "\\s");
    }

    return result;
}

char KConfigIniBackend::charFromHex(const char *str, const KConfigIniBackendAbstractDevice *device, int line)
{
    unsigned char ret = 0;
    for (int i = 0; i < 2; i++) {
        ret <<= 4;
        quint8 c = quint8(str[i]);

        if (c >= '0' && c <= '9') {
            ret |= c - '0';
        } else if (c >= 'a' && c <= 'f') {
            ret |= c - 'a' + 0x0a;
        } else if (c >= 'A' && c <= 'F') {
            ret |= c - 'A' + 0x0a;
        } else {
            QByteArray e(str, 2);
            e.prepend("\\x");
            qCWarning(KCONFIG_CORE_LOG) << warningProlog(device, line) << "Invalid hex character " << c << " in \\x<nn>-type escape sequence \""
                                        << e.constData() << "\".";
            return 'x';
        }
    }
    return char(ret);
}

bool KConfigIniBackend::printableToString(QByteArrayView &aString, const KConfigIniBackendAbstractDevice *device, int line)
{
    if (aString.isEmpty() || aString.indexOf('\\') == -1) {
        return true;
    }
    aString = aString.trimmed();
    int l = aString.size();
    char *r = const_cast<char *>(aString.data());
    char *str = r;

    for (int i = 0; i < l; i++, r++) {
        if (str[i] != '\\') {
            *r = str[i];
        } else {
            // Probable escape sequence
            ++i;
            if (i >= l) { // Line ends after backslash - stop.
                *r = '\\';
                break;
            }

            switch (str[i]) {
            case 's':
                *r = ' ';
                break;
            case 't':
                *r = '\t';
                break;
            case 'n':
                *r = '\n';
                break;
            case 'r':
                *r = '\r';
                break;
            case '\\':
                *r = '\\';
                break;
            case ';':
                // not really an escape sequence, but allowed in .desktop files, don't strip '\;' from the string
                *r = '\\';
                ++r;
                *r = ';';
                break;
            case ',':
                // not really an escape sequence, but allowed in .desktop files, don't strip '\,' from the string
                *r = '\\';
                ++r;
                *r = ',';
                break;
            case 'x':
                if (i + 2 < l) {
                    *r = charFromHex(str + i + 1, device, line);
                    i += 2;
                } else {
                    *r = 'x';
                    i = l - 1;
                }
                break;
            default:
                *r = '\\';
                qCWarning(KCONFIG_CORE_LOG).noquote() << warningProlog(device, line) << QStringLiteral("Invalid escape sequence: «\\%1»").arg(str[i]);
                return false;
            }
        }
    }
    aString.truncate(r - aString.constData());
    return true;
}

void KConfigIniBackend::setPrimaryGroup(const QString &group)
{
    mPrimaryGroup = group;
}

bool KConfigIniBackend::hasOpenableDeviceInterface() const
{
    return mDeviceInterface->isDeviceReadable();
}

QString KConfigIniBackend::backingDevicePath() const
{
    if (dynamic_cast<KConfigIniBackendPathDevice *>(mDeviceInterface.get())) {
        return mDeviceInterface->id();
    }
    return {};
}

#include "moc_kconfigini_p.cpp"

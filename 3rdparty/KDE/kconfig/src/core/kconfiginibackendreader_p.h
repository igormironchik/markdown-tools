// SPDX-License-Identifier: LGPL-2.0-or-later
// SPDX-FileCopyrightText: 2006, 2007 Thomas Braxton <kde.braxton@gmail.com>
// SPDX-FileCopyrightText: 1999 Preston Brown <pbrown@kde.org>
// SPDX-FileCopyrightText: 1997 Matthias Kalle Dalheimer <kalle@kde.org>
// SPDX-FileCopyrightText: 2025 Harald Sitter <sitter@kde.org>

#pragma once

#include <QDir>
#include <QFile>
#include <QLockFile>
#include <QSaveFile>

#ifdef Q_OS_ANDROID
#include <QStandardPaths>
#endif

#include <chrono>
#include <qtpreprocessorsupport.h>

#ifndef Q_OS_WIN
#include <unistd.h> // getuid
#endif
#include <sys/types.h> // uid_t

#include "kconfig_core_log_settings.h"

using namespace Qt::StringLiterals;

// Wraps a QLockFile to allow no-op locking when locking is not supported (e.g. QIODevice based backends).
class AbstractLockFile
{
public:
    AbstractLockFile() = default;
    virtual ~AbstractLockFile() = default;
    Q_DISABLE_COPY_MOVE(AbstractLockFile)

    [[nodiscard]] virtual bool tryLock(std::chrono::milliseconds timeout = std::chrono::milliseconds::zero()) = 0;
    [[nodiscard]] virtual bool isLocked() const = 0;
    [[nodiscard]] virtual QString fileName() const = 0;
    virtual void unlock() = 0;
    [[nodiscard]] virtual QLockFile::LockError error() const = 0;
};

class RealLockFile : public AbstractLockFile
{
public:
    RealLockFile(std::unique_ptr<QLockFile> &&lockFile)
        : m_lockFile(std::move(lockFile))
    {
        m_lockFile->setStaleLockTime(std::chrono::seconds{20});
    }

    [[nodiscard]] bool tryLock(std::chrono::milliseconds timeout) override
    {
        return m_lockFile->tryLock(timeout);
    }

    void unlock() override
    {
        m_lockFile->unlock();
    }

    [[nodiscard]] bool isLocked() const override
    {
        return m_lockFile->isLocked();
    }
    QString fileName() const override
    {
        return m_lockFile->fileName();
    }

    [[nodiscard]] QLockFile::LockError error() const override
    {
        return m_lockFile->error();
    }

private:
    std::unique_ptr<QLockFile> m_lockFile;
};

class FakeLockFile : public AbstractLockFile
{
public:
    [[nodiscard]] bool tryLock(std::chrono::milliseconds timeout) override
    {
        Q_UNUSED(timeout)
        return m_locked = true;
    }

    void unlock() override
    {
        m_locked = false;
    }

    [[nodiscard]] bool isLocked() const override
    {
        return m_locked;
    }

    [[nodiscard]] QString fileName() const override
    {
        return {};
    }

    [[nodiscard]] QLockFile::LockError error() const override
    {
        return QLockFile::LockError::NoError;
    }

private:
    bool m_locked = false;
};

class KConfigIniBackendAbstractDevice
{
public:
    struct OpenResult {
        bool shouldHaveDevice;
        std::shared_ptr<QIODevice> device;
    };

    KConfigIniBackendAbstractDevice() = default;
    virtual ~KConfigIniBackendAbstractDevice() = default;
    Q_DISABLE_COPY_MOVE(KConfigIniBackendAbstractDevice)

    // The local file path for path based devices or a pseudo name for all others.
    [[nodiscard]] virtual QString id() const = 0;
    [[nodiscard]] virtual bool isDeviceReadable() const = 0;
    [[nodiscard]] virtual bool canWriteToDevice() const = 0;
    [[nodiscard]] virtual bool writeToDevice(const std::function<void(QIODevice &)> &write) = 0;
    [[nodiscard]] virtual OpenResult open() = 0;
    [[nodiscard]] virtual std::unique_ptr<AbstractLockFile> lockFile() = 0;
    virtual void createEnclosingEntity() = 0;
    virtual void setFilePath(const QString &path) = 0;
};

class KConfigIniBackendNullDevice : public KConfigIniBackendAbstractDevice
{
public:
    [[nodiscard]] QString id() const override
    {
        return u"((NullDevice))"_s;
    }

    [[nodiscard]] bool isDeviceReadable() const override
    {
        return false;
    }

    [[nodiscard]] bool canWriteToDevice() const override
    {
        return false;
    }

    [[nodiscard]] bool writeToDevice(const std::function<void(QIODevice &)> &write) override
    {
        Q_UNUSED(write)
        return false;
    }

    [[nodiscard]] OpenResult open() override
    {
        return {.shouldHaveDevice = false, .device = nullptr};
    }

    [[nodiscard]] std::unique_ptr<AbstractLockFile> lockFile() override
    {
        return std::make_unique<FakeLockFile>();
    }

    void createEnclosingEntity() override
    {
    }

    void setFilePath(const QString &path) override
    {
        Q_UNUSED(path)
    }
};

class KConfigIniBackendPathDevice : public KConfigIniBackendAbstractDevice
{
public:
    explicit KConfigIniBackendPathDevice(const QString &path)
    {
        setFilePath(path);
    }

    [[nodiscard]] QString id() const override
    {
        return m_localFilePath;
    }

    [[nodiscard]] bool isDeviceReadable() const override
    {
        return !m_localFilePath.isEmpty();
    }

    [[nodiscard]] bool canWriteToDevice() const override
    {
        const QString filePath = m_localFilePath;
        if (filePath.isEmpty()) {
            return false;
        }

        QFileInfo file(filePath);
        if (file.exists()) {
            return file.isWritable();
        }

        // If the file does not exist, check if the deepest existing dir is writable
        QFileInfo dir(file.absolutePath());
        while (!dir.exists()) {
            QString parent = dir.absolutePath(); // Go up. Can't use cdUp() on non-existing dirs.
            if (parent == dir.filePath()) {
                // no parent
                return false;
            }
            dir.setFile(parent);
        }
        return dir.isDir() && dir.isWritable();
    }

    [[nodiscard]] bool writeToDevice(const std::function<void(QIODevice &)> &write) override
    {
        // check if file exists
        QFile::Permissions fileMode = filePath().startsWith(u"/etc/xdg/"_s) ? QFile::ReadUser | QFile::WriteUser | QFile::ReadGroup | QFile::ReadOther //
                                                                            : QFile::ReadUser | QFile::WriteUser;

        bool createNew = true;

        QFileInfo fi(filePath());
        if (fi.exists()) {
#ifdef Q_OS_WIN
            // TODO: getuid does not exist on windows, use GetSecurityInfo and GetTokenInformation instead
            createNew = false;
#else
            if (fi.ownerId() == ::getuid()) {
                // Preserve file mode if file exists and is owned by user.
                fileMode = fi.permissions();
            } else {
                // File is not owned by user:
                // Don't create new file but write to existing file instead.
                createNew = false;
            }
#endif
        }

        if (createNew) {
            QSaveFile file(filePath());
            if (!file.open(QIODevice::WriteOnly)) {
#ifdef Q_OS_ANDROID
                // HACK: when we are dealing with content:// URIs, QSaveFile has to rely on DirectWrite.
                // Otherwise this method returns a false and we're done.
                file.setDirectWriteFallback(true);
                if (!file.open(QIODevice::WriteOnly)) {
                    qWarning(KCONFIG_CORE_LOG) << "Couldn't create a new file:" << filePath() << ". Error:" << file.errorString();
                    return false;
                }
#else
                qWarning(KCONFIG_CORE_LOG) << "Couldn't create a new file:" << filePath() << ". Error:" << file.errorString();
                return false;
#endif
            }

            file.setTextModeEnabled(true); // to get eol translation
            write(file);

            if (!file.size() && (fileMode == (QFile::ReadUser | QFile::WriteUser))) {
                // File is empty and doesn't have special permissions: delete it.
                file.cancelWriting();

                if (fi.exists()) {
                    // also remove the old file in case it existed. this can happen
                    // when we delete all the entries in an existing config file.
                    // if we don't do this, then deletions and revertToDefault's
                    // will mysteriously fail
                    QFile::remove(filePath());
                }
            } else {
                // Normal case: Close the file
                if (file.commit()) {
                    QFile::setPermissions(filePath(), fileMode);
                    return true;
                }
                // Couldn't write. Disk full?
                qCWarning(KCONFIG_CORE_LOG) << "Couldn't write" << filePath() << ". Disk full?";
                return false;
            }
        } else {
            QFile f(filePath());

            // Open existing file. *DON'T* create it if it suddenly does not exist!
            if (!f.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::ExistingOnly)) {
                return false;
            }

            f.setTextModeEnabled(true);
            write(f);
        }

        return true;
    }

    [[nodiscard]] OpenResult open() override
    {
        auto file = std::make_unique<QFile>(m_localFilePath);
        if (!file->open(QIODevice::ReadOnly | QIODevice::Text)) {
            return {.shouldHaveDevice = file->exists(), .device = nullptr};
        }
        return {.shouldHaveDevice = true, .device = std::move(file)};
    }

    void createEnclosingEntity() override
    {
        const QString file = m_localFilePath;
        if (file.isEmpty()) {
            return; // nothing to do
        }

        // Create the containing dir, maybe it wasn't there
        if (!QDir().mkpath(QFileInfo(file).absolutePath())) {
            qWarning("KConfigIniBackend: Could not create enclosing directory for %s", qPrintable(file));
        }
    }

    void setFilePath(const QString &path) override
    {
        if (path.isEmpty()) {
            return;
        }

        Q_ASSERT(QDir::isAbsolutePath(path));
        if (!QDir::isAbsolutePath(path)) {
            qWarning() << "setFilePath" << path;
        }

        const QFileInfo info(path);
        if (info.exists()) {
            setLocalFilePath(info.canonicalFilePath());
            return;
        }

        if (QString filePath = info.dir().canonicalPath(); !filePath.isEmpty()) {
            filePath += QLatin1Char('/') + info.fileName();
            setLocalFilePath(filePath);
        } else {
            setLocalFilePath(path);
        }
    }

    [[nodiscard]] std::unique_ptr<AbstractLockFile> lockFile() override
    {
#ifdef Q_OS_ANDROID
        // handle content Uris properly
        if (filePath().startsWith(QLatin1String("content://"))) {
            // we can't create file at an arbitrary location, so use internal storage to create one

            // NOTE: filename can be the same, but because this lock is short lived we may never have a collision
            return std::make_unique<RealLockFile>(std::make_unique<QLockFile>(QStandardPaths::writableLocation(QStandardPaths::GenericCacheLocation)
                                                                              + QLatin1String("/") + QFileInfo(filePath()).fileName()
                                                                              + QLatin1String(".lock")));
        }
        return std::make_unique<RealLockFile>(std::make_unique<QLockFile>(filePath() + QLatin1String(".lock")));
#else
        return std::make_unique<RealLockFile>(std::make_unique<QLockFile>(filePath() + QLatin1String(".lock")));
#endif
    }

private:
    /* the absolute path to the object */
    [[nodiscard]] QString filePath() const
    {
        return m_localFilePath;
    }

    void setLocalFilePath(const QString &file)
    {
        m_localFilePath = file;
    }

    QString m_localFilePath;
};

class KConfigIniBackendQIODevice : public KConfigIniBackendAbstractDevice
{
public:
    KConfigIniBackendQIODevice(const std::shared_ptr<QIODevice> &device)
        : m_device(device)
    {
    }

    [[nodiscard]] QString id() const override
    {
        return u"((QIODevice))"_s;
    }

    [[nodiscard]] bool isDeviceReadable() const override
    {
        return m_device->isOpen() && m_device->isReadable();
    }

    [[nodiscard]] bool canWriteToDevice() const override
    {
        return m_device->isOpen() && m_device->isWritable();
    }

    [[nodiscard]] bool writeToDevice(const std::function<void(QIODevice &)> &write) override
    {
        m_device->setTextModeEnabled(true);
        write(*m_device);
        return true;
    }

    [[nodiscard]] OpenResult open() override
    {
        return {.shouldHaveDevice = true, .device = m_device};
    }

    [[nodiscard]] std::unique_ptr<AbstractLockFile> lockFile() override
    {
        return std::make_unique<FakeLockFile>();
    }

    void createEnclosingEntity() override
    {
        // nothing to do
    }

    void setFilePath([[maybe_unused]] const QString &path) override
    {
        // nothing to do
    }

private:
    std::shared_ptr<QIODevice> m_device;
};

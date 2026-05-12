/*
    SPDX-FileCopyrightText: 2009 Pino Toscano <pino@kde.org>
    SPDX-FileCopyrightText: 2009-2012 Lukáš Tinkl <ltinkl@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "udisksstorageaccess.h"
#include "udisks2.h"
#include "udisks_debug.h"
#include "udisksutils.h"

#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusMetaType>
#include <QDir>
#include <QGuiApplication>
#include <QMetaObject>
#include <QWindow>

#include <config-solid.h>
#if HAVE_LIBMOUNT
#include <libmount.h>
#endif

struct AvailableAnswer {
    bool checkResult;
    QString binaryName;
};
Q_DECLARE_METATYPE(AvailableAnswer)

QDBusArgument &operator<<(QDBusArgument &argument, const AvailableAnswer &answer)
{
    argument.beginStructure();
    argument << answer.checkResult << answer.binaryName;
    argument.endStructure();
    return argument;
}

const QDBusArgument &operator>>(const QDBusArgument &argument, AvailableAnswer &answer)
{
    argument.beginStructure();
    argument >> answer.checkResult >> answer.binaryName;
    argument.endStructure();
    return argument;
}

using namespace Solid::Backends::UDisks2;

StorageAccess::StorageAccess(Device *device)
    : DeviceInterface(device)
    , m_setupInProgress(false)
    , m_teardownInProgress(false)
    , m_checkInProgress(false)
    , m_repairInProgress(false)
    , m_passphraseRequested(false)
{
    qDBusRegisterMetaType<AvailableAnswer>();

    connect(device, SIGNAL(changed()), this, SLOT(checkAccessibility()));
    updateCache();

    // Delay connecting to DBus signals to avoid the related time penalty
    // in hot paths such as predicate matching
    QMetaObject::invokeMethod(this, &StorageAccess::connectDBusSignals, Qt::QueuedConnection);
}

StorageAccess::~StorageAccess()
{
}

void StorageAccess::connectDBusSignals()
{
    m_device->registerAction(QStringLiteral("setup"), this, SLOT(slotSetupRequested()), SLOT(slotSetupDone(int, QString)));

    m_device->registerAction(QStringLiteral("teardown"), this, SLOT(slotTeardownRequested()), SLOT(slotTeardownDone(int, QString)));

    m_device->registerAction(QStringLiteral("check"), this, SLOT(slotCheckRequested()), SLOT(slotCheckDone(int, QString)));

    m_device->registerAction(QStringLiteral("repair"), this, SLOT(slotRepairRequested()), SLOT(slotRepairDone(int, QString)));
}

bool StorageAccess::isLuksDevice() const
{
    return m_device->isEncryptedContainer(); // encrypted device
}

bool StorageAccess::isAccessible() const
{
    if (isLuksDevice()) { // check if the cleartext slave is mounted
        const QString path = clearTextPath();
        // qDebug() << Q_FUNC_INFO << "CLEARTEXT device path: " << path;
        if (path.isEmpty() || path == QLatin1String("/")) {
            return false;
        }
        Device holderDevice(m_device->manager(), path);
        return holderDevice.isMounted();
    }

    return m_device->isMounted();
}

bool StorageAccess::isEncrypted() const
{
    // FIXME We should also check if physical device is encrypted
    // FIXME Gocryptfs is not supported
    return isLuksDevice() || m_device->isEncryptedCleartext();
}

bool StorageAccess::canCheck() const
{
    const auto idType = m_device->prop(QStringLiteral("IdType")).toString();
    auto c = QDBusConnection::systemBus();
    auto msg = QDBusMessage::createMethodCall(QStringLiteral(UD2_DBUS_SERVICE),
                                              QStringLiteral(UD2_DBUS_PATH_MANAGER),
                                              QStringLiteral("org.freedesktop.UDisks2.Manager"),
                                              QStringLiteral("CanCheck"));
    msg << idType;
    QDBusReply<AvailableAnswer> r = c.call(msg);
    if (!r.isValid()) {
        qCDebug(UDISKS2) << Q_FUNC_INFO << dbusPath() << idType << "DBus error, code" << r.error().type();
        return false;
    }

    const bool ret = r.value().checkResult;
    qCDebug(UDISKS2) << Q_FUNC_INFO << dbusPath() << idType << ret << r.value().binaryName;
    return ret;
}

bool StorageAccess::check()
{
    if (m_setupInProgress || m_teardownInProgress || m_checkInProgress || m_repairInProgress) {
        return false;
    }
    m_checkInProgress = true;
    m_device->broadcastActionRequested(QStringLiteral("check"));

    const auto path = dbusPath();
    auto c = QDBusConnection::systemBus();
    auto msg = QDBusMessage::createMethodCall(QStringLiteral(UD2_DBUS_SERVICE), path, QStringLiteral(UD2_DBUS_INTERFACE_FILESYSTEM), QStringLiteral("Check"));
    msg << QVariantMap{};

    return c.callWithCallback(msg, this, SLOT(slotDBusReply(QDBusMessage)), SLOT(slotDBusError(QDBusError)));
}

bool StorageAccess::canRepair() const
{
    const auto idType = m_device->prop(QStringLiteral("IdType")).toString();
    auto c = QDBusConnection::systemBus();
    auto msg = QDBusMessage::createMethodCall(QStringLiteral(UD2_DBUS_SERVICE),
                                              QStringLiteral(UD2_DBUS_PATH_MANAGER),
                                              QStringLiteral("org.freedesktop.UDisks2.Manager"),
                                              QStringLiteral("CanRepair"));
    msg << idType;
    QDBusReply<AvailableAnswer> r = c.call(msg);
    if (!r.isValid()) {
        qCDebug(UDISKS2) << Q_FUNC_INFO << dbusPath() << idType << "DBus error, code" << r.error().type();
        return false;
    }

    const bool ret = r.value().checkResult;
    qCDebug(UDISKS2) << Q_FUNC_INFO << dbusPath() << idType << ret << r.value().binaryName;
    return ret;
}

bool StorageAccess::repair()
{
    if (m_teardownInProgress || m_setupInProgress || m_checkInProgress || m_repairInProgress) {
        return false;
    }
    m_repairInProgress = true;
    m_device->broadcastActionRequested(QStringLiteral("repair"));

    const auto path = dbusPath();
    auto c = QDBusConnection::systemBus();
    auto msg = QDBusMessage::createMethodCall(QStringLiteral(UD2_DBUS_SERVICE), path, QStringLiteral(UD2_DBUS_INTERFACE_FILESYSTEM), QStringLiteral("Repair"));
    QVariantMap options;
    msg << options;

    qCDebug(UDISKS2) << Q_FUNC_INFO << path;
    return c.callWithCallback(msg, this, SLOT(slotDBusReply(QDBusMessage)), SLOT(slotDBusError(QDBusError)));
}

static QString baseMountPoint(const QByteArray &dev)
{
    QString mountPoint;

#if HAVE_LIBMOUNT
    // UDisks "MountPoints" property contains multiple paths, this happens with
    // devices with bind mounts; try finding the "base" mount point
    if (struct libmnt_table *table = mnt_new_table()) {
        // This parses "/etc/mtab" if present or "/proc/self/mountinfo" by default
        if (mnt_table_parse_mtab(table, "/proc/self/mountinfo") == 0) {
            // BACKWARD because the fs's we're interested in, /dev/sdXY, are typically at the end
            struct libmnt_iter *itr = mnt_new_iter(MNT_ITER_BACKWARD);
            struct libmnt_fs *fs;

            const QByteArray devicePath = Utils::sanitizeValue(dev);

            while (mnt_table_next_fs(table, itr, &fs) == 0) {
                if (mnt_fs_get_srcpath(fs) == devicePath //
                    && (qstrcmp(mnt_fs_get_root(fs), "/") == 0) // Base mount point will have "/" as root fs
                ) {
                    mountPoint = QFile::decodeName(mnt_fs_get_target(fs));
                    break;
                }
            }

            mnt_free_iter(itr);
        }

        mnt_free_table(table);
    }
#else
    Q_UNUSED(dev);
#endif

    return mountPoint;
}

QString StorageAccess::filePath() const
{
    if (isLuksDevice()) { // encrypted (and unlocked) device
        const QString path = clearTextPath();
        if (path.isEmpty() || path == QLatin1String("/")) {
            return QString();
        }
        Device holderDevice(m_device->manager(), path);
        const auto mntPoints = qdbus_cast<QByteArrayList>(holderDevice.prop(QStringLiteral("MountPoints")));
        if (!mntPoints.isEmpty()) {
            return QFile::decodeName(Utils::sanitizeValue(mntPoints.first())); // FIXME Solid doesn't support multiple mount points
        } else {
            return QString();
        }
    }

    const auto mntPoints = qdbus_cast<QByteArrayList>(m_device->prop(QStringLiteral("MountPoints")));
    if (mntPoints.isEmpty()) {
        return {};
    }

    const QString potentialMountPoint = QFile::decodeName(Utils::sanitizeValue(mntPoints.first()));

    if (mntPoints.size() == 1) {
        return potentialMountPoint;
    }

    // Device has bind mounts?
    const QString basePoint = baseMountPoint(m_device->prop(QStringLiteral("Device")).toByteArray());

    return !basePoint.isEmpty() ? basePoint : potentialMountPoint;
}

bool StorageAccess::isIgnored() const
{
    if (m_device->prop(QStringLiteral("HintIgnore")).toBool()) {
        return true;
    }

    const QStringList mountOptions = m_device->prop(QStringLiteral("UserspaceMountOptions")).toStringList();
    if (mountOptions.contains(QLatin1String("x-gdu.hide"))) {
        return true;
    }

    const QString path = filePath();

    const bool inUserPath = (path.startsWith(QLatin1String("/media/")) //
                             || path.startsWith(QLatin1String("/run/media/")) //
                             || path.startsWith(QDir::homePath()));
    return !inUserPath;
}

bool StorageAccess::setup()
{
    if (m_teardownInProgress || m_setupInProgress || m_checkInProgress || m_repairInProgress) {
        return false;
    }
    m_setupInProgress = true;
    m_device->broadcastActionRequested(QStringLiteral("setup"));

    if (m_device->isEncryptedContainer() && clearTextPath().isEmpty()) {
        return requestPassphrase();
    } else {
        return mount();
    }
}

bool StorageAccess::teardown()
{
    if (m_teardownInProgress || m_setupInProgress || m_checkInProgress || m_repairInProgress) {
        return false;
    }
    m_teardownInProgress = true;
    m_device->broadcastActionRequested(QStringLiteral("teardown"));

    return unmount();
}

void StorageAccess::updateCache()
{
    m_isAccessible = isAccessible();
}

void StorageAccess::checkAccessibility()
{
    const bool old_isAccessible = m_isAccessible;
    updateCache();

    if (old_isAccessible != m_isAccessible) {
        Q_EMIT accessibilityChanged(m_isAccessible, m_device->udi());
    }
}

void StorageAccess::slotDBusReply(const QDBusMessage &reply)
{
    if (m_setupInProgress) {
        if (isLuksDevice() && !isAccessible()) { // unlocked device, now mount it
            mount();
        } else { // Don't broadcast setupDone unless the setup is really done. (Fix kde#271156)
            m_setupInProgress = false;
            m_device->broadcastActionDone(QStringLiteral("setup"));

            checkAccessibility();
        }
    } else if (m_teardownInProgress) { // FIXME
        const QString ctPath = clearTextPath();
        qCDebug(UDISKS2) << "Successfully unmounted " << m_device->udi();
        if (isLuksDevice() && !ctPath.isEmpty() && ctPath != QStringLiteral("/")) { // unlocked device, lock it
            callCryptoTeardown();
        } else if (!ctPath.isEmpty() && ctPath != QStringLiteral("/")) {
            callCryptoTeardown(true); // Lock encrypted parent
        } else {
            // try to "eject" (aka safely remove) from the (parent) drive, e.g. SD card from a reader
            QString drivePath = m_device->drivePath();
            if (!drivePath.isEmpty() || drivePath != QStringLiteral("/")) {
                Device drive(m_device->manager(), drivePath);
                QDBusConnection c = QDBusConnection::systemBus();

                if (drive.prop(QStringLiteral("MediaRemovable")).toBool() //
                    && drive.prop(QStringLiteral("MediaAvailable")).toBool() //
                    && !m_device->isOpticalDisc()) { // optical drives have their Eject method
                    QDBusMessage msg = QDBusMessage::createMethodCall(QStringLiteral(UD2_DBUS_SERVICE),
                                                                      drivePath,
                                                                      QStringLiteral(UD2_DBUS_INTERFACE_DRIVE),
                                                                      QStringLiteral("Eject"));
                    msg << QVariantMap(); // options, unused now
                    c.call(msg, QDBus::NoBlock);
                } else if (drive.prop(QStringLiteral("CanPowerOff")).toBool() //
                           && !m_device->isOpticalDisc()) { // avoid disconnecting optical drives from the bus
                    qCDebug(UDISKS2) << "Drive can power off:" << drivePath;
                    QDBusMessage msg = QDBusMessage::createMethodCall(QStringLiteral(UD2_DBUS_SERVICE),
                                                                      drivePath,
                                                                      QStringLiteral(UD2_DBUS_INTERFACE_DRIVE),
                                                                      QStringLiteral("PowerOff"));
                    msg << QVariantMap(); // options, unused now
                    c.call(msg, QDBus::NoBlock);
                }
            }

            m_teardownInProgress = false;
            m_device->broadcastActionDone(QStringLiteral("teardown"));

            checkAccessibility();
        }
    } else if (m_checkInProgress) {
        QDBusReply<bool> r = reply;
        qCDebug(UDISKS2) << "Check reply received " << m_device->udi() << r;
        m_checkInProgress = false;
        if (r.isValid()) {
            m_device->broadcastActionDone(QStringLiteral("check"), Solid::NoError, QString::number(r.value()));
        } else {
            m_device->broadcastActionDone(QStringLiteral("check"), Solid::OperationFailed, reply.errorMessage());
        }
    } else if (m_repairInProgress) {
        qCDebug(UDISKS2) << "Successfully repaired " << m_device->udi();
        m_repairInProgress = false;
        m_device->broadcastActionDone(QStringLiteral("repair"));
    }
}

void StorageAccess::slotDBusError(const QDBusError &error)
{
    // qDebug() << Q_FUNC_INFO << "DBUS ERROR:" << error.name() << error.message();

    if (m_setupInProgress) {
        m_setupInProgress = false;
        m_device->broadcastActionDone(QStringLiteral("setup"), //
                                      m_device->errorToSolidError(error.name()),
                                      m_device->errorToString(error.name()) + QStringLiteral(": ") + error.message());

        checkAccessibility();
    } else if (m_teardownInProgress) {
        m_teardownInProgress = false;
        m_device->broadcastActionDone(QStringLiteral("teardown"), //
                                      m_device->errorToSolidError(error.name()),
                                      m_device->errorToString(error.name()) + QStringLiteral(": ") + error.message());
        checkAccessibility();
    } else if (m_checkInProgress) {
        m_checkInProgress = false;
        m_device->broadcastActionDone(QStringLiteral("check"),
                                      m_device->errorToSolidError(error.name()),
                                      m_device->errorToString(error.name()) + QStringLiteral(": ") + error.message());
    } else if (m_repairInProgress) {
        m_repairInProgress = false;
        m_device->broadcastActionDone(QStringLiteral("repair"),
                                      m_device->errorToSolidError(error.name()),
                                      m_device->errorToString(error.name()) + QStringLiteral(": ") + error.message());
    }
}

void StorageAccess::slotSetupRequested()
{
    m_setupInProgress = true;
    // qDebug() << "SETUP REQUESTED:" << m_device->udi();
    Q_EMIT setupRequested(m_device->udi());
}

void StorageAccess::slotSetupDone(int error, const QString &errorString)
{
    m_setupInProgress = false;
    // qDebug() << "SETUP DONE:" << m_device->udi();
    checkAccessibility();
    Q_EMIT setupDone(static_cast<Solid::ErrorType>(error), errorString, m_device->udi());
}

void StorageAccess::slotTeardownRequested()
{
    m_teardownInProgress = true;
    Q_EMIT teardownRequested(m_device->udi());
}

void StorageAccess::slotTeardownDone(int error, const QString &errorString)
{
    m_teardownInProgress = false;
    checkAccessibility();
    Q_EMIT teardownDone(static_cast<Solid::ErrorType>(error), errorString, m_device->udi());
}

void StorageAccess::slotCheckRequested()
{
    m_checkInProgress = true;
    Q_EMIT checkRequested(m_device->udi());
}

void StorageAccess::slotCheckDone(int error, const QString &errorString)
{
    m_checkInProgress = false;
    Q_EMIT checkDone(static_cast<Solid::ErrorType>(error), errorString, m_device->udi());
}

void StorageAccess::slotRepairRequested()
{
    m_repairInProgress = true;
    Q_EMIT repairRequested(m_device->udi());
}

void StorageAccess::slotRepairDone(int error, const QString &errorString)
{
    m_repairInProgress = false;
    Q_EMIT repairDone(static_cast<Solid::ErrorType>(error), errorString, m_device->udi());
}

bool StorageAccess::mount()
{
    const auto path = dbusPath();

    QDBusConnection c = QDBusConnection::systemBus();
    QDBusMessage msg =
        QDBusMessage::createMethodCall(QStringLiteral(UD2_DBUS_SERVICE), path, QStringLiteral(UD2_DBUS_INTERFACE_FILESYSTEM), QStringLiteral("Mount"));
    QVariantMap options;

    if (m_device->prop(QStringLiteral("IdType")).toString() == QLatin1String("vfat")) {
        options.insert(QStringLiteral("options"), QStringLiteral("flush"));
    }

    msg << options;

    return c.callWithCallback(msg, this, SLOT(slotDBusReply(QDBusMessage)), SLOT(slotDBusError(QDBusError)));
}

bool StorageAccess::unmount()
{
    const auto path = dbusPath();

    QDBusConnection c = QDBusConnection::systemBus();
    QDBusMessage msg =
        QDBusMessage::createMethodCall(QStringLiteral(UD2_DBUS_SERVICE), path, QStringLiteral(UD2_DBUS_INTERFACE_FILESYSTEM), QStringLiteral("Unmount"));

    msg << QVariantMap(); // options, unused now

    qCDebug(UDISKS2) << "Initiating unmount of " << path;
    return c.callWithCallback(msg, this, SLOT(slotDBusReply(QDBusMessage)), SLOT(slotDBusError(QDBusError)), s_unmountTimeout);
}

QString StorageAccess::generateReturnObjectPath()
{
    static QAtomicInt number = 1;

    return QStringLiteral("/org/kde/solid/UDisks2StorageAccess_") + QString::number(number++);
}

QString StorageAccess::clearTextPath() const
{
    const QString path = m_device->prop(QStringLiteral("CleartextDevice")).value<QDBusObjectPath>().path();
    if (path != QLatin1String("/")) {
        return path;
    }
    return QString();
}

QString StorageAccess::dbusPath() const
{
    QString path = m_device->udi();
    if (isLuksDevice()) { // mount options for the cleartext volume
        const QString ctPath = clearTextPath();
        if (!ctPath.isEmpty()) {
            path = ctPath;
        }
    }
    return path;
}

bool StorageAccess::requestPassphrase()
{
    QString udi = m_device->udi();
    QString returnService = QDBusConnection::sessionBus().baseService();
    m_lastReturnObject = generateReturnObjectPath();

    QDBusConnection::sessionBus().registerObject(m_lastReturnObject, this, QDBusConnection::ExportScriptableSlots);

    // TODO: this only works on X11, Wayland doesn't have global window ids.
    // Passing ids to other processes doesn't make any sense
    auto activeWindow = QGuiApplication::focusWindow();
    uint wId = 0;
    if (activeWindow != nullptr) {
        wId = (uint)activeWindow->winId();
    }

    QString appId = QCoreApplication::applicationName();

    const auto plasmaVersionMajor = qEnvironmentVariable("KDE_SESSION_VERSION", QStringLiteral("6"));

    // TODO KF6: remove hard dep on Plasma here which provides the SolidUiServer kded plugin
    QDBusInterface soliduiserver(QStringLiteral("org.kde.kded") + plasmaVersionMajor,
                                 QStringLiteral("/modules/soliduiserver"),
                                 QStringLiteral("org.kde.SolidUiServer"));
    QDBusReply<void> reply = soliduiserver.call(QStringLiteral("showPassphraseDialog"), udi, returnService, m_lastReturnObject, wId, appId);
    m_passphraseRequested = reply.isValid();
    if (!m_passphraseRequested) {
        qCWarning(UDISKS2) << "Failed to call the SolidUiServer, D-Bus said:" << reply.error();
    }

    return m_passphraseRequested;
}

void StorageAccess::passphraseReply(const QString &passphrase)
{
    if (m_passphraseRequested) {
        QDBusConnection::sessionBus().unregisterObject(m_lastReturnObject);
        m_passphraseRequested = false;
        if (!passphrase.isEmpty()) {
            callCryptoSetup(passphrase);
        } else {
            m_setupInProgress = false;
            m_device->broadcastActionDone(QStringLiteral("setup"), Solid::UserCanceled);
        }
    }
}

void StorageAccess::callCryptoSetup(const QString &passphrase)
{
    QDBusConnection c = QDBusConnection::systemBus();
    QDBusMessage msg = QDBusMessage::createMethodCall(QStringLiteral(UD2_DBUS_SERVICE),
                                                      m_device->udi(),
                                                      QStringLiteral(UD2_DBUS_INTERFACE_ENCRYPTED),
                                                      QStringLiteral("Unlock"));

    msg << passphrase;
    msg << QVariantMap(); // options, unused now

    c.callWithCallback(msg, this, SLOT(slotDBusReply(QDBusMessage)), SLOT(slotDBusError(QDBusError)));
}

bool StorageAccess::callCryptoTeardown(bool actOnParent)
{
    QDBusConnection c = QDBusConnection::systemBus();
    QDBusMessage msg =
        QDBusMessage::createMethodCall(QStringLiteral(UD2_DBUS_SERVICE),
                                       actOnParent ? (m_device->prop(QStringLiteral("CryptoBackingDevice")).value<QDBusObjectPath>().path()) : m_device->udi(),
                                       QStringLiteral(UD2_DBUS_INTERFACE_ENCRYPTED),
                                       QStringLiteral("Lock"));
    msg << QVariantMap(); // options, unused now

    return c.callWithCallback(msg, this, SLOT(slotDBusReply(QDBusMessage)), SLOT(slotDBusError(QDBusError)));
}

#include "moc_udisksstorageaccess.cpp"

/*
    SPDX-FileCopyrightText: 2020 David Edmundson <davidedmundson@kde.org>
    SPDX-FileCopyrightText: 2021 Méven Car <meven.car@enioka.com>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "waylandclipboard_p.h"

#include <QBuffer>
#include <QFile>
#include <QGuiApplication>
#include <QImageReader>
#include <QImageWriter>
#include <QMimeData>
#include <QMutex>
#include <QPointer>
#include <QThread>
#include <QTimer>
#include <QWaylandClientExtension>
#include <QWindow>
#include <QtWaylandClientVersion>

#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <poll.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>

#include "waylandpipewriterhelper_p.h"

#include "qwayland-wayland.h"
#include "qwayland-ext-data-control-v1.h"

using namespace std::chrono;

/**
 * Wayland clipboard wraps ext_data_control an additional is used as we need to avoid
 * deadlocks if an application tries to read the normal clipboard whilst it owns the data control.
 *
 * To solve this all ext_data_control classes live on another thread which dispatches events on a separate
 * queue. A recursive mutex allows the main thread to read mimedata and no wayland events which change the mimedata process until this is complete.
 */

static inline QString applicationQtXImageLiteral()
{
    return QStringLiteral("application/x-qt-image");
}

// copied from https://code.woboq.org/qt5/qtbase/src/gui/kernel/qinternalmimedata.cpp.html
static QString utf8Text()
{
    return QStringLiteral("text/plain;charset=utf-8");
}

static QStringList imageMimeFormats(const QList<QByteArray> &imageFormats)
{
    QStringList formats;
    formats.reserve(imageFormats.size());
    for (const auto &format : imageFormats)
        formats.append(QLatin1String("image/") + QLatin1String(format.toLower()));
    // put png at the front because it is best
    int pngIndex = formats.indexOf(QLatin1String("image/png"));
    if (pngIndex != -1 && pngIndex != 0)
        formats.move(pngIndex, 0);
    return formats;
}

static inline QStringList imageReadMimeFormats()
{
    return imageMimeFormats(QImageReader::supportedImageFormats());
}

static inline QStringList imageWriteMimeFormats()
{
    return imageMimeFormats(QImageWriter::supportedImageFormats());
}
// end copied

// The mutex will be held before dispatching any wayland events on our queue which could modify or use the clipboard
// it should also be held if the main thread accesses the clipboard
static QRecursiveMutex s_clipboardLock;

class DataControlDeviceManager : public QWaylandClientExtensionTemplate<DataControlDeviceManager>, public QtWayland::ext_data_control_manager_v1
{
    Q_OBJECT
public:
    DataControlDeviceManager()
        : QWaylandClientExtensionTemplate<DataControlDeviceManager>(1)
    {
    }

    void instantiate()
    {
        initialize();
    }

    ~DataControlDeviceManager()
    {
        if (isInitialized()) {
            destroy();
        }
    }
};

class DataControlOffer : public QMimeData, public QtWayland::ext_data_control_offer_v1
{
    Q_OBJECT
public:
    DataControlOffer(struct ::ext_data_control_offer_v1 *id)
        : QtWayland::ext_data_control_offer_v1(id)
    {
    }

    ~DataControlOffer()
    {
        destroy();
    }

    QStringList formats() const override
    {
        return m_receivedFormats;
    }

    bool containsImageData() const
    {
        if (m_receivedFormats.contains(applicationQtXImageLiteral())) {
            return true;
        }
        const auto formats = imageReadMimeFormats();
        for (const auto &receivedFormat : m_receivedFormats) {
            if (formats.contains(receivedFormat)) {
                return true;
            }
        }
        return false;
    }

    bool hasFormat(const QString &mimeType) const override
    {
        if (mimeType == QStringLiteral("text/plain") && m_receivedFormats.contains(utf8Text())) {
            return true;
        }
        if (m_receivedFormats.contains(mimeType)) {
            return true;
        }

        // If we have image data
        if (containsImageData()) {
            // is the requested output mimeType supported ?
            const QStringList imageFormats = imageWriteMimeFormats();
            for (const QString &imageFormat : imageFormats) {
                if (imageFormat == mimeType) {
                    return true;
                }
            }
            if (mimeType == applicationQtXImageLiteral()) {
                return true;
            }
        }

        return false;
    }

protected:
    void ext_data_control_offer_v1_offer(const QString &mime_type) override
    {
        if (!m_receivedFormats.contains(mime_type)) {
            m_receivedFormats << mime_type;
        }
    }

    QVariant retrieveData(const QString &mimeType, QMetaType type) const override;

private:
    /* reads data from a file descriptor with a timeout of 1 second
     *  true if data is read successfully
     */
    static bool readData(int fd, QByteArray &data, const QString &mimeType);
    QStringList m_receivedFormats;
    mutable QHash<QString, QVariant> m_data;
};

QVariant DataControlOffer::retrieveData(const QString &mimeType, QMetaType type) const
{
    Q_UNUSED(type);

    auto it = m_data.constFind(mimeType);
    if (it != m_data.constEnd())
        return *it;

    QString mime;
    if (!m_receivedFormats.contains(mimeType)) {
        if (mimeType == QStringLiteral("text/plain") && m_receivedFormats.contains(utf8Text())) {
            mime = utf8Text();
        } else if (mimeType == applicationQtXImageLiteral()) {
            const auto writeFormats = imageWriteMimeFormats();
            for (const auto &receivedFormat : m_receivedFormats) {
                if (writeFormats.contains(receivedFormat)) {
                    mime = receivedFormat;
                    break;
                }
            }
            if (mime.isEmpty()) {
                // default exchange format
                mime = QStringLiteral("image/png");
            }
        }

        if (mime.isEmpty()) {
            return QVariant();
        }
    } else {
        mime = mimeType;
    }

    int pipeFds[2];
    if (pipe(pipeFds) != 0) {
        return QVariant();
    }

    auto t = const_cast<DataControlOffer *>(this);
    t->receive(mime, pipeFds[1]);

    close(pipeFds[1]);

    /*
     * Ideally we need to introduce a non-blocking QMimeData object
     * Or a non-blocking constructor to QMimeData with the mimetypes that are relevant
     *
     * However this isn't actually any worse than X.
     */

    auto waylandApp = qGuiApp->nativeInterface<QNativeInterface::QWaylandApplication>();
    auto display = waylandApp->display();

    wl_display_flush(display);

    QFile readPipe;
    if (readPipe.open(pipeFds[0], QIODevice::ReadOnly)) {
        QByteArray data;
        if (readData(pipeFds[0], data, mime)) {
            close(pipeFds[0]);

            if (mimeType == applicationQtXImageLiteral()) {
                QImage img = QImage::fromData(data, mime.mid(mime.indexOf(QLatin1Char('/')) + 1).toLatin1().toUpper().data());
                if (!img.isNull()) {
                    m_data.insert(mimeType, img);
                    return img;
                }
            } else if (data.size() > 1 && mimeType == u"text/uri-list") {
                const auto urls = data.split('\n');
                QVariantList list;
                list.reserve(urls.size());
                for (const QByteArray &s : urls) {
                    if (QUrl url(QUrl::fromEncoded(QByteArrayView(s).trimmed())); url.isValid()) {
                        list.emplace_back(std::move(url));
                    }
                }
                m_data.insert(mimeType, list);
                return list;
            }
            m_data.insert(mimeType, data);
            return data;
        }
        close(pipeFds[0]);
    }

    return QVariant();
}

bool DataControlOffer::readData(int fd, QByteArray &data, const QString &mimeType)
{
    pollfd pfds[1];
    pfds[0].fd = fd;
    pfds[0].events = POLLIN;

    while (true) {
        const int ready = poll(pfds, 1, 1000);
        if (ready < 0) {
            if (errno != EINTR) {
                qWarning("DataControlOffer: poll() failed for mimeType %s: %s", qPrintable(mimeType), strerror(errno));
                return false;
            }
        } else if (ready == 0) {
            qWarning("DataControlOffer: timeout reading from pipe for mimeType %s", qPrintable(mimeType));
            return false;
        } else {
            char buf[4096];
            int n = read(fd, buf, sizeof buf);

            if (n < 0) {
                qWarning("DataControlOffer: read() failed for mimeType %s: %s", qPrintable(mimeType), strerror(errno));
                return false;
            } else if (n == 0) {
                return true;
            } else if (n > 0) {
                data.append(buf, n);
            }
        }
    }
}

class DataControlSource : public QObject, public QtWayland::ext_data_control_source_v1
{
    Q_OBJECT
public:
    DataControlSource(struct ::ext_data_control_source_v1 *id, QMimeData *mimeData);
    DataControlSource() = default;
    ~DataControlSource()
    {
        destroy();
    }

    QMimeData *mimeData()
    {
        return m_mimeData.get();
    }

Q_SIGNALS:
    void cancelled();

protected:
    void ext_data_control_source_v1_send(const QString &mime_type, int32_t fd) override;
    void ext_data_control_source_v1_cancelled() override;

private:
    std::unique_ptr<QMimeData> m_mimeData;
};

DataControlSource::DataControlSource(struct ::ext_data_control_source_v1 *id, QMimeData *mimeData)
    : QtWayland::ext_data_control_source_v1(id)
    , m_mimeData(mimeData)
{
    const auto formats = mimeData->formats();
    for (const QString &format : formats) {
        offer(format);
    }
    if (mimeData->hasText() && !m_mimeData->hasFormat(utf8Text())) {
        // ensure GTK applications get this mimetype to avoid them discarding the offer
        offer(QStringLiteral("text/plain;charset=utf-8"));
    }

    if (mimeData->hasImage()) {
        const QStringList imageFormats = imageWriteMimeFormats();
        for (const QString &imageFormat : imageFormats) {
            if (!formats.contains(imageFormat)) {
                offer(imageFormat);
            }
        }
    }
}

void DataControlSource::ext_data_control_source_v1_send(const QString &mime_type, int32_t fd)
{
    QString send_mime_type = mime_type;
    if (send_mime_type == utf8Text() && !m_mimeData->hasFormat(utf8Text())) {
        // if we get a request on the fallback mime, send the data from the original mime type
        send_mime_type = QStringLiteral("text/plain");
    }

    QByteArray ba;

    // adapted from QInternalMimeData::renderDataHelper
    if (mime_type == applicationQtXImageLiteral() || mime_type.startsWith(QLatin1String("image/"))) {
        if (m_mimeData->hasImage()) {
            const QImage image = qvariant_cast<QImage>(m_mimeData->imageData());
            QBuffer buf(&ba);
            buf.open(QBuffer::WriteOnly);
            if (mime_type == applicationQtXImageLiteral()) {
                // would there not be PNG ??
                image.save(&buf, "PNG");
            } else {
                image.save(&buf, mime_type.mid(mime_type.indexOf(QLatin1Char('/')) + 1).toLatin1().toUpper().data());
            }
        }
    } else { // end adapted
        ba = m_mimeData->data(send_mime_type);
    }

    auto rc = WaylandPipeWriterHelper::safeWriteWithTimeout(fd, ba.constData(), ba.size(), PIPE_BUF, 5s);
    switch (rc) {
    case WaylandPipeWriterHelper::SafeWriteResult::Ok:
        break;
    case WaylandPipeWriterHelper::SafeWriteResult::Timeout:
        qWarning() << "QWaylandDataSource: timeout writing to pipe";
        break;
    case WaylandPipeWriterHelper::SafeWriteResult::Closed:
        qWarning() << "QWaylandDataSource: peer closed pipe";
        break;
    case WaylandPipeWriterHelper::SafeWriteResult::Error:
        qWarning() << "QWaylandDataSource: write() failed";
        break;
    }
    close(fd);
}

void DataControlSource::ext_data_control_source_v1_cancelled()
{
    Q_EMIT cancelled();
}

class DataControlDevice : public QObject, public QtWayland::ext_data_control_device_v1
{
    Q_OBJECT
public:
    DataControlDevice(struct ::ext_data_control_device_v1 *id)
        : QtWayland::ext_data_control_device_v1(id)
    {
    }

    ~DataControlDevice()
    {
        destroy();
    }

    void setSelection(std::unique_ptr<DataControlSource> selection);
    QMimeData *receivedSelection()
    {
        return m_receivedSelection.get();
    }
    QMimeData *selection()
    {
        return m_selection ? m_selection->mimeData() : nullptr;
    }

    void setPrimarySelection(std::unique_ptr<DataControlSource> selection);
    QMimeData *receivedPrimarySelection()
    {
        return m_receivedPrimarySelection.get();
    }
    QMimeData *primarySelection()
    {
        return m_primarySelection ? m_primarySelection->mimeData() : nullptr;
    }

Q_SIGNALS:
    void receivedSelectionChanged();
    void selectionChanged();

    void receivedPrimarySelectionChanged();
    void primarySelectionChanged();

protected:
    void ext_data_control_device_v1_data_offer(struct ::ext_data_control_offer_v1 *id) override
    {
        // this will become memory managed when we retrieve the selection event
        // a compositor calling data_offer without doing that would be a bug
        new DataControlOffer(id);
    }

    void ext_data_control_device_v1_selection(struct ::ext_data_control_offer_v1 *id) override
    {
        if (!id) {
            m_receivedSelection.reset();
        } else {
            auto derivated = QtWayland::ext_data_control_offer_v1::fromObject(id);
            auto offer = dynamic_cast<DataControlOffer *>(derivated); // dynamic because of the dual inheritance
            m_receivedSelection.reset(offer);
        }
        Q_EMIT receivedSelectionChanged();
    }

    void ext_data_control_device_v1_primary_selection(struct ::ext_data_control_offer_v1 *id) override
    {
        if (!id) {
            m_receivedPrimarySelection.reset();
        } else {
            auto derivated = QtWayland::ext_data_control_offer_v1::fromObject(id);
            auto offer = dynamic_cast<DataControlOffer *>(derivated); // dynamic because of the dual inheritance
            m_receivedPrimarySelection.reset(offer);
        }
        Q_EMIT receivedPrimarySelectionChanged();
    }

private:
    std::unique_ptr<DataControlSource> m_selection; // selection set locally
    std::unique_ptr<DataControlOffer> m_receivedSelection; // latest selection set from externally to here

    std::unique_ptr<DataControlSource> m_primarySelection; // selection set locally
    std::unique_ptr<DataControlOffer> m_receivedPrimarySelection; // latest selection set from externally to here
    friend WaylandClipboard;
};

void DataControlDevice::setSelection(std::unique_ptr<DataControlSource> selection)
{
    {
        QMutexLocker locker(&s_clipboardLock);
        set_selection(selection->object());

        // Note the previous selection is destroyed after the set_selection request.
        m_selection = std::move(selection);
        connect(m_selection.get(), &DataControlSource::cancelled, this, [this]() {
            m_selection.reset();
        });
    }

    Q_EMIT selectionChanged();
}

void DataControlDevice::setPrimarySelection(std::unique_ptr<DataControlSource> selection)
{
    {
        QMutexLocker locker(&s_clipboardLock);
        set_primary_selection(selection->object());

        // Note the previous selection is destroyed after the set_primary_selection request.
        m_primarySelection = std::move(selection);
        connect(m_primarySelection.get(), &DataControlSource::cancelled, this, [this]() {
            m_primarySelection.reset();
        });
    }

    Q_EMIT primarySelectionChanged();
}

class ClipboardThread : public QThread
{
public:
    ClipboardThread(wl_display *display)
        : m_display(display)
    {
#if QT_VERSION_CHECK(WAYLAND_VERSION_MAJOR, WAYLAND_VERSION_MINOR, WAYLAND_VERSION_MICRO) >= QT_VERSION_CHECK(1, 23, 0)
        m_queue = wl_display_create_queue_with_name(m_display, "ksystemclipboard queue");
#else
        m_queue = wl_display_create_queue(m_display);
#endif
    }

    ~ClipboardThread()
    {
        if (m_callback) {
            wl_callback_destroy(m_callback);
        }
        wl_event_queue_destroy(m_queue);
    }

    void syncQueue()
    {
        // wl_display_dispatch_queue will block until it gets an event, as KSystemClipboard
        // has the lifespan of the QApplication and the QPA is still alive
        // we need something to explicitly wake up the event queue up before the main thread
        // blocks waiting for this thread.
        auto wrapped_display = wl_proxy_create_wrapper(m_display);
        wl_proxy_set_queue(static_cast<wl_proxy *>(wrapped_display), m_queue);
        m_callback = wl_display_sync(static_cast<struct wl_display *>(wrapped_display));
        wl_display_flush(m_display);
        wl_proxy_wrapper_destroy(wrapped_display);
    }

    void run() override
    {
        while (!isInterruptionRequested()) {
            while (wl_display_prepare_read_queue(m_display, m_queue) != 0) {
                QMutexLocker lock(&s_clipboardLock);
                wl_display_dispatch_queue_pending(m_display, m_queue);
            }
            wl_display_flush(m_display);

            struct pollfd pfd[1];
            pfd[0].fd = wl_display_get_fd(m_display);
            pfd[0].events = POLLIN;
            int pollRet = poll(pfd, 1, -1);
            if (pollRet < 0 && errno != EPIPE) {
                wl_display_cancel_read(m_display);
                break;
            } else {
                wl_display_read_events(m_display);
            }

            QMutexLocker lock(&s_clipboardLock);
            wl_display_dispatch_queue_pending(m_display, m_queue);
        }
    }
    wl_callback *m_callback = nullptr;
    wl_event_queue *m_queue = nullptr;
    wl_display *m_display = nullptr;
};

WaylandClipboard::WaylandClipboard(QObject *parent)
    : KSystemClipboard(parent)
    , m_manager(new DataControlDeviceManager)
{
    connect(m_manager.get(), &DataControlDeviceManager::activeChanged, this, [this]() {
        if (m_manager->isActive()) {
            auto waylandApp = qGuiApp->nativeInterface<QNativeInterface::QWaylandApplication>();
            m_thread = std::make_unique<ClipboardThread>(waylandApp->display());
            auto seat = waylandApp->seat();

            if (!seat) {
                return;
            }
            wl_proxy_set_queue(reinterpret_cast<wl_proxy *>(m_manager->object()), m_thread->m_queue);
            m_device.reset(new DataControlDevice(m_manager->get_data_device(seat)));
            m_device->moveToThread(m_thread.get());

            connect(m_device.get(), &DataControlDevice::receivedSelectionChanged, this, [this]() {
                // When our source is still valid, so the offer is for setting it or we emit changed when it is cancelled
                if (!m_device->selection()) {
                    Q_EMIT changed(QClipboard::Clipboard);
                }
            });
            connect(m_device.get(), &DataControlDevice::selectionChanged, this, [this]() {
                Q_EMIT changed(QClipboard::Clipboard);
            });

            connect(m_device.get(), &DataControlDevice::receivedPrimarySelectionChanged, this, [this]() {
                // When our source is still valid, so the offer is for setting it or we emit changed when it is cancelled
                if (!m_device->primarySelection()) {
                    Q_EMIT changed(QClipboard::Selection);
                }
            });
            connect(m_device.get(), &DataControlDevice::primarySelectionChanged, this, [this]() {
                Q_EMIT changed(QClipboard::Selection);
            });
            m_thread->start();

        } else {
            m_device.reset();
            m_thread->requestInterruption();
            m_thread->wait();
            m_thread.reset();
        }
    });

    m_manager->instantiate();
}

WaylandClipboard::~WaylandClipboard()
{
    if (m_thread && m_thread->isRunning()) {
        m_thread->requestInterruption();
        m_thread->syncQueue();
        m_thread->wait();
    }
}

WaylandClipboard *WaylandClipboard::create(QObject *parent)
{
    auto clipboard = new WaylandClipboard(parent);
    if (clipboard->isValid()) {
        return clipboard;
    }
    delete clipboard;
    return nullptr;
}

bool WaylandClipboard::isValid()
{
    return m_manager && m_manager->isInitialized();
}

void WaylandClipboard::setMimeData(QMimeData *mime, QClipboard::Mode mode)
{
    if (!m_device) {
        return;
    }

    auto source = std::make_unique<DataControlSource>(m_manager->create_data_source(), mime);
    source->moveToThread(m_thread.get());
    if (mode == QClipboard::Clipboard) {
        m_device->setSelection(std::move(source));
    } else if (mode == QClipboard::Selection) {
        m_device->setPrimarySelection(std::move(source));
    }
}

void WaylandClipboard::clear(QClipboard::Mode mode)
{
    if (!m_device) {
        return;
    }

    if (mode == QClipboard::Clipboard) {
        QMutexLocker locker(&s_clipboardLock);
        m_device->set_selection(nullptr);
        m_device->m_selection.reset();
    } else if (mode == QClipboard::Selection) {
        QMutexLocker locker(&s_clipboardLock);
        m_device->set_primary_selection(nullptr);
        m_device->m_primarySelection.reset();
    }
}

const QMimeData *WaylandClipboard::mimeData(QClipboard::Mode mode) const
{
    if (!m_device) {
        return nullptr;
    }

    // WaylandClipboard owns the mimedata, but the caller can read it until the next event loop runs.
    auto lockWithUnlockLater = [this]() {
        s_clipboardLock.lock();
        QTimer::singleShot(0, this, [] {
            s_clipboardLock.unlock();
        });
    };

    // return our locally set selection if it's not cancelled to avoid copying data to ourselves
    if (mode == QClipboard::Clipboard) {
        QMutexLocker lock(&s_clipboardLock);
        if (m_device->selection()) {
            lockWithUnlockLater();
            return m_device->selection();
        }

        // This application owns the clipboard via the regular data_device, use it so we don't block ourselves
        if (QGuiApplication::clipboard()->ownsClipboard()) {
            return QGuiApplication::clipboard()->mimeData(mode);
        }

        lockWithUnlockLater();
        return m_device->receivedSelection();
    } else if (mode == QClipboard::Selection) {
        QMutexLocker lock(&s_clipboardLock);
        if (m_device->primarySelection()) {
            lockWithUnlockLater();
            return m_device->primarySelection();
        }

        // This application owns the primary selection via the regular primary_selection_device, use it so we don't block ourselves
        if (QGuiApplication::clipboard()->ownsSelection()) {
            return QGuiApplication::clipboard()->mimeData(mode);
        }

        lockWithUnlockLater();
        return m_device->receivedPrimarySelection();
    }
    return nullptr;
}

#include "moc_waylandclipboard_p.cpp"
#include "waylandclipboard.moc"

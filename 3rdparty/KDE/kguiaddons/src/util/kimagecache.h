/*  This file is part of the KDE project.
    SPDX-FileCopyrightText: 2010 Michael Pyne <mpyne@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef KIMAGECACHE_H
#define KIMAGECACHE_H

// check that KGUIADDONS_LIB is defined in case the application is not using CMake
// (if KGUIADDONS_LIB is not defined, we cannot assume that KCOREADDONS_LIB not being
// defined means that we are not linked against KCoreAddons)
#if defined(KGUIADDONS_LIB) && !defined(KCOREADDONS_LIB)
#ifdef __GNUC__
#warning "KImageCache requires KF6CoreAddons (for kshareddatacache.h)"
#else
#pragma message("KImageCache requires KF6CoreAddons (for kshareddatacache.h)")
#endif
#endif

#include <klocalimagecacheimpl.h>
#include <kshareddatacache.h>

#include <QImage>
#include <QPixmap>

#define KImageCache KSharedPixmapCacheMixin<KSharedDataCache>

/*!
 * \class KSharedPixmapCacheMixin
 * \inmodule KGuiAddons
 * \inheaderfile KImageCache
 *
 * \brief A simple wrapping layer over KSharedDataCache to support caching
 * images and pixmaps.
 *
 * This class can be used to share images between different processes, which
 * is useful when it is known that such images will be used across many
 * processes, or when creating the image is expensive.
 *
 * In addition, the class also supports caching QPixmaps \e{in a single
 * process} using the setPixmapCaching() function.
 *
 * Tips for use: If you already have QPixmaps that you intend to use, and
 * you do not need access to the actual image data, then try to store and
 * retrieve QPixmaps for use.
 *
 * On the other hand, if you will need to store and retrieve actual image
 * data (to modify the image after retrieval for instance) then you should
 * use QImage to save the conversion cost from QPixmap to QImage.
 *
 * KSharedPixmapCacheMixin is a subclass of KSharedDataCache, so all of the methods that
 * can be used with KSharedDataCache can be used with KSharedPixmapCacheMixin,
 * \e{with the exception of KSharedDataCache::insert() and
 * KSharedDataCache::find()}.
 *
 * \since 4.5
 */
template<class T>
class KSharedPixmapCacheMixin : public T, private KLocalImageCacheImplementation
{
public:
    /*!
     * Constructs an image cache, named by \a cacheName, with a default
     * size of \a defaultCacheSize.
     *
     * \a cacheName Name of the cache to use.
     *
     * \a defaultCacheSize The default size, in bytes, of the cache.
     *
     *  The actual on-disk size will be slightly larger. If the cache already
     *  exists, it will not be resized. If it is required to resize the
     *  cache then use the deleteCache() function to remove that cache first.
     *
     * \a expectedItemSize The expected general size of the items to be
     *  added to the image cache, in bytes. Use 0 if you just want a default
     *  item size.
     */
    KSharedPixmapCacheMixin(const QString &cacheName, unsigned defaultCacheSize, unsigned expectedItemSize = 0)
        : T(cacheName, defaultCacheSize, expectedItemSize)
        , KLocalImageCacheImplementation(defaultCacheSize)
    {
    }

    /*!
     * Inserts the pixmap given by \a pixmap to the cache, accessible with
     * \a key. The pixmap must be converted to a QImage in order to be stored
     * into shared memory. In order to prevent unnecessary conversions from
     * taking place \a pixmap will also be cached (but not in shared
     * memory) and would be accessible using findPixmap() if pixmap caching is
     * enabled.
     *
     * \a key Name to access \a pixmap with.
     *
     * \a pixmap The pixmap to add to the cache.
     *
     * Returns \c true if the pixmap was successfully cached, \c false otherwise.
     *
     * \sa setPixmapCaching()
     */
    bool insertPixmap(const QString &key, const QPixmap &pixmap)
    {
        insertLocalPixmap(key, pixmap);

        // One thing to think about is only inserting things to the shared cache
        // that are frequently used. But that would require tracking the use count
        // in our local cache too, which I think is probably too much work.

        return insertImage(key, pixmap.toImage());
    }

    /*!
     * Inserts the \a image into the shared cache, accessible with \a key. This
     * variant is preferred over insertPixmap() if your source data is already a
     * QImage, if it is essential that the image be in shared memory (such as
     * for SVG icons which have a high render time), or if it will need to be
     * in QImage form after it is retrieved from the cache.
     *
     * \a key Name to access \a image with.
     *
     * \a image The image to add to the shared cache.
     *
     * Returns \c true if the image was successfully cached, \c false otherwise.
     */
    bool insertImage(const QString &key, const QImage &image)
    {
        if (this->insert(key, serializeImage(image))) {
            updateModifiedTime();
            return true;
        }

        return false;
    }

    /*!
     * Copies the cached pixmap identified by \a key to \a destination. If no such
     * pixmap exists \a destination is unchanged.
     *
     * Returns \c true if the pixmap identified by \a key existed, \c false otherwise.
     * \sa setPixmapCaching()
     */
    bool findPixmap(const QString &key, QPixmap *destination) const
    {
        if (findLocalPixmap(key, destination)) {
            return true;
        }

        QByteArray cachedData;
        if (!this->find(key, &cachedData) || cachedData.isNull()) {
            return false;
        }

        if (destination) {
            destination->loadFromData(cachedData, "PNG", Qt::NoOpaqueDetection);

            // Manually re-insert to pixmap cache if we'll be using this one.
            insertLocalPixmap(key, *destination);
        }

        return true;
    }

    /*!
     * Copies the cached image identified by \a key to \a destination. If no such
     * image exists \a destination is unchanged.
     *
     * Returns \c true if the image identified by \a key existed, \c false otherwise.
     */
    bool findImage(const QString &key, QImage *destination) const
    {
        QByteArray cachedData;
        if (!this->find(key, &cachedData) || cachedData.isNull()) {
            return false;
        }

        if (destination) {
            destination->loadFromData(cachedData, "PNG");
        }

        return true;
    }

    /*!
     * Removes all entries from the cache. In addition any cached pixmaps (as per
     * setPixmapCaching()) are also removed.
     */
    void clear()
    {
        clearLocalCache();
        T::clear();
    }

    /*!
     * Returns the time that an image or pixmap was last inserted into a cache.
     */
#ifdef Q_QDOC
    QDateTime lastModifiedTime() const;
#else
    using KLocalImageCacheImplementation::lastModifiedTime;
#endif

    /*!
     * Returns if QPixmaps added with insertPixmap() will be stored in a local
     * pixmap cache as well as the shared image cache. The default is to cache
     * pixmaps locally.
     */
#ifdef Q_QDOC
    bool pixmapCaching() const;
#else
    using KLocalImageCacheImplementation::pixmapCaching;
#endif

    /*!
     * Enables or disables local pixmap caching. If it is anticipated that a pixmap
     * will be frequently needed then this can actually save memory overall since the
     * X server or graphics card will not have to store duplicate copies of the same
     * image.
     *
     * \a enable Enables pixmap caching if true, disables otherwise.
     */
#ifdef Q_QDOC
    void setPixmapCaching(bool enable)
#else
    using KLocalImageCacheImplementation::setPixmapCaching;
#endif

    /*!
     * Returns the highest memory size in bytes to be used by cached pixmaps.
     * \since 4.6
     */
#ifdef Q_QDOC
        int pixmapCacheLimit() const;
#else
    using KLocalImageCacheImplementation::pixmapCacheLimit;
#endif

    /*!
     * Sets the highest memory size the pixmap cache should use.
     *
     * \a size The size in bytes
     * \since 4.6
     */
#ifdef Q_QDOC
    void setPixmapCacheLimit(int size);
#else
    using KLocalImageCacheImplementation::setPixmapCacheLimit;
#endif
};

#endif /* KIMAGECACHE_H */

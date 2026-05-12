/*
    This file is part of the KDE Frameworks

    SPDX-FileCopyrightText: 2022 Mirco Miranda

    SPDX-License-Identifier: LGPL-2.0-or-later
*/
#ifndef KMEMORYINFO_H
#define KMEMORYINFO_H

#include <QSharedDataPointer>

#include <kcoreaddons_export.h>

class KMemoryInfoPrivate;

/*!
 * \class KMemoryInfo
 * \inmodule KCoreAddons
 *
 * \brief The KMemoryInfo class provides an interface to get memory information (RAM/SWAP).
 *
 * To use the class, simply create an instance.
 * \code
 * KMemoryInfo memInfo;
 * if (!memInfo.isNull()) {
 *     ...
 * }
 * \endcode
 *
 * \since 5.95
 */
class KCOREADDONS_EXPORT KMemoryInfo
{
public:
    ~KMemoryInfo();

    /*!
     * Constructs a class with a snapshot of the state of the memory. If an error occurs, a null object is returned.
     * \sa isNull.
     */
    KMemoryInfo();

    KMemoryInfo(const KMemoryInfo &other);

    KMemoryInfo &operator=(const KMemoryInfo &other);

    /*!
     * Returns \c true if this memoryinfo is equal to the other memoryinfo, otherwise \c false.
     */
    bool operator==(const KMemoryInfo &other) const;

    /*!
     * Returns \c true if this memoryinfo is different from the other memoryinfo, otherwise \c false.
     */
    bool operator!=(const KMemoryInfo &other) const;

    /*!
     * Returns \c true if the class is null, otherwise \c false.
     */
    bool isNull() const;

    /*!
     * Returns The total system RAM in bytes.
     */
    quint64 totalPhysical() const;

    /*!
     * The free memory is the amount of free RAM as reported by the operating system.
     * This value is often tainted with caches and buffers used by the operating system, resulting in a low value.
     *
     * \note Don't use this value to determine if you have enough RAM for your data.
     *
     * Returns the free RAM reported by OS in bytes.
     * \sa availablePhysical.
     */
    quint64 freePhysical() const;

    /*!
     * The available memory is the free RAM without considering caches and buffers allocated by the operating system.
     *
     * \note You should always use this value to check if there is enough RAM for your data.
     *
     * Returns The memory available to the processes in bytes.
     * \sa freePhysical.
     */
    quint64 availablePhysical() const;

    /*!
     * Returns The size of RAM used as cache in bytes.
     */
    quint64 cached() const;

    /*!
     * Returns The size of RAM used as buffers in bytes. This value can be zero.
     */
    quint64 buffers() const;

    /*!
     * Returns The size of swap file in bytes.
     *
     * \note On an operating system where the paging file is dynamically allocated, this value can be zero when no memory pages are swapped.
     */
    quint64 totalSwapFile() const;

    /*!
     * Returns The free swap size in bytes.
     */
    quint64 freeSwapFile() const;

private:
    KCOREADDONS_NO_EXPORT bool update();

    QSharedDataPointer<KMemoryInfoPrivate> d;
};

#endif // KMEMORYINFO_H

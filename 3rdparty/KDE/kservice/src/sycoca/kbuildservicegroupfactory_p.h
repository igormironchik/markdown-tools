/*
    This file is part of the KDE project
    SPDX-FileCopyrightText: 2000 Waldo Bastian <bastian@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef KBUILD_SERVICE_GROUP_FACTORY_H
#define KBUILD_SERVICE_GROUP_FACTORY_H

#include <QStringList>
#include <kservice.h>
#include <kservicegroupfactory_p.h>

/*!
 * Service group factory for building ksycoca
 * \internal
 */
class KBuildServiceGroupFactory : public KServiceGroupFactory
{
public:
    /*!
     * Create factory
     */
    explicit KBuildServiceGroupFactory(KSycoca *db);

    ~KBuildServiceGroupFactory() override;

    /*!
     * Create new entry.
     */
    KServiceGroup *createEntry(const QString &) const override;

    KServiceGroup *createEntry(int) const override
    {
        assert(0);
        return nullptr;
    }

    /*!
     * Adds the entry \a newEntry to the menu \a menuName
     */
    void addNewEntryTo(const QString &menuName, const KService::Ptr &newEntry);

    /*!
     * Adds the entry \a newEntry to the "parent group" \a parent, creating
     * the group if necessary.
     * A "parent group" is a group of services that all have the same
     * "X-KDE-ParentApp".
     */
    void addNewChild(const QString &parent, const KSycocaEntry::Ptr &newEntry);

    /*!
     * Add new menu \a menuName defined by \a file
     * When \a entry is non-null it is re-used, otherwise a new group is created.
     * A pointer to the group is returned.
     */
    KServiceGroup::Ptr addNew(const QString &menuName, const QString &file, KServiceGroup::Ptr entry, bool isDeleted);

    /*!
     * Find a group ( by desktop path, e.g. "Applications/Editors")
     */
    KServiceGroup::Ptr findGroupByDesktopPath(const QString &_name, bool deep = true) override;

    /*!
     * Add a new menu entry
     */
    void addEntry(const KSycocaEntry::Ptr &newEntry) override;

    /*!
     * Write out servicegroup  specific index files.
     */
    void save(QDataStream &str) override;

    /*!
     * Write out header information
     */
    void saveHeader(QDataStream &str) override;
};

#endif

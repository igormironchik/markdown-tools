/*
    This file is part of KDE.
    SPDX-FileCopyrightText: 2001, 2002, 2003 Cornelius Schumacher <schumacher@kde.org>
    SPDX-FileCopyrightText: 2003 Waldo Bastian <bastian@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KCONFIGSKELETON_H
#define KCONFIGSKELETON_H

#include <kconfiggui_export.h>

#include <kcoreconfigskeleton.h>

#include <QColor>
#include <QFont>

/*!
 * \class KConfigSkeleton
 * \inmodule KConfigGui
 *
 * \brief Class for handling preferences settings for an application.
 *
 * This class extends KCoreConfigSkeleton with support for GUI types.
 */
class KCONFIGGUI_EXPORT KConfigSkeleton : public KCoreConfigSkeleton
{
    Q_OBJECT
public:
    /*!
     * \inmodule KConfigGui
     * \brief Class for handling a color preferences item.
     */
    class KCONFIGGUI_EXPORT ItemColor : public KConfigSkeletonGenericItem<QColor>
    {
    public:
        /*! */
        ItemColor(const QString &_group, const QString &_key, QColor &reference, const QColor &defaultValue = QColor(128, 128, 128));

        void readConfig(KConfig *config) override;

        void setProperty(const QVariant &p) override;

        bool isEqual(const QVariant &p) const override;

        QVariant property() const override;
    };

    /*!
     * \inmodule KConfigGui
     * \brief Class for handling a font preferences item.
     */
    class KCONFIGGUI_EXPORT ItemFont : public KConfigSkeletonGenericItem<QFont>
    {
    public:
        /*! */
        ItemFont(const QString &_group, const QString &_key, QFont &reference, const QFont &defaultValue = QFont());

        void readConfig(KConfig *config) override;

        void setProperty(const QVariant &p) override;

        bool isEqual(const QVariant &p) const override;

        QVariant property() const override;
    };

public:
    /*!
     * Constructs a configuration skeleton with the given \a configname
     * as a child of \a parent.
     *
     * If no name is given, the default config
     * as returned by KSharedConfig::openConfig() is used.
     */
    explicit KConfigSkeleton(const QString &configname = QString(), QObject *parent = nullptr);

    /*!
     * Constructs a configuration skeleton with the given \a config object
     * as a child of \a parent.
     */
    explicit KConfigSkeleton(KSharedConfig::Ptr config, QObject *parent = nullptr);

    /*!
     * Constructs a configuration skeleton with a given \a config object
     * as a child of \a parent.
     *
     * The \a value must be KCoreConfigSkeleton::IsStdUniqPtr.
     *
     * The use of this single-valued enum is mandatory to disambiguate with the other constructor
     * that may otherwise be ambiguous in case of default nullptr
     * KCoreConfigSkeleton(KSharedConfig::Ptr(nullptr)) or
     * KCoreConfigSkeleton(std::unique_ptr<KConfig> (nullptr)) with parent = nullptr.
     *
     * \since 6.23
     * \sa KCoreConfigSkeleton::DisambiguateConstructor
     */
    explicit KConfigSkeleton(std::unique_ptr<KConfig> config, KCoreConfigSkeleton::DisambiguateConstructor value, QObject *parent = nullptr);

    /*!
     * Registers a QColor item with the given \a name,
     * the initial value \a reference to hold in the item, and the \a defaultValue for the item used
     * when the config file does not yet contain the \a key of this item.
     *
     * The \a reference should be a pointer to the variable that is set by read() calls
     * and read by save() calls.
     *
     * If \a key is a null string, \a name is used as key.
     */
    ItemColor *addItemColor(const QString &name, QColor &reference, const QColor &defaultValue = QColor(128, 128, 128), const QString &key = QString());

    /*!
     * Registers a QFont item with the given \a name,
     * the initial value \a reference to hold in the item, and the \a defaultValue for the item used
     * when the config file does not yet contain the \a key of this item.
     *
     * The \a reference should be a pointer to the variable that is set by read() calls
     * and read by save() calls.
     *
     * If \a key is a null string, \a name is used as key.
     */
    ItemFont *addItemFont(const QString &name, QFont &reference, const QFont &defaultValue = QFont(), const QString &key = QString());
};

#endif

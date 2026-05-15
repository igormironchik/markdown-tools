/*
    This file is part of KDE.

    SPDX-FileCopyrightText: 2001, 2002, 2003 Cornelius Schumacher <schumacher@kde.org>
    SPDX-FileCopyrightText: 2003 Waldo Bastian <bastian@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KCORECONFIGSKELETON_H
#define KCORECONFIGSKELETON_H

#include <kconfigcore_export.h>

#include <kconfiggroup.h>
#include <ksharedconfig.h>

#include <QDate>
#include <QHash>
#include <QRect>
#include <QStringList>
#include <QUrl>
#include <QVariant>

class KCoreConfigSkeletonPrivate;

class KConfigSkeletonItemPrivate;
/*!
 * \class KConfigSkeletonItem
 * \inmodule KConfigCore
 * \inheaderfile KCoreConfigSkeleton
 *
 * \brief Class for storing a preferences setting.
 * \sa KCoreConfigSkeleton
 *
 * This class represents one preferences setting as used by KCoreConfigSkeleton.
 * Subclasses of KConfigSkeletonItem implement storage functions for a certain type of
 * setting. Normally you don't have to use this class directly. Use the special
 * addItem() functions of KCoreConfigSkeleton instead. If you subclass this class you will
 * have to register instances with the function KCoreConfigSkeleton::addItem().
 */
class KCONFIGCORE_EXPORT KConfigSkeletonItem
{
    Q_DECLARE_PRIVATE(KConfigSkeletonItem)
public:
    /*!
     * \typedef KConfigSkeletonItem::List
     */
    typedef QList<KConfigSkeletonItem *> List;

    /*!
     * \typedef KConfigSkeletonItem::Dict
     */
    typedef QHash<QString, KConfigSkeletonItem *> Dict;

    /*!
     * \typedef KConfigSkeletonItem::DictIterator
     */
    typedef QHash<QString, KConfigSkeletonItem *>::Iterator DictIterator;

    /*!
     * Constructs a skeleton item instance with the given Config file \a _group and \a _key.
     */
    KConfigSkeletonItem(const QString &_group, const QString &_key);

    virtual ~KConfigSkeletonItem();

    /*!
     * Sets the config file \a _group.
     */
    void setGroup(const QString &_group);

    /*!
     * Returns the name of the config file group.
     */
    QString group() const;

    /*!
     * Sets a config file group by giving the KConfigGroup \a cg.
     *
     * Allow the item to be in nested groups.
     * \since 5.68
     */
    void setGroup(const KConfigGroup &cg);

    /*!
     * Returns a KConfigGroup, the one provided by setGroup(const KConfigGroup&) if it's valid,
     * or make one from \a config and item's group.
     *
     * \sa setGroup()
     * \since 5.68
     */
    KConfigGroup configGroup(KConfig *config) const;

    /*!
     * Sets the config file \a _key.
     */
    void setKey(const QString &_key);

    /*!
     * Returns the config file key.
     */
    QString key() const;

    /*!
     * Sets the internal name of the settings entry with the given \a _name.
     */
    void setName(const QString &_name);

    /*!
     * Returns the internal name of the settings entry.
     */
    QString name() const;

    /*!
     * Sets the label \a l providing a translated one-line description of the item.
     */
    void setLabel(const QString &l);

    /*!
     * Returns the label of the item.
     * \sa setLabel()
     */
    QString label() const;

    /*!
     * Sets the ToolTip \a t description of this item.
     * \since 4.2
     */
    void setToolTip(const QString &t);

    /*!
     * Returns the ToolTip description of this item.
     * \sa setToolTip()
     * \since 4.2
     */
    QString toolTip() const;

    /*!
     * Sets WhatsThis description \a w of this item.
     */
    void setWhatsThis(const QString &w);

    /*!
     * Returns the WhatsThis description of item.
     * \sa setWhatsThis()
     */
    QString whatsThis() const;

    /*!
     * Sets the write \a flags to be used when writing the configuration.
     * \since 5.58
     * \sa KConfigBase::WriteConfigFlags
     * \sa KConfigWatcher
     */
    void setWriteFlags(KConfigBase::WriteConfigFlags flags);

    /*!
     * Returns write flags to be used when writing the configuration.
     *
     * They should be passed to every call of KConfigGroup::writeEntry() and KConfigGroup::revertToDefault().
     * \since 5.58
     */
    KConfigBase::WriteConfigFlags writeFlags() const;

    /*!
     * This function is called by KCoreConfigSkeleton to read the value for this setting
     * from a config file.
     */
    virtual void readConfig(KConfig *) = 0;

    /*!
     * This function is called by KCoreConfigSkeleton to write the value of this setting
     * to a config file.
     * Make sure to pass writeFlags() to every call of KConfigGroup::writeEntry() and KConfigGroup::revertToDefault().
     */
    virtual void writeConfig(KConfig *) = 0;

    /*!
     * Reads the global default value.
     */
    virtual void readDefault(KConfig *) = 0;

    /*!
     * Sets the settings item to the property \a p.
     */
    virtual void setProperty(const QVariant &p) = 0;

    /*!
     * Checks whether the item is equal to \a p.
     *
     * Use this function to compare items that use custom types,
     * because QVariant::operator== will not work for those.
     *
     * Returns \c true if the item is equal to \a p, \c false otherwise
     */
    virtual bool isEqual(const QVariant &p) const = 0;

    /*!
     * Returns this item as a property.
     */
    virtual QVariant property() const = 0;

    /*!
     * Returns the minimum value of this item or invalid if not specified.
     */
    virtual QVariant minValue() const;

    /*!
     * Returns the maximum value of this item or invalid if not specified.
     */
    virtual QVariant maxValue() const;

    /*!
     * Sets the current value to the default value.
     */
    virtual void setDefault() = 0;

    /*!
     * Exchanges the current value with the default value.
     *
     * Used by KCoreConfigSkeleton::useDefaults(bool).
     */
    virtual void swapDefault() = 0;

    /*!
     * Returns if the entry can be modified.
     */
    bool isImmutable() const;

    /*!
     * Returns if this item is set to its default value.
     * \since 5.64
     */
    bool isDefault() const;

    /*!
     * Returns if the item has a different value than the
     * previously loaded value.
     * \since 5.64
     */
    bool isSaveNeeded() const;

    /*!
     * Returns the default value.
     * \since 5.74
     */
    QVariant getDefault() const;

protected:
    KCONFIGCORE_NO_EXPORT explicit KConfigSkeletonItem(KConfigSkeletonItemPrivate &dd, const QString &_group, const QString &_key);

    /*!
     * Sets mIsImmutable to \c true if mKey in config is immutable.
     *
     * \a group KConfigGroup to check if mKey is immutable in
     */
    void readImmutability(const KConfigGroup &group);

    QString mGroup;
    QString mKey;
    QString mName;

    // HACK: Necessary to avoid introducing new virtuals in KConfigSkeletonItem
    // KF7 TODO: Use proper pure virtuals in KConfigSkeletonItem
    void setIsDefaultImpl(const std::function<bool()> &impl);
    void setIsSaveNeededImpl(const std::function<bool()> &impl);
    void setGetDefaultImpl(const std::function<QVariant()> &impl);

    KConfigSkeletonItemPrivate *const d_ptr;
};

class KPropertySkeletonItemPrivate;

/*!
 * \class KPropertySkeletonItem
 * \inmodule KConfigCore
 * \inheaderfile KCoreConfigSkeleton
 *
 * \brief Class for proxying a QObject property as a preferences setting.
 * \sa KConfigSkeletonItem
 *
 * This class represents one preferences setting as used by KCoreConfigSkeleton.
 * Unlike other KConfigSkeletonItem subclasses, this one won't store the preference
 * in KConfig but will use a QObject property as storage, allowing to implement callbacks to run when a configuration property changes.
 * You will have to register instances of this class with the function KCoreConfigSkeleton::addItem().
 *
 * \since 5.65
 */
class KCONFIGCORE_EXPORT KPropertySkeletonItem : public KConfigSkeletonItem
{
    Q_DECLARE_PRIVATE(KPropertySkeletonItem)
public:
    /*!
     * Constructs a property skeleton item instance that allows to manage the \a propertyName and the property's \a defaultValue present in a QObject \a object.
     */
    KPropertySkeletonItem(QObject *object, const QByteArray &propertyName, const QVariant &defaultValue);

    QVariant property() const override;
    void setProperty(const QVariant &p) override;
    bool isEqual(const QVariant &p) const override;

    void readConfig(KConfig *) override;
    void writeConfig(KConfig *) override;

    void readDefault(KConfig *) override;
    void setDefault() override;
    void swapDefault() override;

    /*!
     * Sets a notify function \a impl that will be invoked when the value of the property changes.
     * \since 5.68
     */
    void setNotifyFunction(const std::function<void()> &impl);
};

/*!
 * \class KConfigSkeletonGenericItem
 * \inmodule KConfigCore
 * \inheaderfile KCoreConfigSkeleton
 *
 * \brief Base class for storing a preferences setting of type \a T.
 */
template<typename T>
class KConfigSkeletonGenericItem : public KConfigSkeletonItem
{
public:
    /*!
     * \brief Constructs a generic skeleton item with the given \a _group, \a _key, the initial value \a reference to hold in the item, and the \a defaultValue for the item.
     */
    KConfigSkeletonGenericItem(const QString &_group, const QString &_key, T &reference, T defaultValue)
        : KConfigSkeletonItem(_group, _key)
        , mReference(reference)
        , mDefault(defaultValue)
        , mLoadedValue(defaultValue)
    {
        setIsDefaultImpl([this] {
            return mReference == mDefault;
        });
        setIsSaveNeededImpl([this] {
            return mReference != mLoadedValue;
        });
        setGetDefaultImpl([this] {
            return QVariant::fromValue(mDefault);
        });
    }

    /*!
     * Sets the value \a v of this KConfigSkeletonItem.
     */
    void setValue(const T &v)
    {
        mReference = v;
    }

    /*!
     * Returns the value of this KConfigSkeletonItem.
     */
    T &value()
    {
        return mReference;
    }

    /*!
     * Returns the const value of this KConfigSkeletonItem.
     */
    const T &value() const
    {
        return mReference;
    }

    /*!
     * Sets the default value \a v for this item.
     */
    virtual void setDefaultValue(const T &v)
    {
        mDefault = v;
    }

    /*!
     * Sets the value for this item to the default value.
     */
    void setDefault() override
    {
        mReference = mDefault;
    }

    void writeConfig(KConfig *config) override
    {
        if (mReference != mLoadedValue) { // Is this needed?
            KConfigGroup cg = configGroup(config);
            if ((mDefault == mReference) && !cg.hasDefault(mKey)) {
                cg.revertToDefault(mKey, writeFlags());
            } else {
                cg.writeEntry(mKey, mReference, writeFlags());
            }
            mLoadedValue = mReference;
        }
    }

    void readDefault(KConfig *config) override
    {
        config->setReadDefaults(true);
        readConfig(config);
        config->setReadDefaults(false);
        mDefault = mReference;
    }

    void swapDefault() override
    {
        T tmp = mReference;
        mReference = mDefault;
        mDefault = tmp;
    }

protected:
    T &mReference; // Stores the value for this item
    T mDefault; // The default value for this item
    T mLoadedValue;
};

/*!
 * \class KConfigCompilerSignallingItem
 * \inmodule KConfigCore
 * \inheaderfile KCoreConfigSkeleton
 *
 * This class wraps a KConfigSkeletonItem and invokes a function whenever the value changes.
 * That function must take one quint64 parameter. Whenever the property value of the wrapped KConfigSkeletonItem
 * changes this function will be invoked with the stored user data passed in the constructor.
 * It does not call a function with the new value since this class is designed solely for the kconfig_compiler generated
 * code and is therefore probably not suited for any other usecases.
 *
 * \internal
 *
 * \sa KConfigSkeletonItem
 */
class KCONFIGCORE_EXPORT KConfigCompilerSignallingItem : public KConfigSkeletonItem
{
public:
    typedef void (QObject::*NotifyFunction)(quint64 arg);
    /*!
     * Constructor.
     *
     * \a item the KConfigSkeletonItem to wrap
     *
     * \a targetFunction the method to invoke whenever the value of \a item changes
     *
     * \a object The object on which the method is invoked.
     *
     * \a userData This data will be passed to \a targetFunction on every property change
     */
    KConfigCompilerSignallingItem(KConfigSkeletonItem *item, QObject *object, NotifyFunction targetFunction, quint64 userData);
    ~KConfigCompilerSignallingItem() override;

    void readConfig(KConfig *) override;
    void writeConfig(KConfig *) override;
    void readDefault(KConfig *) override;
    void setProperty(const QVariant &p) override;
    bool isEqual(const QVariant &p) const override;
    QVariant property() const override;
    QVariant minValue() const override;
    QVariant maxValue() const override;
    void setDefault() override;
    void swapDefault() override;
    // KF7 TODO - fix this
    // Ideally we would do this in an overload of KConfigSkeletonItem, but
    // given we can't, I've shadowed the method. This isn't pretty, but given
    // the docs say it should generally only be used from auto generated code,
    // should be fine.
    void setWriteFlags(KConfigBase::WriteConfigFlags flags);
    KConfigBase::WriteConfigFlags writeFlags() const;
    void setGroup(const KConfigGroup &cg);
    KConfigGroup configGroup(KConfig *config) const;
    // END TODO

private:
    inline void invokeNotifyFunction()
    {
        // call the pointer to member function using the strange ->* operator
        (mObject->*mTargetFunction)(mUserData);
    }

private:
    QScopedPointer<KConfigSkeletonItem> mItem;
    NotifyFunction mTargetFunction;
    QObject *mObject;
    quint64 mUserData;
};

/*!
 * \class KCoreConfigSkeleton
 * \inmodule KConfigCore
 * \inheaderfile KCoreConfigSkeleton
 *
 * \brief Class for handling preferences settings for an application.
 *
 * This class provides an interface to settings that can be used to create preferences UIs.
 * Use KConfigSkeleton if you need GUI types like QColor and QFont as well.
 *
 * Preferences items
 * can be registered by the addItem() function corresponding to the data type of
 * the setting. KCoreConfigSkeleton then handles reading and writing of config files and
 * setting of default values.
 *
 * Normally you will subclass KCoreConfigSkeleton, add data members for the preferences
 * settings and register the members in the constructor of the subclass.
 *
 * Example:
 * \code
 * class MyPrefs : public KCoreConfigSkeleton
 * {
 *   public:
 *     MyPrefs()
 *     {
 *       setCurrentGroup("MyGroup");
 *       addItemBool("MySetting1", mMyBool, false);
 *       addItemPoint("MySetting2", mMyPoint, QPoint(100, 200));
 *
 *       setCurrentGroup("MyOtherGroup");
 *       addItemDouble("MySetting3", mMyDouble, 3.14);
 *     }
 *
 *     bool mMyBool;
 *     QPoint mMyPoint;
 *     double mMyDouble;
 * }
 * \endcode
 *
 * It might be convenient in many cases to make this subclass of KCoreConfigSkeleton a
 * singleton for global access from all over the application without passing
 * references to the KCoreConfigSkeleton object around.
 *
 * You can write the data to the configuration file by calling save()
 * and read the data from the configuration file by calling readConfig().
 * If you want to watch for config changes, use the configChanged() signal.
 *
 * If you have items which are not covered by the existing addItem() functions
 * you can add customized code for reading, writing and default setting by
 * implementing the functions usrUseDefaults(), usrRead() and
 * usrSave().
 *
 * Internally preferences settings are stored in instances of subclasses of
 * KConfigSkeletonItem. You can also add KConfigSkeletonItem subclasses
 * for your own types and call the generic addItem() to register them.
 *
 * In many cases you don't have to write the specific KCoreConfigSkeleton
 * subclasses yourself, but you can use \l{The KDE Configuration Compiler} to automatically
 * generate the C++ code from an XML description of the configuration options.
 *
 * \sa KConfigSkeletonItem
 */
class KCONFIGCORE_EXPORT KCoreConfigSkeleton : public QObject
{
    Q_OBJECT
public:
    /*!
     * \class KCoreConfigSkeleton::ItemString
     * \brief Class for handling a string preferences item.
     * \inmodule KConfigCore
     */
    class KCONFIGCORE_EXPORT ItemString : public KConfigSkeletonGenericItem<QString>
    {
    public:
        /*!
         * The type of string that is held in this item.
         *
         * \value Normal
         *        A normal string
         * \value Password
         *        A password string
         * \value Path
         *        A path to a file or directory
         */
        enum Type {
            Normal,
            Password,
            Path,
        };

        /*!
         * Constructs a string item with the given \a _group, \a _key, the initial value \a reference to hold in the item, the \a defaultValue for the item, and the \a type of string held by the item.
         */
        ItemString(const QString &_group,
                   const QString &_key,
                   QString &reference,
                   const QString &defaultValue = QLatin1String(""), // NOT QString() !!
                   Type type = Normal);

        void writeConfig(KConfig *config) override;

        void readConfig(KConfig *config) override;

        void setProperty(const QVariant &p) override;

        bool isEqual(const QVariant &p) const override;

        QVariant property() const override;

    private:
        Type mType;
    };

    /*!
     * \class KCoreConfigSkeleton::ItemPassword
     * \inmodule KConfigCore
     *
     * \brief Class for handling a password preferences item.
     */
    class KCONFIGCORE_EXPORT ItemPassword : public ItemString
    {
    public:
        /*!
         * Constructs a password item with the given \a _group, \a _key, the initial value \a reference to hold in the item, and the \a defaultValue for the item.
         */
        ItemPassword(const QString &_group, const QString &_key, QString &reference,
                     const QString &defaultValue = QLatin1String("")); // NOT QString() !!
    };

    /*!
     * \class KCoreConfigSkeleton::ItemPath
     * \inmodule KConfigCore
     * \brief Class for handling a path preferences item.
     */
    class KCONFIGCORE_EXPORT ItemPath : public ItemString
    {
    public:
        /*!
         * Constructs a path item with the given \a _group, \a _key, the initial value \a reference to hold in the item, and the \a defaultValue for the item.
         */
        ItemPath(const QString &_group, const QString &_key, QString &reference, const QString &defaultValue = QString());
    };

    /*!
     * \class KCoreConfigSkeleton::ItemUrl
     * \inmodule KConfigCore
     * \brief Class for handling a url preferences item.
     */
    class KCONFIGCORE_EXPORT ItemUrl : public KConfigSkeletonGenericItem<QUrl>
    {
    public:
        /*!
         * Constructs an url item with the given \a _group, \a _key, the initial value \a reference to hold in the item, and the \a defaultValue for the item.
         */
        ItemUrl(const QString &_group, const QString &_key, QUrl &reference, const QUrl &defaultValue = QUrl());

        void writeConfig(KConfig *config) override;

        void readConfig(KConfig *config) override;

        void setProperty(const QVariant &p) override;

        bool isEqual(const QVariant &p) const override;

        QVariant property() const override;
    };

    /*!
     * \class KCoreConfigSkeleton::ItemProperty
     * \inmodule KConfigCore
     * \brief Class for handling a QVariant preferences item.
     */
    class KCONFIGCORE_EXPORT ItemProperty : public KConfigSkeletonGenericItem<QVariant>
    {
    public:
        /*!
         * Constructs a property item with the given \a _group, \a _key, the initial value \a reference to hold in the item, and the \a defaultValue for the item.
         */
        ItemProperty(const QString &_group, const QString &_key, QVariant &reference, const QVariant &defaultValue = QVariant());

        void readConfig(KConfig *config) override;
        void setProperty(const QVariant &p) override;

        bool isEqual(const QVariant &p) const override;

        QVariant property() const override;
    };

    /*!
     * \class KCoreConfigSkeleton::ItemBool
     * \inmodule KConfigCore
     * \brief Class for handling a bool preferences item.
     */
    class KCONFIGCORE_EXPORT ItemBool : public KConfigSkeletonGenericItem<bool>
    {
    public:
        /*!
         * Constructs a bool item with the given \a _group, \a _key, the initial value \a reference to hold in the item, and the \a defaultValue for the item.
         */
        ItemBool(const QString &_group, const QString &_key, bool &reference, bool defaultValue = true);

        void readConfig(KConfig *config) override;

        void setProperty(const QVariant &p) override;

        bool isEqual(const QVariant &p) const override;

        QVariant property() const override;
    };

    /*!
     * \class KCoreConfigSkeleton::ItemInt
     * \inmodule KConfigCore
     * \brief Class for handling a 32-bit integer preferences item.
     */
    class KCONFIGCORE_EXPORT ItemInt : public KConfigSkeletonGenericItem<qint32>
    {
    public:
        /*!
         * Constructs an int item with the given \a _group, \a _key, the initial value \a reference to hold in the item, and the \a defaultValue for the item.
         */
        ItemInt(const QString &_group, const QString &_key, qint32 &reference, qint32 defaultValue = 0);

        void readConfig(KConfig *config) override;

        void setProperty(const QVariant &p) override;

        bool isEqual(const QVariant &p) const override;

        QVariant property() const override;

        QVariant minValue() const override;

        QVariant maxValue() const override;

        /*!
         * Sets the minimum value for the item.
         * \sa minValue()
         */
        void setMinValue(qint32);

        /*!
         * Sets the maximum value for the item.
         * \sa maxValue
         */
        void setMaxValue(qint32);

    private:
        bool mHasMin : 1;
        bool mHasMax : 1;
        qint32 mMin;
        qint32 mMax;
    };

    /*!
     * \class KCoreConfigSkeleton::ItemLongLong
     * \inmodule KConfigCore
     * \brief Class for handling a 64-bit integer preferences item.
     */
    class KCONFIGCORE_EXPORT ItemLongLong : public KConfigSkeletonGenericItem<qint64>
    {
    public:
        /*!
         * Constructs a long long item with the given \a _group, \a _key, the initial value \a reference to hold in the item, and the \a defaultValue for the item.
         */
        ItemLongLong(const QString &_group, const QString &_key, qint64 &reference, qint64 defaultValue = 0);

        void readConfig(KConfig *config) override;

        void setProperty(const QVariant &p) override;

        bool isEqual(const QVariant &p) const override;

        QVariant property() const override;

        QVariant minValue() const override;

        QVariant maxValue() const override;

        /*!
         * Sets the minimum value for the item.
         * \sa minValue()
         */
        void setMinValue(qint64);

        /*!
         * Sets the maximum value for the item.
         * \sa maxValue()
         */
        void setMaxValue(qint64);

    private:
        bool mHasMin : 1;
        bool mHasMax : 1;
        qint64 mMin;
        qint64 mMax;
    };

    /*!
     * \class KCoreConfigSkeleton::ItemEnum
     * \inmodule KConfigCore
     * \brief Class for handling enums.
     *
     * Enums specified with KConfigXT should be mapped to enums in the C++ codebase, as the generated getter for the ItemEnum will return an int.
     * \sa {https://develop.kde.org/docs/features/configuration/kconfig_xt/} {Using KConfig XT}
     * \sa Choice
     */
    class KCONFIGCORE_EXPORT ItemEnum : public ItemInt
    {
    public:
        /*!
         * \struct KCoreConfigSkeleton::ItemEnum::Choice
         * \inmodule KConfigCore
         * \brief Provides strongly typed enum options for use with KConfigXT.
         *
         * Enums specified with KConfigXT should be mapped to enums in the C++ codebase, as the generated getter for the ItemEnum will return an int.
         * \sa {https://develop.kde.org/docs/features/configuration/kconfig_xt/} {Using KConfig XT}
         * \sa ItemEnum
         */
        struct Choice {
            /*!
             * \variable KCoreConfigSkeleton::ItemEnum::Choice::name
             * The choice name. This should match the key name in the C++ codebase.
             *
             * This will be used as the value of the enum key if Choice::value is not specified.
             */
            QString name;

            /*!
             * \variable KCoreConfigSkeleton::ItemEnum::Choice::label
             * The short description for the entry.
             *
             * This tag will be ignored unless you set \c SetUserTexts=true.
             */
            QString label;

            /*!
             * \variable KCoreConfigSkeleton::ItemEnum::Choice::toolTip
             * A more verbose description for the entry.
             *
             * This tag will be ignored unless you set \c SetUserTexts=true.
             */
            QString toolTip;

            /*!
             * \variable KCoreConfigSkeleton::ItemEnum::Choice::whatsThis
             * The WhatsThis description for this entry.
             */
            QString whatsThis;

            /*!
             * \variable KCoreConfigSkeleton::ItemEnum::Choice::value
             * The value for the entry key as displayed in the configuration file.
             */
            QString value;
        };

        /*!
         * Constructs an enum item with the given \a _group, \a _key, the initial value \a reference to hold in the item, the list of enum \a choices, and the \a defaultValue for the item.
         */
        ItemEnum(const QString &_group, const QString &_key, qint32 &reference, const QList<Choice> &choices, qint32 defaultValue = 0);

        /*!
         * Specifies a list of enum choices.
         *
         * The name given to the choices list should match an equivalent enum in your C++ codebase, including the namespace. For example:
         * \code
         * <choices name="KWin::Layer">
         * \endcode
         *
         * Each individual choice in the XML file should then be mapped to the same C++ code enum values in the same order.
         */
        QList<Choice> choices() const;

        void readConfig(KConfig *config) override;

        void writeConfig(KConfig *config) override;

        /*!
         * Returns the value for the choice with the given \a name.
         */
        QString valueForChoice(const QString &name) const;

        /*!
         * Stores a choice value \a valueForChoice for an enum entry with the given \a name.
         */
        void setValueForChoice(const QString &name, const QString &valueForChoice);

    private:
        QList<Choice> mChoices;
    };

    /*!
     * \class KCoreConfigSkeleton::ItemUInt
     * \inmodule KConfigCore
     * \brief Class for handling an unsigned 32-bit integer preferences item.
     */
    class KCONFIGCORE_EXPORT ItemUInt : public KConfigSkeletonGenericItem<quint32>
    {
    public:
        /*!
         * Constructs an uint item with the given \a _group, \a _key, the initial value \a reference to hold in the item, and the \a defaultValue for the item.
         */
        ItemUInt(const QString &_group, const QString &_key, quint32 &reference, quint32 defaultValue = 0);

        void readConfig(KConfig *config) override;

        void setProperty(const QVariant &p) override;

        bool isEqual(const QVariant &p) const override;

        QVariant property() const override;

        QVariant minValue() const override;

        QVariant maxValue() const override;

        /*!
         * Sets the minimum value for the item.
         * \sa minValue()
         */
        void setMinValue(quint32);

        /*!
         * Sets the maximum value for the item.
         * \sa maxValue()
         */
        void setMaxValue(quint32);

    private:
        bool mHasMin : 1;
        bool mHasMax : 1;
        quint32 mMin;
        quint32 mMax;
    };

    /*!
     * \class KCoreConfigSkeleton::ItemULongLong
     * \inmodule KConfigCore
     * \brief Class for handling unsigned 64-bit integer preferences item.
     */
    class KCONFIGCORE_EXPORT ItemULongLong : public KConfigSkeletonGenericItem<quint64>
    {
    public:
        /*!
         * Constructs an unsigned long long item with the given \a _group, \a _key, the initial value \a reference to hold in the item, and the \a defaultValue for the item.
         */
        ItemULongLong(const QString &_group, const QString &_key, quint64 &reference, quint64 defaultValue = 0);

        void readConfig(KConfig *config) override;

        void setProperty(const QVariant &p) override;

        bool isEqual(const QVariant &p) const override;

        QVariant property() const override;

        QVariant minValue() const override;

        QVariant maxValue() const override;

        /*!
         * Sets the minimum value for the item.
         * \sa minValue()
         */
        void setMinValue(quint64);

        /*!
         * Sets the maximum value for the item.
         * \sa maxValue()
         */
        void setMaxValue(quint64);

    private:
        bool mHasMin : 1;
        bool mHasMax : 1;
        quint64 mMin;
        quint64 mMax;
    };

    /*!
     * \class KCoreConfigSkeleton::ItemDouble
     * \inmodule KConfigCore
     * \brief Class for handling a floating point preference item.
     */
    class KCONFIGCORE_EXPORT ItemDouble : public KConfigSkeletonGenericItem<double>
    {
    public:
        /*!
         * Constructs a double item with the given \a _group, \a _key, the initial value \a reference to hold in the item, and the \a defaultValue for the item.
         */
        ItemDouble(const QString &_group, const QString &_key, double &reference, double defaultValue = 0);

        void readConfig(KConfig *config) override;

        void setProperty(const QVariant &p) override;

        bool isEqual(const QVariant &p) const override;

        QVariant property() const override;

        QVariant minValue() const override;

        QVariant maxValue() const override;

        /*!
         * Sets the minimum value for the item.
         * \sa minValue()
         */
        void setMinValue(double);

        /*!
         * Sets the maximum value for the item.
         * \sa maxValue()
         */
        void setMaxValue(double);

    private:
        bool mHasMin : 1;
        bool mHasMax : 1;
        double mMin;
        double mMax;
    };

    /*!
     * \class KCoreConfigSkeleton::ItemRect
     * \inmodule KConfigCore
     * \brief Class for handling a QRect preferences item.
     */
    class KCONFIGCORE_EXPORT ItemRect : public KConfigSkeletonGenericItem<QRect>
    {
    public:
        /*!
         * Constructs a QRect item with the given \a _group, \a _key, the initial value \a reference to hold in the item, and the \a defaultValue for the item.
         */
        ItemRect(const QString &_group, const QString &_key, QRect &reference, const QRect &defaultValue = QRect());

        void readConfig(KConfig *config) override;

        void setProperty(const QVariant &p) override;

        bool isEqual(const QVariant &p) const override;

        QVariant property() const override;
    };

    /*!
     * \class KCoreConfigSkeleton::ItemRectF
     * \inmodule KConfigCore
     * \brief Class for handling a QRectF preferences item.
     */
    class KCONFIGCORE_EXPORT ItemRectF : public KConfigSkeletonGenericItem<QRectF>
    {
    public:
        /*!
         * Constructs a QRectF item with the given \a _group, \a _key, the initial value \a reference to hold in the item, and the \a defaultValue for the item.
         */
        ItemRectF(const QString &_group, const QString &_key, QRectF &reference, const QRectF &defaultValue = QRectF());

        void readConfig(KConfig *config) override;

        void setProperty(const QVariant &p) override;

        bool isEqual(const QVariant &p) const override;

        QVariant property() const override;
    };

    /*!
     * \class KCoreConfigSkeleton::ItemPoint
     * \inmodule KConfigCore
     * \brief Class for handling a QPoint preferences item.
     */
    class KCONFIGCORE_EXPORT ItemPoint : public KConfigSkeletonGenericItem<QPoint>
    {
    public:
        /*!
         * Constructs a QPoint item with the given \a _group, \a _key, the initial value \a reference to hold in the item, and the \a defaultValue for the item.
         */
        ItemPoint(const QString &_group, const QString &_key, QPoint &reference, const QPoint &defaultValue = QPoint());

        void readConfig(KConfig *config) override;

        void setProperty(const QVariant &p) override;

        bool isEqual(const QVariant &p) const override;

        QVariant property() const override;
    };

    /*!
     * \class KCoreConfigSkeleton::ItemPointF
     * \inmodule KConfigCore
     * \brief Class for handling a QPointF preferences item.
     */
    class KCONFIGCORE_EXPORT ItemPointF : public KConfigSkeletonGenericItem<QPointF>
    {
    public:
        /*!
         * Constructs a QPointF item with the given \a _group, \a _key, the initial value \a reference to hold in the item, and the \a defaultValue for the item.
         */
        ItemPointF(const QString &_group, const QString &_key, QPointF &reference, const QPointF &defaultValue = QPointF());

        void readConfig(KConfig *config) override;

        void setProperty(const QVariant &p) override;

        bool isEqual(const QVariant &p) const override;

        QVariant property() const override;
    };

    /*!
     * \class KCoreConfigSkeleton::ItemSize
     * \inmodule KConfigCore
     * \brief Class for handling a QSize preferences item.
     */
    class KCONFIGCORE_EXPORT ItemSize : public KConfigSkeletonGenericItem<QSize>
    {
    public:
        /*!
         * Constructs a QSize item with the given \a _group, \a _key, the initial value \a reference to hold in the item, and the \a defaultValue for the item.
         */
        ItemSize(const QString &_group, const QString &_key, QSize &reference, const QSize &defaultValue = QSize());

        void readConfig(KConfig *config) override;

        void setProperty(const QVariant &p) override;

        bool isEqual(const QVariant &p) const override;

        QVariant property() const override;
    };

    /*!
     * \class KCoreConfigSkeleton::ItemSizeF
     * \inmodule KConfigCore
     * \brief Class for handling a QSizeF preferences item.
     */
    class KCONFIGCORE_EXPORT ItemSizeF : public KConfigSkeletonGenericItem<QSizeF>
    {
    public:
        /*!
         * Constructs a QSizeF item with the given \a _group, \a _key, the initial value \a reference to hold in the item, and the \a defaultValue for the item.
         */
        ItemSizeF(const QString &_group, const QString &_key, QSizeF &reference, const QSizeF &defaultValue = QSizeF());

        void readConfig(KConfig *config) override;

        void setProperty(const QVariant &p) override;

        bool isEqual(const QVariant &p) const override;

        QVariant property() const override;
    };

    /*!
     * \class KCoreConfigSkeleton::ItemDateTime
     * \inmodule KConfigCore
     * \brief Class for handling a QDateTime preferences item.
     */
    class KCONFIGCORE_EXPORT ItemDateTime : public KConfigSkeletonGenericItem<QDateTime>
    {
    public:
        /*!
         * Constructs a QDateTime item with the given \a _group, \a _key, the initial value \a reference to hold in the item, and the \a defaultValue for the item.
         */
        ItemDateTime(const QString &_group, const QString &_key, QDateTime &reference, const QDateTime &defaultValue = QDateTime());

        void readConfig(KConfig *config) override;

        void setProperty(const QVariant &p) override;

        bool isEqual(const QVariant &p) const override;

        QVariant property() const override;
    };

    /*!
     * \class KCoreConfigSkeleton::ItemTime
     * \inmodule KConfigCore
     * \since 6.16
     * \brief Class for handling a QTime preferences item.
     */
    class KCONFIGCORE_EXPORT ItemTime : public KConfigSkeletonGenericItem<QTime>
    {
    public:
        /*!
         * Constructs a QTime item with the given \a _group, \a _key, the initial value \a reference to hold in the item, and the \a defaultValue for the item.
         */
        ItemTime(const QString &_group, const QString &_key, QTime &reference, QTime defaultValue = QTime());

        void readConfig(KConfig *config) override;

        void setProperty(const QVariant &p) override;

        bool isEqual(const QVariant &p) const override;

        QVariant property() const override;
    };

    /*!
     * \class KCoreConfigSkeleton::ItemStringList
     * \inmodule KConfigCore
     * \brief Class for handling a string list preferences item.
     */
    class KCONFIGCORE_EXPORT ItemStringList : public KConfigSkeletonGenericItem<QStringList>
    {
    public:
        /*!
         * Constructs a string list item with the given \a _group, \a _key, the initial value \a reference to hold in the item, and the \a defaultValue for the item.
         */
        ItemStringList(const QString &_group, const QString &_key, QStringList &reference, const QStringList &defaultValue = QStringList());

        void readConfig(KConfig *config) override;

        void setProperty(const QVariant &p) override;

        bool isEqual(const QVariant &p) const override;

        QVariant property() const override;
    };

    /*!
     * \class KCoreConfigSkeleton::ItemPathList
     * \inmodule KConfigCore
     * \brief Class for handling a path list preferences item.
     */
    class KCONFIGCORE_EXPORT ItemPathList : public ItemStringList
    {
    public:
        /*!
         * Constructs a path string list item with the given \a _group, \a _key, the initial value \a reference to hold in the item, and the \a defaultValue for the item.
         */
        ItemPathList(const QString &_group, const QString &_key, QStringList &reference, const QStringList &defaultValue = QStringList());

        void readConfig(KConfig *config) override;
        void writeConfig(KConfig *config) override;
    };

    /*!
     * \class KCoreConfigSkeleton::ItemUrlList
     * \inmodule KConfigCore
     * \brief Class for handling a url list preferences item.
     */
    class KCONFIGCORE_EXPORT ItemUrlList : public KConfigSkeletonGenericItem<QList<QUrl>>
    {
    public:
        /*!
         * Constructs a url list item with the given \a _group, \a _key, the initial value \a reference to hold in the item, and the \a defaultValue for the item.
         */
        ItemUrlList(const QString &_group, const QString &_key, QList<QUrl> &reference, const QList<QUrl> &defaultValue = QList<QUrl>());

        void readConfig(KConfig *config) override;

        void writeConfig(KConfig *config) override;

        void setProperty(const QVariant &p) override;

        bool isEqual(const QVariant &p) const override;

        QVariant property() const override;
    };

    /*!
     * \class KCoreConfigSkeleton::ItemIntList
     * \inmodule KConfigCore
     * \brief Class for handling an integer list preferences item.
     */
    class KCONFIGCORE_EXPORT ItemIntList : public KConfigSkeletonGenericItem<QList<int>>
    {
    public:
        /*!
         * Constructs an int list item with the given \a _group, \a _key, the initial value \a reference to hold in the item, and the \a defaultValue for the item.
         */
        ItemIntList(const QString &_group, const QString &_key, QList<int> &reference, const QList<int> &defaultValue = QList<int>());

        void readConfig(KConfig *config) override;

        void setProperty(const QVariant &p) override;

        bool isEqual(const QVariant &p) const override;

        QVariant property() const override;
    };

public:
    /*!
     * Constructs a KCoreConfigSkeleton object with the given \a configname and \a parent.
     *
     * If no name is given, the default config file as returned by KSharedConfig::openConfig() is used.
     */
    explicit KCoreConfigSkeleton(const QString &configname = QString(), QObject *parent = nullptr);

    /*!
     * Constructs a KCoreConfigSkeleton object with the given KSharedConfig \a config and \a parent.
     */
    explicit KCoreConfigSkeleton(KSharedConfig::Ptr config, QObject *parent = nullptr);

    // TODO KF7: add a KCoreConfigSkeleton(QObject *parent = nullptr) constructor to allow users to opt out the ambiguity
    /*!
     * It is added to disambiguate with the other constructor otherwise ambiguate in case of default nullptr
     *   KCoreConfigSkeleton(KSharedConfig::Ptr(nullptr)) or KCoreConfigSkeleton(std::unique_ptr<KConfig> (nullptr)) with parent = nullptr.
     * \value IsStdUniqPtr
     */
    enum DisambiguateConstructor {
        IsStdUniqPtr = 0,
    };
    Q_ENUM(DisambiguateConstructor)

    /*!
     * Constructs a KCoreConfigSkeleton object with the given KConfig \a config and \a parent.
     *
     * The \a value must be DisambiguateConstructor::IsStdUniqPtr.
     * \since 6.23
     */
    explicit KCoreConfigSkeleton(std::unique_ptr<KConfig> config, DisambiguateConstructor value, QObject *parent = nullptr);

    ~KCoreConfigSkeleton() override;

    /*!
     * Sets all registered items to their default values.
     *
     * This method calls usrSetDefaults() after setting the defaults for the
     * registered items. You can override usrSetDefaults() in derived classes
     * if you have special requirements.
     *
     * If you need more fine-grained control of setting the default values of
     * the registered items you can override setDefaults() in a derived class.
     */
    virtual void setDefaults();

    /*!
     * Reads preferences from the config file. All registered items are set to the
     * values read from disk.
     *
     * This method calls usrRead() after reading the settings of the
     * registered items from the KConfig. You can override usrRead()
     * in derived classes if you have special requirements.
     */
    void load();

    /*!
     * Reads preferences from the KConfig object.
     * This method assumes that the KConfig object was previously loaded,
     * i.e. it uses the in-memory values from KConfig without reloading from disk.
     *
     * This method calls usrRead() after reading the settings of the
     * registered items from the KConfig. You can override usrRead()
     * in derived classes if you have special requirements.
     * \since 5.0
     */
    void read();

    /*!
     * Returns if all the registered items are set to their default value.
     * \since 5.64
     */
    bool isDefaults() const;

    /*!
     * Returns if any registered item has a different value than the
     * previously loaded value.
     * \since 5.64
     */
    bool isSaveNeeded() const;

    /*!
     * Set the config file \a group for subsequent addItem() calls.
     *
     * It is valid until setCurrentGroup() is called with a new argument.
     * Call this before you add any items.
     *
     * The default value is "No Group".
     */
    void setCurrentGroup(const QString &group);

    /*!
     * Returns the current group used for addItem() calls.
     */
    QString currentGroup() const;

    /*!
     * Registers a custom KConfigSkeletonItem \a item with a given \a name.
     *
     * If \a name is a null string, take the name from KConfigSkeletonItem::key().
     *
     * \note All names must be unique but multiple entries can have
     * the same key if they reside in different groups.
     *
     * KCoreConfigSkeleton takes ownership of \a item.
     */
    void addItem(KConfigSkeletonItem *item, const QString &name = QString());

    /*!
     * Registers a string item with a unique \a name by passing a
     * \a reference pointer to the variable and the given \a defaultValue for
     * the \a key used in the config file.
     *
     * If \a key is a null string, \a name is used as key.
     *
     * Returns the created item.
     */
    ItemString *addItemString(const QString &name,
                              QString &reference,
                              const QString &defaultValue = QLatin1String(""), // NOT QString() !!
                              const QString &key = QString());

    /*!
     * Registers a password item of type QString with a unique \a name by passing a
     * \a reference pointer to the variable and the given \a defaultValue for
     * the \a key used in the config file.
     *
     * If \a key is a null string, \a name is used as key.
     *
     * The string value is written encrypted to the config file.
     *
     * Returns the created item.
     *
     * \note The current encryption scheme is very weak.
     */
    ItemPassword *addItemPassword(const QString &name, QString &reference, const QString &defaultValue = QLatin1String(""), const QString &key = QString());

    /*!
     * Registers a path item of type QString with a unique \a name by passing a
     * \a reference pointer to the variable and the given \a defaultValue for
     * the \a key used in the config file.
     *
     * If \a key is a null string, \a name is used as key.
     *
     * The string value is interpreted as a path.
     * This means dollar expansion is activated for this value, so
     * that for example \c $HOME gets expanded.
     *
     * Returns the created item.
     */
    ItemPath *addItemPath(const QString &name, QString &reference, const QString &defaultValue = QLatin1String(""), const QString &key = QString());

    /*!
     * Registers a property item of type QVariant with a unique \a name by passing a
     * \a reference pointer to the variable and the given \a defaultValue for
     * the \a key used in the config file.
     *
     * If \a key is a null string, \a name is used as key.
     *
     * Returns the created item.
     *
     * \note The following QVariant types are allowed:
     * String, StringList, Font, Point, PointF, Rect, RectF, Size, SizeF,
     * Color, Int, UInt, Bool, Double, DateTime and Date.
     */
    ItemProperty *addItemProperty(const QString &name, QVariant &reference, const QVariant &defaultValue = QVariant(), const QString &key = QString());
    /*!
     * Registers a bool item with a unique \a name by passing a
     * \a reference pointer to the variable and the given \a defaultValue for
     * the \a key used in the config file.
     *
     * If \a key is a null string, \a name is used as key.
     *
     * Returns the created item.
     */
    ItemBool *addItemBool(const QString &name, bool &reference, bool defaultValue = false, const QString &key = QString());

    /*!
     * Registers a qint32 item with a unique \a name by passing a
     * \a reference pointer to the variable and the given \a defaultValue for
     * the \a key used in the config file.
     *
     * If \a key is a null string, \a name is used as key.
     *
     * Returns the created item.
     */
    ItemInt *addItemInt(const QString &name, qint32 &reference, qint32 defaultValue = 0, const QString &key = QString());

    /*!
     * Registers a quint32 item with a unique \a name by passing a
     * \a reference pointer to the variable and the given \a defaultValue for
     * the \a key used in the config file.
     *
     * If \a key is a null string, \a name is used as key.
     *
     * Returns the created item.
     */
    ItemUInt *addItemUInt(const QString &name, quint32 &reference, quint32 defaultValue = 0, const QString &key = QString());

    /*!
     * Registers a qint64 item with a unique \a name by passing a
     * \a reference pointer to the variable and the given \a defaultValue for
     * the \a key used in the config file.
     *
     * If \a key is a null string, \a name is used as key.
     *
     * Returns the created item.
     */
    ItemLongLong *addItemLongLong(const QString &name, qint64 &reference, qint64 defaultValue = 0, const QString &key = QString());

    /*!
     * Registers a quint64 item with a unique \a name by passing a
     * \a reference pointer to the variable and the given \a defaultValue for
     * the \a key used in the config file.
     *
     * If \a key is a null string, \a name is used as key.
     *
     * Returns the created item.
     */
    ItemULongLong *addItemULongLong(const QString &name, quint64 &reference, quint64 defaultValue = 0, const QString &key = QString());

    /*!
     * Registers a double item with a unique \a name by passing a
     * \a reference pointer to the variable and the given \a defaultValue for
     * the \a key used in the config file.
     *
     * If \a key is a null string, \a name is used as key.
     *
     * Returns the created item.
     */
    ItemDouble *addItemDouble(const QString &name, double &reference, double defaultValue = 0.0, const QString &key = QString());

    /*!
     * Registers a QRect item with a unique \a name by passing a
     * \a reference pointer to the variable and the given \a defaultValue for
     * the \a key used in the config file.
     *
     * If \a key is a null string, \a name is used as key.
     *
     * Returns the created item.
     */
    ItemRect *addItemRect(const QString &name, QRect &reference, const QRect &defaultValue = QRect(), const QString &key = QString());

    /*!
     * Registers a QRectF item with a unique \a name by passing a
     * \a reference pointer to the variable and the given \a defaultValue for
     * the \a key used in the config file.
     *
     * If \a key is a null string, \a name is used as key.
     *
     * Returns the created item.
     */
    ItemRectF *addItemRectF(const QString &name, QRectF &reference, const QRectF &defaultValue = QRectF(), const QString &key = QString());

    /*!
     * Registers a QPoint item with a unique \a name by passing a
     * \a reference pointer to the variable and the given \a defaultValue for
     * the \a key used in the config file.
     *
     * If \a key is a null string, \a name is used as key.
     *
     * Returns the created item.
     */
    ItemPoint *addItemPoint(const QString &name, QPoint &reference, const QPoint &defaultValue = QPoint(), const QString &key = QString());

    /*!
     * Registers a QPointF item with a unique \a name by passing a
     * \a reference pointer to the variable and the given \a defaultValue for
     * the \a key used in the config file.
     *
     * If \a key is a null string, \a name is used as key.
     *
     * Returns the created item.
     */
    ItemPointF *addItemPointF(const QString &name, QPointF &reference, const QPointF &defaultValue = QPointF(), const QString &key = QString());

    /*!
     * Registers a QSize item with a unique \a name by passing a
     * \a reference pointer to the variable and the given \a defaultValue for
     * the \a key used in the config file.
     *
     * If \a key is a null string, \a name is used as key.
     *
     * Returns the created item.
     */
    ItemSize *addItemSize(const QString &name, QSize &reference, const QSize &defaultValue = QSize(), const QString &key = QString());

    /*!
     * Registers a QSizeF item with a unique \a name by passing a
     * \a reference pointer to the variable and the given \a defaultValue for
     * the \a key used in the config file.
     *
     * If \a key is a null string, \a name is used as key.
     *
     * Returns the created item.
     */
    ItemSizeF *addItemSizeF(const QString &name, QSizeF &reference, const QSizeF &defaultValue = QSizeF(), const QString &key = QString());

    /*!
     * Registers a QDateTime item with a unique \a name by passing a
     * \a reference pointer to the variable and the given \a defaultValue for
     * the \a key used in the config file.
     *
     * If \a key is a null string, \a name is used as key.
     *
     * Returns the created item.
     */
    ItemDateTime *addItemDateTime(const QString &name, QDateTime &reference, const QDateTime &defaultValue = QDateTime(), const QString &key = QString());

    /*!
     * Registers a QTime item with a unique \a name by passing a
     * \a reference pointer to the variable and the given \a defaultValue for
     * the \a key used in the config file.
     *
     * If \a key is a null string, \a name is used as key.
     *
     * Returns the created item.
     * \since 6.16
     */
    ItemTime *addItemTime(const QString &name, QTime &reference, QTime defaultValue = QTime(), const QString &key = QString());

    /*!
     * Registers a string list item with a unique \a name by passing a
     * \a reference pointer to the variable and the given \a defaultValue for
     * the \a key used in the config file.
     *
     * If \a key is a null string, \a name is used as key.
     *
     * Returns the created item.
     */
    ItemStringList *
    addItemStringList(const QString &name, QStringList &reference, const QStringList &defaultValue = QStringList(), const QString &key = QString());

    /*!
     * Registers an int list item with a unique \a name by passing a
     * \a reference pointer to the variable and the given \a defaultValue for
     * the \a key used in the config file.
     *
     * If \a key is a null string, \a name is used as key.
     *
     * Returns the created item.
     */
    ItemIntList *addItemIntList(const QString &name, QList<int> &reference, const QList<int> &defaultValue = QList<int>(), const QString &key = QString());

    /*!
     * Returns the KConfig object used for reading and writing the settings.
     */
    KConfig *config();

    /*!
     * Returns the const KConfig object used for reading and writing the settings.
     */
    const KConfig *config() const;

    /*!
     * Returns the KSharedConfig object used for reading and writing the settings.
     *
     * This might return null if the internal config is a std::unique_ptr<KConfig>.
     * \sa config()
     * \sa KCoreConfigSkeleton::DisambiguateConstructor
     * \since 5.0
     */
    KSharedConfig::Ptr sharedConfig() const;

    /*!
     * Sets the KSharedConfig \a pConfig object used for reading and writing the settings.
     */
    void setSharedConfig(KSharedConfig::Ptr pConfig);

    /*!
     * Sets the KConfig \a config object used for reading and writing the settings.
     * \since 6.23
     */
    void setConfig(std::unique_ptr<KConfig> config);

    /*!
     * Returns the list of items managed by this KCoreConfigSkeleton object.
     */
    KConfigSkeletonItem::List items() const;

    /*!
     * Removes and deletes an item by \a name.
     */
    void removeItem(const QString &name);

    /*!
     * Removes and deletes all items.
     */
    void clearItems();

    /*!
     * Returns whether the item with the given \a name is immutable.
     * \since 4.4
     */
    Q_INVOKABLE bool isImmutable(const QString &name) const;

    /*!
     * Looks up an item by \a name.
     * \since 4.4
     */
    KConfigSkeletonItem *findItem(const QString &name) const;

    /*!
     * Changes the state of this object to reflect the default values (\a b = true)
     * or replaces the default values with the current values (\a b = false).
     *
     * This method is implemented by usrUseDefaults(), which can be overridden
     * in derived classes if you have special requirements and can call
     * usrUseDefaults() directly.
     *
     * If you don't have control whether useDefaults() or usrUseDefaults() is
     * called override useDefaults() directly.
     *
     * Returns the state prior to this call.
     */
    virtual bool useDefaults(bool b);

public Q_SLOTS:
    /*!
     * Writes preferences to the config file. The values of all registered items are
     * written to disk and returns true upon success.
     *
     * This method calls usrSave() after writing the settings from the
     * registered items to the KConfig.
     *
     * You can override usrSave() in derived classes if you have special requirements.
     */
    bool save();

Q_SIGNALS:
    /*!
     * This signal is emitted when the configuration changes.
     * \sa KConfigBase::WriteConfigFlags
     */
    void configChanged();

protected:
    /*!
     * Changes the state of this object to reflect the default values (\a b = true)
     * or replaces the default values with the current values (\a b = false).
     *
     * Called from useDefaults().
     *
     * Implemented by subclasses that use special defaults.
     *
     * Returns the state prior to this call.
     */
    virtual bool usrUseDefaults(bool b);

    /*!
     * Performs the actual setting of default values.
     *
     * Override in derived classes to set special default values.
     *
     * Called from setDefaults().
     */
    virtual void usrSetDefaults();

    /*!
     * Performs the actual reading of the configuration file.
     *
     * Override in derived classes to read special config values.
     *
     * Called from read().
     */
    virtual void usrRead();

    /*!
     * Performs the actual writing of the configuration file and returns true upon success.
     *
     * Override in derived classes to write special config values.
     *
     * Called from save().
     */
    virtual bool usrSave();

private:
    KCoreConfigSkeletonPrivate *const d;
    friend class KConfigSkeleton;
};

Q_DECLARE_TYPEINFO(KCoreConfigSkeleton::ItemEnum::Choice, Q_RELOCATABLE_TYPE);

#endif

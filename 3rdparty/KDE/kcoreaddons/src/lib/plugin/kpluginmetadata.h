/*
    This file is part of the KDE project

    SPDX-FileCopyrightText: 2014 Alex Richardson <arichardson.kde@gmail.com>
    SPDX-FileCopyrightText: 2021 Alexander Lohnau <alexander.lohnau@gmx.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KPLUGINMETADATA_H
#define KPLUGINMETADATA_H

#include "kcoreaddons_export.h"

#include <QExplicitlySharedDataPointer>
#include <QJsonObject>
#include <QMetaType>
#include <QString>
#include <QStringList>

#include <functional>

class QPluginLoader;
struct QStaticPlugin;
class KPluginMetaDataPrivate;
class KAboutPerson;
/*!
  \class KPluginMetaData
  \inmodule KCoreAddons

  \brief This class allows easily accessing some standardized values from the JSON metadata that
  can be embedded into Qt plugins.

  Additional plugin-specific metadata can be retrieved by
  directly reading from the QJsonObject returned by KPluginMetaData::rawData.

  For embedded metadata, you should not specify an id manually. Instead the id will
  be derived from the file basename.

  The following keys will be read from an object "KPlugin" inside the metadata JSON:

   \table
   \header
       \li Key
       \li Accessor function
       \li JSON Type
    \row
        \li Name
        \li name()
        \li string
    \row
        \li Description
        \li description()
        \li string
    \row
        \li Icon
        \li iconName()
        \li string
    \row
        \li Authors
        \li authors()
        \li object array (KAboutPerson)
    \row
        \li Category
        \li category()
        \li string
    \row
        \li License
        \li license()
        \li string
    \row
        \li Copyright
        \li copyrightText()
        \li string
    \row
        \li Id
        \li pluginId()
        \li string
    \row
        \li Version
        \li version()
        \li string
    \row
        \li Website
        \li website()
        \li string
    \row
        \li BugReportUrl
        \li bugReportUrl()
        \li string
    \row
        \li EnabledByDefault
        \li isEnabledByDefault()
        \li bool
    \row
        \li MimeTypes
        \li mimeTypes()
        \li string array
    \row
        \li FormFactors
        \li formFactors()
        \li string array
    \row
        \li Translators
        \li translators()
        \li object array (KAboutPerson)
    \row
        \li OtherContributors
        \li otherContributors()
        \li object array (KAboutPerson)
   \endtable

  The Authors, Translators and OtherContributors keys are expected to be
  list of objects that match the structure expected by KAboutPerson::fromJSON.

  An example metadata json file could look like this:
  \badcode
  {
    "KPlugin": {
       "Name": "Date and Time",
       "Description": "Date and time by timezone",
       "Icon": "preferences-system-time",
       "Authors": [ { "Name": "Aaron Seigo", "Email": "aseigo@kde.org" } ],
       "Category": "Date and Time",
       "EnabledByDefault": "true",
       "License": "LGPL",
       "Version": "1.0",
       "Website": "https://plasma.kde.org/"
    }
  }
  \endcode

  \sa KAboutPerson::fromJSON()
  \since 5.1
 */
class KCOREADDONS_EXPORT KPluginMetaData
{
    Q_GADGET

    /*!
     * \property KPluginMetaData::isValid
     */
    Q_PROPERTY(bool isValid READ isValid CONSTANT)

    /*!
     * \property KPluginMetaData::isHidden
     */
    Q_PROPERTY(bool isHidden READ isHidden CONSTANT)

    /*!
     * \property KPluginMetaData::fileName
     */
    Q_PROPERTY(QString fileName READ fileName CONSTANT)

    /*!
     * \property KPluginMetaData::rawData
     */
    Q_PROPERTY(QJsonObject rawData READ rawData CONSTANT)

    /*!
     * \property KPluginMetaData::name
     */
    Q_PROPERTY(QString name READ name CONSTANT)

    /*!
     * \property KPluginMetaData::description
     */
    Q_PROPERTY(QString description READ description CONSTANT)

    /*!
     * \property KPluginMetaData::authors
     */
    Q_PROPERTY(QList<KAboutPerson> authors READ authors CONSTANT)

    /*!
     * \property KPluginMetaData::translators
     */
    Q_PROPERTY(QList<KAboutPerson> translators READ translators CONSTANT)

    /*!
     * \property KPluginMetaData::otherContributors
     */
    Q_PROPERTY(QList<KAboutPerson> otherContributors READ otherContributors CONSTANT)

    /*!
     * \property KPluginMetaData::category
     */
    Q_PROPERTY(QString category READ category CONSTANT)

    /*!
     * \property KPluginMetaData::iconName
     */
    Q_PROPERTY(QString iconName READ iconName CONSTANT)

    /*!
     * \property KPluginMetaData::license
     */
    Q_PROPERTY(QString license READ license CONSTANT)

    /*!
     * \property KPluginMetaData::licenseText
     */
    Q_PROPERTY(QString licenseText READ licenseText CONSTANT)

    /*!
     * \property KPluginMetaData::copyrightText
     */
    Q_PROPERTY(QString copyrightText READ copyrightText CONSTANT)

    /*!
     * \property KPluginMetaData::pluginId
     */
    Q_PROPERTY(QString pluginId READ pluginId CONSTANT)

    /*!
     * \property KPluginMetaData::version
     */
    Q_PROPERTY(QString version READ version CONSTANT)

    /*!
     * \property KPluginMetaData::website
     */
    Q_PROPERTY(QString website READ website CONSTANT)

    /*!
     * \property KPluginMetaData::bugReportUrl
     */
    Q_PROPERTY(QString bugReportUrl READ bugReportUrl CONSTANT)

    /*!
     * \property KPluginMetaData::mimeTypes
     */
    Q_PROPERTY(QStringList mimeTypes READ mimeTypes CONSTANT)

    /*!
     * \property KPluginMetaData::formFactors
     */
    Q_PROPERTY(QStringList formFactors READ formFactors CONSTANT)

    /*!
     * \property KPluginMetaData::isEnabledByDefault
     */
    Q_PROPERTY(bool isEnabledByDefault READ isEnabledByDefault CONSTANT)

public:
    /*!
     * Options for creating a KPluginMetaData object.
     * \since 5.91
     *
     * \value AllowEmptyMetaData Plugins with empty metaData are considered valid
     * \value [since 6.0] CacheMetaData If KCoreAddons should keep metadata in cache. This makes querying the namespace again faster. Consider using
     * this if you need revalidation of plugins
     *
     */
    enum KPluginMetaDataOption {
        AllowEmptyMetaData = 1,
        CacheMetaData = 2,
    };
    Q_DECLARE_FLAGS(KPluginMetaDataOptions, KPluginMetaDataOption)
    Q_FLAG(KPluginMetaDataOption)

    /*! Creates an invalid KPluginMetaData instance */
    KPluginMetaData();

    /*!
     * Reads the plugin metadata from a QPluginLoader instance. You must call QPluginLoader::setFileName()
     * or use the appropriate constructor on \a loader before calling this.
     *
     * \a options Whether or not plugins without JSON metadata are considered valid.
     */
    KPluginMetaData(const QPluginLoader &loader, KPluginMetaDataOptions options = {});

    /*!
     * Reads the plugin metadata from a plugin which can be loaded from \a file.
     *
     * Platform-specific library suffixes may be omitted since \a file will be resolved
     * using the same logic as QPluginLoader.
     *
     * \a options Whether or not plugins without JSON metadata are considered valid.
     *
     * \sa QPluginLoader::setFileName()
     */
    KPluginMetaData(const QString &pluginFile, KPluginMetaDataOptions options = {});

    /*!
     * Creates a KPluginMetaData from a QJsonObject holding the metadata and a file name
     * This can be used if the data is not retrieved from a Qt C++ plugin library but from some
     * other source.
     *
     * \a metaData the JSON metadata to use for this object
     *
     * \a pluginFile the file that the plugin can be loaded from
     *
     * \since 6.0
     */
    KPluginMetaData(const QJsonObject &metaData, const QString &fileName);

    KPluginMetaData(const KPluginMetaData &);

    KPluginMetaData &operator=(const KPluginMetaData &);

    ~KPluginMetaData();

    /*!
     * Load a KPluginMetaData instance from a .json file. Unlike the constructor with a single file argument,
     * this ensure that only JSON format plugins are loaded and any other type is rejected.
     *
     * \a jsonFile the .json file to load
     *
     * \since 5.91
     */
    static KPluginMetaData fromJsonFile(const QString &jsonFile);

    /*!
     * \a directory The directory to search for plugins. If a relative path is given for \a directory,
     * all entries of QCoreApplication::libraryPaths() will be checked with \a directory appended as a
     * subdirectory. If an absolute path is given only that directory will be searched.
     *
     * \note Check if the returned KPluginMetaData is valid before continuing to use it.
     *
     * \a pluginId The Id of the plugin. The id should be the same as the filename, see KPluginMetaData::pluginId()
     *
     * \a options Whether or not plugins without JSON metadata are considered valid.
     *
     * \since 5.84
     */
    static KPluginMetaData findPluginById(const QString &directory, const QString &pluginId, KPluginMetaDataOptions options = {});

    /*!
     * Find all plugins inside \a directory that satisfy a filter.
     *
     * \a directory The directory to search for plugins. If a relative path is given for \a directory,
     * all entries of QCoreApplication::libraryPaths() will be checked with \a directory appended as a
     * subdirectory. If an absolute path is given only that directory will be searched.
     *
     * \a filter a callback function that returns \c true if the found plugin should be loaded
     * and \c false if it should be skipped. If this argument is omitted all plugins will be loaded.
     *
     * \a options Whether or not plugins without JSON metadata are considered valid.
     *
     * Returns all plugins found in \a directory that fulfill the constraints of \a filter
     * \since 5.86
     */
    static QList<KPluginMetaData>
    findPlugins(const QString &directory, std::function<bool(const KPluginMetaData &)> filter = {}, KPluginMetaDataOptions options = {});

    /*!
     * Returns whether this object holds valid information about a plugin.
     *
     * If this is \c true pluginId() will return a non-empty string.
     */
    bool isValid() const;

    /*!
     * Returns whether this object should be hidden
     *
     * \since 5.8
     */
    bool isHidden() const;

    /*!
     * Returns the path to the plugin.
     *
     * When the KPluginMetaData(QJsonObject, QString) constructor is used, the string is not modified.
     * Otherwise, the path is resolved using QPluginLoader.
     * For static plugins the fileName is the namespace and pluginId concatenated.
     *
     * \note It is not guaranteed that this is a valid path to a shared library (i.e. loadable
     * by QPluginLoader) since the metadata could also refer to a non-C++ plugin.
     */
    QString fileName() const;

    /*!
     * Returns the full metadata stored inside the plugin file.
     */
    QJsonObject rawData() const;

    /*!
     * Returns the user visible name of the plugin.
     */
    QString name() const;

    /*!
     * Returns a short description of the plugin.
     */
    QString description() const;

    /*!
     * Returns the author(s) of this plugin.
     */
    QList<KAboutPerson> authors() const;

    /*!
     * Returns the translator(s) of this plugin.
     *
     * \since 5.18
     */
    QList<KAboutPerson> translators() const;

    /*!
     * Returns a list of people that contributed to this plugin (other than the authors and translators).
     *
     * \since 5.18
     */
    QList<KAboutPerson> otherContributors() const;

    /*!
     * Returns the categories of this plugin (e.g. "playlist/skin").
     */
    QString category() const;

    /*!
     * Returns the icon name for this plugin
     * \sa QIcon::fromTheme()
     */
    QString iconName() const;

    /*!
     * Returns the short license identifier (e.g. LGPL).
     *
     * See KAboutLicense::byKeyword() for retrieving the full license information
     */
    QString license() const;

    /*!
     * Returns the text of the license, equivalent to KAboutLicense::byKeyword(license()).text()
     *
     * \since 5.73
     */
    QString licenseText() const;

    /*!
     * Returns a short copyright statement
     *
     * \since 5.18
     */
    QString copyrightText() const;

    /*!
     * Returns the unique identifier within the namespace of the plugin
     *
     * For C++ plugins, this ID is derived from the filename.
     * It should not be set in the metadata explicitly.
     *
     * When using KPluginMetaData::fromJsonFile or KPluginMetaData(QJsonObject, QString),
     * the "Id" of the "KPlugin" object will be used. If unset, it will be derived
     * from the filename.
     */
    QString pluginId() const;

    /*!
     * Returns the version of the plugin.
     */
    QString version() const;

    /*!
     * Returns the website of the plugin.
     */
    QString website() const;

    /*!
     * Returns the website where people can report a bug found in this plugin
     * \since 5.99
     */
    QString bugReportUrl() const;

    /*!
     * Returns a list of MIME types this plugin can handle (e.g. "application/pdf", "image/png", etc.)
     * \since 5.16
     */
    QStringList mimeTypes() const;

    /*!
     * Returns true if this plugin can handle the given mimetype
     * This is more accurate than mimeTypes().contains(mimeType) because it also
     * takes MIME type inheritance into account.
     * \since 5.66
     */
    bool supportsMimeType(const QString &mimeType) const;

    /*!
     * Returns A string list of formfactors this plugin is useful for, e.g. desktop, handset or mediacenter.
     * The keys for this are not formally defined, though the above-mentioned values should be used when applicable.
     *
     * \since 5.12
     */
    QStringList formFactors() const;

    /*!
     * Returns whether the plugin should be enabled by default.
     * This is only a recommendation, applications can ignore this value if they want to.
     */
    bool isEnabledByDefault() const;

    /*!
     * Returns \c true if the plugin is enabled in \a config, otherwise returns isEnabledByDefault().
     * This can be used in conjunction with KPluginWidget.
     *
     * The \a config param should be a KConfigGroup object, because KCoreAddons can not depend
     * on KConfig directly, this parameter is a template.
     *
     * \a config KConfigGroup where the enabled state is stored
     *
     * \since 5.89
     */
    template<typename T>
    bool isEnabled(const T &config) const
    {
        Q_ASSERT(config.isValid());
        return config.readEntry(pluginId() + QLatin1String("Enabled"), isEnabledByDefault());
    }

    /*!
     * Returns the string value for \a key from the metadata or \a defaultValue if the key does not exist
     *
     * if QString is not the correct type for \a key you should use the other overloads or KPluginMetaData::rawData
     */
    QString value(QStringView key, const QString &defaultValue = QString()) const;
    QString value(const QString &key, const QString &defaultValue = QString()) const; // TODO KF7: remove
    QString value(const QString &key, const char *ch) const = delete;

    /*!
     * \overload
     * \since 5.88
     */
    bool value(QStringView key, bool defaultValue) const;
    bool value(const QString &key, bool defaultValue) const; // TODO KF7: remove

    /*!
     * \overload
     * \since 5.88
     */
    int value(QStringView key, int defaultValue) const;
    int value(const QString &key, int defaultValue) const; // TODO KF7: remove

    /*! Returns the value for \a key from the metadata or \a defaultValue if the key does not exist.
     * If the type of \a key is string, a list containing just that string will be returned.
     * If the type is array, the list will contain one entry for each array member.
     * \overload
     * \since 5.88
     */
    QStringList value(QStringView key, const QStringList &defaultValue) const;
    QStringList value(const QString &key, const QStringList &defaultValue) const; // TODO KF7: remove

    /*!
     *
     */
    bool operator==(const KPluginMetaData &other) const;

    inline bool operator!=(const KPluginMetaData &other) const
    {
        return !(*this == other);
    }

    /*!
     * Returns true if the instance represents a static plugin
     *
     * \note for loading plugin the plugin independently of it being static/dynamic
     * use KPluginFactory::loadFactory or KPluginFactory::instantiatePlugin.
     * \since 5.89
     */
    bool isStaticPlugin() const;

private:
    KCOREADDONS_NO_EXPORT QStaticPlugin staticPlugin() const;
    KCOREADDONS_NO_EXPORT QString requestedFileName() const;

    QExplicitlySharedDataPointer<KPluginMetaDataPrivate> d;
    friend class KPluginFactory;
    friend class KPluginMetaDataPrivate;
};

inline size_t qHash(const KPluginMetaData &md, size_t seed)
{
    return qHash(md.pluginId(), seed);
}

/*!
 * \since 6.0
 * \relates KPluginMetaData
 */
KCOREADDONS_EXPORT QDebug operator<<(QDebug debug, const KPluginMetaData &metaData);

Q_DECLARE_TYPEINFO(KPluginMetaData, Q_RELOCATABLE_TYPE);
Q_DECLARE_OPERATORS_FOR_FLAGS(KPluginMetaData::KPluginMetaDataOptions)

#endif // KPLUGINMETADATA_H

/*
    This file is part of the KDE project
    SPDX-FileCopyrightText: 2013 Martin Gräßlin <mgraesslin@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KCOLORSCHEMEMANAGER_H
#define KCOLORSCHEMEMANAGER_H

#include <kcolorscheme_export.h>

#include <QObject>
#include <memory>

class QAbstractItemModel;
class QGuiApplication;
class QModelIndex;
class QIcon;

class KActionMenu;
class KColorSchemeManagerPrivate;

/*!
 * \class KColorSchemeManager
 * \inmodule KColorScheme
 *
 * \brief A small helper to get access to all available color schemes and activating a scheme in the
 * QApplication.
 *
 * This is useful for applications which want to provide a selection of custom color
 * schemes to their user. For example it is very common for photo and painting applications to use
 * a dark color scheme even if the default is a light scheme. Since version 5.67 it also allows
 * going back to following the system color scheme.
 *
 * The model() member function provides access to the KColorSchemeModel that the KColorSchemeManager uses
 * which holds all the available color schemes. A possible usage looks like the following:
 *
 * \code
 * KColorSchemeManager *manager = new KColorSchemeManager(this);
 * QListView *view = new QListView(this);
 * view->setModel(manager->model());
 * connect(view, &QListView::activated, manager, [manager] (const QModelIndex &index) {
 *     manager->activateSchemeId(index.data(KColorSchemeModel::IdRole).toString());
 *  });
 *
 * \endcode
 *
 * A convenience function that creates a KActionMenu that contains and activates color schemes exists
 * in KColorSchemeMenu::createMenu
 *
 * By default KColorSchemeManager remembers the activated color scheme and restores it on the next
 * start of the application. Use setAutosaveChanges() to change this behavior.
 *
 * \sa KColorSchemeMenu::createMenu, KColorSchemeModel
 *
 * \since 5.0
 */
class KCOLORSCHEME_EXPORT KColorSchemeManager : public QObject
{
    Q_OBJECT
public:
#if KCOLORSCHEME_ENABLE_DEPRECATED_SINCE(6, 6)
    KCOLORSCHEME_DEPRECATED_VERSION(6, 6, "Use KColorSchemeManager::instance()")
    explicit KColorSchemeManager(QObject *parent = nullptr);
#endif

    ~KColorSchemeManager() override;

    /*!
     * A QAbstractItemModel of all available color schemes.
     *
     * The model provides the name of the scheme in Qt::DisplayRole, a preview
     * in Qt::DelegateRole and the full path to the scheme file in Qt::UserRole. The system theme
     * has an empty Qt::UserRole.
     *
     * Returns Model of all available color schemes.
     * \sa KColorSchemeModel
     */
    QAbstractItemModel *model() const;

    /*!
     * Returns the model index for the scheme with the given \param id. If no such
     * scheme exists an invalid index is returned. If you pass an empty string the index
     * returned is equivalent to going back to following the system scheme.
     * \sa model
     *
     * \since 6.6
     */
    QModelIndex indexForSchemeId(const QString &id) const;

    /*!
     * Returns the model index for the scheme with the given \param name. If no such
     * scheme exists an invalid index is returned. If you pass an empty
     * string the index that is equivalent to going back to following the system scheme is returned
     * for versions 5.67 and newer.
     * \sa model
     */
    QModelIndex indexForScheme(const QString &name) const;

#if KCOLORSCHEME_ENABLE_DEPRECATED_SINCE(6, 19)

    /*!
     * Saves the color scheme to config file. The scheme is saved by default whenever it's changed.
     * Use this method when autosaving is turned off, see setAutosaveChanges().
     *
     * \since 5.89
     *
     * \deprecated[6.19]
     * Use saveSchemeIdToConfigFile() instead which is not locale dependent
     */
    KCOLORSCHEME_DEPRECATED_VERSION(6, 19, "Use saveSchemeIdToConfigFile() instead which is not locale dependent")
    void saveSchemeToConfigFile(const QString &schemeName) const;
#endif

    /*!
     * Saves the color scheme to config file. The scheme is saved by default whenever it's changed.
     * Use this method when autosaving is turned off, see setAutosaveChanges().
     *
     * \since 6.19
     */
    void saveSchemeIdToConfigFile(const QString &schemeId) const;
    /*!
     * Sets color scheme autosaving. Default value is \c true.
     * If this is set to \c false, the scheme is not going to be remembered when the
     * application is restarted.
     *
     * \param autosaveChanges Enables/Disables autosaving of the color scheme.
     * \since 5.89
     */
    void setAutosaveChanges(bool autosaveChanges);

    /*!
     * Returns the id of the currently active scheme or an empty string if the default
     * scheme is active.
     *
     * \since 5.107
     */
    QString activeSchemeId() const;

    /*!
     * Returns the name of the currently active scheme or an empty string if the default
     * scheme is active.
     *
     * \since 6.6
     */
    QString activeSchemeName() const;

    /*!
     * Returns the manager for the current application instance.
     * If no instance is existing, it will be constructed.
     * Must be called after construction of the gui application instance.
     *
     * Returns color scheme manager for the current application instance
     *
     * \since 6.6
     */
    static KColorSchemeManager *instance();

public Q_SLOTS:
    /*!
     * \brief Activates the KColorScheme identified by the provided \param index.
     *
     * Installs the KColorScheme as the QApplication's QPalette.
     *
     * \param index The index for the KColorScheme to activate.
     * The index must reference the QAbstractItemModel provided by model(). Since
     * version 5.67 passing an invalid index activates the system scheme.
     * \sa model()
     */
    void activateScheme(const QModelIndex &index);

    /*!
     * \brief Activates the KColorScheme identified by the provided \param schemeId.
     *
     * Installs the KColorScheme as the QApplication's QPalette.
     *
     * \param schemeId The id for the KColorScheme to activate.
     * Passing an empty id activates the system scheme.
     * \sa model()
     *
     * \since 6.19
     */
    void activateSchemeId(const QString &schemeId);

private:
    class KCOLORSCHEME_NO_EXPORT GuardApplicationConstructor
    {
    };
    KCOLORSCHEME_NO_EXPORT explicit KColorSchemeManager(GuardApplicationConstructor, QGuiApplication *app);
    KCOLORSCHEME_NO_EXPORT void init();

    std::unique_ptr<KColorSchemeManagerPrivate> const d;
};

#endif

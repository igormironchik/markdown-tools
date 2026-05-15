/*
    SPDX-FileCopyrightText: 2023 Alexander Lohnau <alexander.lohnau@gmx.de>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KQUICKCONFIGMODULELOADER_H
#define KQUICKCONFIGMODULELOADER_H

#include "kcmutilsquick_export.h"

#include <KPluginFactory>
#include <memory>

class QQmlEngine;
class KQuickConfigModule;

/*!
    \namespace KQuickConfigModuleLoader
    \inmodule KCMUtilsQuick
 */
namespace KQuickConfigModuleLoader
{
/*!
 * \inmodule KCMUtilsQuick
 * \brief Loads a QML KCM from the given plugin \a metaData as a child of \a parent with the given \a args.
 *
 * \a engine The QQmlEngine to use. If not set, an internal engine will be created. If your application has an existing engine, this must be passed in.
 */
KCMUTILSQUICK_EXPORT KPluginFactory::Result<KQuickConfigModule>
loadModule(const KPluginMetaData &metaData, QObject *parent = nullptr, const QVariantList &args = {}, const std::shared_ptr<QQmlEngine> &engine = {});
}
#endif

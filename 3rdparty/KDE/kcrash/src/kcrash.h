/*
    This file is part of the KDE Libraries
    SPDX-FileCopyrightText: 2000 Timo Hummel <timo.hummel@sap.com>
    SPDX-FileCopyrightText: 2000 Tom Braun <braunt@fh-konstanz.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KCRASH_H
#define KCRASH_H

#include <kcrash_export.h>

#include <qglobal.h>

class QString;

/*!
 * \namespace KCrash
 * \inmodule KCrash
 *
 * \brief This namespace contains functions to handle crashes.
 *
 * It allows you to set a crash handler function that will be called
 * when your application crashes and also provides a default crash
 * handler that implements the following functionality:
 * \list
 * \li Launches the KDE crash display application (DrKonqi) to let
 * the user report the bug and/or debug it.
 * \li Calls an emergency save function that you can set with
 * setEmergencySaveFunction() to attempt to save the application's data.
 * \li Autorestarts your application.
 * \endlist
 *
 * \note All the above features are optional and you need to enable them
 * explicitly. By default, the defaultCrashHandler() will not do anything.
 * However, if you are using KApplication, it will by default enable launching
 * DrKonqi on crashes, unless the --nocrashhandler argument was passed on
 * the command line or the environment variable KDE_DEBUG is set to any value.
 */
namespace KCrash
{
/*!
 * Initialize KCrash.
 *
 * This does nothing if $KDE_DEBUG is set.
 *
 * Call this in your main() after setting up KAboutData to ensure that the crash handler is launched.
 * \since 5.15
 */
KCRASH_EXPORT void initialize();

/*!
 * The default crash handler.
 * Do not call this function directly. Instead, use
 * setCrashHandler() to set it as your application's crash handler.
 *
 * \a signal the signal number
 *
 * \note If you implement your own crash handler, you will have to
 * call this function from your implementation if you want to use the
 * features of this namespace.
 */
KCRASH_EXPORT void defaultCrashHandler(int signal);

/*!
 * \typedef KCrash::HandlerType
 *
 * Typedef for a pointer to a crash handler function.
 * The function's argument is the number of the signal.
 */
typedef void (*HandlerType)(int);

/*!
 * Install a function to be called when a crash occurs.
 * A crash occurs when one of the following signals is
 * caught: SIGSEGV, SIGBUS, SIGFPE, SIGILL, SIGABRT.
 *
 * \a handler this can be one of:
 * \list
 * \li null, in which case signal catching is disabled
 * (by setting the signal handler for the crash signals to SIG_DFL)
 * \li a user defined function in the form:
 * static (if in a class) void myCrashHandler(int);
 * \li if handler is omitted, the default crash handler is installed
 * \endlist
 *
 * \note If you use setDrKonqiEnabled(true), setEmergencySaveFunction(myfunc)
 * or setFlags(AutoRestart), you do not need to call this function
 * explicitly. The default crash handler is automatically installed by
 * those functions if needed. However, if you set a custom crash handler,
 * those functions will not change it.
 */
KCRASH_EXPORT void setCrashHandler(HandlerType handler = defaultCrashHandler);

/*!
 * Returns the installed crash handler.
 */
KCRASH_EXPORT HandlerType crashHandler();

/*!
 * Installs a function which should try to save the application's data.
 *
 * \note It is the crash handler's responsibility to call this function.
 * Therefore, if no crash handler is set, the default crash handler
 * is installed to ensure the save function will be called.
 *
 * \a saveFunction the handler to install
 */
KCRASH_EXPORT void setEmergencySaveFunction(HandlerType saveFunction = nullptr);

/*!
 * Returns the currently set emergency save function.
 */
KCRASH_EXPORT HandlerType emergencySaveFunction();

/*!
 * Options to determine how the default crash handler should behave.
 *
 * \value KeepFDs Don't close all file descriptors immediately
 * \value SaferDialog Start DrKonqi without arbitrary disk access
 * \value AlwaysDirectly Never try to to start DrKonqi via kdeinit. Use fork() and exec() instead. This enum has been deprecated. This is now the default, and
 * does not need to be set.
 * \value [since 4.1] AutoRestart autorestart this application. Only sensible for KUniqueApplications
 *
 */
enum CrashFlag {
    KeepFDs = 1,
    SaferDialog = 2,
    AlwaysDirectly = 4,
    AutoRestart = 8,
};
Q_DECLARE_FLAGS(CrashFlags, CrashFlag)
Q_DECLARE_OPERATORS_FOR_FLAGS(CrashFlags)

/*!
 * Set options to determine how the default crash handler should behave.
 *
 * \a flags ORed together CrashFlags
 */
KCRASH_EXPORT void setFlags(KCrash::CrashFlags flags);

/*!
 * Enables or disables launching DrKonqi from the crash handler.
 * By default, launching DrKonqi is enabled when QCoreApplication is created.
 * To disable it:
 * \code
 * void disableDrKonqi()
 * {
 *   KCrash::setDrKonqiEnabled(false);
 * }
 * Q_CONSTRUCTOR_FUNCTION(disableDrKonqi)
 * \endcode
 * \note It is the crash handler's responsibility to launch DrKonqi.
 * Therefore, if no crash handler is set, this method also installs
 * the default crash handler to ensure that DrKonqi will be launched.
 * \since 4.5
 */
KCRASH_EXPORT void setDrKonqiEnabled(bool enabled);

/*!
 * Returns true if DrKonqi is set to be launched from the crash handler or false otherwise.
 * \since 4.5
 */
KCRASH_EXPORT bool isDrKonqiEnabled();

/*!
 * Allows providing information to be included in the bug report. Prefer setErrorExtraInformation as it is more flexible.
 *
 * \since 5.69
 */
KCRASH_EXPORT void setErrorMessage(const QString &message);

/*!
 * Sets the error tags to be included in the crash report. These are rendered as tags in the crash reporting system.
 *
 * Note that server-side limits apply to the length of these so you should only put short, sortable data in here.
 * \since 6.11
 */
KCRASH_EXPORT void setErrorTags(const QHash<QString, QString> &details);

/*!
 * Sets the error details to be included in the crash report. These are rendered as extra blobs of data and can any form.
 *
 * Note that these are subject to event ingestion limits and should be kept at reasonable sizes to prevent event rejection.
 * \since 6.11
 */
KCRASH_EXPORT void setErrorExtraData(const QHash<QString, QString> &details);

/*!
 * Sets better GPU data.
 *
 * By default KCrash will try to determine the GPU name, this may however not be accurate data. In particular on
 * multi-gpu systems it may not be possible to determine whether the integrated or dedicated GPU is in use.
 *
 * You should call this function once you know which GPU will be in use for application. This is a free form string.
 *
 * Server-side limits may apply; keep it as short as possible.
 *
 * At least 'name' should be set. Additional supported fields follow Sentry unless documented otherwise.
 *
 * Supported fields are listed at
 * https://develop.sentry.dev/sdk/data-model/event-payloads/contexts/#gpu-context
 * \since 6.11
 */
KCRASH_EXPORT void setGPUData(const QVariantHash &data);
}

#endif

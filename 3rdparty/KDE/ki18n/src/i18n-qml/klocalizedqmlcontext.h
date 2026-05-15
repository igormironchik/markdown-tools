/*
    SPDX-FileCopyrightText: 2011 Marco Martin <mart@kde.org>
    SPDX-FileCopyrightText: 2015 Aleix Pol Gonzalez <aleixpol@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KLOCALIZEDQMLCONTEXT_H
#define KLOCALIZEDQMLCONTEXT_H

#include <ki18nqml_export.h>

#include <QObject>
#include <QVariant>
#include <QtQmlIntegration>
#include <memory>

class QQmlEngine;

class KLocalizedQmlContextPrivate;

/*!
 * \class KLocalizedQmlContext
 * \inmodule KI18nQml
 *
 * \brief This class is meant to be used to simplify integration of the KI18n framework
 * in QML.
 *
 * The way to do so, is by creating this object and setting it as a context
 * object:
 *
 * \code
 * QQmlApplicationEngine engine;
 * auto ctx = new KLocalizedQmlContext(&engine);
 * engine->rootContext()->setContextObject(ctx);
 * QQmlEngine::setContextForObject(ctx, engine.rootContext());
 * ctx->setTranslationDomain(...);
 * \endcode
 *
 * In many cases this can be simplified using KLocalization::setupLocalizedContext():
 * \code
 * QQmlApplicationEngine engine;
 * KLocalization::setupLocalizedContext(&engine);
 * \endcode
 *
 * Then KI18n.i18n*() and KI18n.xi18n*() functions should be available for use from the code
 * loaded in the engine, for the view.
 *
 * Unlike its predecessor KLocalizedContext this does automatically trigger
 * a binding re-evaluation when the application language is changed at runtime
 * (with Qt 6.6 or higher).
 *
 * \note Plural functions differ from the C/C++ version. On QML/JS we can get a
 * real value easily. To solve warnings on those cases we'll cast the first argument
 * to make sure it's taken into account for the plural.
 *
 * \warning For history reasons the i18n functions are also available on the root context without KI18n.* prefix.
 * It is very recommended to use the KI18n.* version moving forward, so QML tooling like qmlls and qmllint can inspect
 * the API calls. KI18n.* was introduced in version 6.21.
 *
 * \since 6.8
 */

/*!
    \qmltype KI18n
    \since 6.21
    \nativetype KLocalizedQmlContext
    \inqmlmodule org.kde.ki18n
    \brief Advanced internationalization support for QML.

    \sa KLocalizedQmlContext::KLocalizedQmlContext()

    To use it, create an instance and set it as the context object of your QML engine in C++:

    \code
    QQmlApplicationEngine engine;
    KLocalization::setupLocalizedContext(&engine);
    \endcode

    Then you can use the KI18n.i18n*() and KI18n.xi18n*() functions from QML code, for example:

    \qml
    import QtQuick
    import org.kde.ki18n
    Text {
        text: KI18n.i18nc("@label", "Hello World")
    }
    \endqml
*/

/*!
    \qmltype KI18nContext
    \since 6.23
    \nativetype KLocalizedQmlContext
    \inherits KI18n
    \inqmlmodule org.kde.ki18n
    \brief Creatable variant of KI18n

    \sa KI18n
    \sa KLocalizedQmlContext::KLocalizedQmlContext()

    This is a creatable variant of KI18n and may be used to create multiple localized contexts right from within QML.
    In particular useful when working with multiple translation domains (such as in library use cases).

    Unlike the KI18n variant this does not require help from the C++ side and can be used entirely from QML:

    \qml
    readonly property KI18nContext libI18n: KI18nContext {
        translationDomain: "librocketship"
    }
    text: libI18n.i18nc("@title", "Welcome to Space")
    \endqml
*/
class KI18NQML_EXPORT KLocalizedQmlContext : public QObject
{
    Q_OBJECT

    /*!
     * \property KLocalizedQmlContext::translationDomain
     *
     * This property only needs to be specified if the context is being run on a library.
     * in an application there is no need to set the translation domain as the application's
     * domain can be used.
     */
    /*!
        \qmlproperty string KI18nContext::translationDomain

        The translation domain to use for this context. This must be explicitly set for the KI18nContext to be functional.
    */
    Q_PROPERTY(QString translationDomain READ translationDomain WRITE setTranslationDomain NOTIFY translationDomainChanged)

    QML_ELEMENT
    QML_NAMED_ELEMENT(KI18nContext)

public:
    /*!
     *
     */
    explicit KLocalizedQmlContext(QObject *parent = nullptr);
    ~KLocalizedQmlContext() override;

    /*!
     *
     */
    QString translationDomain() const;

    /*!
     *
     */
    void setTranslationDomain(const QString &domain);

    /*!
        \qmlmethod string KI18n::i18n(message: string, param1: variant = undefined, param2: variant = undefined, param3: variant = undefined, param4: variant =
            undefined, param5: variant = undefined, param6: variant = undefined, param7: variant = undefined, param8: variant = undefined, param9: variant =
            undefined, param10: variant = undefined)
        \sa KLocalizedString::i18n()
    */
    Q_INVOKABLE QString i18n(const QString &message,
                             const QVariant &param1 = QVariant(),
                             const QVariant &param2 = QVariant(),
                             const QVariant &param3 = QVariant(),
                             const QVariant &param4 = QVariant(),
                             const QVariant &param5 = QVariant(),
                             const QVariant &param6 = QVariant(),
                             const QVariant &param7 = QVariant(),
                             const QVariant &param8 = QVariant(),
                             const QVariant &param9 = QVariant(),
                             const QVariant &param10 = QVariant()) const;

    /*!
        \qmlmethod string KI18n::i18nc(context: string, message: string, param1: variant = undefined, param2: variant = undefined, param3: variant = undefined,
            param4: variant = undefined, param5: variant = undefined, param6: variant = undefined, param7: variant = undefined, param8: variant = undefined,
            param9: variant = undefined, param10: variant = undefined)
        \sa KLocalizedString::i18nc()
    */
    Q_INVOKABLE QString i18nc(const QString &context,
                              const QString &message,
                              const QVariant &param1 = QVariant(),
                              const QVariant &param2 = QVariant(),
                              const QVariant &param3 = QVariant(),
                              const QVariant &param4 = QVariant(),
                              const QVariant &param5 = QVariant(),
                              const QVariant &param6 = QVariant(),
                              const QVariant &param7 = QVariant(),
                              const QVariant &param8 = QVariant(),
                              const QVariant &param9 = QVariant(),
                              const QVariant &param10 = QVariant()) const;

    /*!
        \qmlmethod string KI18n::i18np(singular: string, plural: string, param1: variant = undefined, param2: variant = undefined, param3: variant = undefined,
            param4: variant = undefined, param5: variant = undefined, param6: variant = undefined, param7: variant = undefined, param8: variant = undefined,
            param9: variant = undefined, param10: variant = undefined)
        \sa KLocalizedString::i18np()
    */
    Q_INVOKABLE QString i18np(const QString &singular,
                              const QString &plural,
                              const QVariant &param1 = QVariant(),
                              const QVariant &param2 = QVariant(),
                              const QVariant &param3 = QVariant(),
                              const QVariant &param4 = QVariant(),
                              const QVariant &param5 = QVariant(),
                              const QVariant &param6 = QVariant(),
                              const QVariant &param7 = QVariant(),
                              const QVariant &param8 = QVariant(),
                              const QVariant &param9 = QVariant(),
                              const QVariant &param10 = QVariant()) const;

    /*!
        \qmlmethod string KI18n::i18ncp(context: string, singular: string, plural: string, param1: variant = undefined, param2: variant = undefined,
            param3: variant = undefined, param4: variant = undefined, param5: variant = undefined, param6: variant = undefined, param7: variant = undefined,
            param8: variant = undefined, param9: variant = undefined, param10: variant = undefined)
        \sa KLocalizedString::i18ncp()
    */
    Q_INVOKABLE QString i18ncp(const QString &context,
                               const QString &singular,
                               const QString &plural,
                               const QVariant &param1 = QVariant(),
                               const QVariant &param2 = QVariant(),
                               const QVariant &param3 = QVariant(),
                               const QVariant &param4 = QVariant(),
                               const QVariant &param5 = QVariant(),
                               const QVariant &param6 = QVariant(),
                               const QVariant &param7 = QVariant(),
                               const QVariant &param8 = QVariant(),
                               const QVariant &param9 = QVariant(),
                               const QVariant &param10 = QVariant()) const;

    /*!
        \qmlmethod string KI18n::i18nd(domain: string, message: string, param1: variant = undefined, param2: variant = undefined, param3: variant = undefined,
            param4: variant = undefined, param5: variant = undefined, param6: variant = undefined, param7: variant = undefined, param8: variant = undefined,
            param9: variant = undefined, param10: variant = undefined)
        \sa KLocalizedString::i18nd()
    */
    Q_INVOKABLE QString i18nd(const QString &domain,
                              const QString &message,
                              const QVariant &param1 = QVariant(),
                              const QVariant &param2 = QVariant(),
                              const QVariant &param3 = QVariant(),
                              const QVariant &param4 = QVariant(),
                              const QVariant &param5 = QVariant(),
                              const QVariant &param6 = QVariant(),
                              const QVariant &param7 = QVariant(),
                              const QVariant &param8 = QVariant(),
                              const QVariant &param9 = QVariant(),
                              const QVariant &param10 = QVariant()) const;

    /*!
        \qmlmethod string KI18n::i18ndc(domain: string, context: string, message: string, param1: variant = undefined, param2: variant = undefined,
            param3: variant = undefined, param4: variant = undefined, param5: variant = undefined, param6: variant = undefined, param7: variant = undefined,
            param8: variant = undefined, param9: variant = undefined, param10: variant = undefined)
        \sa KLocalizedString::i18ndc()
    */
    Q_INVOKABLE QString i18ndc(const QString &domain,
                               const QString &context,
                               const QString &message,
                               const QVariant &param1 = QVariant(),
                               const QVariant &param2 = QVariant(),
                               const QVariant &param3 = QVariant(),
                               const QVariant &param4 = QVariant(),
                               const QVariant &param5 = QVariant(),
                               const QVariant &param6 = QVariant(),
                               const QVariant &param7 = QVariant(),
                               const QVariant &param8 = QVariant(),
                               const QVariant &param9 = QVariant(),
                               const QVariant &param10 = QVariant()) const;

    /*!
        \qmlmethod string KI18n::i18ndp(domain: string, singular: string, plural: string, param1: variant = undefined, param2: variant = undefined,
            param3: variant = undefined, param4: variant = undefined, param5: variant = undefined, param6: variant = undefined, param7: variant = undefined,
            param8: variant = undefined, param9: variant = undefined, param10: variant = undefined)
        \sa KLocalizedString::i18ndp()
    */
    Q_INVOKABLE QString i18ndp(const QString &domain,
                               const QString &singular,
                               const QString &plural,
                               const QVariant &param1 = QVariant(),
                               const QVariant &param2 = QVariant(),
                               const QVariant &param3 = QVariant(),
                               const QVariant &param4 = QVariant(),
                               const QVariant &param5 = QVariant(),
                               const QVariant &param6 = QVariant(),
                               const QVariant &param7 = QVariant(),
                               const QVariant &param8 = QVariant(),
                               const QVariant &param9 = QVariant(),
                               const QVariant &param10 = QVariant()) const;

    /*!
        \qmlmethod string KI18n::i18ndcp(domain: string, context: string, singular: string, plural: string, param1: variant = undefined,
            param2: variant = undefined, param3: variant = undefined, param4: variant = undefined, param5: variant = undefined,
            param6: variant = undefined, param7: variant = undefined, param8: variant = undefined, param9: variant = undefined,
            param10: variant = undefined)
        \sa KLocalizedString::i18ndcp()
    */
    Q_INVOKABLE QString i18ndcp(const QString &domain,
                                const QString &context,
                                const QString &singular,
                                const QString &plural,
                                const QVariant &param1 = QVariant(),
                                const QVariant &param2 = QVariant(),
                                const QVariant &param3 = QVariant(),
                                const QVariant &param4 = QVariant(),
                                const QVariant &param5 = QVariant(),
                                const QVariant &param6 = QVariant(),
                                const QVariant &param7 = QVariant(),
                                const QVariant &param8 = QVariant(),
                                const QVariant &param9 = QVariant(),
                                const QVariant &param10 = QVariant()) const;

    /*!
        \qmlmethod string KI18n::xi18n(message: string, param1: variant = undefined, param2: variant = undefined, param3: variant = undefined, param4: variant =
            undefined, param5: variant = undefined, param6: variant = undefined, param7: variant = undefined, param8: variant = undefined, param9: variant =
            undefined, param10: variant = undefined)
        \sa KLocalizedString::xi18n()
    */
    Q_INVOKABLE QString xi18n(const QString &message,
                              const QVariant &param1 = QVariant(),
                              const QVariant &param2 = QVariant(),
                              const QVariant &param3 = QVariant(),
                              const QVariant &param4 = QVariant(),
                              const QVariant &param5 = QVariant(),
                              const QVariant &param6 = QVariant(),
                              const QVariant &param7 = QVariant(),
                              const QVariant &param8 = QVariant(),
                              const QVariant &param9 = QVariant(),
                              const QVariant &param10 = QVariant()) const;

    /*!
        \qmlmethod string KI18n::xi18nc(context: string, message: string, param1: variant = undefined, param2: variant = undefined, param3: variant = undefined,
            param4: variant = undefined, param5: variant = undefined, param6: variant = undefined, param7: variant = undefined, param8: variant = undefined,
            param9: variant = undefined, param10: variant = undefined)
        \sa KLocalizedString::xi18nc()
    */
    Q_INVOKABLE QString xi18nc(const QString &context,
                               const QString &message,
                               const QVariant &param1 = QVariant(),
                               const QVariant &param2 = QVariant(),
                               const QVariant &param3 = QVariant(),
                               const QVariant &param4 = QVariant(),
                               const QVariant &param5 = QVariant(),
                               const QVariant &param6 = QVariant(),
                               const QVariant &param7 = QVariant(),
                               const QVariant &param8 = QVariant(),
                               const QVariant &param9 = QVariant(),
                               const QVariant &param10 = QVariant()) const;

    /*!
        \qmlmethod string KI18n::xi18np(singular: string, plural: string, param1: variant = undefined, param2: variant = undefined, param3: variant = undefined,
            param4: variant = undefined, param5: variant = undefined, param6: variant = undefined, param7: variant = undefined, param8: variant = undefined,
            param9: variant = undefined, param10: variant = undefined)
        \sa KLocalizedString::xi18np()
    */
    Q_INVOKABLE QString xi18np(const QString &singular,
                               const QString &plural,
                               const QVariant &param1 = QVariant(),
                               const QVariant &param2 = QVariant(),
                               const QVariant &param3 = QVariant(),
                               const QVariant &param4 = QVariant(),
                               const QVariant &param5 = QVariant(),
                               const QVariant &param6 = QVariant(),
                               const QVariant &param7 = QVariant(),
                               const QVariant &param8 = QVariant(),
                               const QVariant &param9 = QVariant(),
                               const QVariant &param10 = QVariant()) const;

    /*!
        \qmlmethod string KI18n::xi18ncp(context: string, singular: string, plural: string, param1: variant = undefined, param2: variant = undefined,
            param3: variant = undefined, param4: variant = undefined, param5: variant = undefined, param6: variant = undefined, param7: variant = undefined,
            param8: variant = undefined, param9: variant = undefined, param10: variant = undefined)
        \sa KLocalizedString::xi18ncp()
    */
    Q_INVOKABLE QString xi18ncp(const QString &context,
                                const QString &singular,
                                const QString &plural,
                                const QVariant &param1 = QVariant(),
                                const QVariant &param2 = QVariant(),
                                const QVariant &param3 = QVariant(),
                                const QVariant &param4 = QVariant(),
                                const QVariant &param5 = QVariant(),
                                const QVariant &param6 = QVariant(),
                                const QVariant &param7 = QVariant(),
                                const QVariant &param8 = QVariant(),
                                const QVariant &param9 = QVariant(),
                                const QVariant &param10 = QVariant()) const;

    /*!
        \qmlmethod string KI18n::xi18nd(domain: string, message: string, param1: variant = undefined, param2: variant = undefined, param3: variant = undefined,
            param4: variant = undefined, param5: variant = undefined, param6: variant = undefined, param7: variant = undefined, param8: variant = undefined,
            param9: variant = undefined, param10: variant = undefined)
        \sa KLocalizedString::xi18nd()
    */
    Q_INVOKABLE QString xi18nd(const QString &domain,
                               const QString &message,
                               const QVariant &param1 = QVariant(),
                               const QVariant &param2 = QVariant(),
                               const QVariant &param3 = QVariant(),
                               const QVariant &param4 = QVariant(),
                               const QVariant &param5 = QVariant(),
                               const QVariant &param6 = QVariant(),
                               const QVariant &param7 = QVariant(),
                               const QVariant &param8 = QVariant(),
                               const QVariant &param9 = QVariant(),
                               const QVariant &param10 = QVariant()) const;

    /*!
        \qmlmethod string KI18n::xi18ndc(domain: string, context: string, message: string, param1: variant = undefined, param2: variant = undefined,
            param3: variant = undefined, param4: variant = undefined, param5: variant = undefined, param6: variant = undefined, param7: variant = undefined,
            param8: variant = undefined, param9: variant = undefined, param10: variant = undefined)
        \sa KLocalizedString::xi18ndc()
    */
    Q_INVOKABLE QString xi18ndc(const QString &domain,
                                const QString &context,
                                const QString &message,
                                const QVariant &param1 = QVariant(),
                                const QVariant &param2 = QVariant(),
                                const QVariant &param3 = QVariant(),
                                const QVariant &param4 = QVariant(),
                                const QVariant &param5 = QVariant(),
                                const QVariant &param6 = QVariant(),
                                const QVariant &param7 = QVariant(),
                                const QVariant &param8 = QVariant(),
                                const QVariant &param9 = QVariant(),
                                const QVariant &param10 = QVariant()) const;

    /*!
        \qmlmethod string KI18n::xi18ndp(domain: string, singular: string, plural: string, param1: variant = undefined, param2: variant = undefined,
            param3: variant = undefined, param4: variant = undefined, param5: variant = undefined, param6: variant = undefined, param7: variant = undefined,
            param8: variant = undefined, param9: variant = undefined, param10: variant = undefined)
        \sa KLocalizedString::xi18ndp()
    */
    Q_INVOKABLE QString xi18ndp(const QString &domain,
                                const QString &singular,
                                const QString &plural,
                                const QVariant &param1 = QVariant(),
                                const QVariant &param2 = QVariant(),
                                const QVariant &param3 = QVariant(),
                                const QVariant &param4 = QVariant(),
                                const QVariant &param5 = QVariant(),
                                const QVariant &param6 = QVariant(),
                                const QVariant &param7 = QVariant(),
                                const QVariant &param8 = QVariant(),
                                const QVariant &param9 = QVariant(),
                                const QVariant &param10 = QVariant()) const;

    /*!
        \qmlmethod string KI18n::xi18ndcp(domain: string, context: string, singular: string, plural: string, param1: variant = undefined,
            param2: variant = undefined, param3: variant = undefined, param4: variant = undefined, param5: variant = undefined,
            param6: variant = undefined, param7: variant = undefined, param8: variant = undefined, param9: variant = undefined,
            param10: variant = undefined)
        \sa KLocalizedString::xi18ndcp()
    */
    Q_INVOKABLE QString xi18ndcp(const QString &domain,
                                 const QString &context,
                                 const QString &singular,
                                 const QString &plural,
                                 const QVariant &param1 = QVariant(),
                                 const QVariant &param2 = QVariant(),
                                 const QVariant &param3 = QVariant(),
                                 const QVariant &param4 = QVariant(),
                                 const QVariant &param5 = QVariant(),
                                 const QVariant &param6 = QVariant(),
                                 const QVariant &param7 = QVariant(),
                                 const QVariant &param8 = QVariant(),
                                 const QVariant &param9 = QVariant(),
                                 const QVariant &param10 = QVariant()) const;

Q_SIGNALS:
    void translationDomainChanged(const QString &translationDomain);

private:
    std::unique_ptr<KLocalizedQmlContextPrivate> const d;
};

/*!
 * \namespace KLocalization
 * \inmodule KI18nQml
 * \brief Namespace containing helpers for localization.
 */
namespace KLocalization
{
namespace Internal
{
[[nodiscard]] KI18NQML_EXPORT KLocalizedQmlContext *createLocalizedContext(QQmlEngine *engine);
}

/*!
 * Creates a KLocalizedQmlContext engine and sets it up in the
 * root context of \a engine.
 *
 * If \c TRANSLATION_DOMAIN is defined, that is also set on
 * the created context.
 *
 * \since 6.8
 * \relates KLocalizedQmlContext
 */
inline KLocalizedQmlContext *setupLocalizedContext(QQmlEngine *engine)
{
    auto ctx = Internal::createLocalizedContext(engine);
#ifdef TRANSLATION_DOMAIN
    ctx->setTranslationDomain(QStringLiteral(TRANSLATION_DOMAIN));
#endif
    return ctx;
}
}

#endif

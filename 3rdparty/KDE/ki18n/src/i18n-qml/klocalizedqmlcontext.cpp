/*
    SPDX-FileCopyrightText: 2013 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

// Undefine this because we don't want our i18n*() method names to be turned into i18nd*()
#undef TRANSLATION_DOMAIN

#include "klocalizedqmlcontext.h"

#include <klocalizedstring.h>

#include <QCoreApplication>
#include <QQmlContext>
#include <QQmlEngine>

#include "ki18n_qml_logging.h"

namespace
{
/*!
    \internal
    \brief Watches for QCoreApplication::languageChange() events and notifies
    QML engines to re-evaluate their bindings.
*/
class LanguageChangeWatcher : public QObject
{
    Q_OBJECT
public:
    /*!
        \brief Registers a QML engine to be notified on language change events.

        \a engine The engine to notify.

        If the engine is already registered, this is a no-op.
    */
    void hello(const QPointer<QQmlEngine> &engine)
    {
        Q_ASSERT(QThread::currentThread() == QCoreApplication::instance()->thread());
        if (!engine) {
            return;
        }

        if (!engines.contains(engine)) {
            engines.push_back(engine);
            qCDebug(KI18N) << "registered engine" << engine << "engines:" << engines;
        }
    }

    bool eventFilter(QObject *watched, QEvent *event) override
    {
        if (event->type() == QEvent::LanguageChange && watched == QCoreApplication::instance()) {
            // For simplicity we don't have Contexts say bye to us (would require refcounting).
            // Instead we always make sure we only operate on living engines.
            engines.removeIf([](const auto &engine) {
                return !engine;
            });
            for (const auto &engine : std::as_const(engines)) {
                qCDebug(KI18N) << "triggering binding reevaluation for engine" << engine;
                // run this deferred so we can be sure other things have reacted, such as KLocalizedString
                // having updated its internal caches
                QMetaObject::invokeMethod(engine, &QQmlEngine::retranslate, Qt::QueuedConnection);
            }
        }
        return QObject::eventFilter(watched, event);
    }

private:
    QList<QPointer<QQmlEngine>> engines;
};

LanguageChangeWatcher s_watcher;
} // namespace

class KLocalizedQmlContextPrivate
{
public:
    void markCurrentFunctionAsTranslationBinding(const KLocalizedQmlContext *q) const;

    QString m_translationDomain;
};

void KLocalizedQmlContextPrivate::markCurrentFunctionAsTranslationBinding(const KLocalizedQmlContext *q) const
{
    if (auto engine = qmlEngine(q); engine) {
        engine->markCurrentFunctionAsTranslationBinding();
    } else {
        qCDebug(KI18N) << "No QML engine available, KLocalizedQmlContext not properly set up?";
    }
}

KLocalizedQmlContext::KLocalizedQmlContext(QObject *parent)
    : QObject(parent)
    , d(new KLocalizedQmlContextPrivate)
{
    static bool filtered = [] {
        QCoreApplication::instance()->installEventFilter(&s_watcher);
        return true;
    }();
    Q_UNUSED(filtered);

    auto engine = qmlEngine(this);
    s_watcher.hello(engine ? engine : qobject_cast<QQmlEngine *>(parent) /* the global context has the engine as parent */);
}

KLocalizedQmlContext::~KLocalizedQmlContext() = default;

QString KLocalizedQmlContext::translationDomain() const
{
    return d->m_translationDomain;
}

void KLocalizedQmlContext::setTranslationDomain(const QString &domain)
{
    if (domain != d->m_translationDomain) {
        d->m_translationDomain = domain;
        Q_EMIT translationDomainChanged(domain);
    }
}

static void subsVariant(KLocalizedString &trMessage, const QVariant &value)
{
    switch (value.userType()) {
    case QMetaType::QString:
        trMessage = trMessage.subs(value.toString());
        break;
    case QMetaType::Int:
        trMessage = trMessage.subs(value.toInt());
        break;
    case QMetaType::Double:
        trMessage = trMessage.subs(value.toDouble());
        break;
    case QMetaType::Char:
        trMessage = trMessage.subs(value.toChar());
        break;
    default:
        if (value.canConvert<QString>()) {
            trMessage = trMessage.subs(value.toString());
        } else {
            trMessage = trMessage.subs(QStringLiteral("???"));
            qCWarning(KI18N) << "couldn't convert" << value << "to translate";
        }
    }
}

static void resolveMessage(KLocalizedString &trMessage,
                           const QVariant &param1,
                           const QVariant &param2,
                           const QVariant &param3,
                           const QVariant &param4,
                           const QVariant &param5,
                           const QVariant &param6,
                           const QVariant &param7,
                           const QVariant &param8,
                           const QVariant &param9,
                           const QVariant &param10 = QVariant())
{
    if (param1.isValid()) {
        subsVariant(trMessage, param1);
    }
    if (param2.isValid()) {
        subsVariant(trMessage, param2);
    }
    if (param3.isValid()) {
        subsVariant(trMessage, param3);
    }
    if (param4.isValid()) {
        subsVariant(trMessage, param4);
    }
    if (param5.isValid()) {
        subsVariant(trMessage, param5);
    }
    if (param6.isValid()) {
        subsVariant(trMessage, param6);
    }
    if (param7.isValid()) {
        subsVariant(trMessage, param7);
    }
    if (param8.isValid()) {
        subsVariant(trMessage, param8);
    }
    if (param9.isValid()) {
        subsVariant(trMessage, param9);
    }
    if (param10.isValid()) {
        subsVariant(trMessage, param10);
    }
}

static void resolvePlural(KLocalizedString &trMessage, const QVariant &param)
{
    trMessage = trMessage.subs(param.toInt());
}

QString KLocalizedQmlContext::i18n(const QString &message,
                                   const QVariant &param1,
                                   const QVariant &param2,
                                   const QVariant &param3,
                                   const QVariant &param4,
                                   const QVariant &param5,
                                   const QVariant &param6,
                                   const QVariant &param7,
                                   const QVariant &param8,
                                   const QVariant &param9,
                                   const QVariant &param10) const
{
    if (message.isEmpty()) {
        qCWarning(KI18N) << "i18n() needs at least one parameter";
        return QString();
    }

    KLocalizedString trMessage;
    if (!d->m_translationDomain.isEmpty()) {
        trMessage = ki18nd(d->m_translationDomain.toUtf8().constData(), message.toUtf8().constData());
    } else {
        trMessage = ki18n(message.toUtf8().constData());
    }

    resolveMessage(trMessage, param1, param2, param3, param4, param5, param6, param7, param8, param9, param10);

    d->markCurrentFunctionAsTranslationBinding(this);
    return trMessage.toString();
}

QString KLocalizedQmlContext::i18nc(const QString &context,
                                    const QString &message,
                                    const QVariant &param1,
                                    const QVariant &param2,
                                    const QVariant &param3,
                                    const QVariant &param4,
                                    const QVariant &param5,
                                    const QVariant &param6,
                                    const QVariant &param7,
                                    const QVariant &param8,
                                    const QVariant &param9,
                                    const QVariant &param10) const
{
    if (context.isEmpty() || message.isEmpty()) {
        qCWarning(KI18N).noquote().nospace() << "i18nc(\"" << context << message << "\") needs at least two arguments";
        return QString();
    }

    KLocalizedString trMessage;
    if (!d->m_translationDomain.isEmpty()) {
        trMessage = ki18ndc(d->m_translationDomain.toUtf8().constData(), context.toUtf8().constData(), message.toUtf8().constData());
    } else {
        trMessage = ki18nc(context.toUtf8().constData(), message.toUtf8().constData());
    }

    resolveMessage(trMessage, param1, param2, param3, param4, param5, param6, param7, param8, param9, param10);

    d->markCurrentFunctionAsTranslationBinding(this);
    return trMessage.toString();
}

QString KLocalizedQmlContext::i18np(const QString &singular,
                                    const QString &plural,
                                    const QVariant &param1,
                                    const QVariant &param2,
                                    const QVariant &param3,
                                    const QVariant &param4,
                                    const QVariant &param5,
                                    const QVariant &param6,
                                    const QVariant &param7,
                                    const QVariant &param8,
                                    const QVariant &param9,
                                    const QVariant &param10) const
{
    if (singular.isEmpty() || plural.isEmpty()) {
        qCWarning(KI18N).noquote().nospace() << "i18np(\"" << singular << plural << "\") needs at least two arguments";
        return QString();
    }

    KLocalizedString trMessage;
    if (!d->m_translationDomain.isEmpty()) {
        trMessage = ki18ndp(d->m_translationDomain.toUtf8().constData(), singular.toUtf8().constData(), plural.toUtf8().constData());
    } else {
        trMessage = ki18np(singular.toUtf8().constData(), plural.toUtf8().constData());
    }

    resolvePlural(trMessage, param1);
    resolveMessage(trMessage, param2, param3, param4, param5, param6, param7, param8, param9, param10);

    d->markCurrentFunctionAsTranslationBinding(this);
    return trMessage.toString();
}

QString KLocalizedQmlContext::i18ncp(const QString &context,
                                     const QString &singular,
                                     const QString &plural,
                                     const QVariant &param1,
                                     const QVariant &param2,
                                     const QVariant &param3,
                                     const QVariant &param4,
                                     const QVariant &param5,
                                     const QVariant &param6,
                                     const QVariant &param7,
                                     const QVariant &param8,
                                     const QVariant &param9,
                                     const QVariant &param10) const
{
    if (context.isEmpty() || singular.isEmpty() || plural.isEmpty()) {
        qCWarning(KI18N) << "i18ncp() needs at least three arguments";
        return QString();
    }

    KLocalizedString trMessage;
    if (!d->m_translationDomain.isEmpty()) {
        trMessage =
            ki18ndcp(d->m_translationDomain.toUtf8().constData(), context.toUtf8().constData(), singular.toUtf8().constData(), plural.toUtf8().constData());
    } else {
        trMessage = ki18ncp(context.toUtf8().constData(), singular.toUtf8().constData(), plural.toUtf8().constData());
    }

    resolvePlural(trMessage, param1);
    resolveMessage(trMessage, param2, param3, param4, param5, param6, param7, param8, param9, param10);

    d->markCurrentFunctionAsTranslationBinding(this);
    return trMessage.toString();
}

QString KLocalizedQmlContext::i18nd(const QString &domain,
                                    const QString &message,
                                    const QVariant &param1,
                                    const QVariant &param2,
                                    const QVariant &param3,
                                    const QVariant &param4,
                                    const QVariant &param5,
                                    const QVariant &param6,
                                    const QVariant &param7,
                                    const QVariant &param8,
                                    const QVariant &param9,
                                    const QVariant &param10) const
{
    if (domain.isEmpty() || message.isEmpty()) {
        qCWarning(KI18N).noquote().nospace() << "i18nd(\"" << domain << message << "\") needs at least two parameters";
        return QString();
    }

    KLocalizedString trMessage = ki18nd(domain.toUtf8().constData(), message.toUtf8().constData());

    resolveMessage(trMessage, param1, param2, param3, param4, param5, param6, param7, param8, param9, param10);

    d->markCurrentFunctionAsTranslationBinding(this);
    return trMessage.toString();
}

QString KLocalizedQmlContext::i18ndc(const QString &domain,
                                     const QString &context,
                                     const QString &message,
                                     const QVariant &param1,
                                     const QVariant &param2,
                                     const QVariant &param3,
                                     const QVariant &param4,
                                     const QVariant &param5,
                                     const QVariant &param6,
                                     const QVariant &param7,
                                     const QVariant &param8,
                                     const QVariant &param9,
                                     const QVariant &param10) const
{
    if (domain.isEmpty() || context.isEmpty() || message.isEmpty()) {
        qCWarning(KI18N) << "i18ndc() needs at least three arguments";
        return QString();
    }

    KLocalizedString trMessage = ki18ndc(domain.toUtf8().constData(), context.toUtf8().constData(), message.toUtf8().constData());

    resolveMessage(trMessage, param1, param2, param3, param4, param5, param6, param7, param8, param9, param10);

    d->markCurrentFunctionAsTranslationBinding(this);
    return trMessage.toString();
}

QString KLocalizedQmlContext::i18ndp(const QString &domain,
                                     const QString &singular,
                                     const QString &plural,
                                     const QVariant &param1,
                                     const QVariant &param2,
                                     const QVariant &param3,
                                     const QVariant &param4,
                                     const QVariant &param5,
                                     const QVariant &param6,
                                     const QVariant &param7,
                                     const QVariant &param8,
                                     const QVariant &param9,
                                     const QVariant &param10) const
{
    if (domain.isEmpty() || singular.isEmpty() || plural.isEmpty()) {
        qCWarning(KI18N) << "i18ndp() needs at least three arguments";
        return QString();
    }

    KLocalizedString trMessage = ki18ndp(domain.toUtf8().constData(), singular.toUtf8().constData(), plural.toUtf8().constData());

    resolvePlural(trMessage, param1);
    resolveMessage(trMessage, param2, param3, param4, param5, param6, param7, param8, param9, param10);

    d->markCurrentFunctionAsTranslationBinding(this);
    return trMessage.toString();
}

QString KLocalizedQmlContext::i18ndcp(const QString &domain,
                                      const QString &context,
                                      const QString &singular,
                                      const QString &plural,
                                      const QVariant &param1,
                                      const QVariant &param2,
                                      const QVariant &param3,
                                      const QVariant &param4,
                                      const QVariant &param5,
                                      const QVariant &param6,
                                      const QVariant &param7,
                                      const QVariant &param8,
                                      const QVariant &param9,
                                      const QVariant &param10) const
{
    if (domain.isEmpty() || context.isEmpty() || singular.isEmpty() || plural.isEmpty()) {
        qCWarning(KI18N) << "i18ndcp() needs at least four arguments";
        return QString();
    }

    KLocalizedString trMessage =
        ki18ndcp(domain.toUtf8().constData(), context.toUtf8().constData(), singular.toUtf8().constData(), plural.toUtf8().constData());

    resolvePlural(trMessage, param1);
    resolveMessage(trMessage, param2, param3, param4, param5, param6, param7, param8, param9, param10);

    d->markCurrentFunctionAsTranslationBinding(this);
    return trMessage.toString();
}

/////////////////////////

QString KLocalizedQmlContext::xi18n(const QString &message,
                                    const QVariant &param1,
                                    const QVariant &param2,
                                    const QVariant &param3,
                                    const QVariant &param4,
                                    const QVariant &param5,
                                    const QVariant &param6,
                                    const QVariant &param7,
                                    const QVariant &param8,
                                    const QVariant &param9,
                                    const QVariant &param10) const
{
    if (message.isEmpty()) {
        qCWarning(KI18N) << "xi18n() needs at least one parameter";
        return QString();
    }

    KLocalizedString trMessage;
    if (!d->m_translationDomain.isEmpty()) {
        trMessage = kxi18nd(d->m_translationDomain.toUtf8().constData(), message.toUtf8().constData());
    } else {
        trMessage = kxi18n(message.toUtf8().constData());
    }

    resolveMessage(trMessage, param1, param2, param3, param4, param5, param6, param7, param8, param9, param10);

    d->markCurrentFunctionAsTranslationBinding(this);
    return trMessage.toString();
}

QString KLocalizedQmlContext::xi18nc(const QString &context,
                                     const QString &message,
                                     const QVariant &param1,
                                     const QVariant &param2,
                                     const QVariant &param3,
                                     const QVariant &param4,
                                     const QVariant &param5,
                                     const QVariant &param6,
                                     const QVariant &param7,
                                     const QVariant &param8,
                                     const QVariant &param9,
                                     const QVariant &param10) const
{
    if (context.isEmpty() || message.isEmpty()) {
        qCWarning(KI18N).noquote().nospace() << "xi18nc(\"" << context << message << "\") needs at least two arguments";
        return QString();
    }

    KLocalizedString trMessage;
    if (!d->m_translationDomain.isEmpty()) {
        trMessage = kxi18ndc(d->m_translationDomain.toUtf8().constData(), context.toUtf8().constData(), message.toUtf8().constData());
    } else {
        trMessage = kxi18nc(context.toUtf8().constData(), message.toUtf8().constData());
    }

    resolveMessage(trMessage, param1, param2, param3, param4, param5, param6, param7, param8, param9, param10);

    d->markCurrentFunctionAsTranslationBinding(this);
    return trMessage.toString();
}

QString KLocalizedQmlContext::xi18np(const QString &singular,
                                     const QString &plural,
                                     const QVariant &param1,
                                     const QVariant &param2,
                                     const QVariant &param3,
                                     const QVariant &param4,
                                     const QVariant &param5,
                                     const QVariant &param6,
                                     const QVariant &param7,
                                     const QVariant &param8,
                                     const QVariant &param9,
                                     const QVariant &param10) const
{
    if (singular.isEmpty() || plural.isEmpty()) {
        qCWarning(KI18N).noquote().nospace() << "xi18np(\"" << singular << plural << "\") needs at least two arguments";
        return QString();
    }

    KLocalizedString trMessage;
    if (!d->m_translationDomain.isEmpty()) {
        trMessage = kxi18ndp(d->m_translationDomain.toUtf8().constData(), singular.toUtf8().constData(), plural.toUtf8().constData());
    } else {
        trMessage = kxi18np(singular.toUtf8().constData(), plural.toUtf8().constData());
    }

    resolvePlural(trMessage, param1);
    resolveMessage(trMessage, param2, param3, param4, param5, param6, param7, param8, param9, param10);

    d->markCurrentFunctionAsTranslationBinding(this);
    return trMessage.toString();
}

QString KLocalizedQmlContext::xi18ncp(const QString &context,
                                      const QString &singular,
                                      const QString &plural,
                                      const QVariant &param1,
                                      const QVariant &param2,
                                      const QVariant &param3,
                                      const QVariant &param4,
                                      const QVariant &param5,
                                      const QVariant &param6,
                                      const QVariant &param7,
                                      const QVariant &param8,
                                      const QVariant &param9,
                                      const QVariant &param10) const
{
    if (context.isEmpty() || singular.isEmpty() || plural.isEmpty()) {
        qCWarning(KI18N) << "xi18ncp() needs at least three arguments";
        return QString();
    }

    KLocalizedString trMessage;
    if (!d->m_translationDomain.isEmpty()) {
        trMessage =
            kxi18ndcp(d->m_translationDomain.toUtf8().constData(), context.toUtf8().constData(), singular.toUtf8().constData(), plural.toUtf8().constData());
    } else {
        trMessage = kxi18ncp(context.toUtf8().constData(), singular.toUtf8().constData(), plural.toUtf8().constData());
    }

    resolvePlural(trMessage, param1);
    resolveMessage(trMessage, param2, param3, param4, param5, param6, param7, param8, param9, param10);

    d->markCurrentFunctionAsTranslationBinding(this);
    return trMessage.toString();
}

QString KLocalizedQmlContext::xi18nd(const QString &domain,
                                     const QString &message,
                                     const QVariant &param1,
                                     const QVariant &param2,
                                     const QVariant &param3,
                                     const QVariant &param4,
                                     const QVariant &param5,
                                     const QVariant &param6,
                                     const QVariant &param7,
                                     const QVariant &param8,
                                     const QVariant &param9,
                                     const QVariant &param10) const
{
    if (domain.isEmpty() || message.isEmpty()) {
        qCWarning(KI18N).noquote().nospace() << "xi18nd(\"" << domain << message << "\") needs at least two parameters";
        return QString();
    }

    KLocalizedString trMessage = kxi18nd(domain.toUtf8().constData(), message.toUtf8().constData());

    resolveMessage(trMessage, param1, param2, param3, param4, param5, param6, param7, param8, param9, param10);

    d->markCurrentFunctionAsTranslationBinding(this);
    return trMessage.toString();
}

QString KLocalizedQmlContext::xi18ndc(const QString &domain,
                                      const QString &context,
                                      const QString &message,
                                      const QVariant &param1,
                                      const QVariant &param2,
                                      const QVariant &param3,
                                      const QVariant &param4,
                                      const QVariant &param5,
                                      const QVariant &param6,
                                      const QVariant &param7,
                                      const QVariant &param8,
                                      const QVariant &param9,
                                      const QVariant &param10) const
{
    if (domain.isEmpty() || context.isEmpty() || message.isEmpty()) {
        qCWarning(KI18N) << "x18ndc() needs at least three arguments";
        return QString();
    }

    KLocalizedString trMessage = kxi18ndc(domain.toUtf8().constData(), context.toUtf8().constData(), message.toUtf8().constData());

    resolveMessage(trMessage, param1, param2, param3, param4, param5, param6, param7, param8, param9, param10);

    d->markCurrentFunctionAsTranslationBinding(this);
    return trMessage.toString();
}

QString KLocalizedQmlContext::xi18ndp(const QString &domain,
                                      const QString &singular,
                                      const QString &plural,
                                      const QVariant &param1,
                                      const QVariant &param2,
                                      const QVariant &param3,
                                      const QVariant &param4,
                                      const QVariant &param5,
                                      const QVariant &param6,
                                      const QVariant &param7,
                                      const QVariant &param8,
                                      const QVariant &param9,
                                      const QVariant &param10) const
{
    if (domain.isEmpty() || singular.isEmpty() || plural.isEmpty()) {
        qCWarning(KI18N) << "xi18ndp() needs at least three arguments";
        return QString();
    }

    KLocalizedString trMessage = kxi18ndp(domain.toUtf8().constData(), singular.toUtf8().constData(), plural.toUtf8().constData());

    resolvePlural(trMessage, param1);
    resolveMessage(trMessage, param2, param3, param4, param5, param6, param7, param8, param9, param10);

    d->markCurrentFunctionAsTranslationBinding(this);
    return trMessage.toString();
}

QString KLocalizedQmlContext::xi18ndcp(const QString &domain,
                                       const QString &context,
                                       const QString &singular,
                                       const QString &plural,
                                       const QVariant &param1,
                                       const QVariant &param2,
                                       const QVariant &param3,
                                       const QVariant &param4,
                                       const QVariant &param5,
                                       const QVariant &param6,
                                       const QVariant &param7,
                                       const QVariant &param8,
                                       const QVariant &param9,
                                       const QVariant &param10) const
{
    if (domain.isEmpty() || context.isEmpty() || singular.isEmpty() || plural.isEmpty()) {
        qCWarning(KI18N) << "xi18ndcp() needs at least four arguments";
        return QString();
    }

    KLocalizedString trMessage =
        kxi18ndcp(domain.toUtf8().constData(), context.toUtf8().constData(), singular.toUtf8().constData(), plural.toUtf8().constData());

    resolvePlural(trMessage, param1);
    resolveMessage(trMessage, param2, param3, param4, param5, param6, param7, param8, param9, param10);

    d->markCurrentFunctionAsTranslationBinding(this);
    return trMessage.toString();
}

KLocalizedQmlContext *KLocalization::Internal::createLocalizedContext(QQmlEngine *engine)
{
    auto ctx = new KLocalizedQmlContext(engine);
    engine->rootContext()->setContextObject(ctx);
    QQmlEngine::setContextForObject(ctx, engine->rootContext());
    return ctx;
}

#include "klocalizedqmlcontext.moc"
#include "moc_klocalizedqmlcontext.cpp"

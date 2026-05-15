// SPDX-FileCopyrightText: 2024 Carl Schwan <carl@carlschwan.eu>
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "pagestackattached.h"

#include "formlayoutattached.h"
#include "loggingcategory.h"

#include <QMetaObject>
#include <QQmlContext>
#include <QQmlEngine>

using namespace Qt::StringLiterals;

template<typename... Args>
bool callIfValid(QObject *object, const char *method, Args &&...args)
{
    auto metaObject = object->metaObject();
    auto index = metaObject->indexOfMethod(method);
    if (index != -1) {
        auto method = metaObject->method(index);
        return method.invoke(object, args...);
    }

    return false;
}

bool tryCall(QObject *object, QByteArrayView pageRowMethod, QByteArrayView stackViewMethod, const QVariant &page, const QVariantMap &properties)
{
    const auto metaObject = object->metaObject();

    QByteArray name = pageRowMethod + "(QVariant,QVariant)";
    if (auto index = metaObject->indexOfMethod(name.data()); index != -1) {
        return metaObject->method(index).invoke(object, page, QVariant::fromValue(properties));
    } else if (QQmlComponent *component = page.value<QQmlComponent *>(); component != nullptr) {
        return metaObject->invokeMethod(object, stackViewMethod.data(), component, properties);
    } else if (QQuickItem *item = page.value<QQuickItem *>(); item != nullptr) {
        return metaObject->invokeMethod(object, stackViewMethod.data(), item, properties);
    } else if (QUrl url = page.toUrl(); !url.isEmpty()) {
        return metaObject->invokeMethod(object, stackViewMethod.data(), url, properties);
    }

    return false;
}

PageStackAttached::PageStackAttached(QObject *parent)
    : QQuickAttachedPropertyPropagator(parent)
{
    m_parentItem = qobject_cast<QQuickItem *>(parent);

    if (!m_parentItem) {
        qCDebug(KirigamiLayoutsLog) << "PageStack must be attached to an Item" << parent;
        return;
    }

    if (hasStackCapabilities(m_parentItem)) {
        setPageStack(m_parentItem);
    } else if (!m_pageStack) {
        QQuickItem *candidate = m_parentItem->parentItem();
        while (candidate) {
            if (hasStackCapabilities(candidate)) {
                qmlAttachedPropertiesObject<PageStackAttached>(candidate, true);
                break;
            }
            candidate = candidate->parentItem();
        }
    }

    initialize();
}

QQuickItem *PageStackAttached::pageStack() const
{
    return m_pageStack;
}

void PageStackAttached::setPageStack(QQuickItem *pageStack)
{
    if (!pageStack || m_pageStack == pageStack || !hasStackCapabilities(pageStack)) {
        return;
    }

    m_customStack = true;
    m_pageStack = pageStack;

    propagatePageStack(pageStack);

    Q_EMIT pageStackChanged();
}

void PageStackAttached::propagatePageStack(QQuickItem *pageStack)
{
    if (!pageStack) {
        return;
    }

    if (!m_customStack && m_pageStack != pageStack) {
        m_pageStack = pageStack;
        Q_EMIT pageStackChanged();
    }

    const auto stacks = attachedChildren();
    for (QQuickAttachedPropertyPropagator *child : stacks) {
        PageStackAttached *stackAttached = qobject_cast<PageStackAttached *>(child);
        if (stackAttached) {
            stackAttached->propagatePageStack(m_pageStack);
        }
    }
}

void PageStackAttached::push(const QVariant &page, const QVariantMap &properties)
{
    if (!m_pageStack) {
        qCWarning(KirigamiLayoutsLog) << "Pushing in an empty PageStackAttached";
        return;
    }

    if (!tryCall(m_pageStack, "push", "pushItem", page, properties)) {
        qCWarning(KirigamiLayoutsLog) << "Invalid parameters to push: " << page << properties;
    }
}

void PageStackAttached::replace(const QVariant &page, const QVariantMap &properties)
{
    if (!m_pageStack) {
        qCWarning(KirigamiLayoutsLog) << "replacing in an empty PageStackAttached";
        return;
    }

    if (!tryCall(m_pageStack, "replace", "replaceCurrentItem", page, properties)) {
        qCWarning(KirigamiLayoutsLog) << "Invalid parameters to replace: " << page << properties;
    }
}

void PageStackAttached::pop(const QVariant &page)
{
    if (!m_pageStack) {
        qCWarning(KirigamiLayoutsLog) << "Pushing in an empty PageStackAttached";
        return;
    }

    if (callIfValid(m_pageStack, "pop(QVariant)", page)) {
        return;
    } else if (page.canConvert<QQuickItem *>() && callIfValid(m_pageStack, "popToItem(QQuickItem*)", page.value<QQuickItem *>())) {
        return;
    } else if (callIfValid(m_pageStack, "popCurrentItem()")) {
        return;
    }

    qCWarning(KirigamiLayoutsLog) << "Pop operation failed on stack" << m_pageStack << "with page" << page;
}

void PageStackAttached::clear()
{
    if (!m_pageStack) {
        qCWarning(KirigamiLayoutsLog) << "Clearing in an empty PageStackAttached";
        return;
    }

    if (!callIfValid(m_pageStack, "clear()")) {
        qCWarning(KirigamiLayoutsLog) << "Call to clear() failed";
    }
}

bool PageStackAttached::hasStackCapabilities(QQuickItem *candidate)
{
    // Duck type the candidate in order to see if it can be used as a stack having the expected methods
    auto metaObject = candidate->metaObject();
    Q_ASSERT(metaObject);

    auto hasPageRowOrStackViewMethod = [metaObject](QByteArrayView pageRowMethod, QByteArrayView stackViewMethod) -> bool {
        // For PageRow, we require a single method that takes QVariant,QVariant as argument.
        QByteArray name = pageRowMethod + "(QVariant,QVariant)";
        if (metaObject->indexOfMethod(name.data()) != -1) {
            return true;
        }

        // For StackView, we require three variants of the method.
        name = stackViewMethod + "(QQmlComponent*,QVariantMap)";
        if (metaObject->indexOfMethod(name.data()) == -1) {
            return false;
        }

        name = stackViewMethod + "(QQuickItem*,QVariantMap)";
        if (metaObject->indexOfMethod(name.data()) == -1) {
            return false;
        }

        name = stackViewMethod + "(QUrl,QVariantMap)";
        if (metaObject->indexOfMethod(name.data()) == -1) {
            return false;
        }

        return true;
    };

    if (!hasPageRowOrStackViewMethod("push", "pushItem")) {
        return false;
    }

    if (!hasPageRowOrStackViewMethod("replace", "replaceCurrentItem")) {
        return false;
    }

    auto index = metaObject->indexOfMethod("pop(QVariant)");
    if (index == -1) {
        index = metaObject->indexOfMethod("popToItem(QQuickItem*)");
        if (index == -1) {
            return false;
        }
        index = metaObject->indexOfMethod("popCurrentItem()");
        if (index == -1) {
            return false;
        }
    }

    index = metaObject->indexOfMethod("clear()");
    if (index == -1) {
        return false;
    }

    return true;
}

PageStackAttached *PageStackAttached::qmlAttachedProperties(QObject *object)
{
    return new PageStackAttached(object);
}

void PageStackAttached::attachedParentChange(QQuickAttachedPropertyPropagator *newParent, QQuickAttachedPropertyPropagator *oldParent)
{
    Q_UNUSED(oldParent);

    PageStackAttached *stackAttached = qobject_cast<PageStackAttached *>(newParent);
    if (stackAttached) {
        propagatePageStack(stackAttached->pageStack());
    }
}

#include "moc_pagestackattached.cpp"

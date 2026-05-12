// SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
// SPDX-FileCopyrightText: 2025 Harald Sitter <sitter@kde.org>

#include "kosreleaseproxy.h"

QString KOSReleaseProxy::name() const
{
    return os.name();
}

QString KOSReleaseProxy::version() const
{
    return os.version();
}

QString KOSReleaseProxy::id() const
{
    return os.id();
}

QStringList KOSReleaseProxy::idLike() const
{
    return os.idLike();
}

QString KOSReleaseProxy::versionCodename() const
{
    return os.versionCodename();
}

QString KOSReleaseProxy::versionId() const
{
    return os.versionId();
}

QString KOSReleaseProxy::prettyName() const
{
    return os.prettyName();
}

QString KOSReleaseProxy::ansiColor() const
{
    return os.ansiColor();
}

QString KOSReleaseProxy::cpeName() const
{
    return os.cpeName();
}

QString KOSReleaseProxy::homeUrl() const
{
    return os.homeUrl();
}

QString KOSReleaseProxy::documentationUrl() const
{
    return os.documentationUrl();
}

QString KOSReleaseProxy::supportUrl() const
{
    return os.supportUrl();
}

QString KOSReleaseProxy::bugReportUrl() const
{
    return os.bugReportUrl();
}

QString KOSReleaseProxy::privacyPolicyUrl() const
{
    return os.privacyPolicyUrl();
}

QString KOSReleaseProxy::buildId() const
{
    return os.buildId();
}
QString KOSReleaseProxy::variant() const
{
    return os.variant();
}

QString KOSReleaseProxy::variantId() const
{
    return os.variantId();
}

QString KOSReleaseProxy::logo() const
{
    return os.logo();
}

QStringList KOSReleaseProxy::extraKeys() const
{
    return os.extraKeys();
}

QString KOSReleaseProxy::extraValue(const QString &key) const
{
    return os.extraValue(key);
}

#include "moc_kosreleaseproxy.cpp"

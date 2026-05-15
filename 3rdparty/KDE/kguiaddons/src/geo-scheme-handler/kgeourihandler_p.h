/*
    SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KGEOURIHANDLER_H
#define KGEOURIHANDLER_H

#include <QString>

class QUrl;

/*! Fallback handler for geo: URIs by forwarding them to a web service.
 *
 *  This handles three cases of geo: URIs:
 *  - when containing a query argument, the query URL template is used
 *  - when containing valid WGS-84 coordinates, the coordinate URL template is used
 *  - otherwise the fallback URL is returned
 *
 *  URL templates can contain any number of the following placeholders in angle brackets:
 *  - LAT - the latitude
 *  - LON - the longitude
 *  - Q - the query string
 *  - Z - the zoom level for a Web Mercator map projection
 *
 *  \sa https://en.wikipedia.org/wiki/Geo_URI_scheme
 *  \sa https://datatracker.ietf.org/doc/html/rfc5870
 *
 * \internal
 */
class KGeoUriHandler
{
public:
    void setCoordinateTemplate(const QString &coordTmpl);
    void setQueryTemplate(const QString &queryTmpl);
    void setFallbackUrl(const QString &fallbackUrl);

    QString handleUri(const QUrl &geoUri);

private:
    QString m_coordTmpl;
    QString m_queryTmpl;
    QString m_fallbackUrl;
};

#endif // KGEOURIHANDLER_H

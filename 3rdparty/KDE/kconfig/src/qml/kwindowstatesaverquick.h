// SPDX-FileCopyrightText: 2024 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: LGPL-2.0-or-later

#ifndef KWINDOWSTATESAVER_QUICK_H
#define KWINDOWSTATESAVER_QUICK_H

#include <QQmlEngine>

/*!
 * \qmltype WindowStateSaver
 * \inqmlmodule org.kde.config
 *
 * \brief Creates a KWindowStateSaver in QML, and assigns it to the window it's parented to.
 *
 * Functions exactly as KWindowStateSaver in C++, as it's a small wrapper around it.
 *
 * The generated file containing the position and size of the application will be stored under \c ~/.local/state/appnamerc,
 * where \c appname derives from the QCoreApplication::setApplicationName or the component name in KAboutData::KAboutData.
 *
 * \code
 * import org.kde.config as KConfig
 *
 * Kirigami.ApplicationWindow {
 *     id: root
 *
 *     title: i18n("My Window")
 *
 *     KConfig.WindowStateSaver {
 *         configGroupName: "MainWindow"
 *     }
 * }
 * \endcode
 * \since 6.5
 *
 * \sa KWindowStateSaver
 */
class KWindowStateSaverQuick : public QObject, public QQmlParserStatus
{
    Q_OBJECT
    QML_ELEMENT
    QML_NAMED_ELEMENT(WindowStateSaver)
    Q_INTERFACES(QQmlParserStatus)

    /*!
     * \qmlproperty string WindowStateSaver::configGroupName
     */
    Q_PROPERTY(QString configGroupName READ configGroupName WRITE setConfigGroupName NOTIFY configGroupNameChanged REQUIRED)

public:
    void classBegin() override;
    void componentComplete() override;

    void setConfigGroupName(const QString &name);
    QString configGroupName() const;

Q_SIGNALS:
    void configGroupNameChanged();

private:
    QString m_configGroupName;
};

#endif

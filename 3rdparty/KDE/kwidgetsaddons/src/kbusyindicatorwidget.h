/*
    SPDX-FileCopyrightText: 2019 Harald Sitter <sitter@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef KBUSYINDICATORWIDGET_H
#define KBUSYINDICATORWIDGET_H

#include <QWidget>
#include <kwidgetsaddons_export.h>
#include <memory>

/*!
 * \class KBusyIndicatorWidget
 * \inmodule KWidgetsAddons
 *
 * \brief Rotating spinning icon to indicate busyness.
 *
 * When you need to communicate to the user that your application is busy with
 * something you'll want to use a KBusyIndicatorWidget to display an spinnning
 * indicator icon. The spinning animation can be stopped and started.
 *
 * A way of using this widget is to combine it with a QLabel to construct a
 * status line:
 *
 * \code
 * auto layout = new QHBoxLayout;
 * layout->addWidget(new KBusyIndicatorWidget);
 * layout->addWidget(new QLabel(QStringLiteral("Watering the flowers...")));
 * \endcode
 *
 * \image kbusyindicatorwidget.png "KBusyIndicatorWidget with label"
 *
 * The KBusyIndicatorWidget differs from the KPixmapSequenceWidget by animating
 * a scaled icon instead of rendering a pixmap sequence. It supports multiple,
 * semi-arbitrary sizes, with quality constrained only by the resolution of the
 * available icons.
 *
 * \since 5.61.0
 */
class KWIDGETSADDONS_EXPORT KBusyIndicatorWidget : public QWidget
{
    Q_OBJECT

    /*!
     * \property KBusyIndicatorWidget::isRunning
     */
    Q_PROPERTY(bool isRunning READ isRunning WRITE setRunning)

public:
    /*!
     * Create a new KBusyIndicatorWidget widget
     */
    explicit KBusyIndicatorWidget(QWidget *parent = nullptr);

    ~KBusyIndicatorWidget() override;

    /*!
     * Return the smallest reasonable size for the widget
     */
    QSize minimumSizeHint() const override;

    /*!
     * Returns whether the spinning animation is running
     *
     * \sa setRunning()
     *
     * \since 6.11.0
     */
    bool isRunning() const;

public Q_SLOTS:
    /*!
     * Start the spinning animation
     *
     * \sa setRunning()
     *
     * \since 6.10.0
     */
    void start();

    /*!
     * Stop the spinning animation
     *
     * \sa setRunning()
     *
     * \since 6.10.0
     */
    void stop();

    /*!
     * By calling this method with @p enable = true the spinning
     * abnimation will be started. Stopped if false.
     *
     * \sa start()
     * \sa stop()
     *
     * \since 6.11.0
     */
    void setRunning(const bool enable = true);

protected:
    void showEvent(QShowEvent *event) override;
    void hideEvent(QHideEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    void paintEvent(QPaintEvent *) override;
    bool event(QEvent *event) override;

private:
    std::unique_ptr<class KBusyIndicatorWidgetPrivate> const d;
};

#endif // KBUSYINDICATORWIDGET_H

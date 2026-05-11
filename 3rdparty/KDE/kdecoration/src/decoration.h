/*
 * SPDX-FileCopyrightText: 2014 Martin Gräßlin <mgraesslin@kde.org>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */
#pragma once

#include "decorationshadow.h"
#include <kdecoration3/kdecoration3_export.h>

#include <QMarginsF>
#include <QObject>
#include <QRectF>
#include <QSharedDataPointer>

class QHoverEvent;
class QMenu;
class QMouseEvent;
class QPainter;
class QWheelEvent;

/**
 * @brief Framework for creating window decorations.
 *
 **/
namespace KDecoration3
{
class DecorationPrivate;
class DecoratedWindow;
class DecorationButton;
class DecorationSettings;
class DecorationStateData;
class PositionerData;

/**
 * \brief Decoration corner radius.
 */
class KDECORATIONS3_EXPORT BorderRadius
{
public:
    BorderRadius();
    explicit BorderRadius(qreal radius);
    explicit BorderRadius(qreal topLeft, qreal topRight, qreal bottomRight, qreal bottomLeft);

    bool operator<=>(const BorderRadius &other) const = default;

    qreal topLeft() const;
    qreal topRight() const;
    qreal bottomRight() const;
    qreal bottomLeft() const;

private:
    qreal m_topLeft = 0;
    qreal m_topRight = 0;
    qreal m_bottomRight = 0;
    qreal m_bottomLeft = 0;
};

/**
 * \brief Decoration border outline.
 */
class KDECORATIONS3_EXPORT BorderOutline
{
public:
    BorderOutline();
    explicit BorderOutline(qreal thickness, const QColor &color = Qt::transparent, const BorderRadius &radius = BorderRadius());

    bool operator==(const BorderOutline &other) const = default;
    bool operator!=(const BorderOutline &other) const = default;

    bool isNull() const;

    qreal thickness() const;
    QColor color() const;
    BorderRadius radius() const;

private:
    qreal m_thickness = 0;
    QColor m_color = Qt::transparent;
    BorderRadius m_radius;
};

/**
 * \brief Decoration state.
 *
 * The DecorationState type represents double bufferred state associated with a decoration.
 *
 * \note Sub-classes of DecorationState must override the clone() function.
 */
class KDECORATIONS3_EXPORT DecorationState
{
public:
    DecorationState();
    DecorationState(const DecorationState &other);
    virtual ~DecorationState();

    virtual std::shared_ptr<DecorationState> clone() const;

    QMarginsF borders() const;
    void setBorders(const QMarginsF &margins);

    BorderRadius borderRadius() const;
    void setBorderRadius(const BorderRadius &radius);

    BorderOutline borderOutline() const;
    void setBorderOutline(const BorderOutline &outline);

private:
    QSharedDataPointer<DecorationStateData> d;
};

/**
 * \brief Popup positioner.
 *
 * The Positioner type provides a way to position a popup relative to the decoration.
 */
class KDECORATIONS3_EXPORT Positioner
{
public:
    Positioner();
    Positioner(const Positioner &other);
    ~Positioner();

    /**
     * Returns the anchor rectangle. The anchor rectangle is specified relative to the top left
     * corner of the decoration.
     */
    QRectF anchorRect() const;

    /**
     * Sets the anchor rectangle to \a rect.
     */
    void setAnchorRect(const QRectF &rect);

private:
    QSharedDataPointer<PositionerData> d;
};

/**
 * @brief Base class for the Decoration.
 *
 * To provide a Decoration one needs to inherit from this class. The framework will instantiate
 * an instance of the inherited class for each DecoratedWindow.
 *
 * The main tasks of the Decoration is to provide borders around the DecoratedWindow. For this
 * the Deocration provides border sizes: those should be adjusted depending on the state of the
 * DecoratedWindow. E.g. commonly a maximized DecoratedWindow does not have borders on the side,
 * only the title bar.
 *
 * Whenever the visual representation of the Decoration changes the slot @link Decoration::update @endlink
 * should be invoked to request a repaint. The framework will in return invoke the
 * @link Decoration::paint @endlink method. This method needs to be implemented by inheriting
 * classes.
 *
 * A Decoration commonly provides buttons for interaction. E.g. a close button to close the
 * DecoratedWindow. For such actions the Decoration provides slots which should be connected to
 * the clicked signals of the buttons. For convenience the framework provides the @link DecorationButton @endlink
 * and the @link DecorationButtonGroup @endlink for easier layout. It is not required to use those,
 * if one uses different ways to represent the actions one needs to filter the events accordingly.
 *
 * @see DecoratedWindow
 * @see DecorationButton
 * @see DecorationButtonGroup
 **/
class KDECORATIONS3_EXPORT Decoration : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QMarginsF borders READ borders NOTIFY bordersChanged)
    Q_PROPERTY(qreal borderLeft READ borderLeft NOTIFY bordersChanged)
    Q_PROPERTY(qreal borderRight READ borderRight NOTIFY bordersChanged)
    Q_PROPERTY(qreal borderTop READ borderTop NOTIFY bordersChanged)
    Q_PROPERTY(qreal borderBottom READ borderBottom NOTIFY bordersChanged)
    Q_PROPERTY(QMarginsF resizeOnlyBorders READ resizeOnlyBorders NOTIFY resizeOnlyBordersChanged)
    Q_PROPERTY(qreal resizeOnlyBorderLeft READ resizeOnlyBorderLeft NOTIFY resizeOnlyBordersChanged)
    Q_PROPERTY(qreal resizeOnlyBorderRight READ resizeOnlyBorderRight NOTIFY resizeOnlyBordersChanged)
    Q_PROPERTY(qreal resizeOnlyBorderTop READ resizeOnlyBorderTop NOTIFY resizeOnlyBordersChanged)
    Q_PROPERTY(qreal resizeOnlyBorderBottom READ resizeOnlyBorderBottom NOTIFY resizeOnlyBordersChanged)
    /**
     * This property denotes the part of the Decoration which is currently under the mouse pointer.
     * It gets automatically updated whenever a QMouseEvent or QHoverEvent gets processed.
     **/
    Q_PROPERTY(Qt::WindowFrameSection sectionUnderMouse READ sectionUnderMouse NOTIFY sectionUnderMouseChanged)
    /**
     * The titleBar is the area inside the Decoration containing all controls (e.g. Buttons)
     * and the caption. The titleBar is the main interaction area, while all other areas of the
     * Decoration are normally used as resize areas.
     **/
    Q_PROPERTY(QRectF titleBar READ titleBar NOTIFY titleBarChanged)
    /**
     * Whether the Decoration is fully opaque. By default a Decoration is considered to
     * use the alpha channel and this property has the value @c false. But for e.g. a maximized
     * DecoratedWindow it is possible that the Decoration is fully opaque. In this case the
     * Decoration should set this property to @c true.
     **/
    Q_PROPERTY(bool opaque READ isOpaque NOTIFY opaqueChanged)
public:
    ~Decoration() override;

    /**
     * The DecoratedWindow for this Decoration.
     **/
    DecoratedWindow *window() const;

    QMarginsF borders() const;
    qreal borderLeft() const;
    qreal borderRight() const;
    qreal borderTop() const;
    qreal borderBottom() const;
    QMarginsF resizeOnlyBorders() const;
    qreal resizeOnlyBorderLeft() const;
    qreal resizeOnlyBorderRight() const;
    qreal resizeOnlyBorderTop() const;
    qreal resizeOnlyBorderBottom() const;
    Qt::WindowFrameSection sectionUnderMouse() const;
    QRectF titleBar() const;
    bool isOpaque() const;

    /**
     * The decoration border radius specifies how much the corners of the decorated window
     * should be rounded. The border radius is specified in the logical pixels.
     */
    BorderRadius borderRadius() const;

    /**
     * The outline around the window.
     */
    BorderOutline borderOutline() const;

    /**
     * DecorationShadow for this Decoration. It is recommended that multiple Decorations share
     * the same DecorationShadow. E.g one DecorationShadow for all inactive Decorations and one
     * for the active Decoration.
     **/
    std::shared_ptr<DecorationShadow> shadow() const;

    /**
     * The decoration's geometry in local coordinates.
     *
     * Basically the size of the DecoratedWindow combined with the borders.
     **/
    QRectF rect() const;
    QSizeF size() const;

    /**
     * The decoration's blur region in local coordinates
     */
    QRegion blurRegion() const;

    /**
     * Invoked by the framework to set the Settings for this Decoration before
     * init is invoked.
     * @internal
     **/
    void setSettings(const std::shared_ptr<DecorationSettings> &settings);
    /**
     * @returns The DecorationSettings used for this Decoration.
     **/
    std::shared_ptr<DecorationSettings> settings() const;

    /**
     * Implement this method in inheriting classes to provide the rendering.
     *
     * The @p painter is set up to paint on an internal QPaintDevice. The painting is
     * implicitly double buffered.
     *
     * @param painter The painter which needs to be used for rendering
     * @param repaintArea The region which needs to be repainted.
     **/
    virtual void paint(QPainter *painter, const QRectF &repaintArea) = 0;

    bool event(QEvent *event) override;

    /**
     * \internal
     *
     * Allocates the resources associated with the decoration, for example state containers.
     *
     * \note This method gets invoked by the compositor before init(), the decoration implementation
     * must not call it.
     */
    void create();

    /**
     * \internal
     *
     * Make the specified \a state current.
     *
     * The decoration maintains a double-buffered state. If a double-buffered property needs
     * to be changed, the next state will be updated and the nextStateChanged() signal will be
     * emitted to notify the compositor about it.
     *
     * When the next state gets applied is subject to compositor policies. For example, the
     * compositor may apply the new state immediately, or it can synchronize double-buffered
     * decoration state with double-buffered toplevel state.
     *
     * \sa currentState(), nextState(), createState()
     */
    void apply(std::shared_ptr<DecorationState> state);

    /**
     * Returns the currently applied state.
     *
     * \sa apply()
     */
    std::shared_ptr<DecorationState> currentState() const;

    /**
     * Returns the next state, i.e. the state that the decoration implementation wants to be current.
     *
     * \sa apply()
     */
    std::shared_ptr<DecorationState> nextState() const;

    /**
     * Notifies the framework that the decoration state has changed. When the new state is applied
     * is subject to compositor policies. For example, the compositor may re-configure the window
     * and apply the new state when the window is repainted.
     */
    void setState(std::function<void(DecorationState *state)> callback);

    /**
     * Shows the given \a menu at the position specified by the \a positioner.
     *
     * If the menu is already shown, it will be re-positioned according to the \a positioner.
     */
    void popup(const Positioner &positioner, QMenu *menu);

public Q_SLOTS:
    void requestClose();
    void requestToggleMaximization(Qt::MouseButtons buttons);
    void requestMinimize();
    void requestContextHelp();
    void requestToggleOnAllDesktops();
    void requestToggleShade();
    void requestToggleKeepAbove();
    void requestToggleKeepBelow();
    void requestToggleExcludeFromCapture();

#if KDECORATIONS3_ENABLE_DEPRECATED_SINCE(5, 21)
    /**
     * @deprecated
     * @see requestShowWindowMenu(const QRect &rect)
     */
    KDECORATIONS3_DEPRECATED_VERSION(5, 21, "Use Decoration::requestShowWindowMenu(QRect)")
    void requestShowWindowMenu();
#endif

    /**
     * @param rect the location at which to show the window menu
     */
    void requestShowWindowMenu(const QRect &rect);
    void requestShowToolTip(const QString &text);
    void requestHideToolTip();

    void showApplicationMenu(int actionId);
    void requestShowApplicationMenu(const QRect &rect, int actionId);

    void update(const QRectF &rect);
    void update();

    /**
     * This method gets invoked from the framework once the Decoration is created and
     * completely setup.
     *
     * An inheriting class should override this method and perform all initialization in
     * this method instead of the constructor.
     *
     * @return true if initialization has been successful,
     * false otherwise (for example, a QML component could not be loaded)
     **/
    virtual bool init() = 0;

Q_SIGNALS:
    void blurRegionChanged();
    void bordersChanged();
    void resizeOnlyBordersChanged();
    void sectionUnderMouseChanged(Qt::WindowFrameSection);
    void titleBarChanged();
    void opaqueChanged(bool);
    void shadowChanged(const std::shared_ptr<DecorationShadow> &shadow);
    void damaged(const QRegion &region);
    void currentStateChanged(std::shared_ptr<DecorationState> state);
    void nextStateChanged(std::shared_ptr<DecorationState> state);
    void borderRadiusChanged();
    void borderOutlineChanged();

protected:
    /**
     * Constructor for the Decoration.
     *
     * The @p args are used by the decoration framework to pass meta information
     * to the Decoration. An inheriting class is supposed to pass the args to the
     * parent class.
     *
     * @param parent The parent of the Decoration
     * @param args Additional arguments passed in from the framework
     **/
    explicit Decoration(QObject *parent, const QVariantList &args);
    void setBorders(const QMarginsF &borders);
    void setResizeOnlyBorders(const QMarginsF &borders);
    void setBlurRegion(const QRegion &region);
    /**
     * An implementation has to invoke this method whenever the area
     * containing the controls and caption changes.
     * @param rect The new geometry of the titleBar in Decoration coordinates
     **/
    void setTitleBar(const QRectF &rect);
    void setOpaque(bool opaque);
    void setShadow(const std::shared_ptr<DecorationShadow> &shadow);
    void setBorderRadius(const BorderRadius &radius);
    void setBorderOutline(const BorderOutline &outline);

    virtual void hoverEnterEvent(QHoverEvent *event);
    virtual void hoverLeaveEvent(QHoverEvent *event);
    virtual void hoverMoveEvent(QHoverEvent *event);
    virtual void mouseMoveEvent(QMouseEvent *event);
    virtual void mousePressEvent(QMouseEvent *event);
    virtual void mouseReleaseEvent(QMouseEvent *event);
    virtual void wheelEvent(QWheelEvent *event);

    /**
     * Create a state container. The decoration implementation can override this method to attach
     * its own properties to the decoration state.
     *
     * The default implementation simply creates an instance of the DecorationState type.
     *
     * \sa currentState(), nextState()
     */
    virtual std::shared_ptr<DecorationState> createState();

private:
    friend class DecorationButton;
    class Private;
    std::unique_ptr<Private> d;
};

} // namespace

Q_DECLARE_METATYPE(KDecoration3::Decoration *)

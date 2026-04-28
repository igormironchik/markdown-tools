/*
    SPDX-FileCopyrightText: 2001, 2002, 2003 Joseph Wenninger <jowenn@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef _KMultitabbar_h_
#define _KMultitabbar_h_

#include <QPushButton>
#include <QString>
#include <memory>

#include <kwidgetsaddons_export.h>

class QPixmap;
class QMenu;
class QStyleOptionToolButton;

class KMultiTabBarInternal;

/*!
 * \class KMultiTabBar
 * \inmodule KWidgetsAddons
 *
 * \brief A Widget for horizontal and vertical tabs.
 * (Note that in Qt4, QTabBar can be vertical as well)
 *
 * It is possible to add normal buttons to the top/left
 * The handling if only one tab at a time or multiple tabs
 * should be raisable is left to the "user".
 *
 * \image kmultitabbar.png "KMultiTabBar Widget"
 */
class KWIDGETSADDONS_EXPORT KMultiTabBar : public QWidget
{
    Q_OBJECT

    /*!
     * \property KMultiTabBar::position
     */
    Q_PROPERTY(KMultiTabBarPosition position READ position WRITE setPosition)

    /*!
     * \property KMultiTabBar::style
     */
    Q_PROPERTY(KMultiTabBarStyle tabStyle READ tabStyle WRITE setStyle)
public:
    /*!
     * \value Left
     * \value Right
     * \value Top
     * \value Bottom
     */
    enum KMultiTabBarPosition {
        Left,
        Right,
        Top,
        Bottom
    };
    Q_ENUM(KMultiTabBarPosition)

    /*!
     * The list of available styles for KMultiTabBar
     *
     * \value VSNET Visual Studio .Net like, always shows icon, only show the text of active tabs
     * \value KDEV3ICON KDevelop 3 like, always shows the text and icons
     * \omitvalue STYLELAST
     */
    enum KMultiTabBarStyle {
        VSNET = 0,
        KDEV3ICON = 2,
        STYLELAST = 0xffff,
    };
    Q_ENUM(KMultiTabBarStyle)

    /*!
     * Create a KMultiTabBar with Left as KMultiTabBar position.
     *
     * \a parent The parent of the widget.
     * \since 5.24
     */
    explicit KMultiTabBar(QWidget *parent = nullptr);

    /*!
     *
     */
    explicit KMultiTabBar(KMultiTabBarPosition pos, QWidget *parent = nullptr);
    ~KMultiTabBar() override;

    /*!
     * append  a new button to the button area. The button can later on be accessed with button(ID)
     * eg for connecting signals to it
     *
     * \a icon a icon for the button
     *
     * \a id an arbitrary ID value. It will be emitted in the clicked signal for identifying the button
     *  if more than one button is connected to a signals.
     *
     * \a popup A popup menu which should be displayed if the button is clicked
     *
     * \a not_used_yet will be used for a popup text in the future
     *
     * \since 5.13
     */
    int appendButton(const QIcon &icon, int id = -1, QMenu *popup = nullptr, const QString &not_used_yet = QString());

    /*!
     * remove a button with the given ID
     */
    void removeButton(int id);

    /*!
     * append a new tab to the tab area. It can be accessed lateron with tabb(id);
     *
     * \a icon a icon for the tab
     *
     * \a id an arbitrary ID which can be used later on to identify the tab
     *
     * \a text if a mode with text is used it will be the tab text, otherwise a mouse over hint
     *
     * \since 5.13
     */
    int appendTab(const QIcon &icon, int id = -1, const QString &text = QString());

    /*!
     * remove a tab with a given ID
     */
    void removeTab(int id);
    /*!
     * set a tab to "raised"
     *
     * \a id The ID of the tab to manipulate
     *
     * \a state true == activated/raised, false == not active
     */
    void setTab(int id, bool state);
    /*!
     * return the state of a tab, identified by its ID
     */
    bool isTabRaised(int id) const;
    /*!
     * get a pointer to a button within the button area identified by its ID
     */
    class KMultiTabBarButton *button(int id) const;

    /*!
     * get a pointer to a tab within the tab area, identified by its ID
     */
    class KMultiTabBarTab *tab(int id) const;

    /*!
     * set the real position of the widget.
     *
     * \a pos if the mode is horizontal, only use top, bottom, if it is vertical use left or right
     */
    void setPosition(KMultiTabBarPosition pos);

    /*!
     * Returns the tabbar position.
     */
    KMultiTabBarPosition position() const;

    /*!
     * set the display style of the tabs
     */
    void setStyle(KMultiTabBarStyle style);

    /*!
     * Returns the display style of the tabs
     */
    KMultiTabBarStyle tabStyle() const;

protected:
    friend class KMultiTabBarButton;
    virtual void fontChange(const QFont &);
    void paintEvent(class QPaintEvent *) override;
    void updateSeparator();

private:
    std::unique_ptr<class KMultiTabBarPrivate> const d;
};

/*!
 * \class KMultiTabBarButton
 * \inmodule KWidgetsAddons
 * \inheaderfile KMultiTabBar
 *
 * \brief Use KMultiTabBar::appendButton to append a button, which creates a KMultiTabBarButton instance
 */
class KWIDGETSADDONS_EXPORT KMultiTabBarButton : public QPushButton
{
    Q_OBJECT
public:
    /*!
     *
     */
    int id() const;
    ~KMultiTabBarButton() override;

public Q_SLOTS:
    /*!
     *
     */
    void setText(const QString &text);

Q_SIGNALS:
    /*!
     * this is emitted if  the button is clicked
     *
     * \a id the ID identifying the button
     */
    void clicked(int id);
protected Q_SLOTS:
    /*!
     *
     */
    virtual void slotClicked();

protected:
    void hideEvent(class QHideEvent *) override;
    void showEvent(class QShowEvent *) override;
    void paintEvent(class QPaintEvent *) override;

    /*!
     * Should not be created directly. Use KMultiTabBar::appendButton
     */
    KMultiTabBarButton(const QIcon &icon, const QString &, int id, QWidget *parent);

private:
    friend class KMultiTabBar;

    int m_id;
    std::unique_ptr<class KMultiTabBarButtonPrivate> const d;
};

/*!
 * \class KMultiTabBarTab
 * \inmodule KWidgetsAddons
 * \inheaderfile KMultiTabBar
 *
 * \brief Use KMultiTabBar::appendTab to append a tab, which creates a KMultiTabBarTab instance.
 */
class KWIDGETSADDONS_EXPORT KMultiTabBarTab : public KMultiTabBarButton
{
    Q_OBJECT
public:
    ~KMultiTabBarTab() override;
    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

public Q_SLOTS:
    /*!
     * this is used internally, but can be used by the user, if (s)he wants to
     * It the according call of KMultiTabBar is invoked though this modifications will be overwritten
     */
    void setPosition(KMultiTabBar::KMultiTabBarPosition);

    /*!
     * this is used internally, but can be used by the user, if (s)he wants to
     * It the according call of KMultiTabBar is invoked though this modifications will be overwritten
     */
    void setStyle(KMultiTabBar::KMultiTabBarStyle);

    /*!
     * set the active state of the tab
     *
     * \a state true==active false==not active
     */
    void setState(bool state);

public:
    using KMultiTabBarButton::setIcon;

protected:
    void paintEvent(QPaintEvent *) override;

private:
    KMultiTabBar::KMultiTabBarPosition m_position;
    KMultiTabBar::KMultiTabBarStyle m_style;

    KWIDGETSADDONS_NO_EXPORT void computeMargins(int *hMargin, int *vMargin) const;
    KWIDGETSADDONS_NO_EXPORT QSize computeSizeHint(bool withText) const;
    KWIDGETSADDONS_NO_EXPORT bool shouldDrawText() const;
    KWIDGETSADDONS_NO_EXPORT bool isVertical() const;

    KWIDGETSADDONS_NO_EXPORT void initStyleOption(QStyleOptionToolButton *opt) const;

    friend class KMultiTabBarInternal;
    /*
     * This class should never be created except with the appendTab call of KMultiTabBar
     */
    KWIDGETSADDONS_NO_EXPORT
    KMultiTabBarTab(const QIcon &icon, const QString &, int id, QWidget *parent, KMultiTabBar::KMultiTabBarPosition pos, KMultiTabBar::KMultiTabBarStyle style);

    std::unique_ptr<class KMultiTabBarTabPrivate> const d;
};

#endif

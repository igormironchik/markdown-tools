/*
  SPDX-FileCopyrightText: 2000 Troll Tech AS
  SPDX-FileCopyrightText: 2003 Lubos Lunak <l.lunak@kde.org>
  SPDX-FileCopyrightText: Bradley T. Hughes <bhughes@trolltech.com>

  SPDX-License-Identifier: MIT
*/

#ifndef netwm_def_h
#define netwm_def_h
#include <QFlags>
#include <QRect>
#include <kwindowsystem_export.h>

/*!
  \class NETPoint
  \inmodule KWindowSystem
  \inheaderfile netwm_def.h
  \brief Simple point class for NET classes.

  This class is a convenience class defining a point x, y.  The existence of
  this class is to keep the implementation from being dependent on a
  separate framework/library.

  NETPoint is only used by the NET API. Usually QPoint is the
  appropriate class for representing a point.
*/
struct NETPoint {
    /*!
       Constructor to initialize this point to 0,0.
    */
    NETPoint()
        : x(0)
        , y(0)
    {
    }

    NETPoint(const QPoint &p)
        : x(p.x())
        , y(p.y())
    {
    }

    QPoint toPoint() const
    {
        return {x, y};
    }

    /*! \brief The point's X coordinate. */
    int x;

    /*! \brief The point's Y coordinate. */
    int y;
};

/*!
  \class NETSize
  \inmodule KWindowSystem
  \brief Simple size class for NET classes.

  This class is a convenience class defining a size width by height.  The
  existence of this class is to keep the implementation from being dependent
  on a separate framework/library.

  NETSize is only used by the NET API. Usually QSize is the
  appropriate class for representing a size.
*/
struct NETSize {
    /*!
       Constructor to initialize this size to 0x0
    */
    NETSize()
        : width(0)
        , height(0)
    {
    }

    NETSize(const QSize &size)
        : width(size.width())
        , height(size.height())
    {
    }

    QSize toSize() const
    {
        return {width, height};
    }
    /*! The size's width. */
    int width;
    /*! The size's height. */
    int height;
};

/*!
   \class NETRect
   \inmodule KWindowSystem
   \brief Simple rectangle class for NET classes.

   This class is a convenience class defining a rectangle as a point x,y with a
   size width by height.  The existence of this class is to keep the implementation
   from being dependent on a separate framework/library;

   NETRect is only used by the NET API. Usually QRect is the
   appropriate class for representing a rectangle.
*/
struct NETRect {
    NETRect()
    {
    }

    NETRect(const QRect &rect)
        : pos(rect.topLeft())
        , size(rect.size())
    {
    }

    QRect toRect() const
    {
        return QRect(pos.x, pos.y, size.width, size.height);
    }

    /*!
       \brief The position of the rectangle's top left corner.
       \sa NETPoint
    */
    NETPoint pos;

    /*!
       \brief The size of the rectangle.
       \sa NETSize
    */
    NETSize size;
};

/*!
   \class NETIcon
   \inmodule KWindowSystem
   \brief Simple icon class for NET classes.

   This class is a convenience class defining an icon of size width by height.
   The existence of this class is to keep the implementation from being
   dependent on a separate framework/library.

   NETIcon is only used by the NET API. Usually QIcon is the
   appropriate class for representing an icon.
*/
struct NETIcon {
    /*!
       \brief Constructor to initialize this icon to 0x0 with data=0.
    */
    NETIcon()
        : data(nullptr)
    {
    }

    /*!
       \brief The size of the icon.
       \sa NETSize
    */
    NETSize size;

    /*!
       \brief Image data for the icon.

       This is an array of 32bit packed CARDINAL ARGB with high byte being A,
       low byte being B. First two bytes are width, height.
       Data is in rows, left to right and top to bottom.
    */
    unsigned char *data;
};

/*!
   \class NETExtendedStrut
   \inmodule KWindowSystem
   \brief Partial strut class for NET classes.

   This class is a convenience class defining a strut with left, right, top and
   bottom border values, and ranges for them.  The existence of this class is to
   keep the implementation from being dependent on a separate framework/library.
   See the _NET_WM_STRUT_PARTIAL property in the NETWM spec.
*/
struct NETExtendedStrut {
    /*!
       \brief Constructor to initialize this struct to 0,0,0,0.
    */
    NETExtendedStrut()
        : left_width(0)
        , left_start(0)
        , left_end(0)
        , right_width(0)
        , right_start(0)
        , right_end(0)
        , top_width(0)
        , top_start(0)
        , top_end(0)
        , bottom_width(0)
        , bottom_start(0)
        , bottom_end(0)
    {
    }

    /*! \brief The width of the left border of the strut. */
    int left_width;
    /*! \brief The start of the left border of the strut. */
    int left_start;
    /*! \brief The end of the left border of the strut. */
    int left_end;

    /*! The width of the right border of the strut. */
    int right_width;
    /*! \brief The start of the right border of the strut. */
    int right_start;
    /*! \brief The end of the right border of the strut. */
    int right_end;

    /*! \brief The width of the top border of the strut. */
    int top_width;
    /*! \brief The start of the top border of the strut. */
    int top_start;
    /*! \brief The end of the top border of the strut. */
    int top_end;

    /*! \brief The width of the bottom border of the strut. */
    int bottom_width;
    /*! \brief The start of the bottom border of the strut. */
    int bottom_start;
    /*! \brief The end of the bottom border of the strut. */
    int bottom_end;
};

/*!
   \class NETStrut
   \inmodule KWindowSystem
   \deprecated use NETExtendedStrut

   \brief Simple strut class for NET classes.

   This class is a convenience class defining a strut with left, right, top and
   bottom border values.  The existence of this class is to keep the implementation
   from being dependent on a separate framework/library. See the _NET_WM_STRUT
   property in the NETWM spec.
*/
struct NETStrut {
    /*!
       \brief Constructor to initialize this struct to 0,0,0,0.
    */
    NETStrut()
        : left(0)
        , right(0)
        , top(0)
        , bottom(0)
    {
    }

    /*! \brief Left border of the strut. */
    int left;

    /*! \brief Right border of the strut. */
    int right;

    /*! \brief Top border of the strut. */
    int top;

    /*! \brief Bottom border of the strut. */
    int bottom;
};

/*!
   \class NETFullscreenMonitors
   \inmodule KWindowSystem
   \brief Simple multiple monitor topology class for NET classes.

   This class is a convenience class, defining a multiple monitor topology
   for fullscreen applications that wish to be present on more than one
   monitor/head. As per the _NET_WM_FULLSCREEN_MONITORS hint in the EWMH spec,
   this topology consists of 4 monitor indices such that the bounding rectangle
   is defined by the top edge of the top monitor, the bottom edge of the bottom
   monitor, the left edge of the left monitor, and the right edge of the right
   monitor. See the _NET_WM_FULLSCREEN_MONITORS hint in the EWMH spec.
*/
struct NETFullscreenMonitors {
    /*!
       \brief Constructor to initialize this struct to -1,0,0,0
       (an initialized, albeit invalid, topology).
    */
    NETFullscreenMonitors()
        : top(-1)
        , bottom(0)
        , left(0)
        , right(0)
    {
    }

    /*! \brief Monitor index whose top border defines the top edge of the topology. */
    int top;

    /*! \brief Monitor index whose bottom border defines the bottom edge of the topology. */
    int bottom;

    /*! \brief Monitor index whose left border defines the left edge of the topology. */
    int left;

    /*! \brief Monitor index whose right border defines the right edge of the topology. */
    int right;

    /*!
       \brief Convenience check to make sure that we don't return
       the initial (invalid) values.

       Note that we don't want to call this isValid() because we're not
       actually validating the monitor topology here, but merely that our initial
       values were overwritten at some point by real (non-negative) monitor indices.
    */
    bool isSet() const
    {
        return (top != -1);
    }
};

/*!
  \class NET
  \inmodule KWindowSystem
  \brief Base namespace class.

  The NET API is an implementation of the NET Window Manager Specification.

  This class is the base class for the NETRootInfo and NETWinInfo classes, which
  are used to retrieve and modify the properties of windows. To keep
  the namespace relatively clean, all enums are defined here.

  \sa https://specifications.freedesktop.org/wm-spec/wm-spec-latest.html
 */
class KWINDOWSYSTEM_EXPORT NET
{
public:
    /*!
       \enum NET::Role
       Application role.  This is used internally to determine how several action
       should be performed (if at all).
       \value Client
       Indicates that the application is a client application.
       \value WindowManager
       Indicates that the application is a window manager application.
    */
    enum Role {
        Client,
        WindowManager,
    };

    /*!
       \enum NET::WindowType
       Window type.
       \sa NET::WindowTypeMask
       \value Unknown
       Indicates that the window did not define a window type.
       \value Normal
       Indicates that this is a normal, top-level window.
       \value Desktop
       Indicates a desktop feature. This can include a single window
       containing desktop icons with the sam*e dimensions as the screen, allowing
       the desktop environment to have full control of the desktop, without the
       need for proxying root window clicks.
       \value Dock
       Indicates a dock or panel feature.
       \value Toolbar
       Indicates a toolbar window.
       \value Menu
       Indicates a pinnable (torn-off) menu window.
       \value Dialog
       Indicates that this is a dialog window.
       \value Override
       Non-standard. Deprecated: has unclear meaning and is KDE-only.
       \value TopMenu
       Non-standard. Indicates a toplevel menu (AKA macmenu). This is a KDE extension to the
       _NET_WM_WINDOW_TYPE mechanism.
       \value Utility
       Indicates a utility window.
       \value Splash
       Indicates that this window is a splash screen window.
       \value DropdownMenu
       Indicates a dropdown menu (from a menubar typically).
       \value PopupMenu
       Indicates a popup menu (a context menu typically).
       \value Tooltip
       Indicates a tooltip window.
       \value Notification
       Indicates a notification window.
       \value ComboBox
       Indicates that the window is a list for a combobox.
       \value DNDIcon
       Indicates a window that represents the dragged object during DND operation.
       \value [since 5.6] OnScreenDisplay
       Non-standard. Indicates an On Screen Display window (such as volume feedback).
       \value [since 5.58] CriticalNotification
       Non-standard. Indicates a critical notification (such as battery is running out).
       \value AppletPopup
       Non-standard. Indicates that this window is an applet.
    */
    enum WindowType {
        Unknown = -1,
        Normal = 0,
        Desktop = 1,
        Dock = 2,
        Toolbar = 3,
        Menu = 4,
        Dialog = 5,
        // cannot deprecate to compiler: used both by clients & manager, later needs to keep supporting it for now
        // KF6: remove
        Override = 6,
        TopMenu = 7,
        Utility = 8,
        Splash = 9,
        DropdownMenu = 10,
        PopupMenu = 11,
        Tooltip = 12,
        Notification = 13,
        ComboBox = 14,
        DNDIcon = 15,
        OnScreenDisplay = 16,
        CriticalNotification = 17,
        AppletPopup = 18,
    };

    /*!
       \enum NET::WindowTypeMask
       Values for WindowType when they should be OR'ed together, e.g.
       for the properties argument of the NETRootInfo constructor.
       \sa NET::WindowType
       \value NormalMask
       \value DesktopMask
       \value DockMask
       \value ToolbarMask
       \value MenuMask
       \value DialogMask
       \value OverrideMask
       \value TopMenuMask
       \value UtilityMask
       \value SplashMask
       \value DropdownMenuMask
       \value PopupMenuMask
       \value TooltipMask
       \value NotificationMask
       \value ComboBoxMask
       \value DNDIconMask
       \value [since 5.6] OnScreenDisplayMask Non-standard.
       \value [since 5.58] CriticalNotificationMask Non-standard.
       \value AppletPopupMask Non-standard.
       \value AllTypesMask All window types.
    */
    enum WindowTypeMask {
        NormalMask = 1u << 0,
        DesktopMask = 1u << 1,
        DockMask = 1u << 2,
        ToolbarMask = 1u << 3,
        MenuMask = 1u << 4,
        DialogMask = 1u << 5,
        OverrideMask = 1u << 6,
        TopMenuMask = 1u << 7,
        UtilityMask = 1u << 8,
        SplashMask = 1u << 9,
        DropdownMenuMask = 1u << 10,
        PopupMenuMask = 1u << 11,
        TooltipMask = 1u << 12,
        NotificationMask = 1u << 13,
        ComboBoxMask = 1u << 14,
        DNDIconMask = 1u << 15,
        OnScreenDisplayMask = 1u << 16,
        CriticalNotificationMask = 1u << 17,
        AppletPopupMask = 1u << 18,
        AllTypesMask = 0U - 1,
    };
    Q_DECLARE_FLAGS(WindowTypes, WindowTypeMask)

    /*!
     * Returns \c true if the given window \a type matches the \a mask given
     * using WindowTypeMask flags.
     */
    static bool typeMatchesMask(WindowType type, WindowTypes mask);

    /*!
       \enum NET::State
       Window state.

       To set the state of a window, you'll typically do something like:
       \code
         KX11Extras::setState( winId(), NET::SkipTaskbar | NET::SkipPager | NET::SkipSwitcher );
       \endcode

       for example to not show the window on the taskbar, desktop pager, or window switcher.
       winId() is a function of QWidget()

       Note that KeepAbove (StaysOnTop) and KeepBelow are meant as user preference and
       applications should avoid setting these states themselves.

       \value Modal
       Indicates that this is a modal dialog box. The WM_TRANSIENT_FOR hint
       MUST be set to indicate which window the dialog is a modal for, or set to
       the root window if the dialog is a modal for its window group.
       \value Sticky
       Indicates that the Window Manager SHOULD keep the window's position
       fixed on the screen, even when the virtual desktop scrolls. Note that this is
       different from being kept on all desktops.
       \value MaxVert
       Indicates that the window is vertically maximized.
       \value MaxHoriz
       Indicates that the window is horizontally maximized.
       \value Max
       convenience value. Equal to MaxVert | MaxHoriz.
       \value Shaded
       Indicates that the window is shaded (rolled-up).
       \value SkipTaskbar
       Indicates that a window should not be included on a taskbar.
       \value KeepAbove
       Indicates that a window should on top of most windows (but below fullscreen
       windows).
       \value SkipPager
       Indicates that a window should not be included on a pager.
       \value Hidden
       Indicates that a window should not be visible on the screen (e.g. when minimised).
       Only the window manager is allowed to change it.
       \value FullScreen
       Indicates that a window should fill the entire screen and have no window
       decorations.
       \value KeepBelow
       Indicates that a window should be below most windows (but above any desktop windows).
       \value DemandsAttention
       there was an attempt to activate this window, but the window manager prevented
       this. E.g. taskbar should mark such window specially to bring user's attention to
       this window. Only the window manager is allowed to change it.
       \value [since 5.45] SkipSwitcher
       Indicates that a window should not be included on a switcher.
       \value [since 5.58] Focused
       Indicates that a client should render as though it has focus
       Only the window manager is allowed to change it.
    */
    enum State {
        Modal = 1u << 0,
        Sticky = 1u << 1,
        MaxVert = 1u << 2,
        MaxHoriz = 1u << 3,
        Max = MaxVert | MaxHoriz,
        Shaded = 1u << 4,
        SkipTaskbar = 1u << 5,
        KeepAbove = 1u << 6,
        SkipPager = 1u << 7,
        Hidden = 1u << 8,
        FullScreen = 1u << 9,
        KeepBelow = 1u << 10,
        DemandsAttention = 1u << 11,
        SkipSwitcher = 1u << 12,
        Focused = 1u << 13,
    };
    Q_DECLARE_FLAGS(States, State)

    /*!
       \enum NET::Direction
       Direction for WMMoveResize.

       When a client wants the Window Manager to start a WMMoveResize, it should
       specify one of:

       \value TopLeft
       \value Top
       \value TopRight
       \value Right
       \value BottomRight
       \value Bottom
       \value BottomLeft
       \value Left
       \value Move For movement only.
       \value KeyboardSize Resizing via keyboard.
       \value KeyboardMove Movement via keyboard.
       \value MoveResizeCancel To ask the WM to stop moving a window.
    */
    enum Direction {
        TopLeft = 0,
        Top = 1,
        TopRight = 2,
        Right = 3,
        BottomRight = 4,
        Bottom = 5,
        BottomLeft = 6,
        Left = 7,
        Move = 8,
        KeyboardSize = 9,
        KeyboardMove = 10,
        MoveResizeCancel = 11,
    };

    /*!
       \enum NET::MappingState
       Client window mapping state.  The class automatically watches the mapping
       state of the client windows, and uses the mapping state to determine how
       to set/change different properties. Note that this is very lowlevel
       and you most probably don't want to use this state.
       \value Visible
       Indicates the client window is visible to the user.
       \value Withdrawn
       Indicates that neither the client window nor its icon is visible.
       \value Iconic
       Indicates that the client window is not visible, but its icon is.
       This can be when the window is minimized or when it's on a
       different virtual desktop. See also NET::Hidden.
    */
    enum MappingState {
        Visible = 1, // NormalState,
        Withdrawn = 0, // WithdrawnState,
        Iconic = 3, // IconicState
    };

    /*!
      \enum NET::Action
      Actions that can be done with a window (_NET_WM_ALLOWED_ACTIONS).
      \value ActionMove
      \value ActionResize
      \value ActionMinimize
      \value ActionShade
      \value ActionStick
      \value ActionMaxVert
      \value ActionMaxHoriz
      \value ActionMax
      \value ActionFullScreen
      \value ActionChangeDesktop
      \value ActionClose
    */
    enum Action {
        ActionMove = 1u << 0,
        ActionResize = 1u << 1,
        ActionMinimize = 1u << 2,
        ActionShade = 1u << 3,
        ActionStick = 1u << 4,
        ActionMaxVert = 1u << 5,
        ActionMaxHoriz = 1u << 6,
        ActionMax = ActionMaxVert | ActionMaxHoriz,
        ActionFullScreen = 1u << 7,
        ActionChangeDesktop = 1u << 8,
        ActionClose = 1u << 9,
    };
    Q_DECLARE_FLAGS(Actions, Action)

    /*!
       \enum NET::Property
       Supported properties.  Clients and Window Managers must define which
       properties/protocols it wants to support.

       \value WMAllProperties
       Root/Desktop window properties and protocols:

       \value Supported
       \value ClientList
       \value ClientListStacking
       \value NumberOfDesktops
       \value DesktopGeometry
       \value DesktopViewport
       \value CurrentDesktop
       \value DesktopNames
       \value ActiveWindow
       \value WorkArea
       \value SupportingWMCheck
       \value VirtualRoots
       \value CloseWindow
       \value WMMoveResize

       Client window properties and protocols:

       \value WMName
       \value WMVisibleName
       \value WMDesktop
       \value WMWindowType
       \value WMState
       \value WMStrut Obsoleted by WM2ExtendedStrut.
       \value WMGeometry
       \value WMFrameExtents
       \value WMIconGeometry
       \value WMIcon
       \value WMIconName
       \value WMVisibleIconName
       \value WMHandledIcons
       \value WMPid
       \value WMPing

       ICCCM properties (provided for convenience):

       \value XAWMState
    */
    enum Property {
        // root
        Supported = 1u << 0,
        ClientList = 1u << 1,
        ClientListStacking = 1u << 2,
        NumberOfDesktops = 1u << 3,
        DesktopGeometry = 1u << 4,
        DesktopViewport = 1u << 5,
        CurrentDesktop = 1u << 6,
        DesktopNames = 1u << 7,
        ActiveWindow = 1u << 8,
        WorkArea = 1u << 9,
        SupportingWMCheck = 1u << 10,
        VirtualRoots = 1u << 11,
        //
        CloseWindow = 1u << 13,
        WMMoveResize = 1u << 14,

        // window
        WMName = 1u << 15,
        WMVisibleName = 1u << 16,
        WMDesktop = 1u << 17,
        WMWindowType = 1u << 18,
        WMState = 1u << 19,
        WMStrut = 1u << 20,
        WMIconGeometry = 1u << 21,
        WMIcon = 1u << 22,
        WMPid = 1u << 23,
        WMHandledIcons = 1u << 24,
        WMPing = 1u << 25,
        XAWMState = 1u << 27,
        WMFrameExtents = 1u << 28,

        // Need to be reordered
        WMIconName = 1u << 29,
        WMVisibleIconName = 1u << 30,
        WMGeometry = 1u << 31,
        WMAllProperties = ~0u,
    };
    Q_DECLARE_FLAGS(Properties, Property)

    /*!
        \enum NET::Property2
        Supported properties. This enum is an extension to NET::Property,
        because them enum is limited only to 32 bits.

        Client window properties and protocols:

        \value WM2UserTime
        \value WM2StartupId
        \value WM2TransientFor Main window for the window (WM_TRANSIENT_FOR).
        \value WM2GroupLeader  Group leader (window_group in WM_HINTS).
        \value WM2AllowedActions
        \value WM2RestackWindow
        \value WM2MoveResizeWindow
        \value WM2ExtendedStrut
        \value WM2KDETemporaryRules Non-standard.
        \value WM2WindowClass  WM_CLASS
        \value WM2WindowRole   WM_WINDOW_ROLE
        \value WM2ClientMachine WM_CLIENT_MACHINE
        \value WM2ShowingDesktop
        \value WM2Opacity _NET_WM_WINDOW_OPACITY
        \value WM2DesktopLayout _NET_DESKTOP_LAYOUT
        \value WM2FullPlacement _NET_WM_FULL_PLACEMENT
        \value WM2FullscreenMonitors _NET_WM_FULLSCREEN_MONITORS
        \value WM2FrameOverlap Non-standard.
        \value [since 4.6] WM2Activities Non-standard.
        \value [since 4.7] WM2BlockCompositing Standard since 5.17.
        \value [since 4.7] WM2KDEShadow Non-standard.
        \value [since 5.3] WM2Urgency Urgency hint in WM_HINTS (see ICCCM 4.1.2.4).
        \value [since 5.3] WM2Input Input hint (input in WM_HINTS, see ICCCM 4.1.2.4).
        \value [since 5.3] WM2Protocols See NET::Protocol.
        \value [since 5.5] WM2InitialMappingState Initial state hint of WM_HINTS (see ICCCM 4.1.2.4).
        \value [since 5.7] WM2IconPixmap Icon pixmap and mask in WM_HINTS (see ICCCM 4.1.2.4).
        \value [since 5.7] WM2OpaqueRegion
        \value [since 5.28] WM2DesktopFileName Non-standard. The base name of the desktop file name or the full path to the desktop file.
        \value [since 5.65] WM2GTKFrameExtents Non-standard. Extents of the shadow drawn by the client.
        \value [since 5.69] WM2AppMenuServiceName. Non-standard.
        \value [since 5.69] WM2AppMenuObjectPath. Non-standard.
        \value [since 5.91] WM2GTKApplicationId Non-standard. _GTK_APPLICATION_ID
        \value [since 5.96] WM2GTKShowWindowMenu Non-standard. _GTK_SHOW_WINDOW_MENU
        \value WM2AllProperties
    */
    enum Property2 {
        WM2UserTime = 1u << 0,
        WM2StartupId = 1u << 1,
        WM2TransientFor = 1u << 2,
        WM2GroupLeader = 1u << 3,
        WM2AllowedActions = 1u << 4,
        WM2RestackWindow = 1u << 5,
        WM2MoveResizeWindow = 1u << 6,
        WM2ExtendedStrut = 1u << 7,
        WM2KDETemporaryRules = 1u << 8,
        WM2WindowClass = 1u << 9,
        WM2WindowRole = 1u << 10,
        WM2ClientMachine = 1u << 11,
        WM2ShowingDesktop = 1u << 12,
        WM2Opacity = 1u << 13,
        WM2DesktopLayout = 1u << 14,
        WM2FullPlacement = 1u << 15,
        WM2FullscreenMonitors = 1u << 16,
        WM2FrameOverlap = 1u << 17,
        WM2Activities = 1u << 18,
        WM2BlockCompositing = 1u << 19,
        WM2KDEShadow = 1u << 20,
        WM2Urgency = 1u << 21,
        WM2Input = 1u << 22,
        WM2Protocols = 1u << 23,
        WM2InitialMappingState = 1u << 24,
        WM2IconPixmap = 1u << 25,
        WM2OpaqueRegion = 1u << 25,
        WM2DesktopFileName = 1u << 26,
        WM2GTKFrameExtents = 1u << 27,
        WM2AppMenuServiceName = 1u << 28,
        WM2AppMenuObjectPath = 1u << 29,
        WM2GTKApplicationId = 1u << 30,
        WM2GTKShowWindowMenu = 1u << 31,
        WM2AllProperties = ~0u,
    };
    Q_DECLARE_FLAGS(Properties2, Property2)

    /*
       Sentinel value to indicate that the client wishes to be visible on
       all desktops.
     */
    enum {
        OnAllDesktops = -1,
    };

    // must match the values for data.l[0] field in _NET_ACTIVE_WINDOW message
    /*!
       \enum NET::RequestSource
       Source of the request.
       \value FromUnknown
       Indicates that the source of the request is unknown.
       \value FromApplication
       Indicates that the request comes from a normal application.
       \value FromTool
       Indicated that the request comes from pager or similar tool.
    */
    enum RequestSource {
        FromUnknown = 0,
        FromApplication = 1,
        FromTool = 2,
    };

    /*!
      \enum NET::Orientation
      Orientation.
      \value OrientationHorizontal
      \value OrientationVertical
    */
    enum Orientation {
        OrientationHorizontal = 0,
        OrientationVertical = 1,
    };

    /*!
     \enum NET::DesktopLayoutCorner
     Starting corner for desktop layout.
     \value DesktopLayoutCornerTopLeft
     \value DesktopLayoutCornerTopRight
     \value DesktopLayoutCornerBottomLeft
     \value DesktopLayoutCornerBottomRight
    */
    enum DesktopLayoutCorner {
        DesktopLayoutCornerTopLeft = 0,
        DesktopLayoutCornerTopRight = 1,
        DesktopLayoutCornerBottomLeft = 2,
        DesktopLayoutCornerBottomRight = 3,
    };

    /*!
     * \enum NET::Protocol
     * Protocols supported by the client.
     * See ICCCM 4.1.2.7.
     *
     * \since 5.3
     * \value NoProtocol
     * \value TakeFocusProtocol WM_TAKE_FOCUS
     * \value DeleteWindowProtocol WM_DELETE_WINDOW
     * \value PingProtocol _NET_WM_PING from EWMH
     * \value SyncRequestProtocol _NET_WM_SYNC_REQUEST from EWMH
     * \value ContextHelpProtocol Non-standard. _NET_WM_CONTEXT_HELP
     */
    enum Protocol {
        NoProtocol = 0,
        TakeFocusProtocol = 1 << 0,
        DeleteWindowProtocol = 1 << 1,
        PingProtocol = 1 << 2,
        SyncRequestProtocol = 1 << 3,
        ContextHelpProtocol = 1 << 4,
    };
    Q_DECLARE_FLAGS(Protocols, Protocol)

    /*!
     \brief Compares two X timestamps, taking into account wrapping and 64bit architectures.
     Return value is like with strcmp(), 0 for equal, -1 for \a time1 < \a time2, 1 for \a time1 > \a time2.
    */
    static int timestampCompare(unsigned long time1, unsigned long time2);
    /*!
     \brief Returns a difference of two X timestamps, \a time2 - \a time1, where \a time2 must be later than \a time1,
     as returned by timestampCompare().
    */
    static int timestampDiff(unsigned long time1, unsigned long time2);
};

Q_DECLARE_OPERATORS_FOR_FLAGS(NET::Properties)
Q_DECLARE_OPERATORS_FOR_FLAGS(NET::Properties2)
Q_DECLARE_OPERATORS_FOR_FLAGS(NET::WindowTypes)
Q_DECLARE_OPERATORS_FOR_FLAGS(NET::States)
Q_DECLARE_OPERATORS_FOR_FLAGS(NET::Actions)
Q_DECLARE_OPERATORS_FOR_FLAGS(NET::Protocols)

#endif // netwm_def_h

/*
    SPDX-FileCopyrightText: 2000 Troll Tech AS
    SPDX-FileCopyrightText: 2003 Lubos Lunak <l.lunak@kde.org>

    SPDX-License-Identifier: MIT
*/

#ifndef netwm_h
#define netwm_h

#include <QSize>
#include <config-kwindowsystem.h>
#include <kwindowsystem_export.h>
#if KWINDOWSYSTEM_HAVE_X11
#include <vector>
#include <xcb/xcb.h>

#include "netwm_def.h"

#define KDE_ALL_ACTIVITIES_UUID "00000000-0000-0000-0000-000000000000"

// forward declaration
struct NETRootInfoPrivate;
struct NETWinInfoPrivate;
template<class Z>
class NETRArray;

/*!
   \class NETRootInfo
   \inheaderfile NETWM
   \inmodule KWindowSystem
   \brief Common API for root window properties/protocols.

   The NETRootInfo class provides a common API for clients and window managers
   to set/read/change properties on the root window as defined by the NET Window
   Manager Specification..

   \sa NET
   \sa NETWinInfo
 **/

class KWINDOWSYSTEM_EXPORT NETRootInfo : public NET
{
public:
    // update also NETRootInfoPrivate::properties[] size when extending this
    /*!
        Indexes for the properties array.
        \value PROTOCOLS
        \value WINDOW_TYPES
        \value STATES
        \value PROTOCOLS2
        \value ACTIONS
        \value PROPERTIES_SIZE
    **/
    enum {
        PROTOCOLS,
        WINDOW_TYPES,
        STATES,
        PROTOCOLS2,
        ACTIONS,
        PROPERTIES_SIZE,
    };

    /*!
       Window Managers should use this constructor to create a NETRootInfo object,
       which will be used to set/update information stored on the rootWindow.
       The application role is automatically set to WindowManager
       when using this constructor.

       \a connection XCB connection

       \a supportWindow The Window id of the supportWindow.  The supportWindow
       must be created by the window manager as a child of the rootWindow.  The
       supportWindow must not be destroyed until the Window Manager exits.

       \a wmName A string which should be the window manager's name (ie. "KWin"
       or "Blackbox").

       \a properties The properties the window manager supports

       \a windowTypes The window types the window manager supports

       \a states The states the window manager supports

       \a properties2 The properties2 the window manager supports

       \a actions The actions the window manager supports

       \a screen For Window Managers that support multiple screen (ie.
       "multiheaded") displays, the screen number may be explicitly defined.  If
       this argument is omitted, the default screen will be used.

       \a doActivate true to activate the window
    **/
    NETRootInfo(xcb_connection_t *connection,
                xcb_window_t supportWindow,
                const char *wmName,
                NET::Properties properties,
                NET::WindowTypes windowTypes,
                NET::States states,
                NET::Properties2 properties2,
                NET::Actions actions,
                int screen = -1,
                bool doActivate = true);

    /*!
       Clients should use this constructor to create a NETRootInfo object, which
       will be used to query information set on the root window. The application
       role is automatically set to Client when using this constructor.

       \a connection XCB connection

       \a properties The properties the client is interested in.

       \a properties2 The properties2 the client is interested in.

       \a properties_size The number of elements in the properties array.

       \a screen For Clients that support multiple screen (ie. "multiheaded")
       displays, the screen number may be explicitly defined. If this argument is
       omitted, the default screen will be used.

       \a doActivate true to call activate() to do an initial data read/update
       of the query information.
    **/
    NETRootInfo(xcb_connection_t *connection,
                NET::Properties properties,
                NET::Properties2 properties2 = NET::Properties2(),
                int screen = -1,
                bool doActivate = true);

    /*!
       Creates a shared copy of the specified NETRootInfo object.

       \a rootinfo the NETRootInfo object to copy
    **/
    NETRootInfo(const NETRootInfo &rootinfo);

    virtual ~NETRootInfo();

    /*!
       Returns the xcb connection used.
    **/
    xcb_connection_t *xcbConnection() const;

    /*!
       Returns the Window id of the rootWindow.
    **/
    xcb_window_t rootWindow() const;

    /*!
       Returns the Window id of the supportWindow.
    **/
    xcb_window_t supportWindow() const;

    /*!
       Returns the name of the Window Manager.
    **/
    const char *wmName() const;

    /*!
      Sets the given property if on is true, and clears the property otherwise.
      In WindowManager mode this function updates _NET_SUPPORTED.
      In Client mode this function does nothing.

      \since 4.4
     **/
    void setSupported(NET::Property property, bool on = true);

    /*!
      \overload
      \since 4.4
     **/
    void setSupported(NET::Property2 property, bool on = true);

    /*!
      \overload
      \since 4.4
     **/
    void setSupported(NET::WindowTypeMask property, bool on = true);

    /*!
      \overload
      \since 4.4
     **/
    void setSupported(NET::State property, bool on = true);

    /*!
      \overload
      \since 4.4
     **/
    void setSupported(NET::Action property, bool on = true);

    /*!
       Returns true if the given property is supported by the window
       manager. Note that for Client mode, NET::Supported needs
       to be passed in the properties argument for this to work.
    **/
    bool isSupported(NET::Property property) const;
    /*!
       \overload
    **/
    bool isSupported(NET::Property2 property) const;
    /*!
       \overload
    **/
    bool isSupported(NET::WindowTypeMask type) const;
    /*!
       \overload
    **/
    bool isSupported(NET::State state) const;

    /*!
       \overload
    **/
    bool isSupported(NET::Action action) const;

    /*!
       In the Window Manager mode, this is equivalent to the properties
       argument passed to the constructor. In the Client mode, if
       NET::Supported was passed in the properties argument, the returned
       value are all properties supported by the Window Manager. Other supported
       protocols and properties are returned by the specific methods.
       \sa supportedProperties2()
       \sa supportedStates()
       \sa supportedWindowTypes()
       \sa supportedActions()
    **/
    NET::Properties supportedProperties() const;
    /*!
     * In the Window Manager mode, this is equivalent to the properties2
     * argument passed to the constructor. In the Client mode, if
     * NET::Supported was passed in the properties argument, the returned
     * value are all properties2 supported by the Window Manager. Other supported
     * protocols and properties are returned by the specific methods.
     * \sa supportedProperties()
     * \sa supportedStates()
     * \sa supportedWindowTypes()
     * \sa supportedActions()
     * \since 5.0
     **/
    NET::Properties2 supportedProperties2() const;
    /*!
     * In the Window Manager mode, this is equivalent to the states
     * argument passed to the constructor. In the Client mode, if
     * NET::Supported was passed in the properties argument, the returned
     * value are all states supported by the Window Manager. Other supported
     * protocols and properties are returned by the specific methods.
     * \sa supportedProperties()
       \sa supportedProperties2()
     * \sa supportedWindowTypes()
     * \sa supportedActions()
     * \since 5.0
     **/
    NET::States supportedStates() const;
    /*!
     * In the Window Manager mode, this is equivalent to the windowTypes
     * argument passed to the constructor. In the Client mode, if
     * NET::Supported was passed in the properties argument, the returned
     * value are all window types supported by the Window Manager. Other supported
     * protocols and properties are returned by the specific methods.
     * \sa supportedProperties()
       \sa supportedProperties2()
     * \sa supportedStates()
     * \sa supportedActions()
     * \since 5.0
     **/
    NET::WindowTypes supportedWindowTypes() const;
    /*!
     * In the Window Manager mode, this is equivalent to the actions
     * argument passed to the constructor. In the Client mode, if
     * NET::Supported was passed in the properties argument, the returned
     * value are all actions supported by the Window Manager. Other supported
     * protocols and properties are returned by the specific methods.
     * \sa supportedProperties()
       \sa supportedProperties2()
     * \sa supportedStates()
     * \sa supportedWindowTypes()
     * \since 5.0
     **/
    NET::Actions supportedActions() const;

    /*!
     * Returns the properties argument passed to the constructor.
     * \sa passedProperties2()
     * \sa passedStates()
     * \sa passedWindowTypes()
     * \sa passedActions()
     **/
    NET::Properties passedProperties() const;
    /*!
     * Returns the properties2 argument passed to the constructor.
     * \sa passedProperties()
     * \sa passedStates()
     * \sa passedWindowTypes()
     * \sa passedActions()
     * \since 5.0
     **/
    NET::Properties2 passedProperties2() const;
    /*!
     * Returns the states argument passed to the constructor.
     * \sa passedProperties()
     * \sa passedProperties2()
     * \sa passedWindowTypes()
     * \sa passedActions()
     * \since 5.0
     **/
    NET::States passedStates() const;
    /*!
     * Returns the windowTypes argument passed to the constructor.
     * \sa passedProperties()
     * \sa passedProperties2()
     * \sa passedStates()
     * \sa passedActions()
     * \since 5.0
     **/
    NET::WindowTypes passedWindowTypes() const;
    /*!
     * Returns the actions argument passed to the constructor.
     * \sa passedProperties()
     * \sa passedProperties2()
     * \sa passedStates()
     * \sa passedWindowTypes()
     * \since 5.0
     **/
    NET::Actions passedActions() const;

    /*!
       Returns an array of Window id's, which contain all managed windows.

       Returns the array of Window id's

       \sa clientListCount()
    **/
    const xcb_window_t *clientList() const;

    /*!
       Returns the number of managed windows in clientList array.

       Returns the number of managed windows in the clientList array

       \sa clientList()
    **/
    int clientListCount() const;

    /*!
       Returns an array of Window id's, which contain all managed windows in
       stacking order.

       Returns the array of Window id's in stacking order

       \sa clientListStackingCount()
    **/
    const xcb_window_t *clientListStacking() const;

    /*!
       Returns the number of managed windows in the clientListStacking array.

       Returns the number of Window id's in the client list

       \sa clientListStacking()
    **/
    int clientListStackingCount() const;

    /*!
       Returns the desktop geometry size.

       \note KDE uses virtual desktops and does not directly support
       viewport in any way. You should use calls for virtual desktops,
       viewport is mapped to them if needed.

       Returns the size of the desktop
    **/
    NETSize desktopGeometry() const;

    /*!
       Returns the viewport of the specified desktop.

       \note KDE uses virtual desktops and does not directly support
       viewport in any way. You should use calls for virtual desktops,
       viewport is mapped to them if needed.

       \a desktop the number of the desktop

       Returns the position of the desktop's viewport
    **/
    NETPoint desktopViewport(int desktop) const;

    /*!
       Returns the workArea for the specified desktop.

       \a desktop the number of the desktop

       Returns the size of the work area
    **/
    NETRect workArea(int desktop) const;

    /*!
       Returns the name for the specified desktop.

       \a desktop the number of the desktop

       Returns the name of the desktop
    **/
    const char *desktopName(int desktop) const;

    /*!
       Returns an array of Window id's, which contain the virtual root windows.

       Returns the array of Window id's

       \sa virtualRootsCount()
    **/
    const xcb_window_t *virtualRoots() const;

    /*!
       Returns the number of window in the virtualRoots array.

       Returns the number of Window id's in the virtual root array

       \sa virtualRoots()
    **/
    int virtualRootsCount() const;

    /*!
       Returns the desktop layout orientation.
    **/
    NET::Orientation desktopLayoutOrientation() const;

    /*!
       Returns the desktop layout number of columns and rows. Note that
       either may be 0 (see _NET_DESKTOP_LAYOUT).
    **/
    QSize desktopLayoutColumnsRows() const;

    /*!
       Returns the desktop layout starting corner.
    **/
    NET::DesktopLayoutCorner desktopLayoutCorner() const;

    /*!
       Returns the number of desktops.

       \note KDE uses virtual desktops and does not directly support
       viewport in any way. They are however mapped to virtual desktops
       if needed.

       \a ignore_viewport if false, viewport is mapped to virtual desktops

       Returns the number of desktops
    **/
    int numberOfDesktops(bool ignore_viewport = false) const;

    /*!
       Returns the current desktop.

       \note KDE uses virtual desktops and does not directly support
       viewport in any way. They are however mapped to virtual desktops
       if needed.

       \a ignore_viewport if false, viewport is mapped to virtual desktops

       Returns the number of the current desktop
    **/
    int currentDesktop(bool ignore_viewport = false) const;

    /*!
       Returns the active (focused) window.

       Returns the id of the active window
    **/
    xcb_window_t activeWindow() const;

    /*!
       Window Managers must call this after creating the NETRootInfo object, and
       before using any other method in the class.  This method sets initial data
       on the root window and does other post-construction duties.

       Clients must also call this after creating the object to do an initial
       data read/update.
    **/
    void activate();

    /*!
       Sets the list of managed windows on the Root/Desktop window.

       \a windows The array of Window id's

       \a count The number of windows in the array
    **/
    void setClientList(const xcb_window_t *windows, unsigned int count);

    /*!
       Sets the list of managed windows in stacking order on the Root/Desktop
       window.

       \a windows The array of Window id's

       \a count The number of windows in the array.
    **/
    void setClientListStacking(const xcb_window_t *windows, unsigned int count);

    /*!
       Sets the current desktop to the specified desktop.

       \note KDE uses virtual desktops and does not directly support
       viewport in any way. It is however mapped to virtual desktops
       if needed.

       \a desktop the number of the desktop

       \a ignore_viewport if false, viewport is mapped to virtual desktops
    **/
    void setCurrentDesktop(int desktop, bool ignore_viewport = false);

    /*!
       Sets the desktop geometry to the specified geometry.

       \note KDE uses virtual desktops and does not directly support
       viewport in any way. You should use calls for virtual desktops,
       viewport is mapped to them if needed.

       \a geometry the new size of the desktop
    **/
    void setDesktopGeometry(const NETSize &geometry);

    /*!
       Sets the viewport for the current desktop to the specified point.

       \note KDE uses virtual desktops and does not directly support
       viewport in any way. You should use calls for virtual desktops,
       viewport is mapped to them if needed.

       \a desktop the number of the desktop

       \a viewport the new position of the desktop's viewport
    **/
    void setDesktopViewport(int desktop, const NETPoint &viewport);

    /*!
       Sets the number of desktops to the specified number.

       \note KDE uses virtual desktops and does not directly support
       viewport in any way. Viewport is mapped to virtual desktops
       if needed, but not for this call.

       \a numberOfDesktops the number of desktops
    **/
    void setNumberOfDesktops(int numberOfDesktops);

    /*!
       Sets the name of the specified desktop.

       \note KDE uses virtual desktops and does not directly support
       viewport in any way. Viewport is mapped to virtual desktops
       if needed, but not for this call.

       \a desktop the number of the desktop

       \a desktopName the new name of the desktop
    **/
    void setDesktopName(int desktop, const char *desktopName);

    /*!
       Requests that the specified window becomes the active (focused) one.

       \a window the id of the new active window

       \a src whether the request comes from normal application
          or from a pager or similar tool

       \a timestamp X server timestamp of the user action that
          caused the request

       \a active_window active window of the requesting application, if any
    **/
    void setActiveWindow(xcb_window_t window, NET::RequestSource src, xcb_timestamp_t timestamp, xcb_window_t active_window);

    /*!
       Sets the active (focused) window the specified window. This should
       be used only in the window manager mode.

       \a window the if of the new active window
    **/
    void setActiveWindow(xcb_window_t window);

    /*!
       Sets the workarea for the specified desktop

       \a desktop the number of the desktop

       \a workArea the new work area of the desktop
    **/
    void setWorkArea(int desktop, const NETRect &workArea);

    /*!
       Sets the list of virtual root windows on the root window.

       \a windows The array of Window id's

       \a count The number of windows in the array.
    **/
    void setVirtualRoots(const xcb_window_t *windows, unsigned int count);

    /*!
       Sets the desktop layout. This is set by the pager. When setting, the pager must
       own the _NET_DESKTOP_LAYOUT_Sn manager selection. See _NET_DESKTOP_LAYOUT for details.
    **/
    void setDesktopLayout(NET::Orientation orientation, int columns, int rows, NET::DesktopLayoutCorner corner);

    /*!
     * Sets the _NET_SHOWING_DESKTOP status (whether desktop is being shown).
     */
    void setShowingDesktop(bool showing);
    /*!
     * Returns the status of _NET_SHOWING_DESKTOP.
     */
    bool showingDesktop() const;

    /*!
       Assignment operator.  Ensures that the shared data reference counts are
       correct.
    **/
    const NETRootInfo &operator=(const NETRootInfo &rootinfo);

    /*!
       Clients (such as pagers/taskbars) that wish to close a window should call
       this function.  This will send a request to the Window Manager, which
       usually can usually decide how to react to such requests.

       \a window the id of the window to close
    **/
    void closeWindowRequest(xcb_window_t window);

    /*!
       Clients (such as pagers/taskbars) that wish to start a WMMoveResize
       (where the window manager controls the resize/movement,
       i.e. _NET_WM_MOVERESIZE) should call this function.
       This will send a request to the Window Manager.

       \a window The client window that would be resized/moved.

       \a x_root X position of the cursor relative to the root window.

       \a y_root Y position of the cursor relative to the root window.

       \a direction One of NET::Direction (see base class documentation for
       a description of the different directions).

       \a button the button which should be pressed.

       \a source who initiated the move resize operation.
    **/
    void moveResizeRequest(xcb_window_t window,
                           int x_root,
                           int y_root,
                           Direction direction,
                           xcb_button_t button = XCB_BUTTON_INDEX_ANY,
                           RequestSource source = RequestSource::FromUnknown);

    /*!
       Clients (such as pagers/taskbars) that wish to move/resize a window
       using WM2MoveResizeWindow (_NET_MOVERESIZE_WINDOW) should call this function.
       This will send a request to the Window Manager. See _NET_MOVERESIZE_WINDOW
       description for details.

       \a window The client window that would be resized/moved.

       \a flags Flags specifying the operation (see _NET_MOVERESIZE_WINDOW description)

       \a x Requested X position for the window

       \a y Requested Y position for the window

       \a width Requested width for the window

       \a height Requested height for the window
    **/
    void moveResizeWindowRequest(xcb_window_t window, int flags, int x, int y, int width, int height);

    /*!
       Clients that wish to show the window menu using WM2GTKShowWindowMenu
       (_GTK_SHOW_WINDOW_MENU) should call this function.
       This will send a request to the Window Manager. See _GTK_SHOW_WINDOW_MENU
       description for details.

       \a window The client window that would be resized/moved.

       \a device_id GTK device id

       \a x Requested X position for the menu relative to the root window

       \a y Requested Y position for the menu relative to the root window
    **/
    void showWindowMenuRequest(xcb_window_t window, int device_id, int x_root, int y_root);

    /*!
       Sends the _NET_RESTACK_WINDOW request.
    **/
    void restackRequest(xcb_window_t window, RequestSource source, xcb_window_t above, int detail, xcb_timestamp_t timestamp);

    /*!
      Sends a ping with the given timestamp to the window, using
      the _NET_WM_PING protocol.
    */
    void sendPing(xcb_window_t window, xcb_timestamp_t timestamp);

    /*!
     * This function takes the passed xcb_generic_event_t and returns the updated properties in the passed in arguments.
     *
     * The new information will be read immediately by the class. It is possible to pass in a
     * null pointer in the arguments. In that case the passed in argument will obviously not
     * be updated, but the class will process the information nevertheless.
     *
     * \a event the event
     *
     * \a properties The NET::Properties that changed
     *
     * \a properties2 The NET::Properties2 that changed
     * \since 5.0
     **/
    void event(xcb_generic_event_t *event, NET::Properties *properties, NET::Properties2 *properties2 = nullptr);

    /*!
       This function takes the passed XEvent and returns an OR'ed list of
       NETRootInfo properties that have changed.  The new information will be
       read immediately by the class. This overloaded version returns
       only a single mask, and therefore cannot check state of all properties
       like the other variant.

       \a event the event

       Returns the properties
    **/
    NET::Properties event(xcb_generic_event_t *event);

protected:
    /*!
       A Client should subclass NETRootInfo and reimplement this function when
       it wants to know when a window has been added.

       \a window the id of the window to add
    **/
    virtual void addClient(xcb_window_t window)
    {
        Q_UNUSED(window);
    }

    /*!
       A Client should subclass NETRootInfo and reimplement this function when
       it wants to know when a window has been removed.

       \a window the id of the window to remove
    **/
    virtual void removeClient(xcb_window_t window)
    {
        Q_UNUSED(window);
    }

    /*!
       A Window Manager should subclass NETRootInfo and reimplement this function
       when it wants to know when a Client made a request to change the number
       of desktops.

       \a numberOfDesktops the new number of desktops
    **/
    virtual void changeNumberOfDesktops(int numberOfDesktops)
    {
        Q_UNUSED(numberOfDesktops);
    }

    /*!
       A Window Manager should subclass NETRootInfo and reimplement this function
       when it wants to know when a Client made a request to change the specified
       desktop geometry.

       \a desktop the number of the desktop

       \a geom the new size
    **/
    virtual void changeDesktopGeometry(int desktop, const NETSize &geom)
    {
        Q_UNUSED(desktop);
        Q_UNUSED(geom);
    }

    /*!
       A Window Manager should subclass NETRootInfo and reimplement this function
       when it wants to know when a Client made a request to change the specified
       desktop viewport.

       \a desktop the number of the desktop

       \a viewport the new position of the viewport
    **/
    virtual void changeDesktopViewport(int desktop, const NETPoint &viewport)
    {
        Q_UNUSED(desktop);
        Q_UNUSED(viewport);
    }

    /*!
       A Window Manager should subclass NETRootInfo and reimplement this function
       when it wants to know when a Client made a request to change the current
       desktop.

       \a desktop the number of the desktop
    **/
    virtual void changeCurrentDesktop(int desktop)
    {
        Q_UNUSED(desktop);
    }

    /*!
       A Window Manager should subclass NETRootInfo and reimplement this function
       when it wants to know when a Client made a request to close a window.

       \a window the id of the window to close
    **/
    virtual void closeWindow(xcb_window_t window)
    {
        Q_UNUSED(window);
    }

    /*!
       A Window Manager should subclass NETRootInfo and reimplement this function
       when it wants to know when a Client made a request to start a move/resize.

       \a window The window that wants to move/resize

       \a x_root X position of the cursor relative to the root window.

       \a y_root Y position of the cursor relative to the root window.

       \a direction One of NET::Direction (see base class documentation for
       a description of the different directions).

       \a button the button which should be pressed.

       \a source who initiated the move resize operation.
    **/
    virtual void moveResize(xcb_window_t window, int x_root, int y_root, unsigned long direction, xcb_button_t button, RequestSource source)
    {
        Q_UNUSED(window);
        Q_UNUSED(x_root);
        Q_UNUSED(y_root);
        Q_UNUSED(direction);
        Q_UNUSED(button);
        Q_UNUSED(source);
    }

    /*!
       A Window Manager should subclass NETRootInfo and reimplement this function
       when it wants to receive replies to the _NET_WM_PING protocol.

       \a window the window from which the reply came

       \a timestamp timestamp of the ping
     */
    virtual void gotPing(xcb_window_t window, xcb_timestamp_t timestamp)
    {
        Q_UNUSED(window);
        Q_UNUSED(timestamp);
    }
    /*!
       A Window Manager should subclass NETRootInfo and reimplement this function
       when it wants to know when a Client made a request to change the active
       (focused) window.

       \a window the id of the window to activate

       \a src the source from which the request came

       \a timestamp the timestamp of the user action causing this request

       \a active_window active window of the requesting application, if any
    **/
    virtual void changeActiveWindow(xcb_window_t window, NET::RequestSource src, xcb_timestamp_t timestamp, xcb_window_t active_window)
    {
        Q_UNUSED(window);
        Q_UNUSED(src);
        Q_UNUSED(timestamp);
        Q_UNUSED(active_window);
    }

    /*!
       A Window Manager should subclass NETRootInfo and reimplement this function
       when it wants to know when a pager made a request to move/resize a window.
       See _NET_MOVERESIZE_WINDOW for details.

       \a window the id of the window to more/resize

       \a flags Flags specifying the operation (see _NET_MOVERESIZE_WINDOW description)

       \a x Requested X position for the window

       \a y Requested Y position for the window

       \a width Requested width for the window

       \a height Requested height for the window
    **/
    virtual void moveResizeWindow(xcb_window_t window, int flags, int x, int y, int width, int height)
    {
        Q_UNUSED(window);
        Q_UNUSED(flags);
        Q_UNUSED(x);
        Q_UNUSED(y);
        Q_UNUSED(width);
        Q_UNUSED(height);
    }

    /*!
       A Window Manager should subclass NETRootInfo and reimplement this function
       when it wants to know when a Client made a request to restack a window.
       See _NET_RESTACK_WINDOW for details.

       \a window the id of the window to restack

       \a source the source of the request

       \a above other window in the restack request

       \a detail restack detail

       \a timestamp the timestamp of the request
    **/
    virtual void restackWindow(xcb_window_t window, RequestSource source, xcb_window_t above, int detail, xcb_timestamp_t timestamp)
    {
        Q_UNUSED(window);
        Q_UNUSED(source);
        Q_UNUSED(above);
        Q_UNUSED(detail);
        Q_UNUSED(timestamp);
    }

    /*!
       A Window Manager should subclass NETRootInfo and reimplement this function
       when it wants to know when a pager made a request to change showing the desktop.
       See _NET_SHOWING_DESKTOP for details.

       \a showing whether to activate the showing desktop mode
    **/
    virtual void changeShowingDesktop(bool showing)
    {
        Q_UNUSED(showing);
    }

    /*!
       A Window Manager should subclass NETRootInfo and reimplement this function
       when it wants to know when a Client made a request to show a window menu.

       \a window The window that wants to move/resize

       \a device_id GTK device id.

       \a x_root X position of the cursor relative to the root window.

       \a y_root Y position of the cursor relative to the root window.
    **/
    virtual void showWindowMenu(xcb_window_t window, int device_id, int x_root, int y_root)
    {
        Q_UNUSED(window);
        Q_UNUSED(device_id);
        Q_UNUSED(x_root);
        Q_UNUSED(y_root);
    }

private:
    void update(NET::Properties properties, NET::Properties2 properties2);
    void setSupported();
    void setDefaultProperties();
    void updateSupportedProperties(xcb_atom_t atom);

protected:
    /*! Virtual hook, used to add new "virtual" functions while maintaining
    binary compatibility. Unused in this class.
    */
    virtual void virtual_hook(int id, void *data);

private:
    NETRootInfoPrivate *p; // krazy:exclude=dpointer (implicitly shared)
};

/*!
   \class NETWinInfo
   \inheaderfile NETWM
   \inmodule KWindowSystem
   \brief Common API for application window properties/protocols.

   The NETWinInfo class provides a common API for clients and window managers to
   set/read/change properties on an application window as defined by the NET
   Window Manager Specification.

   \sa NET
   \sa NETRootInfo
   \sa http://www.freedesktop.org/standards/wm-spec/
 **/

class KWINDOWSYSTEM_EXPORT NETWinInfo : public NET
{
public:
    // update also NETWinInfoPrivate::properties[] size when extending this
    /*!
        Indexes for the properties array.

        \value PROTOCOLS
        \value PROTOCOLS2
        \value PROPERTIES_SIZE
    **/
    enum {
        PROTOCOLS,
        PROTOCOLS2,
        PROPERTIES_SIZE,
    };
    /*!
       Create a NETWinInfo object, which will be used to set/read/change
       information stored on an application window.

       \a connection XCB connection

       \a window The Window id of the application window.

       \a rootWindow The Window id of the root window.

       \a properties The NET::Properties flags

       \a properties2 The NET::Properties2 flags

       \a role Select the application role.  If this argument is omitted,
       the role will default to Client.
    **/
    NETWinInfo(xcb_connection_t *connection,
               xcb_window_t window,
               xcb_window_t rootWindow,
               NET::Properties properties,
               NET::Properties2 properties2,
               Role role = Client);

    /*!
       Creates a shared copy of the specified NETWinInfo object.

       \a wininfo the NETWinInfo to copy
    **/
    NETWinInfo(const NETWinInfo &wininfo);

    virtual ~NETWinInfo();

    const NETWinInfo &operator=(const NETWinInfo &wintinfo);

    /*!
       Returns the xcb connection used.

       Returns the XCB connection
    **/
    xcb_connection_t *xcbConnection() const;

    /*!
       Returns true if the window has any window type set, even if the type
       itself is not known to this implementation. Presence of a window type
       as specified by the NETWM spec is considered as the window supporting
       this specification.
       Returns true if the window has support for the NETWM spec
    **/
    bool hasNETSupport() const;

    /*!
       Returns the properties argument passed to the constructor.
       \sa passedProperties2()
    **/
    NET::Properties passedProperties() const;
    /*!
     * Returns the properties2 argument passed to the constructor.
     * \sa passedProperties()
     * \since 5.0
     **/
    NET::Properties2 passedProperties2() const;

    /*!
       Returns the icon geometry.
    **/
    NETRect iconGeometry() const;

    /*!
       Returns the state of the window (see the NET base class documentation for a
       description of the various states).
    **/
    NET::States state() const;

    /*!
       Returns the extended (partial) strut specified by this client.
       See _NET_WM_STRUT_PARTIAL in the spec.
    **/
    NETExtendedStrut extendedStrut() const;

    // Still used internally, e.g. by KWindowSystem::strutChanged() logic
    /*!
       \deprecated
       use strutPartial()

       Returns the strut specified by this client.
    **/
    NETStrut strut() const;

    /*!
       Returns the window type for this client (see the NET base class
       documentation for a description of the various window types).
       Since clients may specify several windows types for a window
       in order to support backwards compatibility and extensions
       not available in the NETWM spec, you should specify all
       window types you application supports (see the NET::WindowTypeMask
       mask values for various window types). This method will
       return the first window type that is listed in the supported types,
       or NET::Unknown if none of the window types is supported.
    **/
    WindowType windowType(WindowTypes supported_types) const;

    /*!
      This function returns false if the window has not window type
      specified at all. Used by KWindowInfo::windowType() to return either
      NET::Normal or NET::Dialog as appropriate as a fallback.
    **/
    bool hasWindowType() const;

    /*!
       Returns the name of the window in UTF-8 format.
    **/
    const char *name() const;

    /*!
       Returns the visible name as set by the window manager in UTF-8 format.
    **/
    const char *visibleName() const;

    /*!
       Returns the iconic name of the window in UTF-8 format. Note that this has
       nothing to do with icons, but it's for "iconic"
       representations of the window (taskbars etc.), that should be shown
       when the window is in iconic state. See description of _NET_WM_ICON_NAME
       for details.
    **/
    const char *iconName() const;

    /*!
       Returns the visible iconic name as set by the window manager in UTF-8 format.
       Note that this has nothing to do with icons, but it's for "iconic"
       representations of the window (taskbars etc.), that should be shown
       when the window is in iconic state. See description of _NET_WM_VISIBLE_ICON_NAME
       for details.
    **/
    const char *visibleIconName() const;

    /*!
       Returns the desktop where the window is residing.

       \note KDE uses virtual desktops and does not directly support
       viewport in any way. It is however mapped to virtual desktops
       if needed.

       \a ignore_viewport if false, viewport is mapped to virtual desktops

       \sa OnAllDesktops()
    **/
    int desktop(bool ignore_viewport = false) const;

    /*!
       Returns the process id for the client window.
    **/
    int pid() const;

    /*!
       Returns whether or not this client handles icons.
    **/
    bool handledIcons() const;

    /*!
       Returns the mapping state for the window (see the NET base class
       documentation for a description of mapping state).
    **/
    MappingState mappingState() const;

    /*!
       Set icons for the application window.  If replace is True, then
       the specified icon is defined to be the only icon.  If replace is False,
       then the specified icon is added to a list of icons.

       \a icon the new icon

       \a replace true to replace, false to append to the list of icons
    **/
    void setIcon(NETIcon icon, bool replace = true);

    /*!
       Set the icon geometry for the application window.

       \a geometry the new icon geometry
    **/
    void setIconGeometry(NETRect geometry);

    /*!
       Set the extended (partial) strut for the application window.

       \a extended_strut the new strut
    **/
    void setExtendedStrut(const NETExtendedStrut &extended_strut);

    // Still used internally, e.g. by KWindowSystem::strutChanged() logic
    /*!
       \deprecated
       use setExtendedStrut()

       Set the strut for the application window.

       \a strut the new strut
    **/
    void setStrut(NETStrut strut);

    /*!
       Set the state for the application window (see the NET base class documentation
       for a description of window state).

       \a state the name state

       \a mask the mask for the state
    **/
    void setState(NET::States state, NET::States mask);

    /*!
       Sets the window type for this client (see the NET base class
       documentation for a description of the various window types).

       \a type the window type
    **/
    void setWindowType(WindowType type);

    /*!
       Sets the name for the application window.

       \a name the new name of the window
    **/
    void setName(const char *name);

    /*!
       For Window Managers only:  set the visible name ( i.e. xterm, xterm <2>,
       xterm <3>, ... )

       \a visibleName the new visible name
    **/
    void setVisibleName(const char *visibleName);

    /*!
       Sets the iconic name for the application window.

       \a name the new iconic name
    **/
    void setIconName(const char *name);

    /*!
       For Window Managers only: set the visible iconic name ( i.e. xterm, xterm <2>,
       xterm <3>, ... )

       \a name the new visible iconic name
    **/
    void setVisibleIconName(const char *name);

    /*!
       Set which window the desktop is (should be) on.

       \note KDE uses virtual desktops and does not directly support
       viewport in any way. It is however mapped to virtual desktops
       if needed.

       \a desktop the number of the new desktop

       \a ignore_viewport if false, viewport is mapped to virtual desktops

       \sa OnAllDesktops()
    **/
    void setDesktop(int desktop, bool ignore_viewport = false);

    /*!
       Set the application window's process id.

       \a pid the window's process id
    **/
    void setPid(int pid);

    /*!
       Set whether this application window handles icons.

       \a handled true if the window handles icons, false otherwise
    **/
    void setHandledIcons(bool handled);

    /*!
       Set the frame decoration strut, i.e. the width of the decoration borders.

       \a strut the new strut
    **/
    void setFrameExtents(NETStrut strut);

    /*!
       Returns the frame decoration strut, i.e. the width of the decoration borders.

       \since 4.3
    **/
    NETStrut frameExtents() const;

    /*!
       Sets the window frame overlap strut, i.e. how far the window frame extends
       behind the client area on each side.

       Set the strut values to -1 if you want the window frame to cover the whole
       client area.

       The default values are 0.

       \since 4.4
    **/
    void setFrameOverlap(NETStrut strut);

    /*!
       Returns the frame overlap strut, i.e. how far the window frame extends
       behind the client area on each side.

       \since 4.4
    **/
    NETStrut frameOverlap() const;

    /*!
       Sets the extents of the drop-shadow drawn by the client.

       \since 5.65
    **/
    void setGtkFrameExtents(NETStrut strut);

    /*!
       Returns the extents of the drop-shadow drawn by a GTK client.

       \since 5.65
    **/
    NETStrut gtkFrameExtents() const;

    /*!
       Returns an icon.  If width and height are passed, the icon returned will be
       the closest it can find (the next biggest).  If width and height are omitted,
       then the largest icon in the list is returned.

       \a width the preferred width for the icon, -1 to ignore

       \a height the preferred height for the icon, -1 to ignore

       Returns the icon
    **/
    NETIcon icon(int width = -1, int height = -1) const;

    /*!
      Returns a list of provided icon sizes. Each size is pair width,height, terminated
      with pair 0,0.
      \since 4.3
    **/
    const int *iconSizes() const;

    /*!
     * Sets user timestamp \a time on the window (property _NET_WM_USER_TIME).
     * The timestamp is expressed as XServer time. If a window
     * is shown with user timestamp older than the time of the last
     * user action, it won't be activated after being shown, with the special
     * value 0 meaning not to activate the window after being shown.
     */
    void setUserTime(xcb_timestamp_t time);

    /*!
     * Returns the time of last user action on the window, or -1 if not set.
     */
    xcb_timestamp_t userTime() const;

    /*!
     * Sets the startup notification id \a id on the window.
     */
    void setStartupId(const char *startup_id);

    /*!
     * Returns the startup notification id of the window.
     */
    const char *startupId() const;

    /*!
     * Sets opacity (0 = transparent, 0xffffffff = opaque ) on the window.
     */
    void setOpacity(unsigned long opacity);
    /*!
     * Sets opacity (0 = transparent, 1 = opaque) on the window.
     */
    void setOpacityF(qreal opacity);

    /*!
     * Returns the opacity of the window.
     */
    unsigned long opacity() const;
    /*!
     * Returns the opacity of the window.
     */
    qreal opacityF() const;

    /*!
     * Sets actions that the window manager allows for the window.
     */
    void setAllowedActions(NET::Actions actions);

    /*!
     * Returns actions that the window manager allows for the window.
     */
    NET::Actions allowedActions() const;

    /*!
     * Returns the WM_TRANSIENT_FOR property for the window, i.e. the mainwindow
     * for this window.
     */
    xcb_window_t transientFor() const;

    /*!
     * Returns the leader window for the group the window is in, if any.
     */
    xcb_window_t groupLeader() const;

    /*!
     * Returns whether the UrgencyHint is set in the WM_HINTS.flags.
     * See ICCCM 4.1.2.4.
     *
     * \since 5.3
     **/
    bool urgency() const;

    /*!
     * Returns whether the Input flag is set in WM_HINTS.
     * See ICCCM 4.1.2.4 and 4.1.7.
     *
     * The default value is true in case the Client is mapped without a WM_HINTS property.
     *
     * \since 5.3
     **/
    bool input() const;

    /*!
     * Returns the initial mapping state as set in WM_HINTS.
     * See ICCCM 4.1.2.4 and 4.1.4.
     *
     * The default value if Withdrawn in case the Client is mapped without
     * a WM_HINTS property or without the initial state hint set.
     *
     * \since 5.5
     **/
    MappingState initialMappingState() const;

    /*!
     * Returns the icon pixmap as set in WM_HINTS.
     * See ICCCM 4.1.2.4.
     *
     * The default value is XCB_PIXMAP_NONE.
     *
     * Using the ICCCM variant for the icon is deprecated and only
     * offers a limited functionality compared to icon.
     * Only use this variant as a fallback.
     *
     * \sa icccmIconPixmapMask
     * \sa icon
     * \since 5.7
     **/
    xcb_pixmap_t icccmIconPixmap() const;

    /*!
     * Returns the mask for the icon pixmap as set in WM_HINTS.
     * See ICCCM 4.1.2.4.
     *
     * The default value is XCB_PIXMAP_NONE.
     *
     * \sa icccmIconPixmap
     * \since 5.7
     **/
    xcb_pixmap_t icccmIconPixmapMask() const;

    /*!
     * Returns the class component of the window class for the window
     * (i.e. WM_CLASS property).
     */
    const char *windowClassClass() const;

    /*!
     * Returns the name component of the window class for the window
     * (i.e. WM_CLASS property).
     */
    const char *windowClassName() const;

    /*!
     * Returns the window role for the window (i.e. WM_WINDOW_ROLE property).
     */
    const char *windowRole() const;

    /*!
     * Returns the client machine for the window (i.e. WM_CLIENT_MACHINE property).
     */
    const char *clientMachine() const;

    /*!
     * returns a comma-separated list of the activities the window is associated with.
     * FIXME this might be better as a NETRArray ?
     * \since 4.6
     */
    const char *activities() const;

    /*!
     * Sets the comma-separated list of activities the window is associated with.
     * \since 5.1
     */
    void setActivities(const char *activities);

    /*!
     * Sets whether the client wishes to block compositing (for better performance)
     * \since 4.7
     */
    void setBlockingCompositing(bool active);

    /*!
     * Returns whether the client wishes to block compositing (for better performance)
     * \since 4.7
     */
    bool isBlockingCompositing() const;

    /*!
       Places the window frame geometry in frame, and the application window
       geometry in window.  Both geometries are relative to the root window.

       \a frame the geometry for the frame

       \a window the geometry for the window
    **/
    void kdeGeometry(NETRect &frame, NETRect &window);

    /*!
       Sets the desired multiple-monitor topology (4 monitor indices indicating
       the top, bottom, left, and right edges of the window) when the fullscreen
       state is enabled. The indices are from the set returned by the Xinerama
       extension.
       See _NET_WM_FULLSCREEN_MONITORS for details.

       \a topology A struct that models the desired monitor topology, namely:
       top is the monitor whose top edge defines the top edge of the
       fullscreen window, bottom is the monitor whose bottom edge defines
       the bottom edge of the fullscreen window, left is the monitor whose
       left edge defines the left edge of the fullscreen window, and right
       is the monitor whose right edge defines the right edge of the fullscreen
       window.

    **/
    void setFullscreenMonitors(NETFullscreenMonitors topology);

    /*!
       Returns the desired fullscreen monitor topology for this client, should
       it be in fullscreen state.
       See _NET_WM_FULLSCREEN_MONITORS in the spec.
    **/
    NETFullscreenMonitors fullscreenMonitors() const;

    /*!
     * This function takes the passed in xcb_generic_event_t and returns the updated properties
     * in the passed in arguments.
     *
     * The new information will be read immediately by the class. It is possible to pass in a
     * null pointer in the arguments. In that case the passed in
     * argument will obviously not be updated, but the class will process the information
     * nevertheless.
     *
     * \a event the event
     *
     * \a properties The NET::Properties that changed
     *
     * \a properties2 The NET::Properties2 that changed
     * \since 5.0
     **/
    void event(xcb_generic_event_t *event, NET::Properties *properties, NET::Properties2 *properties2 = nullptr);

    /*!
       This function takes the pass XEvent and returns an OR'ed list of NETWinInfo
       properties that have changed.  The new information will be read
       immediately by the class. This overloaded version returns
       only a single mask, and therefore cannot check state of all properties
       like the other variant.

       \a event the event

       Returns the properties
    **/
    NET::Properties event(xcb_generic_event_t *event);

    /*!
     * Returns The window manager protocols this Client supports.
     * \since 5.3
     **/
    NET::Protocols protocols() const;

    /*!
     * Returns true if the Client supports the \a protocol.
     * \a protocol The window manager protocol to test for
     * \since 5.3
     **/
    bool supportsProtocol(NET::Protocol protocol) const;

    /*!
     * Returns The opaque region as specified by the Client.
     * \since 5.7
     **/
    std::vector<NETRect> opaqueRegion() const;

    /*!
     * Sets the \a name as the desktop file name.
     *
     * This is either the base name without full path and without file extension of the
     * desktop file for the window's application (e.g. "org.kde.foo").
     *
     * If the application's desktop file name is not at a standard location it should be
     * the full path to the desktop file name (e.g. "/opt/kde/share/org.kde.foo.desktop").
     *
     * If the window does not know the desktop file name, it should not set the name at all.
     *
     * \since 5.28
     **/
    void setDesktopFileName(const char *name);

    /*!
     * Returns The desktop file name of the window's application if present.
     * \since 5.28
     * \sa setDesktopFileName
     **/
    const char *desktopFileName() const;

    /*!
     * Returns The GTK application id of the window if present.
     * \since 5.91
     **/
    const char *gtkApplicationId() const;

    /*!
     * Sets the \a name as the D-BUS service name for the application menu.
     * \since 5.69
     **/
    void setAppMenuServiceName(const char *name);

    /*!
     * Sets the \a name as the D-BUS object path for the application menu.
     * \since 5.69
     **/
    void setAppMenuObjectPath(const char *path);

    /*!
     * Returns The menu service name of the window's application if present.
     * \since 5.69
     **/
    const char *appMenuServiceName() const;

    /*!
     * Returns The menu object path of the window's application if present.
     * \since 5.69
     **/
    const char *appMenuObjectPath() const;

    /*!
       Sentinel value to indicate that the client wishes to be visible on
       all desktops.

       Returns the value to be on all desktops
    **/
    static const int OnAllDesktops;

protected:
    /*!
       A Window Manager should subclass NETWinInfo and reimplement this function when
       it wants to know when a Client made a request to change desktops (ie. move to
       another desktop).

       \a desktop the number of the desktop
    **/
    virtual void changeDesktop(int desktop)
    {
        Q_UNUSED(desktop);
    }

    /*!
       A Window Manager should subclass NETWinInfo and reimplement this function when
       it wants to know when a Client made a request to change state (ie. to
       Shade / Unshade).

       \a state the new state

       \a mask the mask for the state
    **/
    virtual void changeState(NET::States state, NET::States mask)
    {
        Q_UNUSED(state);
        Q_UNUSED(mask);
    }

    /*!
       A Window Manager should subclass NETWinInfo2 and reimplement this function
       when it wants to know when a Client made a request to change the
       fullscreen monitor topology for its fullscreen state.

       \a topology A structure (top, bottom, left, right) representing the
       fullscreen monitor topology.
    **/
    virtual void changeFullscreenMonitors(NETFullscreenMonitors topology)
    {
        Q_UNUSED(topology);
    }

private:
    void update(NET::Properties dirtyProperties, NET::Properties2 dirtyProperties2 = NET::Properties2());
    void updateWMState();
    void setIconInternal(NETRArray<NETIcon> &icons, int &icon_count, xcb_atom_t property, NETIcon icon, bool replace);
    NETIcon iconInternal(NETRArray<NETIcon> &icons, int icon_count, int width, int height) const;

protected:
    /* Virtual hook, used to add new "virtual" functions while maintaining
    binary compatibility. Unused in this class.
    */
    virtual void virtual_hook(int id, void *data);

private:
    NETWinInfoPrivate *p; // krazy:exclude=dpointer (implicitly shared)
};

// #define KWIN_FOCUS

#endif
#endif // netwm_h

/*
    This file is part of the KDE project
    SPDX-FileCopyrightText: 2000 Simon Hausmann <hausmann@kde.org>
    SPDX-FileCopyrightText: 2000 David Faure <faure@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef kxmlguibuilder_h
#define kxmlguibuilder_h

#include <kxmlgui_export.h>

#include <QStringList>

class KXMLGUIBuilderPrivate;
class KXMLGUIClient;

class QAction;
class QDomElement;
class QWidget;

/*!
 * \class KXMLGUIBuilder
 * \inmodule KXmlGui
 *
 * Implements the creation of the GUI (menubar, menus and toolbars)
 * as requested by the GUI factory.
 *
 * The virtual methods are mostly for historical reasons, there isn't really
 * a need to derive from KXMLGUIBuilder anymore.
 */
class KXMLGUI_EXPORT KXMLGUIBuilder
{
public:
    /*!
     * \brief Constructs a new KXMLGUIBuilder object from \a widget.
     */
    explicit KXMLGUIBuilder(QWidget *widget);
    /*!
     * \brief Destructor.
     */
    virtual ~KXMLGUIBuilder();

    /* \internal */
    KXMLGUIClient *builderClient() const;
    /* \internal */
    void setBuilderClient(KXMLGUIClient *client);
    /* \internal */
    QWidget *widget();

    virtual QStringList containerTags() const;

    /*!
     * \brief Creates a container (menubar/menu/toolbar/statusbar/separator/...)
     * from an \a element at \a index in the XML file.
     *
     * \a parent The parent for the container.
     *
     * \a index The index where the container should be inserted
     * into the parent container/widget.
     *
     * \a element The element from the DOM tree describing the
     * container (use it to access container specified attributes
     * or child elements).
     *
     * \a containerAction The action created for this container;
     * used for e.g. passing to removeContainer.
     */
    virtual QWidget *createContainer(QWidget *parent, int index, const QDomElement &element, QAction *&containerAction);

    /*!
     * \brief Removes the given \a container.
     *
     * \a parent The parent for the container.
     *
     * \a element The element from the DOM tree describing the container
     * (use it to access container specified attributes or child elements).
     *
     * \a containerAction The action created for this container.
     */
    virtual void removeContainer(QWidget *container, QWidget *parent, QDomElement &element, QAction *containerAction);

    virtual QStringList customTags() const;

    virtual QAction *createCustomElement(QWidget *parent, int index, const QDomElement &element);

    virtual void finalizeGUI(KXMLGUIClient *client);

protected:
    virtual void virtual_hook(int id, void *data);

private:
    KXMLGUIBuilderPrivate *const d;
};

#endif

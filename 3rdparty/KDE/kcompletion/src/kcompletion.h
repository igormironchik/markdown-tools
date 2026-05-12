/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 1999, 2000 Carsten Pfeiffer <pfeiffer@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KCOMPLETION_H
#define KCOMPLETION_H

#include <kcompletion_export.h>

#include <QKeySequence>
#include <QObject>
#include <QPointer>
#include <QStringList>
#include <functional>
#include <memory>

class KCompTreeNode;
class KCompletionPrivate;
class KCompletionMatchesWrapper;
class KCompletionMatches;

/*!
 * \class KCompletion
 * \inmodule KCompletion
 *
 * \brief A generic class for completing QStrings.
 *
 * This class offers easy use of "auto completion", "manual completion" or
 * "shell completion" on QString objects. A common use is completing filenames
 * or URLs (see KUrlCompletion()).
 * But it is not limited to URL-completion -- everything should be completable!
 * The user should be able to complete email addresses, telephone numbers,
 * commands, SQL queries...
 * Every time your program knows what the user can type into an edit field, you
 * should offer completion. With KCompletion, this is very easy, and if you are
 * using a line edit widget (KLineEdit), it is even easier.
 * Basically, you tell a KCompletion object what strings should be completable
 * and, whenever completion should be invoked, you call makeCompletion().
 * KLineEdit and (an editable) KComboBox even do this automatically for you.
 *
 * KCompletion offers the completed string via the signal match() and
 * all matching strings (when the result is ambiguous) via the method
 * allMatches().
 *
 * Notice: auto completion, shell completion and manual completion work
 *         slightly differently:
 * \list
 * \li auto completion always returns a complete item as match.
 *     When more than one matching item is available, it will deliver just
 *     the first one (depending on sorting order). Iterating over all matches
 *     is possible via nextMatch() and previousMatch().
 *
 * \li popup completion works in the same way, the only difference being that
 *     the completed items are not put into the edit widget, but into a
 *     separate popup box.
 *
 * \li manual completion works the same way as auto completion, except that
 *     it is not invoked automatically while the user is typing,
 *     but only when the user presses a special key. The difference
 *     of manual and auto completion is therefore only visible in UI classes.
 *     KCompletion needs to know whether to deliver partial matches
 *     (shell completion) or whole matches (auto/manual completion), therefore
 *     KCompletion::CompletionMan and KCompletion::CompletionAuto have the exact
 *     same effect in KCompletion.
 *
 * \li shell completion works like "tab completion" in a shell:
 *     when multiple matches are available, the longest possible string of all
 *     matches is returned (i.e. only a partial item).
 *     Iterating over all matching items (complete, not partial) is possible
 *     via nextMatch() and previousMatch().
 * \endlist
 *
 * As an application programmer, you do not normally have to worry about
 * the different completion modes; KCompletion handles
 * that for you, according to the setting setCompletionMode().
 * The default setting is globally configured by the user and read
 * from completionMode().
 *
 * A short example:
 * \code
 * KCompletion completion;
 * completion.setOrder(KCompletion::Sorted);
 * completion.addItem("pfeiffer@kde.org");
 * completion.addItem("coolo@kde.org");
 * completion.addItem("carpdjih@sp.zrz.tu-berlin.de");
 * completion.addItem("carp\cs.tu-berlin.de");
 *
 * cout << completion.makeCompletion("ca").latin1() << endl;
 * \endcode
 *
 * In shell-completion mode, this will be "carp"; in auto-completion
 * mode it will be "carp\\cs.tu-berlin.de", as that is alphabetically
 * smaller.
 * If setOrder was set to Insertion, "carpdjih\@sp.zrz.tu-berlin.de"
 * would be completed in auto-completion mode, as that was inserted before
 * "carp\\cs.tu-berlin.de".
 *
 * You can dynamically update the completable items by removing and adding them
 * whenever you want.
 * For advanced usage, you could even use multiple KCompletion objects. E.g.
 * imagine an editor like kwrite with multiple open files. You could store
 * items of each file in a different KCompletion object, so that you know (and
 * tell the user) where a completion comes from.
 *
 * \note KCompletion does not work with strings that contain 0x0 characters
 *       (unicode null), as this is used internally as a delimiter.
 *
 * You may inherit from KCompletion and override makeCompletion() in
 * special cases (like reading directories or urls and then supplying the
 * contents to KCompletion, as KUrlCompletion does), but this is usually
 * not necessary.
 */
class KCOMPLETION_EXPORT KCompletion : public QObject
{
    /*!
     * \property KCompletion::order
     */
    Q_PROPERTY(CompOrder order READ order WRITE setOrder)

    /*!
     * \property KCompletion::ignoreCase
     */
    Q_PROPERTY(bool ignoreCase READ ignoreCase WRITE setIgnoreCase)

    /*!
     * \property KCompletion::items
     */
    Q_PROPERTY(QStringList items READ items WRITE setItems)

    Q_OBJECT
    Q_DECLARE_PRIVATE(KCompletion)

public:
    /*!
     * This enum describes the completion mode used for by the KCompletion class.
     *
     * \value CompletionNone No completion is used.
     * \value CompletionAuto Text is automatically filled in whenever possible.
     * \value CompletionMan Same as automatic, but shortest match is used for completion.
     * \value CompletionShell Completes text much in the same way as a typical *nix shell would.
     * \value CompletionPopup Lists all possible matches in a popup list box to choose from.
     * \value CompletionPopupAuto Lists all possible matches in a popup list box to choose from, and automatically fills the result whenever possible.
     *
     * \since 5.0
     */
    enum CompletionMode {
        CompletionNone = 1,
        CompletionAuto,
        CompletionMan,
        CompletionShell,
        CompletionPopup,
        CompletionPopupAuto,
    };

    /*!
     * Constants that represent the order in which KCompletion performs
     * completion lookups.
     *
     * \value Sorted Use alphabetically sorted order or custom sorter logic.
     * \value Insertion Use order of insertion.
     * \value Weighted Use weighted order
     *
     */
    enum CompOrder {
        Sorted,
        Insertion,
        Weighted,
    };
    Q_ENUM(CompOrder)

    /*!
     * The sorter function signature. Deriving classes may provide
     * custom sorting logic via the setSorterFunction method.
     *
     * \since 5.88
     */
    using SorterFunction = std::function<void(QStringList &)>;

    /*!
     * Constructor, nothing special here :)
     */
    KCompletion();

    ~KCompletion() override;

    /*!
     * Returns a list of all completion items that contain the given \a string.
     * \a string the string to complete
     *
     * Returns a list of items which contain \a text as a substring,
     * i.e. not necessarily at the beginning.
     *
     * \sa makeCompletion
     */
    QStringList substringCompletion(const QString &string) const;

    /*!
     * Returns the last match. Might be useful if you need to check whether
     * a completion is different from the last one.
     *
     * QString() is returned when there is no
     *         last match.
     */
    virtual const QString &lastMatch() const;

    /*!
     * Returns a list of all items inserted into KCompletion. This is useful
     * if you need to save the state of a KCompletion object and restore it
     * later.
     *
     * \note When order() == Weighted, then every item in the
     * stringlist has its weight appended, delimited by a colon. E.g. an item
     * "www.kde.org" might look like "www.kde.org:4", where 4 is the weight.
     * This is necessary so that you can save the items along with its
     * weighting on disk and load them back with setItems(), restoring its
     * weight as well. If you really don't want the appended weightings, call
     * setOrder( KCompletion::Insertion ) before calling items().
     *
     * \sa setItems
     */
    QStringList items() const;

    /*!
     * Returns \c true if the completion object contains no entries.
     */
    bool isEmpty() const;

    /*!
     * Sets the completion mode.
     *
     * \a mode the completion mode
     *
     * \sa CompletionMode
     */
    virtual void setCompletionMode(CompletionMode mode);

    /*!
     * Returns the current completion mode, default is CompletionPopup
     * \sa setCompletionMode
     * \sa CompletionMode
     */
    CompletionMode completionMode() const;

    /*!
     * KCompletion offers three different ways in which it offers its items:
     * \list
     * \li in the order of insertion
     * \li sorted alphabetically
     * \li weighted
     * \endlist
     *
     * Choosing weighted makes KCompletion perform an implicit weighting based
     * on how often an item is inserted. Imagine a web browser with a location
     * bar, where the user enters URLs. The more often a URL is entered, the
     * higher priority it gets.
     *
     * \note Setting the order to sorted only affects new inserted items,
     * already existing items will stay in the current order. So you probably
     * want to call setOrder(Sorted) before inserting items if you want
     * everything sorted.
     *
     * Default is insertion order.
     *
     * \a order the new order
     * \sa order
     */
    virtual void setOrder(CompOrder order);

    /*!
     * Returns the completion order.
     * \sa setOrder
     */
    CompOrder order() const;

    /*!
     * Setting this to true makes KCompletion behave case insensitively.
     *
     * E.g. makeCompletion("CA"); might return "carp\\cs.tu-berlin.de".
     *
     * Default is false (case sensitive).
     *
     * \a ignoreCase true to ignore the case
     *
     * \sa ignoreCase
     */
    virtual void setIgnoreCase(bool ignoreCase);

    /*!
     * Returns whether KCompletion acts case insensitively or not.
     *
     * Default is \c false (case sensitive).
     *
     * \sa setIgnoreCase
     */
    bool ignoreCase() const;

    /*!
     * Informs the caller if they should display the auto-suggestion for the last completion operation performed.
     *
     * Applies for CompletionPopupAuto and CompletionAuto modes.
     *
     * Defaults to \c true, but deriving classes may set it to false in special cases via "setShouldAutoSuggest".
     *
     * Returns \c true if auto-suggestion should be displayed for the last completion operation performed.
     * \since 5.87
     */
    bool shouldAutoSuggest() const;

    /*!
     * Returns a list of all items matching the last completed string.
     * It might take some time if you have a lot of items.
     * \sa substringCompletion
     */
    QStringList allMatches();

    /*!
     * Returns a list of all items matching \a string.
     */
    QStringList allMatches(const QString &string);

    /*!
     * Returns a list of all items matching the last completed string.
     * It might take some time if you have a lot of items.
     * The matches are returned as KCompletionMatches, which also
     * keeps the weight of the matches, allowing
     * you to modify some matches or merge them with matches
     * from another call to allWeightedMatches(), and sort the matches
     * after that in order to have the matches ordered correctly.
     *
     * \sa substringCompletion
     */
    KCompletionMatches allWeightedMatches();

    /*!
     * Returns a list of all items matching \a string.
     */
    KCompletionMatches allWeightedMatches(const QString &string);

#if KCOMPLETION_BUILD_DEPRECATED_SINCE(6, 11) // not KCOMPLETION_ENABLE_DEPRECATED_SINCE because this is a virtual function
    /*!
     * Enables/disables emitting a sound when
     * \list
     * \li makeCompletion() can't find a match
     * \li there is a partial completion (= multiple matches in
     *     Shell-completion mode)
     * \li nextMatch() or previousMatch() hit the last possible
     *     match and the list is rotated
     * \endlist
     *
     * KNotifyClient() is used to emit the sounds.
     *
     * \a enable true to enable sounds
     *
     * \deprecated[6.11]
     * not implemented
     * \sa soundsEnabled
     */
    KCOMPLETION_DEPRECATED_VERSION(6, 11, "Not implemented")
    virtual void setSoundsEnabled(bool enable);
#endif

#if KCOMPLETION_ENABLE_DEPRECATED_SINCE(6, 11)
    /*!
     * Tells you whether KCompletion will emit sounds on certain occasions.
     *
     * Default is enabled.
     *
     * \deprecated[6.11]
     * Not implemented
     *
     * Returns \c true if sounds are enabled
     * \sa setSoundsEnabled
     */
    KCOMPLETION_DEPRECATED_VERSION(6, 11, "Not implemented")
    bool soundsEnabled() const;
#endif

    /*!
     * Returns \c true when more than one match is found.
     * \sa multipleMatches
     */
    bool hasMultipleMatches() const;

public Q_SLOTS:
    /*!
     * Attempts to find an item in the list of available completions
     * that begins with \a string. Will either return the first matching item
     * (if there is more than one match) or QString(), if no match is
     * found.
     *
     * In the latter case, a sound will be emitted, depending on
     * soundsEnabled().
     * If a match is found, it will be emitted via the signal
     * match().
     *
     * If this is called twice or more with the same string while no
     * items were added or removed in the meantime, all available completions
     * will be emitted via the signal matches().
     * This happens only in shell-completion mode.
     *
     * \a string the string to complete
     *
     * Returns the matching item, or QString() if there is no matching
     * item.
     * \sa substringCompletion
     */
    virtual QString makeCompletion(const QString &string);

    /*!
     * Returns the next item from the list of matching items.
     *
     * When reaching the beginning, the list is rotated so it will return the
     * last match and a sound is emitted (depending on soundsEnabled()).
     *
     * Returns the next item from the list of matching items.
     * When there is no match, QString() is returned and
     * a sound is emitted.
     */
    QString previousMatch();

    /*!
     * Returns the next item from the list of matching items.
     *
     * When reaching the last item, the list is rotated, so it will return
     * the first match and a sound is emitted (depending on
     * soundsEnabled()).
     *
     * Returns the next item from the list of matching items. When there is no
     * match, QString() is returned and a sound is emitted.
     */
    QString nextMatch();

    /*!
     * Inserts \a items into the list of possible completions.
     *
     * It does the same as setItems(), but without calling clear() before.
     *
     * \a items the items to insert
     */
    void insertItems(const QStringList &items);

    /*!
     * Sets the list of items available for completion. Removes all previous
     * items.
     *
     * \note When order() == Weighted, then the weighting is looked up for
     * every item in the stringlist. Every item should have ":number" appended,
     * where number is an unsigned integer, specifying the weighting.
     * If you don't like this, call
     * setOrder(KCompletion::Insertion)
     * before calling setItems().
     *
     * \a itemList the list of items that are available for completion
     *
     * \sa items
     */
    virtual void setItems(const QStringList &itemList);

    /*!
     * Adds an item to the list of available completions.
     * Resets the current item state (previousMatch() and nextMatch()
     * won't work the next time they are called).
     *
     * \a item the item to add
     */
    void addItem(const QString &item);

    /*!
     * Adds an item to the list of available completions.
     * Resets the current item state (previousMatch() and nextMatch()
     * won't work the next time they are called).
     *
     * Sets the weight of the item to \a weight or adds it to the current
     * weight if the item is already available. The weight has to be greater
     * than 1 to take effect (default weight is 1).
     *
     * \a item the item to add
     *
     * \a weight the weight of the item, default is 1
     */
    void addItem(const QString &item, uint weight);

    /*!
     * Removes an item from the list of available completions.
     * Resets the current item state (previousMatch() and nextMatch()
     * won't work the next time they are called).
     *
     * \a item the item to remove
     */
    void removeItem(const QString &item);

    /*!
     * Removes all inserted items.
     */
    virtual void clear();

Q_SIGNALS:
    /*!
     * This signal is emitted when a match is found.
     *
     * In particular, makeCompletion(), previousMatch() and nextMatch()
     * all emit this signal; makeCompletion() will only emit it when a
     * match is found, but the other methods will always emit it (and so
     * may emit it with an empty string).
     *
     * \a item the matching item, or QString() if there were no more
     * matching items.
     */
    void match(const QString &item);

    /*!
     * This signal is emitted by makeCompletion() in shell-completion mode
     * when the same string is passed to makeCompletion() multiple times in
     * a row.
     *
     * \a matchlist the list of all matching items
     */
    void matches(const QStringList &matchlist);

    /*!
     * This signal is emitted when calling makeCompletion() and more than
     * one matching item is found.
     * \sa hasMultipleMatches
     */
    void multipleMatches();

protected:
    /*!
     * This method is called after a completion is found and before the
     * matching string is emitted. You can override this method to modify the
     * string that will be emitted.
     * This is necessary e.g. in KUrlCompletion(), where files with spaces
     * in their names are shown escaped ("filename\ with\ spaces"), but stored
     * unescaped inside KCompletion.
     * Never delete that pointer!
     *
     * Default implementation does nothing.
     *
     * \a match the match to process
     *
     * \sa postProcessMatches
     */
    virtual void postProcessMatch(QString *match) const;

    /*!
     * This method is called before a list of all available completions is
     * emitted via matches(). You can override this method to modify the
     * found items before match() or matches() are emitted.
     * Never delete that pointer!
     *
     * Default implementation does nothing.
     *
     * \a matchList the matches to process
     *
     * \sa postProcessMatch
     */
    virtual void postProcessMatches(QStringList *matchList) const;

    /*!
     * This method is called before a list of all available completions is
     * emitted via #matches(). You can override this method to modify the
     * found items before #match() or #matches() are emitted.
     * Never delete that pointer!
     *
     * Default implementation does nothing.
     *
     * \a matches the matches to process
     *
     * \sa postProcessMatch
     */
    virtual void postProcessMatches(KCompletionMatches *matches) const;

    /*!
     * Deriving classes may set this property and control whether the auto-suggestion should be displayed
     * for the last completion operation performed.
     *
     * Applies for CompletionPopupAuto and CompletionAuto modes.
     * \since 5.87
     */
    void setShouldAutoSuggest(bool shouldAutosuggest);

    /*!
     * Sets a custom function to be used to sort the matches.
     * Can be set to nullptr to use the default sorting logic.
     *
     * Applies for CompOrder::Sorted mode.
     * \since 5.88
     */
    void setSorterFunction(SorterFunction sortFunc);

private:
    Q_DISABLE_COPY(KCompletion)
    std::unique_ptr<KCompletionPrivate> const d_ptr;
};

#endif // KCOMPLETION_H

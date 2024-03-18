/*
    SPDX-FileCopyrightText: 2016 Volker Krause <vkrause@kde.org>
    SPDX-FileCopyrightText: 2018 Christoph Cullmann <cullmann@kde.org>

    SPDX-License-Identifier: MIT
*/

#ifndef KSYNTAXHIGHLIGHTING_STATE_P_H
#define KSYNTAXHIGHLIGHTING_STATE_P_H

#include <QList>
#include <QSharedData>

#include "definitionref_p.h"

namespace KSyntaxHighlighting
{
class Context;

class StateData : public QSharedData
{
    friend class State;
    friend class AbstractHighlighter;

public:
    StateData() = default;

    static StateData *reset(State &state);
    static StateData *detach(State &state);

    static StateData *get(const State &state)
    {
        return state.d.data();
    }

    int size() const
    {
        return m_contextStack.size();
    }

    void push(Context *context, QStringList &&captures);

    /**
     * Pop the number of elements given from the top of the current stack.
     * Will not pop the initial element.
     * @param popCount number of elements to pop
     * @return false if one has tried to pop the initial context, else true
     */
    bool pop(int popCount);

    Context *topContext() const
    {
        return m_contextStack.last().context;
    }

    const QStringList &topCaptures() const
    {
        return m_contextStack.last().captures;
    }

private:
    struct StackValue {
        Context *context;
        QStringList captures;

        bool operator==(const StackValue &other) const
        {
            return context == other.context && captures == other.captures;
        }
    };

    /**
     * definition id to filter out invalid states
     */
    uint64_t m_defId = 0;

    /**
     * the context stack combines the active context + valid captures
     */
    QList<StackValue> m_contextStack;
};

}

#endif

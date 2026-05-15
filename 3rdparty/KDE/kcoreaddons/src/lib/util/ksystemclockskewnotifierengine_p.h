/*
    SPDX-FileCopyrightText: 2019 Vlad Zahorodnii <vlad.zahorodnii@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#pragma once

#include <QObject>

class KSystemClockSkewNotifierEngine : public QObject
{
    Q_OBJECT

public:
    static std::shared_ptr<KSystemClockSkewNotifierEngine> globalInstance();

protected:
    explicit KSystemClockSkewNotifierEngine(QObject *parent = nullptr);

Q_SIGNALS:
    void skewed();
};

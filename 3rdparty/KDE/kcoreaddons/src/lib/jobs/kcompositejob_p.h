/*
    This file is part of the KDE project

    SPDX-FileCopyrightText: 2006 Kevin Ottens <ervin@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KCOMPOSITEJOB_P_H
#define KCOMPOSITEJOB_P_H

#include "kcompositejob.h"

#include "kjob_p.h"

class KCompositeJobPrivate : public KJobPrivate
{
public:
    KCompositeJobPrivate();
    ~KCompositeJobPrivate() override;

    QList<KJob *> subjobs;

    Q_DECLARE_PUBLIC(KCompositeJob)
};

#endif

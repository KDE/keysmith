/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 * SPDX-FileCopyrightText: 2021 Johan Ouwerkerk <jm.ouwerkerk@gmail.com>
 */

#include "state_p.h"

namespace app
{

    OverviewState::OverviewState(QObject *parent) :
        QObject(parent), m_actionsEnabled(false)
    {
    }

    bool OverviewState::actionsEnabled(void) const
    {
        return m_actionsEnabled;
    }

    void OverviewState::setActionsEnabled(bool enabled)
    {
        if (m_actionsEnabled != enabled) {
            m_actionsEnabled = enabled;
            Q_EMIT actionsEnabledChanged();
        }
    }

    FlowState::FlowState(QObject *parent) :
        QObject(parent), m_flowRunning(false), m_initialFlowDone(false)
    {
    }

    bool FlowState::flowRunning(void) const
    {
        return m_flowRunning;
    }

    void FlowState::setFlowRunning(bool running)
    {
        if (m_flowRunning != running) {
            m_flowRunning = running;
            Q_EMIT flowRunningChanged();
        }
    }

    bool FlowState::initialFlowDone(void) const
    {
        return m_initialFlowDone;
    }

    void FlowState::setInitialFlowDone(bool done)
    {
        if (m_initialFlowDone != done) {
            m_initialFlowDone = done;
            Q_EMIT initialFlowDoneChanged();
        }
    }
}

#include "moc_state_p.cpp"

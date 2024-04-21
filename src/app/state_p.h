/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 * SPDX-FileCopyrightText: 2021 Johan Ouwerkerk <jm.ouwerkerk@gmail.com>
 */
#ifndef APP_STATE_P_H
#define APP_STATE_P_H

#include <QObject>

namespace app
{
class OverviewState : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool actionsEnabled READ actionsEnabled WRITE setActionsEnabled NOTIFY actionsEnabledChanged)
public:
    explicit OverviewState(QObject *parent = nullptr);
    bool actionsEnabled(void) const;
    void setActionsEnabled(bool enabled);
Q_SIGNALS:
    void actionsEnabledChanged(void);

private:
    bool m_actionsEnabled;
};

class FlowState : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool initialFlowDone READ initialFlowDone WRITE setInitialFlowDone NOTIFY initialFlowDoneChanged)
    Q_PROPERTY(bool flowRunning READ flowRunning WRITE setFlowRunning NOTIFY flowRunningChanged)
public:
    explicit FlowState(QObject *parent = nullptr);
    bool flowRunning(void) const;
    void setFlowRunning(bool running);
    bool initialFlowDone(void) const;
    void setInitialFlowDone(bool done);
Q_SIGNALS:
    void initialFlowDoneChanged(void);
    void flowRunningChanged(void);

private:
    bool m_flowRunning;
    bool m_initialFlowDone;
};
}

#endif

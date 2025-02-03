/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 * SPDX-FileCopyrightText: 2021 Johan Ouwerkerk <jm.ouwerkerk@gmail.com>
 * SPDX-FileCopyrightText: 2025 Jack Hill <jackhill3103@gmail.com>
 */
#ifndef APP_FLOWS_P_H
#define APP_FLOWS_P_H

#include <QCommandLineParser>

#include "../model/accounts.h"
#include "../model/password.h"
#include "keysmith.h"
#include "vms.h"

namespace app
{
class InitialFlow : public QObject
{
    Q_OBJECT
public:
    explicit InitialFlow(Keysmith *app);

public:
    void run(const QCommandLineParser &parser);
private Q_SLOTS:
    void onNewAccountInvalid(void);
    void onNewAccountProcessed(void);
    void onNewAccountAccepted(void);
    void onNewAccountRejected(void);
    void resume(void);

private:
    bool m_started;
    bool m_uriToAdd;
    bool m_uriParsed;
    bool m_passwordPromptResolved;

private:
    Keysmith *const m_app;
    model::AccountInput *const m_input;
    model::PasswordRequest *const m_passwordRequest;
};

class ManualAddAccountFlow : public QObject
{
    Q_OBJECT
public:
    explicit ManualAddAccountFlow(Keysmith *app);
    void run(void);
private Q_SLOTS:
    void back(void);
    void onAccepted(void);

private:
    Keysmith *const m_app;
    model::AccountInput *const m_input;
};

class ExternalCommandLineFlow : public QObject
{
    Q_OBJECT
public:
    explicit ExternalCommandLineFlow(Keysmith *app);

public:
    void run(const QCommandLineParser &parser);
private Q_SLOTS:
    void onNewAccountInvalid(void);
    void onNewAccountProcessed(void);
    void back(void);
    void onAccepted(void);

private:
    Keysmith *const m_app;
    model::AccountInput *const m_input;
};

class ManualImportAccountFlow: public QObject
{
    Q_OBJECT
public:
    explicit ManualImportAccountFlow(Keysmith *app);
    void run(void);
private Q_SLOTS:
    void back(void);
    void onAccepted(void);
private:
    Keysmith * const m_app;
    model::ImportInput * const m_input;
};

class AddAccountFromQRFlow : public QObject
{
    Q_OBJECT
public:
    explicit AddAccountFromQRFlow(Keysmith *app);

public:
    void run(void);

private:
    void startScan(void);

private Q_SLOTS:
    void scanComplete(const QByteArray &uri);
    void scanCompleteText(const QString &uri);
    void parseComplete(void);
    void onAccepted(void);
    void back(void);
    void exit(void);

private:
    Keysmith *const m_app;
    model::AccountInput *const m_input;
    ScanQRViewModel *const m_scan_vm;
    bool m_scanned = false;
};
}

#endif

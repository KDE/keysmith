/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 * SPDX-FileCopyrightText: 2020 Johan Ouwerkerk <jm.ouwerkerk@gmail.com>
 */
#ifndef APP_COMMAND_LINE_H
#define APP_COMMAND_LINE_H

#include "../model/input.h"

#include <QCommandLineParser>
#include <QObject>
#include <QString>

namespace app
{

    class CommandLineAccountJob: public QObject
    {
        Q_OBJECT
    public:
        explicit CommandLineAccountJob(model::AccountInput *recipient);
        void run(const QString &uri);
    Q_SIGNALS:
        void newAccountInvalid(void);
        void newAccountProcessed(void);
    private:
        static void processNewAccount(CommandLineAccountJob *target, const QString &uri);
    private Q_SLOTS:
        void expired(void);
    private:
        bool m_alive;
        model::AccountInput * const m_recipient;
    };

    class CommandLineOptions: public QObject
    {
        Q_OBJECT
        Q_PROPERTY(bool optionsOk READ optionsOk CONSTANT);
        Q_PROPERTY(QString errorText READ errorText CONSTANT);
        Q_PROPERTY(bool newAccountRequested READ newAccountRequested CONSTANT);
    public:
        static void addOptions(QCommandLineParser &parser);
        explicit CommandLineOptions(QCommandLineParser &parser, bool parseOk, QObject *parent = nullptr);
        QString errorText(void) const;
        bool optionsOk(void) const;
        bool newAccountRequested(void) const;
    public Q_SLOTS:
        void handleNewAccount(model::AccountInput *recipient);
    Q_SIGNALS:
        void newAccountInvalid(void);
        void newAccountProcessed(void);
    private:
        static void processNewAccount(CommandLineOptions *target, model::AccountInput *recipient);
    private:
        const bool m_parseOk;
        const QString m_errorText;
        QCommandLineParser &m_parser;
    };
};

#endif

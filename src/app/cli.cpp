/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 * SPDX-FileCopyrightText: 2020 Johan Ouwerkerk <jm.ouwerkerk@gmail.com>
 */
#include "cli.h"
#include "../logging_p.h"
#include "../model/qr.h"

#include <KLocalizedString>
#include <QCommandLineOption>
#include <QtConcurrent>

KEYSMITH_LOGGER(logger, ".app.cli")

namespace app
{
    void CommandLineOptions::addOptions(QCommandLineParser &parser)
    {
        parser.addPositionalArgument(
            QStringLiteral("<uri>"),
            i18nc("@info (<uri> placeholder)", "Optional account to add, formatted as otpauth:// URI (e.g. from a QR code)")
        );
    }

    CommandLineOptions::CommandLineOptions(QCommandLineParser &parser, bool parseOk, QObject *parent) :
        QObject(parent), m_parseOk(parseOk), m_errorText(parseOk ? QString() : parser.errorText()), m_parser(parser)
    {
    }

    QString CommandLineOptions::errorText(void) const
    {
        return m_errorText;
    }

    bool CommandLineOptions::optionsOk(void) const
    {
        return m_parseOk;
    }

    bool CommandLineOptions::newAccountRequested(void) const
    {
        return optionsOk() && !m_parser.positionalArguments().isEmpty();
    }

    void CommandLineOptions::handleNewAccount(model::AccountInput *recipient)
    {
        if (!newAccountRequested()) {
            qCDebug(logger) << "Ignoring request to handle new account:"
                << "Invalid commandline options or no URI was received on the commandline";
            return;
        }

        auto job = new CommandLineAccountJob(recipient);
        const auto argv = m_parser.positionalArguments();
        QObject::connect(job, &CommandLineAccountJob::newAccountProcessed, this, &CommandLineOptions::newAccountProcessed);
        QObject::connect(job, &CommandLineAccountJob::newAccountInvalid, this, &CommandLineOptions::newAccountInvalid);
        job->run(argv[0]);
    }

    CommandLineAccountJob::CommandLineAccountJob(model::AccountInput *recipient) :
        QObject(), m_alive(true), m_recipient(recipient)
    {
        QObject::connect(recipient, &QObject::destroyed, this, &CommandLineAccountJob::expired);
    }

    void CommandLineAccountJob::expired(void)
    {
        m_alive = false;
    }

    void CommandLineAccountJob::run(const QString &uri)
    {
        QtConcurrent::run(&CommandLineAccountJob::processNewAccount, this, uri);
    }

    void CommandLineAccountJob::processNewAccount(CommandLineAccountJob *target, const QString &uri)
    {
        const auto result = model::QrParameters::parse(uri);
        bool invoked = false;
        if (result) {
            qCInfo(logger) << "Successfully parsed the URI passed on the commandline";
            invoked = QMetaObject::invokeMethod(target, [target, result](void) -> void {
                if (target->m_alive) {
                    result->populate(target->m_recipient);
                    qCDebug(logger) << "Reporting success parsing URI from commandline";
                    Q_EMIT target->newAccountProcessed();
                } else {
                    qCDebug(logger) << "Not reporting success parsing URI from commandline: recipient has expired";
                }
                QTimer::singleShot(0, target, &QObject::deleteLater);
            });
        } else {
            qCInfo(logger) << "Failed to parse the URI passed on the commandline";
            invoked = QMetaObject::invokeMethod(target, [target](void) -> void {
                if (target->m_alive) {
                    qCDebug(logger) << "Reporting failure to parse URI from commandline";
                    Q_EMIT target->newAccountInvalid();
                } else {
                    qCDebug(logger) << "Not reporting failure to parse URI from commandline: recipient has expired";
                }
                QTimer::singleShot(0, target, &QObject::deleteLater);
            });
        }

        if (!invoked) {
            Q_ASSERT_X(false, Q_FUNC_INFO, "should be able to invoke meta method");
            qCDebug(logger) << "Failed to signal result of processing the URI passed on the commandline";
        }
    }
}

/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 * SPDX-FileCopyrightText: 2020 Johan Ouwerkerk <jm.ouwerkerk@gmail.com>
 */
#include "account/actions_p.h"

#include "../test-utils/output.h"
#include "../test-utils/spy.h"

#include <QFile>
#include <QSet>
#include <QSignalSpy>
#include <QString>
#include <QTest>
#include <QUuid>
#include <QtDebug>

static QString existingAccountsIni(QLatin1String("existing-accounts.ini"));
static QString emptyAccountsIni(QLatin1String("empty-accounts.ini"));

class DeleteAccountsTest: public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void removeAccounts(void);
    void removeAccounts_data(void);
};

static void define_test_data(void)
{
    QTest::addColumn<QSet<QUuid>>("ids");
    QTest::addColumn<QString>("accounts");
    QTest::addColumn<QString>("isolated");
    QTest::addColumn<QString>("expected");
}

static void define_test_case(const char *label, const QString &accounts, const QString &isolated, const QString &expected, const QSet<QUuid> &toRemove)
{
    QTest::newRow(label) << toRemove << accounts << isolated << expected;
}

void DeleteAccountsTest::removeAccounts(void)
{
    QFETCH(QSet<QUuid>, ids);
    QFETCH(QString, accounts);
    QFETCH(QString, isolated);
    QFETCH(QString, expected);

    QVERIFY2(test::copyResourceAsWritable(accounts, isolated), "accounts INI resource should be available as file");

    bool actionRun = false;
    const QString actual = test::path(isolated);
    const accounts::SettingsProvider settings([&actual, &actionRun](const accounts::PersistenceAction &action) -> void
    {
        QSettings data(actual, QSettings::IniFormat);
        actionRun = true;
        action(data);
    });

    accounts::DeleteAccounts uut(settings, ids);
    QSignalSpy invalidSettings(&uut, &accounts::DeleteAccounts::invalid);
    QSignalSpy jobFinished(&uut, &accounts::DeleteAccounts::finished);

    uut.run();

    QVERIFY2(test::signal_eventually_emitted_once(jobFinished), "job should be finished");
    QVERIFY2(actionRun, "accounts action should have run");
    QCOMPARE(invalidSettings.count(), 0);

    QFile result(actual);
    QVERIFY2(result.exists(), "accounts file should still exist");
    QCOMPARE(test::slurp(actual), test::slurp(expected));
}

void DeleteAccountsTest::removeAccounts_data(void)
{
    static const QString existing(QLatin1String(":/delete-accounts/sample-accounts.ini"));
    static const QString empty(QLatin1String(":/delete-accounts/empty-accounts.ini"));
    static const QString hotp(QLatin1String(":/delete-accounts/only-hotp-left.ini"));
    static const QString totp(QLatin1String(":/delete-accounts/only-totp-left.ini"));

    define_test_data();
    define_test_case("remove hotp from sample accounts", existing, QLatin1String("remove-hotp-from-sample.ini"), totp, QSet<QUuid>() << QUuid(QLatin1String("072a645d-6c26-57cc-81eb-d9ef3b9b39e2")) );
    define_test_case("remove totp from sample accounts", existing, QLatin1String("remove-totp-from-sample.ini"), hotp, QSet<QUuid>() << QUuid(QLatin1String("534cc72e-e9ec-5e39-a1ff-9f017c9be8cc")));
    define_test_case("remove both from sample accounts", existing, QLatin1String("remove-both-from-sample.ini"), empty, QSet<QUuid>() << QUuid(QLatin1String("072a645d-6c26-57cc-81eb-d9ef3b9b39e2")) << QUuid(QLatin1String("534cc72e-e9ec-5e39-a1ff-9f017c9be8cc")));
    define_test_case("remove hotp from totp only account", totp, QLatin1String("remove-hotp-from-totp.ini"), totp, QSet<QUuid>() << QUuid(QLatin1String("072a645d-6c26-57cc-81eb-d9ef3b9b39e2")));
    define_test_case("remove totp from hotp only account", hotp, QLatin1String("remove-totp-from-hotp.ini"), hotp, QSet<QUuid>() << QUuid(QLatin1String("534cc72e-e9ec-5e39-a1ff-9f017c9be8cc")));
    define_test_case("remove both from empty accounts", empty, QLatin1String("remove-both-from-empty.ini"), empty, QSet<QUuid>() << QUuid(QLatin1String("072a645d-6c26-57cc-81eb-d9ef3b9b39e2")) << QUuid(QLatin1String("534cc72e-e9ec-5e39-a1ff-9f017c9be8cc")));
    define_test_case("remove bogus accounts from sample accounts", existing, QLatin1String("remove-bogus-from-sample.ini"), existing, QSet<QUuid>() << QUuid(QLatin1String("1885b51f-c4cf-448d-8cad-53306a1f558e")));
    define_test_case("remove nothing from sample accounts", existing, QLatin1String("remove-nothing-from-sample.ini"), existing, QSet<QUuid>());
}

QTEST_MAIN(DeleteAccountsTest)

#include "delete-accounts.moc"

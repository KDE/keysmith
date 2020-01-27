/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 * SPDX-FileCopyrightText: 2020 Johan Ouwerkerk <jm.ouwerkerk@gmail.com>
 */
#include "account/validation.h"

#include <QObject>
#include <QTest>

class AccountValidationTest: public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void checkId(void);
    void checkId_data(void);
    void checkName(void);
    void checkName_data(void);
    void checkSecret(void);
    void checkSecret_data(void);
    void checkTokenLength(void);
    void checkTokenLength_data(void);
    void checkTimeStep(void);
    void checkTimeStep_data(void);
};

void AccountValidationTest::checkId(void)
{
    QFETCH(QUuid, id);
    QTEST(accounts::checkId(id), "expected");
}

void AccountValidationTest::checkName(void)
{
    QFETCH(QString, name);
    QTEST(accounts::checkName(name), "expected");
}

void AccountValidationTest::checkSecret(void)
{
    QFETCH(QString, secret);
    QTEST(accounts::checkSecret(secret), "expected");
}

void AccountValidationTest::checkTokenLength(void)
{
    QFETCH(int, tokenLength);
    QTEST(accounts::checkTokenLength(tokenLength), "expected");
}

void AccountValidationTest::checkTimeStep(void)
{
    QFETCH(uint, timeStep);
    QTEST(accounts::checkTimeStep(timeStep), "expected");
}

void AccountValidationTest::checkId_data()
{
    QTest::addColumn<QUuid>("id");
    QTest::addColumn<bool>("expected");

    QTest::newRow("implicit null uuid") << QUuid() << false;
    QTest::newRow("explicit null uuid") << QUuid(QLatin1String("00000000-0000-0000-0000-000000000000")) << false;
    QTest::newRow("valid uuid") << QUuid(QLatin1String("52c32412-8472-4fdc-8fad-2d53ecffd391")) << true;
}

void AccountValidationTest::checkName_data()
{
    QTest::addColumn<QString>("name");
    QTest::addColumn<bool>("expected");

    QTest::newRow("null value") << QString() << false;
    QTest::newRow("empty name") << QString(QLatin1String("")) << false;
    QTest::newRow("valid name") << QString(QLatin1String("something")) << true;
}

void AccountValidationTest::checkSecret_data()
{
    QTest::addColumn<QString>("secret");
    QTest::addColumn<bool>("expected");

    QTest::newRow("null value") << QString() << false;
    QTest::newRow("empty secret") << QString(QLatin1String("")) << false;
    QTest::newRow("valid secret") << QString(QLatin1String("ONSWG4TFOQ======")) << true;
}

void AccountValidationTest::checkTokenLength_data()
{
    QTest::addColumn<int>("tokenLength");
    QTest::addColumn<bool>("expected");

    QTest::newRow("too small") << 5 << false;
    QTest::newRow("too large") << 9 << false;
    QTest::newRow("minimum") << 6 << true;
    QTest::newRow("maximum") << 8 << true;
}

void AccountValidationTest::checkTimeStep_data()
{
    QTest::addColumn<uint>("timeStep");
    QTest::addColumn<bool>("expected");

    QTest::newRow("too small") << 0U << false;
    QTest::newRow("minimum") << 1U << true;
    QTest::newRow("default") << 30U << true;
}

QTEST_APPLESS_MAIN(AccountValidationTest)

#include "account-validation.moc"

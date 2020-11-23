/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 * SPDX-FileCopyrightText: 2020 Johan Ouwerkerk <jm.ouwerkerk@gmail.com>
 */
#include "account/keys.h"

#include "../../test-utils/spy.h"

#include <QSignalSpy>
#include <QTest>

class PasswordFlowTest : public QObject // clazy:exclude=ctor-missing-parent-argument
{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase(void);
    void supplyExistingPassword(void);
    void cancelExistingPassword(void);
    void supplyNewPassword(void);
    void cancelNewPassword(void);
private:
    QByteArray m_salt;
    std::optional<secrets::KeyDerivationParameters> m_keyParams = secrets::KeyDerivationParameters::create(
        crypto_secretbox_KEYBYTES, crypto_pwhash_ALG_DEFAULT, crypto_pwhash_MEMLIMIT_MIN, crypto_pwhash_OPSLIMIT_MIN
    );
};

void PasswordFlowTest::initTestCase(void)
{
    m_salt.resize(crypto_pwhash_SALTBYTES);
    QVERIFY2(m_keyParams, "should be able to construct key derivation parameters");
}

void PasswordFlowTest::supplyNewPassword(void)
{
    accounts::AccountSecret uut;
    QSignalSpy existingPasswordNeeded(&uut, &accounts::AccountSecret::existingPasswordNeeded);
    QSignalSpy newPasswordNeeded(&uut, &accounts::AccountSecret::newPasswordNeeded);
    QSignalSpy passwordAvailable(&uut, &accounts::AccountSecret::passwordAvailable);
    QSignalSpy requestsCancelled(&uut, &accounts::AccountSecret::requestsCancelled);
    QSignalSpy keyAvailable(&uut, &accounts::AccountSecret::keyAvailable);

    // check correct initial state is reported
    QCOMPARE(uut.isStillAlive(), true);
    QCOMPARE(uut.isNewPasswordRequested(), false);
    QCOMPARE(uut.isExistingPasswordRequested(), false);
    QCOMPARE(uut.isPasswordAvailable(), false);
    QCOMPARE(uut.isKeyAvailable(), false);
    QCOMPARE(uut.key(), nullptr);

    // advance the state: request password
    QVERIFY2(uut.requestNewPassword(), "should be able to request a (new) password");
    QVERIFY2(test::signal_eventually_emitted_once(newPasswordNeeded), "request for (new) password should be signalled");

    // check the state is correctly updated
    QCOMPARE(uut.isStillAlive(), true);
    QCOMPARE(uut.isNewPasswordRequested(), true);
    QCOMPARE(uut.isExistingPasswordRequested(), false);
    QCOMPARE(uut.isPasswordAvailable(), false);
    QCOMPARE(uut.isKeyAvailable(), false);
    QCOMPARE(uut.key(), nullptr);

    QCOMPARE(newPasswordNeeded.count(), 1);
    QCOMPARE(passwordAvailable.count(), 0);
    QCOMPARE(existingPasswordNeeded.count(), 0);
    QCOMPARE(keyAvailable.count(), 0);
    QCOMPARE(requestsCancelled.count(), 0);

    // advance the state: supply password
    QString password(QStringLiteral("hello, world"));
    QVERIFY2(m_keyParams, "should be able to construct key derivation parameters");

    QVERIFY2(uut.answerNewPassword(password, *m_keyParams), "(new) password should be accepted");
    QVERIFY2(test::signal_eventually_emitted_once(passwordAvailable), "availability of the (new) password should be signalled");
    QCOMPARE(password, QStringLiteral("************"));

    // check the state is correctly updated
    QCOMPARE(uut.isStillAlive(), true);
    QCOMPARE(uut.isNewPasswordRequested(), true);
    QCOMPARE(uut.isExistingPasswordRequested(), false);
    QCOMPARE(uut.isPasswordAvailable(), true);
    QCOMPARE(uut.isKeyAvailable(), false);
    QCOMPARE(uut.key(), nullptr);

    QCOMPARE(newPasswordNeeded.count(), 1);
    QCOMPARE(passwordAvailable.count(), 1);
    QCOMPARE(existingPasswordNeeded.count(), 0);
    QCOMPARE(keyAvailable.count(), 0);
    QCOMPARE(requestsCancelled.count(), 0);

    // advance the state: derive the master key
    QVERIFY2(uut.deriveKey(), "key derivation should succeed");
    QVERIFY2(test::signal_eventually_emitted_once(keyAvailable), "availability of the master key should be signalled");

    // check the state is correctly updated
    QCOMPARE(uut.isStillAlive(), true);
    QCOMPARE(uut.isNewPasswordRequested(), true);
    QCOMPARE(uut.isExistingPasswordRequested(), false);
    QCOMPARE(uut.isPasswordAvailable(), false);
    QCOMPARE(uut.isKeyAvailable(), true);
    QVERIFY2(uut.key(), "should have a master key by now");

    QCOMPARE(newPasswordNeeded.count(), 1);
    QCOMPARE(passwordAvailable.count(), 1);
    QCOMPARE(existingPasswordNeeded.count(), 0);
    QCOMPARE(keyAvailable.count(), 1);
    QCOMPARE(requestsCancelled.count(), 0);
}

void PasswordFlowTest::cancelNewPassword(void)
{
    accounts::AccountSecret uut;
    QSignalSpy existingPasswordNeeded(&uut, &accounts::AccountSecret::existingPasswordNeeded);
    QSignalSpy newPasswordNeeded(&uut, &accounts::AccountSecret::newPasswordNeeded);
    QSignalSpy passwordAvailable(&uut, &accounts::AccountSecret::passwordAvailable);
    QSignalSpy requestsCancelled(&uut, &accounts::AccountSecret::requestsCancelled);
    QSignalSpy keyAvailable(&uut, &accounts::AccountSecret::keyAvailable);

    // check correct initial state is reported
    QCOMPARE(uut.isStillAlive(), true);
    QCOMPARE(uut.isNewPasswordRequested(), false);
    QCOMPARE(uut.isExistingPasswordRequested(), false);
    QCOMPARE(uut.isPasswordAvailable(), false);
    QCOMPARE(uut.isKeyAvailable(), false);
    QCOMPARE(uut.key(), nullptr);

    // advance the state: request password
    QVERIFY2(uut.requestNewPassword(), "should be able to request a (new) password");
    QVERIFY2(test::signal_eventually_emitted_once(newPasswordNeeded), "request for (new) password should be signalled");

    // check the state is correctly updated
    QCOMPARE(uut.isStillAlive(), true);
    QCOMPARE(uut.isNewPasswordRequested(), true);
    QCOMPARE(uut.isExistingPasswordRequested(), false);
    QCOMPARE(uut.isPasswordAvailable(), false);
    QCOMPARE(uut.isKeyAvailable(), false);
    QCOMPARE(uut.key(), nullptr);

    QCOMPARE(newPasswordNeeded.count(), 1);
    QCOMPARE(passwordAvailable.count(), 0);
    QCOMPARE(existingPasswordNeeded.count(), 0);
    QCOMPARE(keyAvailable.count(), 0);
    QCOMPARE(requestsCancelled.count(), 0);

    // advance the state: cancel the request
    uut.cancelRequests();
    QVERIFY2(test::signal_eventually_emitted_once(requestsCancelled), "requests for (new) password should be cancelled by now");

    // check the state is correctly updated
    QCOMPARE(uut.isStillAlive(), false);
    QCOMPARE(uut.isNewPasswordRequested(), true);
    QCOMPARE(uut.isExistingPasswordRequested(), false);
    QCOMPARE(uut.isPasswordAvailable(), false);
    QCOMPARE(uut.isKeyAvailable(), false);
    QCOMPARE(uut.key(), nullptr);

    QCOMPARE(newPasswordNeeded.count(), 1);
    QCOMPARE(passwordAvailable.count(), 0);
    QCOMPARE(existingPasswordNeeded.count(), 0);
    QCOMPARE(keyAvailable.count(), 0);
    QCOMPARE(requestsCancelled.count(), 1);
}

void PasswordFlowTest::cancelExistingPassword(void)
{
    accounts::AccountSecret uut;
    QSignalSpy existingPasswordNeeded(&uut, &accounts::AccountSecret::existingPasswordNeeded);
    QSignalSpy newPasswordNeeded(&uut, &accounts::AccountSecret::newPasswordNeeded);
    QSignalSpy passwordAvailable(&uut, &accounts::AccountSecret::passwordAvailable);
    QSignalSpy requestsCancelled(&uut, &accounts::AccountSecret::requestsCancelled);
    QSignalSpy keyAvailable(&uut, &accounts::AccountSecret::keyAvailable);

    // check correct initial state is reported
    QCOMPARE(uut.isStillAlive(), true);
    QCOMPARE(uut.isNewPasswordRequested(), false);
    QCOMPARE(uut.isExistingPasswordRequested(), false);
    QCOMPARE(uut.isPasswordAvailable(), false);
    QCOMPARE(uut.isKeyAvailable(), false);
    QCOMPARE(uut.key(), nullptr);

    // advance the state: request password
    QVERIFY2(uut.requestExistingPassword(m_salt, *m_keyParams), "should be able to request a (existing) password");
    QVERIFY2(test::signal_eventually_emitted_once(existingPasswordNeeded), "request for (existing) password should be signalled");

    // check the state is correctly updated
    QCOMPARE(uut.isStillAlive(), true);
    QCOMPARE(uut.isNewPasswordRequested(), false);
    QCOMPARE(uut.isExistingPasswordRequested(), true);
    QCOMPARE(uut.isPasswordAvailable(), false);
    QCOMPARE(uut.isKeyAvailable(), false);
    QCOMPARE(uut.key(), nullptr);

    QCOMPARE(newPasswordNeeded.count(), 0);
    QCOMPARE(passwordAvailable.count(), 0);
    QCOMPARE(existingPasswordNeeded.count(), 1);
    QCOMPARE(keyAvailable.count(), 0);
    QCOMPARE(requestsCancelled.count(), 0);

    // advance the state: cancel the request
    uut.cancelRequests();
    QVERIFY2(test::signal_eventually_emitted_once(requestsCancelled), "requests for (new) password should be cancelled by now");

    // check the state is correctly updated
    QCOMPARE(uut.isStillAlive(), false);
    QCOMPARE(uut.isNewPasswordRequested(), false);
    QCOMPARE(uut.isExistingPasswordRequested(), true);
    QCOMPARE(uut.isPasswordAvailable(), false);
    QCOMPARE(uut.isKeyAvailable(), false);
    QCOMPARE(uut.key(), nullptr);

    QCOMPARE(newPasswordNeeded.count(), 0);
    QCOMPARE(passwordAvailable.count(), 0);
    QCOMPARE(existingPasswordNeeded.count(), 1);
    QCOMPARE(keyAvailable.count(), 0);
    QCOMPARE(requestsCancelled.count(), 1);
}

void PasswordFlowTest::supplyExistingPassword(void)
{
    accounts::AccountSecret uut;
    QSignalSpy existingPasswordNeeded(&uut, &accounts::AccountSecret::existingPasswordNeeded);
    QSignalSpy newPasswordNeeded(&uut, &accounts::AccountSecret::newPasswordNeeded);
    QSignalSpy passwordAvailable(&uut, &accounts::AccountSecret::passwordAvailable);
    QSignalSpy requestsCancelled(&uut, &accounts::AccountSecret::requestsCancelled);
    QSignalSpy keyAvailable(&uut, &accounts::AccountSecret::keyAvailable);

    // check correct initial state is reported
    QCOMPARE(uut.isStillAlive(), true);
    QCOMPARE(uut.isNewPasswordRequested(), false);
    QCOMPARE(uut.isExistingPasswordRequested(), false);
    QCOMPARE(uut.isPasswordAvailable(), false);
    QCOMPARE(uut.isKeyAvailable(), false);
    QCOMPARE(uut.key(), nullptr);

    // advance the state: request password
    QVERIFY2(uut.requestExistingPassword(m_salt, *m_keyParams), "should be able to request a (existing) password");
    QVERIFY2(test::signal_eventually_emitted_once(existingPasswordNeeded), "request for (existing) password should be signalled");

    // check the state is correctly updated
    QCOMPARE(uut.isStillAlive(), true);
    QCOMPARE(uut.isNewPasswordRequested(), false);
    QCOMPARE(uut.isExistingPasswordRequested(), true);
    QCOMPARE(uut.isPasswordAvailable(), false);
    QCOMPARE(uut.isKeyAvailable(), false);
    QCOMPARE(uut.key(), nullptr);

    QCOMPARE(newPasswordNeeded.count(), 0);
    QCOMPARE(passwordAvailable.count(), 0);
    QCOMPARE(existingPasswordNeeded.count(), 1);
    QCOMPARE(keyAvailable.count(), 0);
    QCOMPARE(requestsCancelled.count(), 0);

    // advance the state: supply password
    QString password(QStringLiteral("hello, world"));

    QVERIFY2(uut.answerExistingPassword(password), "(existing) password should be accepted");
    QVERIFY2(test::signal_eventually_emitted_once(passwordAvailable), "availability of the (existing) password should be signalled");
    QCOMPARE(password, QStringLiteral("************"));

    // check the state is correctly updated
    QCOMPARE(uut.isStillAlive(), true);
    QCOMPARE(uut.isNewPasswordRequested(), false);
    QCOMPARE(uut.isExistingPasswordRequested(), true);
    QCOMPARE(uut.isPasswordAvailable(), true);
    QCOMPARE(uut.isKeyAvailable(), false);
    QCOMPARE(uut.key(), nullptr);

    QCOMPARE(newPasswordNeeded.count(), 0);
    QCOMPARE(passwordAvailable.count(), 1);
    QCOMPARE(existingPasswordNeeded.count(), 1);
    QCOMPARE(keyAvailable.count(), 0);
    QCOMPARE(requestsCancelled.count(), 0);

    // advance the state: derive the master key
    QVERIFY2(uut.deriveKey(), "key derivation should succeed");
    QVERIFY2(test::signal_eventually_emitted_once(keyAvailable), "availability of the master key should be signalled");

    // check the state is correctly updated
    QCOMPARE(uut.isStillAlive(), true);
    QCOMPARE(uut.isNewPasswordRequested(), false);
    QCOMPARE(uut.isExistingPasswordRequested(), true);
    QCOMPARE(uut.isPasswordAvailable(), false);
    QCOMPARE(uut.isKeyAvailable(), true);
    QVERIFY2(uut.key(), "should have a master key by now");

    QCOMPARE(newPasswordNeeded.count(), 0);
    QCOMPARE(passwordAvailable.count(), 1);
    QCOMPARE(existingPasswordNeeded.count(), 1);
    QCOMPARE(keyAvailable.count(), 1);
    QCOMPARE(requestsCancelled.count(), 0);
}

QTEST_MAIN(PasswordFlowTest)

#include "account-secret-password-flow.moc"

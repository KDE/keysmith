/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 * SPDX-FileCopyrightText: 2020 Johan Ouwerkerk <jm.ouwerkerk@gmail.com>
 */
#include "account/keys.h"

#include "../../secrets/test-utils/random.h"
#include "../../test-utils/spy.h"

#include <QSignalSpy>
#include <QTest>

#include <cstring>

static QByteArray fill(int size)
{
    QByteArray a;
    a.resize(size);
    /*
     * Because this value is used to generate the expected challenge value(s) up front, this salt has to match the
     * behaviour of  test::fakeRandom() in case of 'new' passwords where the actual salt will be drawn 'randomly'
     * (from test::fakeRandom()).
     */
    a.fill('A', -1);
    return a;
}

static QByteArray salt()
{
    return fill(crypto_pwhash_SALTBYTES);
}

static QByteArray masterPassword(void)
{
    static QByteArray MASTER_PASSWORD("hello, world");
    return MASTER_PASSWORD;
}

static secrets::SecureMemory *secret(void)
{
    const auto master = masterPassword();
    size_t size = (size_t)master.size();
    auto memory = secrets::SecureMemory::allocate(size);
    if (memory) {
        std::memcpy(memory->data(), master.constData(), size);
    }
    return memory;
}

static std::optional<secrets::KeyDerivationParameters> keyParams =
    secrets::KeyDerivationParameters::create(crypto_secretbox_KEYBYTES, crypto_pwhash_ALG_DEFAULT, crypto_pwhash_MEMLIMIT_MIN, crypto_pwhash_OPSLIMIT_MIN);

static secrets::SecureMasterKey *key(secrets::SecureMemory *password)
{
    if (!keyParams) {
        qDebug() << "Unable to setup() dummy master key to generate test data with";
        return nullptr;
    }
    return secrets::SecureMasterKey::derive(password, *keyParams, salt(), &test::fakeRandom);
}

static std::optional<secrets::EncryptedSecret> challenge(void)
{
    QScopedPointer<secrets::SecureMemory> s(secret());
    if (!s) {
        qDebug() << "Unable to generate challenge(), unable to allocate buffer for password secret.";
        return std::nullopt;
    }

    QScopedPointer<secrets::SecureMasterKey> k(key(s.data()));
    if (!k) {
        qDebug() << "Unable to generate challenge(), unable to setup dummy master key.";
        return std::nullopt;
    }
    return k->encrypt(s.data());
}

class PasswordFlowTest : public QObject // clazy:exclude=ctor-missing-parent-argument
{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase(void);
    void supplyExistingPassword(void);
    void cancelExistingPassword(void);
    void supplyNewPassword(void);
    void cancelNewPassword(void);
    void retryExistingPassword(void);

private:
    QByteArray m_salt = salt();
    std::optional<secrets::EncryptedSecret> m_challenge = challenge();
};

void PasswordFlowTest::initTestCase(void)
{
    QVERIFY2(keyParams, "should be able to construct key derivation parameters");
    QVERIFY2(m_challenge, "should be able to construct password challenge");
    qDebug() << "Running with challenge:" << m_challenge->cryptText().toBase64() << "nonce:" << m_challenge->nonce().toBase64();
}

void PasswordFlowTest::supplyNewPassword(void)
{
    accounts::AccountSecret uut(&test::fakeRandom);
    QSignalSpy existingPasswordNeeded(&uut, &accounts::AccountSecret::existingPasswordNeeded);
    QSignalSpy newPasswordNeeded(&uut, &accounts::AccountSecret::newPasswordNeeded);
    QSignalSpy passwordAvailable(&uut, &accounts::AccountSecret::passwordAvailable);
    QSignalSpy requestsCancelled(&uut, &accounts::AccountSecret::requestsCancelled);
    QSignalSpy keyAvailable(&uut, &accounts::AccountSecret::keyAvailable);
    QSignalSpy keyFailed(&uut, &accounts::AccountSecret::keyFailed);

    // check correct initial state is reported
    QCOMPARE(uut.isStillAlive(), true);
    QCOMPARE(uut.isNewPasswordRequested(), false);
    QCOMPARE(uut.isExistingPasswordRequested(), false);
    QCOMPARE(uut.isPasswordAvailable(), false);
    QCOMPARE(uut.isKeyAvailable(), false);
    QCOMPARE(uut.key(), nullptr);
    QVERIFY2(!uut.challenge(), "should not have a (generated) password challenge yet");

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
    QVERIFY2(!uut.challenge(), "should not have a (generated) password challenge yet");

    QCOMPARE(newPasswordNeeded.count(), 1);
    QCOMPARE(passwordAvailable.count(), 0);
    QCOMPARE(existingPasswordNeeded.count(), 0);
    QCOMPARE(keyAvailable.count(), 0);
    QCOMPARE(keyFailed.count(), 0);
    QCOMPARE(requestsCancelled.count(), 0);

    // advance the state: supply password
    QString password = QString::fromUtf8(masterPassword());
    QString wiped = QStringLiteral("*").repeated(password.size());

    QVERIFY2(uut.answerNewPassword(password, *keyParams), "(new) password should be accepted");
    QVERIFY2(test::signal_eventually_emitted_once(passwordAvailable), "availability of the (new) password should be signalled");
    QCOMPARE(password, wiped);

    // check the state is correctly updated
    QCOMPARE(uut.isStillAlive(), true);
    QCOMPARE(uut.isNewPasswordRequested(), true);
    QCOMPARE(uut.isExistingPasswordRequested(), false);
    QCOMPARE(uut.isPasswordAvailable(), true);
    QCOMPARE(uut.isKeyAvailable(), false);
    QCOMPARE(uut.key(), nullptr);
    QVERIFY2(!uut.challenge(), "should still not have a (generated) password challenge yet");

    QCOMPARE(newPasswordNeeded.count(), 1);
    QCOMPARE(passwordAvailable.count(), 1);
    QCOMPARE(existingPasswordNeeded.count(), 0);
    QCOMPARE(keyAvailable.count(), 0);
    QCOMPARE(keyFailed.count(), 0);
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
    const auto generatedChallenge = uut.challenge();
    QVERIFY2(generatedChallenge, "should have a (generated) password challenge by now");
    QCOMPARE(generatedChallenge->cryptText(), m_challenge->cryptText());
    QCOMPARE(generatedChallenge->nonce(), m_challenge->nonce());

    QCOMPARE(newPasswordNeeded.count(), 1);
    QCOMPARE(passwordAvailable.count(), 1);
    QCOMPARE(existingPasswordNeeded.count(), 0);
    QCOMPARE(keyAvailable.count(), 1);
    QCOMPARE(keyFailed.count(), 0);
    QCOMPARE(requestsCancelled.count(), 0);
}

void PasswordFlowTest::cancelNewPassword(void)
{
    accounts::AccountSecret uut(&test::fakeRandom);
    QSignalSpy existingPasswordNeeded(&uut, &accounts::AccountSecret::existingPasswordNeeded);
    QSignalSpy newPasswordNeeded(&uut, &accounts::AccountSecret::newPasswordNeeded);
    QSignalSpy passwordAvailable(&uut, &accounts::AccountSecret::passwordAvailable);
    QSignalSpy requestsCancelled(&uut, &accounts::AccountSecret::requestsCancelled);
    QSignalSpy keyAvailable(&uut, &accounts::AccountSecret::keyAvailable);
    QSignalSpy keyFailed(&uut, &accounts::AccountSecret::keyFailed);

    // check correct initial state is reported
    QCOMPARE(uut.isStillAlive(), true);
    QCOMPARE(uut.isNewPasswordRequested(), false);
    QCOMPARE(uut.isExistingPasswordRequested(), false);
    QCOMPARE(uut.isPasswordAvailable(), false);
    QCOMPARE(uut.isKeyAvailable(), false);
    QCOMPARE(uut.key(), nullptr);
    QVERIFY2(!uut.challenge(), "should not have a (generated) password challenge yet");

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
    QVERIFY2(!uut.challenge(), "should still not have a (generated) password challenge yet");

    QCOMPARE(newPasswordNeeded.count(), 1);
    QCOMPARE(passwordAvailable.count(), 0);
    QCOMPARE(existingPasswordNeeded.count(), 0);
    QCOMPARE(keyAvailable.count(), 0);
    QCOMPARE(keyFailed.count(), 0);
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
    QVERIFY2(!uut.challenge(), "should still not acknowledge a (generated) password challenge");

    QCOMPARE(newPasswordNeeded.count(), 1);
    QCOMPARE(passwordAvailable.count(), 0);
    QCOMPARE(existingPasswordNeeded.count(), 0);
    QCOMPARE(keyAvailable.count(), 0);
    QCOMPARE(keyFailed.count(), 0);
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
    QSignalSpy keyFailed(&uut, &accounts::AccountSecret::keyFailed);

    // check correct initial state is reported
    QCOMPARE(uut.isStillAlive(), true);
    QCOMPARE(uut.isNewPasswordRequested(), false);
    QCOMPARE(uut.isExistingPasswordRequested(), false);
    QCOMPARE(uut.isPasswordAvailable(), false);
    QCOMPARE(uut.isKeyAvailable(), false);
    QCOMPARE(uut.key(), nullptr);
    QVERIFY2(!uut.challenge(), "should not have a password challenge yet");

    // advance the state: request password
    QVERIFY2(uut.requestExistingPassword(*m_challenge, m_salt, *keyParams), "should be able to request a (existing) password");
    QVERIFY2(test::signal_eventually_emitted_once(existingPasswordNeeded), "request for (existing) password should be signalled");

    // check the state is correctly updated
    QCOMPARE(uut.isStillAlive(), true);
    QCOMPARE(uut.isNewPasswordRequested(), false);
    QCOMPARE(uut.isExistingPasswordRequested(), true);
    QCOMPARE(uut.isPasswordAvailable(), false);
    QCOMPARE(uut.isKeyAvailable(), false);
    QCOMPARE(uut.key(), nullptr);
    const auto preservedChallenge = uut.challenge();
    QVERIFY2(preservedChallenge, "should have the supplied password challenge by now");
    QCOMPARE(preservedChallenge->cryptText(), m_challenge->cryptText());
    QCOMPARE(preservedChallenge->nonce(), m_challenge->nonce());

    QCOMPARE(newPasswordNeeded.count(), 0);
    QCOMPARE(passwordAvailable.count(), 0);
    QCOMPARE(existingPasswordNeeded.count(), 1);
    QCOMPARE(keyAvailable.count(), 0);
    QCOMPARE(keyFailed.count(), 0);
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
    QVERIFY2(!uut.challenge(), "should no longer acknowledge to the supplied password challenge");

    QCOMPARE(newPasswordNeeded.count(), 0);
    QCOMPARE(passwordAvailable.count(), 0);
    QCOMPARE(existingPasswordNeeded.count(), 1);
    QCOMPARE(keyAvailable.count(), 0);
    QCOMPARE(keyFailed.count(), 0);
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
    QSignalSpy keyFailed(&uut, &accounts::AccountSecret::keyFailed);

    // check correct initial state is reported
    QCOMPARE(uut.isStillAlive(), true);
    QCOMPARE(uut.isNewPasswordRequested(), false);
    QCOMPARE(uut.isExistingPasswordRequested(), false);
    QCOMPARE(uut.isPasswordAvailable(), false);
    QCOMPARE(uut.isKeyAvailable(), false);
    QCOMPARE(uut.key(), nullptr);
    QVERIFY2(!uut.challenge(), "should not have a password challenge yet");

    // advance the state: request password
    QVERIFY2(uut.requestExistingPassword(*m_challenge, m_salt, *keyParams), "should be able to request a (existing) password");
    QVERIFY2(test::signal_eventually_emitted_once(existingPasswordNeeded), "request for (existing) password should be signalled");
    const auto suppliedChallenge = uut.challenge();
    QVERIFY2(suppliedChallenge, "should have the supplied password challenge by now");
    QCOMPARE(suppliedChallenge->cryptText(), m_challenge->cryptText());
    QCOMPARE(suppliedChallenge->nonce(), m_challenge->nonce());

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
    QCOMPARE(keyFailed.count(), 0);
    QCOMPARE(requestsCancelled.count(), 0);

    // advance the state: supply password
    QString password = QString::fromUtf8(masterPassword());
    QString wiped = QStringLiteral("*").repeated(password.size());

    QVERIFY2(uut.answerExistingPassword(password), "(existing) password should be accepted");
    QVERIFY2(test::signal_eventually_emitted_once(passwordAvailable), "availability of the (existing) password should be signalled");
    QCOMPARE(password, wiped);

    // check the state is correctly updated
    QCOMPARE(uut.isStillAlive(), true);
    QCOMPARE(uut.isNewPasswordRequested(), false);
    QCOMPARE(uut.isExistingPasswordRequested(), true);
    QCOMPARE(uut.isPasswordAvailable(), true);
    QCOMPARE(uut.isKeyAvailable(), false);
    QCOMPARE(uut.key(), nullptr);
    const auto preservedChallenge = uut.challenge();
    QVERIFY2(preservedChallenge, "should still have the same supplied password challenge after answering with a password");
    QCOMPARE(preservedChallenge->cryptText(), m_challenge->cryptText());
    QCOMPARE(preservedChallenge->nonce(), m_challenge->nonce());

    QCOMPARE(newPasswordNeeded.count(), 0);
    QCOMPARE(passwordAvailable.count(), 1);
    QCOMPARE(existingPasswordNeeded.count(), 1);
    QCOMPARE(keyAvailable.count(), 0);
    QCOMPARE(keyFailed.count(), 0);
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
    const auto finalChallenge = uut.challenge();
    QVERIFY2(finalChallenge, "should still have the same supplied password challenge after key derivation");
    QCOMPARE(finalChallenge->cryptText(), m_challenge->cryptText());
    QCOMPARE(finalChallenge->nonce(), m_challenge->nonce());

    QCOMPARE(newPasswordNeeded.count(), 0);
    QCOMPARE(passwordAvailable.count(), 1);
    QCOMPARE(existingPasswordNeeded.count(), 1);
    QCOMPARE(keyAvailable.count(), 1);
    QCOMPARE(keyFailed.count(), 0);
    QCOMPARE(requestsCancelled.count(), 0);
}

void PasswordFlowTest::retryExistingPassword(void)
{
    accounts::AccountSecret uut;
    QSignalSpy existingPasswordNeeded(&uut, &accounts::AccountSecret::existingPasswordNeeded);
    QSignalSpy newPasswordNeeded(&uut, &accounts::AccountSecret::newPasswordNeeded);
    QSignalSpy passwordAvailable(&uut, &accounts::AccountSecret::passwordAvailable);
    QSignalSpy requestsCancelled(&uut, &accounts::AccountSecret::requestsCancelled);
    QSignalSpy keyAvailable(&uut, &accounts::AccountSecret::keyAvailable);
    QSignalSpy keyFailed(&uut, &accounts::AccountSecret::keyFailed);

    // check correct initial state is reported
    QCOMPARE(uut.isStillAlive(), true);
    QCOMPARE(uut.isNewPasswordRequested(), false);
    QCOMPARE(uut.isExistingPasswordRequested(), false);
    QCOMPARE(uut.isPasswordAvailable(), false);
    QCOMPARE(uut.isKeyAvailable(), false);
    QCOMPARE(uut.key(), nullptr);
    QVERIFY2(!uut.challenge(), "should not have a password challenge yet");

    // advance the state: request password
    QVERIFY2(uut.requestExistingPassword(*m_challenge, m_salt, *keyParams), "should be able to request a (existing) password");
    QVERIFY2(test::signal_eventually_emitted_once(existingPasswordNeeded), "request for (existing) password should be signalled");
    const auto suppliedChallenge = uut.challenge();
    QVERIFY2(suppliedChallenge, "should have the supplied password challenge by now");
    QCOMPARE(suppliedChallenge->cryptText(), m_challenge->cryptText());
    QCOMPARE(suppliedChallenge->nonce(), m_challenge->nonce());

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
    QCOMPARE(keyFailed.count(), 0);
    QCOMPARE(requestsCancelled.count(), 0);

    // advance the state: supply wrong password
    QString wrongPassword(QStringLiteral("wrong"));
    QString wipedWrongPassword = QStringLiteral("*").repeated(wrongPassword.size());

    QVERIFY2(uut.answerExistingPassword(wrongPassword), "password attempt should be accepted");
    QVERIFY2(test::signal_eventually_emitted_once(passwordAvailable), "availability of an attempt should be signalled");
    QCOMPARE(wrongPassword, wipedWrongPassword);

    // advance the state: attempt to derive the master key
    QVERIFY2(!uut.deriveKey(), "key derivation should fail on wrong password");
    QVERIFY2(test::signal_eventually_emitted_once(keyFailed), "failure to derive the master key should be signalled");

    // check the state is correctly updated
    QCOMPARE(uut.isStillAlive(), true);
    QCOMPARE(uut.isNewPasswordRequested(), false);
    QCOMPARE(uut.isExistingPasswordRequested(), true);
    QCOMPARE(uut.isPasswordAvailable(), false);
    QCOMPARE(uut.isKeyAvailable(), false);
    QCOMPARE(uut.key(), nullptr);
    const auto stillPreservedChallenge = uut.challenge();
    QVERIFY2(stillPreservedChallenge, "should still have the same supplied password challenge after answering with a password");
    QCOMPARE(stillPreservedChallenge->cryptText(), m_challenge->cryptText());
    QCOMPARE(stillPreservedChallenge->nonce(), m_challenge->nonce());

    QCOMPARE(newPasswordNeeded.count(), 0);
    QCOMPARE(passwordAvailable.count(), 1);
    QCOMPARE(existingPasswordNeeded.count(), 1);
    QCOMPARE(keyAvailable.count(), 0);
    QCOMPARE(requestsCancelled.count(), 0);

    // advance the state: supply correct password
    QString correctPassword = QString::fromUtf8(masterPassword());
    QString wipedCorrectPassword = QStringLiteral("*").repeated(correctPassword.size());

    QVERIFY2(uut.answerExistingPassword(correctPassword), "(existing) password should be accepted");
    QVERIFY2(test::signal_eventually_emitted_twice(passwordAvailable), "availability of the (existing) password should be signalled");
    QCOMPARE(correctPassword, wipedCorrectPassword);

    // advance the state: attempt to derive the master key
    QVERIFY2(uut.deriveKey(), "key derivation should succeed");
    QVERIFY2(test::signal_eventually_emitted_once(keyAvailable), "availability of the master key should be signalled");

    // check the state is correctly updated
    QCOMPARE(uut.isStillAlive(), true);
    QCOMPARE(uut.isNewPasswordRequested(), false);
    QCOMPARE(uut.isExistingPasswordRequested(), true);
    QCOMPARE(uut.isPasswordAvailable(), false);
    QCOMPARE(uut.isKeyAvailable(), true);
    QVERIFY2(uut.key(), "should have a master key by now");
    const auto finalChallenge = uut.challenge();
    QVERIFY2(finalChallenge, "should still have the same supplied password challenge after key derivation");
    QCOMPARE(finalChallenge->cryptText(), m_challenge->cryptText());
    QCOMPARE(finalChallenge->nonce(), m_challenge->nonce());

    QCOMPARE(newPasswordNeeded.count(), 0);
    QCOMPARE(passwordAvailable.count(), 2);
    QCOMPARE(existingPasswordNeeded.count(), 1);
    QCOMPARE(keyAvailable.count(), 1);
    QCOMPARE(requestsCancelled.count(), 0);
}

QTEST_MAIN(PasswordFlowTest)

#include "account-secret-password-flow.moc"

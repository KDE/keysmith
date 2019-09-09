/*****************************************************************************
 * Copyright: 2019 Johan Ouwerkerk <jm.ouwerkerk@gmail.com>                  *
 *                                                                           *
 * This project is free software: you can redistribute it and/or modify      *
 * it under the terms of the GNU General Public License as published by      *
 * the Free Software Foundation, either version 3 of the License, or         *
 * (at your option) any later version.                                       *
 *                                                                           *
 * This project is distributed in the hope that it will be useful,           *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of            *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             *
 * GNU General Public License for more details.                              *
 *                                                                           *
 * You should have received a copy of the GNU General Public License         *
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.     *
 *                                                                           *
 ****************************************************************************/

#include "account.h"

#include <QTest>
#include <QtDebug>

class HOTPGeneratorSamplesTest: public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testDefaults(void);
    void testDefaults_data(void);
};

void HOTPGeneratorSamplesTest::testDefaults(void)
{
    /*
     * RFC test vector uses the key: 12345678901234567890
     * The secret value below is the bas32 encoded version of that
     */
    static QLatin1String secret("GEZDGNBVGY3TQOJQGEZDGNBVGY3TQOJQ");

    // doesn't really matter, just some random UUID
    static QUuid uuid("5df6378b-92b2-45c8-88bd-c5a178f7b538");

    Account a(uuid);
    a.setName(QLatin1String("RFC test vector sample"));
    a.setType(Account::TypeHOTP);
    a.setPinLength(6);
    a.setSecret(secret);

    QFETCH(quint64, counter);

    a.setCounter(counter);
    a.generate();

    QTEST(a.otp(), "rfc-test-vector");
}

static void define_test_case(int k, const char *expected)
{

    QByteArray output(expected, 6);

    QTest::newRow(qPrintable(QStringLiteral("RFC 4226 test vector, counter value = %1").arg(k))) << (quint64) k << QString::fromLocal8Bit(output);
}

void HOTPGeneratorSamplesTest::testDefaults_data(void)
{
    static const char * corpus[10] {
        "755224",
        "287082",
        "359152",
        "969429",
        "338314",
        "254676",
        "287922",
        "162583",
        "399871",
        "520489"
    };

    QTest::addColumn<quint64>("counter");
    QTest::addColumn<QString>("rfc-test-vector");

    for(int k = 0; k < 10; ++k) {
        define_test_case(k, corpus[k]);
    }
}

QTEST_APPLESS_MAIN(HOTPGeneratorSamplesTest)

#include "hotp-generator-samples.moc"

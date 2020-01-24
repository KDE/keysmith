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

#include "base32/base32.h"

#include <QTest>
#include <QtDebug>

Q_DECLARE_METATYPE(std::optional<QByteArray>);

class Base32DecodingTest: public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testSample(void);
    void testSample_data(void);
};

static void define_test_case(const char *testCase, const char *data, int length, const QString &base32)
{
    std::optional<QByteArray> msg = data ? std::optional<QByteArray>(QByteArray(data, length)) : std::nullopt;
    QTest::newRow(qPrintable(QLatin1String(testCase))) << msg << base32;
}

void Base32DecodingTest::testSample(void)
{

    QFETCH(QString, base32);
    QTEST(base32::decode(base32), "message");
}

void Base32DecodingTest::testSample_data(void)
{
    static const char ok_corpus[13][5] = {
        { 'A', 'B', 'C', 'D', '\xA' },
        { '?', 'A', 'B', 'C', 'D' },
        { '2', '0', '1', '6' },
        { '=', '=' },
        { '?' },
        { '8' },

        { '\x0', '\x1', '\x2' },
        { '\x1', '\x0', '\x2' },
        { '\x1', '\x2', '\x0' },


        { '\x0', 'A', 'B', '\x1', '\x2' },
        { '\x1', 'A', 'B', '\x0', '\x2' },
        { '\x1', 'A', 'B', '\x2', '\x0' },

        {}
    };

    QTest::addColumn<std::optional<QByteArray>>("message");
    QTest::addColumn<QString>("base32");

    define_test_case("'ABCD\\n'", ok_corpus[0], 5, QLatin1String("IFBEGRAK"));
    define_test_case("'?ABCD'", ok_corpus[1], 5, QLatin1String("H5AUEQ2E"));
    define_test_case("'2016'", ok_corpus[2], 4, QLatin1String("GIYDCNQ="));
    define_test_case("'=='", ok_corpus[3], 2, QLatin1String("HU6Q===="));
    define_test_case("'?'", ok_corpus[4], 1, QLatin1String("H4======"));
    define_test_case("'8'", ok_corpus[5], 1, QLatin1String("HA======"));

    define_test_case("'\\x0\\x1\\x2'", ok_corpus[6], 3, QLatin1String("AAAQE==="));
    define_test_case("'\\x1\\x0\\x2'", ok_corpus[7], 3, QLatin1String("AEAAE==="));
    define_test_case("'\\x1\\x2\\x0'", ok_corpus[8], 3, QLatin1String("AEBAA==="));

    define_test_case("'\\x0AB\\x1\\x2'", ok_corpus[9], 5, QLatin1String("ABAUEAIC"));
    define_test_case("'\\x1AB\\x0\\x2'", ok_corpus[10], 5, QLatin1String("AFAUEAAC"));
    define_test_case("'\\x1AB\\x2\\x0'", ok_corpus[11], 5, QLatin1String("AFAUEAQA"));

    define_test_case("''", ok_corpus[12], 0, QLatin1String(""));

    define_test_case("without any padding", NULL, 0, QLatin1String("ZZ"));
    define_test_case("too little padding", NULL, 0, QLatin1String("ZZ==="));
    define_test_case("padding only", NULL, 0, QLatin1String("========"));
    define_test_case("embedded spaces", NULL,  0, QLatin1String("ZZ \n===="));
    define_test_case("invalid base32 (1)", NULL, 0, QLatin1String("1AABBCCD"));
    define_test_case("invalid base32 (8)", NULL, 0, QLatin1String("AABBCC8D"));
    define_test_case("invalid base32 (@)", NULL, 0, QLatin1String("AABBCCD@"));
}


QTEST_APPLESS_MAIN(Base32DecodingTest)

#include "base32-decode.moc"

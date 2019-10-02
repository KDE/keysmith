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

#ifndef VALIDATOR_TEST_UTIL_H
#define VALIDATOR_TEST_UTIL_H

#include <QLocale>
#include <QObject>
#include <QTest>
#include <QValidator>
#include <QtDebug>

Q_DECLARE_METATYPE(QValidator::State);
Q_DECLARE_METATYPE(QLocale);

namespace validators
{
    namespace test
    {
        void define_test_data_columns(void)
        {
            QTest::addColumn<QString>("input");
            QTest::addColumn<QString>("fixed");
            QTest::addColumn<int>("cursor");
            QTest::addColumn<QLocale>("locale");
            QTest::addColumn<QValidator::State>("result");

        };

        void define_test_case(
            const QString &input,
            const QString &fixed,
            int cursor=0,
            QValidator::State result=QValidator::Intermediate,
            const QLocale &locale=QLocale::c())
        {
            QString name = QStringLiteral("locale=%1, input=%2").arg(locale.name()).arg(input);
            QTest::newRow(qPrintable(name)) << input << fixed << cursor << locale << result;
        };

        template<class T, auto f_fixup_data, auto f_validate_data>
        class ValidatorTestBase : public QObject
        {
        public:
            virtual ~ValidatorTestBase()
            {
            };

            void testValidate(void)
            {
                T uut;
                QFETCH(QLocale, locale);
                uut.setLocale(locale);

                QFETCH(QString, input);
                QFETCH(QString, fixed);
                QFETCH(int, cursor);

                int position = input.size();

                QTEST(uut.validate(input, position), "result");
                QCOMPARE(input, fixed);
                QCOMPARE(position, cursor);
            };

            void testFixup(void)
            {
                T uut;
                QFETCH(QLocale, locale);
                uut.setLocale(locale);

                QFETCH(QString, input);

                uut.fixup(input);
                QTEST(input, "fixed");

                // test if output is stable
                // ie.: fixup(fixup()) == fixup()
                uut.fixup(input);
                QTEST(input, "fixed");
            };

            void testValidate_data(void)
            {
                define_test_data_columns();
                f_validate_data();
            };

            void testFixup_data(void)
            {
                define_test_data_columns();
                f_fixup_data();
            };
        };
    }
}

#define DEFINE_VALIDATOR_TEST(name, type, fixup_data, validate_data) \
class name : public validators::test::ValidatorTestBase<type, fixup_data, validate_data> \
{ \
    Q_OBJECT \
private Q_SLOTS: \
    void testFixup(void) \
    { \
        ValidatorTestBase::testFixup(); \
    }; \
    \
    void testValidate(void) \
    { \
        ValidatorTestBase::testValidate(); \
    }; \
    \
    void testFixup_data(void) \
    { \
        ValidatorTestBase::testFixup_data(); \
    }; \
    \
    void testValidate_data(void) \
    { \
        ValidatorTestBase::testValidate_data(); \
    }; \
}

#endif

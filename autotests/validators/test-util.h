/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 * SPDX-FileCopyrightText: 2019 Johan Ouwerkerk <jm.ouwerkerk@gmail.com>
 */

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
    QTest::addColumn<QLocale>("locale");
    QTest::addColumn<QValidator::State>("result");
};

void define_test_case(const QString &input, const QString &fixed, QValidator::State result = QValidator::Intermediate, const QLocale &locale = QLocale::c())
{
    QString name = QStringLiteral("locale=%1, input=%2").arg(locale.name()).arg(input);
    QTest::newRow(qPrintable(name)) << input << fixed << locale << result;
};

template<class T, auto f_data>
class ValidatorTestBase : public QObject
{
public:
    virtual ~ValidatorTestBase() {};

    void testValidate(T &uut)
    {
        QFETCH(QString, input);
        QFETCH(QLocale, locale);

        uut.setLocale(locale);
        int position = input.size();
        int copy = position;

        QTEST(uut.validate(input, position), "result");
        QCOMPARE(position, copy);
    };

    void testFixup(T &uut)
    {
        QFETCH(QString, input);
        QFETCH(QLocale, locale);

        uut.setLocale(locale);

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
        f_data();
    };

    void testFixup_data(void)
    {
        define_test_data_columns();
        f_data();
    };
};
}
}

#define DEFINE_VALIDATOR_TEST(name, type, data_tables, ...)                                                                                                    \
    class name : public validators::test::ValidatorTestBase<type, data_tables>                                                                                 \
    {                                                                                                                                                          \
        Q_OBJECT                                                                                                                                               \
    private Q_SLOTS:                                                                                                                                           \
        void testFixup(void)                                                                                                                                   \
        {                                                                                                                                                      \
            type uut{__VA_ARGS__};                                                                                                                             \
            ValidatorTestBase::testFixup(uut);                                                                                                                 \
        };                                                                                                                                                     \
                                                                                                                                                               \
        void testValidate(void)                                                                                                                                \
        {                                                                                                                                                      \
            type uut{__VA_ARGS__};                                                                                                                             \
            ValidatorTestBase::testValidate(uut);                                                                                                              \
        };                                                                                                                                                     \
                                                                                                                                                               \
        void testFixup_data(void)                                                                                                                              \
        {                                                                                                                                                      \
            ValidatorTestBase::testFixup_data();                                                                                                               \
        };                                                                                                                                                     \
                                                                                                                                                               \
        void testValidate_data(void)                                                                                                                           \
        {                                                                                                                                                      \
            ValidatorTestBase::testValidate_data();                                                                                                            \
        };                                                                                                                                                     \
    }

#endif

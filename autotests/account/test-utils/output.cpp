/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 * SPDX-FileCopyrightText: 2020 Johan Ouwerkerk <jm.ouwerkerk@gmail.com>
 */
#include "output.h"

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QTextStream>
#include <QtDebug>

static QString outputDirBaseName(QLatin1String("output"));
static QString resourceDirBaseName(QLatin1String("resources"));

namespace test
{
bool ensureOutputDirectory(void)
{
    QDir base(QCoreApplication::applicationDirPath());
    return base.mkpath(outputDirBaseName) && base.cd(outputDirBaseName) && base.mkpath(QCoreApplication::applicationName());
}

bool copyResource(const QString &resource, const QString &outputRelPath)
{
    QFile source(resource);
    if (ensureOutputDirectory() && source.exists()) {
        QDir base(QCoreApplication::applicationDirPath());

        if (base.cd(outputDirBaseName) && base.cd(QCoreApplication::applicationName())) {
            QString dest = base.absoluteFilePath(outputRelPath);
            return (!base.exists(dest) || base.remove(dest)) && source.copy(dest);
        }
    }

    return false;
}

bool ensureWritable(const QString &outputRelPath)
{
    QFile result(test::path(outputRelPath));
    return result.exists() && result.setPermissions(result.permissions() | QFileDevice::WriteOwner | QFileDevice::WriteUser);
}

bool copyResourceAsWritable(const QString &resource, const QString &outputRelPath)
{
    return copyResource(resource, outputRelPath) && ensureWritable(outputRelPath);
}

QString path(const QString &relPath)
{
    QDir base(QCoreApplication::applicationDirPath());
    base.cd(outputDirBaseName);
    base.cd(QCoreApplication::applicationName());
    return base.absoluteFilePath(relPath);
}

QString slurp(const QString &path)
{
    QFile source(path);
    bool opened = source.open(QIODevice::ReadOnly | QIODevice::Text);
    if (opened) {
        QTextStream input(&source);
        QString result = input.readAll();
        source.close();
        return result;
    } else {
        qDebug() << "Failed to open:" << path;
        return QString();
    }
}
}

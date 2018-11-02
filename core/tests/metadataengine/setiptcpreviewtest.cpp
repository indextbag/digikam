/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-02-04
 * Description : an unit-test to set IPTC Preview
 *
 * Copyright (C) 2009-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#include "setiptcpreviewtest.h"

// Qt includes

#include <QDebug>
#include <QTest>
#include <QFile>
#include <QImage>

// Local includes

#include "dmetadata.h"
#include "wstoolutils.h"

using namespace Digikam;

QTEST_MAIN(SetIptcPreviewTest)

QDir          s_tempDir;
QString       s_tempPath;
const QString s_originalImageFolder(QFINDTESTDATA("data/"));

void SetIptcPreviewTest::initTestCase()
{
    MetaEngine::initializeExiv2();
    qDebug() << "Using Exiv2 Version:" << MetaEngine::Exiv2Version();
    s_tempPath = QString::fromLatin1(QTest::currentAppName());
    s_tempPath.replace(QLatin1String("./"), QString());
}

void SetIptcPreviewTest::init()
{
    s_tempDir = WSToolUtils::makeTemporaryDir(s_tempPath.toLatin1().data());
}

void SetIptcPreviewTest::cleanup()
{
    WSToolUtils::removeTemporaryDir(s_tempPath.toLatin1().data());
}

void SetIptcPreviewTest::cleanupTestCase()
{
    MetaEngine::cleanupExiv2();
}

void SetIptcPreviewTest::testSetIptcPreview()
{
    setIptcPreview(s_originalImageFolder + QLatin1String("2015-07-22_00001.JPG"));
}

void SetIptcPreviewTest::setIptcPreview(const QString& file)
{
    qDebug() << "File to process:" << file;
    bool ret     = false;

    QString path = s_tempDir.filePath(QFileInfo(file).fileName().trimmed());

    qDebug() << "Temporary target file:" << path;

    ret = !path.isNull();
    QVERIFY(ret);

    QFile::remove(path);
    QFile target(file);
    ret = target.copy(path);
    QVERIFY(ret);

    QImage  preview;
    QImage  image(path);
    QVERIFY(!image.isNull());

    QSize previewSize = image.size();
    previewSize.scale(1280, 1024, Qt::KeepAspectRatio);

    // Ensure that preview is not upscaled
    if (previewSize.width() >= (int)image.width())
        preview = image.copy();
    else
        preview = image.scaled(previewSize.width(), previewSize.height(), Qt::IgnoreAspectRatio).copy();

    QVERIFY(!preview.isNull());

    DMetadata meta;
    ret = meta.load(path);
    QVERIFY(ret);

    meta.setImagePreview(preview);
    ret = meta.applyChanges();
    QVERIFY(ret);

    QImage preview2;
    DMetadata meta2;
    ret = meta2.load(path);
    QVERIFY(ret);

    ret = meta2.getImagePreview(preview2);
    QVERIFY(ret);

    QVERIFY(!preview2.isNull());

    QCOMPARE(preview.size(), preview2.size());
}

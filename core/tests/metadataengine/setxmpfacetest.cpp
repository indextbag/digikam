/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2013-02-21
 * Description : an unit-test to set and clear faces in Picassa format with DMetadata
 *
 * Copyright (C) 2013 by Veaceslav Munteanu <veaceslav dot munteanu90 at gmail dot com>
 * Copyright (C) 2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "setxmpfacetest.h"

// Qt includes

#include <QFile>
#include <QMultiMap>
#include <QRectF>

QTEST_MAIN(SetXmpFaceTest)

void SetXmpFaceTest::testSetXmpFace()
{
    setXmpFace(m_originalImageFolder + QLatin1String("nikon-e2100.jpg"));
}
    
void SetXmpFaceTest::setXmpFace(const QString& file)
{
    qDebug() << "File to process:          " << file;

    QString filePath = m_tempDir.filePath(QFileInfo(file).fileName().trimmed());

    qDebug() << "Temporary target file:    " << filePath;

    bool ret = !filePath.isNull();
    QVERIFY(ret);

    // Copy image file in temporary dir.
    QFile::remove(filePath);
    QFile target(file);
    ret = target.copy(filePath);
    QVERIFY(ret);

    DMetadata meta;
    ret = meta.load(filePath);
    QVERIFY(ret);

    // Add random rectangles with facetags

    QMultiMap<QString, QVariant> faces;

    QString name  = QLatin1String("Bob Marley");
    QRectF rect(10, 10, 60, 60);
    faces.insert(name, QVariant(rect));

    QString name2 = QLatin1String("Alice in wonderland");
    QRectF rect2(20, 20, 30, 30);
    faces.insert(name2, QVariant(rect2));

    ret = meta.setImageFacesMap(faces, true);
    QVERIFY(ret);

    ret = meta.applyChanges();
    QVERIFY(ret);

    // Check if face tags are well assigned.

    DMetadata meta2;
    ret = meta2.load(filePath);
    QVERIFY(ret);

    QMultiMap<QString, QVariant> faces2;
    ret = meta2.getImageFacesMap(faces2);
    QVERIFY(ret);

    QVERIFY(!faces2.isEmpty());
    QVERIFY(faces2.contains(name));
    QVERIFY(faces2.contains(name2));
    QVERIFY(faces2.value(name)  == rect);
    QVERIFY(faces2.value(name2) == rect2);

    // Now clear face tags and check if well removed.

    QMultiMap<QString, QVariant> faces3;

    ret = meta2.setImageFacesMap(faces3, true);
    QVERIFY(ret);

    ret = meta2.applyChanges();
    QVERIFY(ret);

    DMetadata meta3;
    ret = meta3.load(filePath);
    QVERIFY(ret);

    QMultiMap<QString, QVariant> faces4;
    ret = meta3.getImageFacesMap(faces4);
    QVERIFY(ret);
    QVERIFY(faces4.isEmpty());
}

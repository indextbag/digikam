/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2017-09-24
 * Description : Test ImageMagick loader to QImage.
 *
 * Copyright (C) 2019 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

// IM includes

#include <Magick++.h>
using namespace Magick;

#if MagickLibVersion < 0x700
#   include <magick/magick.h>
#endif

// Qt includes

#include <QImage>
#include <QPixmap>
#include <QLabel>
#include <QDir>
#include <QStringList>
#include <QList>
#include <QDebug>
#include <QApplication>
#include <QStandardPaths>
#include <QFileDialog>
#include <QMimeDatabase>

/** Convert from QImage to IM::Image
 */
/*Image* toImage(QImage* const qimage)
{
    qDebug() << "toImage:" << qimage->width() << qimage->height();

    Image* const newImage = new Image(Magick::Geometry(qimage->width(), qimage->height()), Magick::ColorRGB(0.5, 0.2, 0.3));
    newImage->modifyImage();

    double scale                = 1 / 256.0;
    Magick::Quantum* pixels = 0;
    Magick::ColorRGB mgc;

    for (int y = 0 ; y < qimage->height() ; ++y)
    {
        pixels = newImage->setPixels(0, y, newImage->columns(), 1);

        for (int x = 0 ; x < qimage->width() ; ++x)
        {
            QColor pix = qimage->pixel(x, y);
            mgc.red  (scale * pix.red());
            mgc.green(scale * pix.green());
            mgc.blue (scale * pix.blue());
            *pixels++ = mgc;
        }

        newImage->syncPixels();
    }

    return newImage;
}
*/
/** Convert pixel from IM::Image to QImage.
 */

// -- ImageMagick codecs to QImage --------------------------------------------------------

bool loadWithImageMagick(const QString& path, QImage& qimg)
{
    qDebug() << "Try to load image with ImageMagick codecs";

    try
    {
        Image image;
        image.read(path.toUtf8().constData());

        qDebug() << "IM toQImage     :" << image.columns() << image.rows();
        qDebug() << "IM QuantumRange :" << QuantumRange;

        Blob* const pixelBlob = new Blob;
        image.write(pixelBlob, "BGRA", 8);
        qDebug() << "IM blob size    :" << pixelBlob->length();

        qimg = QImage((uchar*)pixelBlob->data(), image.columns(), image.rows(), QImage::Format_ARGB32);
        qDebug() << "QImage data size:" << qimg.byteCount();

        if (qimg.isNull())
        {
            return false;
        }
    }
    catch (Exception& error_)
    {
        qWarning() << "Cannot load [" << path << "] due to ImageMagick exception:" << error_.what();
        qimg = QImage();
        return false;
    }

    return true;
}

// ---------------------------------------------------------------------------------------------------

int main(int argc, char** argv)
{
    MagickCore::ExceptionInfo ex;
    size_t n                              = 0;
    const MagickCore::MagickInfo** inflst = MagickCore::GetMagickInfoList("*", &n, &ex);
    qDebug().noquote() << "Name             :: Module           :: Mime Type                    :: Mode :: Version                      :: Description";

    for (uint i = 0 ; i < n ; ++i)
    {
        const MagickCore::MagickInfo* inf = inflst[i];
        QString mode;

        if (inf->decoder) mode.append(QLatin1Char('R'));
        else              mode.append(QLatin1Char('-'));

        if (inf->encoder) mode.append(QLatin1Char('W'));
        else              mode.append(QLatin1Char('-'));

        QString module = QLatin1String(inf->module);
        QString mime   = QMimeDatabase().mimeTypeForFile(QFileInfo(QString::fromLatin1("foo.%1").arg(module))).name();

        if (module != QLatin1String("DNG")  &&
            module != QLatin1String("JPEG") &&
            module != QLatin1String("PNG")  &&
            module != QLatin1String("TIFF") &&
            module != QLatin1String("JP2")  &&
            mime.startsWith(QLatin1String("image/")))
        {
            qDebug().noquote()
                 << QString::fromLatin1("%1").arg(QLatin1String(inf->name),        16) << "::"
                 << QString::fromLatin1("%1").arg(module,                          16) << "::"
                 << QString::fromLatin1("%1").arg(mime,                            28) << "::"
                 << QString::fromLatin1("%1").arg(mode,                             5) << "::"
                 << QString::fromLatin1("%1").arg(QLatin1String(inf->version),     28) << "::"
                 << QString::fromLatin1("%1").arg(QLatin1String(inf->description), 64);
        }
    }

    QApplication app(argc, argv);

    QStringList list;

    if (argc <= 1)
    {
        list = QFileDialog::getOpenFileNames(nullptr, QString::fromLatin1("Select Image Files to Load"),
                                             QStandardPaths::standardLocations(QStandardPaths::PicturesLocation).first(),
                                             QLatin1String("Image Files (*.png *.jpg *.tif *.bmp *.gif *.xcf *.kra *.psd)"));
    }
    else
    {
        for (int i = 1 ; i < argc ; ++i)
        {
            list.append(QString::fromLocal8Bit(argv[i]));
        }
    }

    qDebug() << "Files to load:" << list;

    if (!list.isEmpty())
    {
        foreach (const QString& path, list)
        {
            QImage qimg;
            bool ret = loadWithImageMagick(path, qimg);

            if (ret)
            {
                QLabel* const lbl  = new QLabel;
                lbl->setPixmap(QPixmap::fromImage(qimg.scaled(512, 512, Qt::KeepAspectRatio)));
                lbl->show();
            }
            else
            {
                qWarning() << "exit -1";
                return -1;
            }
        }

        app.exec();
    }

    return 0;
}

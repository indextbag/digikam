/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-11-01
 * Description : A digital camera RAW files loader for DImg
 *               framework using an external dcraw instance.
 *
 * Copyright (C) 2005-2010 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2005-2010 by Marcel Wiesweg <marcel.wiesweg@gmx.de>
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

#include "rawloader.moc"

// C++ includes

#include <cmath>

// Qt includes

#include <QByteArray>

// KDE includes

#include <kstandarddirs.h>
#include <kdebug.h>

// Local includes

#include "dimg.h"
#include "dimgloaderobserver.h"
#include "icctransform.h"
#include "imagehistogram.h"
#include "imagecurves.h"
#include "imagelevels.h"
#include "bcgfilter.h"
#include "whitebalance.h"
#include "globals.h"

namespace Digikam
{

RAWLoader::RAWLoader(DImg* image, DRawDecoding rawDecodingSettings)
         : DImgLoader(image)
{
    m_rawDecodingSettings = rawDecodingSettings;
    m_customRawSettings   = rawDecodingSettings;
    m_observer            = 0;
}

bool RAWLoader::load(const QString& filePath, DImgLoaderObserver *observer)
{
    m_observer = observer;

    readMetadata(filePath, DImg::RAW);

    // NOTE: Here, we don't check a possible embedded work-space color profile using
    // the method checkExifWorkingColorSpace() like with JPEG, PNG, and TIFF loaders,
    // because RAW file are always in linear mode.

    if (m_loadFlags & LoadImageData)
    {
        int        width, height, rgbmax;
        QByteArray data;

        if (m_rawDecodingSettings.outputColorSpace == DRawDecoding::CUSTOMOUTPUTCS)
        {
            if (m_rawDecodingSettings.outputProfile == IccProfile::sRGB().filePath())
                m_rawDecodingSettings.outputColorSpace = DRawDecoding::SRGB;
            else if (m_rawDecodingSettings.outputProfile == IccProfile::adobeRGB().filePath())
                m_rawDecodingSettings.outputColorSpace = DRawDecoding::ADOBERGB;
            else if (m_rawDecodingSettings.outputProfile == IccProfile::wideGamutRGB().filePath())
                m_rawDecodingSettings.outputColorSpace = DRawDecoding::WIDEGAMMUT;
            else if (m_rawDecodingSettings.outputProfile == IccProfile::proPhotoRGB().filePath())
                m_rawDecodingSettings.outputColorSpace = DRawDecoding::PROPHOTO;
            else
            {
                // Specifying a custom output is broken somewhere. We use the extremely
                // wide gamut pro photo profile for 16bit (sRGB for 8bit) and convert afterwards.
                m_customOutputProfile = m_rawDecodingSettings.outputProfile;
                if (m_rawDecodingSettings.sixteenBitsImage)
                    m_rawDecodingSettings.outputColorSpace = DRawDecoding::PROPHOTO;
                else
                    m_rawDecodingSettings.outputColorSpace = DRawDecoding::SRGB;
            }
        }

        if (!KDcrawIface::KDcraw::decodeRAWImage(filePath, m_rawDecodingSettings,
             data, width, height, rgbmax))
            return false;

        return (loadedFromDcraw(data, width, height, rgbmax, observer));
    }
    else
    {
        KDcrawIface::DcrawInfoContainer dcrawIdentify;
        if (!KDcrawIface::KDcraw::rawFileIdentify(dcrawIdentify, filePath))
            return false;
        imageWidth()  = dcrawIdentify.imageSize.width();
        imageHeight() = dcrawIdentify.imageSize.height();
        imageSetAttribute("format", "RAW");
        imageSetAttribute("originalColorModel", DImg::COLORMODELRAW);
        imageSetAttribute("originalBitDepth", 16);
        return true;
    }
}

bool RAWLoader::checkToCancelWaitingData()
{
    return (m_observer ? !m_observer->continueQuery(m_image) : false);
}

void RAWLoader::setWaitingDataProgress(double value)
{
    if (m_observer)
        m_observer->progressInfo(m_image, value);
}

#if KDCRAW_VERSION < 0x000400
bool RAWLoader::checkToCancelReceivingData()
{
    return (m_observer ? m_observer->isShuttingDown() : false);
}

void RAWLoader::setReceivingDataProgress(double value)
{
    if (m_observer)
        m_observer->progressInfo(m_image, value);
}
#endif

bool RAWLoader::loadedFromDcraw(QByteArray data, int width, int height, int rgbmax,
                                DImgLoaderObserver *observer)
{
    int checkpoint = 0;

    if (m_rawDecodingSettings.sixteenBitsImage)       // 16 bits image
    {
        uchar *image = new_failureTolerant(width*height*8);
        if (!image)
        {
            kDebug() << "Failed to allocate memory for loading raw file";
            return false;
        }

        unsigned short *dst = (unsigned short *)image;
        uchar          *src = (uchar*)data.data();
        float fac           = 65535.0 / rgbmax;
        checkpoint          = 0;

        for (int h = 0; h < height; ++h)
        {
            if (observer && h == checkpoint)
            {
                checkpoint += granularity(observer, height, 1.0);
                if (!observer->continueQuery(m_image))
                {
                    return false;
                }
                observer->progressInfo(m_image, 0.7 + 0.2*(((float)h)/((float)height)) );
            }

            for (int w = 0; w < width; ++w)
            {
                if (QSysInfo::ByteOrder == QSysInfo::LittleEndian)     // Intel
                {
                    #if KDCRAW_VERSION < 0x000400
                    dst[0] = (unsigned short)((src[4]*256 + src[5]) * fac);      // Blue
                    dst[1] = (unsigned short)((src[2]*256 + src[3]) * fac);      // Green
                    dst[2] = (unsigned short)((src[0]*256 + src[1]) * fac);      // Red
                    #else
                    dst[0] = (unsigned short)((src[5]*256 + src[4]) * fac);      // Blue
                    dst[1] = (unsigned short)((src[3]*256 + src[2]) * fac);      // Green
                    dst[2] = (unsigned short)((src[1]*256 + src[0]) * fac);      // Red
                    #endif
                }
                else
                {
                    dst[0] = (unsigned short)((src[4]*256 + src[5]) * fac);      // Blue
                    dst[1] = (unsigned short)((src[2]*256 + src[3]) * fac);      // Green
                    dst[2] = (unsigned short)((src[0]*256 + src[1]) * fac);      // Red
                }
                dst[3] = 0xFFFF;

                dst += 4;
                src += 6;
            }
        }

#if KDCRAW_VERSION < 0x000400
        // ----------------------------------------------------------

        // Special case: if Color Management is not used here, output color space is in sRGB* color space
        // RAW decoded image is a linear-histogram image with 16 bits color depth.
        // No auto white balance and no gamma adjustments are performed. Image is a black hole.
        // We need to reproduce all dcraw 8 bits color depth adjustments here.

        if (m_rawDecodingSettings.outputColorSpace != DRawDecoding::RAWCOLOR)
        {
            ImageHistogram histogram(image, width, height, true);
            histogram.calculate();

            int perc, val, total;
            float white=0.0, r, gamma=2.222222;
            unsigned short lut[65536];

            // Search 99th percentile white level.

            perc = (int)(width * height * 0.01);
            kDebug() << "White Level: " << perc;
            for (int c = 1 ; c < 4 ; ++c)
            {
                total = 0;
                for (val = 65535 ; val > 256 ; --val)
                    if ((total += (int)histogram.getValue(c, val)) > perc)
                        break;

                if (white < val) white = (float)val;
            }

            white *= 1.0 / m_rawDecodingSettings.brightness;

            kDebug() << "White Point: " << white;

            // Compute the Gamma lut accordingly.

            for (int i=0; i < 65536; ++i)
            {
                r = i / white;
                val = (int)(65536.0 * (r <= 0.018 ? r*4.5 : pow(r, 1.0/gamma) * 1.099-0.099));
                if (val > 65535) val = 65535;
                lut[i] = val;
            }

            //  Apply Gamma lut to the whole image.

            unsigned short *im = (unsigned short *)image;
            for (int i = 0; i < width*height; ++i)
            {
                im[0] = lut[im[0]];      // Blue
                im[1] = lut[im[1]];      // Green
                im[2] = lut[im[2]];      // Red
                im += 4;
            }
        }
#endif
        // ----------------------------------------------------------

        imageData() = (uchar *)image;
    }
    else        // 8 bits image
    {
        uchar *image = new_failureTolerant(width*height*4);
        if (!image)
        {
            kDebug() << "Failed to allocate memory for loading raw file";
            return false;
        }

        uchar *dst   = image;
        uchar *src   = (uchar*)data.data();
        checkpoint   = 0;

        for (int h = 0; h < height; ++h)
        {

            if (observer && h == checkpoint)
            {
                checkpoint += granularity(observer, height, 1.0);
                if (!observer->continueQuery(m_image))
                {
                    return false;
                }
                observer->progressInfo(m_image, 0.7 + 0.2*(((float)h)/((float)height)) );
            }

            for (int w = 0; w < width; ++w)
            {
                // No need to adapt RGB components accordingly with rgbmax value because dcraw
                // always return rgbmax to 255 in 8 bits/color/pixels.

                dst[0] = src[2];    // Blue
                dst[1] = src[1];    // Green
                dst[2] = src[0];    // Red
                dst[3] = 0xFF;      // Alpha

                dst += 4;
                src += 3;
            }
        }

        // NOTE: if Color Management is not used here, output color space is in sRGB* color space.
        // Gamma and White balance are previously adjusted by dcraw in 8 bits color depth.

        imageData() = image;
    }

    //----------------------------------------------------------
    // Assign the right color-space profile.

    switch(m_rawDecodingSettings.outputColorSpace)
    {
        case DRawDecoding::SRGB:
        {
            imageSetIccProfile(IccProfile::sRGB());
            break;
        }
        case DRawDecoding::ADOBERGB:
        {
            imageSetIccProfile(IccProfile::adobeRGB());
            break;
        }
        case DRawDecoding::WIDEGAMMUT:
        {
            imageSetIccProfile(IccProfile::wideGamutRGB());
            break;
        }
        case DRawDecoding::PROPHOTO:
        {
            imageSetIccProfile(IccProfile::proPhotoRGB());
            break;
        }
        case DRawDecoding::CUSTOMOUTPUTCS:
        {
            imageSetIccProfile(m_rawDecodingSettings.outputProfile);
            break;
        }
        case DRawDecoding::RAWCOLOR:
        {
            // No icc color-space profile to assign in RAW color mode.
            imageSetAttribute("uncalibratedColor", true);
            break;
        }
    }

    //----------------------------------------------------------

    imageWidth()  = width;
    imageHeight() = height;
    imageSetAttribute("format", "RAW");
    imageSetAttribute("rawDecodingSettings", QVariant::fromValue(m_customRawSettings));
    imageSetAttribute("originalColorModel", DImg::COLORMODELRAW);
    imageSetAttribute("originalBitDepth", 16);

    return true;
}

void RAWLoader::postProcess(DImgLoaderObserver *observer)
{
    // emulate LibRaw custom output profile
    if (!m_customOutputProfile.isNull())
    {
        // Note the m_image is not yet ready in load()!
        IccTransform trans;
        trans.setIntent(IccTransform::Perceptual);
        trans.setEmbeddedProfile(*m_image);
        trans.setOutputProfile(m_customOutputProfile);
        trans.apply(*m_image, observer);
        imageSetIccProfile(m_customOutputProfile);
    }

    if (!m_customRawSettings.postProcessingSettingsIsDirty())
        return;

    if (m_customRawSettings.exposureComp != 0.0 || m_customRawSettings.saturation != 1.0)
    {
        WhiteBalance wb(m_rawDecodingSettings.sixteenBitsImage);
        wb.whiteBalance(imageData(), imageWidth(), imageHeight(), m_rawDecodingSettings.sixteenBitsImage,
                        0.0,                                // black
                        m_customRawSettings.exposureComp,   // exposure
                        6500.0,                             // temperature (neutral)
                        1.0,                                // green
                        0.5,                                // dark
                        1.0,                                // gamma
                        m_customRawSettings.saturation);    // saturation
    }
    if (observer) observer->progressInfo(m_image, 0.92F);

    if (m_customRawSettings.lightness != 0.0 ||
        m_customRawSettings.contrast  != 1.0 ||
        m_customRawSettings.gamma     != 1.0)
    {
        BCGContainer settings;
        settings.brightness = m_customRawSettings.lightness;
        settings.contrast   = m_customRawSettings.contrast;
        settings.gamma      = m_customRawSettings.gamma;
        BCGFilter bcg(imageData(), imageWidth(), imageHeight(), m_rawDecodingSettings.sixteenBitsImage, settings);
    }
    if (observer) observer->progressInfo(m_image, 0.94F);

    if (!m_customRawSettings.curveAdjust.isEmpty())
    {
        DImg tmp(imageWidth(), imageHeight(), m_rawDecodingSettings.sixteenBitsImage);
        ImageCurves curves(m_rawDecodingSettings.sixteenBitsImage);
        curves.setCurvePoints(LuminosityChannel, m_customRawSettings.curveAdjust);
        curves.curvesCalculateCurve(LuminosityChannel);
        curves.curvesLutSetup(AlphaChannel);
        curves.curvesLutProcess(imageData(), tmp.bits(), imageWidth(), imageHeight());
        memcpy(imageData(), tmp.bits(), tmp.numBytes());
    }
    if (observer) observer->progressInfo(m_image, 0.96F);

    if (!m_customRawSettings.levelsAdjust.isEmpty())
    {
        DImg tmp(imageWidth(), imageHeight(), m_rawDecodingSettings.sixteenBitsImage);
        ImageLevels levels(m_rawDecodingSettings.sixteenBitsImage);
        int j=0;
        for (int i = 0 ; i < 4; ++i)
        {
            levels.setLevelLowInputValue(i,   m_customRawSettings.levelsAdjust[j++]);
            levels.setLevelHighInputValue(i,  m_customRawSettings.levelsAdjust[j++]);
            levels.setLevelLowOutputValue(i,  m_customRawSettings.levelsAdjust[j++]);
            levels.setLevelHighOutputValue(i, m_customRawSettings.levelsAdjust[j++]);
        }

        levels.levelsLutSetup(AlphaChannel);
        levels.levelsLutProcess(imageData(), tmp.bits(), imageWidth(), imageHeight());
        memcpy(imageData(), tmp.bits(), tmp.numBytes());
    }
    if (observer) observer->progressInfo(m_image, 0.98F);
}

}  // namespace Digikam

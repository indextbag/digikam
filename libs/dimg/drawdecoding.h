/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2008-08-06
 * Description : Raw decoding settings for digiKam:
 *               standard libkdcraw parameters plus
 *               few customized for post processing.
 *
 * Copyright (C) 2008-2010 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef DRAW_DECODING_H
#define DRAW_DECODING_H

// Qt includes

#include <QList>
#include <QMetaType>
#include <QPolygon>

// LibKDcraw includes

#include <libkdcraw/rawdecodingsettings.h>

// Local includes

#include "digikam_export.h"

using namespace KDcrawIface;

namespace Digikam
{

class DIGIKAM_EXPORT DRawDecoding : public RawDecodingSettings
{

public:

    /** Standard constructor with default settings 
     */
    DRawDecoding()
    {
        resetPostProcessingSettings();
    };

    /** Standard destructor 
     */
    virtual ~DRawDecoding(){};

    /** Method to use a settings to optimize time loading, for example to compute image histogram 
     */
    void optimizeTimeLoading()
    {
        KDcrawIface::RawDecodingSettings::optimizeTimeLoading();
        resetPostProcessingSettings();
    };

    /** Method to reset to default values all Raw processing settings.
     */
    void resetPostProcessingSettings()
    {
        lightness    = 0.0;
        contrast     = 1.0;
        gamma        = 1.0;
        saturation   = 1.0;
        exposureComp = 0.0;
        curveAdjust  = QPolygon();
    };

    /** Method to check is a post-processing setting have been changed 
     */
    bool postProcessingSettingsIsDirty()
    {
        return (lightness    != 0.0    ||
                contrast     != 1.0    ||
                gamma        != 1.0    ||
                saturation   != 1.0    ||
                exposureComp != 0.0    ||
                !curveAdjust.isEmpty());
    }

    bool operator==(const DRawDecoding& other) const
    {
        return lightness     == other.lightness &&
               contrast      == other.contrast &&
               gamma         == other.gamma &&
               saturation    == other.saturation &&
               exposureComp  == other.exposureComp &&
               curveAdjust   == other.curveAdjust &&
               levelsAdjust  == other.levelsAdjust;
    }

public:

    /** Lightness correction value.
    */
    double lightness;

    /** Contrast correction value.
    */
    double contrast;

    /** Gamma correction value.
    */
    double gamma;

    /** Color saturation correction value.
    */
    double saturation;

    /** Exposure compensation value.
    */
    double exposureComp;

    /** Luminosity curve adjustments.
    */
    QPolygon curveAdjust;

    /** Levels adjustments: 4 channels (L, R, G, B * 2 values).
    */
    QList<int> levelsAdjust;
};

}  // namespace Digikam

Q_DECLARE_METATYPE(Digikam::DRawDecoding)

#endif /* DRAW_DECODING_H */

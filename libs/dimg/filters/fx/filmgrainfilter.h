/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-05-25
 * Description : filter to add Film Grain to image.
 * 
 * Copyright (C) 2005-2010 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2005-2010 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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
  
#ifndef FILMGRAINFILTER_H
#define FILMGRAINFILTER_H

// Local includes

#include "digikam_export.h"
#include "dimgthreadedfilter.h"
#include "globals.h"
#include "dcolor.h"

namespace Digikam
{
  
class FilmGrainFilterPriv;  

class DIGIKAM_EXPORT FilmGrainContainer
{

public:

    FilmGrainContainer()
    {
        grainSize               = 1;

        addLuminanceNoise       = true;
        lumaIntensity           = 25;
        lumaShadows             = -100;
        lumaMidtones            = 0; 
        lumaHighlights          = -100;

        addChrominanceBlueNoise = false;
        chromaBlueIntensity     = 25;
        chromaBlueShadows       = -100;
        chromaBlueMidtones      = 0;
        chromaBlueHighlights    = -100;

        addChrominanceRedNoise  = false;
        chromaRedIntensity      = 25;
        chromaRedShadows        = -100;
        chromaRedMidtones       = 0;
        chromaRedHighlights     = -100;
    };

    ~FilmGrainContainer(){};

public:

    int  grainSize;

    bool addLuminanceNoise;
    int  lumaIntensity;
    int  lumaShadows;
    int  lumaMidtones;
    int  lumaHighlights;

    bool addChrominanceBlueNoise;
    int  chromaBlueIntensity;
    int  chromaBlueShadows; 
    int  chromaBlueMidtones;
    int  chromaBlueHighlights;

    bool addChrominanceRedNoise;
    int  chromaRedIntensity;
    int  chromaRedShadows; 
    int  chromaRedMidtones;
    int  chromaRedHighlights;
};

// -----------------------------------------------------------------------------------------------
  
class DIGIKAM_EXPORT FilmGrainFilter : public DImgThreadedFilter
{

public:

    explicit FilmGrainFilter(DImg* orgImage, QObject* parent=0, const FilmGrainContainer& settings=FilmGrainContainer());
    // Constructor for slave mode: execute immediately in current thread with specified master filter
    explicit FilmGrainFilter(DImgThreadedFilter* parentFilter, const DImg& orgImage, const DImg& destImage,
                             int progressBegin=0, int progressEnd=100,
                             const FilmGrainContainer& settings=FilmGrainContainer());
    ~FilmGrainFilter();

private:

    void filterImage();

    inline double interpolate(int shadows, int midtones, int highlights, const DColor& col);
    inline double randomizeUniform(double range, bool sixteenbits);
    inline double randomizeGauss(double sigma, bool sixteenbits);
    inline void   adjustYCbCr(DColor& col, double range, double nRand, int channel);

private:

    FilmGrainFilterPriv* const d;
};

}  // namespace Digikam

#endif /* FILMGRAINFILTER_H */

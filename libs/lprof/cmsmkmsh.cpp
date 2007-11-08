/* */
/*  Little cms - profiler construction set */
/*  Copyright (C) 1998-2001 Marti Maria <marti@littlecms.com> */
/* */
/* THIS SOFTWARE IS PROVIDED "AS-IS" AND WITHOUT WARRANTY OF ANY KIND, */
/* EXPRESS, IMPLIED OR OTHERWISE, INCLUDING WITHOUT LIMITATION, ANY */
/* WARRANTY OF MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE. */
/* */
/* IN NO EVENT SHALL MARTI MARIA BE LIABLE FOR ANY SPECIAL, INCIDENTAL, */
/* INDIRECT OR CONSEQUENTIAL DAMAGES OF ANY KIND, */
/* OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, */
/* WHETHER OR NOT ADVISED OF THE POSSIBILITY OF DAMAGE, AND ON ANY THEORY OF */
/* LIABILITY, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE */
/* OF THIS SOFTWARE. */
/* */
/* This file is free software; you can redistribute it and/or modify it */
/* under the terms of the GNU General Public License as published by */
/* the Free Software Foundation; either version 2 of the License, or */
/* (at your option) any later version. */
/* */
/* This program is distributed in the hope that it will be useful, but */
/* WITHOUT ANY WARRANTY; without even the implied warranty of */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU */
/* General Public License for more details. */
/* */
/* You should have received a copy of the GNU General Public License */
/* along with this program; if not, write to the Free Software */
/* Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA. */
/* */
/* As a special exception to the GNU General Public License, if you */
/* distribute this file as part of a program that contains a */
/* configuration script generated by Autoconf, you may include it under */
/* the same distribution terms that you use for the rest of that program. */
/* */
/* Version 1.08a */


#include "lcmsprf.h"


BOOL cdecl cmsxComputeMatrixShaper(const char* ReferenceSheet,                              
                             const char* MeasurementSheet,  
                             int Medium,
                             LPGAMMATABLE TransferCurves[3],
                             LPcmsCIEXYZ WhitePoint,
                             LPcmsCIEXYZ BlackPoint,
                             LPcmsCIExyYTRIPLE Primaries);



/* ------------------------------------------------------------- Implementation  */


static
void Div100(LPcmsCIEXYZ xyz)
{
    xyz -> X /= 100; xyz -> Y /= 100; xyz -> Z /= 100;
}



/* Compute endpoints */

static
BOOL ComputeWhiteAndBlackPoints(LPMEASUREMENT Linearized,
                                LPGAMMATABLE TransferCurves[3], 
                                LPcmsCIEXYZ Black, LPcmsCIEXYZ White)
{

    double Zeroes[3], Ones[3], lmin[3], lmax[3];    

    SETOFPATCHES Neutrals = cmsxPCollBuildSet(Linearized, false);
    
    cmsxPCollPatchesNearNeutral(Linearized, Linearized->Allowed,
                                                    15, Neutrals); 

    Zeroes[0] = Zeroes[1] = Zeroes[2] = 0.0;
    Ones[0]   = Ones[1]   = Ones[2]   = 255.0;


    cmsxApplyLinearizationTable(Zeroes,  TransferCurves, lmin);
    cmsxApplyLinearizationTable(Ones,    TransferCurves, lmax);

    
    /* Global regression to find White & Black points */
    if (!cmsxRegressionInterpolatorRGB(Linearized, PT_XYZ,                                       
                                       4,
                                       true,
                                       12,
                                       lmin[0], lmin[1], lmin[2],
                                       Black)) return false;

    if (!cmsxRegressionInterpolatorRGB(Linearized, PT_XYZ,                                       
                                       4,
                                       true,
                                       12,
                                       lmax[0], lmax[1], lmax[2],
                                       White)) return false;

    _cmsxClampXYZ100(White);
    _cmsxClampXYZ100(Black);
    
    return true;
    
}


/* Study convergence of primary axis */

static
BOOL ComputePrimary(LPMEASUREMENT Linearized,
                    LPGAMMATABLE TransferCurves[3], 
                    int n,
                    LPcmsCIExyY Primary)
{

    double Ones[3], lmax[3];    
    cmsCIEXYZ PrimXYZ;
    SETOFPATCHES SetPrimary;
    int nR;
           

    /* At first, try to see if primaries are already in measurement */

    SetPrimary = cmsxPCollBuildSet(Linearized, false);
    nR = cmsxPCollPatchesNearPrimary(Linearized, Linearized->Allowed,
                                           n, 32, SetPrimary);

    Ones[0]  = Ones[1] = Ones[2] = 0;
    Ones[n]  = 255.0;

    cmsxApplyLinearizationTable(Ones,  TransferCurves, lmax);
    
    /* Do incremental regression to find primaries */
    if (!cmsxRegressionInterpolatorRGB(Linearized, PT_XYZ,                                       
                                       4,
                                       false, 
                                       12,
                                       lmax[0], lmax[1], lmax[2],                                   
                                       &PrimXYZ)) return false;

    _cmsxClampXYZ100(&PrimXYZ);
    cmsXYZ2xyY(Primary, &PrimXYZ);
    return true;
    

}



/* Does compute a matrix-shaper based on patches.  */

static
double Clip(double d)
{
    return d > 0 ? d: 0;
}


BOOL cmsxComputeMatrixShaper(const char* ReferenceSheet,                                
                             const char* MeasurementSheet, 
                             int Medium,
                             LPGAMMATABLE TransferCurves[3],
                             LPcmsCIEXYZ WhitePoint,
                             LPcmsCIEXYZ BlackPoint,
                             LPcmsCIExyYTRIPLE Primaries)
{

    MEASUREMENT Linearized;     
    cmsCIEXYZ Black, White;
    cmsCIExyYTRIPLE  PrimarySet;
    LPPATCH PatchWhite, PatchBlack;
    LPPATCH PatchRed, PatchGreen, PatchBlue;
    double Distance;

    /* Load sheets */

    if (!cmsxPCollBuildMeasurement(&Linearized, 
                             ReferenceSheet,
                             MeasurementSheet,
                             PATCH_HAS_XYZ|PATCH_HAS_RGB)) return false;



    /* Any patch to deal of? */
    if (cmsxPCollCountSet(&Linearized, Linearized.Allowed) <= 0) return false;


    /* Try to see if proper primaries, white and black already present */
    PatchWhite = cmsxPCollFindWhite(&Linearized, Linearized.Allowed, &Distance);
    if (Distance != 0)
        PatchWhite = NULL;  

    PatchBlack = cmsxPCollFindBlack(&Linearized, Linearized.Allowed, &Distance);
    if (Distance != 0)
        PatchBlack = NULL;  

    PatchRed = cmsxPCollFindPrimary(&Linearized, Linearized.Allowed, 0, &Distance);
    if (Distance != 0)
        PatchRed = NULL;    

    PatchGreen = cmsxPCollFindPrimary(&Linearized, Linearized.Allowed, 1, &Distance);
    if (Distance != 0)
        PatchGreen = NULL;  

    PatchBlue = cmsxPCollFindPrimary(&Linearized, Linearized.Allowed, 2, &Distance);
    if (Distance != 0)
        PatchBlue= NULL;    

    /* If we got primaries, then we can also get prelinearization */
    /* by  Levenberg-Marquardt. This applies on monitor profiles */

    if (PatchWhite && PatchRed && PatchGreen && PatchBlue) {

        /* Build matrix with primaries */

        MAT3 Mat, MatInv;
        LPSAMPLEDCURVE Xr,Yr, Xg, Yg, Xb, Yb;
        int i, nRes, cnt;
        
        VEC3init(&Mat.v[0], PatchRed->XYZ.X, PatchGreen->XYZ.X, PatchBlue->XYZ.X);
        VEC3init(&Mat.v[1], PatchRed->XYZ.Y, PatchGreen->XYZ.Y, PatchBlue->XYZ.Y);
        VEC3init(&Mat.v[2], PatchRed->XYZ.Z, PatchGreen->XYZ.Z, PatchBlue->XYZ.Z);

        /* Invert matrix */
        MAT3inverse(&Mat, &MatInv);

        nRes = cmsxPCollCountSet(&Linearized, Linearized.Allowed);

        Xr = cmsAllocSampledCurve(nRes);
        Yr = cmsAllocSampledCurve(nRes);
        Xg = cmsAllocSampledCurve(nRes);
        Yg = cmsAllocSampledCurve(nRes);
        Xb = cmsAllocSampledCurve(nRes);
        Yb = cmsAllocSampledCurve(nRes);
                
        /* Convert XYZ of all patches to RGB */
        cnt = 0;
        for (i=0; i < Linearized.nPatches; i++) {

            if (Linearized.Allowed[i]) {

                VEC3 RGBprime, XYZ;
                LPPATCH p;
                
                p = Linearized.Patches + i;
                XYZ.n[0] = p -> XYZ.X;
                XYZ.n[1] = p -> XYZ.Y;
                XYZ.n[2] = p -> XYZ.Z;

                MAT3eval(&RGBprime, &MatInv, &XYZ);

                Xr ->Values[cnt] = p ->Colorant.RGB[0];
                Yr ->Values[cnt] = Clip(RGBprime.n[0]);

                Xg ->Values[cnt] = p ->Colorant.RGB[1];
                Yg ->Values[cnt] = Clip(RGBprime.n[1]);

                Xb ->Values[cnt] = p ->Colorant.RGB[2];
                Yb ->Values[cnt] = Clip(RGBprime.n[2]); 

                cnt++;

            }
        }

        TransferCurves[0] = cmsxEstimateGamma(Xr, Yr, 1024);
        TransferCurves[1] = cmsxEstimateGamma(Xg, Yg, 1024);
        TransferCurves[2] = cmsxEstimateGamma(Xb, Yb, 1024);

        if (WhitePoint) {
        
            WhitePoint->X = PatchWhite->XYZ.X;
            WhitePoint->Y= PatchWhite ->XYZ.Y;
            WhitePoint->Z= PatchWhite ->XYZ.Z;
        }

        if (BlackPoint && PatchBlack) {

            BlackPoint->X = PatchBlack ->XYZ.X;
            BlackPoint->Y = PatchBlack ->XYZ.Y;
            BlackPoint->Z = PatchBlack ->XYZ.Z;
        }

        if (Primaries) {

            cmsXYZ2xyY(&Primaries->Red,   &PatchRed ->XYZ);
            cmsXYZ2xyY(&Primaries->Green, &PatchGreen ->XYZ);
            cmsXYZ2xyY(&Primaries->Blue,  &PatchBlue ->XYZ);

        }


        cmsFreeSampledCurve(Xr);
        cmsFreeSampledCurve(Yr);
        cmsFreeSampledCurve(Xg);
        cmsFreeSampledCurve(Yg);
        cmsFreeSampledCurve(Xb);
        cmsFreeSampledCurve(Yb);

        cmsxPCollFreeMeasurements(&Linearized); 

        return true;
    }




    /* Compute prelinearization */
    cmsxComputeLinearizationTables(&Linearized, PT_XYZ, TransferCurves, 1024, Medium);   
      
    /* Linearize measurements */
    cmsxPCollLinearizePatches(&Linearized, Linearized.Allowed, TransferCurves);
       

    /* Endpoints */
    ComputeWhiteAndBlackPoints(&Linearized, TransferCurves, &Black, &White);
        
    /* Primaries */
    ComputePrimary(&Linearized, TransferCurves, 0, &PrimarySet.Red);
    ComputePrimary(&Linearized, TransferCurves, 1, &PrimarySet.Green);
    ComputePrimary(&Linearized, TransferCurves, 2, &PrimarySet.Blue);

    
    if (BlackPoint) {
        *BlackPoint = Black;            
        Div100(BlackPoint);
    }

    if (WhitePoint) {
        *WhitePoint = White;
        Div100(WhitePoint);     
    }


    if (Primaries) {
        *Primaries = PrimarySet;    
    }
    
    cmsxPCollFreeMeasurements(&Linearized); 
    
    return true;
}




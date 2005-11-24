/* ============================================================
 * Author: Gilles Caulier <caulier dot gilles at free.fr> 
 * Date  : 2005-11-01
 * Description : A digital camera RAW files loader for DImg 
 *               framework using dcraw program.
 * 
 * Copyright 2005 by Gilles Caulier
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
 * ============================================================ */

#ifndef RAWLOADER_H
#define RAWLOADER_H

// Local includes.

#include "dimgloader.h"
#include "digikam_export.h"

namespace Digikam
{
class DImg;

class DIGIKAM_EXPORT RAWLoader : public DImgLoader
{
public:

    RAWLoader(DImg* image);

    bool load(const QString& filePath);
    bool save(const QString& filePath);

    virtual bool hasAlpha()   const;
    virtual bool sixteenBit() const;
    virtual bool isReadOnly() const { return true; };

private:

    bool m_sixteenBit;
    bool m_hasAlpha;
    
private:

    bool load8bits(const QString& filePath);
    bool load16bits(const QString& filePath);
    
};
    
}  // NameSpace Digikam
    
#endif /* RAWLOADER_H */

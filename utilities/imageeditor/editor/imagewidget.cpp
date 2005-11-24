/* ============================================================
 * Author: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Date  : 2004-02-14
 * Description : 
 * 
 * Copyright 2004 by Renchi Raju
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
 * 
 * ============================================================ */

// C++ includes.
  
#include <cstdio>

// Qt includes.

#include <qregion.h>
#include <qpainter.h>

// Local includes.

#include "imageiface.h"
#include "imagewidget.h"

namespace Digikam
{

ImageWidget::ImageWidget(int w, int h, QWidget *parent)
           : QWidget(parent, 0, Qt::WDestructiveClose)
{
    setBackgroundMode(Qt::NoBackground);
    setMinimumSize(w, h);

    m_iface = new ImageIface(w, h);
    m_image = m_iface->getPreviewImage();
    m_rect = QRect(w/2 - m_image.width()/2, h/2 - m_image.height()/2, 
                   m_image.width(), m_image.height());    
}

ImageWidget::~ImageWidget()
{
    delete m_iface;
}

ImageIface* ImageWidget::imageIface()
{
    return m_iface;
}

void ImageWidget::paintEvent(QPaintEvent *)
{
    m_iface->paint(this, m_rect.x(), m_rect.y(),
                   m_rect.width(), m_rect.height());

    QRect r(0, 0, width(), height());
    QRegion reg(r);
    reg -= m_rect;

    QPainter p(this);
    p.setClipRegion(reg);
    p.fillRect(r, colorGroup().background());
    p.end();
}

void ImageWidget::resizeEvent(QResizeEvent * e)
{
    int w   = e->size().width();
    int h   = e->size().height();
    m_image = m_iface->setPreviewImageSize(w, h);
    m_rect = QRect(w/2 - m_image.width()/2, h/2 - m_image.height()/2, 
                   m_image.width(), m_image.height());    
    emit signalResized();
}

}

#include "imagewidget.moc"

/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-12-01
 * Description : a widget to draw histogram curves
 *
 * Copyright (C) 2004-2009 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#define CLAMP(x,l,u) ((x)<(l)?(l):((x)>(u)?(u):(x)))

#include "curveswidget.moc"

// C++ includes

#include <cmath>
#include <cstdlib>

// Qt includes

#include <QPixmap>
#include <QPainter>
#include <QPoint>
#include <QPen>
#include <QEvent>
#include <QTimer>
#include <QRect>
#include <QColor>
#include <QFont>
#include <QFontMetrics>
#include <QCustomEvent>
#include <QPaintEvent>
#include <QMouseEvent>

// KDE includes

#include <kcursor.h>
#include <klocale.h>
#include <kiconloader.h>
#include <kdebug.h>

// Local includes

#include "imagehistogram.h"
#include "imagecurves.h"
#include "globals.h"
#include "histogrampainter.h"

namespace Digikam
{

class CurvesWidgetPriv
{
public:

    enum RepaintType
    {
        HistogramDataLoading = 0, // Image Data loading in progress.
        HistogramNone,            // No current histogram values calculation.
        HistogramStarted,         // Histogram values calculation started.
        HistogramCompleted,       // Histogram values calculation completed.
        HistogramFailed           // Histogram values calculation failed.
    };

    CurvesWidgetPriv(CurvesWidget *q)
        : q(q)
    {
        curves        = 0;
        grabPoint     = -1;
        last          = 0;
        guideVisible  = false;
        xMouseOver    = -1;
        yMouseOver    = -1;
        clearFlag     = HistogramNone;
        progressCount = 0;
        progressTimer = 0;
        progressPix   = SmallIcon("process-working", 22);
    }

    bool              readOnlyMode;
    bool              guideVisible;

    int               clearFlag;          // Clear drawing zone with message.
    int               leftMost;
    int               rightMost;
    int               grabPoint;
    int               last;
    int               xMouseOver;
    int               yMouseOver;
    int               progressCount;      // Position of animation during loading/calculation.

    QTimer*           progressTimer;

    QPixmap           progressPix;

    DColor            colorGuide;

    ImageCurves*      curves;             // Curves data instance.

    HistogramPainter* histogramPainter;

    // --- misc methods ---

    int getDelta()
    {
        // TODO magic number, what is this?
        return q->m_imageHistogram->getHistogramSegments() / 16;
    }

    void getHistogramCoordinates(const QPoint& mousePosition, int& x, int& y, int& closestPoint)
    {

        x = CLAMP((int)(mousePosition.x() *
                           ((float)(q->m_imageHistogram->getMaxSegmentIndex()) / (float)q->width())),
                        0, q->m_imageHistogram->getMaxSegmentIndex());
        y = CLAMP((int)(mousePosition.y() *
                           ((float)(q->m_imageHistogram->getMaxSegmentIndex()) / (float)q->height())),
                        0, q->m_imageHistogram->getMaxSegmentIndex());

        int distance = NUM_SEGMENTS_16BIT;

        closestPoint = 0;
        for (int i = 0; i < ImageCurves::NUM_POINTS; ++i)
        {
            int xcurvepoint = curves->getCurvePointX(q->m_channelType, i);

            if (xcurvepoint != -1)
            {
                if (abs(x - xcurvepoint) < distance)
                {
                    distance     = abs(x - xcurvepoint);
                    closestPoint = i;
                }
            }
        }

        // TODO magic number, what is this?
        if (distance > 8)
        {
            closestPoint = (x + getDelta() / 2) / getDelta();
        }

    }

    // --- rendering ---

    void renderLoadingAnimation()
    {

        QPixmap anim(progressPix.copy(0, progressCount*22, 22, 22));
        progressCount++;
        if (progressCount == 8)
        {
            progressCount = 0;
        }

        // ... and we render busy text.

        QPixmap pm(q->size());
        QPainter p1;
        p1.begin(&pm);
        p1.initFrom(q);
        p1.fillRect(0, 0, q->width(), q->height(), q->palette().color(QPalette::Active, QPalette::Background));
        p1.setPen(QPen(q->palette().color(QPalette::Active, QPalette::Foreground), 1, Qt::SolidLine));
        p1.drawRect(0, 0, q->width()-1, q->height()-1);
        p1.drawPixmap(q->width()/2 - anim.width() /2, anim.height(), anim);
        p1.setPen(q->palette().color(QPalette::Active, QPalette::Text));

        if (clearFlag == CurvesWidgetPriv::HistogramDataLoading)
        {
            p1.drawText(0, 0, q->width(), q->height(), Qt::AlignCenter,
                        i18n("Loading image..."));
        }
        else
        {
            p1.drawText(0, 0, q->width(), q->height(), Qt::AlignCenter,
                        i18n("Histogram calculation..."));
        }

        p1.end();
        QPainter p3(q);
        p3.drawPixmap(0, 0, pm);
        p3.end();
    }

    void renderHistogramFailed()
    {
        QPixmap pm(q->size());
        QPainter p1;
        p1.begin(&pm);
        p1.initFrom(q);
        p1.fillRect(0, 0, q->width(), q->height(), q->palette().color(QPalette::Active, QPalette::Background));
        p1.setPen(QPen(q->palette().color(QPalette::Active, QPalette::Foreground), 1, Qt::SolidLine));
        p1.drawRect(0, 0, q->width()-1, q->height()-1);
        p1.setPen(q->palette().color(QPalette::Active, QPalette::Text));
        p1.drawText(0, 0, q->width(), q->height(), Qt::AlignCenter,
                    i18n("Histogram\ncalculation\nfailed."));
        p1.end();
        QPainter p2(q);
        p2.drawPixmap(0, 0, pm);
        p2.end();
    }

    void renderCurve(QPixmap& pm)
    {
        QPainter p1;
        p1.begin(&pm);
        p1.initFrom(q);

        int wWidth  = pm.width();
        int wHeight = pm.height();

        // Drawing curves.
        QPainterPath curvePath;
        curvePath.moveTo(0, wHeight);
        for (int x = 0 ; x < wWidth; ++x)
        {

            // TODO duplicate code...
            int i = (x * q->m_imageHistogram->getHistogramSegments()) / wWidth;

            int curveVal = curves->getCurveValue(q->m_channelType, i);
            curvePath.lineTo(x, wHeight - ((curveVal * wHeight) / q->m_imageHistogram->getHistogramSegments()));
        }
        curvePath.lineTo(wWidth, wHeight);

        p1.save();
        p1.setRenderHint(QPainter::Antialiasing);
        p1.setPen(QPen(q->palette().color(QPalette::Active, QPalette::Link), 2, Qt::SolidLine));
        p1.drawPath(curvePath);
        p1.restore();


        // Drawing curves points.
        if (!readOnlyMode && curves->getCurveType(q->m_channelType) == ImageCurves::CURVE_SMOOTH)
        {

            p1.save();
            p1.setPen(QPen(Qt::red, 3, Qt::SolidLine));
            p1.setRenderHint(QPainter::Antialiasing);

            for (int p = 0 ; p < ImageCurves::NUM_POINTS ; ++p)
            {
                QPoint curvePoint = curves->getCurvePoint(q->m_channelType, p);

                if (curvePoint.x() >= 0)
                {
                    p1.drawEllipse( ((curvePoint.x() * wWidth) / q->m_imageHistogram->getHistogramSegments()) - 2,
                                 wHeight - 2 - ((curvePoint.y() * wHeight) / q->m_imageHistogram->getHistogramSegments()),
                                 4, 4 );
                }
            }
            p1.restore();
        }
    }

    void renderGrid(QPixmap& pm)
    {
        QPainter p1;
        p1.begin(&pm);
        p1.initFrom(q);

        int wWidth  = pm.width();
        int wHeight = pm.height();

        // Drawing black/middle/highlight tone grid separators.
        p1.setPen(QPen(q->palette().color(QPalette::Active, QPalette::Base), 1, Qt::SolidLine));
        p1.drawLine(wWidth/4, 0, wWidth/4, wHeight);
        p1.drawLine(wWidth/2, 0, wWidth/2, wHeight);
        p1.drawLine(3*wWidth/4, 0, 3*wWidth/4, wHeight);
        p1.drawLine(0, wHeight/4, wWidth, wHeight/4);
        p1.drawLine(0, wHeight/2, wWidth, wHeight/2);
        p1.drawLine(0, 3*wHeight/4, wWidth, 3*wHeight/4);

    }

    void renderMousePosition(QPixmap& pm)
    {
        QPainter p1;
        p1.begin(&pm);
        p1.initFrom(q);

        int wWidth  = pm.width();
        int wHeight = pm.height();

        // Drawing X,Y point position dragged by mouse over widget.
        p1.setPen(QPen(Qt::red, 1, Qt::DotLine));

        if (xMouseOver != -1 && yMouseOver != -1)
        {
            QString string = i18n("x:%1\ny:%2",xMouseOver,yMouseOver);
            QFontMetrics fontMt(string);
            QRect rect     = fontMt.boundingRect(0, 0, wWidth, wHeight, 0, string);
            rect.moveRight(wWidth);
            rect.moveBottom(wHeight);
            p1.drawText(rect, Qt::AlignLeft||Qt::AlignTop, string);
        }
    }

    void renderFrame(QPixmap& pm)
    {
        QPainter p1;
        p1.begin(&pm);
        p1.initFrom(q);
        p1.setPen(QPen(q->palette().color(QPalette::Active,
                       QPalette::Foreground), 1, Qt::SolidLine));
        p1.drawRect(0, 0, pm.width() - 1, pm.height() - 1);
    }

    // --- patterns for storing / restoring state ---

    static QString getChannelTypeOption(const QString& prefix, int channel)
    {
        return QString(prefix + "Channel%1Type").arg(channel);
    }

    static QString getPointOption(const QString& prefix, int channel, int point)
    {
        return QString(prefix + "Channel%1Point%2").arg(channel).arg(point);
    }

private:

    CurvesWidget *q;
};

CurvesWidget::CurvesWidget(int w, int h, QWidget *parent, bool readOnly)
            : QWidget(parent), d(new CurvesWidgetPriv(this))
{
    setAttribute(Qt::WA_DeleteOnClose);
    setup(w, h, readOnly);
}

CurvesWidget::CurvesWidget(int w, int h,
                           uchar *i_data, uint i_w, uint i_h, bool i_sixteenBits,
                           QWidget *parent, bool readOnly)
            : QWidget(parent), d(new CurvesWidgetPriv(this))
{
    setAttribute(Qt::WA_DeleteOnClose);
    setup(w, h, readOnly);
    updateData(i_data, i_w, i_h, i_sixteenBits);
}

CurvesWidget::~CurvesWidget()
{
    d->progressTimer->stop();

    if (m_imageHistogram)
        delete m_imageHistogram;

    if (d->curves)
        delete d->curves;

    delete d;
}

void CurvesWidget::setup(int w, int h, bool readOnly)
{
    d->readOnlyMode     = readOnly;
    d->curves           = new ImageCurves(true);
    d->histogramPainter = new HistogramPainter(this);
    d->histogramPainter->setChannelType(LuminosityChannel);
    d->histogramPainter->setRenderXGrid(false);
    d->histogramPainter->setHighlightSelection(false);
    d->histogramPainter->initFrom(this);
    m_channelType       = LuminosityChannel;
    m_scaleType         = LogScaleHistogram;
    m_imageHistogram    = 0;

    setMouseTracking(true);
    setMinimumSize(w, h);

    d->progressTimer = new QTimer(this);

    connect(d->progressTimer, SIGNAL(timeout()),
            this, SLOT(slotProgressTimerDone()));
}

void CurvesWidget::saveCurve(KConfigGroup& group, const QString& prefix)
{
    kDebug() << "Storing curves";

    for (int channel = 0; channel < ImageCurves::NUM_CHANNELS; ++channel)
    {

        group.writeEntry(CurvesWidgetPriv::getChannelTypeOption(prefix, channel),
                         (int) curves()->getCurveType(channel));

        for (int point = 0; point <= ImageCurves::NUM_POINTS; ++point)
        {
            QPoint p = curves()->getCurvePoint(channel, point);
            if (!isSixteenBits() && p != ImageCurves::getDisabledValue())
            {
                // Store point as 16 bits depth.
                p.setX(p.x() * ImageCurves::MULTIPLIER_16BIT);
                p.setY(p.y() * ImageCurves::MULTIPLIER_16BIT);
            }

            group.writeEntry(CurvesWidgetPriv::getPointOption(prefix, channel, point), p);
        }
    }
}

void CurvesWidget::restoreCurve(KConfigGroup& group, const QString& prefix)
{
    kDebug() << "Restoring curves";

    reset();

    kDebug() << "curves " << curves() << " isSixteenBits = " << isSixteenBits();

    for (int channel = 0; channel < ImageCurves::NUM_CHANNELS; ++channel)
    {

        curves()->setCurveType(channel, (ImageCurves::CurveType) group.readEntry(
                                        CurvesWidgetPriv::getChannelTypeOption(
                                        prefix, channel), 0));

        for (int point = 0; point <= ImageCurves::NUM_POINTS; ++point)
        {
            QPoint p = group.readEntry(CurvesWidgetPriv::getPointOption(prefix,
                                       channel, point), ImageCurves::getDisabledValue());

            // always load a 16 bit curve and stretch it to 8 bit if necessary
            if (!isSixteenBits() && p != ImageCurves::getDisabledValue())
            {
                p.setX(p.x() / ImageCurves::MULTIPLIER_16BIT);
                p.setY(p.y() / ImageCurves::MULTIPLIER_16BIT);
            }

            curves()->setCurvePoint(channel, point, p);
        }

        curves()->curvesCalculateCurve(channel);
    }
}

void CurvesWidget::updateData(uchar* i_data, uint i_w, uint i_h, bool i_sixteenBits)
{
    kDebug() << "updating data";

    stopHistogramComputation();

    // Remove old histogram data from memory.
    if (m_imageHistogram)
        delete m_imageHistogram;

    m_imageHistogram = new ImageHistogram(i_data, i_w, i_h, i_sixteenBits);

    connect(m_imageHistogram, SIGNAL(calculationStarted(const ImageHistogram*)),
            this, SLOT(slotCalculationStarted(const ImageHistogram*)));

    connect(m_imageHistogram, SIGNAL(calculationFinished(const ImageHistogram*, bool)),
            this, SLOT(slotCalculationFinished(const ImageHistogram*, bool)));

    m_imageHistogram->calculateInThread();

    // keep the old curve
    ImageCurves* newCurves = new ImageCurves(i_sixteenBits);
    newCurves->setCurveType(ImageCurves::CURVE_SMOOTH);
    if (d->curves)
    {
        newCurves->fillFromOtherCurvers(d->curves);
        delete d->curves;
    }
    d->curves = newCurves;

    resetUI();
}

bool CurvesWidget::isSixteenBits()
{
    return curves()->isSixteenBits();
}

void CurvesWidget::reset()
{
    if (d->curves)
        d->curves->curvesReset();

    resetUI();
}

void CurvesWidget::resetUI()
{
    d->grabPoint    = -1;
    d->guideVisible = false;
    repaint();
}

ImageCurves* CurvesWidget::curves() const
{
    return d->curves;
}

void CurvesWidget::setDataLoading()
{
    if (d->clearFlag != CurvesWidgetPriv::HistogramDataLoading)
    {
        setCursor(Qt::WaitCursor);
        d->clearFlag     = CurvesWidgetPriv::HistogramDataLoading;
        d->progressCount = 0;
        d->progressTimer->start(100);
    }
}

void CurvesWidget::setLoadingFailed()
{
    d->clearFlag     = CurvesWidgetPriv::HistogramFailed;
    d->progressCount = 0;
    d->progressTimer->stop();
    repaint();
    setCursor(Qt::ArrowCursor);
}

void CurvesWidget::setCurveGuide(const DColor& color)
{
    d->guideVisible = true;
    d->colorGuide   = color;
    repaint();
}

void CurvesWidget::curveTypeChanged()
{
    switch (d->curves->getCurveType(m_channelType))
    {
        case ImageCurves::CURVE_SMOOTH:

            //  pick representative points from the curve and make them control points

            for (int i = 0; i <= 8; ++i)
            {
                int index = CLAMP(i * m_imageHistogram->getHistogramSegments() / 8,
                                  0, m_imageHistogram->getMaxSegmentIndex());

                d->curves->setCurvePoint(m_channelType, i * 2,
                                         QPoint(index, d->curves->getCurveValue(m_channelType, index)));
            }

            d->curves->curvesCalculateCurve(m_channelType);
            break;

        case ImageCurves::CURVE_FREE:
            break;
    }

    repaint();
    emit signalCurvesChanged();
}

void CurvesWidget::slotCalculationStarted(const ImageHistogram*)
{
    setCursor(Qt::WaitCursor);
    d->clearFlag = CurvesWidgetPriv::HistogramStarted;
    d->progressTimer->start(200);
    repaint();
}

void CurvesWidget::slotCalculationFinished(const ImageHistogram*, bool success)
{
    if (success)
    {
        // Repaint histogram
        d->clearFlag = CurvesWidgetPriv::HistogramCompleted;
        d->progressTimer->stop();
        repaint();
        setCursor(Qt::ArrowCursor);
    }
    else
    {
        d->clearFlag = CurvesWidgetPriv::HistogramFailed;
        d->progressTimer->stop();
        repaint();
        setCursor(Qt::ArrowCursor);
        emit signalHistogramComputationFailed();
    }
}

void CurvesWidget::stopHistogramComputation()
{
    if (m_imageHistogram)
        m_imageHistogram->stopCalculation();

    d->progressTimer->stop();
    d->progressCount = 0;
}

void CurvesWidget::slotProgressTimerDone()
{
    repaint();
    d->progressTimer->start(200);
}

void CurvesWidget::paintEvent(QPaintEvent*)
{
    // special cases

    if (d->clearFlag == CurvesWidgetPriv::HistogramDataLoading ||
        d->clearFlag == CurvesWidgetPriv::HistogramStarted)
    {
        d->renderLoadingAnimation();
        return;
    }
    else if (d->clearFlag == CurvesWidgetPriv::HistogramFailed)
    {
        d->renderHistogramFailed();
        return;
    }

    // normal case, histogram present

    if (!m_imageHistogram)
    {
        kWarning() << "Should render a histogram, but did not get one.";
        return;
    }

    // render subelements on a pixmap (double buffering)
    QPixmap pm(size());

    d->histogramPainter->setScale(m_scaleType);
    d->histogramPainter->setHistogram(m_imageHistogram);
    d->histogramPainter->setChannelType(m_channelType);
    if (d->guideVisible)
    {
        d->histogramPainter->enableHistogramGuideByColor(d->colorGuide);
    }
    else
    {
        d->histogramPainter->disableHistogramGuide();
    }
    d->histogramPainter->render(pm);
    d->renderCurve(pm);
    d->renderGrid(pm);
    d->renderMousePosition(pm);
    d->renderFrame(pm);

    // render pixmap on widget
    QPainter p2(this);
    p2.drawPixmap(0, 0, pm);
    p2.end();
}

void CurvesWidget::mousePressEvent(QMouseEvent *e)
{
    if (d->readOnlyMode || !m_imageHistogram) return;

    if (e->button() != Qt::LeftButton || d->clearFlag == CurvesWidgetPriv::HistogramStarted)
        return;

    int x, y, closest_point;
    d->getHistogramCoordinates(e->pos(), x, y, closest_point);

    setCursor(Qt::CrossCursor);

    switch(d->curves->getCurveType(m_channelType))
    {
        case ImageCurves::CURVE_SMOOTH:
        {
            // Determine the leftmost and rightmost points.

            d->leftMost = -1;

            for (int i = closest_point - 1; i >= 0; --i)
            {
                if (d->curves->getCurvePointX(m_channelType, i) != -1)
                {
                    d->leftMost = d->curves->getCurvePointX(m_channelType, i);
                    break;
                }
            }

            d->rightMost = m_imageHistogram->getHistogramSegments();

            for (int i = closest_point + 1; i < ImageCurves::NUM_POINTS; ++i)
            {
                if (d->curves->getCurvePointX(m_channelType, i) != -1)
                {
                    d->rightMost = d->curves->getCurvePointX(m_channelType, i);
                    break;
                }
            }

            d->grabPoint = closest_point;
            d->curves->setCurvePoint(m_channelType, d->grabPoint,
                                     QPoint(x, m_imageHistogram->getHistogramSegments() - y));

            break;
        }

        case ImageCurves::CURVE_FREE:
        {
            d->curves->setCurveValue(m_channelType, x, m_imageHistogram->getHistogramSegments() - y);
            d->grabPoint = x;
            d->last      = y;
            break;
        }
    }

    d->curves->curvesCalculateCurve(m_channelType);
    repaint();
}

void CurvesWidget::mouseReleaseEvent(QMouseEvent* e)
{
    if (d->readOnlyMode || !m_imageHistogram) return;

    if (e->button() != Qt::LeftButton || d->clearFlag == CurvesWidgetPriv::HistogramStarted)
        return;

    setCursor(Qt::ArrowCursor);
    d->grabPoint = -1;
    d->curves->curvesCalculateCurve(m_channelType);
    repaint();
    emit signalCurvesChanged();
}

void CurvesWidget::mouseMoveEvent(QMouseEvent* e)
{
    if (d->readOnlyMode || !m_imageHistogram) return;

    if (d->clearFlag == CurvesWidgetPriv::HistogramStarted)
        return;

    int x, y, closest_point;
    d->getHistogramCoordinates(e->pos(), x, y, closest_point);

    switch (d->curves->getCurveType(m_channelType))
    {
        case ImageCurves::CURVE_SMOOTH:
        {
            if (d->grabPoint == -1)   // If no point is grabbed...
            {
                if (d->curves->getCurvePointX(m_channelType, closest_point) != -1)
                    setCursor(Qt::ArrowCursor);
                else
                    setCursor(Qt::CrossCursor);
            }
            else                      // Else, drag the grabbed point
            {
                setCursor(Qt::CrossCursor);

                d->curves->setCurvePointX(m_channelType, d->grabPoint, -1);

                if (x > d->leftMost && x < d->rightMost)
                {
                    closest_point = (x + d->getDelta() / 2) / d->getDelta();

                    if (d->curves->getCurvePointX(m_channelType, closest_point) == -1)
                        d->grabPoint = closest_point;

                    d->curves->setCurvePoint(m_channelType, d->grabPoint,
                                             QPoint(x, m_imageHistogram->getMaxSegmentIndex() - y));
                }

                d->curves->curvesCalculateCurve(m_channelType);
                emit signalCurvesChanged();
            }

            break;
        }

        case ImageCurves::CURVE_FREE:
        {
            if (d->grabPoint != -1)
            {
                int x1, x2, y1, y2;
                if (d->grabPoint > x)
                {
                    x1 = x;
                    x2 = d->grabPoint;
                    y1 = y;
                    y2 = d->last;
                }
                else
                {
                    x1 = d->grabPoint;
                    x2 = x;
                    y1 = d->last;
                    y2 = y;
                }

                if (x2 != x1)
                {
                    for (int i = x1 ; i <= x2 ; ++i)
                        d->curves->setCurveValue(m_channelType, i, m_imageHistogram->getMaxSegmentIndex() - 
                                                                   (y1 + ((y2 - y1) * (i - x1)) / (x2 - x1)));
                }
                else
                {
                    d->curves->setCurveValue(m_channelType, x, m_imageHistogram->getMaxSegmentIndex() - y);
                }

                d->grabPoint = x;
                d->last      = y;
            }

           emit signalCurvesChanged();

           break;
        }
    }

    d->xMouseOver = x;
    d->yMouseOver = m_imageHistogram->getMaxSegmentIndex() - y;
    emit signalMouseMoved(d->xMouseOver, d->yMouseOver);
    repaint();
}

void CurvesWidget::leaveEvent(QEvent*)
{
    d->xMouseOver = -1;
    d->yMouseOver = -1;
    emit signalMouseMoved(d->xMouseOver, d->yMouseOver);
    repaint();
}

}  // namespace Digikam

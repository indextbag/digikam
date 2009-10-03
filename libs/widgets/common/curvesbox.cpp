/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-06-14
 * Description : a curves widget with additional control elements
 *
 * Copyright (C) 2009 by Andi Clemens <andi dot clemens at gmx dot net>
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

#include "curvesbox.h"
#include "curvesbox.moc"

// Qt includes

#include <QButtonGroup>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QPixmap>
#include <QPushButton>
#include <QTextStream>
#include <QToolButton>

// KDE includes

#include <kconfig.h>
#include <kconfiggroup.h>
#include <kdialog.h>
#include <klocale.h>
#include <kstandarddirs.h>

// Local includes

#include "colorgradientwidget.h"
#include "curveswidget.h"
#include "editortoolsettings.h"
#include "imagecurves.h"
#include "imagehistogram.h"
#include "debug.h"
#include "globals.h"

namespace Digikam
{

class CurvesBoxPriv
{
public:

    CurvesBoxPriv()
    {
        curvesWidget    = 0;
        hGradient       = 0;
        vGradient       = 0;
        curveFree       = 0;
        curveSmooth     = 0;
        curveType       = 0;
        pickBlack       = 0;
        pickGray        = 0;
        pickWhite       = 0;
        pickerBox       = 0;
        pickerType      = 0;
        resetButton     = 0;
        currentChannel  = LuminosityChannel;
        sixteenBit      = false;
    }
    int                  currentChannel;
    bool                 sixteenBit;
    CurvesWidget*        curvesWidget;
    ColorGradientWidget* hGradient;
    ColorGradientWidget* vGradient;
    QToolButton*         curveFree;
    QToolButton*         curveSmooth;
    QToolButton*         pickBlack;
    QToolButton*         pickGray;
    QToolButton*         pickWhite;
    QButtonGroup*        curveType;
    QButtonGroup*        pickerType;
    QWidget*             pickerBox;
    QPushButton*         resetButton;
};

CurvesBox::CurvesBox(int w, int h, QWidget *parent, bool readOnly)
         : QWidget(parent), d(new CurvesBoxPriv)
{
    d->curvesWidget = new CurvesWidget(w, h, this, readOnly);
    setup();
}

CurvesBox::CurvesBox(int w, int h, uchar *i_data, uint i_w, uint i_h,
                     bool i_sixteenBits, QWidget *parent, bool readOnly)
         : QWidget(parent), d(new CurvesBoxPriv)
{
    d->sixteenBit     = i_sixteenBits;
    d->curvesWidget   = new CurvesWidget(w, h, i_data, i_w, i_h, i_sixteenBits, this, readOnly);
    d->currentChannel = d->curvesWidget->m_channelType;
    setup();
}


void CurvesBox::setup()
{
    QWidget *curveBox = new QWidget();

    d->vGradient = new ColorGradientWidget(Qt::Vertical, 10);
    d->vGradient->setColors(QColor("white"), QColor("black"));

    d->hGradient = new ColorGradientWidget(Qt::Horizontal, 10);
    d->hGradient->setColors(QColor("black"), QColor("white"));

    QGridLayout* curveBoxLayout = new QGridLayout;
    curveBoxLayout->addWidget(d->vGradient,     0, 0, 1, 1);
    curveBoxLayout->addWidget(d->curvesWidget,  0, 2, 1, 1);
    curveBoxLayout->addWidget(d->hGradient,     2, 2, 1, 1);
    curveBoxLayout->setRowMinimumHeight(1, 2);
    curveBoxLayout->setColumnMinimumWidth(1, 2);
    curveBoxLayout->setMargin(0);
    curveBoxLayout->setSpacing(0);
    curveBox->setLayout(curveBoxLayout);

    // -------------------------------------------------------------

    QWidget *typeBox = new QWidget();

    d->curveFree = new QToolButton;
    d->curveFree->setIcon(QPixmap(KStandardDirs::locate("data", "digikam/data/curvefree.png")));
    d->curveFree->setCheckable(true);
    d->curveFree->setToolTip(i18n("Curve free mode"));
    d->curveFree->setWhatsThis( i18n("With this button, you can draw your curve free-hand "
                                     "with the mouse."));

    d->curveSmooth = new QToolButton;
    d->curveSmooth->setIcon(QPixmap(KStandardDirs::locate("data", "digikam/data/curvemooth.png")));
    d->curveSmooth->setCheckable(true);
    d->curveSmooth->setToolTip(i18n("Curve smooth mode"));
    d->curveSmooth->setWhatsThis( i18n("With this button, the curve type is constrained to "
                                       "be a smooth line with tension."));

    d->curveType = new QButtonGroup(typeBox);
    d->curveType->addButton(d->curveFree, FreeDrawing);
    d->curveType->addButton(d->curveSmooth, SmoothDrawing);

    d->curveType->setExclusive(true);
    d->curveSmooth->setChecked(true);

    QHBoxLayout *typeBoxLayout = new QHBoxLayout;
    typeBoxLayout->addWidget(d->curveFree);
    typeBoxLayout->addWidget(d->curveSmooth);
    typeBoxLayout->setMargin(0);
    typeBoxLayout->setSpacing(0);
    typeBox->setLayout(typeBoxLayout);

    // -------------------------------------------------------------

    d->pickerBox = new QWidget();

    d->pickBlack = new QToolButton;
    d->pickBlack->setIcon(KIcon("color-picker-black"));
    d->pickBlack->setCheckable(true);
    d->pickBlack->setToolTip(i18n("All channels shadow tone color picker"));
    d->pickBlack->setWhatsThis( i18n("With this button, you can pick the color from original "
                                     "image used to set <b>Shadow Tone</b> "
                                     "smooth curves point on Red, Green, Blue, and Luminosity channels."));

    d->pickGray = new QToolButton;
    d->pickGray->setIcon(KIcon("color-picker-grey"));
    d->pickGray->setCheckable(true);
    d->pickGray->setToolTip(i18n("All channels middle tone color picker"));
    d->pickGray->setWhatsThis( i18n("With this button, you can pick the color from original "
                                    "image used to set <b>Middle Tone</b> "
                                    "smooth curves point on Red, Green, Blue, and Luminosity channels."));

    d->pickWhite = new QToolButton;
    d->pickWhite->setIcon(KIcon("color-picker-white"));
    d->pickWhite->setCheckable(true);
    d->pickWhite->setToolTip( i18n( "All channels highlight tone color picker" ) );
    d->pickWhite->setWhatsThis( i18n("With this button, you can pick the color from original "
                                     "image used to set <b>Highlight Tone</b> "
                                     "smooth curves point on Red, Green, Blue, and Luminosity channels."));

    d->pickerType = new QButtonGroup(d->pickerBox);
    d->pickerType->addButton(d->pickBlack, BlackTonal);
    d->pickerType->addButton(d->pickGray, GrayTonal);
    d->pickerType->addButton(d->pickWhite, WhiteTonal);

    QHBoxLayout *pickerBoxLayout = new QHBoxLayout;
    pickerBoxLayout->addWidget(d->pickBlack);
    pickerBoxLayout->addWidget(d->pickGray);
    pickerBoxLayout->addWidget(d->pickWhite);
    pickerBoxLayout->setMargin(0);
    pickerBoxLayout->setSpacing(0);
    d->pickerBox->setLayout(pickerBoxLayout);

    d->pickerType->setExclusive(true);

    // -------------------------------------------------------------

    d->resetButton = new QPushButton(i18n("&Reset"));
    d->resetButton->setIcon(KIconLoader::global()->loadIcon("document-revert", KIconLoader::Toolbar));
    d->resetButton->setToolTip(i18n("Reset current channel curves' values."));
    d->resetButton->setWhatsThis(i18n("If you press this button, all curves' values "
                                      "from the currently selected channel "
                                      "will be reset to the default values."));

    QHBoxLayout* l3 = new QHBoxLayout();
    l3->addWidget(typeBox);
    l3->addWidget(d->pickerBox);
    l3->addStretch(10);
    l3->addWidget(d->resetButton);

    // -------------------------------------------------------------

    QGridLayout* mainLayout = new QGridLayout();
    mainLayout->addWidget(curveBox,   0, 0, 1, 1);
    mainLayout->addLayout(l3,         1, 0, 1, 1);
    mainLayout->setRowStretch(2, 10);
    mainLayout->setMargin(0);
    mainLayout->setSpacing(KDialog::spacingHint());
    setLayout(mainLayout);

    // default: disable all control widgets
    enableHGradient(false);
    enableVGradient(false);
    enableControlWidgets(false);

    // -------------------------------------------------------------

    connect(d->curvesWidget, SIGNAL(signalCurvesChanged()),
            this, SIGNAL(signalCurvesChanged()));

    connect(d->pickerType, SIGNAL(buttonReleased(int)),
            this, SIGNAL(signalPickerChanged(int)));

    connect(d->resetButton, SIGNAL(clicked()),
            this, SLOT(slotResetChannel()));

    connect(d->curveType, SIGNAL(buttonClicked(int)),
            this, SLOT(slotCurveTypeChanged(int)));
}

CurvesBox::~CurvesBox()
{
    delete d;
}

void CurvesBox::enablePickers(bool enable)
{
    d->pickBlack->setVisible(enable);
    d->pickGray->setVisible(enable);
    d->pickWhite->setVisible(enable);
}

void CurvesBox::enableHGradient(bool enable)
{
    d->hGradient->setVisible(enable);
}

void CurvesBox::enableVGradient(bool enable)
{
    d->vGradient->setVisible(enable);
}

void CurvesBox::enableGradients(bool enable)
{
    enableHGradient(enable);
    enableVGradient(enable);
}

void CurvesBox::enableResetButton(bool enable)
{
    d->resetButton->setVisible(enable);
}

void CurvesBox::enableCurveTypes(bool enable)
{
    d->curveFree->setVisible(enable);
    d->curveSmooth->setVisible(enable);
}

void CurvesBox::enableControlWidgets(bool enable)
{
    enablePickers(enable);
    enableResetButton(enable);
    enableCurveTypes(enable);
}

void CurvesBox::slotCurveTypeChanged(int type)
{
    switch(type)
    {
       case SmoothDrawing:
       {
          d->curvesWidget->curves()->setCurveType(d->curvesWidget->m_channelType, ImageCurves::CURVE_SMOOTH);
          d->pickerBox->setEnabled(true);
          break;
       }

       case FreeDrawing:
       {
          d->curvesWidget->curves()->setCurveType(d->curvesWidget->m_channelType, ImageCurves::CURVE_FREE);
          d->pickerBox->setEnabled(false);
          break;
       }
    }

    d->curvesWidget->curveTypeChanged();
    emit signalCurveTypeChanged(type);
}

void CurvesBox::setScale(int type)
{
    d->curvesWidget->m_scaleType = type;
    d->curvesWidget->repaint();
}

void CurvesBox::setChannel(int channel)
{
    d->currentChannel = channel;

    switch (channel)
    {
        case LuminosityChannel:
            d->curvesWidget->m_channelType = LuminosityChannel;
            d->hGradient->setColors(QColor("white"), QColor("black"));
            d->vGradient->setColors(QColor("white"), QColor("black"));
            break;

        case RedChannel:
            d->curvesWidget->m_channelType = RedChannel;
            d->hGradient->setColors(QColor("red"), QColor("black"));
            d->vGradient->setColors(QColor("red"), QColor("black"));
            break;

        case GreenChannel:
            d->curvesWidget->m_channelType = GreenChannel;
            d->hGradient->setColors(QColor("green"), QColor("black"));
            d->vGradient->setColors(QColor("green"), QColor("black"));
            break;

        case BlueChannel:
            d->curvesWidget->m_channelType = BlueChannel;
            d->hGradient->setColors(QColor("blue"), QColor("black"));
            d->vGradient->setColors(QColor("blue"), QColor("black"));
            break;

        case AlphaChannel:
            d->curvesWidget->m_channelType = AlphaChannel;
            d->hGradient->setColors(QColor("white"), QColor("black"));
            d->vGradient->setColors(QColor("white"), QColor("black"));
            break;
    }

    d->curveType->button(d->curvesWidget->curves()->getCurveType(channel))->setChecked(true);
    d->curvesWidget->repaint();
}

int CurvesBox::channel() const
{
    return d->currentChannel;
}

int CurvesBox::picker() const
{
    return d->pickerType->checkedId();
}

void CurvesBox::resetPickers()
{
    d->pickerType->setExclusive(false);
    d->pickBlack->setChecked(false);
    d->pickGray->setChecked(false);
    d->pickWhite->setChecked(false);
    d->pickerType->setExclusive(true);
    emit signalPickerChanged(NoPicker);
}

void CurvesBox::resetChannel(int channel)
{
    d->curvesWidget->curves()->curvesChannelReset(channel);
    d->curvesWidget->repaint();
}

void CurvesBox::slotResetChannel()
{
    resetChannel(d->currentChannel);
    emit signalChannelReset(d->currentChannel);
}

void CurvesBox::slotResetChannels()
{
    resetChannels();
}

void CurvesBox::resetChannels()
{
    for (int channel = 0; channel < 5; ++channel)
    {
        d->curvesWidget->curves()->curvesChannelReset(channel);
    }
    reset();
}

void CurvesBox::reset()
{
    d->curvesWidget->curves()->setCurveType(d->curvesWidget->m_channelType, ImageCurves::CURVE_SMOOTH);
    d->curvesWidget->reset();
}

void CurvesBox::readCurveSettings(KConfigGroup& group)
{
    for (int i = 0 ; i < 5 ; ++i)
    {
        d->curvesWidget->curves()->curvesChannelReset(i);
        d->curvesWidget->curves()->setCurveType(i,
                (ImageCurves::CurveType)group.readEntry(QString("CurveTypeChannel%1").arg(i),
                        (int)ImageCurves::CURVE_SMOOTH));

        QPoint disable(-1, -1);
        for (int j = 0 ; j < 17 ; ++j)
        {
            QPoint p = group.readEntry(QString("Channel%1Point%2").arg(i).arg(j), disable);

            if (d->sixteenBit && p.x() != -1)
            {
                p.setX(p.x()*255);
                p.setY(p.y()*255);
            }

            d->curvesWidget->curves()->setCurvePoint(i, j, p);
        }

        d->curvesWidget->curves()->curvesCalculateCurve(i);
    }
}

void CurvesBox::writeCurveSettings(KConfigGroup& group)
{
    for (int i = 0 ; i < 5 ; ++i)
    {
        group.writeEntry(QString("CurveTypeChannel%1").arg(i), d->curvesWidget->curves()->getCurveType(i));

        for (int j = 0 ; j < 17 ; ++j)
        {
            QPoint p = d->curvesWidget->curves()->getCurvePoint(i, j);

            if (d->sixteenBit && p.x() != -1)
            {
                p.setX(p.x()/255);
                p.setY(p.y()/255);
            }

            group.writeEntry(QString("Channel%1Point%2").arg(i).arg(j), p);
        }
    }
}

ImageCurves* CurvesBox::curves() const
{
    return d->curvesWidget->curves();
}

void CurvesBox::setCurveGuide(const DColor& color)
{
    d->curvesWidget->setCurveGuide(color);
}

} // namespace Digikam

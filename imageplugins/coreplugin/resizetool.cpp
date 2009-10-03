/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-04-07
 * Description : a tool to resize an image
 *
 * Copyright (C) 2005-2009 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "resizetool.h"
#include "resizetool.moc"

// C++ includes

#include <cmath>

// Qt includes

#include <QBrush>
#include <QCheckBox>
#include <QCloseEvent>
#include <QComboBox>
#include <QCustomEvent>
#include <QEvent>
#include <QFile>
#include <QFrame>
#include <QGridLayout>
#include <QGroupBox>
#include <QImage>
#include <QLabel>
#include <QPixmap>
#include <QPushButton>
#include <QTimer>
#include <QVBoxLayout>

// KDE includes

#include <kapplication.h>
#include <kcursor.h>

#include <kfiledialog.h>
#include <kglobal.h>
#include <kglobalsettings.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kseparator.h>
#include <kstandarddirs.h>
#include <ktabwidget.h>
#include <ktoolinvocation.h>
#include <kurllabel.h>

// LibKDcraw includes

#include <libkdcraw/rnuminput.h>

// Local includes

#include "dimg.h"
#include "imageiface.h"
#include "imagewidget.h"
#include "editortoolsettings.h"
#include "editortooliface.h"
#include "dimgthreadedfilter.h"
#include "greycstorationiface.h"
#include "greycstorationwidget.h"
#include "greycstorationsettings.h"

using namespace KDcrawIface;
using namespace Digikam;

namespace DigikamImagesPluginCore
{

class ResizeImage : public DImgThreadedFilter
{

public:

    ResizeImage(DImg *orgImage, int newWidth, int newHeight, QObject *parent=0)
        : DImgThreadedFilter(orgImage, parent, "resizeimage")

    {
        m_newWidth  = newWidth;
        m_newHeight = newHeight;
        initFilter();
    };

    ~ResizeImage(){};

private:

    void filterImage()
    {
        postProgress(25);
        m_destImage = m_orgImage.copy();
        postProgress(50);
        m_destImage.resize(m_newWidth, m_newHeight);
        postProgress(75);
    };

private:

    int m_newWidth;
    int m_newHeight;
};

// -------------------------------------------------------------

class ResizeToolPriv
{
public:

    ResizeToolPriv()
    {
        previewWidget        = 0;
        preserveRatioBox     = 0;
        useGreycstorationBox = 0;
        mainTab              = 0;
        gboxSettings         = 0;
        wInput               = 0;
        hInput               = 0;
        wpInput              = 0;
        hpInput              = 0;
        settingsWidget       = 0;
        cimgLogoLabel        = 0;
        restorationTips      = 0;
    }

    int                   orgWidth;
    int                   orgHeight;
    int                   prevW;
    int                   prevH;

    double                prevWP;
    double                prevHP;

    QLabel               *restorationTips;

    QCheckBox            *preserveRatioBox;
    QCheckBox            *useGreycstorationBox;

    KTabWidget           *mainTab;

    KUrlLabel            *cimgLogoLabel;

    ImageWidget          *previewWidget;

    RIntNumInput         *wInput;
    RIntNumInput         *hInput;

    RDoubleNumInput      *wpInput;
    RDoubleNumInput      *hpInput;

    EditorToolSettings   *gboxSettings;
    GreycstorationWidget *settingsWidget;
};

// -------------------------------------------------------------

ResizeTool::ResizeTool(QObject* parent)
          : EditorToolThreaded(parent), d(new ResizeToolPriv)
{
    setObjectName("resizeimage");
    setToolName(i18n("Resize Image"));
    setToolIcon(SmallIcon("transform-scale"));

    d->previewWidget = new ImageWidget("resizeimage Tool", 0, QString(),
                                       false, ImageGuideWidget::HVGuideMode, false);

    setToolView(d->previewWidget);

    // -------------------------------------------------------------

    d->gboxSettings = new EditorToolSettings;
    d->gboxSettings->setButtons(EditorToolSettings::Default|
                                EditorToolSettings::Try|
                                EditorToolSettings::Ok|
                                EditorToolSettings::Load|
                                EditorToolSettings::SaveAs|
                                EditorToolSettings::Cancel);

    ImageIface iface(0, 0);
    d->orgWidth  = iface.originalWidth();
    d->orgHeight = iface.originalHeight();
    d->prevW     = d->orgWidth;
    d->prevH     = d->orgHeight;
    d->prevWP    = 100.0;
    d->prevHP    = 100.0;

    // -------------------------------------------------------------

    QGridLayout* gridSettings = new QGridLayout(d->gboxSettings->plainPage());
    d->mainTab                = new KTabWidget(d->gboxSettings->plainPage());
    QWidget* firstPage        = new QWidget(d->mainTab);
    QGridLayout* grid         = new QGridLayout(firstPage);

    d->mainTab->addTab(firstPage, i18n("New Size"));

    QLabel *label1 = new QLabel(i18n("Width:"), firstPage);
    d->wInput      = new RIntNumInput(firstPage);
    d->wInput->setRange(1, qMax(d->orgWidth * 10, 9999), 1);
    d->wInput->setDefaultValue(d->orgWidth);
    d->wInput->setSliderEnabled(true);
    d->wInput->setObjectName("wInput");
    d->wInput->setWhatsThis( i18n("Set here the new image width in pixels."));

    QLabel *label2 = new QLabel(i18n("Height:"), firstPage);
    d->hInput      = new RIntNumInput(firstPage);
    d->hInput->setRange(1, qMax(d->orgHeight * 10, 9999), 1);
    d->hInput->setDefaultValue(d->orgHeight);
    d->hInput->setSliderEnabled(true);
    d->hInput->setObjectName("hInput");
    d->hInput->setWhatsThis( i18n("New image height in pixels (px)."));

    QLabel *label3 = new QLabel(i18n("Width (%):"), firstPage);
    d->wpInput     = new RDoubleNumInput(firstPage);
    d->wpInput->input()->setRange(1.0, 999.0, 1.0, true);
    d->wpInput->setDefaultValue(100.0);
    d->wpInput->setObjectName("wpInput");
    d->wpInput->setWhatsThis( i18n("New image width in percent (%)."));

    QLabel *label4 = new QLabel(i18n("Height (%):"), firstPage);
    d->hpInput     = new RDoubleNumInput(firstPage);
    d->hpInput->input()->setRange(1.0, 999.0, 1.0, true);
    d->hpInput->setDefaultValue(100.0);
    d->hpInput->setObjectName("hpInput");
    d->hpInput->setWhatsThis( i18n("New image height in percent (%)."));

    d->preserveRatioBox = new QCheckBox(i18n("Maintain aspect ratio"), firstPage);
    d->preserveRatioBox->setWhatsThis( i18n("Enable this option to maintain aspect "
                                            "ratio with new image sizes."));

    d->cimgLogoLabel = new KUrlLabel(firstPage);
    d->cimgLogoLabel->setText(QString());
    d->cimgLogoLabel->setUrl("http://cimg.sourceforge.net");
    d->cimgLogoLabel->setPixmap(QPixmap(KStandardDirs::locate("data", "digikam/data/logo-cimg.png")));
    d->cimgLogoLabel->setToolTip(i18n("Visit CImg library website"));

    d->useGreycstorationBox = new QCheckBox(i18n("Restore photograph (slow)"), firstPage);
    d->useGreycstorationBox->setWhatsThis( i18n("Enable this option to scale-up an image to a huge size. "
                                                "<b>Warning</b>: This process can take some time."));

    d->restorationTips = new QLabel(i18n("<b>Note:</b> use Restoration Mode to scale-up an image to a huge size. "
                                         "This process can take some time."), firstPage);
    d->restorationTips->setWordWrap(true);

    grid->addWidget(d->preserveRatioBox,       0, 0, 1, 3);
    grid->addWidget(label1,                    1, 0, 1, 1);
    grid->addWidget(d->wInput,                 1, 1, 1, 2);
    grid->addWidget(label2,                    2, 0, 1, 1);
    grid->addWidget(d->hInput,                 2, 1, 1, 2);
    grid->addWidget(label3,                    3, 0, 1, 1);
    grid->addWidget(d->wpInput,                3, 1, 1, 2);
    grid->addWidget(label4,                    4, 0, 1, 1);
    grid->addWidget(d->hpInput,                4, 1, 1, 2);
    grid->addWidget(new KSeparator(firstPage), 5, 0, 1, 3);
    grid->addWidget(d->cimgLogoLabel,          6, 0, 3, 1);
    grid->addWidget(d->useGreycstorationBox,   6, 1, 1, 2);
    grid->addWidget(d->restorationTips,        7, 1, 1, 2);
    grid->setRowStretch(8, 10);
    grid->setMargin(d->gboxSettings->spacingHint());
    grid->setSpacing(d->gboxSettings->spacingHint());

    // -------------------------------------------------------------

    d->settingsWidget = new GreycstorationWidget(d->mainTab);

    gridSettings->addWidget(d->mainTab,                               0, 1, 1, 1);
    gridSettings->addWidget(new QLabel(d->gboxSettings->plainPage()), 1, 1, 1, 1);
    gridSettings->setMargin(d->gboxSettings->spacingHint());
    gridSettings->setSpacing(d->gboxSettings->spacingHint());
    gridSettings->setRowStretch(2, 10);

    setToolSettings(d->gboxSettings);
    init();

    // -------------------------------------------------------------

    connect(d->cimgLogoLabel, SIGNAL(leftClickedUrl(const QString&)),
            this, SLOT(processCImgUrl(const QString&)));

    connect(d->wInput, SIGNAL(valueChanged(int)),
            this, SLOT(slotValuesChanged()));

    connect(d->hInput, SIGNAL(valueChanged(int)),
            this, SLOT(slotValuesChanged()));

    connect(d->wpInput, SIGNAL(valueChanged(double)),
            this, SLOT(slotValuesChanged()));

    connect(d->hpInput, SIGNAL(valueChanged(double)),
            this, SLOT(slotValuesChanged()));

    connect(d->useGreycstorationBox, SIGNAL(toggled(bool)),
            this, SLOT(slotRestorationToggled(bool)) );

    // -------------------------------------------------------------

    GreycstorationSettings defaults;
    defaults.setResizeDefaultSettings();
    d->settingsWidget->setDefaultSettings(defaults);

    QTimer::singleShot(0, this, SLOT(slotResetSettings()));
}

ResizeTool::~ResizeTool()
{
    delete d;
}

void ResizeTool::readSettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group("resize Tool Dialog");

    GreycstorationSettings settings;
    GreycstorationSettings defaults;
    defaults.setResizeDefaultSettings();

    settings.fastApprox = group.readEntry("FastApprox",    defaults.fastApprox);
    settings.interp     = group.readEntry("Interpolation", defaults.interp);
    settings.amplitude  = group.readEntry("Amplitude",     (double)defaults.amplitude);
    settings.sharpness  = group.readEntry("Sharpness",     (double)defaults.sharpness);
    settings.anisotropy = group.readEntry("Anisotropy",    (double)defaults.anisotropy);
    settings.alpha      = group.readEntry("Alpha",         (double)defaults.alpha);
    settings.sigma      = group.readEntry("Sigma",         (double)defaults.sigma);
    settings.gaussPrec  = group.readEntry("GaussPrec",     (double)defaults.gaussPrec);
    settings.dl         = group.readEntry("Dl",            (double)defaults.dl);
    settings.da         = group.readEntry("Da",            (double)defaults.da);
    settings.nbIter     = group.readEntry("Iteration",     defaults.nbIter);
    settings.tile       = group.readEntry("Tile",          defaults.tile);
    settings.btile      = group.readEntry("BTile",         defaults.btile);
    d->settingsWidget->setSettings(settings);
}

void ResizeTool::writeSettings()
{
    GreycstorationSettings settings = d->settingsWidget->getSettings();
    KConfigGroup group              = KGlobal::config()->group("resize Tool Dialog");

    group.writeEntry("FastApprox",        settings.fastApprox);
    group.writeEntry("Interpolation",     settings.interp);
    group.writeEntry("Amplitude",         (double)settings.amplitude);
    group.writeEntry("Sharpness",         (double)settings.sharpness);
    group.writeEntry("Anisotropy",        (double)settings.anisotropy);
    group.writeEntry("Alpha",             (double)settings.alpha);
    group.writeEntry("Sigma",             (double)settings.sigma);
    group.writeEntry("GaussPrec",         (double)settings.gaussPrec);
    group.writeEntry("Dl",                (double)settings.dl);
    group.writeEntry("Da",                (double)settings.da);
    group.writeEntry("Iteration",         settings.nbIter);
    group.writeEntry("Tile",              settings.tile);
    group.writeEntry("BTile",             settings.btile);
    group.writeEntry("RestorePhotograph", d->useGreycstorationBox->isChecked());
    group.sync();
}

void ResizeTool::slotResetSettings()
{
    GreycstorationSettings settings;
    settings.setResizeDefaultSettings();

    d->settingsWidget->setSettings(settings);
    d->useGreycstorationBox->setChecked(false);
    slotRestorationToggled(d->useGreycstorationBox->isChecked());

    blockWidgetSignals(true);

    d->preserveRatioBox->setChecked(true);
    d->wInput->slotReset();
    d->hInput->slotReset();
    d->wpInput->slotReset();
    d->hpInput->slotReset();

    blockWidgetSignals(false);
}

void ResizeTool::slotValuesChanged()
{
    blockWidgetSignals(true);

    QString s(sender()->objectName());

    if (s == "wInput")
    {
        double val  = d->wInput->value();
        double pval = val / (double)(d->orgWidth) * 100.0;

        d->wpInput->setValue(pval);

        if (d->preserveRatioBox->isChecked())
        {
            int h = (int)(pval * d->orgHeight / 100);

            d->hpInput->setValue(pval);
            d->hInput->setValue(h);
        }
    }
    else if (s == "hInput")
    {
        double val  = d->hInput->value();
        double pval = val / (double)(d->orgHeight) * 100.0;

        d->hpInput->setValue(pval);

        if (d->preserveRatioBox->isChecked())
        {
            int w = (int)(pval * d->orgWidth / 100);

            d->wpInput->setValue(pval);
            d->wInput->setValue(w);
        }
    }
    else if (s == "wpInput")
    {
        double val = d->wpInput->value();
        int w      = (int)(val * d->orgWidth / 100);

        d->wInput->setValue(w);

        if (d->preserveRatioBox->isChecked())
        {
            int h = (int)(val * d->orgHeight / 100);

            d->hpInput->setValue(val);
            d->hInput->setValue(h);
        }
    }
    else if (s == "hpInput")
    {
        double val = d->hpInput->value();
        int h = (int)(val * d->orgHeight / 100);

        d->hInput->setValue(h);

        if (d->preserveRatioBox->isChecked())
        {
            int w = (int)(val * d->orgWidth / 100);

            d->wpInput->setValue(val);
            d->wInput->setValue(w);
        }
    }

    d->prevW  = d->wInput->value();
    d->prevH  = d->hInput->value();
    d->prevWP = d->wpInput->value();
    d->prevHP = d->hpInput->value();

    blockWidgetSignals(false);
}

void ResizeTool::prepareEffect()
{
    if (d->prevW  != d->wInput->value()  || d->prevH  != d->hInput->value() ||
        d->prevWP != d->wpInput->value() || d->prevHP != d->hpInput->value())
        slotValuesChanged();

    d->settingsWidget->setEnabled(false);
    d->preserveRatioBox->setEnabled(false);
    d->useGreycstorationBox->setEnabled(false);
    d->wInput->setEnabled(false);
    d->hInput->setEnabled(false);
    d->wpInput->setEnabled(false);
    d->hpInput->setEnabled(false);

    ImageIface* iface = d->previewWidget->imageIface();
    int w             = iface->previewWidth();
    int h             = iface->previewHeight();
    DImg imTemp       = iface->getOriginalImg()->smoothScale(w, h, Qt::KeepAspectRatio);
    int new_w         = (int)(w*d->wpInput->value()/100.0);
    int new_h         = (int)(h*d->hpInput->value()/100.0);

    if (d->useGreycstorationBox->isChecked())
    {
        setFilter(dynamic_cast<DImgThreadedFilter*>(
                  new GreycstorationIface(&imTemp,
                                          d->settingsWidget->getSettings(),
                                          GreycstorationIface::Resize,
                                          new_w, new_h,
                                          QImage(),
                                          this)));
    }
    else
    {
        // See B.K.O #152192: CImg resize() sound like defective or unadapted
        // to resize image without good quality.
        setFilter(dynamic_cast<DImgThreadedFilter*>(new ResizeImage(&imTemp, new_w, new_h, this)));
    }
}

void ResizeTool::prepareFinal()
{
    if (d->prevW  != d->wInput->value()  || d->prevH  != d->hInput->value() ||
        d->prevWP != d->wpInput->value() || d->prevHP != d->hpInput->value())
        slotValuesChanged();

    d->mainTab->setCurrentIndex(0);
    d->settingsWidget->setEnabled(false);
    d->preserveRatioBox->setEnabled(false);
    d->useGreycstorationBox->setEnabled(false);
    d->wInput->setEnabled(false);
    d->hInput->setEnabled(false);
    d->wpInput->setEnabled(false);
    d->hpInput->setEnabled(false);

    ImageIface iface(0, 0);

    if (d->useGreycstorationBox->isChecked())
    {
        setFilter(dynamic_cast<DImgThreadedFilter*>(
                  new GreycstorationIface(iface.getOriginalImg(),
                                          d->settingsWidget->getSettings(),
                                          GreycstorationIface::Resize,
                                          d->wInput->value(),
                                          d->hInput->value(),
                                          QImage(),
                                          this)));
    }
    else
    {
        // See B.K.O #152192: CImg resize() sound like defective or unadapted
        // to resize image without good quality.
        setFilter(dynamic_cast<DImgThreadedFilter*>(new ResizeImage(iface.getOriginalImg(),
                                                    d->wInput->value(),
                                                    d->hInput->value(),
                                                    this)));
    }
}

void ResizeTool::putPreviewData()
{
    ImageIface* iface = d->previewWidget->imageIface();
    int w             = iface->previewWidth();
    int h             = iface->previewHeight();
    DImg imTemp       = filter()->getTargetImage().smoothScale(w, h, Qt::KeepAspectRatio);
    DImg imDest(w, h, filter()->getTargetImage().sixteenBit(),
                filter()->getTargetImage().hasAlpha());

    QColor background = toolView()->backgroundRole();
    imDest.fill(DColor(background, filter()->getTargetImage().sixteenBit()));
    imDest.bitBltImage(&imTemp, (w-imTemp.width())/2, (h-imTemp.height())/2);

    iface->putPreviewImage((imDest.smoothScale(iface->previewWidth(), iface->previewHeight())).bits());
    d->previewWidget->updatePreview();
}

void ResizeTool::renderingFinished()
{
    d->settingsWidget->setEnabled(true);
    d->useGreycstorationBox->setEnabled(true);
    d->preserveRatioBox->setEnabled(true);
    d->wInput->setEnabled(true);
    d->hInput->setEnabled(true);
    d->wpInput->setEnabled(true);
    d->hpInput->setEnabled(true);
}

void ResizeTool::putFinalData()
{
    ImageIface iface(0, 0);
    DImg targetImage = filter()->getTargetImage();
    iface.putOriginalImage(i18n("Resize"),
                            targetImage.bits(),
                            targetImage.width(), targetImage.height());
}

void ResizeTool::blockWidgetSignals(bool b)
{
    d->preserveRatioBox->blockSignals(b);
    d->wInput->blockSignals(b);
    d->hInput->blockSignals(b);
    d->wpInput->blockSignals(b);
    d->hpInput->blockSignals(b);
}

void ResizeTool::slotRestorationToggled(bool b)
{
    d->settingsWidget->setEnabled(b);
    d->cimgLogoLabel->setEnabled(b);
    toolSettings()->enableButton(EditorToolSettings::Load, b);
    toolSettings()->enableButton(EditorToolSettings::SaveAs, b);
}

void ResizeTool::processCImgUrl(const QString& url)
{
    KToolInvocation::invokeBrowser(url);
}

void ResizeTool::slotLoadSettings()
{
    KUrl loadBlowupFile = KFileDialog::getOpenUrl(KGlobalSettings::documentPath(),
                                       QString( "*" ), kapp->activeWindow(),
                                       QString( i18n("Photograph Resizing Settings File to Load")) );
    if ( loadBlowupFile.isEmpty() )
       return;

    QFile file(loadBlowupFile.toLocalFile());

    if ( file.open(QIODevice::ReadOnly) )
    {
        if (!d->settingsWidget->loadSettings(file, QString("# Photograph Resizing Configuration File")))
        {
           KMessageBox::error(kapp->activeWindow(),
                        i18n("\"%1\" is not a Photograph Resizing settings text file.",
                        loadBlowupFile.fileName()));
           file.close();
           return;
        }
    }
    else
    {
        KMessageBox::error(kapp->activeWindow(),
                           i18n("Cannot load settings from the Photograph Resizing text file."));
    }

    file.close();
}

void ResizeTool::slotSaveAsSettings()
{
    KUrl saveBlowupFile = KFileDialog::getSaveUrl(KGlobalSettings::documentPath(),
                                       QString( "*" ), kapp->activeWindow(),
                                       QString( i18n("Photograph Resizing Settings File to Save")) );
    if ( saveBlowupFile.isEmpty() )
       return;

    QFile file(saveBlowupFile.toLocalFile());

    if ( file.open(QIODevice::WriteOnly) )
        d->settingsWidget->saveSettings(file, QString("# Photograph Resizing Configuration File"));
    else
        KMessageBox::error(kapp->activeWindow(), i18n("Cannot save settings to the Photograph Resizing text file."));

    file.close();
}

}  // namespace DigikamImagesPluginCore

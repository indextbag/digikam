/* ============================================================
 * File  : imageguiclient.h
 * Author: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Date  : 2004-02-12
 * Description : 
 * 
 * Copyright 2004 by Renchi Raju

 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published bythe Free Software Foundation;
 * either version 2, or (at your option)
 * any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * ============================================================ */

#ifndef IMAGEGUICLIENT_H
#define IMAGEGUICLIENT_H

#include <qobject.h>
#include <guiclient.h>

class QWidget;
class KAction;
class KToggleAction;

class ImageGUIClient : public QObject, public Digikam::GUIClient
{
    Q_OBJECT

public:
    
    ImageGUIClient(QWidget *parent);

    virtual QStringList guiDefinition() const;
    
signals:

    void signalNext();
    void signalPrev();
    void signalFirst();
    void signalLast();
    void signalExit();

    void signalZoomPlus();
    void signalZoomMinus();
    void signalZoomFit();
    
    void signalRotate90();    
    void signalRotate180();    
    void signalRotate270();
    void signalFlipHoriz();
    void signalFlipVert();
    void signalCrop();
    void signalResize();

    void signalSave();
    void signalSaveAs();
    void signalRestore();

private slots:

    void slotHelp();
    void slotAboutApp();
    void slotAboutKDE();

private:

    QWidget       *m_parent;

    KAction       *m_navNextAction;
    KAction       *m_navPrevAction;
    KAction       *m_navFirstAction;
    KAction       *m_navLastAction;

    KAction       *m_saveAction;
    KAction       *m_saveAsAction;
    KAction       *m_restoreAction;
    
    KAction       *m_zoomPlusAction;
    KAction       *m_zoomMinusAction;
    KToggleAction *m_zoomFitAction;

    KActionMenu   *m_rotateAction;
    KActionMenu   *m_flipAction;
    KAction       *m_cropAction;

    friend class ImageWindow;
};

#endif /* IMAGEGUICLIENT_H */

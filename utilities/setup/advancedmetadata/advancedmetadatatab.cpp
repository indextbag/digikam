/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2015-06-16
 * Description : Advanced Configuration tab for metadata.
 *
 * Copyright (C) 2015 by Veaceslav Munteanu <veaceslav dot munteanu90 at gmail.com>
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

#include "advancedmetadatatab.h"

#include <QApplication>
#include <QVBoxLayout>
#include <QComboBox>
#include <QHBoxLayout>
#include <QCheckBox>
#include <QPushButton>
#include <QListView>
#include <QStandardItemModel>
#include <QStandardItem>
#include <QDebug>

#include "klocale.h"
#include "dmetadatasettings.h"
#include "namespacelistview.h"
#include "namespaceeditdlg.h"

namespace Digikam
{

class AdvancedMetadataTab::Private
{
public:
    Private()
    {

    }
    QComboBox* metadataType;
    QComboBox* operationType;
    QPushButton* addButton;
    QPushButton* editButton;
    QPushButton* deleteButton;
    QPushButton* moveUpButton;
    QPushButton* moveDownButton;
    QPushButton* resetButton;
    QCheckBox*   unifyReadWrite;
    QList<QStandardItemModel*> models;
    NamespaceListView* namespaceView;
    DMetadataSettingsContainer container;
};
AdvancedMetadataTab::AdvancedMetadataTab(QWidget* parent)
    :QWidget(parent), d(new Private())
{
    // ---------- Advanced Configuration Panel -----------------------------
    d->container = DMetadataSettings::instance()->settings();
    setUi();
    setModels();
    connectButtons();

    d->unifyReadWrite->setChecked(d->container.unifyReadWrite);
    connect(d->unifyReadWrite, SIGNAL(toggled(bool)), this, SLOT(slotUnifyChecked(bool)));
    connect(d->metadataType, SIGNAL(currentIndexChanged(int)), this, SLOT(slotIndexChanged()));
    connect(d->operationType, SIGNAL(currentIndexChanged(int)), this, SLOT(slotIndexChanged()));

    if(d->unifyReadWrite->isChecked())
    {
        d->operationType->setEnabled(false);
    }
}

AdvancedMetadataTab::~AdvancedMetadataTab()
{
    delete d;
}

void AdvancedMetadataTab::slotResetView()
{
    d->models.at(getModelIndex())->clear();
    switch(getModelIndex())
    {
    case 0:
        setModelData(d->models.at(0),d->container.readTagNamespaces);
        break;
    case 1:
        setModelData(d->models.at(1),d->container.readRatingNamespaces);
        break;
    case 2:
        setModelData(d->models.at(2),d->container.readCommentNamespaces);
        break;
    case 3:
        setModelData(d->models.at(3),d->container.writeTagNamespaces);
        break;
    case 4:
        setModelData(d->models.at(4),d->container.writeRatingNamespaces);
        break;
    case 5:
        setModelData(d->models.at(5),d->container.writeCommentNamespaces);
        break;
    default:
        qDebug() << "warning, Unknown case";
    }

    d->namespaceView->setModel(d->models.at(getModelIndex()));
}

void AdvancedMetadataTab::slotAddNewNamespace()
{
    NamespaceEntry entry;

    if (!NamespaceEditDlg::create(qApp->activeWindow(), entry))
    {
        return;
    }

    QStandardItem* root = d->models.at(getModelIndex())->invisibleRootItem();
    QString text = entry.namespaceName + QLatin1String(",") + i18n("Separator:") + entry.separator;
    QStandardItem* item = new QStandardItem(text);
    item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsDragEnabled | Qt::ItemIsEnabled);
    root->appendRow(item);
    d->container.readTagNamespaces.append(entry);
}

void AdvancedMetadataTab::slotEditNamespace()
{

    if(!d->namespaceView->currentIndex().isValid())
        return;

    NamespaceEntry entry = d->container.readTagNamespaces.at(d->namespaceView->currentIndex().row());


    if (!NamespaceEditDlg::edit(qApp->activeWindow(), entry))
    {
        return;
    }


    QStandardItem* root = d->models.at(getModelIndex())->invisibleRootItem();
    QStandardItem* item = root->child(d->namespaceView->currentIndex().row());
    QString text = entry.namespaceName + QLatin1String(",") + i18n("Separator:") + entry.separator;
    item->setText(text);

}

void AdvancedMetadataTab::applySettings()
{
    d->container.readTagNamespaces.clear();
    saveModelData(d->models.at(READ_TAGS),d->container.readTagNamespaces);

    d->container.readRatingNamespaces.clear();
    saveModelData(d->models.at(READ_RATINGS),d->container.readRatingNamespaces);

    d->container.readCommentNamespaces.clear();
    saveModelData(d->models.at(READ_COMMENTS),d->container.readCommentNamespaces);

    d->container.writeTagNamespaces.clear();
    saveModelData(d->models.at(WRITE_TAGS),d->container.writeTagNamespaces);

    d->container.writeRatingNamespaces.clear();
    saveModelData(d->models.at(WRITE_RATINGS),d->container.writeRatingNamespaces);

    d->container.writeCommentNamespaces.clear();
    saveModelData(d->models.at(WRITE_COMMENTS),d->container.writeCommentNamespaces);

    d->container.unifyReadWrite = d->unifyReadWrite->isChecked();
    DMetadataSettings::instance()->setSettings(d->container);
}

void AdvancedMetadataTab::slotUnifyChecked(bool value)
{
    d->operationType->setDisabled(value);
    d->container.unifyReadWrite = value;
    if(true)
        d->operationType->setCurrentIndex(0);
    slotIndexChanged();
}

void AdvancedMetadataTab::slotIndexChanged()
{
    d->namespaceView->setModel(d->models.at(getModelIndex()));
}


void AdvancedMetadataTab::connectButtons()
{
    connect(d->addButton, SIGNAL(clicked()), this, SLOT(slotAddNewNamespace()));
    connect(d->editButton, SIGNAL(clicked()), this, SLOT(slotEditNamespace()));
    connect(d->deleteButton, SIGNAL(clicked()), d->namespaceView, SLOT(slotDeleteSelected()));
    connect(d->resetButton, SIGNAL(clicked()), this, SLOT(slotResetView()));
    connect(d->moveUpButton, SIGNAL(clicked()), d->namespaceView, SLOT(slotMoveItemUp()));
    connect(d->moveDownButton, SIGNAL(clicked()), d->namespaceView, SLOT(slotMoveItemDown()));
}

void AdvancedMetadataTab::setModelData(QStandardItemModel* model, QList<NamespaceEntry> const &container)
{

    QStandardItem* root = model->invisibleRootItem();
    for(NamespaceEntry e : container)
    {
        QStandardItem* item = new QStandardItem(e.namespaceName);
        item->setData(e.namespaceName,NAME_ROLE);
        item->setData((int)e.tagPaths, ISTAG_ROLE);
        item->setData(e.separator, SEPARATOR_ROLE);
        item->setData(e.extraXml, EXTRAXML_ROLE);
        item->setData((int)e.nsType, NSTYPE_ROLE);
        if(e.nsType == NamespaceEntry::RATING)
        {
           item->setData(e.convertRatio.at(0), ZEROSTAR_ROLE);
           item->setData(e.convertRatio.at(1), ONESTAR_ROLE);
           item->setData(e.convertRatio.at(2), TWOSTAR_ROLE);
           item->setData(e.convertRatio.at(3), THREESTAR_ROLE);
           item->setData(e.convertRatio.at(4), FOURSTAR_ROLE);
           item->setData(e.convertRatio.at(5), FIVESTAR_ROLE);
        }
        item->setData((int)e.specialOpts, SPECIALOPTS_ROLE);
        qDebug() << "Loading ++++++++ " << e.namespaceName << e.specialOpts;
        item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsDragEnabled | Qt::ItemIsEnabled);
        root->appendRow(item);
    }

}

void AdvancedMetadataTab::setUi()
{
    QVBoxLayout* const advancedConfLayout = new QVBoxLayout(this);

    QHBoxLayout* const topLayout = new QHBoxLayout(this);
    QHBoxLayout* const bottomLayout = new QHBoxLayout(this);

    //--- Top layout ----------------
    d->metadataType = new QComboBox(this);

    d->operationType = new QComboBox(this);

    d->metadataType->insertItems(0, QStringList() << i18n("Tags") << i18n("Ratings") << i18n("Comments"));

    d->operationType->insertItems(0, QStringList() << i18n("Read Options") << i18n("Write Options"));

    d->unifyReadWrite = new QCheckBox(i18n("Unify read and write"));


    topLayout->addWidget(d->metadataType);
    topLayout->addWidget(d->operationType);
    topLayout->addWidget(d->unifyReadWrite);

    //------------ Bottom Layout-------------
    // View
    d->namespaceView = new NamespaceListView(this);


    // Buttons
    QVBoxLayout* buttonsLayout = new QVBoxLayout(this);
    buttonsLayout->setAlignment(Qt::AlignTop);
    d->addButton = new QPushButton(i18n("Add"));
    d->editButton = new QPushButton(i18n("Edit"));
    d->deleteButton = new QPushButton(i18n("Delete"));

    d->moveUpButton = new QPushButton(i18n("Move Up"));
    d->moveDownButton = new QPushButton(i18n("Move Down"));
    d->resetButton = new QPushButton(i18n("Reset"));


    buttonsLayout->addWidget(d->addButton);
    buttonsLayout->addWidget(d->editButton);
    buttonsLayout->addWidget(d->deleteButton);
    buttonsLayout->addWidget(d->moveUpButton);
    buttonsLayout->addWidget(d->moveDownButton);
    buttonsLayout->addWidget(d->resetButton);

    QVBoxLayout* vbox = new QVBoxLayout(this);
    vbox->addWidget(d->namespaceView);

    bottomLayout->addLayout(vbox);
    bottomLayout->addLayout(buttonsLayout);

    advancedConfLayout->addLayout(topLayout);
    advancedConfLayout->addLayout(bottomLayout);

    this->setLayout(advancedConfLayout);
}

int AdvancedMetadataTab::getModelIndex()
{
    if(d->unifyReadWrite->isChecked())
    {
        return d->metadataType->currentIndex();
    }
    else
    {
        // read operation = 3*0 + (0, 1, 2)
        // write operation = 3*1 + (0, 1, 2) = (3, 4 ,5)
        return (3*d->operationType->currentIndex()) + d->metadataType->currentIndex();
    }
}

void AdvancedMetadataTab::setModels()
{
    // Append 6 empty models
    for(int i = 0 ; i < 6; i++)
        d->models.append(new QStandardItemModel(this));

    setModelData(d->models.at(READ_TAGS), d->container.readTagNamespaces);
    setModelData(d->models.at(READ_RATINGS), d->container.readRatingNamespaces);
    setModelData(d->models.at(READ_COMMENTS), d->container.readCommentNamespaces);

    setModelData(d->models.at(WRITE_TAGS), d->container.writeTagNamespaces);
    setModelData(d->models.at(WRITE_RATINGS), d->container.writeRatingNamespaces);
    setModelData(d->models.at(WRITE_COMMENTS), d->container.writeCommentNamespaces);

    slotIndexChanged();

}

void AdvancedMetadataTab::saveModelData(QStandardItemModel *model, QList<NamespaceEntry> &container)
{
    QStandardItem* root = model->invisibleRootItem();

    if(!root->hasChildren())
        return;

    for(int i = 0 ; i < root->rowCount(); i++)
    {
        NamespaceEntry ns;
        QStandardItem  *current = root->child(i);
        ns.namespaceName        = current->data(NAME_ROLE).toString();
        ns.tagPaths             = (NamespaceEntry::TagType)current->data(ISTAG_ROLE).toInt();
        ns.separator            = current->data(SEPARATOR_ROLE).toString();
        ns.extraXml             = current->data(EXTRAXML_ROLE).toString();
        ns.nsType               = (NamespaceEntry::NamespaceType)current->data(NSTYPE_ROLE).toInt();
        if(ns.nsType == NamespaceEntry::RATING)
        {
            ns.convertRatio.append(current->data(ZEROSTAR_ROLE).toInt());
            ns.convertRatio.append(current->data(ONESTAR_ROLE).toInt());
            ns.convertRatio.append(current->data(TWOSTAR_ROLE).toInt());
            ns.convertRatio.append(current->data(THREESTAR_ROLE).toInt());
            ns.convertRatio.append(current->data(FOURSTAR_ROLE).toInt());
            ns.convertRatio.append(current->data(FIVESTAR_ROLE).toInt());
        }
        ns.specialOpts          = (NamespaceEntry::SpecialOptions)current->data(SPECIALOPTS_ROLE).toInt();
        ns.index                = i;
        qDebug() << "saving+++++" << ns.namespaceName << " " << ns.index << " " << ns.specialOpts;
        container.append(ns);
    }
}

}



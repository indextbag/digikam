/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2013-03-02
 * Description : Table view: Tree view subelement
 *
 * Copyright (C) 2013 by Michael G. Hansen <mike at mghansen dot de>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#ifndef DIGIKAM_TABLE_VIEW_TREEVIEW_H
#define DIGIKAM_TABLE_VIEW_TREEVIEW_H

// Qt includes

#include <QTreeView>
#include <QWidget>

// Local includes

#include "digikam_export.h"
#include "dragdropimplementations.h"
#include "itemalbummodel.h"
#include "itemfiltermodel.h"
#include "statesavingobject.h"
#include "tableview_columnfactory.h"
#include "tableview_shared.h"
#include "thumbnailloadthread.h"
#include "groupingviewimplementation.h"

class QMenu;
class QContextMenuEvent;

namespace Digikam
{

/// @todo For proper drag-and-drop support, we probably have to implement more
///       of DragDropModelImplementation's functions in the TableViewModel or
///       in the sort model. Subclassing DragDropModelImplementation would not
///       work there, because we want to re-use ItemDragDropHandler...
class TableViewTreeView : public QTreeView, public DragDropViewImplementation, public GroupingViewImplementation
{
    Q_OBJECT

public:

    explicit TableViewTreeView(TableViewShared* const tableViewShared, QWidget* const parent = nullptr);
    virtual ~TableViewTreeView();

    Album* albumAt(const QPoint& pos) const;

protected:

    virtual bool eventFilter(QObject* watched, QEvent* event);

    DECLARE_VIEW_DRAG_DROP_METHODS(QTreeView)

    virtual AbstractItemDragDropHandler* dragDropHandler() const;
    virtual QModelIndex mapIndexForDragDrop(const QModelIndex& index) const;
    virtual QPixmap     pixmapForDrag(const QList<QModelIndex>& indexes) const;
    virtual void wheelEvent(QWheelEvent* event);

    virtual bool hasHiddenGroupedImages(const ItemInfo& info) const;

private:

    void addColumnDescriptionsToMenu(const QList<TableViewColumnDescription>& columnDescriptions, QMenu* const menu);
    void showHeaderContextMenu(QEvent* const event);

private Q_SLOTS:

    void slotHeaderContextMenuAddColumn();
    void slotHeaderContextMenuConfigureColumn();
    void slotHeaderContextMenuActionRemoveColumnTriggered();
    void slotModelGroupingModeChanged();

Q_SIGNALS:

    void signalZoomInStep();
    void signalZoomOutStep();

private:

    class Private;
    const QScopedPointer<Private> d;

    TableViewShared* const s;
};

} // namespace Digikam

#endif // DIGIKAM_TABLE_VIEW_TREEVIEW_H

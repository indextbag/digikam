/* ============================================================
 * Author: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Date  : 2005-05-31
 * Copyright 2005 by Renchi Raju
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

#ifndef FOLDERITEM_H
#define FOLDERITEM_H

#include <qlistview.h>

class FolderItem : public QListViewItem
{
public:

    FolderItem(QListView* parent, const QString& text, bool special=false);
    FolderItem(QListViewItem* parent, const QString& text, bool special=false);

    virtual ~FolderItem();
    
protected:

    void paintCell(QPainter* p, const QColorGroup & cg, int column, int width, int align);
    void setup();

private:

    bool m_special;
};

#endif /* FOLDERITEM_H */

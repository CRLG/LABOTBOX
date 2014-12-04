/****************************************************************************
**
** Copyright (C) 2013 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the examples of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** You may use this file under the terms of the BSD license as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of Digia Plc and its Subsidiary(-ies) nor the names
**     of its contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "SensorsActuatorsList.h"

#include <QDrag>
#include <QDragEnterEvent>
#include <QMimeData>

SensorsActuatorsList::SensorsActuatorsList(QWidget *parent)
    : QListWidget(parent)
{
    setDragEnabled(true);
    setViewMode(QListView::IconMode);
    setIconSize(QSize(32, 32));
    //setSpacing(10);
    setAcceptDrops(true);
    setDropIndicatorShown(true);
}

void SensorsActuatorsList::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->mimeData()->hasFormat("application/x-sensor-actuator-element"))
        event->accept();
    else
        event->ignore();
}

void SensorsActuatorsList::dragMoveEvent(QDragMoveEvent *event)
{
    if (event->mimeData()->hasFormat("application/x-sensor-actuator-element")) {
        event->setDropAction(Qt::MoveAction);
        event->accept();
    } else {
        event->ignore();
    }
}

/*!
 * \brief SensorsActuatorsList::dropEvent réimplémentation du dropEvent dans le cas ou le drop s'effectue dans le même objet objet que
 * le drag en l'occurence la liste des sensors et actuators
 * \param event
 */
void SensorsActuatorsList::dropEvent(QDropEvent *event)
{
    //C'est bien un élément de type sensor ou actuator
    if (event->mimeData()->hasFormat("application/x-sensor-actuator-element"))
    {
        //On récupère la donnée correspondant au mimetype
        QByteArray elementData = event->mimeData()->data("application/x-sensor-actuator-element");
        //On ouvre un flux pour manipuler les données
        QDataStream dataStream(&elementData, QIODevice::ReadOnly);

        //On crée les objets pour stocker les données
        QPixmap pixmap;
        QString name_var;
        //on extrait les données
        dataStream >> pixmap >> name_var;

        //on ajoute l'élément
        addElement(pixmap,name_var);

        event->setDropAction(Qt::MoveAction);
        event->accept();
    } else {
        event->ignore();
    }
}

void SensorsActuatorsList::addElement(QPixmap pixmap, QString name_var)
{
    QListWidgetItem *pieceItem = new QListWidgetItem(this);


    pieceItem->setIcon(QIcon(pixmap));
    pieceItem->setText(name_var);
    pieceItem->setData(Qt::UserRole, QVariant(pixmap));
    //pieceItem->setData(Qt::UserRole+1, location);
    pieceItem->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsDragEnabled);
}


void SensorsActuatorsList::startDrag(Qt::DropActions /*supportedActions*/)
{
    QListWidgetItem *item = currentItem();

    QByteArray itemData;
    QDataStream dataStream(&itemData, QIODevice::WriteOnly);
    QPixmap pixmap = qvariant_cast<QPixmap>(item->data(Qt::UserRole));

    QString var_name = item->text();

    dataStream << pixmap << var_name;

    QMimeData *mimeData = new QMimeData;
    mimeData->setData("application/x-sensor-actuator-element", itemData);

    QDrag *drag = new QDrag(this);
    drag->setMimeData(mimeData);
    drag->setHotSpot(QPoint(16,16));
    drag->setPixmap(pixmap.scaled(32,32));

    if (drag->exec(Qt::MoveAction) == Qt::MoveAction)
        delete takeItem(row(item));
}


#include "SensorsActuatorsPlace.h"

#include <QDrag>
#include <QDragEnterEvent>
#include <QMimeData>
#include <QPainter>

SensorsActuatorsPlace::SensorsActuatorsPlace(QWidget *parent)
    : QWidget(parent)
{
    setAcceptDrops(true);
    setDragEnabled(true);
    setMinimumSize(300, 300);
    setMaximumSize(300, 300);
}

void SensorsActuatorsPlace::setDragEnabled(bool value)
{
    isDragEnabled=value;
}


void SensorsActuatorsPlace::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->mimeData()->hasFormat("application/x-sensor-actuator-element"))
        event->accept();
    else
        event->ignore();
}

void SensorsActuatorsPlace::dragMoveEvent(QDragMoveEvent *event)
{
    if (event->mimeData()->hasFormat("application/x-sensor-actuator-element"))
    {
        event->setDropAction(Qt::MoveAction);
        event->accept();
    } else {
        event->ignore();
    }
}

void SensorsActuatorsPlace::dropEvent(QDropEvent *event)
{
    if (event->mimeData()->hasFormat("application/x-sensor-actuator-element"))
    {
        QByteArray pieceData = event->mimeData()->data("application/x-sensor-actuator-element");
        QDataStream dataStream(&pieceData, QIODevice::ReadOnly);

        QPixmap pixmap;
        QString var_name;
        dataStream >> pixmap >> var_name;

        emit addWidget(var_name,event->pos());

        event->setDropAction(Qt::MoveAction);
        event->accept();
    }
    else
        event->ignore();
}

void SensorsActuatorsPlace::mousePressEvent(QMouseEvent *event)
{
    if (isDragEnabled)
    {
        QWidget *itemClicked=childAt(event->pos());

        if (itemClicked!=0)
        {
            //emit resume();
            QPixmap pixmap(":/icons/mime_default.png");
            QString location=itemClicked->objectName();

            QByteArray itemData;
            QDataStream dataStream(&itemData, QIODevice::WriteOnly);

            dataStream << pixmap << location;

            QMimeData *mimeData = new QMimeData;
            mimeData->setData("application/x-sensor-actuator-element", itemData);

            QDrag *drag = new QDrag(this);
            drag->setMimeData(mimeData);
            drag->setHotSpot(QPoint(16,16));
            drag->setPixmap(pixmap.scaled(32,32));

            if (drag->exec(Qt::MoveAction) == Qt::MoveAction)
                delete itemClicked;
        }
    }
}

void SensorsActuatorsPlace::paintEvent(QPaintEvent *e)
{
    QPainter painter(this);
    painter.drawPixmap(0, 0, QPixmap(":/images/grosbot_2014.jpg").scaled(size()));
    QWidget::paintEvent(e);
}

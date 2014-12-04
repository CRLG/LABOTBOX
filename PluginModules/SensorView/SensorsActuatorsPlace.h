#ifndef SensorsActuatorsPlace_H
#define SensorsActuatorsPlace_H

#include <QList>
#include <QPoint>
#include <QPixmap>
#include <QWidget>
#include <QLabel>
#include <QDebug>

QT_BEGIN_NAMESPACE
class QDragEnterEvent;
class QDropEvent;
class QMouseEvent;
QT_END_NAMESPACE

class SensorsActuatorsPlace : public QWidget
{
    Q_OBJECT

public:
    explicit SensorsActuatorsPlace(QWidget *parent = 0);

protected:
    void dragEnterEvent(QDragEnterEvent *event);
    void dragMoveEvent(QDragMoveEvent *event);
    void dropEvent(QDropEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void paintEvent(QPaintEvent *e);

signals:
    void addWidget(QString var_Name,QPoint var_pos);
};

#endif // SensorsActuatorsPlace_H

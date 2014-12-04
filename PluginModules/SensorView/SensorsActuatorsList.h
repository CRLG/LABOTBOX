#ifndef SENSORSACTUATORSLIST_H
#define SENSORSACTUATORSLIST_H

#include <QListWidget>
#include <CEEPROM.h>

typedef enum {
    mime_sensor_tor,
    mime_sensor_ana,
    mime_computed_signal,
    mime_sensor_actuator
} eLBB_typeMime;

class SensorsActuatorsList : public QListWidget
{
    Q_OBJECT

public:
    explicit SensorsActuatorsList(QWidget *parent = 0);
    void addElement(QPixmap pixmap, QString name_var);

private:
    CEEPROM *m_eeprom_link;

protected:
    void dragEnterEvent(QDragEnterEvent *event);
    void dragMoveEvent(QDragMoveEvent *event);
    void dropEvent(QDropEvent *event);
    void startDrag(Qt::DropActions supportedActions);

    int m_type;
};

#endif // SENSORSACTUATORSLIST_H

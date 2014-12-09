#ifndef SENSORSACTUATORSLIST_H
#define SENSORSACTUATORSLIST_H

#include <QListWidget>
#include <CEEPROM.h>



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
signals:
    void refreshList(QString var_name);
    void resume(void);
    void start(void);
};

#endif // SENSORSACTUATORSLIST_H

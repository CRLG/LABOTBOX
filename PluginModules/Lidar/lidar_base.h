#ifndef _LIDAR_BASE_H_
#define _LIDAR_BASE_H_

#include <QObject>
#include "CEEPROM.h"
#include "lidar_data.h"

class LidarBase : public QObject
{
    Q_OBJECT
public:
    explicit LidarBase(QObject *parent = nullptr)
        : QObject{parent}
    { }
    virtual ~LidarBase() {}

    //! Démarre la gestion du Lidar
    virtual void start() { }

    //! Renvoie le widget de gestion du lidar à afficher
    virtual QWidget *get_widget() { return Q_NULLPTR; }

    //! Renvoie le nom du Lidar
    virtual QString get_name() { return "NoName"; }

    //! Lecture des paramètres de configuration du lidar dans la section dédiée du fichier EEPROM.ini
    virtual void read_settings(CEEPROM *eeprom, QString section_name)
    {
        Q_UNUSED(eeprom)
        Q_UNUSED(section_name)
    }

    //! sauvegarde des paramètres de configuration du lidar dans la section dédiée du fichier EEPROM.ini
    virtual void save_settings(CEEPROM *eeprom, QString section_name)
    {
        Q_UNUSED(eeprom)
        Q_UNUSED(section_name)
    }

signals:
    void new_data(const CLidarData& data);
    void connected();
    void disconnected();
    void error(QString msg);
};

#endif // _LIDAR_BASE_H_

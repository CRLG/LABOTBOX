#ifndef _YDLIDAR_TMINIPLUS_H_
#define _YDLIDAR_TMINIPLUS_H_

#include <QWidget>
#include <QSerialPort>
#include <QElapsedTimer>

#include "lidar_base.h"
#include "ydlidar_tminiplus_base.h"
#include "ui_ydlidar_tminiplus.h"


class YDLIDAR_TminiPlus : public LidarBase, public YDLIDAR_TminiPlusBase
{
    Q_OBJECT
public:
    explicit YDLIDAR_TminiPlus(QObject *parent = Q_NULLPTR);
    ~YDLIDAR_TminiPlus();

    // Réimpléméntation des méthodes de la classe de base LidarBase
    /*virtual*/ QWidget *get_widget() override;
    /*virtual*/ QString get_name() override;
    /*virtual*/ void start() override;
    /*virtual*/ void read_settings(CEEPROM *eeprom, QString section_name) override;
    /*virtual*/ void save_settings(CEEPROM *eeprom, QString section_name) override;


protected :
    // Réimpléméntation des méthodes de la classe de base YDLIDAR_TminiPlusBase
    virtual void new_packet() override;
    virtual void packet_error() override;
    virtual void new_cycle() override;
    /*virtual*/ bool write_serial(const char buff[], unsigned long len) override;
    /*virtual*/ bool read_serial(const char buff[], unsigned long len) override;

private :
    bool openSerialPort(QString portname);
    void closeSerialPort();

    Ui::ihm_ydlidar_tminiplus m_ihm;
    QWidget *m_widget;

    QSerialPort m_serial;

    CLidarData m_current_lidar_data;
    int m_current_index;

    // Horloge monotone datant chaque tour complet (CLidarData::m_timestamp, en usec)
    // Demarree a la construction du driver : l'origine est arbitraire, seule compte la
    // difference entre deux tours (cadence, vitesse d'un objet, recalage sur la tirette)
    QElapsedTimer m_horloge_tours;


private slots :
    void serial_data_received();


};

#endif // _YDLIDAR_TMINIPLUS_H_




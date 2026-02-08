#ifndef FILTER_PARAMS_H
#define FILTER_PARAMS_H

#include <QWidget>
#include "ui_lidar_filter_params.h"

class CLidarDataFilterModuleBase;
class CLidarFilterParams : public QWidget
{
    Q_OBJECT
public:
    explicit CLidarFilterParams(CLidarDataFilterModuleBase *filter, QString eeprom_pathfilename, QWidget *parent = nullptr);

    void fromFile(QString pathfilename);
    void toFile(QString pathfilename);

    Ui::ihm_lidar_filter_params m_ihm;

private :
    CLidarDataFilterModuleBase *m_filter;
    QString m_eeprom_pathfilename;

    QString getEEPROMFileSection();

    /*virtual*/void closeEvent(QCloseEvent *event);

signals:
    void closed();

private slots:
    void apply_value();
    void selected_param_change(QString param_name);
    void save();
};

#endif // FILTER_PARAMS_H

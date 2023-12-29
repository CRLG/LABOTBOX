#ifndef FILTER_PARAMS_H
#define FILTER_PARAMS_H

#include <QWidget>
#include "ui_lidar_filter_params.h"

class CLidarDataFilterBase;
class CLidarFilterParams : public QWidget
{
    Q_OBJECT
public:
    explicit CLidarFilterParams(CLidarDataFilterBase *filter, QWidget *parent = nullptr);

    Ui::ihm_lidar_filter_params m_ihm;

private :
    CLidarDataFilterBase *m_filter;

    /*virtual*/void closeEvent(QCloseEvent *event);

signals:
    void closed();

private slots:
    void apply_value();
    void selected_param_change(QString param_name);
};

#endif // FILTER_PARAMS_H

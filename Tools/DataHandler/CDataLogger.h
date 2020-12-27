/*! \file CDataLogger.h
 * A brief file description.
 * A more elaborated file description.
 */
#ifndef CDATA_LOGGER_H_
#define CDATA_LOGGER_H_

#include <QObject>
#include <QTimer>
#include <QElapsedTimer>
#include <QFile>
#include <QVector>

#include "CData.h"


 /*! \addtogroup DATA_LOGGER
   * 
   *  @{
   */

/*! @brief class CDataLogger
 */	   
class CDataLogger : public QObject
{
    Q_OBJECT

public:
    explicit CDataLogger(QObject *parent = Q_NULLPTR);
    virtual ~CDataLogger();

public slots :
    bool start(QString pathfilename);
    void stop();
    void pause();
    void continue_log();
    void activeDatation(bool on_off);
    void setRefreshPeriod(int period_ms);

    void setDataList(QVector<CData*> data_list);

private slots :
    void tick_log();

private :
    QVector<CData*> m_data_list;
    QTimer m_timer_tick;
    QElapsedTimer m_elapsed_timer;
    QFile m_logger_file;
    bool m_datation_active;
};

#endif // CDATA_LOGGER_H_

/*! @} */


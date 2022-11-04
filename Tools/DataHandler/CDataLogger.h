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

    typedef enum {
        LOGGER_OFF,
        LOGGER_READY_TO_START,
        LOGGER_ON,
        LOGGER_PAUSE,
        LOGGER_CONTINUE,
    }tLoggerStatus;

public slots :
    bool start(QString pathfilename, bool force_first_log=true);
    void stop();
    void pause();
    void continue_log();
    void log_once();
    void activeLogPeriodically(bool on_off);
    void activeLogOnDataChanged(bool on_off);
    void activeLogOnDataUpdated(bool on_off);
    void setRefreshPeriod(int period_ms);
    bool isLogging();

    void setDataList(QVector<CData*> data_list);

private slots :
    void _log_once();

    void connect_disconnect_changed_event(bool on_off);
    void connect_disconnect_updated_event(bool on_off);

private :
    QVector<CData*> m_data_list;
    QTimer m_timer_tick;
    QElapsedTimer m_elapsed_timer;
    QFile m_logger_file;
    bool m_enable_log_periodically;
    bool m_enable_log_on_data_changed;
    bool m_enable_log_on_data_updated;

    const QString CSV_SEPARATOR = ";";

signals :
    void status_changed(int status);
    void logger_started(QString pathfilename);  // indique l'enregistrement vient de commencer
    void logger_finished(QString pathfilename); // indique que l'enregistremen est terminé
};

#endif // CDATA_LOGGER_H_

/*! @} */


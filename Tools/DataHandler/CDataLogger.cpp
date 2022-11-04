/*! \file CDataLogger.cpp
 * A brief file description CPP.
 * A more elaborated file description.
 */
#include <QTextStream>
#include <QDebug>
#include "CDataLogger.h"

/*! \addtogroup DATA_LOGGER
   *
   *  @{
   */

// _____________________________________________________________________
CDataLogger::CDataLogger(QObject *parent)
    : QObject(parent),
      m_enable_log_periodically(true),
      m_enable_log_on_data_changed(false),
      m_enable_log_on_data_updated(false)
{
    setRefreshPeriod(1000);

    connect(&m_timer_tick, SIGNAL(timeout()), this, SLOT(_log_once()));
}

// _____________________________________________________________________
CDataLogger::~CDataLogger()
{
    stop();
}

// _____________________________________________________________________
bool CDataLogger::start(QString pathfilename, bool force_first_log)
{
    if(isLogging()) return false;

    // Pas la peine de commencer l'enregistrement si la liste est vide
    if (m_data_list.size() == 0) return false;

    // Ouvre et construit le fichier
    m_logger_file.setFileName(pathfilename);
    if (!m_logger_file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return false;
    }

    // Ecrit l'entête du fichier
    QTextStream out(&m_logger_file);
    // 1ere ligne du fichier = nom des colonnes
    out << "Timestamp [msec]";
    foreach(CData *data, m_data_list) {
        if (data) out << CSV_SEPARATOR << data->getName();
    }
    out << endl;

    m_elapsed_timer.restart();
    if (force_first_log) log_once(); // première enregistrement immédiatement pour avoir un état sans attendre le premier évènement

    if (m_enable_log_periodically)      m_timer_tick.start();
    if (m_enable_log_on_data_changed)   connect_disconnect_changed_event(true);
    if (m_enable_log_on_data_updated)   connect_disconnect_updated_event(true);

    emit status_changed(LOGGER_ON);
    emit logger_started(m_logger_file.fileName());

    return true;
}

// _____________________________________________________________________
void CDataLogger::stop()
{
    if (!isLogging()) return;

    if (m_timer_tick.isActive()) m_timer_tick.stop();
    if (m_enable_log_on_data_changed)   connect_disconnect_changed_event(false);
    if (m_enable_log_on_data_updated)   connect_disconnect_updated_event(false);

    if (m_logger_file.isOpen()) m_logger_file.close();

    emit status_changed(LOGGER_OFF);
    emit logger_finished(m_logger_file.fileName());
}

// _____________________________________________________________________
void CDataLogger::pause()
{
    if (!isLogging()) return;

    if (m_timer_tick.isActive()) m_timer_tick.stop();
    if (m_enable_log_on_data_changed)   connect_disconnect_changed_event(false);
    if (m_enable_log_on_data_updated)   connect_disconnect_updated_event(false);
    emit status_changed(LOGGER_PAUSE);
}

// _____________________________________________________________________
void CDataLogger::continue_log()
{
    if (!isLogging()) return;

    if (m_enable_log_periodically)      m_timer_tick.start();
    if (m_enable_log_on_data_changed)   connect_disconnect_changed_event(true);
    if (m_enable_log_on_data_updated)   connect_disconnect_updated_event(true);
    emit status_changed(LOGGER_CONTINUE);
}

// _____________________________________________________________________
void CDataLogger::log_once()
{
    if (isLogging()) _log_once();
}

// _____________________________________________________________________
void CDataLogger::setRefreshPeriod(int period_ms)
{
    m_timer_tick.setInterval(period_ms);
}

// _____________________________________________________________________
bool CDataLogger::isLogging()
{
    return m_logger_file.isOpen();
}

// _____________________________________________________________________
void CDataLogger::activeLogPeriodically(bool on_off)
{
    m_enable_log_periodically = on_off;
}

// _____________________________________________________________________
void CDataLogger::activeLogOnDataChanged(bool on_off)
{
    m_enable_log_on_data_changed = on_off;
}

// _____________________________________________________________________
void CDataLogger::activeLogOnDataUpdated(bool on_off)
{
    m_enable_log_on_data_updated = on_off;
}

// _____________________________________________________________________
void CDataLogger::setDataList(QVector<CData *> data_list)
{
    m_data_list = data_list;
}

// _____________________________________________________________________
void CDataLogger::_log_once()
{
    QTextStream out(&m_logger_file);

    out << m_elapsed_timer.elapsed();

    foreach(CData *data, m_data_list) {
        if (data) out << CSV_SEPARATOR << data->read().toString();
    }
    out << endl;
}

// _____________________________________________________________________
void CDataLogger::connect_disconnect_changed_event(bool on_off)
{
    foreach(CData *data, m_data_list) {
        if (data) {
            if (on_off) connect(data, SIGNAL(valueChanged(QVariant)), this, SLOT(_log_once()));
            else        disconnect(data, SIGNAL(valueChanged(QVariant)), this, SLOT(_log_once()));
        }
    }
}

// _____________________________________________________________________
void CDataLogger::connect_disconnect_updated_event(bool on_off)
{
    foreach(CData *data, m_data_list) {
        if (data) {
            if (on_off) connect(data, SIGNAL(valueUpdated(QVariant)), this, SLOT(_log_once()));
            else        disconnect(data, SIGNAL(valueUpdated(QVariant)), this, SLOT(_log_once()));
        }
    }
}

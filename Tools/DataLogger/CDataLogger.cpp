/*! \file CDataLogger.cpp
 * A brief file description CPP.
 * A more elaborated file description.
 */
#include <QTextStream>
#include "CDataLogger.h"

/*! \addtogroup DATA_LOGGER
   *
   *  @{
   */

// _____________________________________________________________________
CDataLogger::CDataLogger(QObject *parent)
    : QObject(parent)
{
    m_datation_active = false;
    setRefreshPeriod(1000);

    connect(&m_timer_tick, SIGNAL(timeout()), this, SLOT(tick_log()));
}

// _____________________________________________________________________
CDataLogger::~CDataLogger()
{
    stop();
}

// _____________________________________________________________________
bool CDataLogger::start(QString pathfilename)
{
    if(m_logger_file.isOpen()) return false;

    // Ouvre et construit le fichier
    m_logger_file.setFileName(pathfilename);
    if (!m_logger_file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return false;
    }

    // Ecrit l'entête du fichier
    QTextStream out(&m_logger_file);
    if (m_datation_active) {
        // 1ere ligne du fichier = nom des colonnes
        out << "time;";
    }
    for (int i=0; i<m_data_list.size(); i++)
    {
        CData *data = m_data_list.at(i);
        if (data) {
            out << data->getName();
            if (i != (m_data_list.size()-1))
                out << ";";
        }
    }
    out << "\n";

    m_elapsed_timer.restart();
    m_timer_tick.start();
    return true;
}

// _____________________________________________________________________
void CDataLogger::stop()
{
    if (m_timer_tick.isActive()) m_timer_tick.stop();
    if (m_logger_file.isOpen()) m_logger_file.close();
}

// _____________________________________________________________________
void CDataLogger::pause()
{
    m_timer_tick.stop();
}

// _____________________________________________________________________
void CDataLogger::continue_log()
{
    if (m_logger_file.isOpen()) {
        m_timer_tick.start();
    }
}

// _____________________________________________________________________
void CDataLogger::setRefreshPeriod(int period_ms)
{
    m_timer_tick.setInterval(period_ms);
}

// _____________________________________________________________________
void CDataLogger::activeDatation(bool on_off)
{
    m_datation_active = on_off;
}

// _____________________________________________________________________
void CDataLogger::setDataList(QVector<CData *> data_list)
{
    m_data_list = data_list;
}

// _____________________________________________________________________
void CDataLogger::tick_log()
{
    QTextStream out(&m_logger_file);
    if (m_datation_active) {
        out << m_elapsed_timer.elapsed();
        out << ";";
    }
    for (int i=0; i<m_data_list.size(); i++)
    {
        CData *data = m_data_list.at(i);
        if (data) {
            out << data->read().toString();
            if (i != (m_data_list.size()-1))
                out << ";";
        }
    }
    out << "\n";
}

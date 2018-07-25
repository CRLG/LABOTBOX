/*! \file CSignalGenerator.h
 * A brief file description.
 * A more elaborated file description.
 */
#ifndef _CSignalGenerator_H_
#define _CSignalGenerator_H_

#include <QMainWindow>
#include <QThread>
#include <QVector>
#include <QVariant>

 /*! \addtogroup DataPlayer
   * 
   *  @{
   */


class CDataManager;

// Structure représentant un échantillon de valeur
typedef struct {
    QVariant    value;
    int         duration;    // [msec]. Valeur -1 spécifique pour indiquer que la durée de l'échantillon est la durée commune de tout le module
}tSample;

typedef QVector<tSample>tSamples;


/*! @brief class CSignalGenerator
 */	   
class CSignalGenerator : public QThread
{
    Q_OBJECT

#define C_PERIODIC_SIGNAL   (-1)
#define C_COMMON_DURATION   (-1)


public:
    CSignalGenerator(CDataManager *data_manager, QString var_name);
    ~CSignalGenerator();

    // Accesseurs
    void setCommonSampleDuration(unsigned long duration_msec)   { m_common_samples_duration = duration_msec; }
    void setCycleNumber(int cycle_number)                       { m_cycle_number = cycle_number; }
    void setSignalFilename(QString signal_filename);
    unsigned long getCommonSampleDuration(void)     { return(m_common_samples_duration); }
    int getCycleNumber(void)                        { return(m_cycle_number); }
    QString getSignalFilename(void)                 { return(m_signal_filename); }
    bool isGeneratorActive(void)                    { return(m_generator_in_progress); }


private :
    CDataManager        *m_data_manager;
    QString             m_var_name;
    QString             m_signal_filename;
    unsigned long       m_common_samples_duration;
    int                 m_cycle_number;  // Nombre de période à effectuer. Valeur spéciale C_PERIODIC_SIGNAL pour indiquer que le signal doit tournner en boucle (periodique)
    int                 m_cycle_count;   // Comptabilise le nombre de période effectuées
    tSamples            m_samples;
    bool                m_generator_in_progress;

    // Hériatage de QThread
    virtual void run(void);
signals :
    void statusChanged(unsigned long status);
    void profilenameChanged(QString profilename);
    void signalFinished(QString var_name);
    void signalStarted(QString var_name);

public slots:
    void start_generator(void);
    void stop_generator(void);
    void activeInhibeGenerator(bool state);
};

#endif // _CSignalGenerator_H_

/*! @} */


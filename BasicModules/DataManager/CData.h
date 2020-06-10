/*! \file CData.h
 * A brief file description header.
 * A more elaborated file description.
 */

#ifndef _CDATA_H_
#define _CDATA_H_

#include <QObject>
#include <QVariant>
#include <QMutex>
#include <QHash>
#include <QDateTime>
#include <QTimer>

/*! \addtogroup DataManager
   *  Additional documentation for group DataManager
   *  @{
   */

typedef QHash<QString, QVariant>t_map_properties;

/*! @brief class CData in @link DataManager.
 */
class CData : public QObject
{
    Q_OBJECT

public:
    CData();
    CData(QString name, QVariant val);
    ~CData();

    QVariant read(void);
    QString getName(void) { return(m_name); }
    quint64 getTime() { return m_update_time; }

    // Accesseurs sur les propriétés
    void setProperty(QString name, QVariant value);
    QVariant getProperty(QString name);
    void getPropertiesList(QStringList &list);
    QString getPropertiesString(void);
    void startMonotoring(unsigned long timeout_msec, QVariant valdef);
    void stopMonitoring();
    bool isLost();
    bool isMonitored();

private:
    //! Stockage de la valeur de la data
    QVariant m_data;
    //! Instant de la dernière mse à à jour (temps absolue en msec)
    qint64 m_update_time;
    //! Nom de la data
    QString m_name;
    //! Mutex pour l'accès concurrentiel à la data
    QMutex m_mutex;
    //! Liste d'options associées à la data (QString = nom de la propriété / QVariant = valeur de la propriété)
    t_map_properties m_properties;

    // Monitoring : surveillance de permet de mise à jour
    //! Est-ce que la donnée doit être surveillée (absence de mise à jour)
    bool m_monitored;
    //! Valeur par défaut
    QVariant m_data_valdef;
    //! Durée sans rafraichissement avant de déclarer la donnée perdue
    unsigned long m_lost_time_confirmation;
    //! Timer de gestion de la perte de la data
    QTimer m_lost_timer;
    //! Flag de data perdue
    bool m_lost;

signals:
    //! Signal emis par la donnee lorsqu'une ecriture est faite et que la valeur a changé
    void valueChanged(QVariant value);
    void valueChanged(QVariant value, quint64 update_time);
    //! Signal emis par la donnee lorsqu'une ecriture est faite et que la valeur a changé
    void valueChanged(bool value);
    //! Signal emis par la donnee lorsqu'une ecriture est faite et que la valeur a changé
    void valueChanged(int value);
    //! Signal emis par la donnee lorsqu'une ecriture est faite et que la valeur a changé
    void valueChanged(double value);
    //! Signal emis par la donnee lorsqu'une ecriture est faite et que la valeur a changé
    void valueChanged(QString value);
    //! Signal emis par la donnee lorsqu'une ecriture est faite sans forcément modification de la valeur
    void valueUpdated(QVariant value);
    void valueUpdated(QVariant value, quint64 update_time);
    //! Signal emis par la donnee lorque la perte de communication est confirmee
    void lost();

public slots :
    void write(QVariant data);
    void write(bool value);
    void write(int value);
    void write(double value);
    void write(QString value);

private slots :
    void lost_timeout();
};


#endif // _CDATA_H_

/*! @} */


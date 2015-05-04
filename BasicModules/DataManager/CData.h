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

    void write(QVariant data);
    QVariant read(void);
    QString getName(void) { return(m_name); }

    // Accesseurs sur les propriétés
    void setProperty(QString name, QVariant value);
    QVariant getProperty(QString name);
    void getPropertiesList(QStringList &list);
    QString getPropertiesString(void);

private:
	//! Stockage de la valeur de la data
	QVariant m_data;
    //! Nom de la data
    QString m_name;
    //! Mutex pour l'accès concurrentiel à la data
    QMutex m_mutex;
    //! Liste d'options associées à la data (QString = nom de la propriété / QVariant = valeur de la propriété)
    t_map_properties m_properties;

signals:
    //! Signal emis par la donnee lorsqu'une ecriture est faite
    void valueChanged(QVariant value);
    //! Signal emis par la donnee lorsqu'une ecriture est faite
    void valueChanged(bool value);
    //! Signal emis par la donnee lorsqu'une ecriture est faite
    void valueChanged(int value);
    //! Signal emis par la donnee lorsqu'une ecriture est faite
    void valueChanged(double value);
    //! Signal emis par la donnee lorsqu'une ecriture est faite
    void valueChanged(QString value);
};


#endif // _CDATA_H_

/*! @} */


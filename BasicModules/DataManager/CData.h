/*! \file CData.h
 * A brief file description header.
 * A more elaborated file description.
 */

#ifndef _CDATA_H_
#define _CDATA_H_

#include <QObject>
#include <QVariant>
#include <QMutex>

/*! \addtogroup DataManager
   *  Additional documentation for group DataManager
   *  @{
   */

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

private:
	//! Stockage de la valeur de la data
	QVariant m_data;
    //! Nom de la data
    QString m_name;
    //! Mutex pour l'accès concurrentiel à la data
    QMutex m_mutex;

signals:
    //! Signal emis par la donnee lorsqu'une ecriture est faite
    void valueChanged(QVariant value);
};


#endif // _CDATA_H_

/*! @} */


/*! \file CDataManager.h
 * A brief file description.
 * A more elaborated file description.
 */
#ifndef _CDATA_MANAGER_H_
#define _CDATA_MANAGER_H_

#include <QHash>
#include "CData.h"
#include "CBasicModule.h"

 /*! \addtogroup DataManager
   * 
   *  @{
   */

	   
/*! @brief class CDataManager in @link DataManager basic module.
 */	   
class CDataManager : public CBasicModule
{
    Q_OBJECT
#define     VERSION_DATA_MANAGER         "1.1"
#define     AUTEUR_DATA_MANAGER          "Nico"
#define     INFO_DATA_MANAGER            "Gestionnaire de données"

typedef QHash<QString, CData *> t_map_data;

public:
    CDataManager(const char *plugin_name);
    ~CDataManager();

    virtual void init(CApplication *application);
    virtual void close(void);
    virtual bool hasGUI(void) { return(false); }
    virtual QIcon getIcon(void) { return(QIcon("")); }


    void            write(QString varname, QVariant val);
    QVariant        read(QString varname);
    CData           *createData(QString varname, QVariant init_val=QVariant());
    CData           *getData(QString varname, bool force_creation=false);
    void            debug(void);
    QString         getDataValues(void);
    void            getDataList(QVector<CData *> &data_list);
    bool            isExist(QString varname);
    void            remove(QString varname);
    unsigned long   size(void);
    void            getListeVariablesName(QStringList &var_list);
    void            setDataProperty(QString varname, QString prop_name, QVariant val);
    QVariant        getDataProperty(QString varname, QString prop_name);
    void            startMonitoring(QString varname, unsigned long timeout_msec, QVariant valdef);

private:
    t_map_data m_map_data;
    //! Mutex pour l'accès concurrentiel au map
    QMutex m_mutex;

private :
    void loadDataProperties(void);
    void saveDataProperties(void);

signals:
    void valueChanged(CData *data);
    void valueUpdated(CData *data);
    void dataCreated(CData *data);

};

#endif // _CDATA_MANAGER_H_

/*! @} */


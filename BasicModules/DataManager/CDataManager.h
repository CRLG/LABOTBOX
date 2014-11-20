/*! \file CDataManager.h
 * A brief file description.
 * A more elaborated file description.
 */
#ifndef _CDATA_MANAGER_H_
#define _CDATA_MANAGER_H_

#include <QMap>
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
#define     VERSION_DATA_MANAGER         "1.0"
#define     AUTEUR_DATA_MANAGER          "Nico"
#define     INFO_DATA_MANAGER            "Gestionnaire de données"

typedef QMap<QString, CData *> t_map_data;

public:
    CDataManager(const char *plugin_name);
    ~CDataManager();

    virtual bool hasGUI(void) { return(false); }
    virtual QIcon getIcon(void) { return(QIcon("")); }


    void            write(QString str, QVariant val);
    QVariant        read(QString varname);
    CData           *getData(QString varname);
    void            debug(void);
    QString         getDataValues(void);
    bool            isExist(QString varname);
    void            remove(QString varname);
    unsigned long   size(void);
    void            getListeVariablesName(QStringList &var_list);

private:
    t_map_data m_map_data;
    //! Mutex pour l'accÃ¨s concurrentiel au map
    QMutex m_mutex;


signals:
    void valueChanged(CData *data);
};

#endif // _CDATA_MANAGER_H_

/*! @} */


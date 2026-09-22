// Banc logique robot : DataManager minimal.
//
// Le plugin librobotlogic_*.so appelle, dans le shell Simulia, exactement quatre methodes non
// virtuelles de CDataManager : write, read, getData et isExist (verifie par
// "nm -D --undefined-only"). Ce fichier en fournit une implementation autonome, de meme nom et de
// meme signature, pour charger la logique robot hors de Simulia sans embarquer toute la mecanique
// des modules du shell (CBasicModule, IHM...). Les objets CData, eux, sont les vrais : le plugin
// accede a certains de leurs membres en ligne (getName), leur disposition doit donc etre exacte.
//
// Semantique reprise de BasicModules/DataManager/CDataManager.cpp : creation a la premiere ecriture,
// getData sans creation par defaut.
#ifndef _BANC_DATA_MANAGER_H_
#define _BANC_DATA_MANAGER_H_

#include <QHash>
#include <QString>
#include <QVariant>
#include "CData.h"

class CDataManager
{
public:
    CDataManager() {}
    ~CDataManager();

    void     write(QString varname, QVariant val);
    QVariant read(QString varname);
    CData   *getData(QString varname, bool force_creation=false);
    bool     isExist(QString varname);

private:
    QHash<QString, CData *> m_map_data;
};

#endif // _BANC_DATA_MANAGER_H_

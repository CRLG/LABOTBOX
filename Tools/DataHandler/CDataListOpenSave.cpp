/*! \file CDataListOpenSave.cpp
 * A brief file description CPP.
 * A more elaborated file description.
 */
#include <QFile>
#include <QTextStream>
#include "CDataListOpenSave.h"

const QString LINE_SEPARATOR = "\n";

/*! \addtogroup DATA_LIST
   *
   *  @{
   */

// _____________________________________________________________________
CDataListOpenSave::CDataListOpenSave()
{
}

// _____________________________________________________________________
CDataListOpenSave::~CDataListOpenSave()
{
}

// _____________________________________________________________________
// Un fichier liste de données est un simple fichier
// une ligne par donnée
bool CDataListOpenSave::open(QString pathfilename, QStringList &out_data_lst)
{
    QFile file;
    file.setFileName(pathfilename);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return false;
    }

    QTextStream in(&file);
    QString line = in.readLine();
    while (!line.isNull()) {
        out_data_lst.append(line.simplified());
        line = in.readLine();
    }
    return true;
}


// _____________________________________________________________________
bool CDataListOpenSave::save(QString pathfilename, QStringList& in_data_lst)
{
    QFile file;
    file.setFileName(pathfilename);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return false;
    }
    QTextStream out(&file);
    foreach(QString data, in_data_lst) {
        out << data << LINE_SEPARATOR;
    }
    file.close();
    return true;
}



#ifndef _CSVDATA_H_
#define _CSVDATA_H_

#include <QStringList>
#include <QVector>

class csvData
{
public:
    csvData();
    ~csvData();

    QStringList m_header;           //! Description of each column
    QVector<QStringList> m_datas;   //! Datas : vector of columns. Each cell is QString type

    unsigned long getColumnsCount();
    unsigned long getLinesCount();
    void debug();
};

#endif // _CSVDATA_H_

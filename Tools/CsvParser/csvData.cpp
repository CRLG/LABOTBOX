#include <QDebug>

#include "csvData.h"

csvData::csvData()
{
}

csvData::~csvData()
{
}


// ______________________________________________
unsigned long csvData::getColumnsCount()
{
    unsigned long columns_count = 0;
    if (m_header.size() > 0) columns_count = m_header.size();
    else if (m_datas.size() > 0) columns_count = m_datas.at(0).size();
    return columns_count;
}


// ______________________________________________
unsigned long csvData::getLinesCount()
{
    return m_datas.size();
}

// ______________________________________________
void csvData::debug()
{
    qDebug() << "- HEADER - ";
    qDebug() << "   " << m_header;
    qDebug() << "- DATAS - ";
    for (long i=0; i<m_datas.size(); i++) {
        qDebug() << "   [" << i+1 << "]" << m_datas.at(i);
    }

    if (m_header.size() != 0)
    qDebug() << "Number of Columns: " << getColumnsCount();
    qDebug() << "Number of lines: " << getLinesCount();
}

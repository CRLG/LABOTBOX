#ifndef _TEST_H_
#define _TEST_H_

#include <QtTest/QtTest>
#include <QVariant>
#include "CData.h"

// =======================================================
//      CLASSE PRINCIPALE
// =======================================================
class Test: public QObject
{
    Q_OBJECT
public :
  int initTestCase();

private slots:
    // CData
    void test_read_write(void);
    void test_connect_write(void);
    // CDataManager
    //void test_CDataManager_general(void);
    //void test_CDataManager_nombreuses_variables(void);
    //void test_CDataManager_BasicModule(void);
};



// =======================================================
//      CLASSE DE TEST DE CONNECTION
// =======================================================
class CTest_connect: public QObject
{
    Q_OBJECT

public :
    CTest_connect();

    CData           m_data;
    QVariant        m_data_variant;
    unsigned long   m_nbre_appel_slot;

private slots :
  void test_slot_data_changed(QVariant data);
};

#endif // _TEST_H_

/*! \file CStrategyDesigner.h
 * A brief file description.
 * A more elaborated file description.
 */
#ifndef _CBASIC_MODULE_StrategyDesigner_H_
#define _CBASIC_MODULE_StrategyDesigner_H_

#include <QMainWindow>

#include "CPluginModule.h"
#include "ui_ihm_StrategyDesigner.h"

#include <QLineEdit>
#include <QTableWidget>
#include <QComboBox>
#include <QPushButton>
#include <QDebug>
#include <QFile>
#include <QDate>
#include <QTime>
#include <QString>
#include "CData.h"
#include <QtTest/QTest>
#include <QtMath>
#include <QtXml/QDomDocument>
#include <QFileDialog>

 class Cihm_StrategyDesigner : public QMainWindow
{
    Q_OBJECT
public:
    Cihm_StrategyDesigner(QWidget *parent = 0)  : QMainWindow(parent) { ui.setupUi(this); }
    ~Cihm_StrategyDesigner() { }

    Ui::ihm_StrategyDesigner ui;

    CLaBotBox *m_application;
 };



 /*! \addtogroup StrategyDesigner
   * 
   *  @{
   */

enum EDeplacements
    {
        X_Y=1,
        X_Y_Teta,
        Distance,
        Distance_Angle,
        Angle,
        boucleOuverte
    };

	   
/*! @brief class CStrategyDesigner in @link TraceLogger basic module.
 */	   
class CStrategyDesigner : public CPluginModule
{
    Q_OBJECT
#define     VERSION_StrategyDesigner   "1.0"
#define     AUTEUR_StrategyDesigner    "Steph2"
#define     INFO_StrategyDesigner      "Description du module de Steph no2"

public:
    CStrategyDesigner(const char *plugin_name);
    ~CStrategyDesigner();

    Cihm_StrategyDesigner *getIHM(void) { return(&m_ihm); }

    virtual void init(CLaBotBox *application);
    virtual void close(void);
    virtual bool hasGUI(void) { return(true); }
    virtual QIcon getIcon(void) { return(QIcon(":/icons/edit_add.png")); }

private:
    Cihm_StrategyDesigner m_ihm;
    QLineEdit *lineEdit_X;
    QLineEdit *lineEdit_Y;
    QLineEdit *lineEdit_Theta;
    QLineEdit *lineEdit_Distance;
    QLineEdit *lineEdit_Angle;
    QTableWidget *tableWidget_SM;
    bool blistFirstFilled;
    bool bPlay;
    bool bResume;
    bool bStop;
    QStringList servoName;
    QStringList servoValue;
    QStringList moteurName;
    QStringList moteurValue;

public slots :
//    void Slot_X(QVariant val);
//    void Slot_Y(QVariant val);
//    void Slot_Theta(QVariant val);
    void Slot_CoordChanged(CData* data);
    void Slot_AddDeplacement();
    void Slot_AddAction();
    void Slot_AddTransition();
    void Slot_GenerateMFile();
    void cellEnteredSlot(int row, int column);
    void Slot_Play(void);
    void Slot_Resume(void);
    void Slot_Stop(void);
    void Slot_Save(void);
    void Slot_Load(void);
    void Slot_Remove(void);
    void Slot_ChoseAction(QString actionChoisie);


public:
    CData m_data1;
};

#endif // _CBASIC_MODULE_StrategyDesigner_H_

/*! @} */


﻿/*! \file CSimuBot.h
 * A brief file description.
 * A more elaborated file description.
 */
#ifndef _CBASIC_MODULE_SimuBot_H_
#define _CBASIC_MODULE_SimuBot_H_

#include <QMainWindow>

#include "CPluginModule.h"
#include "ui_ihm_SimuBot.h"

//ajouté par Steph
#include <QGraphicsView>
#include "graphicElement.h"
#include "graphicEnvironnement.h"
#include <QDebug>
#include <QtMath>
#include "CData.h"

 class Cihm_SimuBot : public QMainWindow
{
    Q_OBJECT
public:
    Cihm_SimuBot(QWidget *parent = 0)  : QMainWindow(parent) { ui.setupUi(this); }
    ~Cihm_SimuBot() { }

    Ui::ihm_SimuBot ui;

    CLaBotBox *m_application;

    //ajouté par Steph
    QGraphicsView *simuView;
 };



 /*! \addtogroup SimuBot
   * 
   *  @{
   */

	   
/*! @brief class CSimuBot in @link TraceLogger basic module.
 */	   
class CSimuBot : public CPluginModule
{
    Q_OBJECT
#define     VERSION_SimuBot   "1.0"
#define     AUTEUR_SimuBot    "Steph1"
#define     INFO_SimuBot      "Description du module de Steph no1"

public:
    CSimuBot(const char *plugin_name);
    ~CSimuBot();

    Cihm_SimuBot *getIHM(void) { return(&m_ihm); }

    virtual void init(CLaBotBox *application);
    virtual void close(void);
    virtual bool hasGUI(void) { return(true); }
    virtual QIcon getIcon(void) { return(QIcon(":/icons/edit_add.png")); }

private:
    Cihm_SimuBot m_ihm;

    //ajouté par Steph
    GraphicElement *GrosBot;
    GraphicElement *OldGrosBot;
    QGraphicsLineItem *liaison_GrosBot;
    float deltaAngle;
    float deltaDistance;
    GraphicEnvironnement *terrain;
    int modeVisu;
    //bool isRelativToBot;
    bool setAndGetInRad;

signals:
    void displayCoord(qreal value_x,qreal value_y);
    void displayAngle(qreal value_theta);
public slots:
    void viewChanged(QList<QRectF> regions);
    void initView(void);
    void changeEquipe(float X_init_1,float Y_init_1, float Theta_init_1,float X_init_2,float Y_init_2, float Theta_init_2);
    void returnCapture_XY();
    void returnCapture_Theta();
    void changeMode(int iMode);
    void coordChanged(CData* data);
};

#endif // _CBASIC_MODULE_SimuBot_H_

/*! @} */


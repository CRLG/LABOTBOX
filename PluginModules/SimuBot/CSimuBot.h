/*! \file CSimuBot.h
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
#include <QTimer>
#include "CData.h"

enum SIMUBOT{
    TEST=0,
    VISU,
    SIMU
};

enum EQUIPE{
    EQUIPE1=0,
    EQUIPE2
};

enum SECTEURS{
    AVG=0,
    AVD,
    ARG,
    ARD
};

 class Cihm_SimuBot : public QMainWindow
{
    Q_OBJECT
public:
    Cihm_SimuBot(QWidget *parent = 0)  : QMainWindow(parent) { ui.setupUi(this); }
    ~Cihm_SimuBot() { }

    Ui::ihm_SimuBot ui;

    CApplication *m_application;

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

    virtual void init(CApplication *application);
    virtual void close(void);
    virtual bool hasGUI(void) { return(true); }
    virtual QIcon getIcon(void) { return(QIcon(":/icons/edit_add.png")); }

private slots :
    void onRightClicGUI(QPoint pos);

    void playOther();
        void stopOther();
        void nextStepOther();
        void enableMoveOther(int state);
        void syncMove(bool activated);
        void enableTwoBots(int state);
private:
    Cihm_SimuBot m_ihm;

    //ajouté par Steph
    GraphicElement *GrosBot;
    GraphicElement *OldGrosBot;
    QGraphicsLineItem *liaison_GrosBot;
    GraphicElement *MiniBot;
	GraphicElement *OtherBot;
    QList<QGraphicsLineItem*> evitement;
    float deltaAngle;
    float deltaDistance;
    GraphicEnvironnement *terrain;
    int modeVisu;
    //bool isRelativToBot;
    bool setAndGetInRad;

    void initEquipe(int equipe);

    Coord equipe1_bot1;
    Coord equipe2_bot1;
    Coord equipe1_bot2;
    Coord equipe2_bot2;
    Coord equipeOther;
    bool twoBotsEnabled;
    float iniTetaAsserv_bot1[2];
    float iniTetaAsserv_bot2[2];

    //pour la stratégie du robot adverse
    int currentIndex;
    int convergenceOther;
    QTimer * cadenceur;
    bool isStarted;
    bool isStarted_old;


    void addStepOther(double x, double y, double teta, int row);
    QPolygonF getForm(QStringList strL_Form);
    void getUSDistance(Coord bot, Coord obstacle, float capteurs[]);
signals:
    void displayCoord(qreal value_x,qreal value_y);
    void displayAngle(qreal value_theta);
    void displayCoord2(qreal value_x,qreal value_y);
    void displayAngle2(qreal value_theta);
    void setSequence();
public slots:
    void viewChanged(QList<QRectF> regions);
    void initView(void);
    void changeEquipe(void);
    void returnCapture_XY();
    void returnCapture_Theta();
    void changeMode(int iMode);
    void coordChanged(CData* data);
    void zoom(int value);
    void slot_dial_turned(void);
    void slot_getPath();
    void estimate_Environment_Interactions();
    void real_robot_position_changed();
    void catchDoubleClick();
};

#endif // _CBASIC_MODULE_SimuBot_H_

/*! @} */


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
#include "CExternalControlerClient.h"
#include <QGraphicsRectItem>
#include <QGraphicsEllipseItem>
#include "CPhysicalEngine.h"
#include "CKinematicEngine.h"

//#define DEBUG_OTHER
//#define DEBUG_SIMULIA


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
    bool setAndGetInRad;

private slots :
    void onRightClicGUI(QPoint pos);

    void playOther();
        void stopOther();
        void nextStepOther();
        void enableMoveOther(int state);
        void syncMove(bool activated);
        void enableTwoBots(int state);
        void on_active_external_robot2(bool state);
        //void on_timeout_external_robot2();
        void updateStepFromSimulia();
        void box2d_enable(bool flag);
        void slot_clearPath();

        void updateStepFromSimuBot();

private:
    Cihm_SimuBot m_ihm;

    //ajouté par Steph
    GraphicElement *GrosBot;
    GraphicElement *OldGrosBot;
    QGraphicsLineItem *liaison_GrosBot;
    GraphicElement *MiniBot;
	GraphicElement *OtherBot;
    QList<QGraphicsLineItem*> evitement;
    QGraphicsRectItem * elementsJeu[48];
    float deltaAngle;
    float deltaDistance;
    GraphicEnvironnement *terrain;
    int modeVisu;
    //bool isRelativToBot;


    void initEquipe(int equipe);

    Coord equipe1_bot1;
    Coord equipe2_bot1;
    Coord equipe1_bot2;
    Coord equipe2_bot2;
    Coord equipeOther;
    bool twoBotsEnabled;
    float iniTetaAsserv_bot1[2];
    float iniTetaAsserv_bot2[2];

    //pour gérer simulia ou actuatorsequencer
    CPhysicalEngine m_physical_engine;
    CKinematicEngine m_kinematic_engine; // moteur cinematique analytique (etape 2 migration v2)
    bool m_use_kinematic;                // true => chemin cinematique, false => box2d (defaut)
    QString m_engine_choice;             // valeur EEPROM "engine" : "box2d" | "kinematic"
    // Offset d'init du robot : x_pos/y_pos publies dans le DataManager sont RELATIFS au
    // point de depart (comme Box2D : _x1(x)=x-m_x_init1). Le moteur cinematique travaille
    // en terrain absolu ; on soustrait cet offset au moment d'ecrire x_pos/y_pos.
    float m_kin_x_init;
    float m_kin_y_init;
    // Gain de conversion consigne -> vitesse roue (cm/s) pour le moteur cinematique.
    // Simulia publie vect_G/D = 80 * commande_moteur_% (valeur d'impulsion calibree pour
    // Box2D, cf. CRoues_simu.cpp), PAS une vitesse. On convertit : vitesse_cm_s = vect * gain.
    // Calibrable via EEPROM "kin_speed_gain" (defaut 0.01 -> ~80 cm/s a pleine commande).
    float m_kin_speed_gain;
    // Constante de temps moteur (s) du moteur cinematique : inertie premier ordre sur la
    // vitesse roue (asserv en vitesse). Calibrable via EEPROM "kin_motor_tau" (defaut 0.15).
    float m_kin_motor_tau;
    bool m_box2d_Enabled; //TODO probablement inutile (à nettoyer
    int m_step;
    bool m_simulia_Enabled;

    //pour la stratégie du robot adverse
    int currentIndex;
    int convergenceOther;
    QTimer * cadenceur;
    bool isStarted;
    bool isStarted_old;

    //pour l'intégration de la stratégie du 2eme robot
    CExternalControlerClient m_external_controler_client_robot2;
    QTimer m_timer_external_robot2;
    bool m_connected_host;
    int m_step2;

    void addStepOther(double x, double y, double teta, int row);
    QPolygonF getForm(QStringList strL_Form);
    void getUSDistance(Coord bot, Coord obstacle, float capteurs[], float lidar[]);
    QGraphicsRectItem *setElementJeu(float x, float y, int Color, bool vertical);
    //design robot
    QGraphicsScene * scene_design;

    //pour le design
    QGraphicsEllipseItem *points_design[5][8];
    QGraphicsLineItem *lignes_design[5][8];
    void initDesign();
signals:
    void displayCoord(qreal value_x,qreal value_y);
    void displayAngle(qreal value_theta);
    void displayCoord2(qreal value_x,qreal value_y);
    void displayAngle2(qreal value_theta);
    void setSequence(double angle, double distance);
public slots:
    void viewChanged(QList<QRectF> regions);
    void initView(void);
    void changeEquipe(void);
    void returnCapture_XY();
    void returnCapture_Theta();
    void changeMode(int iMode);
    void zoom(int value);
    void slot_dial_turned(void);
    void slot_getPath();
    void estimate_Environment_Interactions();
    void real_robot_position_changed();
    void catchDoubleClick();
    void Slot_catch_TxSync();
    void slot_designChanged(QList<QRectF> regions);
    void slot_enableSimulia(int state);
};

#endif // _CBASIC_MODULE_SimuBot_H_

/*! @} */


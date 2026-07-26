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
#include <QGraphicsPolygonItem>
#include <QFileSystemWatcher>
#include <QVector>
#include "CKinematicEngine.h"
#include "CCollisionEngine.h"
#include "CRoseDesVents.h"
// L'include de CAsservissementBase.h (reconstructeur de pose "l'asserv fait foi") est place plus
// bas, sous #ifdef SIMUBOT_ROBOT_LOGIC : il tire CGlobale.h -> toute la logique robot, disponible
// seulement dans le build Simulia. Le build LaBotBox utilise une copie autonome (voir plus bas).

// Dimensions du terrain (cm). Anciennement definies dans CPhysicalEngine.h, relocalisees
// ici lors du retrait de Box2D (etape 3 migration SimuBot v2). Servent a configurer la
// collision bordure du moteur cinematique.
#define X_TERRAIN 300.0f
#define Y_TERRAIN 200.0f

// Declaration anticipee : evite d'inclure <QJsonArray>/<QJsonObject> dans le header
// (l'implementation les inclut).
class QJsonArray;
class QJsonObject;

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
    // simuView est resolu par findChild() dans CSimuBot::init() : initialise a nullptr pour que
    // tout code appele avant (ex. refreshRoseDesVents via un changed() de la scene) puisse tester
    // sa validite au lieu de dereferencer un pointeur non initialise.
    Cihm_SimuBot(QWidget *parent = 0)  : QMainWindow(parent), simuView(nullptr) { ui.setupUi(this); }
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

	   
// ETAPE 3bis : reconstructeur de pose "l'asserv fait foi" pour le chemin asserv interne.
// ETAPE 6 : decouplage build Simulia (avec logique robot) vs LaBotBox (sans). Le type utilise par
// CSimuBot est CPoseReconstructeur (typedef ci-dessous), d'interface identique dans les deux cas
// (setPosition_XYTeta, distance_roue_D/G[_prec], CalculXY, X_robot/Y_robot/angle_robot,
// DISTANCE_PAR_PAS_CODEUR_D/G) -> CSimuBot.cpp est identique quel que soit le build.
#ifdef SIMUBOT_ROBOT_LOGIC
// Build Simulia : on utilise le VRAI CAsservissementBase (representativite max, synchro firmware).
// CalculXY est protected dans CAsservissementBase ; cette sous-classe l'expose en public sans rien
// changer d'autre. On n'engage jamais la regulation : seul CalculXY est appele, il ne depend que
// de distance_roue_D/G (publics) et de son etat interne _prec.
#include "CAsservissementBase.h"
class CPoseReconstructeurAsserv : public CAsservissementBase {
public:
    using CAsservissementBase::CalculXY;
};
typedef CPoseReconstructeurAsserv CPoseReconstructeur;
#else
// Build LaBotBox (pas de logique robot) : copie autonome fidele de CalculXY, comportement identique
// tant qu'elle reste alignee sur CAsservissementBase (cf. CPoseReconstructeurStandalone.h).
#include "CPoseReconstructeurStandalone.h"
typedef CPoseReconstructeurStandalone CPoseReconstructeur;
#endif

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

    // Etape 6 (correctif) : stoppe l'asserv interne (cible invalidee, forces a zero) pour figer
    // le sprite. Appele sur arret/fin HIL (CBlockBotLab, lambda hilFinished, couvre Stop HIL ET
    // FIN_MISSION) et par syncInternalSim quand la sim interne cesse de jouer le role du robot.
    void stopInternalSim();

    // ETAPE 7 : la rose des vents doit se replier dans le viewport quand l'ancrage naturel
    // (au-dessus du terrain) n'est plus visible. Un redimensionnement de la fenetre change la
    // surface visible SANS emettre QGraphicsScene::changed : sans ce filtre, la rose resterait
    // hors champ jusqu'a la prochaine modification de la scene (cas du mode TEST, robot immobile).
    virtual bool eventFilter(QObject *watched, QEvent *event) override;

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
        void slot_clearPath();

        void updateStepFromSimuBot();

        // --- Etape 6 : bascule transparente robot RS232 <-> sim interne pour le HIL en VISU ---
        // Handlers des commandes de deplacement (front descendant des COMMANDE_*_TxSync). Un par
        // cle = source non ambigue. Actifs uniquement quand la sim interne joue le role du robot
        // (internalSimActive) ; sinon no-op (CTrameFactory transmet la commande au vrai robot).
        void onCmdMoveXY();
        void onCmdMoveXYT();
        void onCmdMoveDA();
        // (Re)ajuste la sim interne quand la connexion RS232 au robot apparait/disparait.
        void onRobotConnectionChanged();

        // --- Etape 7 : rose des vents ---
        // Met a jour les deux caps de la rose ET recalcule son ancrage (naturel au-dessus du
        // terrain, ou repli dans le viewport si l'ancrage naturel n'est plus visible). Slot pour
        // pouvoir etre raccorde directement aux barres de defilement de la vue.
        void refreshRoseDesVents(void);

private:
    Cihm_SimuBot m_ihm;

    //ajouté par Steph
    GraphicElement *GrosBot;
    GraphicElement *OldGrosBot;
    QGraphicsLineItem *liaison_GrosBot;
    GraphicElement *MiniBot;
	GraphicElement *OtherBot;
    QList<QGraphicsLineItem*> evitement;
    // Sprites des elements de jeu. Etape 4 : conteneur dynamique de QGraphicsItem* (base commune
    // rect / polygone / ellipse) au lieu d'un tableau fixe de rectangles : le nombre ET la forme
    // des elements varient chaque annee et sont charges depuis un JSON. L'index dans ce vecteur =
    // id de l'obstacle cote CCollisionEngine (cf. loadTerrainFromJson / refreshGameElementsView).
    QVector<QGraphicsItem*> elementsJeu;
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
    // Moteur de simulation : cinematique differentielle analytique (etape 2 migration v2).
    // Box2D (CPhysicalEngine) a ete retire a l'etape 3 ; ce moteur est desormais l'unique
    // implementation.
    CKinematicEngine m_kinematic_engine;
    // Collisions geometriques maison (etape 3) : bordures (demi-plans) + elements de jeu
    // (SAT polygone-polygone), en repere terrain. Branche dans m_kinematic_engine.
    CCollisionEngine m_collision_engine;
    // ETAPE 7 : rose des vents (indicateur de cap, remplace le "nez" du robot retire a l'etape 5).
    // Ajoutee a la scene par init() -> la scene en devient proprietaire, rien a liberer ici.
    CRoseDesVents *m_rose_des_vents;
    // Cap de depart du robot en degres, dans le repere ASSERV (celui du code de strategie) :
    // capture a chaque raz (initView) via GrosBot->getTheta(), donc avec le meme accesseur que le
    // cap courant -> les deux aiguilles de la rose sont dans le meme repere par construction.
    qreal m_cap_init_deg;
    // Gain de conversion consigne -> vitesse roue (cm/s) pour le moteur cinematique.
    // Simulia publie vect_G/D = 80 * commande_moteur_% (valeur d'impulsion calibree pour
    // Box2D, cf. CRoues_simu.cpp), PAS une vitesse. On convertit : vitesse_cm_s = vect * gain.
    // Calibrable via EEPROM "kin_speed_gain" (defaut 0.01 -> ~80 cm/s a pleine commande).
    // Type double (pas float) pour que QSettings ecrive la valeur en clair dans EEPROM.ini
    // ("0.01") et non en blob binaire @Variant(...) reserve aux types non textuels (float).
    double m_kin_speed_gain;
    // Constante de temps moteur (s) du moteur cinematique : inertie premier ordre sur la
    // vitesse roue (asserv en vitesse). Calibrable via EEPROM "kin_motor_tau" (defaut 0.15).
    // double pour la meme raison que m_kin_speed_gain (lisibilite EEPROM.ini).
    double m_kin_motor_tau;
    // ETAPE 3bis - chemin asserv interne (SIMU sans Simulia) : reconstructeur de pose
    // autonome. Aucun code Modelia ne tourne dans ce chemin, donc on rejoue localement la
    // reconstruction que ferait le firmware : meme classe de calcul (CAsservissementBase::
    // CalculXY, schema "tourne-puis-avance") alimentee par la distance cumulee par roue,
    // pour publier x_pos/y_pos/teta_pos en repere asserv (representatif, comme le chemin
    // Simulia). N'engage PAS la regulation (ni gains, ni CommandeMouvement) : seul CalculXY
    // est appele, qui ne depend que de distance_roue_D/G et de son etat _prec.
    CPoseReconstructeur m_pose_reconstructor_interne;
    // Distance cumulee par roue [cm] alimentant le reconstructeur (somme des pas codeurs du
    // moteur cinematique x DISTANCE_PAR_PAS_CODEUR). Remise a 0 au raz.
    float m_cumul_distance_D;
    float m_cumul_distance_G;
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
    void getUSDistance(Coord bot, Coord obstacle, float capteurs[], float lidar[]);
    QGraphicsRectItem *setElementJeu(float x, float y, int Color, bool vertical);
    // Repercute dans la scene Qt le deplacement des elements de jeu pousses par le robot
    // (le moteur de collisions deplace les caisses mobiles ; on translate leur sprite).
    void refreshGameElementsView();
    // --- Etape 4 : terrain + elements de jeu externalises en JSON rechargeable a chaud ---
    // Charge le terrain (dimensions) et les elements de jeu depuis un fichier JSON : cree les
    // sprites Qt (rect / polygone / cercle / ellipse) ET declare les obstacles au moteur de
    // collisions (id = index dans elementsJeu). Cercle/ellipse approximes en polygone (n_cotes)
    // pour le SAT. Renvoie false si le fichier est absent/invalide (on garde alors zero element
    // plutot que de planter). Chemin resolu : absolu tel quel, sinon relatif a applicationDirPath.
    bool loadTerrainFromJson(const QString& path);
    // Cree les sprites + obstacles d'une liste JSON (decor OU elements de jeu). forceFixed=true
    // force mobile=false (decor fixe, comme les bordures). runningId : id courant (partage entre
    // decor et elements, = index dans elementsJeu), incremente pour chaque entree.
    void addJsonElements(const QJsonArray& arr, bool forceFixed, int& runningId);
    // Ecrit un fichier terrain JSON par defaut (terrain 300x200 + regles en commentaire) si absent,
    // a l'instar de EEPROM.ini. Cree le dossier Config au besoin.
    void writeDefaultTerrainFile(const QString& path);
    // Detruit les sprites d'elements de jeu (retrait de la scene + delete) et vide elementsJeu.
    void clearGameElements();
    // Convertit un nom de couleur JSON ("jaune"/"bleu"/"rouge"/...) en QColor (defaut gris).
    QColor colorFromName(const QString& name) const;
    // Chemin du fichier terrain JSON (chemin fixe dans Config/) et surveillance hot-reload.
    QString m_terrain_json_path;
    QFileSystemWatcher* m_terrain_watcher;
    // --- Etape 5 : forme du/des robot(s) externalisee en JSON (remplace les cles EEPROM
    // "polygon"/"polygon2"). Chemin fixe Config/robot.json, auto-cree si absent. ---
    // Charge les polygones GrosBot + MiniBot depuis un JSON (repere robot local, y haut, cm ->
    // scene y bas). Cree un defaut si absent. Renvoie false et laisse un repli (octogone) si KO.
    bool loadRobotFromJson(const QString& path, QPolygonF& gros, QPolygonF& mini);
    // Convertit un objet { "sommets_cm": [[x,y],...] } (repere robot, y haut) en QPolygonF scene.
    QPolygonF polygonFromJson(const QJsonObject& obj) const;
    // Ecrit un robot JSON par defaut (octogone GrosBot + MiniBot) si absent, comme terrain.json.
    void writeDefaultRobotFile(const QString& path);
    // Chemin du fichier robot JSON (chemin fixe dans Config/).
    QString m_robot_json_path;
    // --- Etape 6 : bascule transparente robot RS232 <-> sim interne (HIL en mode VISU) ---
    // Vrai quand la sim interne doit produire x_pos/y_pos/teta_pos (+ Convergence) a la place du
    // robot : SIMU-sans-Simulia OU VISU avec robot RS232 deconnecte.
    bool internalSimActive() const;
    // Etat de la liaison RS232 au robot ("Robot_Connecte", maintenu par CMessagerieBot). Defaut
    // deconnecte si la cle n'existe pas (CMessagerieBot absent) -> la sim interne prend la main.
    bool robotConnecte() const;
    // Demarre/arrete le cadenceur de la sim interne selon internalSimActive().
    void syncInternalSim();
    // Remet a 0 Convergence/convergence_rapide a l'arrivee d'une nouvelle commande de deplacement.
    void resetPublishedConvergence();
    // Positionne l'etat "Simulia actif" ET la case a cocher correspondante, en un seul geste.
    // Utilise pour aligner automatiquement la case sur le mode de visualisation (cf. changeMode) :
    // le mode VISU n'est pas dedie a Simulia, le mode SIMU l'est.
    void setSimuliaEnabled(bool enabled);
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
    void estimate_Environment_Interactions();
    void real_robot_position_changed();
    void catchDoubleClick();
    // Hot-reload du terrain : declenche par QFileSystemWatcher quand terrain_json_path change.
    void reloadTerrain();
    void slot_enableSimulia(int state);
};

#endif // _CBASIC_MODULE_SimuBot_H_

/*! @} */


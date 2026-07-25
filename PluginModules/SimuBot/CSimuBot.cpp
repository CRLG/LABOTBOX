/*! \file CSimuBot.cpp
 * A brief file description CPP.
 * A more elaborated file description.
 */
#include <QDebug>
#include "CSimuBot.h"
#include "CApplication.h"
#include "CPrintView.h"
#include "CMainWindow.h"
#include "CEEPROM.h"
#include "CDataManager.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QCoreApplication>
#include <QColor>
#include <math.h>

/*!
 * \brief normalizeAngleDeg utilitaire de normalisation d'angle en degré
 * \param angleRaw
 * \return
 */
static qreal normalizeAngleDeg(qreal angleRaw)
{
    //angleRaw=(-1.0)*angleRaw;
    while (angleRaw < -180)
        angleRaw += 360;
    while (angleRaw > 180)
        angleRaw -= 360;
    //angleRaw=(angleRaw/360.0)*TwoPi;
    return angleRaw;
}

/*! \addtogroup PluginModule_Test2
   * 
   *  @{
   */
	   
// _____________________________________________________________________
/*!
*  Constructeur
*
*/
CSimuBot::CSimuBot(const char *plugin_name)
    :CPluginModule(plugin_name, VERSION_SimuBot, AUTEUR_SimuBot, INFO_SimuBot)
{
    //on initialise l'environnement
    terrain=new GraphicEnvironnement;

    //On positionne par défaut l'affichage en radian
    //attention cette valeur est écrasée par celle du fichier eeprom si elle est renseignée
    setAndGetInRad=true;

    //init des variables de positionnement relatif du robot
    deltaAngle=0;
    deltaDistance=0;

    m_connected_host=false;

    // Watcher de hot-reload du terrain JSON : cree dans init() (cf. loadTerrainFromJson).
    m_terrain_watcher=nullptr;
}


// _____________________________________________________________________
/*!
*  Destructeur
*
*/
CSimuBot::~CSimuBot()
{

}


// _____________________________________________________________________
/*!
*  Initialisation du module
*   Lecture des valeurs de l'eeprom (contour du robot,
*
*/
void CSimuBot::init(CApplication *application)
{
    CPluginModule::init(application);
    setGUI(&m_ihm); // indique à la classe de base l'IHM
    setNiveauTrace(MSG_TOUS);

    // Gère les actions sur clic droit sur le panel graphique du module
    m_ihm.setContextMenuPolicy(Qt::CustomContextMenu);
    connect(&m_ihm, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(onRightClicGUI(QPoint)));

    // Restore la taille de la fenêtre
    QVariant val;
    val = m_application->m_eeprom->read(getName(), "geometry", QRect(50, 50, 150, 150));
    m_ihm.setGeometry(val.toRect());
    // Restore le fait que la fenêtre est visible ou non
    val = m_application->m_eeprom->read(getName(), "visible", QVariant(true));
    if (val.toBool()) { m_ihm.show(); }
    else              { m_ihm.hide(); }
    // Restore le niveau d'affichage
    val = m_application->m_eeprom->read(getName(), "niveau_trace", QVariant(MSG_TOUS));
    setNiveauTrace(val.toUInt());
    // Restore la couleur de fond
    val = m_application->m_eeprom->read(getName(), "background_color", QVariant(DEFAULT_MODULE_COLOR));
    setBackgroundColor(val.value<QColor>());

    // Recuperation des contours de GrosBot et MiniBot depuis un fichier JSON dedie (etape 5
    // migration v2). Auparavant lues dans EEPROM.ini (cles "polygon"/"polygon2", format
    // "(doublexdouble)") ; externalisees dans Config/robot.json, aux cotes d'EEPROM.ini et de
    // terrain.json, avec auto-creation d'un defaut si absent (a l'instar du terrain). Chemin
    // FIXE (pas d'entree EEPROM). Repere du fichier : robot local, front vers +x, y vers le
    // HAUT, cm ; loadRobotFromJson convertit en repere scene (y bas) via QPointF(x,-y).
    QPolygonF GrosBotForme, MiniBotForme;
    m_robot_json_path = m_application->m_pathname_config_file + "/robot.json";
    loadRobotFromJson(m_robot_json_path, GrosBotForme, MiniBotForme);
    QPolygonF GrosBotFormeOrientation;
    GrosBotFormeOrientation << QPointF(0.0,5.0) << QPointF((4.0+(GrosBotForme.boundingRect().width())/2.0),0.0) << QPointF(0.0,-5.0);
    QPolygonF GrosBotFormeOriented=GrosBotForme.united(GrosBotFormeOrientation);

    QPolygonF MiniBotFormeOrientation;
    MiniBotFormeOrientation << QPointF(0.0,5.0) << QPointF((4.0+(MiniBotForme.boundingRect().width())/2.0),0.0) << QPointF(0.0,-5.0);
    QPolygonF MiniBotFormeOriented=MiniBotForme.united(MiniBotFormeOrientation);

    // Moteur de simulation : cinematique differentielle analytique (CKinematicEngine).
    // Box2D (CPhysicalEngine) a ete retire a l'etape 3 de la migration v2 ; il n'y a plus
    // de selecteur de moteur ni de monde physique a creer (cf. rapport_simubot.md etape 3).
    // Gain consigne -> vitesse roue (cm/s) du moteur cinematique. Simulia publie
    // vect_G/D = 80 * commande_% (impulsion Box2D, cf. CRoues_simu.cpp), pas une vitesse :
    // on convertit vitesse_cm_s = vect * gain. Calibrable a chaud via EEPROM.
    val = m_application->m_eeprom->read(getName(), "kin_speed_gain", QVariant(0.01));
    m_kin_speed_gain = val.toDouble();
    // Constante de temps moteur (inertie, asserv en vitesse) : evite le pompage de l'asserv
    // sur les petits deplacements / en zone de convergence. Calibrable a chaud.
    val = m_application->m_eeprom->read(getName(), "kin_motor_tau", QVariant(0.15));
    m_kin_motor_tau = val.toDouble();
    m_kinematic_engine.setMotorTau(m_kin_motor_tau);
    // Configuration de la collision bordure du moteur cinematique : terrain 300x200 cm
    // et rayon du cercle englobant la forme GrosBot. Sert de repli (modele disque, etape 2)
    // si aucun moteur de collisions geometriques n'est branche.
    m_kinematic_engine.setTerrain(0.0f, 0.0f, X_TERRAIN, Y_TERRAIN);
    m_kinematic_engine.setRobotRadius(0.5f * (float)qMax(GrosBotForme.boundingRect().width(),
                                                         GrosBotForme.boundingRect().height()));

    // Collisions geometriques maison (etape 3) : bordures + elements de jeu en repere terrain,
    // SAT polygone-polygone. Le robot est sa vraie forme polygonale (GrosBotForme, repere local
    // cm, front +x), pas un disque -> contact "coin" correct, ce que l'AABB ne donnerait pas.
    m_collision_engine.setTerrain(0.0f, 0.0f, X_TERRAIN, Y_TERRAIN);
    std::vector<CCollisionEngine::Vec2> robotShape;
    for (int ip = 0; ip < GrosBotForme.size(); ++ip)
    {
        robotShape.push_back({ (float)GrosBotForme.at(ip).x(), (float)GrosBotForme.at(ip).y() });
    }
    m_collision_engine.setRobotShape(robotShape);
    m_collision_engine.clearObstacles(); // idempotent si init() rejoue
    // Branche le moteur de collisions sur la cinematique : step() resout desormais les
    // collisions polygonales (les pas codeurs derivent du deplacement reel -> bijection SIL).
    m_kinematic_engine.setCollisionEngine(&m_collision_engine);

    //récupération des positions d'init de GrosBot
    val = m_application->m_eeprom->read(getName(), "X_init_1_bot1", QVariant(42.0));
    float X_init_1=val.toFloat();
    val = m_application->m_eeprom->read(getName(), "Y_init_1_bot1", QVariant(165.0));
    float Y_init_1=val.toFloat();
    val = m_application->m_eeprom->read(getName(), "Theta_init_1_bot1", QVariant(-M_PI_2f));
    float Theta_init_1=val.toFloat();
    val = m_application->m_eeprom->read(getName(), "X_init_2_bot1", QVariant(258.0));
    float X_init_2=val.toFloat();
    val = m_application->m_eeprom->read(getName(), "Y_init_2_bot1", QVariant(165.0));
    float Y_init_2=val.toFloat();
    val = m_application->m_eeprom->read(getName(), "Theta_init_2_bot1", QVariant(-M_PI_2f));
    float Theta_init_2=val.toFloat();
    equipe1_bot1.init(X_init_1,Y_init_1,Theta_init_1,true);
    equipe2_bot1.init(X_init_2,Y_init_2,Theta_init_2,false);

    val = m_application->m_eeprom->read(getName(), "Theta_asserv_init_1_bot1", QVariant(-M_PI_2f));
    iniTetaAsserv_bot1[EQUIPE1]=val.toFloat();
    val = m_application->m_eeprom->read(getName(), "Theta_asserv_init_2_bot1", QVariant(M_PI_2f));
    iniTetaAsserv_bot1[EQUIPE2]=val.toFloat();

    //récupération des positions d'init de MiniBot
    val = m_application->m_eeprom->read(getName(), "X_init_1_bot2", QVariant(18.0));
    X_init_1=val.toFloat();
    val = m_application->m_eeprom->read(getName(), "Y_init_1_bot2", QVariant(104.0));
    Y_init_1=val.toFloat();
    val = m_application->m_eeprom->read(getName(), "Theta_init_1_bot2", QVariant(-1.57));
    Theta_init_1=val.toFloat();
    val = m_application->m_eeprom->read(getName(), "X_init_2_bot2", QVariant(282.0));
    X_init_2=val.toFloat();
    val = m_application->m_eeprom->read(getName(), "Y_init_2_bot2", QVariant(104.0));
    Y_init_2=val.toFloat();
    val = m_application->m_eeprom->read(getName(), "Theta_init_2_bot2", QVariant(1.57));
    Theta_init_2=val.toFloat();
    equipe1_bot2.init(X_init_1,Y_init_1,Theta_init_1,true);
    equipe2_bot2.init(X_init_2,Y_init_2,Theta_init_2,false);

    val = m_application->m_eeprom->read(getName(), "Theta_asserv_init_1_bot2", QVariant(-1.57));
    iniTetaAsserv_bot2[EQUIPE1]=val.toFloat();
    val = m_application->m_eeprom->read(getName(), "Theta_asserv_init_2_bot2", QVariant(1.57));
    iniTetaAsserv_bot2[EQUIPE2]=val.toFloat();

    //ajout des limites physiques du terrain
    QGraphicsPixmapItem *surface=new QGraphicsPixmapItem();
    surface->setPixmap(QPixmap(":/icons/terrain_2026_simubot.png"));
    surface->setPos(0,-200);
    QGraphicsRectItem *bordures=new QGraphicsRectItem(QRect(0, -200 , 300, 200));
    terrain->addItem(surface);
    terrain->addItem(bordures);

    //ajout des éléments de jeu
    // Terrain (etape 4) : dimensions + decor fixe (estrades, tasseaux... = obstacles au meme titre
    // que les bordures) + elements de jeu, externalises en JSON dans le dossier Config, aux cotes
    // de EEPROM.ini. Le nombre ET la forme des elements changent chaque annee (rectangles,
    // polygones, disques...). Cree avec un contenu par defaut si absent, a l'instar de EEPROM.ini.
    // Chemin FIXE (pas d'entree EEPROM). loadTerrainFromJson cree les sprites (rect/polygone/cercle/
    // ellipse) ET declare les obstacles au moteur de collisions (cercle approxime en polygone SAT).
    m_terrain_json_path = m_application->m_pathname_config_file + "/terrain.json";
    loadTerrainFromJson(m_terrain_json_path);

    // Hot-reload : surveiller le fichier et recharger a chaque modification (sans recompiler).
    m_terrain_watcher = new QFileSystemWatcher(this);
    connect(m_terrain_watcher, SIGNAL(fileChanged(QString)), this, SLOT(reloadTerrain()));
    if (QFile::exists(m_terrain_json_path))
        m_terrain_watcher->addPath(m_terrain_json_path);


    //ajout des tasseaux 2022

    /*QGraphicsLineItem * Tasseau01;
    Tasseau01=new QGraphicsLineItem(0,-51,51,0);
    Tasseau01->setPen(QPen(Qt::yellow,1));
    terrain->addItem(Tasseau01);
    QGraphicsLineItem * Tasseau02;
    Tasseau02=new QGraphicsLineItem(249,0,300,-51);
    Tasseau02->setPen(QPen(Qt::blue,1));
    terrain->addItem(Tasseau02);
    QGraphicsLineItem * Tasseau03;
    Tasseau03=new QGraphicsLineItem(150,-200,150,-170);
    Tasseau03->setPen(QPen(Qt::gray,2));
    terrain->addItem(Tasseau03);*/



    //ajout du robot
    val = m_application->m_eeprom->read(getName(), "bot_oriented", QVariant(false));
    bool isOriented=val.toBool();
    if(isOriented)
        GrosBot=new GraphicElement(GrosBotFormeOriented,255,255,255);
    else
        GrosBot=new GraphicElement(GrosBotForme,255,255,255);
    GrosBot->setBrush(QBrush(QColor(255, 255,255, 255)));

    //ajout du point de référence du robot
    QPolygonF OldGrosBotForme;
    OldGrosBotForme << QPointF(5,-7) << QPointF(-5,-7)<< QPointF(-7,-5)<< QPointF(-7,5) << QPointF(-5,7);
    OldGrosBotForme << QPointF(5,7) << QPointF(7,5) << QPointF(7,-5) << QPointF(5,-7);
    OldGrosBot= new GraphicElement(OldGrosBotForme,255,255,255);
    OldGrosBot->setFlag(QGraphicsItem::ItemIsMovable, false);
    OldGrosBot->setFlag(QGraphicsItem::ItemIsSelectable, false);
    OldGrosBot->setBrush(QBrush(QColor(255,255,255, 100)));

    //on lie les deux points
    QLineF liaison_Line(GrosBot->getX(),GrosBot->getY(),OldGrosBot->getX(),OldGrosBot->getY());
    liaison_GrosBot= new QGraphicsLineItem(liaison_Line);
	
	//ajout d'un robot adverse
    QPolygonF OtherBotForme;
    OtherBotForme << QPointF(9,-15) << QPointF(-9,-15)<< QPointF(-15,-9)<< QPointF(-15,9) << QPointF(-9,15);
    OtherBotForme << QPointF(9,15) << QPointF(15,9) << QPointF(15,-9) << QPointF(9,-15);
    OtherBot= new GraphicElement(OtherBotForme,255,255,255);
    OtherBot->setFlag(QGraphicsItem::ItemIsMovable, true);
    OtherBot->setFlag(QGraphicsItem::ItemIsSelectable, true);
    OtherBot->setBrush(QBrush(QColor(255,0,0, 100)));

    //ajout d'un MiniBot
    if(isOriented)
        MiniBot= new GraphicElement(MiniBotFormeOriented,255,255,255);
    else
        MiniBot= new GraphicElement(MiniBotForme,255,255,255);
    MiniBot->setFlag(QGraphicsItem::ItemIsMovable, true);
    MiniBot->setFlag(QGraphicsItem::ItemIsSelectable, true);
    MiniBot->setBrush(QBrush(QColor(255,255,255, 255)));

    //on place le robot et on crée le mécanisme d'init
    QPushButton *pushButton_init=m_ihm.findChild<QPushButton*>("pushButton_init");
    connect(pushButton_init,SIGNAL(clicked()),this,SLOT(initView()));
    QRadioButton *radioButton_couleur_1=m_ihm.findChild<QRadioButton*>("radioButton_couleur_1");
    QRadioButton *radioButton_couleur_2=m_ihm.findChild<QRadioButton*>("radioButton_couleur_2");
    connect(radioButton_couleur_1,SIGNAL(clicked()),this,SLOT(changeEquipe()));
    connect(radioButton_couleur_2,SIGNAL(clicked()),this,SLOT(changeEquipe()));

    //ecrasement des données
    connect(m_ihm.ui.lineEdit_x,SIGNAL(editingFinished()),this,SLOT(returnCapture_XY()));
    connect(m_ihm.ui.lineEdit_y,SIGNAL(editingFinished()),this,SLOT(returnCapture_XY()));
    connect(m_ihm.ui.lineEdit_theta,SIGNAL(editingFinished()),this,SLOT(returnCapture_Theta()));
    connect(m_ihm.ui.dial_rotation_bot,SIGNAL(sliderReleased()),this,SLOT(slot_dial_turned()));

    //demande de déplacement du robot
    connect(this, SIGNAL(displayCoord(qreal,qreal)), GrosBot,SLOT(display_XY(qreal,qreal)));
    connect(this,SIGNAL(displayAngle(qreal)),GrosBot,SLOT(display_theta(qreal)));
    connect(this, SIGNAL(displayCoord2(qreal,qreal)), MiniBot,SLOT(display_XY(qreal,qreal)));
    connect(this,SIGNAL(displayAngle2(qreal)),MiniBot,SLOT(display_theta(qreal)));

    //pour changer de mode visu ou placement
    connect(m_ihm.ui.horizontalSlider_toggle_simu,SIGNAL(valueChanged(int)),this,SLOT(changeMode(int)));
    val=m_application->m_eeprom->read(getName(),"mode_visu",QVariant(0));
    // Bloquer les signaux avant setValue() pour éviter un double appel à changeMode() :
    // si la valeur sauvegardée est identique à la valeur par défaut du slider (0 = TEST),
    // setValue() n'émet pas valueChanged → changeMode() ne serait jamais appelé →
    // modeVisu resterait indéterminé → initView() échouerait silencieusement sa condition.
    m_ihm.ui.horizontalSlider_toggle_simu->blockSignals(true);
    m_ihm.ui.horizontalSlider_toggle_simu->setValue(val.toInt());
    m_ihm.ui.horizontalSlider_toggle_simu->blockSignals(false);
    changeMode(val.toInt());

    //on initialise et ajoute le robot au terrain
    //TODO: prendre un fichier de config pour l'emplacement et l'angle de départ pour le robot
    // pour l'instant c'est en dur dans le constructeur
    connect(terrain, SIGNAL(changed(QList<QRectF>)), this, SLOT(viewChanged(QList<QRectF>)));
    connect(GrosBot,SIGNAL(center(qreal,qreal)),OldGrosBot,SLOT(replace(qreal,qreal)));
    connect(GrosBot,SIGNAL(isDoubleClicked()),this,SLOT(catchDoubleClick()));
	
	
    terrain->addItem(OldGrosBot);
    terrain->addItem(GrosBot);
    terrain->addItem(liaison_GrosBot);
	terrain->addItem(OtherBot);
    terrain->addItem(MiniBot);

    //Mise en place du terrain
    m_ihm.simuView=m_ihm.findChild<QGraphicsView*>("simuGraphicsView");
    m_ihm.simuView->setRenderHint(QPainter::Antialiasing);
    m_ihm.simuView->centerOn(QPointF(163,118));
    m_ihm.simuView->setCacheMode(QGraphicsView::CacheBackground);
    m_ihm.simuView->setViewportUpdateMode(QGraphicsView::BoundingRectViewportUpdate);
    m_ihm.simuView->setDragMode(QGraphicsView::ScrollHandDrag);
    m_ihm.simuView->resize(326, 236);
    m_ihm.simuView->setScene(terrain);
    connect(m_ihm.ui.verticalSlider_zoom_scene,SIGNAL(valueChanged(int)),this,SLOT(zoom(int)));
    val = m_application->m_eeprom->read(getName(), "zoom", QVariant(1));
    m_ihm.ui.verticalSlider_zoom_scene->setValue(val.toInt());

    //changement de repère suivant le choix de coordonnées relatives au terrain ou au robot
    val = m_application->m_eeprom->read(getName(), "isRelativToBot", QVariant(true));
    if (val.toBool()) {m_ihm.ui.radioButton_robot_relative->setChecked(true); }
    else              {m_ihm.ui.radioButton_terrain_relative->setChecked(true); }

    GrosBot->isRelativToBot=val.toBool();
    OldGrosBot->isRelativToBot=val.toBool();
    MiniBot->isRelativToBot=val.toBool();
    OtherBot->isRelativToBot=val.toBool();

    //affichage et saisie en radian ou en degré
    val = m_application->m_eeprom->read(getName(), "anglesEnRadian", QVariant(true));
    if (val.toBool()) {m_ihm.ui.radioButton_radian->setChecked(true); setAndGetInRad=true; }
    else              {m_ihm.ui.radioButton_degre->setChecked(true); setAndGetInRad=false; }

    //pour calculer une trajectoire d'evitement
    // A* (pathfinding) retire : c'etait un test, non conserve pour la Coupe (cf. rapport_simubot.md).
    // Le bouton pb_Astar de l'IHM reste present mais n'est plus connecte.
    connect(m_ihm.ui.pB_clearEvitement,SIGNAL(clicked()),this,SLOT(slot_clearPath()));

    // Positions x, y, teta du robot physique
    m_application->m_data_center->write("x_pos", 0);
    m_application->m_data_center->write("y_pos", 0);
    m_application->m_data_center->write("teta_pos", 0);
    connect(m_application->m_data_center->getData("x_pos"), SIGNAL(valueChanged(QVariant)), this, SLOT(real_robot_position_changed()));
    connect(m_application->m_data_center->getData("y_pos"), SIGNAL(valueChanged(QVariant)), this, SLOT(real_robot_position_changed()));
    connect(m_application->m_data_center->getData("teta_pos"), SIGNAL(valueChanged(QVariant)), this, SLOT(real_robot_position_changed()));
    m_application->m_data_center->write("x_pos2", 0);
    m_application->m_data_center->write("y_pos2", 0);
    m_application->m_data_center->write("teta_pos2", 0);
    connect(m_application->m_data_center->getData("x_pos2"), SIGNAL(valueChanged(QVariant)), this, SLOT(real_robot_position_changed()));
    connect(m_application->m_data_center->getData("y_pos2"), SIGNAL(valueChanged(QVariant)), this, SLOT(real_robot_position_changed()));
    connect(m_application->m_data_center->getData("teta_pos2"), SIGNAL(valueChanged(QVariant)), this, SLOT(real_robot_position_changed()));


    //init de ces variables dans le data manager
    /*m_application->m_data_center->write("PosTeta_robot", 0.);
    m_application->m_data_center->write("PosX_robot", 0.);
    m_application->m_data_center->write("PosY_robot", 0.);
    m_application->m_data_center->write("PosTeta_robot2", 0.);
    m_application->m_data_center->write("PosX_robot2", 0.);
    m_application->m_data_center->write("PosY_robot2", 0.);
    m_application->m_data_center->write("Cde_MotG", 0);
    m_application->m_data_center->write("Cde_MotD", 0);*/
    m_application->m_data_center->write("Simubot.Telemetres.AVG", 99);
    m_application->m_data_center->write("Simubot.Telemetres.AVD", 99);
    m_application->m_data_center->write("Simubot.Telemetres.ARG", 99);
    m_application->m_data_center->write("Simubot.Telemetres.ARD", 99);
    /*m_application->m_data_center->write("Simubot.OtherBot.x", 0);
    m_application->m_data_center->write("Simubot.OtherBot.y", 0);
    m_application->m_data_center->write("Simubot.Bot.x", 0);
    m_application->m_data_center->write("Simubot.Bot.y", 0);
    m_application->m_data_center->write("Simubot.Bot.teta", 0);*/
    m_application->m_data_center->write("Simubot.Telemetres.AVG2", 99);
    m_application->m_data_center->write("Simubot.Telemetres.AVD2", 99);
    m_application->m_data_center->write("Simubot.Telemetres.ARG2", 99);
    m_application->m_data_center->write("Simubot.Telemetres.ARD2", 99);
    m_application->m_data_center->write("Simubot.Init", true);

    //pour le mode simu (fonctionnement avec Simulia)
    // Aligne sur le mode restaure depuis l'EEPROM, comme le fait ensuite tout changement de mode
    // (cf. changeMode) : le mode VISU n'est pas dedie a Simulia, TEST et SIMU le sont. Sans cette
    // coherence, la restauration ecraserait la decision que changeMode vient de prendre plus haut.
    setSimuliaEnabled(modeVisu != SIMUBOT::VISU);
    connect(m_ihm.ui.checkBox_enableSimulia,SIGNAL(stateChanged(int)),this,SLOT(slot_enableSimulia(int)));

    cadenceur=new QTimer(); //timer pour cadencer les mouvements autonomes du 2ème robot
    //connections pour éditer/gérer le déplacement autonome du 2ème robot
    connect(m_ihm.ui.pB_US,SIGNAL(clicked(bool)),this,SLOT(estimate_Environment_Interactions()));
    connect(m_ihm.ui.pB_playOther,SIGNAL(clicked(bool)),this,SLOT(playOther()));
    connect(m_ihm.ui.pB_stopOther,SIGNAL(clicked(bool)),this,SLOT(stopOther()));
    connect(cadenceur, SIGNAL(timeout()), this, SLOT(updateStepFromSimuBot()));
    connect(m_ihm.ui.chkBox_enableMoveOther, SIGNAL(stateChanged(int)), this, SLOT(enableMoveOther(int)));
    connect(m_application->m_data_center->getData("Simubot.Init", true), SIGNAL(valueChanged(bool)), this, SLOT(initView()));
    connect(m_application->m_data_center->getData("Capteurs.Tirette", true), SIGNAL(valueChanged(bool)),this , SLOT(syncMove(bool)));
    QStringList QS_Labels;
    QS_Labels << "x" << "y" << "teta";
    m_ihm.ui.tableWidget->setHorizontalHeaderLabels(QS_Labels);
    addStepOther(0,-50,0,0);
    addStepOther(200,-50,0,1);
    addStepOther(200,70,0,2);
    addStepOther(0,70,0,3);
    addStepOther(0,0,0,4);
    isStarted=false;
    isStarted_old=false;

    MiniBot->setVisible(false);
    twoBotsEnabled=false;
    m_ihm.ui.groupBox_MiniBot->setEnabled(false);
    connect(m_ihm.ui.ckhB_2Bot,SIGNAL(stateChanged(int)),this,SLOT(enableTwoBots(int)));
    val = m_application->m_eeprom->read(getName(), "use_2_bots", QVariant(false));
    m_ihm.ui.ckhB_2Bot->setChecked(val.toBool());

    // Robot n°2 externe
    connect(m_ihm.ui.active_external_robot2, SIGNAL(clicked(bool)), this, SLOT(on_active_external_robot2(bool)));
    //connect(&m_timer_external_robot2, SIGNAL(timeout()), this, SLOT(on_timeout_external_robot2()));

    //pour le moteur de simulation cinematique (declenche a chaque pas de Simulia)
    connect(m_application->m_data_center->getData("Simulia.step", true), SIGNAL(valueChanged(bool)), this, SLOT(updateStepFromSimulia()));

    //pour le fonctionnement avec actuatorsequencer / HIL (etape 6 migration v2)
    // Interception des commandes de deplacement : en VISU avec robot RS232 deconnecte (ou en
    // SIMU-sans-Simulia), elles pilotent la sim interne A LA PLACE du robot -> bascule transparente
    // robot<->sim pour le HIL (cf. internalSimActive/onCmdMove*). En robot connecte, les handlers
    // sont no-op : CTrameFactory transmet la commande au vrai robot, qui renvoie sa position.
    // Un handler par cle = source non ambigue (pas de sender() a inspecter).
    m_application->m_data_center->write("COMMANDE_MVT_XY_TxSync", 0);
    m_application->m_data_center->write("COMMANDE_MVT_XY_TETA_TxSync", 0);
    m_application->m_data_center->write("COMMANDE_DISTANCE_ANGLE_TxSync", 0);
    connect(m_application->m_data_center->getData("COMMANDE_MVT_XY_TxSync", true),         SIGNAL(valueChanged(bool)), this, SLOT(onCmdMoveXY()));
    connect(m_application->m_data_center->getData("COMMANDE_MVT_XY_TETA_TxSync", true),    SIGNAL(valueChanged(bool)), this, SLOT(onCmdMoveXYT()));
    connect(m_application->m_data_center->getData("COMMANDE_DISTANCE_ANGLE_TxSync", true), SIGNAL(valueChanged(bool)), this, SLOT(onCmdMoveDA()));
    // Bascule automatique quand la connexion RS232 au robot apparait/disparait (Robot_Connecte est
    // maintenu par CMessagerieBot sur timeout de trame). Cree la cle si absente (defaut deconnecte).
    connect(m_application->m_data_center->getData("Robot_Connecte", true), SIGNAL(valueChanged(QVariant)), this, SLOT(onRobotConnectionChanged()));

    // Editeur de design robot retire (etape 5 migration v2) : la forme vient desormais de
    // Config/robot.json (cf. loadRobotFromJson), plus d'edition graphique en parallele.

    //positionnement par défaut
    initEquipe(EQUIPE1);
    initView();
}

// _____________________________________________________________________
/*!
*  Fermeture du module
*
*/
void CSimuBot::close(void)
{
  // Mémorise en EEPROM l'état de la fenêtre
  m_application->m_eeprom->write(getName(), "geometry", QVariant(m_ihm.geometry()));
  m_application->m_eeprom->write(getName(), "visible", QVariant(m_ihm.isVisible()));
  m_application->m_eeprom->write(getName(), "background_color", QVariant(getBackgroundColor()));
  m_application->m_eeprom->write(getName(), "niveau_trace", QVariant((unsigned int)getNiveauTrace()));
  m_application->m_eeprom->write(getName(), "mode_visu", QVariant((unsigned int)m_ihm.ui.horizontalSlider_toggle_simu->value()));
  m_application->m_eeprom->write(getName(), "zoom", QVariant((unsigned int)m_ihm.ui.verticalSlider_zoom_scene->value()));
  m_application->m_eeprom->write(getName(), "use_2_bots", QVariant((bool)m_ihm.ui.ckhB_2Bot->isChecked()));
  // Gain de calibration consigne -> vitesse roue du moteur cinematique.
  // QVariant double (pas float) : QSettings serialise les double en texte lisible dans
  // EEPROM.ini ("0.01"), alors qu'un float tombe dans la branche binaire @Variant(...).
  m_application->m_eeprom->write(getName(), "kin_speed_gain", QVariant((double)m_kin_speed_gain));
  // Constante de temps moteur (inertie) du moteur cinematique.
  m_application->m_eeprom->write(getName(), "kin_motor_tau", QVariant((double)m_kin_motor_tau));
  // NB : le chemin du terrain JSON n'est PAS en EEPROM (fichier fixe Config/terrain.json).
}

// _____________________________________________________________________
/*!
*  Création des menus sur clic droit sur la fenêtre du module
*
*/
void CSimuBot::onRightClicGUI(QPoint pos)
{
  QMenu *menu = new QMenu();

  menu->addAction("Select background color", this, SLOT(selectBackgroundColor()));
  menu->exec(m_ihm.mapToGlobal(pos));
}


/*!
 * \brief CSimuBot::viewChanged on met à jour l'IHM quand la scene graphique a été modifiée
 * \param regions
 */
void CSimuBot::viewChanged(QList<QRectF> regions)
{
  Q_UNUSED(regions)
    //récupération des différentes coordonnées (graphique et réelles)
    qreal x_view=GrosBot->getX();
    qreal y_view=GrosBot->getY();
    qreal x_graphic=GrosBot->x();
    qreal y_graphic=GrosBot->y();
    qreal theta_view=GrosBot->getTheta();

    //récupération des différentes coordonnées (graphique et réelles) pour l'ancienne position du robot
    qreal x_prim_view=OldGrosBot->getX();
    qreal y_prim_view=OldGrosBot->getY();
    qreal x_prim_graphic=OldGrosBot->x();
    qreal y_prim_graphic=OldGrosBot->y();

    //on redessine la ligne entre l'ancienne et la nouvelle position
    //TODO : déporter cette mise à jour car ça crée un 2ème appel à ce slot
    liaison_GrosBot->setLine(x_graphic,y_graphic,x_prim_graphic,y_prim_graphic);

    //on calcule la distance entre l'ancienne et la nouvelle position
    deltaDistance= sqrt(pow((x_graphic-x_prim_graphic),2)+pow((y_graphic-y_prim_graphic),2));

    //on calcule de l'angle par rapport à l'ancienne position
    if (x_view==x_prim_view)
    {
        if (y_view>y_prim_view)
            deltaAngle=Pi/2;
        else if (y_view==y_prim_view)
            deltaAngle=0;
        else if (y_view<y_prim_view)
            deltaAngle=-Pi/2;
    }
    else if (x_view>x_prim_view)
        deltaAngle=atan((y_view-y_prim_view)/(x_view-x_prim_view));
    else if (x_view<x_prim_view)
    {
        if (y_view>y_prim_view)
            deltaAngle=atan((y_view-y_prim_view)/(x_view-x_prim_view))+Pi;
        else if (y_view==y_prim_view)
            deltaAngle=Pi;
        else if (y_view<y_prim_view)
            deltaAngle=atan((y_view-y_prim_view)/(x_view-x_prim_view))-Pi;
    }

    //Affichage des nouvelles valeurs position, angle du robot
    if (setAndGetInRad)
        m_ihm.ui.lcdNumber_theta->display(theta_view);
    else
        m_ihm.ui.lcdNumber_theta->display(normalizeAngleDeg(180*theta_view/Pi));
    m_ihm.ui.lcdNumber_x->display(x_view);
    m_ihm.ui.lcdNumber_y->display(y_view);
    m_ihm.ui.lcdNumber_x_terrain->display(GrosBot->getX_terrain());
    m_ihm.ui.lcdNumber_y_terrain->display(GrosBot->getY_terrain());

    //récupération des coordonnées du 2eme robot
    /*qreal x_view2=MiniBot->getX();
    qreal y_view2=MiniBot->getY();
    qreal theta_view2=MiniBot->getTheta();
    if(twoBotsEnabled && !m_ihm.ui.active_external_robot2->isChecked())
    {
        if (setAndGetInRad)
            m_application->m_data_center->write("PosTeta_robot2", theta_view2);
        else
            m_application->m_data_center->write("PosTeta_robot2", normalizeAngleDeg(180*theta_view2/Pi));

        m_application->m_data_center->write("PosX_robot2", x_view2);
        m_application->m_data_center->write("PosY_robot2", y_view2);
    }*/

    //Si on est en mode TEST, on affiche les valeurs complémentaires vis à vis de l'ancienne position du robot
    //et on met à jour le data manager
    switch(modeVisu)
    {
    case SIMUBOT::TEST :
        m_ihm.ui.lcdNumber_distance->display(deltaDistance);
        if (setAndGetInRad){
            m_ihm.ui.lcdNumber_angle->display(deltaAngle);
            m_ihm.ui.lineEdit_theta->setValue(theta_view);
            m_application->m_data_center->write("PosTeta_robot", theta_view);
            m_application->m_data_center->write("DirAngle_robot", deltaAngle);
        }
        else{
            m_ihm.ui.lcdNumber_angle->display(normalizeAngleDeg(180*deltaAngle/Pi));
            m_ihm.ui.lineEdit_theta->setValue(normalizeAngleDeg(180*theta_view/Pi));
            m_application->m_data_center->write("PosTeta_robot", normalizeAngleDeg(180*theta_view/Pi));
            m_application->m_data_center->write("DirAngle_robot", normalizeAngleDeg(180*deltaAngle/Pi));
        }
        m_ihm.ui.lineEdit_x->setValue(x_view);
        m_ihm.ui.lineEdit_y->setValue(y_view);
        //m_ihm.ui.lineEdit_theta->setValue(theta_view);

        // Informe le DataCenter de la mise à jour
        m_application->m_data_center->write("x_pos", x_view);
        m_application->m_data_center->write("y_pos", y_view);
        m_application->m_data_center->write("teta_pos", theta_view);
        m_application->m_data_center->write("DirDistance_robot", deltaDistance);

        /*m_application->m_data_center->write("Simubot.Bot.x", GrosBot->getX_terrain());
        m_application->m_data_center->write("Simubot.Bot.y", GrosBot->getY_terrain());
        m_application->m_data_center->write("Simubot.Bot.teta", GrosBot->getTheta());
        m_application->m_data_center->write("Simubot.OtherBot.x", OtherBot->getX_terrain());
        m_application->m_data_center->write("Simubot.OtherBot.y", OtherBot->getY_terrain());*/
        // capteurs US et detection bordure
        estimate_Environment_Interactions();
        break;
    case SIMUBOT::VISU :
        break;
    case SIMUBOT::SIMU:
        //TODO SIMU: donner les interactions avec l'environnement au datamanger
        estimate_Environment_Interactions();
        /*m_application->m_data_center->write("Simubot.Bot.x", GrosBot->x());
        m_application->m_data_center->write("Simubot.Bot.y", GrosBot->y());
        m_application->m_data_center->write("Simubot.Bot.teta", GrosBot->getTheta());
        m_application->m_data_center->write("Simubot.OtherBot.x", OtherBot->x());
        m_application->m_data_center->write("Simubot.OtherBot.y", OtherBot->y());*/
        /*m_application->m_data_center->write("Simubot.Bot.x", GrosBot->getX_terrain());
        m_application->m_data_center->write("Simubot.Bot.y", GrosBot->getY_terrain());
        m_application->m_data_center->write("Simubot.Bot.teta", GrosBot->getTheta());
        m_application->m_data_center->write("Simubot.OtherBot.x", OtherBot->getX_terrain());
        m_application->m_data_center->write("Simubot.OtherBot.y", OtherBot->getY_terrain());*/
        break;

    default:
        break;

    }

}

void CSimuBot::initView(void){
    bool flag_init=false;
    flag_init=m_application->m_data_center->read("Simubot.Init").toBool();

    if((modeVisu==SIMUBOT::TEST)||(modeVisu==SIMUBOT::VISU)||(modeVisu==SIMUBOT::SIMU && flag_init))
    {
        m_step=0;
        m_step2=0;
        m_application->m_data_center->write("Simubot.step", m_step);

        GrosBot->isRelativToBot=m_ihm.ui.radioButton_robot_relative->isChecked();
        OldGrosBot->isRelativToBot=m_ihm.ui.radioButton_robot_relative->isChecked();
        OtherBot->isRelativToBot=m_ihm.ui.radioButton_robot_relative->isChecked();
        MiniBot->isRelativToBot=m_ihm.ui.radioButton_robot_relative->isChecked();

        setAndGetInRad=m_ihm.ui.radioButton_radian->isChecked();

        //coord init GrosBot
        qreal x_init=m_ihm.ui.lineEdit_X_init->value();
        qreal y_init=m_ihm.ui.lineEdit_Y_init->value();
        qreal theta_init=m_ihm.ui.lineEdit_Theta_init->value();

        //coord init MiniBot
        qreal x_init_2=m_ihm.ui.lineEdit_X_init_2->value();
        qreal y_init_2=m_ihm.ui.lineEdit_Y_init_2->value();
        qreal theta_init_2=m_ihm.ui.lineEdit_Theta_init_2->value();

        //Init des positions d'init sur l'affichage terrain et asservissement
        if (setAndGetInRad)
        {
            GrosBot->setAsservInit(m_ihm.ui.sB_X_init_asserv->value(),
                                   m_ihm.ui.sB_Y_init_asserv->value(),
                                   180*(m_ihm.ui.sB_Theta_init_asserv->value())/Pi);
            OtherBot->setAsservInit(0,0,0);
            MiniBot->setAsservInit(0,0,180*(m_ihm.ui.sB_Theta_init_asserv_2->value())/Pi);
            //MiniBot->setAsservInit(0,0,0);
            GrosBot->raz(x_init,y_init,normalizeAngleDeg(180*theta_init/Pi));
            OldGrosBot->raz(x_init,y_init,normalizeAngleDeg(180*theta_init/Pi));
            OtherBot->raz(equipeOther.x,equipeOther.y,normalizeAngleDeg(180*equipeOther.teta/Pi));
            MiniBot->raz(x_init_2,y_init_2,normalizeAngleDeg(180*theta_init_2/Pi));
            // Moteur cinematique : meme pose d'init, teta en radians (deja en rad ici).
            m_kinematic_engine.init(x_init,y_init,theta_init);
            // ETAPE 3bis : reconstructeur asserv interne cale sur le MEME repere asserv que
            // GrosBot (setAsservInit ci-dessus). setPosition_XYTeta reproduit le leurre d'init
            // du firmware ; sB_*_init_asserv sont les conditions initiales asserv (per-annee,
            // EEPROM). teta_asserv deja en radians dans cette branche.
            m_pose_reconstructor_interne.setPosition_XYTeta(m_ihm.ui.sB_X_init_asserv->value(),
                                                            m_ihm.ui.sB_Y_init_asserv->value(),
                                                            m_ihm.ui.sB_Theta_init_asserv->value());
            m_pose_reconstructor_interne.distance_roue_D=0.f; m_pose_reconstructor_interne.distance_roue_G=0.f;
            m_pose_reconstructor_interne.distance_roue_D_prec=0.f; m_pose_reconstructor_interne.distance_roue_G_prec=0.f;
            m_cumul_distance_D=0.f; m_cumul_distance_G=0.f;
        }
        else
        {
            GrosBot->setAsservInit(m_ihm.ui.sB_X_init_asserv->value(),
                                   m_ihm.ui.sB_Y_init_asserv->value(),
                                   m_ihm.ui.sB_Theta_init_asserv->value());
            OtherBot->setAsservInit(0,0,0);
            MiniBot->setAsservInit(0,0,m_ihm.ui.sB_Theta_init_asserv_2->value());
            GrosBot->raz(x_init,y_init,theta_init);
            OldGrosBot->raz(x_init,y_init,theta_init);
            OtherBot->raz(equipeOther.x,equipeOther.y,equipeOther.teta);
            MiniBot->raz(x_init_2,y_init_2,theta_init_2);
            // Moteur cinematique : meme pose d'init, teta converti deg -> rad.
            m_kinematic_engine.init(x_init,y_init,Pi*theta_init/180.0f);
            // ETAPE 3bis : reconstructeur asserv interne cale sur le MEME repere asserv que
            // GrosBot. teta_asserv en degres dans cette branche -> conversion en radians.
            m_pose_reconstructor_interne.setPosition_XYTeta(m_ihm.ui.sB_X_init_asserv->value(),
                                                            m_ihm.ui.sB_Y_init_asserv->value(),
                                                            Pi*m_ihm.ui.sB_Theta_init_asserv->value()/180.0f);
            m_pose_reconstructor_interne.distance_roue_D=0.f; m_pose_reconstructor_interne.distance_roue_G=0.f;
            m_pose_reconstructor_interne.distance_roue_D_prec=0.f; m_pose_reconstructor_interne.distance_roue_G_prec=0.f;
            m_cumul_distance_D=0.f; m_cumul_distance_G=0.f;
        }

        // Elements de jeu : crees une fois par setElementJeu() dans init() (rectangles
        // 15x5 / 5x15 en repere scene Qt, position initiale). Ce sont des caisses LIBRES,
        // poussables par le robot (etape 3, CCollisionEngine en mode mobile). Sur un raz on
        // les ramene a leur position de depart : remise a zero du deplacement cote moteur de
        // collisions, puis recalage des sprites (setPos(0,0) via refreshGameElementsView()).
        m_collision_engine.resetObstaclePoses();
        refreshGameElementsView();

        //init data manager 1er Robot
        qreal x_reel_init=GrosBot->getX();
        qreal y_reel_init=GrosBot->getY();
        qreal theta_reel_init=GrosBot->getTheta();

        m_application->m_data_center->write("PosX_robot", x_reel_init);
        m_application->m_data_center->write("PosY_robot", y_reel_init);

        if (setAndGetInRad)
            m_application->m_data_center->write("PosTeta_robot", theta_reel_init);
        else
            m_application->m_data_center->write("PosTeta_robot", normalizeAngleDeg(180*theta_reel_init/Pi));

        // Initialisation des clés de position simulée utilisées par les autres modules (ex. CBlockBotLab).
        // Sans ces écritures, x_pos/y_pos/teta_pos restent à 0 en mode TEST jusqu'au premier déplacement manuel.
        m_application->m_data_center->write("x_pos", x_reel_init);
        m_application->m_data_center->write("y_pos", y_reel_init);
        if (setAndGetInRad)
            m_application->m_data_center->write("teta_pos", theta_reel_init);
        else
            m_application->m_data_center->write("teta_pos", normalizeAngleDeg(180*theta_reel_init/Pi));

        m_ihm.ui.lcdNumber_x_terrain->display(GrosBot->getX_terrain());
        m_ihm.ui.lcdNumber_y_terrain->display(GrosBot->getY_terrain());

        //init data manager 2ème Robot
        qreal x_reel_init2=MiniBot->getX();
        qreal y_reel_init2=MiniBot->getY();
        qreal theta_reel_init2=MiniBot->getTheta();
        m_application->m_data_center->write("PosX_robot2", x_reel_init2);
        m_application->m_data_center->write("PosY_robot2", y_reel_init2);

        if (setAndGetInRad)
            m_application->m_data_center->write("PosTeta_robot2", theta_reel_init2);
        else
            m_application->m_data_center->write("PosTeta_robot2", normalizeAngleDeg(180*theta_reel_init2/Pi));

        m_application->m_data_center->write("Simubot.Init", false);
}
}

void CSimuBot::initEquipe(int equipe)
{
    Coord cGrosBot;
    Coord cMiniBot;

    if(equipe==EQUIPE1)
    {
        cGrosBot.x=equipe1_bot1.x;
        cGrosBot.y=equipe1_bot1.y;
        cGrosBot.teta=equipe1_bot1.teta;
        cGrosBot.ortho=equipe1_bot1.ortho;

        cMiniBot.x=equipe1_bot2.x;
        cMiniBot.y=equipe1_bot2.y;
        cMiniBot.teta=equipe1_bot2.teta;
        cMiniBot.ortho=equipe1_bot2.ortho;

        equipeOther.x=equipe2_bot1.x;
        equipeOther.y=equipe2_bot1.y;
        equipeOther.teta=M_PIf64;//equipe2_bot1.teta;
        equipeOther.ortho=equipe2_bot1.ortho;
    }
    else
    {
        cGrosBot.x=equipe2_bot1.x;
        cGrosBot.y=equipe2_bot1.y;
        cGrosBot.teta=equipe2_bot1.teta;
        cGrosBot.ortho=equipe2_bot1.ortho;

        cMiniBot.x=equipe2_bot2.x;
        cMiniBot.y=equipe2_bot2.y;
        cMiniBot.teta=equipe2_bot2.teta;
        cMiniBot.ortho=equipe2_bot2.ortho;

        equipeOther.x=equipe1_bot1.x;
        equipeOther.y=equipe1_bot1.y;
        equipeOther.teta=0.0;//equipe1_bot1.teta;
        equipeOther.ortho=equipe1_bot1.ortho;
    }

    m_ihm.ui.lineEdit_X_init->setValue(cGrosBot.x);
    m_ihm.ui.lineEdit_Y_init->setValue(cGrosBot.y);
    m_ihm.ui.lineEdit_Theta_init->setValue(cGrosBot.teta);

    m_ihm.ui.sB_Theta_init_asserv->setValue(iniTetaAsserv_bot1[equipe]);

    m_ihm.ui.lineEdit_X_init_2->setValue(cMiniBot.x);
    m_ihm.ui.lineEdit_Y_init_2->setValue(cMiniBot.y);
    m_ihm.ui.lineEdit_Theta_init_2->setValue(cMiniBot.teta);

    m_ihm.ui.sB_Theta_init_asserv_2->setValue(iniTetaAsserv_bot2[equipe]);

    //sens trigo appliqué au robot placé à gauche sur l'image repère orthogonal
    if(cGrosBot.ortho)
    {
        GrosBot->sensOrtho=1;
        OldGrosBot->sensOrtho=1;
        OtherBot->sensOrtho=-1;
        MiniBot->sensOrtho=1;
    }
    else
    {
        GrosBot->sensOrtho=-1;
        OldGrosBot->sensOrtho=-1;
        OtherBot->sensOrtho=1;
        MiniBot->sensOrtho=-1;
    }
}

void CSimuBot::changeEquipe(void)
{
    QObject *radioButton_couleur=QObject::sender();
    QString name_radio_button;
    if(radioButton_couleur)
        name_radio_button=radioButton_couleur->objectName();
    else
        name_radio_button="radioButton_couleur_1";


    if(name_radio_button.compare("radioButton_couleur_1")==0) //bleu
    {
        initEquipe(EQUIPE1);
        //qDebug() << "bleu";
    }
    else if(name_radio_button.compare("radioButton_couleur_2")==0) //jaune
    {
        initEquipe(EQUIPE2);
        //qDebug() << "jaune";
    }
    initView();
}

void CSimuBot::returnCapture_Theta()
{
    double new_angle=m_ihm.ui.lineEdit_theta->value();
    if(m_ihm.ui.lineEdit_theta->hasFocus())
    {
        if (setAndGetInRad)
        {
            m_ihm.ui.dial_rotation_bot->setValue((int)(10000*new_angle/Pi));
            emit displayAngle(new_angle);
        }
        else
        {
            m_ihm.ui.dial_rotation_bot->setValue((int)(10000*new_angle/180));
            emit displayAngle(Pi*normalizeAngleDeg(new_angle)/180);
        }
    }
}

/*!
 * \brief CSimuBot::changeMode gère les modes TEST, VISU et SIMU
 * \param iMode mode choisi
 *
 * # COMPORTEMENT
 * - rend effectif le mode choisi
 * - Active/Désactive des éléments de l'IHM selon le mode (par exemple il faut rendre impossible la manipulation du robot pendant le mode VISU ou
 * il est inutile d'afficher l'ancienne position du robot en dehors du mode TEST)
 */
void CSimuBot::changeMode(int iMode)
{
    modeVisu=iMode;

    switch (modeVisu) {
    case SIMUBOT::TEST:
        GrosBot->setFlag(QGraphicsItem::ItemIsMovable, true);
        GrosBot->setFlag(QGraphicsItem::ItemIsSelectable, true);
        GrosBot->setBrush(QBrush(QColor(255, 255,255, 255)));
        OldGrosBot->show();
        liaison_GrosBot->show();
        m_ihm.ui.lineEdit_x->setEnabled(true);
        m_ihm.ui.lineEdit_y->setEnabled(true);
        m_ihm.ui.lineEdit_theta->setEnabled(true);
        m_ihm.ui.checkBox_setSequence->setEnabled(true);

        m_ihm.ui.active_external_robot2->setEnabled(false);
        m_ihm.ui.active_external_robot2->setChecked(false);

        m_ihm.ui.pushButton_init->setEnabled(true);

        break;
    case SIMUBOT::VISU:
        // Le mode VISU n'est pas dedie a Simulia : la position vient du robot RS232 ou, a defaut,
        // de la simulation interne (HIL). Laisser "Enable Simulia" coche ferait attendre une source
        // de position qui ne viendra pas -> on decoche automatiquement.
        setSimuliaEnabled(false);

        m_ihm.ui.active_external_robot2->setEnabled(false);
        m_ihm.ui.active_external_robot2->setChecked(false);

        GrosBot->setFlag(QGraphicsItem::ItemIsMovable, false);
        GrosBot->setFlag(QGraphicsItem::ItemIsSelectable, false);
        GrosBot->setBrush(QBrush(QColor(153, 51,255, 255)));
        OldGrosBot->hide();
        liaison_GrosBot->hide();
        m_ihm.ui.lcdNumber_distance->display(0);
        m_ihm.ui.lcdNumber_angle->display(0);

        m_ihm.ui.lineEdit_x->clear();
        m_ihm.ui.lineEdit_y->clear();
        m_ihm.ui.lineEdit_theta->clear();
        m_ihm.ui.lineEdit_x->setEnabled(false);
        m_ihm.ui.lineEdit_y->setEnabled(false);
        m_ihm.ui.lineEdit_theta->setEnabled(false);
        m_ihm.ui.checkBox_setSequence->setChecked(false);
        m_ihm.ui.checkBox_setSequence->setEnabled(false);

        m_ihm.ui.pushButton_init->setEnabled(true);

        break;
    case SIMUBOT::SIMU:
        // Le mode SIMU est, lui, reellement dedie a Simulia : c'est la logique robot qui pilote la
        // simulation. On recoche automatiquement, symetriquement au decochage du mode VISU.
        setSimuliaEnabled(true);

        if(m_ihm.ui.ckhB_2Bot->isChecked())
            m_ihm.ui.active_external_robot2->setEnabled(true);
        GrosBot->setFlag(QGraphicsItem::ItemIsMovable, false);
        GrosBot->setFlag(QGraphicsItem::ItemIsSelectable, false);
        GrosBot->setBrush(QBrush(QColor(153, 51,255, 255)));
        OldGrosBot->hide();
        liaison_GrosBot->hide();
        m_ihm.ui.lcdNumber_distance->display(0);
        m_ihm.ui.lcdNumber_angle->display(0);

        m_ihm.ui.lineEdit_x->clear();
        m_ihm.ui.lineEdit_y->clear();
        m_ihm.ui.lineEdit_theta->clear();
        m_ihm.ui.lineEdit_x->setEnabled(false);
        m_ihm.ui.lineEdit_y->setEnabled(false);
        m_ihm.ui.lineEdit_theta->setEnabled(false);
        m_ihm.ui.checkBox_setSequence->setChecked(false);
        m_ihm.ui.checkBox_setSequence->setEnabled(false);

        m_ihm.ui.pushButton_init->setEnabled(false);

        break;
    default:
        break;
    }

    // Etape 6 : (re)ajuste la sim interne selon le nouveau mode (elle tourne en SIMU-sans-Simulia
    // ou en VISU robot deconnecte). Entrer/sortir de VISU demarre/arrete le cadenceur en consequence.
    syncInternalSim();
}

// _____________________________________________________________________
/*!
 * \brief CSimuBot::real_robot_position_changed slot appelé quand l'une des coordonnées du robot issues du datamanager change
 *
 * # COMPORTEMENT
 *  Permet au mode VISU et SIMU de faire évoluer le robot dans le simulateur selon les vrais évolutions des coordonnées
 */
void CSimuBot::real_robot_position_changed()
{
    //Si on est en mode VISU ou SIMU
    //attention en mode TEST la modification de la position du robot change x_pos et y_pos dans le data center
    if ((modeVisu==SIMUBOT::VISU) || (modeVisu==SIMUBOT::SIMU))
    {
        //on récupère les nouvelles coordonnées du robot
        float pos_x = m_application->m_data_center->getData("x_pos")->read().toFloat();
        float pos_y = m_application->m_data_center->getData("y_pos")->read().toFloat();
        float teta_pos = m_application->m_data_center->getData("teta_pos")->read().toFloat();
        //on récupère les nouvelles coordonnées du deuxième robot
        float pos_x2 = m_application->m_data_center->getData("x_pos2")->read().toFloat();
        float pos_y2 = m_application->m_data_center->getData("y_pos2")->read().toFloat();
        float teta_pos2 = m_application->m_data_center->getData("teta_pos2")->read().toFloat();

        //mise à jour des coordonnées dans simubot
        emit displayCoord(pos_x, pos_y);
        emit displayAngle(teta_pos);
        //mise à jour des coordonnées dans simubot
        emit displayCoord2(pos_x2, pos_y2);
        emit displayAngle2(teta_pos2);
    }
}

/*!
 * \brief CSimuBot::zoom pour zoomer ou dezoomer l'affichage du terrain
 * \param value
 */
void CSimuBot::zoom(int value)
{
    float factor=value*1.0;
    m_ihm.simuView->resetTransform();
    m_ihm.simuView->scale(factor,factor);
    m_ihm.simuView->centerOn(QPointF(151,101));
}

void CSimuBot::returnCapture_XY()
{
    QString lineEdit_coord_name=QObject::sender()->objectName();

    if (lineEdit_coord_name.compare(m_ihm.ui.lineEdit_x->objectName())==0)
        if (m_ihm.ui.lineEdit_x->hasFocus())
            emit displayCoord(m_ihm.ui.lineEdit_x->value(),5000);

    if (lineEdit_coord_name.compare(m_ihm.ui.lineEdit_y->objectName())==0)
        if (m_ihm.ui.lineEdit_y->hasFocus())
            emit displayCoord(5000,m_ihm.ui.lineEdit_y->value());
}

void CSimuBot::slot_dial_turned(void)
{
    double angle_percent=m_ihm.ui.dial_rotation_bot->value();
    double new_angle;

    if (setAndGetInRad)
    {
        new_angle=Pi*angle_percent/10000;

        emit displayAngle(new_angle);
    }
    else
    {
        new_angle=180*angle_percent/10000;
        emit displayAngle(Pi*normalizeAngleDeg(new_angle)/180);
    }
}

void CSimuBot::slot_clearPath(void)
{
    //effacement de la dernière trajectoire
    if (!(evitement.isEmpty()))
            for(int k=0;k<evitement.size();k++ ){
                //QGraphicsLineItem
                terrain->removeItem(evitement.at(k));
            }
    evitement.clear();
}

void CSimuBot::catchDoubleClick()
{
    if(m_ihm.ui.checkBox_setSequence->isChecked())
    {
        double angle    = m_ihm.ui.lcdNumber_angle->value();
        double distance = m_ihm.ui.lcdNumber_distance->value();
        // Normalisation : le signal setSequence transmet toujours l'angle en degrés,
        // quel que soit le réglage d'affichage du widget groupBox_angles.
        if (setAndGetInRad)
            angle = angle * 180.0 / Pi;
        emit setSequence(angle, distance);
    }
}

/*!
 * \brief CSimuBot::estimateEnvironmentVariables
 *
 * # COMPORTEMENT
 * Calcule toutes les interactions entre le robot et l'environnement
 * - capteurs US
 * - blocage terrain
 */

void CSimuBot::estimate_Environment_Interactions()
{
    //estimation de l'environnement US
    //récupération des différentes coordonnées (graphique et réelles) de l'adversaire
    Coord cOtherBot(OtherBot->getX_terrain(),OtherBot->getY_terrain(),OtherBot->getTheta(),(OtherBot->sensOrtho>0));

    //récupération des différentes coordonnées (graphique et réelles) de notre premier robot
    Coord cGrosBot(GrosBot->getX_terrain(),GrosBot->getY_terrain(),GrosBot->getTheta(),(GrosBot->sensOrtho>0));
    //qreal x_bot_graphic=GrosBot->getX_terrain();
    //qreal theta_bot=GrosBot->getTheta();

    //récupération des différentes coordonnées (graphique et réelles) de notre premier robot
    Coord cMiniBot(MiniBot->getX_terrain(),MiniBot->getY_terrain(),MiniBot->getTheta(),(MiniBot->sensOrtho>0));

    //détection GrosBot
    //de l'adversaire
    float capteurs1[4];
	float lidar1[2];
	for(int i=0;i<2;i++)
            lidar1[i]=0.;
    getUSDistance(cGrosBot,cOtherBot,capteurs1,lidar1);
    //de minibot
    float capteurs2[4];
	float lidar2[2];
	for(int i=0;i<2;i++)
            lidar2[i]=0.;
    if(twoBotsEnabled)
	{
        getUSDistance(cGrosBot,cMiniBot,capteurs2,lidar2);
	}
    else
    {
        for(int i=0;i<4;i++)
            capteurs2[i]=99;
    }
    //synthèse
	if(lidar1[0]>0.)
	{
		m_application->m_data_center->write("Lidar.Obstacle1.Distance", lidar1[0]);
		m_application->m_data_center->write("Lidar.Obstacle1.Angle", lidar1[1]);
	}
		if(lidar2[0]>0.)
	{
		m_application->m_data_center->write("Lidar.Obstacle2.Distance", lidar2[0]);
		m_application->m_data_center->write("Lidar.Obstacle2.Angle", lidar2[1]);
	}
	//TODO ajouter la possibilité de déconnecter le lidar (simulation panne)
	/*
	typedef enum {
        LIDAR_OK    = 0,
        LIDAR_DISCONNECTED,
        LIDAR_ERROR
    }eLidarStatus;
	*/
    if(m_ihm.ui.checkBox_DisconnectLidar->isChecked())
        m_application->m_data_center->write("Lidar.Status", 1);
    else
         m_application->m_data_center->write("Lidar.Status", 0);

    float capteurs3[4];
    for(int i=0;i<4;i++)
    {
        if(capteurs1[i]<=capteurs2[i])
            capteurs3[i]=capteurs1[i];
        else
            capteurs3[i]=capteurs2[i];
    }

    m_application->m_data_center->write("Simubot.Telemetres.AVG", capteurs3[AVG]);
    m_application->m_data_center->write("Simubot.Telemetres.AVD", capteurs3[AVD]);
    m_application->m_data_center->write("Simubot.Telemetres.ARG", capteurs3[ARG]);
    m_application->m_data_center->write("Simubot.Telemetres.ARD", capteurs3[ARD]);

    //détection MiniBot
    for(int i=0;i<2;i++)
    {
            lidar1[i]=0.;
            lidar2[i]=0.;
    }
    //de l'adversaire
    getUSDistance(cMiniBot,cOtherBot,capteurs1,lidar1);
    //de grosbot
    getUSDistance(cMiniBot,cGrosBot,capteurs2,lidar2);
    //synthèse
    for(int i=0;i<4;i++)
    {
        if(capteurs1[i]<=capteurs2[i])
            capteurs3[i]=capteurs1[i];
        else
            capteurs3[i]=capteurs2[i];
    }
    //pour l'instant pas de mise à jour du lidar pour MiniBot bien que l'info soit calculée
    m_application->m_data_center->write("Simubot.Telemetres.AVG2", capteurs3[AVG]);
    m_application->m_data_center->write("Simubot.Telemetres.AVD2", capteurs3[AVD]);
    m_application->m_data_center->write("Simubot.Telemetres.ARG2", capteurs3[ARG]);
    m_application->m_data_center->write("Simubot.Telemetres.ARD2", capteurs3[ARD]);

    /*if((capteurs3[AVG]<30)||(capteurs3[ARG]<30)||(capteurs3[AVD]<30)||(capteurs3[ARD]<30))
    {
    qDebug() << "AVG "<<capteurs3[AVG] <<" (" << capteurs1[AVG] << capteurs2[AVG] <<") - AVD "<<capteurs3[AVD]<<" (" << capteurs1[AVD] << capteurs2[AVD] <<")";
    qDebug() << "ARG "<<capteurs3[ARG] <<" (" << capteurs1[ARG] << capteurs2[ARG] <<") - ARD "<<capteurs3[ARD]<<" (" << capteurs1[ARD] << capteurs2[ARD] <<")";
    qDebug() << "------------------------------------------------------";
    }*/




    //estimation de blocage sur le terrain
    //l'idée est d'interdire le mouvement d'un côté si un blocage est détecté à gauche ou a droite
    /*if(m_ihm.ui.checkBox_enableBlocking->isChecked())
    {
        float cmde_MotG = m_application->m_data_center->getData("Cde_MotG")->read().toFloat();
        float cmde_MotD = m_application->m_data_center->getData("Cde_MotD")->read().toFloat();
        bool blocage_G=false;
        bool blocage_D=false;

        //conversion en degre
        double teta_deg= theta_bot*180/Pi; //angle en degré
        //modulo 360 degré
        while (teta_deg < 0)
            teta_deg += 360;
        while (teta_deg > 360)
            teta_deg -= 360;
        //collision avec la bordure en x=0
        double x_limit=fabs(23.5*sin(theta_bot+(Pi/4)));

        if(x_bot_graphic<=x_limit)
        {
            if(((teta_deg<=1)||(teta_deg>=359))&&(cmde_MotG<0)&&(cmde_MotD<0))
            {
                blocage_G=true;
                blocage_D=true;
            }
            else if (((teta_deg>1)&&(teta_deg<90))&&(cmde_MotG<0))
            {
                blocage_G=true;
            }
            else if (((teta_deg>270)&&(teta_deg<359))&&(cmde_MotD<0))
            {
                blocage_D=true;
            }
        }

        m_application->m_data_center->write("Simubot.blocage.gauche", blocage_G);
        m_application->m_data_center->write("Simubot.blocage.droite", blocage_D);

    }*/

}

/*!
 * \brief CSimuBot::playOther
 *
 * # COMPORTEMENT
 * Lance le déplacement d'un robot adverse (aka OtherBot dans le code)
 * seulement si sa vitesse définie dans l'ihm est non nulle.
 * Cette fonction execute également le premier ordre de déplacement défini
 * dans la liste de l'onglet Simu
 */

void CSimuBot::playOther()
{
    //récupération de la vitesse définie dans l'ihm
    double speedOther=m_ihm.ui.doubleSpinBox_speed->value();

    //déplacement effectif si la vitesse est non nulle
    if(speedOther>0)
    {
        currentIndex=0;

        //récupération du premier déplacement défini dans la liste
        QTableWidgetItem * widget_x=m_ihm.ui.tableWidget->item(currentIndex,0);
        QTableWidgetItem * widget_y=m_ihm.ui.tableWidget->item(currentIndex,1);
        QTableWidgetItem * widget_teta=m_ihm.ui.tableWidget->item(currentIndex,2);

        //Si toutes les données sont bien définies
        if((widget_x)&&(widget_y)&&(widget_teta))
        {
            //extraction des données
            QString str_x_record=m_ihm.ui.tableWidget->item(currentIndex,0)->text();
            QString str_y_record=m_ihm.ui.tableWidget->item(currentIndex,1)->text();
            QString str_teta_record=m_ihm.ui.tableWidget->item(currentIndex,2)->text();

            //Si les données sont valides
            if((!str_x_record.isEmpty())&&(!str_y_record.isEmpty())&&(!str_teta_record.isEmpty()))
            {
                m_ihm.ui.pB_playOther->setDisabled(true);
                m_ihm.ui.doubleSpinBox_speed->setDisabled(true);
                m_ihm.ui.pB_stopOther->setDisabled(false);

                OtherBot->setSpeed(speedOther);
                OtherBot->startInternalAsserv();
                float x_record=str_x_record.toFloat();
                float y_record=str_y_record.toFloat();
                //float teta_record=str_teta_record.toFloat();
#ifdef DEBUG_OTHER
                qDebug() << "[SimuBot] Demande n°1 de déplacement du robot adverse ("<<x_record<<","<<y_record<<") à la vitesse "<<speedOther;
#endif
                OtherBot->display_XY(x_record,y_record);
                isStarted=true;
            }
        }
    }
}

void CSimuBot::stopOther()
{
#ifdef DEBUG_OTHER
    qDebug() << "[SimuBot] Fin de déplacement du robot adverse et désactivation de son asservissement interne";
#endif
    isStarted=false;
    currentIndex=0;
    OtherBot->setSpeed(0.0);
    OtherBot->stopInternalAsserv();

    m_ihm.ui.pB_playOther->setDisabled(false);
    m_ihm.ui.doubleSpinBox_speed->setDisabled(false);
    m_ihm.ui.pB_stopOther->setDisabled(true);
}

void CSimuBot::nextStepOther()
{
    //Si le robot adverse est lancé (cad en déplacement)
    if (isStarted)
    {
        //Si le robot adverse est en convergence distance
        if(OtherBot->isConvergenceXY)
        {
            //on récupère les prochaines consignes de déplacement (limitation à 10 consignes)
            currentIndex++;
            if(currentIndex<=10)
            {
                //récupération du prochain déplacement défini dans la liste
                QTableWidgetItem * widget_x=m_ihm.ui.tableWidget->item(currentIndex,0);
                QTableWidgetItem * widget_y=m_ihm.ui.tableWidget->item(currentIndex,1);
                QTableWidgetItem * widget_teta=m_ihm.ui.tableWidget->item(currentIndex,2);

                //Si toutes les données sont bien définies
                if((widget_x)&&(widget_y)&&(widget_teta))
                {
                    //extraction des données
                    QString str_x_record=m_ihm.ui.tableWidget->item(currentIndex,0)->text();
                    QString str_y_record=m_ihm.ui.tableWidget->item(currentIndex,1)->text();
                    QString str_teta_record=m_ihm.ui.tableWidget->item(currentIndex,2)->text();

                    //Si toutes les données sont valides
                    if((!str_x_record.isEmpty())&&(!str_y_record.isEmpty())&&(!str_teta_record.isEmpty()))
                    {
                        float x_record=str_x_record.toFloat();
                        float y_record=str_y_record.toFloat();
                        //float teta_record=str_teta_record.toFloat();
#ifdef DEBUG_OTHER
                        qDebug() << "[SimuBot] Demande n°"<<currentIndex+1<<" de déplacement du robot adverse ("<<x_record<<","<<y_record<<")";
#endif
                        OtherBot->display_XY(x_record,y_record);
                    }
                    //si une des données n'est pas valide on arrête le robot adverse en invalidant son asservissement interne
                    else
                        stopOther();
                }
                //si une des données n'est pas définie on arrête le robot adverse en invalidant son asservissement interne
                else
                {
                    stopOther();
                }
            }
            //s'il y a eu plus de 10 consignes de déplacement on arrête le robot adverse en invalidant son asservissement interne
            else
            {
                stopOther();
            }
        }
        //si le robot n'est pas lancé (en déplacement) on désactive son asservissement interne (par sécurité)
        else
            OtherBot->stepInternalAsserv();
    }
}

void CSimuBot::addStepOther(double x, double y, double teta, int row)
{
    QTableWidgetItem * qTbW_x=new QTableWidgetItem();
    QTableWidgetItem * qTbW_y=new QTableWidgetItem();
    QTableWidgetItem * qTbW_teta=new QTableWidgetItem();

    QString str_x, str_y, str_teta;
    qTbW_x->setText(str_x.setNum(x));
    qTbW_y->setText(str_y.setNum(y));
    qTbW_teta->setText(str_teta.setNum(teta));

    m_ihm.ui.tableWidget->setItem(row,0,qTbW_x);
    m_ihm.ui.tableWidget->setItem(row,1,qTbW_y);
    m_ihm.ui.tableWidget->setItem(row,2,qTbW_teta);
}

void CSimuBot::enableMoveOther(int state)
{
    if(state==Qt::Checked)
    {
        m_ihm.ui.pB_playOther->setDisabled(false);
        m_ihm.ui.doubleSpinBox_speed->setDisabled(false);
        m_ihm.ui.chkBox_syncMeanBot->setDisabled(false);
    }
    else
    {
        stopOther();
        m_ihm.ui.pB_playOther->setDisabled(true);
        m_ihm.ui.doubleSpinBox_speed->setDisabled(true);
        m_ihm.ui.chkBox_syncMeanBot->setDisabled(true);
    }
}

void CSimuBot::syncMove(bool activated)
{
    int cmdTirette=m_application->m_data_center->read("Capteurs.Tirette").toInt();
    if(cmdTirette==1)
    {
        if(activated&&(m_ihm.ui.chkBox_enableMoveOther->isChecked())&&(m_ihm.ui.chkBox_syncMeanBot->isChecked()))
        {
            /*qDebug() << "\n\ntirette old "<<isStarted_old << "et new "<<isStarted<<"\n\n";
            if(isStarted == !isStarted_old)
                qDebug()<< "[SimuBot] Valeur tirette modifiée de " << isStarted_old << " à "<<isStarted;*/

            if(!isStarted) //front montant tirette
           {
                qDebug() << "[Simubot] Tirette levée!";
                playOther();
            }
        }
        /*else
            m_simulia_Enabled=true;*/
    }
}

/*!
 * \brief CSimuBot::polygonFromJson
 * Convertit un objet { "sommets_cm": [[x,y],...] } (repere robot local, y vers le HAUT, cm) en
 * QPolygonF en repere scene Qt (y vers le bas) : QPointF(x, -y) — meme convention que l'ancien
 * getForm. Renvoie un polygone vide si la liste est absente/vide (l'appelant garde son repli).
 */
QPolygonF CSimuBot::polygonFromJson(const QJsonObject& obj) const
{
    QPolygonF poly;
    const QJsonArray sommets = obj.value("sommets_cm").toArray();
    for (int s = 0; s < sommets.size(); ++s)
    {
        const QJsonArray p = sommets.at(s).toArray();
        if (p.size() >= 2)
            poly << QPointF(p.at(0).toDouble(), -p.at(1).toDouble());
    }
    return poly;
}

/*!
 * \brief CSimuBot::loadRobotFromJson
 * Charge la forme (polygone) du GrosBot et du MiniBot depuis un fichier JSON dedie (etape 5
 * migration v2). Remplace les anciennes cles EEPROM "polygon"/"polygon2". Si le fichier est
 * absent, un defaut est cree (writeDefaultRobotFile, a l'instar de terrain.json / EEPROM.ini).
 * Repere du fichier : robot local (front +x), y vers le HAUT, cm -> QPointF(x,-y) pour la scene.
 * Renvoie false et laisse une forme de repli (octogone) si le fichier reste absent/invalide.
 */
bool CSimuBot::loadRobotFromJson(const QString& path, QPolygonF& gros, QPolygonF& mini)
{
    // Forme de repli (octogone ~30x30) : identique a l'ancien defaut de getForm quand aucune
    // forme n'etait definie. Sert si le JSON reste illisible malgre l'auto-creation.
    QPolygonF fallback;
    fallback << QPointF(10,-15) << QPointF(-10,-15) << QPointF(-15,-10) << QPointF(-15,10)
             << QPointF(-10,15) << QPointF(10,15) << QPointF(15,10) << QPointF(15,-10);
    gros = fallback;
    mini = fallback;

    // Absent : on cree un fichier par defaut (octogone GrosBot + MiniBot), a l'instar d'EEPROM.ini.
    if (!QFile::exists(path))
        writeDefaultRobotFile(path);

    QFile file(path);
    if (!file.open(QIODevice::ReadOnly))
    {
        m_application->m_print_view->print_error(this, "SimuBot: robot JSON introuvable: " + path);
        return false;
    }
    QJsonParseError perr;
    const QJsonDocument doc = QJsonDocument::fromJson(file.readAll(), &perr);
    file.close();
    if (perr.error != QJsonParseError::NoError || !doc.isObject())
    {
        m_application->m_print_view->print_error(this, "SimuBot: robot JSON invalide: " + perr.errorString());
        return false;
    }
    const QJsonObject root = doc.object();
    // Chaque robot est un objet { "sommets_cm": [[x,y],...] }. Absent/vide -> on garde le repli.
    const QPolygonF g = polygonFromJson(root.value("grosbot").toObject());
    const QPolygonF m = polygonFromJson(root.value("minibot").toObject());
    if (!g.isEmpty()) gros = g;
    if (!m.isEmpty()) mini = m;
    return true;
}

/*!
 * \brief CSimuBot::writeDefaultRobotFile
 * Ecrit un fichier robot JSON par defaut si absent (a l'instar d'EEPROM.ini / terrain.json) : la
 * forme par defaut est l'octogone ~30x30 (ancien repli de getForm), pour GrosBot et MiniBot.
 * L'utilisateur ajuste ensuite les sommets (ou depose le fichier de son robot).
 */
void CSimuBot::writeDefaultRobotFile(const QString& path)
{
    QFileInfo fi(path);
    QDir().mkpath(fi.absolutePath()); // cree le dossier Config au besoin
    QFile f(path);
    if (!f.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        m_application->m_print_view->print_error(this, "SimuBot: impossible de creer le robot par defaut: " + path);
        return;
    }
    static const char* DEFAULT_ROBOT_JSON =
R"JSON({
  "version": 2026,
  "_commentaire": "Forme(s) du/des robot(s) SimuBot. Repere ROBOT LOCAL : origine au centre, front vers +x, y vers le HAUT, en cm. 'sommets_cm' : polygone convexe (sens trigo) servant a la fois au sprite et a la collision SAT. 'grosbot' = robot principal, 'minibot' = second robot. Forme par defaut : octogone ~30x30.",
  "grosbot": { "sommets_cm": [[10,15],[-10,15],[-15,10],[-15,-10],[-10,-15],[10,-15],[15,-10],[15,10]] },
  "minibot": { "sommets_cm": [[10,15],[-10,15],[-15,10],[-15,-10],[-10,-15],[10,-15],[15,-10],[15,10]] }
}
)JSON";
    f.write(DEFAULT_ROBOT_JSON);
    f.close();
    m_application->m_print_view->print_info(this, "SimuBot: robot par defaut cree: " + path);
}

void CSimuBot::enableTwoBots(int state)
{
    if(state==Qt::Checked)
    {
        MiniBot->setVisible(true);
        twoBotsEnabled=true;
        m_ihm.ui.groupBox_MiniBot->setEnabled(true);
        if(modeVisu==SIMU)
        {
            m_ihm.ui.active_external_robot2->setEnabled(true);
            // bot2 etait simule par Box2D (m_physical_engine.activateBot2). Box2D retire
            // (etape 3) : bot2 est desormais pilote par une instance Simulia externe
            // (cf. rapport_simubot.md etape 2). Reactivation locale reportee (etape 6).
        }
    }
    else
    {
        MiniBot->setVisible(false);
        twoBotsEnabled=false;
        m_ihm.ui.groupBox_MiniBot->setEnabled(false);
        m_ihm.ui.active_external_robot2->setEnabled(false);
        m_ihm.ui.active_external_robot2->setChecked(false);
    }
}

void CSimuBot::getUSDistance(Coord bot, Coord obstacle, float capteurs[], float lidar[])
{
    //estimation de l'environnement US
    //récupération des différentes coordonnées (graphique et réelles) de l'adversaire
    float x_other_graphic=obstacle.x;
    float y_other_graphic=obstacle.y;

    //récupération des différentes coordonnées (graphique et réelles) de notre robot
    float x_bot_graphic=bot.x;
    float y_bot_graphic=bot.y;
    float theta_bot=bot.teta;
    bool sensOrtho_bot=bot.ortho;

    //on calcule la distance entre l'ancienne et la nouvelle position
    float distanceAdversaire = sqrt(pow((x_other_graphic-x_bot_graphic),2)+pow((y_other_graphic-y_bot_graphic),2));
    float angleAdversaire=0.0;

    //on calcule l'angle entre les 2 robots
    if (x_other_graphic==x_bot_graphic)
    {
        if (y_other_graphic>y_bot_graphic)
            angleAdversaire=Pi/2;
        else if (y_other_graphic==y_bot_graphic)
            angleAdversaire=0;
        else if (y_other_graphic<y_bot_graphic)
            angleAdversaire=-Pi/2;
    }
    else if (x_other_graphic>x_bot_graphic)
    {
        angleAdversaire=atan((y_other_graphic-y_bot_graphic)/(x_other_graphic-x_bot_graphic));
    }
    else if (x_other_graphic<x_bot_graphic)
    {
        if (y_other_graphic>y_bot_graphic)
            angleAdversaire=atan((y_other_graphic-y_bot_graphic)/(x_other_graphic-x_bot_graphic))+Pi;
        else if (y_other_graphic==y_bot_graphic)
            angleAdversaire=Pi;
        else if (y_other_graphic<y_bot_graphic)
            angleAdversaire=atan((y_other_graphic-y_bot_graphic)/(x_other_graphic-x_bot_graphic))-Pi;
    }

    double arad=angleAdversaire-theta_bot; //angle en radian
    if(!sensOrtho_bot)
        arad=arad+Pi;
    double adeg= arad*180/Pi; //angle en degré
	double adeg2=adeg;
    //modulo 360
    while (adeg < 0)
        adeg += 360;
    while (adeg > 360)
        adeg -= 360;
	//modulo 360 entre -180 et 180
    while (adeg2 < -180)
        adeg2 += 360;
    while (adeg2 > 180)
        adeg2 -= 360;
	//mise à jour lidar
    lidar[0]=distanceAdversaire*10.;
	lidar[1]=adeg2;
    /*
    //Affichage des nouvelles valeurs position, angle du robot
    qDebug() << "coord graphique: G(" <<x_prim_graphic <<","<<y_prim_graphic<<")\tAd("<<x_graphic<<","<<y_graphic<<")";
    qDebug() << "coord réelles: G(" <<x_prim_view <<","<<y_prim_view<<")\tAd("<<x_view<<","<<y_view<<")";
    qDebug() << "distance : " << distanceAdversaire;
    qDebug() << "angle : " << adeg;
    */
    distanceAdversaire=distanceAdversaire-15.0; //prise en compte du diamètre moyen du robot adverse
    capteurs[AVG]=99;
    capteurs[AVD]=99;
    capteurs[ARG]=99;
    capteurs[ARD]=99;
    //qDebug() << distanceAdversaire << "," << adeg;
    if(distanceAdversaire<=120)
    {
        if(((adeg>=0)&&(adeg<45))||((adeg>340)&&(adeg<=360))) //robot adverse devant à gauche
        {
            capteurs[AVG]=sqrt(pow((distanceAdversaire*cos(arad)-12),2)+pow((distanceAdversaire*sin(arad)-16),2));
        }
        if(((adeg>315)&&(adeg<=360))||((adeg>=0)&&(adeg<20))) //robot adverse devant à droite
        {
            capteurs[AVD]=sqrt(pow((distanceAdversaire*cos(arad)-12),2)+pow((distanceAdversaire*sin(arad)+16),2));
        }
        if((adeg>135)&&(adeg<=200)) //robot adverse arrière à gauche
        {
            capteurs[ARG]=sqrt(pow((distanceAdversaire*cos(arad)+12),2)+pow((distanceAdversaire*sin(arad)-16),2));
        }
        if((adeg>=160)&&(adeg<225)) //robot adverse arrière à droite
        {
            capteurs[ARD]=sqrt(pow((distanceAdversaire*cos(arad)+12),2)+pow((distanceAdversaire*sin(arad)+16),2));
        }
    }
}

/*!
 * \brief CSimuBot::on_active_external_robot2
 * \param state
 * Gestion du second robot dans Simubot
 */
void CSimuBot::on_active_external_robot2(bool state)
{
    if (state) {
          QString host_external_robot2;
          int port_external_robot2;
          host_external_robot2 = m_application->m_eeprom->read(getName(), "host_external_robot2", QVariant("127.0.0.1")).toString();
          port_external_robot2 = m_application->m_eeprom->read(getName(), "port_external_robot2", QVariant(1234)).toInt();
          bool connected = m_external_controler_client_robot2.open((char*)host_external_robot2.toStdString().c_str(), port_external_robot2);
          if (connected) {
              m_connected_host=true;
              m_external_controler_client_robot2.writeData("Simubot.step", m_step2);
              m_application->m_print_view->print_info(this, QString("Connecte au robot 2 externe %1 / port %2").arg(host_external_robot2).arg(port_external_robot2));
              MiniBot->setFlag(QGraphicsItem::ItemIsMovable, false);
              MiniBot->setFlag(QGraphicsItem::ItemIsSelectable, false);
              MiniBot->setBrush(QBrush(QColor(153, 51,255, 255)));
          }
          else
          {
              m_connected_host=false;
              m_application->m_print_view->print_error(this, QString("Impossible de se connecter au client %1 / port %2").arg(host_external_robot2).arg(port_external_robot2));
              m_ihm.ui.active_external_robot2->setChecked(false);
          }
    }
    else {
        //m_timer_external_robot2.stop();
        if(m_connected_host)
        {
            m_external_controler_client_robot2.close();
        }
        m_connected_host=false;
        MiniBot->setFlag(QGraphicsItem::ItemIsMovable, true);
        MiniBot->setFlag(QGraphicsItem::ItemIsSelectable, true);
        MiniBot->setBrush(QBrush(QColor(255, 255,255, 255)));
    }
}

/*!
 * \brief CSimuBot::setElementJeu
 * \param x
 * \param y
 * \param Color
 * \return
 * Fonction de positionnement des éléments de jeu
 */
QGraphicsRectItem * CSimuBot::setElementJeu(float x, float y, int Color, bool vertical)
{
    //QGraphicsEllipseItem* element=new QGraphicsEllipseItem(x-3.65,-(y+3.65),7.3,7.3);
    QGraphicsRectItem* element=new QGraphicsRectItem();
    float w = vertical ? 5.0f : 15.0f;
    float h = vertical ? 15.0f : 5.0f;
    element->setRect(QRectF(x,y,w,h));
    // Origine de transformation = centre du rectangle (en coordonnees item) : la rotation
    // d'un element pousse (cf. refreshGameElementsView) se fait autour de son propre centre,
    // coherent avec la rotation autour du centroide cote CCollisionEngine.
    element->setTransformOriginPoint(x + w/2.0f, y + h/2.0f);
    //QPolygonF element_shape;
    /*element_shape << QPointF(7.5*cos(M_PI/6),7.5*sin(M_PI/6)) << QPointF(0,7.5);
    element_shape << QPointF(7.5*cos(5*M_PI/6),7.5*sin(5*M_PI/6)) << QPointF(7.5*cos(-5*M_PI/6),7.5*sin(-5*M_PI/6));
    element_shape << QPointF(0,-7.5) << QPointF(7.5*cos(-M_PI/6),7.5*sin(-M_PI/6));
    element_shape << QPointF(7.5*cos(M_PI/6),7.5*sin(M_PI/6));
    QGraphicsPolygonItem* element=new QGraphicsPolygonItem(element_shape);*/
    //element->setPos(x,-y);//
    if(Color==Qt::cyan)
        element->setBrush(QBrush(QColor(255,0,255, 255)));
    if(Color==Qt::blue)
        element->setBrush(QBrush(QColor(0,0,255, 255)));
    if(Color==Qt::green)
        element->setBrush(QBrush(QColor(0,255,0, 255)));
    if(Color==Qt::red)
        element->setBrush(QBrush(QColor(139,69,19, 255)));
    if(Color==Qt::yellow)
        element->setBrush(QBrush(QColor(255,255,0, 255)));
    if(Color==Qt::gray)
        element->setBrush(QBrush(QColor(128,128,128, 255)));

    terrain->addItem(element);

    return element;
}

/*!
 * \brief CSimuBot::refreshGameElementsView
 * Repercute dans la scene Qt le deplacement des caisses de noisettes poussees par le robot.
 * Le moteur de collisions (CCollisionEngine) deplace les elements mobiles en repere terrain ;
 * on lit ce deplacement (dx, dy) par id et on translate le sprite correspondant. Conversion
 * repere : terrain (y vers le haut) -> scene Qt (y vers le bas), d'ou setPos(dx, -dy). Les
 * sprites ont ete crees a leur position initiale (pos = (0,0)), setPos applique l'offset.
 */
void CSimuBot::refreshGameElementsView()
{
    for(int id=0; id<elementsJeu.size(); id++)
    {
        // Un id peut porter un sprite nul (type d'element JSON inconnu ignore, cf.
        // loadTerrainFromJson) : on preserve l'alignement id<->index, on saute l'affichage.
        if(elementsJeu[id]==nullptr) continue;
        float dx=0.0f, dy=0.0f, dteta=0.0f;
        if(m_collision_engine.getObstacleDisplacement(id, dx, dy, dteta))
        {
            // Translation : terrain (y vers le haut) -> scene Qt (y vers le bas).
            elementsJeu[id]->setPos(dx, -dy);
            // Rotation : teta terrain (sens trigo) -> rotation scene Qt. La scene a l'axe y
            // inverse, donc une rotation trigo correspond a setRotation(-teta) en degres
            // (meme convention que graphicElement : setRotation(360 - teta_deg)).
            elementsJeu[id]->setRotation(-dteta * 180.0f / Pi);
        }
    }
}

/*!
 * \brief CSimuBot::colorFromName
 * Convertit un nom de couleur JSON en QColor. Reprend la convention historique de setElementJeu
 * (rouge = marron des caisses). Defaut : gris.
 */
QColor CSimuBot::colorFromName(const QString& name) const
{
    const QString nm = name.toLower();
    if (nm == "jaune")                   return QColor(255,255,0);
    if (nm == "bleu")                    return QColor(0,0,255);
    if (nm == "rouge")                   return QColor(139,69,19); // marron (convention Qt::red existante)
    if (nm == "vert")                    return QColor(0,255,0);
    if (nm == "magenta" || nm == "cyan") return QColor(255,0,255);
    if (nm == "gris")                    return QColor(128,128,128);
    return QColor(128,128,128); // defaut
}

/*!
 * \brief CSimuBot::clearGameElements
 * Detruit les sprites d'elements de jeu (retrait de la scene + delete) et vide elementsJeu.
 * Utilise avant chaque (re)chargement du terrain pour repartir propre.
 */
void CSimuBot::clearGameElements()
{
    for (int i = 0; i < elementsJeu.size(); ++i)
    {
        if (elementsJeu[i] != nullptr)
        {
            terrain->removeItem(elementsJeu[i]);
            delete elementsJeu[i];
        }
    }
    elementsJeu.clear();
}

/*!
 * \brief CSimuBot::loadTerrainFromJson
 * Charge depuis un fichier JSON : les dimensions du terrain, le decor FIXE (estrades, tasseaux...
 * = obstacles comme les bordures) et les elements de JEU (poussables si mobile=true). Cree pour
 * chaque entree son sprite Qt ET son obstacle de collision. Si le fichier est absent, un contenu
 * par defaut est cree (comme EEPROM.ini). Renvoie false si le fichier reste absent/invalide.
 */
bool CSimuBot::loadTerrainFromJson(const QString& path)
{
    m_terrain_json_path = path;
    // Absent : on cree un fichier par defaut (terrain 300x200 + regles en commentaire), a
    // l'instar de EEPROM.ini. L'utilisateur l'edite ensuite (ou depose le fichier de l'annee).
    if (!QFile::exists(path))
        writeDefaultTerrainFile(path);

    QFile file(path);
    if (!file.open(QIODevice::ReadOnly))
    {
        m_application->m_print_view->print_error(this, "SimuBot: terrain JSON introuvable: " + path);
        return false;
    }
    QJsonParseError perr;
    const QJsonDocument doc = QJsonDocument::fromJson(file.readAll(), &perr);
    file.close();
    if (perr.error != QJsonParseError::NoError || !doc.isObject())
    {
        m_application->m_print_view->print_error(this, "SimuBot: terrain JSON invalide: " + perr.errorString());
        return false;
    }
    const QJsonObject root = doc.object();

    // Dimensions terrain -> moteurs cinematique + collisions (repli X/Y_TERRAIN si absent).
    const QJsonObject terr = root.value("terrain").toObject();
    const float larg = (float)terr.value("largeur_cm").toDouble(X_TERRAIN);
    const float haut = (float)terr.value("hauteur_cm").toDouble(Y_TERRAIN);
    m_kinematic_engine.setTerrain(0.0f, 0.0f, larg, haut);
    m_collision_engine.setTerrain(0.0f, 0.0f, larg, haut);

    // On repart d'un jeu d'elements/obstacles vide (recharge a chaud possible).
    clearGameElements();
    m_collision_engine.clearObstacles();

    // Decor FIXE d'abord (obstacles comme les bordures, mobile force a false), puis elements de
    // JEU (poussables si mobile=true). id de collision partage = index dans elementsJeu.
    int runningId = 0;
    addJsonElements(root.value("decor").toArray(),    true,  runningId);
    addJsonElements(root.value("elements").toArray(), false, runningId);
    return true;
}

/*!
 * \brief CSimuBot::addJsonElements
 * Cree, pour chaque entree d'une liste JSON (decor OU elements de jeu), son sprite Qt ET son
 * obstacle de collision. forceFixed=true force mobile=false (decor : obstacle fixe comme une
 * bordure). runningId (partage decor+elements) = id de collision = index dans elementsJeu.
 * Types : "rect" (cx,cy,w,h,angle_deg) / "polygone" (sommets_cm) / "cercle" (cx,cy,r,n_cotes) /
 * "ellipse" (cx,cy,rx,ry,n_cotes). Repere JSON : terrain (y vers le haut, cm) ; sprite en repere
 * scene (y inverse). Cercle/ellipse et rect tourne approximes en polygone convexe pour le SAT.
 */
void CSimuBot::addJsonElements(const QJsonArray& arr, bool forceFixed, int& runningId)
{
    for (int k = 0; k < arr.size(); ++k)
    {
        const int id = runningId++; // id de collision = index dans elementsJeu (decor puis jeu)
        const QJsonObject e = arr.at(k).toObject();
        const QString type  = e.value("type").toString("rect");
        // Decor : toujours fixe (comme une bordure). Element de jeu : selon le champ "mobile".
        const bool    mobile= forceFixed ? false : e.value("mobile").toBool(true);
        const QColor  col   = colorFromName(e.value("couleur").toString("gris"));
        QGraphicsItem* sprite = nullptr;

        if (type == "rect")
        {
            const float cx = (float)e.value("cx_cm").toDouble();
            const float cy = (float)e.value("cy_cm").toDouble();
            const float w  = (float)e.value("w_cm").toDouble();
            const float h  = (float)e.value("h_cm").toDouble();
            const float ang= (float)e.value("angle_deg").toDouble(0.0);
            if (ang == 0.0f)
            {
                // Rectangle axis-aligned : sprite scene (y inverse) + obstacle rect exact.
                QGraphicsRectItem* it = new QGraphicsRectItem(QRectF(cx - w/2.0f, -cy - h/2.0f, w, h));
                it->setTransformOriginPoint(cx, -cy); // origine de rotation = centre (comme les pousses)
                it->setBrush(QBrush(col));
                terrain->addItem(it);
                sprite = it;
                m_collision_engine.addObstacleRect(cx - w/2.0f, cy - h/2.0f, cx + w/2.0f, cy + h/2.0f, id, mobile);
            }
            else
            {
                // Rectangle tourne : on le traite comme un polygone convexe (4 coins tournes),
                // pour que la rotation de collision (dteta) s'ajoute proprement a l'affichage.
                const float cr = cosf(ang * (float)M_PI / 180.0f);
                const float sr = sinf(ang * (float)M_PI / 180.0f);
                const float hx = w/2.0f, hy = h/2.0f;
                const float lx[4] = { -hx,  hx,  hx, -hx };
                const float ly[4] = { -hy, -hy,  hy,  hy };
                std::vector<CCollisionEngine::Vec2> v(4);
                QPolygonF poly;
                for (int c = 0; c < 4; ++c)
                {
                    const float wx = cx + lx[c]*cr - ly[c]*sr;
                    const float wy = cy + lx[c]*sr + ly[c]*cr;
                    v[c] = { wx, wy };
                    poly << QPointF(wx, -wy);
                }
                QGraphicsPolygonItem* it = new QGraphicsPolygonItem(poly);
                it->setTransformOriginPoint(cx, -cy);
                it->setBrush(QBrush(col));
                terrain->addItem(it);
                sprite = it;
                m_collision_engine.addObstacleConvex(v, id, mobile);
            }
        }
        else if (type == "polygone")
        {
            const QJsonArray sj = e.value("sommets_cm").toArray();
            QPolygonF poly;                          // sprite (scene)
            std::vector<CCollisionEngine::Vec2> v;   // collision (terrain)
            float gcx = 0.0f, gcy = 0.0f;
            for (int s = 0; s < sj.size(); ++s)
            {
                const QJsonArray p = sj.at(s).toArray();
                const float px = (float)p.at(0).toDouble();
                const float py = (float)p.at(1).toDouble();
                poly << QPointF(px, -py);
                v.push_back({ px, py });
                gcx += px; gcy += py;
            }
            if (!v.empty()) { gcx /= (float)v.size(); gcy /= (float)v.size(); }
            QGraphicsPolygonItem* it = new QGraphicsPolygonItem(poly);
            it->setTransformOriginPoint(gcx, -gcy);
            it->setBrush(QBrush(col));
            terrain->addItem(it);
            sprite = it;
            m_collision_engine.addObstacleConvex(v, id, mobile);
        }
        else if (type == "cercle" || type == "ellipse")
        {
            const float cx = (float)e.value("cx_cm").toDouble();
            const float cy = (float)e.value("cy_cm").toDouble();
            const float rx = (type == "cercle") ? (float)e.value("r_cm").toDouble()
                                                : (float)e.value("rx_cm").toDouble();
            const float ry = (type == "cercle") ? rx
                                                : (float)e.value("ry_cm").toDouble();
            const int   nCotes = e.value("n_cotes").toInt(16);
            QGraphicsEllipseItem* it = new QGraphicsEllipseItem(QRectF(cx - rx, -cy - ry, 2.0f*rx, 2.0f*ry));
            it->setTransformOriginPoint(cx, -cy);
            it->setBrush(QBrush(col));
            terrain->addItem(it);
            sprite = it;
            // Collision : ellipse/cercle approximes en polygone convexe (nCotes) pour le SAT.
            m_collision_engine.addObstacleEllipse(cx, cy, rx, ry, id, mobile, nCotes);
        }
        else
        {
            m_application->m_print_view->print_warning(this, "SimuBot: type d'element JSON inconnu: " + type);
            // sprite reste nullptr et aucun obstacle n'est declare, mais on pousse un placeholder
            // ci-dessous pour conserver l'alignement id == index (cf. refreshGameElementsView).
        }
        elementsJeu.push_back(sprite);
    }
}

/*!
 * \brief CSimuBot::writeDefaultTerrainFile
 * Ecrit un fichier terrain JSON par defaut (terrain 300x200, sans obstacle) avec les regles de
 * construction en commentaire, si le fichier est absent (a l'instar de EEPROM.ini). L'utilisateur
 * complete ensuite le decor et les elements (ou depose le fichier de l'annee).
 */
void CSimuBot::writeDefaultTerrainFile(const QString& path)
{
    QFileInfo fi(path);
    QDir().mkpath(fi.absolutePath()); // cree le dossier Config au besoin
    QFile f(path);
    if (!f.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        m_application->m_print_view->print_error(this, "SimuBot: impossible de creer le terrain par defaut: " + path);
        return;
    }
    static const char* DEFAULT_TERRAIN_JSON =
R"JSON({
  "version": 2026,
  "_commentaire": "Description du terrain de la Coupe pour SimuBot. Repere TERRAIN : origine coin bas-gauche, x vers la droite, y vers le HAUT, en cm. 'terrain' : dimensions (toujours 300x200 pour la Coupe). Deux listes d'obstacles au meme sous-schema de forme : 'decor' = elements de DECOR FIXES (estrades, tasseaux...) qui bloquent le robot comme les bordures (jamais pousses) ; 'elements' = elements de JEU, poussables si mobile=true. Types de forme : rect{cx_cm,cy_cm,w_cm,h_cm,angle_deg} / polygone{sommets_cm:[[x,y],...]} (convexe, sens trigo) / cercle{cx_cm,cy_cm,r_cm,n_cotes} / ellipse{cx_cm,cy_cm,rx_cm,ry_cm,n_cotes}. couleur : jaune/bleu/rouge/vert/magenta/gris. cercle et ellipse approximes en polygone (n_cotes, defaut 16) pour la collision SAT. id interne = ordre de chargement (decor puis elements). Fichier recharge a chaud a chaque sauvegarde.",
  "terrain": { "largeur_cm": 300, "hauteur_cm": 200 },
  "decor": [],
  "elements": []
}
)JSON";
    f.write(DEFAULT_TERRAIN_JSON);
    f.close();
    m_application->m_print_view->print_info(this, "SimuBot: terrain par defaut cree: " + path);
}

/*!
 * \brief CSimuBot::reloadTerrain
 * Slot de hot-reload : recharge terrain + elements depuis le JSON edite pendant l'execution.
 */
void CSimuBot::reloadTerrain()
{
    loadTerrainFromJson(m_terrain_json_path);
    // Certains editeurs remplacent le fichier (nouvel inode) -> QFileSystemWatcher perd le path.
    // On le re-ajoute si necessaire pour que les modifications suivantes soient encore captees.
    if (m_terrain_watcher != nullptr
        && !m_terrain_watcher->files().contains(m_terrain_json_path)
        && QFile::exists(m_terrain_json_path))
    {
        m_terrain_watcher->addPath(m_terrain_json_path);
    }
    // Replace les sprites (le deplacement de collision est remis a zero au chargement).
    refreshGameElementsView();
}

/*!
 * \brief CSimuBot::internalSimActive
 * Vrai quand la simulation interne (asserv interne de GrosBot -> x_pos/y_pos/teta_pos) doit jouer
 * le role du robot, c.-a-d. quand il n'existe aucune autre source de position :
 *   - mode SIMU sans Simulia externe (chemin "asserv interne" de l'etape 3bis-B), OU
 *   - mode VISU avec robot RS232 deconnecte (etape 6 : HIL sans robot reel).
 * En VISU robot connecte, x_pos vient de CTrameFactory -> la sim interne reste eteinte.
 */
bool CSimuBot::internalSimActive() const
{
    return !m_simulia_Enabled
        && ( (modeVisu==SIMUBOT::SIMU) || (modeVisu==SIMUBOT::VISU && !robotConnecte()) );
}

/*!
 * \brief CSimuBot::robotConnecte
 * Etat de la liaison RS232 avec le vrai robot, publie par CMessagerieBot dans "Robot_Connecte"
 * (true sur reception de trame, false sur timeout). getData(...,true) cree la cle si CMessagerieBot
 * n'est pas charge -> defaut "deconnecte", donc la sim interne prend la main (comportement voulu).
 */
bool CSimuBot::robotConnecte() const
{
    CData* d = m_application->m_data_center->getData("Robot_Connecte", true);
    return d && d->read().toBool();
}

/*!
 * \brief CSimuBot::syncInternalSim
 * Demarre/arrete le cadenceur de la sim interne selon internalSimActive(). Appele quand un
 * parametre de la condition change : mode (changeMode), Simulia on/off (slot_enableSimulia),
 * connexion robot (onRobotConnectionChanged), ou nouvelle commande de deplacement.
 */
void CSimuBot::syncInternalSim()
{
    if (!cadenceur) return; // garde : appelable tot (changeMode peut precéder la creation du timer)
    if (internalSimActive()) { if (!cadenceur->isActive()) cadenceur->start(25); }
    else
    {
        if (cadenceur->isActive()) cadenceur->stop();
        // La sim interne cesse de jouer le role du robot (mode TEST, Simulia active, robot
        // reconnecte) : on invalide toute cible d'asserv en cours pour ne pas la rejouer a la
        // prochaine reactivation et figer le sprite immediatement (cf. stopInternalSim).
        stopInternalSim();
    }
}

/*!
 * \brief CSimuBot::stopInternalSim
 * Fige l'asserv interne de GrosBot : cible invalidee + forces roue a zero (via stopInternalAsserv).
 * Le cadenceur peut rester actif (internalSimActive) ; sans cible, updateStepFromSimuBot injecte
 * une vitesse nulle et le sprite s'arrete (deceleration par l'inertie du moteur cinematique).
 * Appele a l'arret/fin du HIL (CBlockBotLab) et depuis syncInternalSim quand la sim interne
 * cesse de jouer le role du robot.
 */
void CSimuBot::stopInternalSim()
{
    if (GrosBot) GrosBot->stopInternalAsserv();
}

/*!
 * \brief CSimuBot::resetPublishedConvergence
 * Remet a 0 les convergences publiees dans le DataManager a l'arrivee d'une nouvelle commande de
 * deplacement (le robot n'est plus "arrive"). Republiees a 1 par updateStepFromSimuBot quand
 * l'asserv interne atteint la cible -> permet aux transitions HIL de progresser.
 */
void CSimuBot::resetPublishedConvergence()
{
    m_application->m_data_center->write("Convergence", 0);
    m_application->m_data_center->write("convergence_rapide", 0);
}

/*!
 * \brief CSimuBot::onCmdMoveXY
 * Commande de deplacement XY (front descendant de COMMANDE_MVT_XY_TxSync). En l'absence de robot
 * reel, arme la cible de l'asserv interne. Sinon no-op (CTrameFactory transmet au robot).
 */
void CSimuBot::onCmdMoveXY()
{
    if (!internalSimActive()) return;
    if (m_application->m_data_center->read("COMMANDE_MVT_XY_TxSync").toInt() != 0) return; // front descendant
    float x = m_application->m_data_center->read("X_consigne").toFloat();
    float y = m_application->m_data_center->read("Y_consigne").toFloat();
    GrosBot->setSpeed(0.0);
    GrosBot->setTargetXY(x, y);
    resetPublishedConvergence();
    syncInternalSim();
}

/*!
 * \brief CSimuBot::onCmdMoveXYT
 * Commande de deplacement XY+Teta (front descendant de COMMANDE_MVT_XY_TETA_TxSync).
 */
void CSimuBot::onCmdMoveXYT()
{
    if (!internalSimActive()) return;
    if (m_application->m_data_center->read("COMMANDE_MVT_XY_TETA_TxSync").toInt() != 0) return; // front descendant
    float x = m_application->m_data_center->read("XYT_X_consigne").toFloat();
    float y = m_application->m_data_center->read("XYT_Y_consigne").toFloat();
    float t = m_application->m_data_center->read("XYT_angle_consigne").toFloat();
    GrosBot->setSpeed(0.0);
    GrosBot->setTargetXY(x, y);
    GrosBot->setTargetTeta(t);
    resetPublishedConvergence();
    syncInternalSim();
}

/*!
 * \brief CSimuBot::onCmdMoveDA
 * Commande de deplacement Distance/Angle (front descendant de COMMANDE_DISTANCE_ANGLE_TxSync).
 * angle_consigne = CAP ABSOLU en radians dans le repere asserv (cf. firmware
 * CAsservissementBase::CommandeMouvementDistanceAngle, appele avec inputs()->angle_robot dans
 * sm_evitement). On calcule la cible depuis la pose asserv courante (x_pos/y_pos) : avance de
 * 'distance' le long du cap 'angle'.
 */
void CSimuBot::onCmdMoveDA()
{
    if (!internalSimActive()) return;
    if (m_application->m_data_center->read("COMMANDE_DISTANCE_ANGLE_TxSync").toInt() != 0) return; // front descendant
    float d = m_application->m_data_center->read("distance_consigne").toFloat(); // cm
    float a = m_application->m_data_center->read("angle_consigne").toFloat();     // rad, cap absolu (repere asserv)
    float xc = m_application->m_data_center->read("x_pos").toFloat();
    float yc = m_application->m_data_center->read("y_pos").toFloat();
    float xt = xc + d * (float)qCos(a);
    float yt = yc + d * (float)qSin(a);
    GrosBot->setSpeed(0.0);
    GrosBot->setTargetXY(xt, yt);
    GrosBot->setTargetTeta(a);
    resetPublishedConvergence();
    syncInternalSim();
}

/*!
 * \brief CSimuBot::onRobotConnectionChanged
 * Reagit a un changement de "Robot_Connecte" : perte de connexion en VISU -> la sim interne reprend
 * la main (HIL sans robot) ; (re)connexion -> on la rend au robot (CTrameFactory publie x_pos).
 */
void CSimuBot::onRobotConnectionChanged()
{
    syncInternalSim();
}

/*!
 * \brief CSimuBot::updateStepFromSimulia
 * Fonction qui traite à chaque pas les déplacements demandés par le module Simulia
 */
void CSimuBot::updateStepFromSimulia()
{
    int updatedStep=m_application->m_data_center->read("Simulia.step").toInt();
    //qDebug() << "[CSimuBot] Simulia step n°\t" << updatedStep <<" and old step n°\t"<<m_step;
    if((updatedStep>0) && (updatedStep>m_step) && (modeVisu==SIMUBOT::SIMU) && m_simulia_Enabled)
    {
        //qDebug() << "[CSimuBot] Simulia in loop step n°\t" << updatedStep;
        float vect_G_B1 = m_application->m_data_center->getData("Simulia.vect_G")->read().toFloat();
        float vect_D_B1 = m_application->m_data_center->getData("Simulia.vect_D")->read().toFloat();
        //qDebug() << "[CSimuBot] Simulia commandes moteur (G,D):("<<vect_G_B1<<","<<vect_D_B1<<")";

        //déplacement du robot adverse
        nextStepOther();

        // Moteur cinematique (unique depuis le retrait de Box2D, etape 3) : SIL pur sur le
        // robot principal. Pas de recal externe (on ne court-circuite jamais la boucle SIL,
        // cf. feedback_simubot_sil_loop). bot2 est desormais pilote par une instance Simulia
        // externe (la simulation locale par Box2D a ete retiree) ; la collision
        // robot<->elements de jeu sera portee par CCollisionEngine (sous-etape SAT).
        // Conversion consigne Simulia (80*commande_%) -> vitesse roue cm/s via gain calibrable.
        m_kinematic_engine.step(0.02f, vect_G_B1 * m_kin_speed_gain, vect_D_B1 * m_kin_speed_gain);
        // ETAPE 3bis : l'asservissement fait foi. x_pos/y_pos/teta_pos ne viennent plus du
        // moteur cinematique exact mais de la croyance de l'asserv firmware (via Simulia) :
        // pose reconstruite par CAsservissementBase::CalculXY (schema "tourne-puis-avance",
        // approximations embarquees incluses), en REPERE ASSERV relatif au depart, avec le
        // leurre d'init (setPosition dans match_started) deja pris en compte. Aucun offset
        // m_kin_x_init ici : le placement terrain est applique par display_XY (repere asserv
        // -> scene). Le moteur cinematique continue de produire les pas codeurs + collisions.
        m_application->m_data_center->write("x_pos",    m_application->m_data_center->read("Simulia.x_pos").toFloat());
        m_application->m_data_center->write("y_pos",    m_application->m_data_center->read("Simulia.y_pos").toFloat());
        m_application->m_data_center->write("teta_pos", m_application->m_data_center->read("Simulia.teta_pos").toFloat());
        m_application->m_data_center->write("Simubot.codeur_G", m_kinematic_engine.deltaCodeurG());
        m_application->m_data_center->write("Simubot.codeur_D", m_kinematic_engine.deltaCodeurD());

        // Repercute le deplacement des caisses poussees par le robot (elements mobiles).
        refreshGameElementsView();

        //on avertit Simulia des changement de déplacement codeur
        m_step=updatedStep;
        m_application->m_data_center->write("Simubot.step", m_step);
    }
}

/*!
 * \brief CSimuBot::updateStepFromSimuBot
 * Fonction qui traite à chaque pas les déplacements demandés par le module CActuatorSequencer
 */
void CSimuBot::updateStepFromSimuBot()
{
    // La sim interne produit la pose (x_pos/y_pos/teta_pos) quand elle joue le role du robot :
    // SIMU-sans-Simulia (chemin 3bis-B) OU VISU robot RS232 deconnecte (etape 6 : HIL sans robot).
    // Sinon (VISU robot connecte, Simulia, TEST) la position vient d'ailleurs (CTrameFactory /
    // Simulia) -> on ne fait rien.
    if(internalSimActive())
    {
        //défnition et récupération des forces de déplacement
        float vect_G_B1 = 0.0;
        float vect_D_B1=0.0;
        GrosBot->stepInternalAsserv();
        GrosBot->getForcesAsserv(&vect_G_B1,&vect_D_B1);

        // Moteur cinematique (unique depuis le retrait de Box2D, etape 3). ETAPE 6 (correctif) :
        // l'asserv interne (GraphicElement::stepInternalAsserv, branche vitesse) sort deja une
        // vitesse roue en cm/s -> on l'injecte TELLE QUELLE, sans kin_speed_gain. Ce gain (0.01)
        // ne sert qu'au chemin Simulia pour convertir l'echelle artefact 80xcommande_% de
        // CRoues_simu ; l'appliquer ici ecrasait la consigne d'un facteur 100 (rotation trop
        // lente, translation demesuree, jamais de convergence).
        m_kinematic_engine.step(0.02f, vect_G_B1, vect_D_B1);
        // ETAPE 3bis - chemin asserv interne : l'asserv fait foi. Aucun Modelia ne tourne ici,
        // on reconstruit donc la pose comme le firmware : distance cumulee par roue -> CalculXY
        // (schema "tourne-puis-avance"), publiee en REPERE ASSERV relatif au depart (le
        // reconstructeur a ete cale sur ce repere au raz via setPosition_XYTeta). Aucun offset
        // m_kin_x_init : le placement terrain est applique par display_XY (repere asserv->scene).
        // Le moteur cinematique continue de produire les pas codeurs + collisions (boucle SIL).
        // Constantes via le typedef CPoseReconstructeur : cote Simulia = CAsservissementBase::*
        // (definies dans CAsservissement_simu.cpp), cote LaBotBox = CPoseReconstructeurStandalone::*
        // (memes valeurs). Meme code source dans les deux builds.
        m_cumul_distance_D += m_kinematic_engine.deltaCodeurD() * CPoseReconstructeur::DISTANCE_PAR_PAS_CODEUR_D;
        m_cumul_distance_G += m_kinematic_engine.deltaCodeurG() * CPoseReconstructeur::DISTANCE_PAR_PAS_CODEUR_G;
        m_pose_reconstructor_interne.distance_roue_D = m_cumul_distance_D;
        m_pose_reconstructor_interne.distance_roue_G = m_cumul_distance_G;
        m_pose_reconstructor_interne.CalculXY();
        m_application->m_data_center->write("x_pos",    m_pose_reconstructor_interne.X_robot);
        m_application->m_data_center->write("y_pos",    m_pose_reconstructor_interne.Y_robot);
        m_application->m_data_center->write("teta_pos", m_pose_reconstructor_interne.angle_robot);
        m_application->m_data_center->write("Simubot.codeur_G", m_kinematic_engine.deltaCodeurG());
        m_application->m_data_center->write("Simubot.codeur_D", m_kinematic_engine.deltaCodeurD());

        // Repercute le deplacement des caisses poussees par le robot (elements mobiles).
        refreshGameElementsView();

        // Etape 6 : republie la convergence comme le ferait le robot reel (via CTrameFactory),
        // pour que les transitions HIL (convergence_expert / convergence_rapide_expert) progressent.
        // Seulement si aucun robot n'est connecte, afin de ne pas concurrencer CTrameFactory.
        if(!robotConnecte())
        {
            int conv = (GrosBot->isConvergenceXY && GrosBot->isConvergenceTeta) ? 1 : 0;
            m_application->m_data_center->write("Convergence", conv);
            m_application->m_data_center->write("convergence_rapide", conv);
        }
    }

}
// Editeur de design robot (initDesign / slot_designChanged) retire a l'etape 5 de la migration
// SimuBot v2 : la forme du robot vient desormais de Config/robot.json (cf. loadRobotFromJson),
// il n'y a plus d'edition graphique de la forme en parallele.

/*!
 * \brief CSimuBot::setSimuliaEnabled
 * Positionne l'etat "Simulia actif" et la case a cocher ensemble, sans dependre de l'ordre
 * d'initialisation : la case n'est connectee a slot_enableSimulia qu'apres sa restauration, donc
 * ecrire uniquement le widget ne suffirait pas a mettre l'etat interne a jour. Quand la connexion
 * existe, setChecked() rejoue slot_enableSimulia avec la meme valeur -> geste idempotent.
 */
void CSimuBot::setSimuliaEnabled(bool enabled)
{
    m_simulia_Enabled = enabled;
    if (m_ihm.ui.checkBox_enableSimulia->isChecked() != enabled) {
        m_ihm.ui.checkBox_enableSimulia->setChecked(enabled);
    }
}

void CSimuBot::slot_enableSimulia(int state)
{
    m_simulia_Enabled=((state==Qt::Checked)?true:false);
    /*if(m_ihm.ui.horizontalSlider_toggle_simu->value()==SIMUBOT::SIMU)
    {
        //on est déjà en mode SIMU on se contente de réinitialiser la vue en simulant un changement de mode
        changeMode(SIMUBOT::SIMU);
    }
    else
    {
        //sinon on change vraiment de vue
        m_ihm.ui.horizontalSlider_toggle_simu->setValue(SIMUBOT::SIMU);
    }*/

    qDebug() << "[SimuBot] Simulia est maintenant " << (m_simulia_Enabled?"activé":"désactivé");
    // Le cadenceur de la sim interne suit desormais internalSimActive() (mode + connexion robot),
    // pas seulement l'etat Simulia : demarre en SIMU-sans-Simulia OU VISU-robot-deconnecte.
    syncInternalSim();
}

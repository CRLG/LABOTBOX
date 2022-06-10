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
#include "CAStar.h"

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

    //récupération des contours de GrosBot dans le fichier d'eeprom
    //un point est décrit par une chaine de caractères
    //formatée comme suit "(doublexdouble)", un contour existe par défaut
    val=m_application->m_eeprom->read(getName(),"polygon",QStringList());
    QStringList listePointsPolygon=val.toStringList();
    QPolygonF GrosBotForme=getForm(listePointsPolygon);
    QPolygonF GrosBotFormeOrientation;
    GrosBotFormeOrientation << QPointF(0.0,5.0) << QPointF((4.0+(GrosBotForme.boundingRect().width())/2.0),0.0) << QPointF(0.0,-5.0);
    QPolygonF GrosBotFormeOriented=GrosBotForme.united(GrosBotFormeOrientation);

    //récupération des contours de MiniBot dans le fichier d'eeprom
    val=m_application->m_eeprom->read(getName(),"polygon2",QStringList());
    listePointsPolygon.clear();
    listePointsPolygon=val.toStringList();
    QPolygonF MiniBotForme=getForm(listePointsPolygon);
    QPolygonF MiniBotFormeOrientation;
    MiniBotFormeOrientation << QPointF(0.0,5.0) << QPointF((4.0+(MiniBotForme.boundingRect().width())/2.0),0.0) << QPointF(0.0,-5.0);
    QPolygonF MiniBotFormeOriented=MiniBotForme.united(MiniBotFormeOrientation);

    //Initialisation du moteur physique
    m_box2d_Enabled=true;
    m_physical_engine.createPhysicalWorld(m_application,GrosBotForme,MiniBotForme);

    //récupération des positions d'init de GrosBot
    val = m_application->m_eeprom->read(getName(), "X_init_1_bot1", QVariant(18.0));
    float X_init_1=val.toFloat();
    val = m_application->m_eeprom->read(getName(), "Y_init_1_bot1", QVariant(123.0));
    float Y_init_1=val.toFloat();
    val = m_application->m_eeprom->read(getName(), "Theta_init_1_bot1", QVariant(0.0));
    float Theta_init_1=val.toFloat();
    val = m_application->m_eeprom->read(getName(), "X_init_2_bot1", QVariant(282.0));
    float X_init_2=val.toFloat();
    val = m_application->m_eeprom->read(getName(), "Y_init_2_bot1", QVariant(123.0));
    float Y_init_2=val.toFloat();
    val = m_application->m_eeprom->read(getName(), "Theta_init_2_bot1", QVariant(3.14));
    float Theta_init_2=val.toFloat();
    equipe1_bot1.init(X_init_1,Y_init_1,Theta_init_1,true);
    equipe2_bot1.init(X_init_2,Y_init_2,Theta_init_2,false);

    val = m_application->m_eeprom->read(getName(), "Theta_asserv_init_1_bot1", QVariant(0.0));
    iniTetaAsserv_bot1[EQUIPE1]=val.toFloat();
    val = m_application->m_eeprom->read(getName(), "Theta_asserv_init_2_bot1", QVariant(0.0));
    iniTetaAsserv_bot1[EQUIPE2]=val.toFloat();

    //récupération des positions d'init de GrosBot
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
    surface->setPixmap(QPixmap(":/icons/terrain_2022_simubot.png"));
    surface->setPos(0,-200);
    QGraphicsRectItem *bordures=new QGraphicsRectItem(QRect(0, -200 , 300, 200));
    terrain->addItem(surface);
    terrain->addItem(bordures);

    //ajout des éléments de jeu
    elementsJeu[0]=setElementJeu(90.0f,144.5f,Qt::cyan);
    elementsJeu[1]=setElementJeu(83.0f,132.5f,Qt::cyan);
    elementsJeu[2]=setElementJeu(90.0f,120.5f,Qt::cyan);
    elementsJeu[3]=setElementJeu(210.0f,144.5f,Qt::cyan);
    elementsJeu[4]=setElementJeu(217.0f,132.5f,Qt::cyan);
    elementsJeu[5]=setElementJeu(210.0f,120.5f,Qt::cyan);

    elementsJeu[6]=setElementJeu(87.5f,70.0f,Qt::red);
    elementsJeu[7]=setElementJeu(92.5f,53.0f,Qt::blue);
    elementsJeu[8]=setElementJeu(106.0f,70.0f,Qt::green);
    elementsJeu[9]=setElementJeu(192.5f,70.0f,Qt::red);
    elementsJeu[10]=setElementJeu(197.5f,53.0f,Qt::blue);
    elementsJeu[11]=setElementJeu(211.0f,70.0f,Qt::green);

    //ajout des tasseaux 2022
    QGraphicsLineItem * Tasseau01;
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
    terrain->addItem(Tasseau03);


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
    m_ihm.ui.horizontalSlider_toggle_simu->setValue(val.toInt());

    //on initialise et ajoute le robot au terrain
    //TODO: prendre un fichier de config pour l'emplacement et l'angle de départ pour le robot
    // pour l'instant c'est en dur dans le constructeur
    connect(terrain, SIGNAL(changed(QList<QRectF>)), this, SLOT(viewChanged(QList<QRectF>)));
    connect(GrosBot,SIGNAL(center(qreal,qreal,float)),OldGrosBot,SLOT(replace(qreal,qreal,float)));
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
    connect(m_ihm.ui.pb_Astar,SIGNAL(clicked()),this,SLOT(slot_getPath()));
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
    m_application->m_data_center->write("Simubot.box2d.activated", true);
    m_application->m_data_center->write("Simubot.Init", true);

    //pour le mode simu (fonctionnement avec Simulia)
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

      //pour le moteur physique
      connect(m_application->m_data_center->getData("Simubot.box2d.activated", true), SIGNAL(valueChanged(bool)), this, SLOT(box2d_enable(bool)));
        connect(m_application->m_data_center->getData("Simulia.step", true), SIGNAL(valueChanged(bool)), this, SLOT(updateStepFromSimulia()));

        //pour le fonctionnement avec actuatorsequencer
        m_application->m_data_center->write("COMMANDE_MVT_XY_TETA_TxSync", 0);
        connect(m_application->m_data_center->getData("COMMANDE_MVT_XY_TETA_TxSync", true), SIGNAL(valueChanged(bool)), this, SLOT(Slot_catch_TxSync()));


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
            m_physical_engine.Init(x_init,y_init,theta_init,x_init_2,y_init_2,theta_init_2,twoBotsEnabled);
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
            m_physical_engine.Init(x_init,y_init,Pi*theta_init/180.0f,x_init_2,y_init_2,Pi*theta_init_2/180.0f,twoBotsEnabled);
        }

        //placement des élements de jeu dans le monde simulé
        for(int i=0;i<12;i++)
        {
            elementsJeu[i]->setPos(m_physical_engine.getElement(i).x(), -m_physical_engine.getElement(i).y());
            elementsJeu[i]->setRotation(m_physical_engine.getElementRotation(i));
        }

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

void CSimuBot::slot_getPath(void)
{
    AStar *Recherche=new AStar();
    int xA, yA, xB, yB; //A depart, B objetcif
    int xC,yC;
    int xD,yD;

    xA=OldGrosBot->getX_terrain();
    yA=OldGrosBot->getY_terrain();
    xB=GrosBot->getX_terrain();
    yB=GrosBot->getY_terrain();
    xC=OtherBot->getX_terrain();
    yC=OtherBot->getY_terrain();

    //initialisation de la carte des obstacles
    Recherche->initMap(xC,yC);
    if (m_ihm.ui.ckhB_2Bot->isChecked())
    {
        xD=MiniBot->getX_terrain();
        yD=MiniBot->getY_terrain();
        Recherche->addBot2Map(xD,yD);
    }

    int nb_points_asser=0;

    //lancement de l'algorithme et estimation du temps de calcul
    clock_t start = std::clock();
    int nb_points=Recherche->pathFind(xA, yA, xB, yB);
    clock_t end = std::clock();
    if(nb_points<=0)
    {
        qDebug()<<"\n[CSimuBot][A*] Calcul d'une trajectoire d'évitement";
        qDebug()<<"[CSimuBot][A*] Pas de chemin trouve!\n";
        /*qDebug()<<"[CSimuBot][A*] Dimension carte (X,Y):\t"<<n*PRECISION<<"x"<<m*PRECISION;
        qDebug()<<"[CSimuBot][A*] Depart=>Objectif:\t("<<xA<<","<<yA<<")=>("<<xB<<","<<yB<<")";
        qDebug()<<"[CSimuBot][A*] Autre robot:\t\t("<<xC<<","<<yC<<")";
        double time_elapsed = double(double(end - start)/CLOCKS_PER_SEC);
        qDebug()<<"[CSimuBot][A*] Temps calcul (s):\t"<<time_elapsed<<"\n";*/
    }
    else
        //l'algorithme étant itératif on est obligé de rebrousser chemin pour le reconstruire
        nb_points_asser=Recherche->pathBuild(xA,yA);

    //dans le cas où on a trouvé un chemin possible
    if(nb_points>0)
    {
        qDebug()<<"\n[CSimuBot][A*] Calcul d'une trajectoire d'évitement";
        qDebug()<<"[CSimuBot][A*] Dimension carte (X,Y):\t"<<n*PRECISION<<"x"<<m*PRECISION;
        qDebug()<<"[CSimuBot][A*] Depart=>Objectif:\t("<<xA<<","<<yA<<")=>("<<xB<<","<<yB<<")";
        qDebug()<<"[CSimuBot][A*] Autre robot:\t\t("<<xC<<","<<yC<<")";
        double time_elapsed = double(double(end - start)/CLOCKS_PER_SEC);
        qDebug()<<"[CSimuBot][A*] Temps calcul (s):\t"<<time_elapsed;
        qDebug()<<"[CSimuBot][A*] Route:\t\t\t"<<nb_points<<" pts";
        qDebug()<<"[CSimuBot][A*] Trajectoire:\t\t"<<nb_points_asser<<" pts\n";

        //effacement graphique de la dernière trajectoire
        slot_clearPath();

        int x_temp=xB;
        int y_temp=-yB;
        int j=nb_points_asser-1;

        //reconstruction du chemin en segments à afficher
        while(j>=0)
        {
            //qDebug()<<"("<<x_temp<<","<<y_temp<<") => ("<<(Recherche->i_x_dir[j])<<","<<-(Recherche->i_y_dir[j])<<")";

            //on attend de calculer un point avant de tracer un segment
            if(j!=nb_points_asser-1)
                evitement.append(new QGraphicsLineItem(x_temp,y_temp,(Recherche->i_x_dir[j]),-(Recherche->i_y_dir[j])));

            //sauvegarde du point en cours pour tracer le prochain segment
            x_temp=(Recherche->i_x_dir[j]);
            y_temp=-(Recherche->i_y_dir[j]);

            j--;
        }

        //traçage du chemin d'évitement et affichage
        for(int k=0;k<evitement.size();k++)
        {
            evitement.at(k)->setPen(QPen(Qt::red,3));
            terrain->addItem(evitement.at(k));
        }
    }
}


void CSimuBot::catchDoubleClick()
{
    if(m_ihm.ui.checkBox_setSequence->isChecked())
        emit setSequence();
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
    qreal x_bot_graphic=GrosBot->getX_terrain();
    qreal theta_bot=GrosBot->getTheta();

    //récupération des différentes coordonnées (graphique et réelles) de notre premier robot
    Coord cMiniBot(MiniBot->getX_terrain(),MiniBot->getY_terrain(),MiniBot->getTheta(),(MiniBot->sensOrtho>0));

    //détection GrosBot
    //de l'adversaire
    float capteurs1[4];
    getUSDistance(cGrosBot,cOtherBot,capteurs1);
    //de minibot
    float capteurs2[4];
    if(twoBotsEnabled)
        getUSDistance(cGrosBot,cMiniBot,capteurs2);
    else
    {
        for(int i=0;i<4;i++)
            capteurs2[i]=99;
    }
    //synthèse
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
    //de l'adversaire
    getUSDistance(cMiniBot,cOtherBot,capteurs1);
    //de grosbot
    getUSDistance(cMiniBot,cGrosBot,capteurs2);
    //synthèse
    for(int i=0;i<4;i++)
    {
        if(capteurs1[i]<=capteurs2[i])
            capteurs3[i]=capteurs1[i];
        else
            capteurs3[i]=capteurs2[i];
    }

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
 * \brief CSimuBot::estimateEnvironmentVariables
 *
 * # COMPORTEMENT
 * Calcule toutes les interactions entre le robot et l'environnement
 * - capteurs US
 * - blocage terrain
 */

void CSimuBot::playOther()
{

    double speedOther=m_ihm.ui.doubleSpinBox_speed->value();
    if(speedOther>0)
    {
        currentIndex=0;
        QTableWidgetItem * widget_x=m_ihm.ui.tableWidget->item(currentIndex,0);
        QTableWidgetItem * widget_y=m_ihm.ui.tableWidget->item(currentIndex,1);
        QTableWidgetItem * widget_teta=m_ihm.ui.tableWidget->item(currentIndex,2);
        if((widget_x)&&(widget_y)&&(widget_teta))
        {
            QString str_x_record=m_ihm.ui.tableWidget->item(currentIndex,0)->text();
            QString str_y_record=m_ihm.ui.tableWidget->item(currentIndex,1)->text();
            QString str_teta_record=m_ihm.ui.tableWidget->item(currentIndex,2)->text();
            if((!str_x_record.isEmpty())&&(!str_y_record.isEmpty())&&(!str_teta_record.isEmpty()))
            {
                m_ihm.ui.pB_playOther->setDisabled(true);
                m_ihm.ui.doubleSpinBox_speed->setDisabled(true);
                m_ihm.ui.pB_stopOther->setDisabled(false);

                OtherBot->setSpeed(speedOther);
                float x_record=str_x_record.toFloat();
                float y_record=str_y_record.toFloat();
                //float teta_record=str_teta_record.toFloat();
                //qDebug() << "[SimuBot] Demande n°1 de déplacement du robot adverse ("<<x_record<<","<<y_record<<") à la vitesse "<<speedOther;
                OtherBot->display_XY(x_record,y_record);
                isStarted=true;
            }
        }
    }
}

void CSimuBot::stopOther()
{
    isStarted=false;
    currentIndex=0;
    OtherBot->setSpeed(0);

    m_ihm.ui.pB_playOther->setDisabled(false);
    m_ihm.ui.doubleSpinBox_speed->setDisabled(false);
    m_ihm.ui.pB_stopOther->setDisabled(true);
}

void CSimuBot::nextStepOther()
{
    if (isStarted)
    {
        if(OtherBot->isConvergenceXY)
        {
            currentIndex++;
            if(currentIndex<=10)
            {
                QTableWidgetItem * widget_x=m_ihm.ui.tableWidget->item(currentIndex,0);
                QTableWidgetItem * widget_y=m_ihm.ui.tableWidget->item(currentIndex,1);
                QTableWidgetItem * widget_teta=m_ihm.ui.tableWidget->item(currentIndex,2);
                if((widget_x)&&(widget_y)&&(widget_teta))
                {
                    QString str_x_record=m_ihm.ui.tableWidget->item(currentIndex,0)->text();
                    QString str_y_record=m_ihm.ui.tableWidget->item(currentIndex,1)->text();
                    QString str_teta_record=m_ihm.ui.tableWidget->item(currentIndex,2)->text();
                    if((!str_x_record.isEmpty())&&(!str_y_record.isEmpty())&&(!str_teta_record.isEmpty()))
                    {
                        float x_record=str_x_record.toFloat();
                        float y_record=str_y_record.toFloat();
                        //float teta_record=str_teta_record.toFloat();
                        //qDebug() << "[SimuBot] Demande n°"<<currentIndex<<" de déplacement du robot adverse ("<<x_record<<","<<y_record<<")";
                        OtherBot->display_XY(x_record,y_record);
                    }
                    else
                        stopOther();
                }
                else
                {
                    stopOther();
                }
            }
            else
            {
                stopOther();
            }
        }
        else
            OtherBot->moveAtSpeed();
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
    }
}

QPolygonF CSimuBot::getForm(QStringList strL_Form)
{
    QPolygonF BotForme;
    if(strL_Form.isEmpty()){
      BotForme << QPointF(10,-15) << QPointF(-10,-15)<< QPointF(-15,-10)<< QPointF(-15,10) << QPointF(-10,15);
      BotForme << QPointF(10,15) << QPointF(15,10) << QPointF(15,-10) << QPointF(10,-15);
    }
    else
    {
      QString unStringPoint, temp, xString, yString;
      QStringList ListXY;
      double xFromString, yFromString;
      for(int ind=0;ind<strL_Form.size();ind++)
      {
          unStringPoint=strL_Form.at(ind);
          //qDebug() << unStringPoint;
          ListXY=unStringPoint.split('x');
          if (ListXY.size()>=2){
              temp=ListXY.at(0);
              xString=temp.remove(QChar('('), Qt::CaseInsensitive);
              xFromString=xString.toDouble();
              temp=ListXY.at(1);
              yString=temp.remove(QChar(')'), Qt::CaseInsensitive);
              yFromString=(-1.0)*yString.toDouble();
              BotForme << QPointF(xFromString,yFromString);
          }
       }
    }

    return BotForme;
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
            m_physical_engine.activateBot2(true);
        }
    }
    else
    {
        MiniBot->setVisible(false);
        twoBotsEnabled=false;
        m_ihm.ui.groupBox_MiniBot->setEnabled(false);
        m_ihm.ui.active_external_robot2->setEnabled(false);
        m_ihm.ui.active_external_robot2->setChecked(false);
        m_physical_engine.activateBot2(false);
    }
}

void CSimuBot::getUSDistance(Coord bot, Coord obstacle, float capteurs[])
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
    //modulo 360
    while (adeg < 0)
        adeg += 360;
    while (adeg > 360)
        adeg -= 360;
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

// =======================================================
//              GESTION DU ROBOT N°2 EXTERNE
// =======================================================
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
              m_external_controler_client_robot2.writeData("Simubot.box2d.activated", false);
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
            m_external_controler_client_robot2.writeData("Simubot.box2d.activated", true);
            m_external_controler_client_robot2.close();
        }
        m_connected_host=false;
        MiniBot->setFlag(QGraphicsItem::ItemIsMovable, true);
        MiniBot->setFlag(QGraphicsItem::ItemIsSelectable, true);
        MiniBot->setBrush(QBrush(QColor(255, 255,255, 255)));
    }
}

QGraphicsPolygonItem * CSimuBot::setElementJeu(float x, float y, int Color)
{
    //QGraphicsEllipseItem* element=new QGraphicsEllipseItem(QRect(x,-y,7.2,7.2));
    QPolygonF element_shape;
    element_shape << QPointF(7.5*cos(M_PI/6),7.5*sin(M_PI/6)) << QPointF(0,7.5);
    element_shape << QPointF(7.5*cos(5*M_PI/6),7.5*sin(5*M_PI/6)) << QPointF(7.5*cos(-5*M_PI/6),7.5*sin(-5*M_PI/6));
    element_shape << QPointF(0,-7.5) << QPointF(7.5*cos(-M_PI/6),7.5*sin(-M_PI/6));
    element_shape << QPointF(7.5*cos(M_PI/6),7.5*sin(M_PI/6));
    QGraphicsPolygonItem* element=new QGraphicsPolygonItem(element_shape);
    element->setPos(x,-y);
    if(Color==Qt::cyan)
        element->setBrush(QBrush(QColor(139,69,19, 255)));
    if(Color==Qt::blue)
        element->setBrush(QBrush(QColor(0,0,255, 255)));
    if(Color==Qt::green)
        element->setBrush(QBrush(QColor(0,255,0, 255)));
    if(Color==Qt::red)
        element->setBrush(QBrush(QColor(255,0,0, 255)));

    terrain->addItem(element);

    return element;
}

void CSimuBot::box2d_enable(bool flag)
{
    m_box2d_Enabled=flag;
}

void CSimuBot::updateStepFromSimulia()
{
    int updatedStep=m_application->m_data_center->read("Simulia.step").toInt();
    //int box2Activated=m_application->m_data_center->read("Simubot.box2d.activated").toBool();
    //qDebug() << "[CSimuBot] Simulia step n°\t" << updatedStep <<" and old step n°\t"<<m_step;
    if((updatedStep>0) && (updatedStep>m_step) && (modeVisu==SIMUBOT::SIMU))
    {
        //qDebug() << "[CSimuBot] Simulia in loop step n°\t" << updatedStep;
        float vect_G_B1 = m_application->m_data_center->getData("Simulia.vect_G")->read().toFloat();
        float vect_D_B1 = m_application->m_data_center->getData("Simulia.vect_D")->read().toFloat();
        //qDebug() << "[CSimuBot] Simulia commandes moteur (G,D):("<<vect_G_B1<<","<<vect_D_B1<<")";

        float vect_G_B2 = 0;
        float vect_D_B2 = 0;
        int updatedStep2=0;
        // Lecture des données de la simu externe du robot n°2 s'il est connecté
        if(m_connected_host)
        {
           m_external_controler_client_robot2.writeData("Capteurs.Tirette", m_application->m_data_center->getData("Capteurs.Tirette")->read());
            int error_code;
            QVariant val;

            val = m_external_controler_client_robot2.readData("Simulia.step", &error_code);
            if (error_code == 0) updatedStep2=val.toInt();
            val = m_external_controler_client_robot2.readData("Simulia.vect_G", &error_code);
            if (error_code == 0) vect_G_B2=val.toFloat();
            val = m_external_controler_client_robot2.readData("Simulia.vect_D", &error_code);
            if (error_code == 0) vect_D_B2=val.toFloat();
        }

        //déplacement du robot adverse
        nextStepOther();

        //simulation des déplacements du robot avec le moteur box2d
        m_physical_engine.step(0.02f,vect_G_B1,vect_D_B1,vect_G_B2,vect_D_B2);

        //récupération des données simulées pour les éléments de jeu
        for(int i=0;i<12;i++)
        {
            elementsJeu[i]->setPos(m_physical_engine.getElement(i).x(), -m_physical_engine.getElement(i).y());
            elementsJeu[i]->setRotation(m_physical_engine.getElementRotation(i));
        }

        //environnement physique mis à jour, on l'affiche dans SimuBot
        m_application->m_data_center->write("x_pos", m_physical_engine.x_pos);
        m_application->m_data_center->write("y_pos", m_physical_engine.y_pos);
        m_application->m_data_center->write("teta_pos", m_physical_engine.teta_pos);

        //environnement physique mis à jour, on l'affiche dans SimuBot
        m_application->m_data_center->write("x_pos2", m_physical_engine.x_pos_2);
        m_application->m_data_center->write("y_pos2", m_physical_engine.y_pos_2);
        m_application->m_data_center->write("teta_pos2", m_physical_engine.teta_pos_2);

        //on écrit les modifs des déplacements codeur
        m_application->m_data_center->write("Simubot.codeur_G", m_physical_engine.m_deltaRoue_G_bot1);
        m_application->m_data_center->write("Simubot.codeur_D", m_physical_engine.m_deltaRoue_D_bot1);

        //idem pour la simu externe du robot n°2 s'il est connecté
        if(m_connected_host)
        {
            m_external_controler_client_robot2.writeData("Capteurs.Tirette", m_application->m_data_center->getData("Capteurs.Tirette")->read());
            m_external_controler_client_robot2.writeData("Simubot.Telemetres.ARD", m_application->m_data_center->getData("Simubot.Telemetres.ARD2")->read());
            m_external_controler_client_robot2.writeData("Simubot.Telemetres.ARG", m_application->m_data_center->getData("Simubot.Telemetres.ARG2")->read());
            m_external_controler_client_robot2.writeData("Simubot.Telemetres.AVD", m_application->m_data_center->getData("Simubot.Telemetres.AVD2")->read());
            m_external_controler_client_robot2.writeData("Simubot.Telemetres.AVG", m_application->m_data_center->getData("Simubot.Telemetres.AVG2")->read());
            m_external_controler_client_robot2.writeData("Simubot.codeur_G", m_physical_engine.m_deltaRoue_G_bot2);
            m_external_controler_client_robot2.writeData("Simubot.codeur_D", m_physical_engine.m_deltaRoue_D_bot2);

            //on avertit Simulia des changement de déplacement codeur
            //if(updatedStep2>=m_step2)
                m_step2=updatedStep2;
            m_external_controler_client_robot2.writeData("Simubot.step", m_step2);
        }

        //on avertit Simulia des changement de déplacement codeur
        m_step=updatedStep;
        m_application->m_data_center->write("Simubot.step", m_step);
    }
}

void CSimuBot::Slot_catch_TxSync()
{
    //TODO faire démarrer le timer (cadenceur->start(25);)
    int cmd_XYTETA=m_application->m_data_center->read("COMMANDE_MVT_XY_TETA_TxSync").toInt();
    if(cmd_XYTETA==0) //front descendant de COMMANDE_MVT_XY_TETA_TxSync
    {
        float x_target=m_application->m_data_center->read("XYT_X_consigne").toFloat();
        float y_target=m_application->m_data_center->read("XYT_Y_consigne").toFloat();
        float teta_target=m_application->m_data_center->read("XYT_angle_consigne").toFloat();

        qDebug() << "Teta target=" << teta_target << "\t\terror=" << GrosBot->getErrorAngle(x_target,y_target);
        qDebug() << "XY target=" << x_target << " , "<<y_target << "\t\terror=" << GrosBot->getErrorDistance(x_target,y_target);
    }
}

void CSimuBot::updateStepFromSimuBot()
{
    //TODO
    //appel à box2d
    //gérer le cadenceur suivant les convergences
}

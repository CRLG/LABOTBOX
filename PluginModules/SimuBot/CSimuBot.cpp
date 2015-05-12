/*! \file CSimuBot.cpp
 * A brief file description CPP.
 * A more elaborated file description.
 */
#include <QDebug>
#include "CSimuBot.h"
#include "CLaBotBox.h"
#include "CPrintView.h"
#include "CMainWindow.h"
#include "CEEPROM.h"
#include "CDataManager.h"

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
void CSimuBot::init(CLaBotBox *application)
{
    CPluginModule::init(application);
    setGUI(&m_ihm); // indique à la classe de base l'IHM
    setNiveauTrace(MSG_TOUS);

    // Restore la taille de la fenêtre
    QVariant val;
    val = m_application->m_eeprom->read(getName(), "geometry", QRect(50, 50, 150, 150));
    m_ihm.setGeometry(val.toRect());

    // Restore le fait que la fenêtre est visible ou non
    val = m_application->m_eeprom->read(getName(), "visible", QVariant(true));
    if (val.toBool()) { m_ihm.show(); }
    else              { m_ihm.hide(); }

    //récupération des contours du robot dans le fichier d'eeprom, un point est décrit par une chaine de caractères
    //formatée comme suit "(doublexdouble)", un contour existe par défaut
    val=m_application->m_eeprom->read(getName(),"polygon",QStringList());
    QStringList listePointsPolygon=val.toStringList();
    QPolygonF polygonIni;
    if(listePointsPolygon.isEmpty()){
      polygonIni << QPointF(10,-15) << QPointF(-10,-15)<< QPointF(-15,-10)<< QPointF(-15,10) << QPointF(-10,15);
      polygonIni << QPointF(10,15) << QPointF(15,10) << QPointF(15,-10) << QPointF(10,-15);
    }
    else{
      QString unStringPoint, temp, xString, yString;
      QStringList ListXY;
      double xFromString, yFromString;
      for(int ind=0;ind<listePointsPolygon.size();ind++)
      {
          unStringPoint=listePointsPolygon.at(ind);
          //qDebug() << unStringPoint;
          ListXY=unStringPoint.split('x');
          if (ListXY.size()>=2){
              temp=ListXY.at(0);
              xString=temp.remove(QChar('('), Qt::CaseInsensitive);
              xFromString=xString.toDouble();
              temp=ListXY.at(1);
              yString=temp.remove(QChar(')'), Qt::CaseInsensitive);
              yFromString=(-1.0)*yString.toDouble();
              polygonIni << QPointF(xFromString,yFromString);
          }
       }
    }

    //récupération des positions d'init
    /*float X_init_1=24;
    float Y_init_1=101;
    float Theta_init_1=0;
    float X_init_2=265;
    float Y_init_2=101;
    float Theta_init_2=180;*/
    val = m_application->m_eeprom->read(getName(), "X_init_1", QVariant(24.0));
    float X_init_1=val.toFloat();
    val = m_application->m_eeprom->read(getName(), "Y_init_1", QVariant(101.0));
    float Y_init_1=val.toFloat();
    val = m_application->m_eeprom->read(getName(), "Theta_init_1", QVariant(0.0));
    float Theta_init_1=val.toFloat();
    val = m_application->m_eeprom->read(getName(), "X_init_2", QVariant(265.0));
    float X_init_2=val.toFloat();
    val = m_application->m_eeprom->read(getName(), "Y_init_2", QVariant(101.0));
    float Y_init_2=val.toFloat();
    val = m_application->m_eeprom->read(getName(), "Theta_init_2", QVariant(180.0));
    float Theta_init_2=val.toFloat();

    //ajout des limites physiques du terrain
    QGraphicsPixmapItem *surface=new QGraphicsPixmapItem();
    surface->setPixmap(QPixmap(":/icons/terrain_2015_simubot.png"));
    surface->setPos(0,-202);
    QGraphicsRectItem *bordures=new QGraphicsRectItem(QRect(0, -202 , 302, 202));
    terrain->addItem(bordures);
    terrain->addItem(surface);
    QGraphicsEllipseItem *spot[8];
    spot[0]=new QGraphicsEllipseItem(QRectF(110-3,177-202-3,6,6));
    spot[1]=new QGraphicsEllipseItem(QRectF(9-3,175-202-3,6,6));
    spot[2]=new QGraphicsEllipseItem(QRectF(9-3,185-202-3,6,6));
    spot[3]=new QGraphicsEllipseItem(QRectF(130-3,140-202-3,6,6));
    spot[4]=new QGraphicsEllipseItem(QRectF(87-3,135.5-202-3,6,6));
    spot[5]=new QGraphicsEllipseItem(QRectF(85-3,20-202-3,6,6));
    spot[6]=new QGraphicsEllipseItem(QRectF(85-3,10-202-3,6,6));
    spot[7]=new QGraphicsEllipseItem(QRectF(9-3,20-202-3,6,6));
    for(int ind=0;ind<8;ind++){
        spot[ind]->setBrush(QBrush(QColor(0, 255,255, 255)));
        terrain->addItem(spot[ind]);
    }
    QGraphicsEllipseItem *gobelet[3];
    gobelet[0]=new QGraphicsEllipseItem(QRectF(150-4.75,165-202-4.75,9.5,9.5));
    gobelet[1]=new QGraphicsEllipseItem(QRectF(25-4.75,175-202-4.75,9.5,9.5));
    gobelet[2]=new QGraphicsEllipseItem(QRectF(91-4.75,80-202-4.75,9.5,9.5));

    for(int ind1=0;ind1<3;ind1++){
        gobelet[ind1]->setBrush(QBrush(QColor(255, 255,255, 255)));
        terrain->addItem(gobelet[ind1]);
    }


    //ajout du robot
    GrosBot=new GraphicElement(polygonIni,255,255,255);
    GrosBot->setBrush(QBrush(QColor(255, 255,255, 255)));

    //ajout du point de référence du robot
    QPolygonF ref_GrosBotForme;
    ref_GrosBotForme << QPointF(5,-7) << QPointF(-5,-7)<< QPointF(-7,-5)<< QPointF(-7,5) << QPointF(-5,7);
    ref_GrosBotForme << QPointF(5,7) << QPointF(7,5) << QPointF(7,-5) << QPointF(5,-7);
    OldGrosBot= new GraphicElement(ref_GrosBotForme,255,255,255);
    OldGrosBot->setFlag(QGraphicsItem::ItemIsMovable, false);
    OldGrosBot->setFlag(QGraphicsItem::ItemIsSelectable, false);
    OldGrosBot->setBrush(QBrush(QColor(255,255,255, 100)));

    //on lie les deux points
    QLineF liaison_Line(GrosBot->getX(),GrosBot->getY(),OldGrosBot->getX(),OldGrosBot->getY());
    liaison_GrosBot= new QGraphicsLineItem(liaison_Line);

    //on place le robot et on crée le mécanisme d'init
    QPushButton *pushButton_init=m_ihm.findChild<QPushButton*>("pushButton_init");
    connect(pushButton_init,SIGNAL(clicked()),this,SLOT(initView()));
    QRadioButton *radioButton_couleur_1=m_ihm.findChild<QRadioButton*>("radioButton_couleur_1");
    QRadioButton *radioButton_couleur_2=m_ihm.findChild<QRadioButton*>("radioButton_couleur_2");
    connect(radioButton_couleur_1,SIGNAL(clicked()),this,SLOT(changeEquipe()));
    connect(radioButton_couleur_2,SIGNAL(clicked()),this,SLOT(changeEquipe()));

    //ecrasement des données
    QLineEdit *lineEdit_x=m_ihm.findChild<QLineEdit*>("lineEdit_x");
    QLineEdit *lineEdit_y=m_ihm.findChild<QLineEdit*>("lineEdit_y");
    QLineEdit *lineEdit_theta=m_ihm.findChild<QLineEdit*>("lineEdit_theta");
    connect(lineEdit_x,SIGNAL(returnPressed()),this,SLOT(returnCapture_XY()));
    connect(lineEdit_y,SIGNAL(returnPressed()),this,SLOT(returnCapture_XY()));
    connect(lineEdit_theta,SIGNAL(returnPressed()),this,SLOT(returnCapture_Theta()));
    connect(this, SIGNAL(displayCoord(qreal,qreal)), GrosBot,SLOT(display_XY(qreal,qreal)));
    connect(this,SIGNAL(displayAngle(qreal)),GrosBot,SLOT(display_theta(qreal)));

    QSlider *horizontalSlider_toggle_simu=m_ihm.findChild<QSlider*>("horizontalSlider_toggle_simu");
    modeVisu=horizontalSlider_toggle_simu->value();
    connect(horizontalSlider_toggle_simu,SIGNAL(valueChanged(int)),this,SLOT(changeMode(int)));

    //on initialise et ajoute le robot au terrain
    //TODO: prendre un fichier de config pour l'emplacement et l'angle de départ pour le robot
    // pour l'instant c'est en dur dans le constructeur
    connect(terrain, SIGNAL(changed(QList<QRectF>)), this, SLOT(viewChanged(QList<QRectF>)));
    connect(GrosBot,SIGNAL(center(qreal,qreal,float)),OldGrosBot,SLOT(replace(qreal,qreal,float)));
    terrain->addItem(OldGrosBot);
    terrain->addItem(GrosBot);
    terrain->addItem(liaison_GrosBot);

    //TODO: ajouter nouveaux element comme un MiniBot, un adversaire avec un trajet aléatoire,...
    //ajout des éléments de jeu
//    QList<QGraphicsEllipseItem*> listSpots;
//    listSpots << &QGraphicsEllipseItem(QRect(100, 100 , 6, 6));
//    //QGraphicsEllipseItem * =new QGraphicsRectItem(QRect(0, -202 , 302, 202));
//    terrain->addItem((listSpots.at(0)));


    //ajouté par Steph
    m_ihm.simuView=m_ihm.findChild<QGraphicsView*>("simuGraphicsView");
    m_ihm.simuView->setRenderHint(QPainter::Antialiasing);
    m_ihm.simuView->centerOn(QPointF(151,101));
    m_ihm.simuView->setCacheMode(QGraphicsView::CacheBackground);
    m_ihm.simuView->setViewportUpdateMode(QGraphicsView::BoundingRectViewportUpdate);
    m_ihm.simuView->setDragMode(QGraphicsView::ScrollHandDrag);
    m_ihm.simuView->resize(302, 202);
    m_ihm.simuView->setScene(terrain);

    //pour le mode visu on se connecte aux changements du datamanager
    connect(m_application->m_data_center,SIGNAL(valueChanged(CData*)),this,SLOT(coordChanged(CData*)));

    //changement de repère suivant le choix de coordonnées relatives au terrain ou au robot
    // Restore le fait que la fenêtre est visible ou non
    val = m_application->m_eeprom->read(getName(), "isRelativToBot", QVariant(true));
    QRadioButton *radioButton_robot_relative=m_ihm.findChild<QRadioButton*>("radioButton_robot_relative");
    QRadioButton *radioButton_terrain_relative=m_ihm.findChild<QRadioButton*>("radioButton_terrain_relative");
    if (val.toBool()) {radioButton_robot_relative->setChecked(true); }
    else              {radioButton_terrain_relative->setChecked(true); }

    GrosBot->isRelativToBot=val.toBool();
    OldGrosBot->isRelativToBot=val.toBool();

    //affichage et saisi en radian ou en degré
    //setAndGetInRad
    val = m_application->m_eeprom->read(getName(), "anglesEnRadian", QVariant(true));
    QRadioButton *radioButton_radian=m_ihm.findChild<QRadioButton*>("radioButton_radian");
    QRadioButton *radioButton_degre=m_ihm.findChild<QRadioButton*>("radioButton_degre");
    if (val.toBool()) {radioButton_radian->setChecked(true); setAndGetInRad=true; }
    else              {radioButton_degre->setChecked(true); setAndGetInRad=false; }


    //positionnement par défaut
    changeEquipe(X_init_1,Y_init_1,Theta_init_1,X_init_2,Y_init_2,Theta_init_2);
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
}


/*!
 * \brief CSimuBot::viewChanged on met à jour l'IHM - TODO: à relier au data manager et prendre les ref des pointeurs dans la classe qui gère l'IHM
 * \param regions
 */
void CSimuBot::viewChanged(QList<QRectF> regions)
{
  Q_UNUSED(regions)
    qreal x_view=GrosBot->getX();
    qreal y_view=GrosBot->getY();
    qreal x_graphic=GrosBot->x();
    qreal y_graphic=GrosBot->y();
    qreal theta_view=GrosBot->getTheta();

    qreal x_prim_view=OldGrosBot->getX();
    qreal y_prim_view=OldGrosBot->getY();
    qreal x_prim_graphic=OldGrosBot->x();
    qreal y_prim_graphic=OldGrosBot->y();

    liaison_GrosBot->setLine(x_graphic,y_graphic,x_prim_graphic,y_prim_graphic);

    deltaDistance= sqrt(pow((x_graphic-x_prim_graphic),2)+pow((y_graphic-y_prim_graphic),2));

    //calcul de l'angle par rapport à l'ancienne position
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



    QLCDNumber *lcdNumber_theta=m_ihm.findChild<QLCDNumber*>("lcdNumber_theta");
    QLCDNumber *lcdNumber_X=m_ihm.findChild<QLCDNumber*>("lcdNumber_x");
    QLCDNumber *lcdNumber_Y=m_ihm.findChild<QLCDNumber*>("lcdNumber_y");
    QLCDNumber *lcdNumber_Distance=m_ihm.findChild<QLCDNumber*>("lcdNumber_distance");
    QLCDNumber *lcdNumber_Angle=m_ihm.findChild<QLCDNumber*>("lcdNumber_angle");
    //qDebug() << "view changed";
    if (setAndGetInRad)
        lcdNumber_theta->display(theta_view);
    else
        lcdNumber_theta->display(normalizeAngleDeg(180*theta_view/Pi));
    lcdNumber_X->display(x_view);
    lcdNumber_Y->display(y_view);


    QLineEdit *lineEdit_x=m_ihm.findChild<QLineEdit*>("lineEdit_x");
    QLineEdit *lineEdit_y=m_ihm.findChild<QLineEdit*>("lineEdit_y");
    QLineEdit *lineEdit_theta=m_ihm.findChild<QLineEdit*>("lineEdit_theta");
    QString str;


    if(modeVisu==0)
    {
        lcdNumber_Distance->display(deltaDistance);
        if (setAndGetInRad){
            lcdNumber_Angle->display(deltaAngle);
            lineEdit_theta->setText(str.number(theta_view,'f',2));
            m_application->m_data_center->write("PosTeta_robot", theta_view);
            m_application->m_data_center->write("DirAngle_robot", deltaAngle);
        }
        else{
            lcdNumber_Angle->display(normalizeAngleDeg(180*deltaAngle/Pi));
            lineEdit_theta->setText(str.number(normalizeAngleDeg(180*theta_view/Pi),'f',2));
            m_application->m_data_center->write("PosTeta_robot", normalizeAngleDeg(180*theta_view/Pi));
            m_application->m_data_center->write("DirAngle_robot", normalizeAngleDeg(180*deltaAngle/Pi));
        }
        lineEdit_x->setText(str.number(x_view,'f',2));
        lineEdit_y->setText(str.number(y_view,'f',2));
        //lineEdit_theta->setText(str.number(theta_view,'f',2));

        // Informe le DataCenter de la mise à jour
        m_application->m_data_center->write("PosX_robot", x_view);
        m_application->m_data_center->write("PosY_robot", y_view);
        m_application->m_data_center->write("DirDistance_robot", deltaDistance);


    }
}

void CSimuBot::initView(void){
    QRadioButton *radioButton_robot_relative=m_ihm.findChild<QRadioButton*>("radioButton_robot_relative");
    if(radioButton_robot_relative->isChecked()){
        GrosBot->isRelativToBot=true;
        OldGrosBot->isRelativToBot=true;
    }
    else{
        GrosBot->isRelativToBot=false;
        OldGrosBot->isRelativToBot=false;
    }

    QRadioButton *radioButton_radian=m_ihm.findChild<QRadioButton*>("radioButton_radian");
    if(radioButton_radian->isChecked())
        setAndGetInRad=true;
    else
        setAndGetInRad=false;

    QLineEdit *lineEdit_X_init=m_ihm.findChild<QLineEdit*>("lineEdit_X_init");
    QLineEdit *lineEdit_Y_init=m_ihm.findChild<QLineEdit*>("lineEdit_Y_init");
    QLineEdit *lineEdit_Theta_init=m_ihm.findChild<QLineEdit*>("lineEdit_Theta_init");
    qreal x_init=QString(lineEdit_X_init->text()).toDouble();
    qreal y_init=QString(lineEdit_Y_init->text()).toDouble();
    qreal theta_init=QString(lineEdit_Theta_init->text()).toDouble();
    GrosBot->raz(x_init,y_init,theta_init);
    OldGrosBot->raz(x_init,y_init,theta_init);
    qreal x_reel_init=GrosBot->getX();
    qreal y_reel_init=GrosBot->getY();
    qreal theta_reel_init=GrosBot->getTheta();
    m_application->m_data_center->write("PosX_robot", x_reel_init);
    m_application->m_data_center->write("PosY_robot", y_reel_init);
    if (setAndGetInRad)
        m_application->m_data_center->write("PosTeta_robot", theta_reel_init);
    else
        m_application->m_data_center->write("PosTeta_robot", normalizeAngleDeg(180*theta_reel_init/Pi));
}

void CSimuBot::changeEquipe(float X_init_1,float Y_init_1, float Theta_init_1,float X_init_2,float Y_init_2, float Theta_init_2)
{
    QLineEdit *lineEdit_X_init=m_ihm.findChild<QLineEdit*>("lineEdit_X_init");
    QLineEdit *lineEdit_Y_init=m_ihm.findChild<QLineEdit*>("lineEdit_Y_init");
    QLineEdit *lineEdit_Theta_init=m_ihm.findChild<QLineEdit*>("lineEdit_Theta_init");

    QObject *radioButton_couleur=QObject::sender();
    QString name_radio_button;
    if(radioButton_couleur)
        name_radio_button=radioButton_couleur->objectName();
    else
        name_radio_button="radioButton_couleur_1";

    QString texteValue;
    if(name_radio_button.compare("radioButton_couleur_1")==0) //jaune
    {
        lineEdit_X_init->setText(texteValue.setNum(X_init_1));
        lineEdit_Y_init->setText(texteValue.setNum(Y_init_1));
        lineEdit_Theta_init->setText(texteValue.setNum(Theta_init_1));
    }
    else if(name_radio_button.compare("radioButton_couleur_2")==0) //vert
    {
        lineEdit_X_init->setText(texteValue.setNum(X_init_2));
        lineEdit_Y_init->setText(texteValue.setNum(Y_init_2));
        lineEdit_Theta_init->setText(texteValue.setNum(Theta_init_2));
    }
}

void CSimuBot::returnCapture_Theta()
{
    QLineEdit *lineEdit_theta=m_ihm.findChild<QLineEdit*>("lineEdit_theta");
    if (setAndGetInRad)
        emit displayAngle(QString(lineEdit_theta->text()).toDouble());
    else
        emit displayAngle(Pi*normalizeAngleDeg(QString(lineEdit_theta->text()).toDouble())/180);
}

void CSimuBot::changeMode(int iMode)
{
    modeVisu=iMode;
    QLineEdit *lineEdit_x=m_ihm.findChild<QLineEdit*>("lineEdit_x");
    QLineEdit *lineEdit_y=m_ihm.findChild<QLineEdit*>("lineEdit_y");
    QLineEdit *lineEdit_theta=m_ihm.findChild<QLineEdit*>("lineEdit_theta");

    QLCDNumber *lcdNumber_Distance=m_ihm.findChild<QLCDNumber*>("lcdNumber_distance");
    QLCDNumber *lcdNumber_Angle=m_ihm.findChild<QLCDNumber*>("lcdNumber_angle");

    if (modeVisu==1)
    {
        GrosBot->setFlag(QGraphicsItem::ItemIsMovable, false);
        GrosBot->setFlag(QGraphicsItem::ItemIsSelectable, false);
        OldGrosBot->hide();
        liaison_GrosBot->hide();
        lcdNumber_Distance->display(0);
        lcdNumber_Angle->display(0);
        lineEdit_x->clear();
        lineEdit_y->clear();
        lineEdit_theta->clear();
        lineEdit_x->setEnabled(false);
        lineEdit_y->setEnabled(false);
        lineEdit_theta->setEnabled(false);

    }
    else
    {
        GrosBot->setFlag(QGraphicsItem::ItemIsMovable, true);
        GrosBot->setFlag(QGraphicsItem::ItemIsSelectable, true);
        OldGrosBot->show();
        liaison_GrosBot->show();
        lineEdit_x->setEnabled(true);
        lineEdit_y->setEnabled(true);
        lineEdit_theta->setEnabled(true);
    }
}

void CSimuBot::coordChanged(CData *data)
{
    if(modeVisu==1)
    {
        QString dataName=data->getName();
        if(dataName.compare("PosX_robot")==0)
        {
            //qDebug() << data->read();
            GrosBot->display_XY(data->read().toDouble(),5000);
        }
        if(dataName.compare("PosY_robot")==0)
            GrosBot->display_XY(5000,data->read().toDouble());
        if(dataName.compare("PosTeta_robot")==0){
            if(setAndGetInRad)
                GrosBot->display_theta(data->read().toDouble());
            else
                GrosBot->display_theta(Pi*normalizeAngleDeg(data->read().toDouble())/180);
        }
    }
}

void CSimuBot::returnCapture_XY()
{
    QLineEdit *lineEdit_x=m_ihm.findChild<QLineEdit*>("lineEdit_x");
    QLineEdit *lineEdit_y=m_ihm.findChild<QLineEdit*>("lineEdit_y");

    QString lineEdit_coord_name=QObject::sender()->objectName();

    if (lineEdit_coord_name.compare(lineEdit_x->objectName())==0)
        emit displayCoord(QString(lineEdit_x->text()).toDouble(),5000);

    if (lineEdit_coord_name.compare(lineEdit_y->objectName())==0)
        emit displayCoord(5000,QString(lineEdit_y->text()).toDouble());
}

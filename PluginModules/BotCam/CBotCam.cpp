/*! \file CBotCam.cpp
 * A brief file description CPP.
 * A more elaborated file description.
 */
#include <QDebug>
#include "CBotCam.h"
#include "CLaBotBox.h"
#include "CPrintView.h"
#include "CMainWindow.h"
#include "CEEPROM.h"
#include "CDataManager.h"
#include <QTest>
#include <QFile>
#include <QDate>
#include <QTime>
#include <QtMath>
#include <QtXml/QDomDocument>
#include <QFileDialog>
#include <QMessageBox>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <iostream>

//using namespace cv;
//using namespace std;

/*! \addtogroup Module_Test2
   * 
   *  @{
   */
	   
// _____________________________________________________________________
/*!
*  Constructeur
*
*/
CBotCam::CBotCam(const char *plugin_name)
    :CPluginModule(plugin_name, VERSION_BotCam, AUTEUR_BotCam, INFO_BotCam)
{

/*capture= new VideoCapture(0);
    if(capture->isOpened())
        CamIsOK=true;
    else
        CamIsOK=false;
*/
}


// _____________________________________________________________________
/*!
*  Destructeur
*
*/
CBotCam::~CBotCam()
{
    /*if (isRecording)
        if(enregistreur!=NULL)
        {
            isRecording=false;
            qDebug()<<"Enregistrement vidéo arrété";
            enregistreur->~VideoWriter();
        }*/
    //on ferme la vidéo
    closeCam();
}


// _____________________________________________________________________
/*!
*  Initialisation du module
*
*/
void CBotCam::init(CLaBotBox *application)
{
  CModule::init(application);
  setGUI(&m_ihm); // indique à la classe de base l'IHM

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
  
  //init du scheduler
  schedulerCam=new QTimer();
  schedulerCam->stop();

      //compteur d'images déjà bypassées
    compteurImages=0;

    //init des seuils limites de détection des elements (par défaut)
    H_median=60;
    S_min=255*10/100;
    int H_min_temp=(H_median-(30/2))/2;
    if (H_min_temp<0) H_min=0;
    else H_min=H_min_temp;
    H_max=(H_median+(30/2))/2;
    superficieObjetMax=73000;
    superficieObjetMin=30;

    //mise à jour de l'ihm
    QStringList colorsEnabled;
    colorsEnabled<<"rouge"<<"jaune"<<"vert"<<"cyan"<<"bleu"<<"magenta";
    m_ihm.ui.comboBox_camera->addItems(colorsEnabled);
    //Initialisation de la capture
	val = m_application->m_eeprom->read(getName(), "camUsed", QVariant(false));

    bool camUsed_init=val.toBool();

    //Cam utilisee
    val = m_application->m_eeprom->read(getName(), "camNumber", QVariant(0));
    camNumber=val.toInt();
    QString camIsUsed=(camUsed_init)?"oui":"non";
qDebug() << "cam utilisée: "<<camIsUsed;
qDebug() << "cam n°: "<<camNumber;
    capture= new cv::VideoCapture(camNumber);

    //camUsed=val.toBool();
    //int camNumber=1;
    //capture= new cv::VideoCapture(camNumber);


    //Init des éléments détectés
    for(int i=0;i<NB_ELEMENTS;i++)
    {
        elementsDetectes[i].X=0;
        elementsDetectes[i].Y=0;
        elementsDetectes[i].distance=0.0;
    }

    //par défaut il on désactive la boucle d'enregistrement
    //nb_frames_in_buffer=0;
    isRecording=false;


    //l'init de la capture a-t-elle fonctionné
    if(capture->isOpened())
    {
        //on prévient l'IA que la caméra est OK
        CamIsOK=true;
        qDebug()<<"La caméra est opérationnelle.";

        setSeuil(m_ihm.ui.SliderCouleur->value(),
                 m_ihm.ui.SliderPur->value(),
                 m_ihm.ui.SliderSurface->value());

        //infos de debug
        qDebug() << endl <<"camera choisie :" << camNumber;
        int FourCC=capture->get(CV_CAP_PROP_FOURCC);
        qDebug("FOURCC code: %c%c%c%c", FourCC, FourCC>>8, FourCC>>16, FourCC>>24);
        qDebug() << "Has to be converted to RGB :" << capture->get(CV_CAP_PROP_CONVERT_RGB); //((capture->get(CV_CAP_PROP_CONVERT_RGB)==1) ? "YES":"NO");
        qDebug() << "Set height to 240 :" << ((capture->set(CV_CAP_PROP_FRAME_HEIGHT,240)) ? "OK" : "NOK");
        qDebug() << "Set width to 320 :" << ((capture->set(CV_CAP_PROP_FRAME_WIDTH,320)) ? "OK" : "NOK") << endl;
#ifdef OPENCV_IHM
        cv::namedWindow( "capture", cv::WINDOW_AUTOSIZE );// Create a window for display.
#endif
    }
    else
    {
        CamIsOK=false;
        qDebug() << endl << "Caméra inopérante :-(" << endl;
     }
	 
//    viewfinder=m_ihm.ui.ui_viewfinder;
//    connect(m_ihm.ui.cB_camera,SIGNAL(stateChanged(int)),this,SLOT(enableCamera(int)));
    cameraEnabled=false;
    m_ihm.ui.cB_camera->setChecked(camUsed);

    connect(m_ihm.ui.SliderCouleur,SIGNAL(valueChanged(int)),this,SLOT(calibrateCam()));
    connect(m_ihm.ui.SliderPur,SIGNAL(valueChanged(int)),this,SLOT(calibrateCam()));
    connect(m_ihm.ui.SliderSurface,SIGNAL(valueChanged(int)),this,SLOT(calibrateCam()));
    connect(m_ihm.ui.comboBox_camera,SIGNAL(currentIndexChanged(QString)),this,SLOT(setCouleur(QString)));
    connect(this,SIGNAL(frameCaptured(QImage,int)),this, SLOT(displayFrame(QImage,int)));
        connect(schedulerCam, SIGNAL(timeout()), this, SLOT(getCam()));
        connect(m_ihm.ui.cB_camera, SIGNAL(toggled(bool)),this,SLOT(runCam(bool)));
        runCam(camUsed_init);



}


// _____________________________________________________________________
/*!
*  Fermeture du module
*
*/
void CBotCam::close(void)
{
  // Mémorise en EEPROM l'état de la fenêtre
  m_application->m_eeprom->write(getName(), "geometry", QVariant(m_ihm.geometry()));
  m_application->m_eeprom->write(getName(), "visible", QVariant(m_ihm.isVisible()));
  m_application->m_eeprom->write(getName(), "niveau_trace", QVariant((unsigned int)getNiveauTrace()));
  m_application->m_eeprom->write(getName(), "background_color", QVariant(getBackgroundColor()));
}

// _____________________________________________________________________
/*!
*  Création des menus sur clic droit sur la fenêtre du module
*
*/
void CBotCam::onRightClicGUI(QPoint pos)
{
  QMenu *menu = new QMenu();

  menu->addAction("Select background color", this, SLOT(selectBackgroundColor()));
  menu->exec(m_ihm.mapToGlobal(pos));
}

//! Accesseur de la capture vidéo
/*!
* Appel une image de la capture et lance l'analyse de celle-ci.
* Typiquement on appel cette fonction dans une boucle ou périodiquement
* C'est la fonction appelée par l'IA
*/
bool CBotCam::getCam(void)
{
    cv::Mat frame; //image du buffer video
    cv::Mat frameCloned;

    //capture d'une image du flux video
    capture->grab();

    //récupération de l'image
    bool captureOK=capture->retrieve(frame,0);

    //l'image a-t-elle bien été récupérée
    if (captureOK)
    {
        //clone de l'image pour la persistence des données
        frameCloned=frame.clone();

        //analyse de l'image
        analyseCam(frameCloned);

        //opencv ne profite pas du garbage collector de Qt
        frame.release();
        frameCloned.release();

        return true;
    }
    else
    {
        //opencv ne profite pas du garbage collector de Qt
        frame.release();
        frameCloned.release();

        return false;
    }
}

void CBotCam::calibrateCam()
{
    setSeuil(m_ihm.ui.SliderCouleur->value(),
             m_ihm.ui.SliderPur->value(),
             m_ihm.ui.SliderSurface->value());
}

//! Accesseur pour fermer la capture vidéo
/*!
* Ferme proprement la capture vidéo
*/

bool CBotCam::closeCam(void)
{
    capture->release();
    return true;
}


//! Constructeur de la caméra
/*!
* Initialise la vidéo et les paramètres nécessaires au traitement de l'image
* \param *parent pointeur vers l'objet Qt parent
* \param camNumber numéro de la caméra utilisé par OpenCV
*/

void CBotCam::analyseCam(cv::Mat frame)
{
    //images temporaires pour le traitement video
    cv::Mat frameColor; //image bidouillee pour utilisation GUI
    cv::Mat frameHSV;
    cv::Mat frameBlur;
    cv::Mat frameGray;
    cv::Mat frameSobel;
    cv::Mat frameRecorded;

    QMap<int,QPoint> map_points;

    int idx=0; //index de l'élément détecté

    //séquence qui contiendra les contours des éléments rtouvés
    std::vector < std::vector<cv::Point> > contours;

    //recuperation des hauteurs et largeur de l'image
    int iH = frame.rows;
    int iL = frame.cols;



    //OpenCV travaille en BGR, on convertit donc en RGB
    cv::cvtColor(frame, frameColor, CV_BGR2RGB);
    //cv::cvtColor(frame, frameColor, CV_YUV2RGB);
    //on clone l'image couleur pour les différents traitement couleur
    frameBlur=frameColor.clone();
#ifdef OPENCV_IHM
    cv::imshow("capture",frameColor);
#endif

    //on affiche l'image couleur pour le controle
    //afficheCam(frameColor,true,affichage_Couleur_Origine);

    //conversion en N&B pour les traitements binaires de l'image
    //cv::cvtColor(frameColor,frameGray,CV_RGB2GRAY);
    cv::cvtColor(frame,frameGray,CV_BGR2GRAY);
    //on clone l'image N&B pour les différents traitement N&B
    frameSobel=frameGray.clone();

    //floutage de l'image pour enlever les parasites
    cv::GaussianBlur(frameColor,frameBlur,cv::Size(9,9),10.0,0,cv::BORDER_DEFAULT);

    //Conversion de l'image floutée dans l'espace colorimetrique HSV
    cv::cvtColor(frameBlur,frameHSV,CV_RGB2HSV);


    //seuillage de l'image en extrayant les couleurs des items de l'image HSV
    //si supérieur à la purete, inferieur à l'angle max et superieur a l'angle min alors le pixel nous interesse
    //methode alternative: determiner un histogramme caracteristique de l'element et l'extraire de l'image

    int x, y;
    for (y = 0; y < iH; y++) {
        for (x = 0; x < iL; x++) {
            uchar H_pixel=((uchar*)(frameHSV.data + frameHSV.step*y))[x*3];
            uchar S_pixel=((uchar*)(frameHSV.data + frameHSV.step*y))[x*3+1];
            //uchar V_pixel=((uchar*)(frameHSV.data + frameHSV.step*y))[x*3+2]; //pour le seuillage noir et blanc si on veut

            //detection oranges
            if (H_pixel > H_min && H_pixel < H_max && S_pixel > S_min) {
                ((uchar*)(frameGray.data + frameGray.step*y))[x] = 255;
            }
            else ((uchar*)(frameGray.data + frameGray.step*y))[x] = 0;
        }
    }

    //Debug: Traitement avec le filtre Sobel pour vérifier si le traitement N&B fonctionne
    //cv::Sobel(frameGray,frameSobel,frameSobel.depth(),1,1,3,1,0.0,BORDER_DEFAULT);

    //On affiche l'image binarisée et avec une couleur filtrée pour le controle
    //très utile pour la calibration de la caméra
    afficheCam(frameGray,false,affichage_NetB_Reglage_HSV);

    //détection des contours
    cv::findContours(frameGray,contours,CV_RETR_CCOMP,CV_CHAIN_APPROX_SIMPLE);

    //RAZ des éléments détectés
    int i,j;

    for (size_t contourIdx = 0; contourIdx < contours.size(); contourIdx++)
    {
        std::vector<cv::Point> approx;
        cv::approxPolyDP(contours[contourIdx], approx, 5, true);
        double aire_detectee = contourArea(approx);
        if((aire_detectee>superficieObjetMin)&&(aire_detectee<superficieObjetMax)&&(idx<NB_ELEMENTS))
        {
            //barycentre circonscrit de la forme détectée
            cv::Rect RectCirconscrit=cv::boundingRect( approx );

            ////barycentre du rectangle
            float centre_x=RectCirconscrit.x+RectCirconscrit.width/2;
            float centre_y=RectCirconscrit.y+RectCirconscrit.height/2;
            QPoint centre=QPoint((centre_x/iL)*100,(centre_y/iH)*100);

            map_points.insert(centre.manhattanLength(),centre);

            idx++;
            cv::rectangle(frameColor,RectCirconscrit,CV_RGB(255,0,0),5,8,0);
            //drawContours(frameColor,contours,contourIdx,CV_RGB(255,0,0),5,8,noArray(),0);
        }
    }

    //tri de la table de hash
    //comme les éléments sont réinitialisés on exclut les valeurs nulles
    int indice_proche=0;
    QMap<int,QPoint>::iterator it=map_points.begin();

//    while((elementsDetectes[indice_proche].distance>0)&&(indice_proche<NB_ELEMENTS)) indice_proche++;

    //mettre le point distance
    //circle( frameColor, cvPoint((elementsDetectes[indice_proche-1].X)*iL/100,(elementsDetectes[indice_proche-1].Y)*iH/100), 5, CV_RGB(0,255,0), -1, 8, 0 );
    cv::circle( frameColor, cvPoint((it.value().x())*iL/100,(it.value().y())*iH/100), 5, CV_RGB(0,255,0), -1, 8, 0 );
    //on affiche l'image traitée
    afficheCam(frameColor,true,affichage_Couleur_Origine);

    //Si le flag d'enregistrement est levé (normalement synchro avec la tirette)
    //on enregistre l'image traitée
    //bis repetita: OpenCV travaille en BGR, on convertit donc pour l'enregistrement
    cv::cvtColor(frameColor, frameRecorded, CV_RGB2BGR);
    if (isRecording)
        enregistreur->write(frameRecorded);

    //opencv ne profite pas du garbage collector de Qt
    frameColor.release();
    frameBlur.release();
    frameGray.release();
    frameSobel.release();
    frameHSV.release();
    frameRecorded.release();

}


void CBotCam::afficheCam(cv::Mat frameToQt,bool Colored,int type_Image)
{
    QImage imgConst; //image du buffer video au format Qt

    //recuperation des hauteurs et largeur de l'image
    int iH = frameToQt.rows;
    int iL = frameToQt.cols;

    if (Colored)
    {
        //recuperation des donnees de la frame
        //et referencement dans l'image Qt
        //ATTENTION: pas de copie des donnees -> garbage collector
        const uchar *qImageBuffer = (const uchar*)frameToQt.data;
        //cv::imwrite("tata.jpg",frame);
        QImage img(qImageBuffer, iL, iH, QImage::Format_RGB888);

        //copie complete de l'image pour eviter une desallocation sauvage de la frame
        imgConst=img.copy(QRect(0,0,iL,iH));

        emit(frameCaptured(imgConst,type_Image));
    }
    else
    {
        //recuperation des donnees de la frame
        //et referencement dans l'image Qt
        //ATTENTION: pas de copie des donnees -> garbage collector
        const uchar *qImageBuffer = (const uchar*)frameToQt.data;
        //cv::imwrite("tata.jpg",frame);
        QImage img(qImageBuffer, iL, iH, QImage::Format_Indexed8);

        //image NetB on recree les index de couleur
        QVector<QRgb> colorTable;
        for (int i = 0; i < 256; i++)
        {
            colorTable.push_back(qRgb(i, i, i));
        }
        img.setColorTable(colorTable);

        //copie complete de l'image pour eviter une desallocation sauvage de la frame
        imgConst=img.copy(QRect(0,0,iL,iH));

        emit(frameCaptured(imgConst,type_Image));
    }
}

void CBotCam::runCam(bool b_activate)
{
    camUsed=b_activate;
    if((b_activate)&&(CamIsOK))
    {
        qDebug() <<"lancement capture cam";
        m_ihm.ui.label_cam_state->setText("running");

        schedulerCam->start(300);
    }
    else
    {

        schedulerCam->stop();
        qDebug() <<"arret capture cam";
        m_ihm.ui.label_cam_state->setText("stopped");

    }
}

void  CBotCam::setSeuil(double H_angle_detection, double S_purete_detection, double area_detection)
{
    S_min=255*S_purete_detection/100;
    int H_min_temp=(H_median-(H_angle_detection/2))/2;
    if (H_min_temp<0) H_min=0;
    else H_min=H_min_temp;
    H_max=(H_median+(H_angle_detection/2))/2;
    superficieObjetMin=area_detection;
    m_ihm.ui.lcdCouleur->display(H_angle_detection);
    m_ihm.ui.lcdPur->display(S_min);
    m_ihm.ui.lcdSurface->display(m_ihm.ui.SliderSurface->value());
             //m_ihm.ui.SliderPur->value();
             //m_ihm.ui.SliderSurface->value());
}

void CBotCam::setCouleur(QString couleur)
{
    /*
    http://fr.wikipedia.org/wiki/Teinte_Saturation_Valeur
    0° ou 360° : rouge ;
    60° : jaune ;
    120° : vert ;
    180° : cyan ;
    240° : bleu ;
    300° : magenta;
    */

    //Sauvegarde de l'angle de détection actuel
    double H_angle_detection_back;
    H_angle_detection_back=((2*H_max)-H_median)*2;

    if(couleur.contains("rouge",Qt::CaseInsensitive))
           H_median=0;
    if(couleur.contains("jaune",Qt::CaseInsensitive))
           H_median=60;
    if(couleur.contains("vert",Qt::CaseInsensitive))
           H_median=120;
    if(couleur.contains("cyan",Qt::CaseInsensitive))
           H_median=180;
    if(couleur.contains("bleu",Qt::CaseInsensitive))
           H_median=240;
    if(couleur.contains("magenta",Qt::CaseInsensitive))
            H_median=300;
    qDebug() << "choix couleur" << couleur;

    int H_min_temp=(H_median-(H_angle_detection_back/2))/2;
    if (H_min_temp<0) H_min=0;
    else H_min=H_min_temp;
    H_max=(H_median+(H_angle_detection_back/2))/2;
}

void CBotCam::displayFrame(QImage imgConst, int type)
{
   if(type==affichage_NetB_Reglage_HSV)
       m_ihm.ui.frameNB->setPixmap(QPixmap::fromImage(imgConst));
   if(type==affichage_Couleur_Origine)
       m_ihm.ui.frameC->setPixmap(QPixmap::fromImage(imgConst));

}

/*void CBotCam::recording(bool flag)
{
    //Est-ce qu'on demande l'enregistrement
    if (flag)
    {
        //est-ce que l'enregistreur est déjà créé
        if(enregistreur==NULL)
        {
            //construction du nom de fichier
            QString fileName = "Match_";
            QDateTime aujourdhui=QDateTime::currentDateTime();
            fileName = fileName + aujourdhui.toString("dd_MM_yyyy_hh'h'mm'min'ss's'")+".avi";

            //création de l'objet OpenCV qui enregistre des frame
            enregistreur= new VideoWriter(fileName.toStdString(),CV_FOURCC('X','V','I','D'),5,Size(320,240),true);
            if (enregistreur->isOpened())
            {
                qDebug()<<"Enregistrement vidéo démarré:" << fileName;
                isRecording=true;
            }
        }
    }
    else
    {
        //on demande l'arrêt de l'enregistrement
        //est-ce que l'enregistreur est déjà arrêté
        if(enregistreur!=NULL)
        {
            qDebug()<<"Enregistrement vidéo arrété";
            //fin de la boucle d'enregistrement dans la boucle de traitement d'image
            isRecording=false;
            //appel du destructeur
            enregistreur->~VideoWriter();
        }
    }
}
*/

/*
void CBotCam::setCamera(int numDevice) //const QByteArray &cameraDevice)
{
    if (imageCapture)
    delete imageCapture;
    if (camera)
    delete camera;

    if (QCamera::availableDevices().isEmpty())
        camera = new QCamera;
    else
        camera = new QCamera(QCamera::availableDevices().at(numDevice));//cameraDevice);
    cameraEnabled=true;
    //TODO: relier le cameraEnabled au systeme d'erreur de la camera
    //connect(camera, SIGNAL(error(QCamera::Error)), this, SLOT(displayCameraError()));
    //debug purpose
//    for(int idxDevice=0;idxDevice<QCamera::availableDevices().count();idxDevice++)
    imageCapture = new QCameraImageCapture(camera);

    camera->setViewfinder(viewfinder);

    camera->setCaptureMode(QCamera::CaptureStillImage);
    camera->start();
    camera->searchAndLock();
}
*/
/*
void CBotCam::enableCamera(int state)
{
    if(state==Qt::Checked){
        if (QCamera::availableDevices().isEmpty())
        {
            disconnect(m_ihm.ui.cB_camera,SIGNAL(stateChanged(int)),this,SLOT(enableCamera(int)));
            m_ihm.ui.cB_camera->setChecked(false);
            connect(m_ihm.ui.cB_camera,SIGNAL(stateChanged(int)),this,SLOT(enableCamera(int)));
        }
        else
        {
            m_ihm.ui.comboBox_camera->setDisabled(false);
            m_ihm.ui.comboBox_camera->clear();
            for(int idxDevice=0;idxDevice<QCamera::availableDevices().count();idxDevice++)
            {
                m_ihm.ui.comboBox_camera->addItem(QCamera::deviceDescription(QCamera::availableDevices().at(idxDevice)),QVariant(idxDevice));
            }
            setCamera(m_ihm.ui.comboBox_camera->currentData(Qt::UserRole).toInt());
        }
    }
    else if(state==Qt::Unchecked)
    {
        if (imageCapture)
            delete imageCapture;
        if (camera)
            delete camera;
        cameraEnabled=false;

        m_ihm.ui.comboBox_camera->setDisabled(false);
        m_ihm.ui.comboBox_camera->clear();

    }
}*/

/*
void CBotCam::displayCameraError()
{
    QMessageBox::warning(this, tr("Camera error"), camera->errorString());
}
*/
/*
void CBotCam::takePicture()
{
    if (cameraEnabled)
    {
        imageCapture->capture();
    }
}
*/


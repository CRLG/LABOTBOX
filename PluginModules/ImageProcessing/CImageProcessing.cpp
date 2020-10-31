/*! \file CImageProcessing.cpp
 * A brief file description CPP.
 * A more elaborated file description.
 */
#include <QDebug>

#include "CImageProcessing.h"
#include "CApplication.h"
#include "CPrintView.h"
#include "CEEPROM.h"
#include "CDataManager.h"

/*! \addtogroup Module_ImageProcessing
   * 
   *  @{
   */
	   
// _____________________________________________________________________
/*!
*  Constructeur
*
*/
CImageProcessing::CImageProcessing(const char *plugin_name)
    :CPluginModule(plugin_name, VERSION_ImageProcessing, AUTEUR_ImageProcessing, INFO_ImageProcessing),
      m_video_worker(NULL)
{
    refresh_camera_list();

    m_ihm.ui.active_debug->setChecked(false);

    connect(m_ihm.ui.start_work, SIGNAL(released()), this, SLOT(startVideoWork()));
    connect(m_ihm.ui.stop_work, SIGNAL(released()), this, SLOT(stopVideoWork()));
    connect(m_ihm.ui.init_thread, SIGNAL(released()), this, SLOT(initVideoThread()));
    connect(m_ihm.ui.kill_thread, SIGNAL(released()), this, SLOT(killVideoThread()));
    connect(m_ihm.ui.active_debug, SIGNAL(stateChanged(int)), this, SLOT(activeDebug(int)));

}


// _____________________________________________________________________
/*!
*  Destructeur
*
*/
CImageProcessing::~CImageProcessing()
{
    stopVideoWork();
    killVideoThread();
}


// _____________________________________________________________________
/*!
*  Initialisation du module
*
*/
void CImageProcessing::init(CApplication *application)
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
  //enregistrement de la vidéo
  val = m_application->m_eeprom->read(getName(), "record", QVariant(false));
  m_record=val.toBool();
  m_ihm.ui.cB_record->setChecked(m_record);
  //parametres intrinseques de la camera
  val = m_application->m_eeprom->read(getName(), "camera_parameters", QVariant("cam_parameters.txt"));
  m_camera_parameters=val.toString();
  //en cas de balise la camera démarre toute seule
  val = m_application->m_eeprom->read(getName(), "auto_on", QVariant(false));
  m_auto_on=val.toBool();
  m_ihm.ui.chck_asBeacon->setChecked(m_auto_on);

  m_application->m_data_center->write("TempsMatch", -1);

  /*
  VIDEO_PROCESS_BALISE_MAT = 0,
  VIDEO_PROCESS_NORD_SUD,
  VIDEO_PROCESS_SEQUENCE_COULEUR
    VIDEO_PROCESS_CALIBRATION*/
  QStringList str_list_algo;
  str_list_algo << "MAT BALISE" << "RECO NORD SUD" << "SEQUENCE COULEURS" <<"CALIBRATION";
  m_ihm.ui.list_algo->clear();
  m_ihm.ui.list_algo->addItems(str_list_algo);

    connect(m_ihm.ui.pB_setCharuco,SIGNAL(clicked(bool)),this,SLOT(setCharucoCalibration()));
    connect(m_ihm.ui.pB_getCharuco,SIGNAL(clicked(bool)),this,SLOT(getCharucoCalibration()));
    connect(m_ihm.ui.cB_typeCalibration,SIGNAL(stateChanged(int)),this,SLOT(enableCharucoCalibration(int)));
    connect(m_ihm.ui.pB_SetCalibration,SIGNAL(clicked(bool)),this,SLOT(setCalibration()));
    connect(m_ihm.ui.cB_record,SIGNAL(stateChanged(int)),this,SLOT(setRecord(int)));

  // Crée les variables dans le data manager
  // 3 jeux de données (X, Y, Teta) par robot
  // 4 robots possibles gérés par le plugin (Grosbot, Minibot, robot adverse n°1, robot adverse n°2)
  m_application->m_data_center->write("Robot1_X",  -32000);
  m_application->m_data_center->write("Robot1_Y",  -32000);
  m_application->m_data_center->write("Robot1_Teta",  -32000);
  m_application->m_data_center->write("Robot2_X",  -32000);
  m_application->m_data_center->write("Robot2_Y",  -32000);
  m_application->m_data_center->write("Robot2_Teta",  -32000);
  m_application->m_data_center->write("Robot3_X",  -32000);
  m_application->m_data_center->write("Robot3_Y",  -32000);
  m_application->m_data_center->write("Robot3_Teta",  -32000);
  m_application->m_data_center->write("Robot4_X",  -32000);
  m_application->m_data_center->write("Robot4_Y",  -32000);
  m_application->m_data_center->write("Robot4_Teta",  -32000);
  m_application->m_data_center->write("VideoActive", 0);
  m_application->m_data_center->write("TIMESTAMP_MATCH.Timestamp", 0);
    connect(m_application->m_data_center->getData("TIMESTAMP_MATCH.Timestamp"), SIGNAL(valueChanged(QVariant)), this, SLOT(TpsMatch_changed(QVariant)));
    b_robStarted=false;
    refresh_camera_list();

    m_compteur_Nord=0;
        m_compteur_Sud=0;

    if(m_auto_on)
    {
        initVideoThread();
        startVideoWork();
    }

}


// _____________________________________________________________________
/*!
*  Fermeture du module
*
*/
void CImageProcessing::close(void)
{
  // Mémorise en EEPROM l'état de la fenêtre
  m_application->m_eeprom->write(getName(), "geometry", QVariant(m_ihm.geometry()));
  m_application->m_eeprom->write(getName(), "visible", QVariant(m_ihm.isVisible()));
  m_application->m_eeprom->write(getName(), "niveau_trace", QVariant((unsigned int)getNiveauTrace()));
  m_application->m_eeprom->write(getName(), "background_color", QVariant(getBackgroundColor()));
  //mémorise les paramètres de la caméra
  m_auto_on=m_ihm.ui.chck_asBeacon->isChecked();
  m_application->m_eeprom->write(getName(), "auto_on", QVariant(m_auto_on));
  m_application->m_eeprom->write(getName(), "record", QVariant(m_record));

}

// _____________________________________________________________________
/*!
*  Création des menus sur clic droit sur la fenêtre du module
*
*/
void CImageProcessing::onRightClicGUI(QPoint pos)
{
  QMenu *menu = new QMenu();

  menu->addAction("Select background color", this, SLOT(selectBackgroundColor()));
  menu->exec(m_ihm.mapToGlobal(pos));
}


// ______________________________________________________________________________
void CImageProcessing::refresh_camera_list()
{
    m_cameras = QCameraInfo::availableCameras();
    m_ihm.ui.video_devices_list->clear();
    int i=0;
    foreach (const QCameraInfo &cameraInfo, m_cameras) {
        m_ihm.ui.video_devices_list->insertItem(i++, cameraInfo.deviceName());
    }

    if(m_ihm.ui.video_devices_list->count()>0)
        m_ihm.ui.video_devices_list->setCurrentRow(0);
}

// ______________________________________________________________________________
void CImageProcessing::video_worker_init(int video_source_id)
{
    qRegisterMetaType<tVideoInput>();
    qRegisterMetaType<tVideoResult>();

    m_video_worker = new VideoWorker;
    m_video_worker->activeDebug(m_ihm.ui.active_debug->isChecked());
    connect(m_video_worker, SIGNAL(camStateChanged(int)),this, SLOT(getCamState(int)));
    if(m_video_worker->init(video_source_id,m_camera_parameters))
    {
        m_video_worker->moveToThread(&m_video_worker_thread);
        connect(&m_video_worker_thread, &QThread::finished, m_video_worker, &QObject::deleteLater);
        connect(&m_video_worker_thread, &QThread::finished, this, &CImageProcessing::videoThreadStopped);
        connect(this, SIGNAL(operate(tVideoInput)), m_video_worker, SLOT(doWork(tVideoInput)));
        connect(m_video_worker, SIGNAL(resultReady(tVideoResult,QImage)), this, SLOT(videoHandleResults(tVideoResult,QImage)));
        connect(m_video_worker, SIGNAL(workStarted()), this, SLOT(videoWorkStarted()));
        connect(m_video_worker, SIGNAL(workFinished()), this, SLOT(videoWorkFinished())); 
        m_ihm.ui.tabCalibration->setEnabled(true);
        m_video_worker_thread.start();
    }
    else
    {
        m_ihm.ui.tabCalibration->setEnabled(false);
        disconnect(m_video_worker, SIGNAL(camStateChanged(int)),this, SLOT(getCamState(int)));
        delete m_video_worker;
        m_video_worker = NULL;
    }
}

// ======================================================================
// Video Worker events
// ======================================================================
void CImageProcessing::videoHandleResults(tVideoResult result, QImage imgConst)
{
    //on prend les dimensions de l'image
    int w = imgConst.width();
    int h = imgConst.height();
    //on adapte au zoom demandé
    int zoom=m_ihm.ui.zoomVideo->value();

    //affichage de la vidéo uniquement si l'onglet est sélectionné
    if(m_ihm.ui.tabWidget->currentIndex()==1)
    {
        if(m_ihm.ui.active_debug->isChecked())
        {
            //le zoom est adapté on laisse l'image à sa dimension
            if((zoom==0 && ((h==240)||(w==320)))||(zoom==1 && ((h==480)||(w==640)))||(zoom==2 && ((h==768)||(w==1024))))
            {
                m_ihm.ui.containerVideo->setPixmap(QPixmap::fromImage(imgConst));
            }
            else if(zoom==0 && ((h!=240)||(w!=320)))
            {
                //zoom QVGA
                m_ihm.ui.containerVideo->setPixmap(QPixmap::fromImage(imgConst).scaled(320,240,Qt::KeepAspectRatio));
            }
            else if(zoom==1 && ((h!=480)||(w!=640)))
            {
                //zoom VGA
                m_ihm.ui.containerVideo->setPixmap(QPixmap::fromImage(imgConst).scaled(640,480,Qt::KeepAspectRatio));
            }
            else if(zoom==2 && ((h!=768)||(w!=1024)))
            {
                //zoom XGA
                m_ihm.ui.containerVideo->setPixmap(QPixmap::fromImage(imgConst).scaled(1024,768,Qt::KeepAspectRatio));
            }
            else
            {
                //dans tous les cas on affiche la vidéo
                m_ihm.ui.containerVideo->setPixmap(QPixmap::fromImage(imgConst));
            }
        }
        else
            m_ihm.ui.containerVideo->setPixmap(QPixmap(":/icons/cancel.png"));
    }

    if(m_ihm.ui.tabWidget->currentIndex()==3)
    {
        if(m_ihm.ui.active_debug->isChecked())
        {
            //le zoom est adapté on laisse l'image à sa dimension
            if((h==240)&&(w==320))
            {
                m_ihm.ui.containerVideoCalibration->setPixmap(QPixmap::fromImage(imgConst));
            }
            else
            {
                //zoom QVGA
                m_ihm.ui.containerVideoCalibration->setPixmap(QPixmap::fromImage(imgConst).scaled(320,240,Qt::KeepAspectRatio));
            }
        }
        else
            m_ihm.ui.containerVideo->setPixmap(QPixmap(":/icons/cancel.png"));
    }

    float angle1=result.value[IDX_ROBOT1_ANGLE];
    float ratio1=getLensRatio(result.value[IDX_ROBOT1_ANGLE]);
    int distance1=floorf(10*ratio1*result.value[IDX_ROBOT1_DIST]); //en mm
    m_ihm.ui.rob1_dist->setValue(distance1);
    m_ihm.ui.rob1_angle->setValue(angle1);

    float angle2=result.value[IDX_ROBOT2_ANGLE];
    float ratio2=getLensRatio(result.value[IDX_ROBOT2_ANGLE]);
    float distance2=floorf(10*ratio2*result.value[IDX_ROBOT2_DIST]); //en mm
    m_ihm.ui.rob2_dist->setValue(distance2);
    m_ihm.ui.rob2_angle->setValue(angle2);

    m_ihm.ui.rob3_dist->setValue(result.value[IDX_ROBOT3_DIST]);
    m_ihm.ui.rob3_angle->setValue(result.value[IDX_ROBOT3_ANGLE]);

    m_ihm.ui.qLed_Nord->setValue(((result.value[IDX_NORD]==1.)?true:false));
    m_ihm.ui.qLed_Sud->setValue(((result.value[IDX_SUD]==1.)?true:false));

    m_compteur_Nord=m_compteur_Nord+result.value[IDX_NORD];
    m_compteur_Sud=m_compteur_Sud+result.value[IDX_SUD];



    m_ihm.ui.qLed_Nord_2->setValue(((result.value[IDX_NORD]==1.)?true:false));
    m_ihm.ui.qLed_Sud_2->setValue(((result.value[IDX_SUD]==1.)?true:false));

    int iG=Qt::green;
    int iR=Qt::red;
    switch((int)result.value[IDX_SEQUENCE])
    {
        case SEQUENCE_UNKNOWN:
            showResultGobelets(0,0,0,0,0);
            break;
        case SEQUENCE_GRGGR:
            showResultGobelets(iG,iR,iG,iG,iR);
            break;
        case SEQUENCE_GRRGR:
            showResultGobelets(iG,iR,iR,iG,iR);
            break;
        case SEQUENCE_GGRGR:
            showResultGobelets(iG,iG,iR,iG,iR);
            break;
        case SEQUENCE_GRGRR:
            showResultGobelets(iG,iR,iG,iR,iR);
            break;
        case SEQUENCE_GGGRR:
            showResultGobelets(iG,iG,iG,iR,iR);
            break;
        case SEQUENCE_GGRRR:
            showResultGobelets(iG,iG,iR,iR,iR);
            break;
        default:
            showResultGobelets(0,0,0,0,0);
            break;
    }

    m_ihm.ui.fps->setValue(result.m_fps);

    //Mise à jour des informations en fonction de l'algo
    //float ro=0.;
    //float teta=0.;
    switch(m_ihm.ui.list_algo->currentIndex())
    {
        case VIDEO_PROCESS_BALISE_MAT:
            //Robot 1
            m_application->m_data_center->write("Camera.Robot1_Dist",  distance1/10);
            m_application->m_data_center->write("Camera.Robot1_Teta",  angle1);

            //Robot 2
            m_application->m_data_center->write("Camera.Robot2_Dist",  distance2/10);
            m_application->m_data_center->write("Camera.Robot2_Teta",  angle2);

            //envoi de l'info au mbed
            m_application->m_data_center->write("MBED_CMDE_TxSync", 1);
            m_application->m_data_center->write("valeur_mbed_cmde_01", distance1);
            m_application->m_data_center->write("valeur_mbed_cmde_02", distance2);
            m_application->m_data_center->write("valeur_mbed_cmde_03", 0);
            m_application->m_data_center->write("valeur_mbed_cmde_04", 0);
            m_application->m_data_center->write("CodeCommande",1 );
            m_application->m_data_center->write("MBED_CMDE_TxSync", 0);
        break;

        case VIDEO_PROCESS_NORD_SUD:
            m_application->m_data_center->write("Camera.Nord", ((result.value[IDX_NORD]==1.)?1:0));
            m_application->m_data_center->write("Camera.Sudd", ((result.value[IDX_SUD]==1.)?1:0));

            //envoi de l'info au mbed
            m_application->m_data_center->write("MBED_CMDE_TxSync", 1);
            m_application->m_data_center->write("valeur_mbed_cmde_01", 0);
            m_application->m_data_center->write("valeur_mbed_cmde_02", 0);
            m_application->m_data_center->write("valeur_mbed_cmde_03", ((result.value[IDX_NORD]==1.)?1:0));
            m_application->m_data_center->write("valeur_mbed_cmde_04", ((result.value[IDX_SUD]==1.)?1:0));
            m_application->m_data_center->write("CodeCommande",2 );
            m_application->m_data_center->write("MBED_CMDE_TxSync", 0);
        break;

        /*case VIDEO_PROCESS_SEQUENCE_COULEUR:

        break;*/

        default: break;
    }

}

void CImageProcessing::videoWorkStarted()
{
    m_ihm.ui.work_status->setValue(1);
    m_ihm.ui.start_work->setEnabled(false);
    m_ihm.ui.kill_thread->setEnabled(false);
    m_ihm.ui.list_algo->setEnabled(false);
    m_ihm.ui.stop_work->setEnabled(true);
    m_ihm.ui.tabCalibration->setEnabled(true);
    //qDebug() << "[CImageProcessing] Le traitement Video est lancé.";
}

void CImageProcessing::videoWorkFinished()
{
    m_ihm.ui.work_status->setValue(0);
    m_ihm.ui.start_work->setEnabled(true);
    m_ihm.ui.kill_thread->setEnabled(true);
    m_ihm.ui.list_algo->setEnabled(true);
    m_ihm.ui.stop_work->setEnabled(false);
    m_ihm.ui.tabCalibration->setEnabled(false);
    //qDebug() << "[CImageProcessing] Le traitement Vidéo est arreté.";
}


void CImageProcessing::videoThreadStopped()
{
    m_ihm.ui.work_status->setValue(0);
    m_ihm.ui.kill_thread->setEnabled(true);
    //qDebug() << "[CImageProcessing] Le Thread Vidéo est arrêté.";
}

// ======================================================================
void CImageProcessing::initVideoThread()
{
    if (m_video_worker == NULL) {
        // récupère l'identifiant du port vidéo sélectionné
        int video_source_id=0;
        if(m_ihm.ui.video_devices_list->count()>0)
        {
            video_source_id=m_ihm.ui.video_devices_list->currentRow();
            //qDebug() << "[CImageProcessing] Vidéo sélectionnée (nom,identifiant)=("<<m_ihm.ui.video_devices_list->currentItem()->text()<<","<<video_source_id<<")";
        }
        else
        {
            //qDebug() << "[CImageProcessing] Aucun périphérique vidéo choisi: on essaye le premier disponible s'il existe";
        }

        //initialisation du thread vidéo
        video_worker_init(video_source_id);
    }

    //Thread Vidéo initialisé, on peut lancer le traitement
    if (m_video_worker != NULL)
    {
        bool state = true;
        m_ihm.ui.kill_thread->setEnabled(state);
        m_ihm.ui.start_work->setEnabled(state);
        m_ihm.ui.list_algo->setEnabled(state);
        m_ihm.ui.active_debug->setEnabled(true);
        m_ihm.ui.cB_record->setEnabled(true);
    }
}

void CImageProcessing::killVideoThread()
{
    if (m_video_worker == NULL) return;

    disconnect(&m_video_worker_thread, &QThread::finished, m_video_worker, &QObject::deleteLater);
    disconnect(this, SIGNAL(operate(tVideoInput)), m_video_worker, SLOT(doWork(tVideoInput)));
    disconnect(m_video_worker, SIGNAL(resultReady(tVideoResult,QImage)), this, SLOT(videoHandleResults(tVideoResult,QImage)));
    disconnect(m_video_worker, SIGNAL(workStarted()), this, SLOT(videoWorkStarted()));
    disconnect(m_video_worker, SIGNAL(workFinished()), this, SLOT(videoWorkFinished()));
    disconnect(m_video_worker, SIGNAL(camStateChanged(int)),this, SLOT(getCamState(int)));

    m_video_worker_thread.quit();
    m_video_worker_thread.wait();

    delete m_video_worker;
    m_video_worker = NULL;
    m_application->m_data_center->write("VideoActive", 0);

    bool state = false;
    m_ihm.ui.cam_status->setValue(0);
    m_ihm.ui.kill_thread->setEnabled(state);
    m_ihm.ui.start_work->setEnabled(state);
    m_ihm.ui.stop_work->setEnabled(state);
    m_ihm.ui.list_algo->setEnabled(state);
    m_ihm.ui.active_debug->setEnabled(state);
    m_ihm.ui.cB_record->setEnabled(state);
    m_ihm.ui.cB_typeCalibration->setEnabled(true);
    m_ihm.ui.pB_getCharuco->setEnabled(false);
}

void CImageProcessing::startVideoWork(void)
{
    tVideoInput param;
    param.video_process_algo = (tVideoProcessAlgoType)m_ihm.ui.list_algo->currentIndex();
    param.value[IDX_PARAM_01] = m_ihm.ui.in_data1->value();
    param.value[IDX_PARAM_02] = m_ihm.ui.in_data2->value();
    param.value[IDX_PARAM_03] = m_ihm.ui.in_data3->value();
    param.value[IDX_PARAM_ECART_VERT]=m_ihm.ui.ecartVert->value();
    param.value[IDX_PARAM_PURETE_VERT]=m_ihm.ui.pureteVert->value();
    param.value[IDX_PARAM_ECART_ROUGE]=m_ihm.ui.ecartVert->value();
    param.value[IDX_PARAM_PURETE_ROUGE]=m_ihm.ui.pureteRouge->value();
    param.value[IDX_PARAM_PIXEL_MIN]=m_ihm.ui.pixelMin->value();
    param.value[IDX_PARAM_PIXEL_MAX]=m_ihm.ui.pixelMax->value();
    param.record=m_record;

    if(m_ihm.ui.list_algo->currentIndex()==VIDEO_PROCESS_CALIBRATION)
    {
        m_ihm.ui.active_debug->setChecked(true);
        setCalibration();
    }

    emit operate(param);
}

void CImageProcessing::stopVideoWork()
{
    //qDebug() << "[CImageProcessing] Arrêt du traitement vidéo via l'IHM.";
    if (m_video_worker) m_video_worker->stopWork();
}

void CImageProcessing::activeDebug(int state)
{
    bool on_off=(state==Qt::Checked?true:false);
    if (m_video_worker) m_video_worker->activeDebug(on_off);
}

void CImageProcessing::getCamState(int state)
{
    //initialise l'état caméra
    m_ihm.ui.cam_status->setValue(state);
    m_application->m_data_center->write("VideoActive",  state);
}

void CImageProcessing::TpsMatch_changed(QVariant val)
{
    const unsigned char DUREE_MATCH = 100;

    int TpsMatch=val.toInt();
    if((TpsMatch>0)&&(TpsMatch<=DUREE_MATCH))
    {
        if(b_robStarted==false)
        {
            stopVideoWork();
            startVideoWork();
            b_robStarted=true;
        }
    }
     if(TpsMatch>DUREE_MATCH)
         stopVideoWork();
}

void CImageProcessing::setCalibration()
{
    if (m_video_worker == NULL) return;

    m_video_worker->m_internal_param[IDX_PARAM_01] = m_ihm.ui.in_data1->value();
    m_video_worker->m_internal_param[IDX_PARAM_02] = m_ihm.ui.in_data2->value();
    m_video_worker->m_internal_param[IDX_PARAM_03] = m_ihm.ui.in_data3->value();
    m_video_worker->m_internal_param[IDX_PARAM_ECART_VERT]=m_ihm.ui.ecartVert->value();
    m_video_worker->m_internal_param[IDX_PARAM_PURETE_VERT]=m_ihm.ui.pureteVert->value();
    m_video_worker->m_internal_param[IDX_PARAM_ECART_ROUGE]=m_ihm.ui.ecartVert->value();
    m_video_worker->m_internal_param[IDX_PARAM_PURETE_ROUGE]=m_ihm.ui.pureteRouge->value();
    m_video_worker->m_internal_param[IDX_PARAM_PIXEL_MIN]=m_ihm.ui.pixelMin->value();
    m_video_worker->m_internal_param[IDX_PARAM_PIXEL_MAX]=m_ihm.ui.pixelMax->value();
}

void CImageProcessing::showResultGobelets(int gob1, int gob2, int gob3, int gob4, int gob5)
{
    switch(gob1)
    {
     case 0:
        m_ihm.ui.qLed_gob_01->setValue(false);
        break;
    case Qt::green:
        m_ihm.ui.qLed_gob_01->setOnColor(QLed::ledColor::Green);
        m_ihm.ui.qLed_gob_01->setValue(true);
        break;
    case Qt::red:
        m_ihm.ui.qLed_gob_01->setOnColor(QLed::ledColor::Red);
        m_ihm.ui.qLed_gob_01->setValue(true);
        break;
    default:
        m_ihm.ui.qLed_gob_01->setValue(false);
        break;
    }
    switch(gob2)
    {
     case 0:
        m_ihm.ui.qLed_gob_02->setValue(false);
        break;
    case Qt::green:
        m_ihm.ui.qLed_gob_02->setOnColor(QLed::ledColor::Green);
        m_ihm.ui.qLed_gob_02->setValue(true);
        break;
    case Qt::red:
        m_ihm.ui.qLed_gob_02->setOnColor(QLed::ledColor::Red);
        m_ihm.ui.qLed_gob_02->setValue(true);
        break;
    default:
        m_ihm.ui.qLed_gob_02->setValue(false);
        break;
    }
    switch(gob3)
    {
     case 0:
        m_ihm.ui.qLed_gob_03->setValue(false);
        break;
    case Qt::green:
        m_ihm.ui.qLed_gob_03->setOnColor(QLed::ledColor::Green);
        m_ihm.ui.qLed_gob_03->setValue(true);
        break;
    case Qt::red:
        m_ihm.ui.qLed_gob_03->setOnColor(QLed::ledColor::Red);
        m_ihm.ui.qLed_gob_03->setValue(true);
        break;
    default:
        m_ihm.ui.qLed_gob_03->setValue(false);
        break;
    }
    switch(gob4)
    {
     case 0:
        m_ihm.ui.qLed_gob_04->setValue(false);
        break;
    case Qt::green:
        m_ihm.ui.qLed_gob_04->setOnColor(QLed::ledColor::Green);
        m_ihm.ui.qLed_gob_04->setValue(true);
        break;
    case Qt::red:
        m_ihm.ui.qLed_gob_04->setOnColor(QLed::ledColor::Red);
        m_ihm.ui.qLed_gob_04->setValue(true);
        break;
    default:
        m_ihm.ui.qLed_gob_04->setValue(false);
        break;
    }
    switch(gob5)
    {
     case 0:
        m_ihm.ui.qLed_gob_05->setValue(false);
        break;
    case Qt::green:
        m_ihm.ui.qLed_gob_05->setOnColor(QLed::ledColor::Green);
        m_ihm.ui.qLed_gob_05->setValue(true);
        break;
    case Qt::red:
        m_ihm.ui.qLed_gob_05->setOnColor(QLed::ledColor::Red);
        m_ihm.ui.qLed_gob_05->setValue(true);
        break;
    default:
        m_ihm.ui.qLed_gob_05->setValue(false);
        break;
    }
}

void CImageProcessing::getCharucoCalibration()
{
    if (m_video_worker == NULL) return;

    m_video_worker->m_internal_param[IDX_PARAM_GET_CHARUCO_FRAME] = 1.;
    m_ihm.ui.pB_setCharuco->setEnabled(true);
}

void CImageProcessing::setCharucoCalibration()
{
    m_video_worker->m_internal_param[IDX_PARAM_SET_CHARUCO_FRAME] = 1.;
    m_ihm.ui.pB_setCharuco->setEnabled(false);
    m_ihm.ui.pB_getCharuco->setEnabled(false);
}

void CImageProcessing::enableCharucoCalibration(int state)
{
    if((state==Qt::Checked)&&(m_ihm.ui.cB_typeCalibration->isEnabled()))
    {
        m_ihm.ui.cB_typeCalibration->setEnabled(false);
        //m_ihm.ui.cB_typeCalibration->setChecked(false);
        m_ihm.ui.pB_getCharuco->setEnabled(true);
        m_video_worker->m_internal_param[IDX_PARAM_CALIB_TYPE]=1.;
    }
}

void CImageProcessing::setRecord(int state)
{
    if(state==Qt::Checked)
        m_record=true;
    else
        m_record=false;
}

float CImageProcessing::getLensRatio(float angle)
{
    return 1.618*angle*angle-0.05*angle+0.781;
}

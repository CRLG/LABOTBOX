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

    connect(m_ihm.ui.start_work, SIGNAL(released()), this, SLOT(startVideoWork()));
    connect(m_ihm.ui.stop_work, SIGNAL(released()), this, SLOT(stopVideoWork()));
    connect(m_ihm.ui.init_thread, SIGNAL(released()), this, SLOT(initVideoThread()));
    connect(m_ihm.ui.kill_thread, SIGNAL(released()), this, SLOT(killVideoThread()));
    connect(m_ihm.ui.active_debug, SIGNAL(toggled(bool)), this, SLOT(activeDebug(bool)));

    m_ihm.ui.active_debug->setChecked(true);
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
  //parametres intrinseques de la camera
  val = m_application->m_eeprom->read(getName(), "camera_parameters", QVariant(0));
  m_camera_parameters=val.toString();
  //en cas de balise la camera démarre toute seule
  val = m_application->m_eeprom->read(getName(), "auto_on", QVariant(false));
  m_auto_on=val.toBool();

  m_application->m_data_center->write("TempsMatch", -1);

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

    if(m_auto_on)
    {
        initVideoThread();
        startVideoWork(false);
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
  m_application->m_eeprom->write(getName(), "auto_on", QVariant(m_auto_on));

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

    // insert a dummy camera (for test)
    //m_ihm.ui.video_devices_list->insertItem(i++, "/dev/video_dummy1");
    //m_ihm.ui.video_devices_list->insertItem(i++, "/dev/video_dummy2");
}

// ______________________________________________________________________________
void CImageProcessing::video_worker_init(int video_source_id)
{
    qRegisterMetaType<tVideoInput>();
    qRegisterMetaType<tVideoResult>();

    m_video_worker = new VideoWorker;
    connect(m_video_worker, SIGNAL(setCamState(int)),this, SLOT(getCamState(int)));
    m_video_worker->init(video_source_id,m_camera_parameters);
    m_video_worker->moveToThread(&m_video_worker_thread);
    connect(&m_video_worker_thread, &QThread::finished, m_video_worker, &QObject::deleteLater);
    connect(&m_video_worker_thread, &QThread::finished, this, &CImageProcessing::videoThreadStopped);
    connect(this, SIGNAL(operate(tVideoInput)), m_video_worker, SLOT(doWork(tVideoInput)));
    connect(m_video_worker, SIGNAL(resultReady(tVideoResult,QImage)), this, SLOT(videoHandleResults(tVideoResult,QImage)));
    connect(m_video_worker, SIGNAL(workStarted()), this, SLOT(videoWorkStarted()));
    connect(m_video_worker, SIGNAL(workFinished()), this, SLOT(videoWorkFinished()));

    m_video_worker_thread.start();
}

// ======================================================================
// Video Worker events
// ======================================================================
void CImageProcessing::videoHandleResults(tVideoResult result, QImage imgConst)
{
    /*qDebug() << "Video result available:";
    qDebug() << "   result1:" << result.result1;
    qDebug() << "   result2:" << result.result2;*/
    //on prend les dimensions de l'image
    int w = imgConst.width();
    int h = imgConst.height();
    //on adapte au zoom demandé
    int zoom=m_ihm.ui.zoomVideo->value();

    //affichage selon l'onglet sélectionné
    switch(m_ihm.ui.tabWidget->currentIndex())
    {
    case 1:
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
        break;
    case 2:
        m_ihm.ui.rob1_dist->setValue(result.robot1_dist);
        m_ihm.ui.rob1_angle->setValue(result.robot1_angle);
        m_ihm.ui.rob2_dist->setValue(result.robot2_dist);
        m_ihm.ui.rob2_angle->setValue(result.robot2_angle);
        m_ihm.ui.fps->setValue(result.m_fps);
        break;
    default:
        break;
    }

    //Robot 1
    //calcul des cordonnées polaires rapportées au terrain (centre = projection de la caméra
    float ro=sqrt(result.robot1_dist*result.robot1_dist+57*57);
    float teta=result.robot1_angle; 
    //envoi des coordonnées cartésiennes
    m_application->m_data_center->write("Robot1_X",  ro*cos(teta));
    m_application->m_data_center->write("Robot1_Y",  ro*sin(teta));
    m_application->m_data_center->write("Robot1_Teta",  teta);

    //Robot 2
    //calcul des cordonnées polaires rapportées au terrain (centre = projection de la caméra
    ro=sqrt(result.robot2_dist*result.robot2_dist+57*57);
    teta=result.robot2_angle;
    //envoi des coordonnées cartésiennes
    m_application->m_data_center->write("Robot2_X",  ro*cos(teta));
    m_application->m_data_center->write("Robot2_Y",  ro*sin(teta));
    m_application->m_data_center->write("Robot2_Teta",  teta);
}

void CImageProcessing::videoWorkStarted()
{
    m_ihm.ui.work_status->setValue(1);
    m_ihm.ui.start_work->setEnabled(false);
    m_ihm.ui.list_algo->setEnabled(false);
    m_ihm.ui.stop_work->setEnabled(true);

    qDebug() << "Video work is started";
}

void CImageProcessing::videoWorkFinished()
{
    m_ihm.ui.work_status->setValue(0);
    m_ihm.ui.start_work->setEnabled(true);
    m_ihm.ui.list_algo->setEnabled(true);
    m_ihm.ui.stop_work->setEnabled(false);

    qDebug() << "Video work is finished";
}


void CImageProcessing::videoThreadStopped()
{
    m_ihm.ui.work_status->setValue(0);
    qDebug() << "Thread is stopped";
}

// ======================================================================
void CImageProcessing::initVideoThread()
{
    if (m_video_worker == NULL) {
        // récupère le nom du port vidéo sélectionné
        /*QList<QListWidgetItem*> list_items = m_ihm.ui.video_devices_list->selectedItems();
        QString video_source_name = "";
        if (list_items.count() != 0) {
            video_source_name = list_items.at(0)->text();
        }*/
        int video_source_id=0;
        if(m_ihm.ui.video_devices_list->count()>0)
        {
            video_source_id=m_ihm.ui.video_devices_list->currentRow();

            qDebug() << "Video device:("<<m_ihm.ui.video_devices_list->currentItem()->text()<<","<<video_source_id<<")";
        }
        else
            qDebug() << "No video device chosen: try with the first one if any";


        video_worker_init(video_source_id);
    }

    if (m_video_worker != NULL) {
        bool state = true;
        m_ihm.ui.kill_thread->setEnabled(state);
        m_ihm.ui.start_work->setEnabled(state);
        m_ihm.ui.list_algo->setEnabled(state);
        m_ihm.ui.active_debug->setEnabled(true);
    }
}

void CImageProcessing::killVideoThread()
{
    if (m_video_worker == NULL) return;

    qDebug() << "CImageProcessing::killVideoThread Disconnect all";
    disconnect(&m_video_worker_thread, &QThread::finished, m_video_worker, &QObject::deleteLater);
    disconnect(this, SIGNAL(operate(tVideoInput)), m_video_worker, SLOT(doWork(tVideoInput)));
    disconnect(m_video_worker, SIGNAL(resultReady(tVideoResult)), this, SLOT(videoHandleResults(tVideoResult)));
    disconnect(m_video_worker, SIGNAL(workStarted()), this, SLOT(videoWorkStarted()));
    disconnect(m_video_worker, SIGNAL(workFinished()), this, SLOT(videoWorkFinished()));
    disconnect(m_video_worker, SIGNAL(setCamState(int)),this, SLOT(getCamState(int)));

    m_video_worker_thread.quit();
    m_video_worker_thread.wait();

    delete m_video_worker;
    m_video_worker = NULL;
    m_application->m_data_center->write("VideoActive", 0);

    bool state = false;
    m_ihm.ui.kill_thread->setEnabled(state);
    m_ihm.ui.start_work->setEnabled(state);
    m_ihm.ui.stop_work->setEnabled(state);
    m_ihm.ui.list_algo->setEnabled(state);
    m_ihm.ui.active_debug->setEnabled(state);
}



void CImageProcessing::startVideoWork(bool b_record)
{
    tVideoInput param;
    param.video_process_algo = (tVideoProcessAlgoType)m_ihm.ui.list_algo->currentIndex();
    param.data1 = m_ihm.ui.in_data1->value();
    param.data2 = m_ihm.ui.in_data2->value();
    param.data3 = m_ihm.ui.in_data3->value();
    param.record=b_record;
    emit operate(param);
}

void CImageProcessing::startVideoWork(void)
{
    tVideoInput param;
    param.video_process_algo = (tVideoProcessAlgoType)m_ihm.ui.list_algo->currentIndex();
    param.data1 = m_ihm.ui.in_data1->value();
    param.data2 = m_ihm.ui.in_data2->value();
    param.data3 = m_ihm.ui.in_data3->value();
    param.record=false;
    emit operate(param);
}

void CImageProcessing::stopVideoWork()
{
    qDebug() << "UI Stop work";
    if (m_video_worker) m_video_worker->stopWork();
}

void CImageProcessing::activeDebug(bool on_off)
{
    if (m_video_worker) m_video_worker->activeDebug(on_off);
}

void CImageProcessing::getCamState(int state)
{
    //initialise l'état caméra
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
            startVideoWork(true);
            b_robStarted=true;
        }
    }
     if(TpsMatch>DUREE_MATCH)
         stopVideoWork();


}

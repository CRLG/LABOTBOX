#include <qdebug.h>
#include <QDir>

#include "video_thread.h"

// ======================================================
// ======================================================
VideoWorker::VideoWorker()
{
    m_iH=0;
    m_iL=0;
    parameterConfirmed=false;
    recordInitialized=false;
    m_dbg_active=false;
}

void VideoWorker::stopWork()
{
    m_stop_work_request = true;
}

void VideoWorker::activeDebug(bool on_off)
{
    m_dbg_active = on_off;
}

// ======================================================
// ======================================================
bool VideoWorker::init(int video_id, QString parameter_file)
{
    bool isInit=false;

    calibrationFixParameters="%YAML:1.0 \n\
            --- \n\
            calibration_time: \"ven. 28 sept. 2018 16:51:07 CEST\"\n\
            image_width: 640\n\
            image_height: 480\n\
            aspectRatio: 1.\n\
            flags: 2\n\
            camera_matrix: !!opencv-matrix\n\
               rows: 3\n\
               cols: 3\n\
               dt: d\n\
               data: [  745.    , 0.    , 311.,\n\
                        0.      ,745.   , 250.,\n\
                        0.      , 0.    , 1. ]\n\
            distortion_coefficients: !!opencv-matrix\n\
               rows: 1\n\
               cols: 5\n\
               dt: d\n\
               data: [ -0.3, 5.7,-0.02,0.05,-0.3]\n\
            avg_reprojection_error: 0.8";

    //m_video_name = video_name;
    m_video_id=video_id;

    //ouverture de la vidéo
    capture= new cv::VideoCapture(m_video_id);
    if(!(capture->isOpened()))
        capture= new cv::VideoCapture(-1);

    if(capture->isOpened())
    {
        int FourCC=capture->get(CV_CAP_PROP_FOURCC);
        int hasToBeConvertedRGB=capture->get(CV_CAP_PROP_CONVERT_RGB);
        int setHeight=capture->set(CV_CAP_PROP_FRAME_HEIGHT,480);
        int setWidth=capture->set(CV_CAP_PROP_FRAME_WIDTH,640);

        if(m_dbg_active)
        {
            unsigned char m_1_CC = FourCC;
            unsigned char m_2_CC = FourCC>>8;
            unsigned char m_3_CC = FourCC>>16;
            unsigned char m_4_CC = FourCC>>24;
            qDebug() << "[CImageProcessing] La caméra est opérationnelle.";
            qDebug() << "[CImageProcessing] Id de la camera choisie : " << video_id;
            qDebug() << "[CImageProcessing] Code FOURCC de l'image acquise:\t\t" << m_1_CC << m_2_CC << m_3_CC << m_4_CC;
            qDebug() << "[CImageProcessing] Image a convertir de BGR à RGB avant traitement:" << ((hasToBeConvertedRGB==1)? "OUI":"NON");
            qDebug() << "[CImageProcessing] Réglage de la hauteur de l'image à 480:\t" << ((setHeight==1) ? "REUSSI" : "ERREUR");
            qDebug() << "[CImageProcessing] Réglage de la longueur de l'image à 640 :\t" << ((setWidth==1) ? "REUSSI" : "ERREUR");
            //qDebug() << "[CImageProcessing] Mettre l'exposition à 50 :\t" << ((capture->set( CV_CAP_PROP_EXPOSURE, 50)) ? "REUSSI" : "ERREUR") << endl;
        }

        //calibration de la caméra
        QDir directory;
        QString path=directory.currentPath()+"/Config/"+parameter_file;
        std::string contents = "";

        //lecture du fichier de calibration
        QFile f(path);
        if(!f.open(QFile::ReadOnly))
        {
            if(m_dbg_active)
            {
                qDebug() << "[CImageProcessing] Ouverture impossible du fichier de calibration " << path;
                qDebug() << "[CImageProcessing] Données de calibration par défaut, le traitement peut être dégradé voire dysfonctionnel!";
            }
            contents=calibrationFixParameters;
        }
        else
        {
            contents = f.readAll().toStdString();
            f.close();
        }

        //Réglage de la caméra avec les données de calibration
        cv::FileStorage fSettings(contents, cv::FileStorage::READ|cv::FileStorage::MEMORY);
        if (!fSettings.isOpened())
        {
            fSettings.release();
            bCalibrated=false;
            if(m_dbg_active)
                qDebug() << "[CImageProcessing] Enregistrement impossible des données du fichier de calibration " << path;
        }
        else
        {
            fSettings["camera_matrix"] >> camMatrix;
            fSettings["distortion_coefficients"] >> distCoeffs;
            fSettings.release();
            bCalibrated=true;
            if(m_dbg_active)
                qDebug() << "[CImageProcessing] Camera calibrée";
        }

        //init de la taille des marqueurs AruCo
        markerLength=7;

        //on ne peut pas travailler avec une caméra non calibrée
        if(bCalibrated)
        {
            isInit=true;
            camState=true;
            emit camStateChanged(1);
        }
        else
        {
            capture->release();
            isInit=false;
            camState=false;
            emit camStateChanged(0);
            if(m_dbg_active)
                qDebug() << "[CImageProcessing] Caméra inopérante :-(";
        }
    }
    else
    {
        isInit=false;
        camState=false;
        emit camStateChanged(0);
        if(m_dbg_active)
            qDebug() << "[CImageProcessing] Caméra inopérante :-(";
    }

    return isInit;
}

VideoWorker::~VideoWorker()
{
    //on ferme la camera
    if(capture->isOpened())
    {
        emit camStateChanged(0);
        capture->release();
    }
}

bool VideoWorker::getCamState()
{
    return camState;
}

// _________________________________________________________________
void VideoWorker::doWork(tVideoInput parameter) {
    m_stop_work_request = false;

    emit workStarted();
    // ----------------------------------------------
    while (!m_stop_work_request)
    {
        switch(parameter.video_process_algo)
        {
            case VIDEO_PROCESS_BALISE_MAT :
                _video_process_Balise(parameter);
            break;

            case VIDEO_PROCESS_SEQUENCE_COULEUR :
                _video_process_ColorSequence(parameter);
            break;

            case VIDEO_PROCESS_NORD_SUD :
                _video_process_NordSud(parameter);
            break;

            default :
                // nothing to do
                QThread::msleep(50);
            break;
        }
        QThread::msleep(5);
    }
    // ----------------------------------------------
    emit workFinished();
}


// ========================================================
void VideoWorker::_video_process_Balise(tVideoInput parameter)
{
    tVideoResult result;
    _init_tResult(&result);
    QTime t;
      t.start();


    //capture d'une image du flux video
    capture->grab();

    //récupération de l'image
    bool captureOK=capture->retrieve(m_frame,0);

    //l'image a-t-elle bien été récupérée
    if (captureOK)
    {
        //dans le cas ou la configuration de la caméra aurait mal fonctionné
        if(!parameterConfirmed)
            _video_confirm_parameters(m_frame);

        //clone de l'image pour la persistence des données
        m_frameCloned=m_frame.clone();

        if(parameter.record)
            _video_record(m_frame);//on enregistre le flux video

        //analyse de l'image
        cv::Mat inputImage=m_frameCloned.clone();

        std::vector<int> markerIds;
        std::vector<std::vector<cv::Point2f>> markerCorners, rejectedCandidates;
        std::vector< cv::Vec3d > rvecs, tvecs;

        cv::Ptr<cv::aruco::Dictionary> dictionary = cv::aruco::getPredefinedDictionary(cv::aruco::DICT_4X4_100);
        cv::aruco::detectMarkers(inputImage, dictionary, markerCorners, markerIds);


        if (markerIds.size() > 0)
        {
            //on dessine les marqueurs trouvés
            if (m_dbg_active)
            cv::aruco::drawDetectedMarkers(m_frameCloned, markerCorners, markerIds);
            //on estime leur position en 3D par rapport à la caméra
            cv::aruco::estimatePoseSingleMarkers(markerCorners, markerLength, camMatrix, distCoeffs, rvecs,tvecs);
            //on donne la valeur normale à la caméra dans la console pour cahque marqueur
            for(unsigned int i = 0; i < markerIds.size(); i++)
            {
                if((markerIds.at(i)==1)||(markerIds.at(i)==2))
                {
                    cv::aruco::drawAxis(m_frameCloned, camMatrix, distCoeffs, rvecs[i], tvecs[i], markerLength * 0.5f);
                    double dist=sqrt(tvecs[i][0]*tvecs[i][0]+tvecs[i][1]*tvecs[i][1]+tvecs[i][2]*tvecs[i][2]);
                    result.value[IDX_ROBOT1_DIST] = dist;
                    float teta=0;
                    if(tvecs[i][1]!=0)
                        teta=acos(tvecs[i][0]/tvecs[i][1]);
                    result.value[IDX_ROBOT1_ANGLE]=teta;
                    /*result.markers_detected = markerIds;*/
                }

                if((markerIds.at(i)==3)||(markerIds.at(i)==4))
                {
                    cv::aruco::drawAxis(m_frameCloned, camMatrix, distCoeffs, rvecs[i], tvecs[i], markerLength * 0.5f);
                    double dist=sqrt(tvecs[i][0]*tvecs[i][0]+tvecs[i][1]*tvecs[i][1]+tvecs[i][2]*tvecs[i][2]);
                    result.value[IDX_ROBOT2_DIST] = dist;
                    float teta=0;
                    if(tvecs[i][1]!=0)
                        teta=acos(tvecs[i][0]/tvecs[i][1]);
                    result.value[IDX_ROBOT2_ANGLE]=teta;
                    /*result.markers_detected = markerIds;*/
                }
            }
        }

        int fps=1000/(t.elapsed());
        result.m_fps=fps;

        //TIMESTAMP_MATCH.Timestamp

        QImage imgConst(m_iL,m_iH,QImage::Format_RGB888); //image du buffer video au format Qt

        //on affiche l'image traitée
        if (m_dbg_active)
        {
            //recuperation des donnees de la frame
            //et referencement dans l'image Qt
            //ATTENTION: pas de copie des donnees -> garbage collector
            //conversion de l'image en RGB
            cv::Mat frame_converted;
            frame_converted=m_frame.clone();
            cv::cvtColor(m_frameCloned,frame_converted, CV_BGR2RGB);

            //affichage overlay
            char str[200];
            sprintf(str,"%d fps",fps);
            cv::putText(frame_converted, str, cv::Point2f(20,20), cv::FONT_HERSHEY_PLAIN, 2,  cv::Scalar(255,0,0,0));

            const uchar *qImageBuffer = (const uchar*)frame_converted.data;
            //const uchar *qImageBuffer = (const uchar*)m_frameCloned.data;
            //cv::imwrite("tata.jpg",frame);
            QImage img(qImageBuffer, m_iL, m_iH, QImage::Format_RGB888);

            //copie complete de l'image pour eviter une desallocation sauvage de la frame
            imgConst=img.copy(QRect(0,0,m_iL,m_iH));
             frame_converted.release();
        }

        emit resultReady(result,imgConst);

        //QThread::msleep(5);
        inputImage.release();

       //opencv ne profite pas du garbage collector de Qt
        m_frame.release();
        m_frameCloned.release();
    }
    else
    {
        //opencv ne profite pas du garbage collector de Qt
        m_frame.release();
        m_frameCloned.release();
    }
}

// ========================================================
void VideoWorker::_video_process_NordSud(tVideoInput parameter)
{
    tVideoResult result;
    _init_tResult(&result);

    QTime t;
      t.start();


    //capture d'une image du flux video
    capture->grab();

    //récupération de l'image
    bool captureOK=capture->retrieve(m_frame,0);

    //l'image a-t-elle bien été récupérée
    if (captureOK)
    {
        //dans le cas ou la configuration de la caméra aurait mal fonctionné
        if(!parameterConfirmed)
            _video_confirm_parameters(m_frame);

        //clone de l'image pour la persistence des données
        m_frameCloned=m_frame.clone();

        if(parameter.record)
            _video_record(m_frame);//on enregistre le flux video

        //analyse de l'image
        cv::Mat inputImage=m_frameCloned.clone();

        std::vector<int> markerIds;
        std::vector<std::vector<cv::Point2f>> markerCorners, rejectedCandidates;
        std::vector< cv::Vec3d > rvecs, tvecs;

        cv::Ptr<cv::aruco::Dictionary> dictionary = cv::aruco::getPredefinedDictionary(cv::aruco::DICT_4X4_100);
        cv::aruco::detectMarkers(inputImage, dictionary, markerCorners, markerIds);


        if (markerIds.size() > 0)
        {

            //on dessine les marqueurs trouvés
            if (m_dbg_active)
            cv::aruco::drawDetectedMarkers(m_frameCloned, markerCorners, markerIds);
            //on estime leur position en 3D par rapport à la caméra
            cv::aruco::estimatePoseSingleMarkers(markerCorners, markerLength, camMatrix, distCoeffs, rvecs,tvecs);
            //on donne la valeur normale à la caméra dans la console pour cahque marqueur
            /*
            for(unsigned int i = 0; i < markerIds.size(); i++)
            {
                cv::aruco::drawAxis(m_frameCloned, camMatrix, distCoeffs, rvecs[i], tvecs[i], markerLength * 0.5f);
                qDebug() << 100*tvecs[0][2] << "cm";
            }
            */
            for(unsigned int i = 0; i < markerIds.size(); i++)
            {
                //La girouette a le marqueur n°17
                if(markerIds.at(i)==17)
                {
                    cv::aruco::drawAxis(m_frameCloned, camMatrix, distCoeffs, rvecs[i], tvecs[i], markerLength * 0.5f);

                    //double dist=sqrt(tvecs[i][0]*tvecs[i][0]+tvecs[i][1]*tvecs[i][1]+tvecs[i][2]*tvecs[i][2]);
                    if(fabs(rvecs[i][0])>2.8)
                        result.value[IDX_NORD]=1.0;
                    else
                        result.value[IDX_NORD]=0.0;

                    if(fabs(rvecs[i][1])>2.8)
                        result.value[IDX_SUD]=1.0;
                    else
                        result.value[IDX_SUD]=0.0;

                    /*result.value[IDX_ROBOT1_DIST] = tvecs[i][0];
                    result.value[IDX_ROBOT2_DIST] = tvecs[i][1];
                    result.value[IDX_ROBOT3_DIST] = tvecs[i][2];
                    result.value[IDX_ROBOT1_ANGLE] = rvecs[i][0];
                    result.value[IDX_ROBOT2_ANGLE] = rvecs[i][1];
                    result.value[IDX_ROBOT3_ANGLE] = rvecs[i][2];*/
                    /*result.markers_detected = markerIds;*/
                }
            }
        }

        int fps=1000/(t.elapsed());
        result.m_fps=fps;

        //TIMESTAMP_MATCH.Timestamp

        QImage imgConst(m_iL,m_iH,QImage::Format_RGB888); //image du buffer video au format Qt

        //on affiche l'image traitée
        if (m_dbg_active)
        {
            //recuperation des donnees de la frame
            //et referencement dans l'image Qt
            //ATTENTION: pas de copie des donnees -> garbage collector
            //conversion de l'image en RGB
            cv::Mat frame_converted;
            frame_converted=m_frame.clone();
            cv::cvtColor(m_frameCloned,frame_converted, CV_BGR2RGB);

            //affichage overlay
            char str[200];
            sprintf(str,"%d fps",fps);
            cv::putText(frame_converted, str, cv::Point2f(20,20), cv::FONT_HERSHEY_PLAIN, 2,  cv::Scalar(255,0,0,0));

            const uchar *qImageBuffer = (const uchar*)frame_converted.data;
            //const uchar *qImageBuffer = (const uchar*)m_frameCloned.data;
            //cv::imwrite("tata.jpg",frame);
            QImage img(qImageBuffer, m_iL, m_iH, QImage::Format_RGB888);

            //copie complete de l'image pour eviter une desallocation sauvage de la frame
            imgConst=img.copy(QRect(0,0,m_iL,m_iH));
             frame_converted.release();
        }

        emit resultReady(result,imgConst);

        //QThread::msleep(5);
        inputImage.release();

       //opencv ne profite pas du garbage collector de Qt
        m_frame.release();
        m_frameCloned.release();
    }
    else
    {
        //opencv ne profite pas du garbage collector de Qt
        m_frame.release();
        m_frameCloned.release();
    }
}

// ========================================================
void VideoWorker::_video_process_ColorSequence(tVideoInput parameter)
{
    /*
    http://fr.wikipedia.org/wiki/Teinte_Saturation_Valeur
    Le modèle TSV pour Teinte Saturation Valeur (en anglais HSV pour Hue Saturation Value),
    est un système de gestion des couleurs en informatique.

    Teinte ou "H":
    La teinte est codée suivant l'angle qui lui correspond sur le cercle des couleurs ==>
    0° ou 360° : rouge ;
    60° : jaune ;
    120° : vert ;
    180° : cyan ;
    240° : bleu ;
    300° : magenta;

    Saturation ou "S":
    La saturation est l'« intensité » de la couleur :
    elle varie entre 0 et 100 % ;
    elle est parfois appelée « pureté » ;
    plus la saturation d'une couleur est faible, plus l'image sera « grisée » et plus elle apparaîtra fade

    Valeur ou "V"
    La valeur est la « brillance » de la couleur :
    elle varie entre 0 et 100 %
    plus la valeur d'une couleur est faible, plus la couleur est sombre. Une valeur de 0 correspond au noir.
    */

    tVideoResult result;
    _init_tResult(&result);
    QTime t;
      t.start();


    //capture d'une image du flux video
    capture->grab();

    //récupération de l'image
    bool captureOK=capture->retrieve(m_frame,0);

    //l'image a-t-elle bien été récupérée
    if (captureOK)
    {
        //images temporaires pour le traitement video
        cv::Mat frameColor;
        cv::Mat frameHSV;
        cv::Mat frameBlur;
        cv::Mat frameGray;
        cv::Mat frameSobel;

        //dans le cas ou la configuration de la caméra aurait mal fonctionné
        if(!parameterConfirmed)
            _video_confirm_parameters(m_frame);

        //clone de l'image pour la persistence des données
        m_frameCloned=m_frame.clone();

        if(parameter.record)
            _video_record(m_frame);//on enregistre le flux video

        //analyse de l'image
        cv::Mat inputImage=m_frameCloned.clone();

        //OpenCV travaille en BGR, on convertit donc en RGB
        cv::cvtColor(inputImage, frameColor, CV_BGR2RGB);
        //on clone l'image couleur pour les différents traitement couleur
        frameBlur=frameColor.clone();

        //conversion en N&B pour les traitements binaires de l'image
        cv::cvtColor(inputImage,frameGray,CV_BGR2GRAY);
        //on clone l'image N&B pour les différents traitement N&B
        frameSobel=frameGray.clone();

        //floutage de l'image couleur pour enlever les parasites
        cv::GaussianBlur(frameColor,frameBlur,cv::Size(9,9),10.0,0,cv::BORDER_DEFAULT);

        //Conversion de l'image floutée dans l'espace colorimetrique HSV
        cv::cvtColor(frameBlur,frameHSV,CV_RGB2HSV);

        //Recherche de la séquence de couleur
        cv::Mat frameGrey_red=frameGray.clone();
        cv::Mat frameGrey_green=frameGray.clone();
        cv::Mat frameHSV_red=frameHSV.clone();
        cv::Mat frameHSV_green=frameHSV.clone();

        //Nombre de pixels minimum à détecter
        int nb_pixels_minimum=50;

        //pour stocker les résultats
        cv::Point centre_couleur(0,0);
        QList< QPair<QString,int> > combinaison;

        //MAJ du zonage
        //TODO récupérer des paramètres
        int x_traitement=0;
        int y_traitement=0;
        int H_traitement=m_iH;
        int L_traitement=m_iL;

        int iPureteRed=0;
        int iEcartPureteRed=0;
        int iTeinteRed=0;
        int iPureteGreen=0;
        int iEcartPureteGreen=0;
        int iTeinteGreen=120;

        //rouge
        //S_min=255*S_purete_detection/100;
        //H_median=30;
        _seuillageImage(&frameHSV_red,&frameGrey_red,Qt::red,255,(int)(iPureteRed/2.55),iEcartPureteRed);
        centre_couleur=_isColor(&frameGrey_red,x_traitement,y_traitement,H_traitement,L_traitement,nb_pixels_minimum);
        if((centre_couleur.x!=0)&&(centre_couleur.y!=0))
        {
           QPair<QString,int> O("O",centre_couleur.x);
           combinaison.append(O);
           cv::circle( frameColor, centre_couleur, 20, CV_RGB(0,128,255), -1, 8, 0 );
        }

        //vert
        //H_median=120;
        _seuillageImage(&frameHSV_green,&frameGrey_green,Qt::green,255,(int)(iPureteGreen/2.55),iEcartPureteGreen);
        centre_couleur=_isColor(&frameGrey_green,x_traitement,y_traitement,H_traitement,L_traitement,nb_pixels_minimum);
        if((centre_couleur.x!=0)&&(centre_couleur.y!=0))
        {
           QPair<QString,int> V("V",centre_couleur.x);
           combinaison.append(V);
           cv::circle( frameColor, centre_couleur, 20, CV_RGB(0,255,0), -1, 8, 0 );
        }


        int fps=1000/(t.elapsed());
        result.m_fps=fps;

        //TIMESTAMP_MATCH.Timestamp

        QImage imgConst(m_iL,m_iH,QImage::Format_RGB888); //image du buffer video au format Qt

        //on affiche l'image traitée
        if (m_dbg_active)
        {
            //recuperation des donnees de la frame
            //et referencement dans l'image Qt
            //ATTENTION: pas de copie des donnees -> garbage collector
            //conversion de l'image en RGB
            cv::Mat frame_converted;
            frame_converted=m_frame.clone();
            cv::cvtColor(m_frameCloned,frame_converted, CV_BGR2RGB);

            //affichage overlay
            char str[200];
            sprintf(str,"%d fps",fps);
            cv::putText(frame_converted, str, cv::Point2f(20,20), cv::FONT_HERSHEY_PLAIN, 2,  cv::Scalar(255,0,0,0));

            const uchar *qImageBuffer = (const uchar*)frame_converted.data;
            //const uchar *qImageBuffer = (const uchar*)m_frameCloned.data;
            //cv::imwrite("tata.jpg",frame);
            QImage img(qImageBuffer, m_iL, m_iH, QImage::Format_RGB888);

            //copie complete de l'image pour eviter une desallocation sauvage de la frame
            imgConst=img.copy(QRect(0,0,m_iL,m_iH));
             frame_converted.release();
        }

        emit resultReady(result,imgConst);

        //QThread::msleep(5);
        inputImage.release();

       //opencv ne profite pas du garbage collector de Qt
        m_frame.release();
        m_frameCloned.release();
        frameGrey_red.release();
        frameGrey_green.release();
        frameHSV_red.release();
        frameHSV_green.release();
        frameColor.release();
        frameBlur.release();
        frameGray.release();
        frameSobel.release();
        frameHSV.release();

    }
    else
    {
        //opencv ne profite pas du garbage collector de Qt
        m_frame.release();
        m_frameCloned.release();
    }
}

void VideoWorker::_video_record(cv::Mat frametoRecord)
{
    if(!recordInitialized)
    {
        //construction du nom de fichier d'enregistrement de la video
        QString pathfilename = getVideoLogFilename();

        //création de l'objet OpenCV qui enregistre des frame
        record= new cv::VideoWriter(pathfilename.toStdString(),CV_FOURCC('X','V','I','D'),15,cv::Size(m_iL,m_iH),true);
    }
    record->write(frametoRecord);//on enregistre le flux video
}

void VideoWorker::_video_confirm_parameters(cv::Mat frameSample)
{
    //recuperation des hauteurs et largeur de l'image
    m_iH = frameSample.rows;
    m_iL = frameSample.cols;
    //qDebug() << "Récupération des paramètres Vidéos";
    //qDebug() << "Résolution " << m_iH << "x" << m_iL;
    parameterConfirmed=true;
}

// _____________________________________________________________________
QString VideoWorker::getVideoLogFilename()
{
    QString path = "./Capture";
    QDir dir(path);
    if (!dir.exists()) {
        dir.mkdir(path);
    }

    QString pathfilename;
    for (int i=1; i<10000; i++) {
        pathfilename =  path +
                        "/Match_" +
                        QString::number(i) +
                        ".avi";
        QFileInfo fileinfo(pathfilename);
        if (!fileinfo.exists()) break;
    }
    return pathfilename;
}

void VideoWorker::_seuillageImage(cv::Mat* frameHSV, cv::Mat* frameGray,
                             int Couleur, int Saturation,int Purete, int EcartCouleur)
{
    int iH = frameHSV->rows;
    int iL = frameHSV->cols;
    int x, y;

    //Quelle couleur?
    int angleCouleur=0;
    switch(Couleur){
        case Qt::red : angleCouleur=0;break;
        case Qt::yellow : angleCouleur=60;break;
        case Qt::green : angleCouleur=120;break;
        case Qt::cyan : angleCouleur=180;break;
        case Qt::blue : angleCouleur=240;break;
        case Qt::magenta : angleCouleur=300;break;
    default : angleCouleur=0;break;
    }

    //On considere un ecart potentiel par rapport a la couleur souhaitee
    int angleCouleurMin,angleCouleurMax;
    int angleCouleurMin_temp=(angleCouleur-(EcartCouleur/2))/2;
    if (angleCouleurMin_temp<0) angleCouleurMin=360+angleCouleurMin_temp;
    else angleCouleurMin=angleCouleurMin_temp;
    angleCouleurMax=(angleCouleur+(EcartCouleur/2))/2;

    //seuillage de l'image en extrayant les couleurs des items de l'image HSV
    //si supérieur à la purete, inferieur à l'angle max et superieur a l'angle min alors le pixel nous interesse
    //methode alternative: determiner un histogramme caracteristique de l'element et l'extraire de l'image
    for (y = 0; y < iH; y++) {
        for (x = 0; x < iL; x++) {
            uchar angleCouleur_pixel=((uchar*)(frameHSV->data + frameHSV->step*y))[x*3];
            uchar Purete_pixel=((uchar*)(frameHSV->data + frameHSV->step*y))[x*3+1];
            uchar Saturation_pixel=((uchar*)(frameHSV->data + frameHSV->step*y))[x*3+2]; //pour le seuillage noir et blanc si on veut

            if(Couleur==Qt::black)
            {
                //detection couleur quelconque
                if (Saturation_pixel<Saturation)
                    ((uchar*)(frameGray->data + frameGray->step*y))[x] = 255;
                else ((uchar*)(frameGray->data + frameGray->step*y))[x] = 0;
            }
            else if(angleCouleurMin_temp<0)
            {
                //detection a la limite du modulo 2pi (pour le rouge par exemple)
                if (((angleCouleurMin < angleCouleur_pixel)
                        || (angleCouleur_pixel < angleCouleurMax))
                        && (Purete_pixel > Purete))
                    ((uchar*)(frameGray->data + frameGray->step*y))[x] = 255;
                else ((uchar*)(frameGray->data + frameGray->step*y))[x] = 0;
            }
            else
            {
                //detection couleur quelconque
                if ((angleCouleurMin < angleCouleur_pixel)
                        && (angleCouleur_pixel < angleCouleurMax)
                        && (Purete_pixel > Purete))
                    ((uchar*)(frameGray->data + frameGray->step*y))[x] = 255;
                else ((uchar*)(frameGray->data + frameGray->step*y))[x] = 0;
            }
        }
    }
}

cv::Point VideoWorker::_isColor(cv::Mat* frameGray,int ROIx, int ROIy, int ROIh, int ROIl,int seuil)
{
    cv::Point centre_couleur;
    centre_couleur.x=0;
    centre_couleur.y=0;
    int detect=0;
    for (int Y = ROIy; Y < ROIy+ROIh; Y++) {
        for (int X = ROIx; X < ROIx+ROIl; X++) {

            if(((uchar*)(frameGray->data + frameGray->step*Y))[X] == 255)
            {
                centre_couleur.x=centre_couleur.x+X;
                centre_couleur.y=centre_couleur.y+Y;
                detect++;
            }
        }
    }

    if(detect>seuil){
        centre_couleur.x=centre_couleur.x/detect;
        centre_couleur.y=centre_couleur.y/detect;
    }
    else{
        centre_couleur.x=0;
        centre_couleur.y=0;
    }

    return (centre_couleur);
}

void VideoWorker::_init_tResult(tVideoResult * parameter)
{
    /*parameter->robot1_dist=0.;
    parameter->robot1_angle=0.;
    parameter->robot2_dist=0.;
    parameter->robot2_angle=0.;
    parameter->robot3_dist=0.;
    parameter->robot3_angle=0.;*/
    parameter->m_fps=0;
    for(int i=0;i<20;i++)
        parameter->value[i]=0.;
}

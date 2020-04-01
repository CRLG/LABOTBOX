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
    for(int i=0;i<20;i++)
        m_internal_param[i]=0;
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

            case VIDEO_PROCESS_CALIBRATION :
                _video_process_Calibration(parameter);
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
    ATTENTION ramené de 0 à 180 dans opencv

    Saturation ou "S":
    La saturation est l'« intensité » de la couleur :
    elle varie entre 0 et 100 % ;
    elle est parfois appelée « pureté » ;
    plus la saturation d'une couleur est faible, plus l'image sera « grisée » et plus elle apparaîtra fade
    ATTENTION codé sur un octet dans opencv (0/255)

    Valeur ou "V"
    La valeur est la « brillance » de la couleur :
    elle varie entre 0 et 100 %
    plus la valeur d'une couleur est faible, plus la couleur est sombre. Une valeur de 0 correspond au noir.
    ATTENTION codé sur un octet dans opencv (0/255)
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
        cv::Mat inputImage;
        cv::Mat3b frameHSV;
        cv::Mat3b inputRGB;
        cv::Mat3b maskedRGB;
        cv::Mat3b maskedRed;
        cv::Mat3b maskedGreen;
        cv::Mat1b maskRGB;
        cv::Mat1b maskRed;
        cv::Mat1b maskRed_1;
        cv::Mat1b maskRed_2;
        cv::Mat1b maskGreen;
        cv::Mat3b frameBlur;
        cv::Mat3b frameAffichage;

        //dans le cas ou la configuration de la caméra aurait mal fonctionné
        if(!parameterConfirmed)
            _video_confirm_parameters(m_frame);

        //clone de l'image pour la persistence des données
        m_frameCloned=m_frame.clone();

        if(parameter.record)
            _video_record(m_frame);//on enregistre le flux video

        //clone de l'image acquise de l'image
        inputImage=m_frameCloned.clone();

        //conversion en RGB pour l'affichage
        cv::cvtColor(inputImage,inputRGB,CV_BGR2RGB);

        //init des images filtrées
        maskedRGB=cv::Mat::zeros(inputImage.rows, inputImage.cols, CV_32F);
        maskedRed=cv::Mat::zeros(inputImage.rows, inputImage.cols, CV_32F);
        maskedGreen=cv::Mat::zeros(inputImage.rows, inputImage.cols, CV_32F);

        //floutage de l'image couleur pour enlever les pixels parasites
        cv::GaussianBlur(inputImage,frameBlur,cv::Size(9,9),10.0,0,cv::BORDER_DEFAULT);

        //Conversion de l'image floutée dans l'espace colorimetrique HSV
        cv::cvtColor(frameBlur,frameHSV,CV_BGR2HSV);

        //pour stocker les résultats
        QList<int> combinaison_color;
        QList<int> combinaison_x;
        QList<int> combinaison_width;

        //MAJ du zonage
        //TODO récupérer des paramètres
        /*int x_traitement=0;
        int y_traitement=0;
        int H_traitement=m_iH;
        int L_traitement=m_iL;*/

        //Nombre de pixels minimum à détecter
        int nb_pixels_minimum=1000;
        int nb_pixels_maximum=50000;
        int iPureteRed=parameter.value[IDX_PARAM_PURETE_ROUGE];
        int iEcartTeinteRed=parameter.value[IDX_PARAM_ECART_ROUGE];
        int iPureteGreen=parameter.value[IDX_PARAM_PURETE_VERT];
        int iEcartTeinteGreen=parameter.value[IDX_PARAM_ECART_VERT];

        cv::Scalar minHSV = cv::Scalar(60-iEcartTeinteGreen, 255-iPureteGreen, 50);
        cv::Scalar maxHSV = cv::Scalar(60+iEcartTeinteGreen, 255, 255);
        cv::inRange(frameHSV, minHSV, maxHSV, maskGreen);
        cv::bitwise_and(inputImage, inputImage, maskedGreen, maskGreen);

        minHSV = cv::Scalar(0, 255-iPureteRed, 50);
        maxHSV = cv::Scalar(0+iEcartTeinteRed, 255, 255);
        cv::inRange(frameHSV, minHSV, maxHSV, maskRed_1);
        minHSV = cv::Scalar(180-iEcartTeinteRed, 255-iPureteRed, 50);
        maxHSV = cv::Scalar(180, 255, 255);
        cv::inRange(frameHSV, minHSV, maxHSV, maskRed_2);
        maskRed=maskRed_1|maskRed_2;
        cv::bitwise_and(inputImage, inputImage, maskedRed, maskRed);

        maskRGB = maskGreen | maskRed;
        cv::bitwise_and(inputRGB, inputRGB, maskedRGB, maskRGB);

        //séquence qui contiendra les contours des éléments verts trouvés
        std::vector < std::vector<cv::Point> > contoursGreen;
        cv::findContours(maskGreen,contoursGreen,CV_RETR_CCOMP,CV_CHAIN_APPROX_SIMPLE);

        for (size_t contourIdx = 0; contourIdx < contoursGreen.size(); contourIdx++)
        {
            std::vector<cv::Point> approx;
            cv::approxPolyDP(contoursGreen[contourIdx], approx, 5, true);
            double aire_detectee = cv::contourArea(approx);
            if((aire_detectee>nb_pixels_minimum)&&(aire_detectee<nb_pixels_maximum))
            {
                //Rectangle circonscrit de la forme détectée
                cv::Rect RectCirconscrit=cv::boundingRect( approx );

                //barycentre du rectangle
                int centre_x=RectCirconscrit.x+RectCirconscrit.width/2;
                combinaison_x << centre_x;
                combinaison_color << Qt::green;
                combinaison_width << RectCirconscrit.width;
                //float centre_y=RectCirconscrit.y+RectCirconscrit.height/2;
                //QPoint centre=QPoint((centre_x/m_iL)*100,(centre_y/m_iH)*100);

                cv::rectangle(inputRGB,RectCirconscrit,CV_RGB(0,255,0),5,8,0);
            }
        }

        //séquence qui contiendra les contours des éléments verts trouvés
        std::vector < std::vector<cv::Point> > contoursRed;
        cv::findContours(maskRed,contoursRed,CV_RETR_CCOMP,CV_CHAIN_APPROX_SIMPLE);

        for (size_t contourIdx = 0; contourIdx < contoursRed.size(); contourIdx++)
        {
            std::vector<cv::Point> approx;
            cv::approxPolyDP(contoursRed[contourIdx], approx, 5, true);
            double aire_detectee = cv::contourArea(approx);
            if((aire_detectee>nb_pixels_minimum)&&(aire_detectee<nb_pixels_maximum))
            {
                //Rectangle circonscrit de la forme détectée
                cv::Rect RectCirconscrit=cv::boundingRect( approx );

                //barycentre du rectangle
                int centre_x=RectCirconscrit.x+RectCirconscrit.width/2;
                combinaison_x << centre_x;
                combinaison_color << Qt::red;
                combinaison_width << RectCirconscrit.width;
                //float centre_y=RectCirconscrit.y+RectCirconscrit.height/2;
                //QPoint centre=QPoint((centre_x/m_iL)*100,(centre_y/m_iH)*100);

                cv::rectangle(inputRGB,RectCirconscrit,CV_RGB(0,0,255),5,8,0);
            }
        }


        //on analyse les résultats
        if(!combinaison_x.isEmpty())
        {
            //on réordonne selon les x croissants
            for(int i=combinaison_x.count()-1;i>0;i--)
                for(int j=0;j<i;j++)
                {
                    if(combinaison_x[j+1] < combinaison_x[j])
                    {
                        combinaison_x.swap(j+1,j);
                        combinaison_color.swap(j+1,j);
                        combinaison_width.swap(j+1,j);
                    }
                }

            /*for(int i=0;i<combinaison_x.count();i++)
                qDebug() << (combinaison_color[i]==Qt::green?"VERT ":"ROUGE ") << combinaison_width[i];
            qDebug() << "\n\n";*/

            //coupe 2020 : cf règlement page 11
            //CAS N°1:
            //côté BLEU: GRGGR   côté JAUNE: GRRGR
            //CAS N°2:
            //côté BLEU: GGRGR   côté JAUNE: GRGRR
            //CAS N°3:
            //côté BLEU: GGGRR   côté JAUNE: GGRRR

            //largeur de tous les gobelets détectés
            int largeur_totale=0;
            for(int i=0;i<combinaison_width.count();i++)
                largeur_totale=largeur_totale+combinaison_width[i];
            //largeur moyenne d'un gobelet
            int largeur_gobelet = largeur_totale/5;
            QString str_combinaison;
            for(int i=0;i<combinaison_width.count();i++)
            {
                float nb_gobelet_estim = combinaison_width[i] / largeur_gobelet;
                if(nb_gobelet_estim < 1.4)
                    str_combinaison=str_combinaison+(combinaison_color[i]==Qt::green?"G":"R");
                else if((nb_gobelet_estim > 1.6)&&(nb_gobelet_estim < 2.4))
                    str_combinaison=str_combinaison+(combinaison_color[i]==Qt::green?"GG":"RR");
                else
                    str_combinaison=str_combinaison+(combinaison_color[i]==Qt::green?"GGG":"RRR");
            }

            //qDebug() << str_combinaison <<"\n";
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

            //affichage overlay
            char str[200];
            sprintf(str,"%d fps",fps);
            cv::putText(maskedRGB, str, cv::Point2f(20,20), cv::FONT_HERSHEY_PLAIN, 2,  cv::Scalar(255,0,0,0));

            const uchar *qImageBuffer = (const uchar*)inputRGB.data;
            QImage img(qImageBuffer, m_iL, m_iH, QImage::Format_RGB888);
            //const uchar *qImageBuffer = (const uchar*)frameGrey_green.data;
            //QImage img(qImageBuffer, m_iL, m_iH, QImage::Format_Grayscale8);

            //copie complete de l'image pour eviter une desallocation sauvage de la frame
            imgConst=img.copy(QRect(0,0,m_iL,m_iH));
        }

        emit resultReady(result,imgConst);

        //QThread::msleep(5);
        inputImage.release();

       //opencv ne profite pas du garbage collector de Qt
        m_frame.release();
        m_frameCloned.release();
        inputImage.release();
        inputRGB.release();
        frameBlur.release();
        frameHSV.release();
        maskedRGB.release();
        maskedRed.release();
        maskedGreen.release();
        maskRGB.release();
        maskRed.release();
        maskRed_1.release();
        maskRed_2.release();
        maskGreen.release();
        frameAffichage.release();
    }
    else
    {
        //opencv ne profite pas du garbage collector de Qt
        m_frame.release();
        m_frameCloned.release();
    }
}

void VideoWorker::_video_process_Calibration(tVideoInput parameter)
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
        //images temporaires pour le traitement video
        cv::Mat inputImage;
        cv::Mat3b frameHSV;
        cv::Mat3b inputRGB;
        cv::Mat3b maskedRGB;
        cv::Mat3b maskedRed;
        cv::Mat3b maskedGreen;
        cv::Mat1b maskRGB;
        cv::Mat1b maskRed;
        cv::Mat1b maskRed_1;
        cv::Mat1b maskRed_2;
        cv::Mat1b maskGreen;
        cv::Mat3b frameBlur;
        cv::Mat3b frameAffichage;

        //dans le cas ou la configuration de la caméra aurait mal fonctionné
        if(!parameterConfirmed)
            _video_confirm_parameters(m_frame);

        //clone de l'image pour la persistence des données
        m_frameCloned=m_frame.clone();

        if(parameter.record)
            _video_record(m_frame);//on enregistre le flux video

        //clone de l'image acquise de l'image
        inputImage=m_frameCloned.clone();

        //conversion en RGB pour l'affichage
        cv::cvtColor(inputImage,inputRGB,CV_BGR2RGB);

        //init des images filtrées
        maskedRGB=cv::Mat::zeros(inputImage.rows, inputImage.cols, CV_32F);
        maskedRed=cv::Mat::zeros(inputImage.rows, inputImage.cols, CV_32F);
        maskedGreen=cv::Mat::zeros(inputImage.rows, inputImage.cols, CV_32F);

        //floutage de l'image couleur pour enlever les pixels parasites
        cv::GaussianBlur(inputImage,frameBlur,cv::Size(9,9),10.0,0,cv::BORDER_DEFAULT);

        //Conversion de l'image floutée dans l'espace colorimetrique HSV
        cv::cvtColor(frameBlur,frameHSV,CV_BGR2HSV);

        //Nombre de pixels minimum à détecter
        int nb_pixels_minimum=m_internal_param[IDX_PARAM_PIXEL_MIN];
        int nb_pixels_maximum=m_internal_param[IDX_PARAM_PIXEL_MAX];
        int iPureteRed=m_internal_param[IDX_PARAM_PURETE_ROUGE];
        int iEcartTeinteRed=m_internal_param[IDX_PARAM_ECART_ROUGE];
        int iPureteGreen=m_internal_param[IDX_PARAM_PURETE_VERT];
        int iEcartTeinteGreen=m_internal_param[IDX_PARAM_ECART_VERT];

        cv::Scalar minHSV = cv::Scalar(60-iEcartTeinteGreen, 255-iPureteGreen, 50);
        cv::Scalar maxHSV = cv::Scalar(60+iEcartTeinteGreen, 255, 255);
        cv::inRange(frameHSV, minHSV, maxHSV, maskGreen);
        cv::bitwise_and(inputImage, inputImage, maskedGreen, maskGreen);

        minHSV = cv::Scalar(0, 255-iPureteRed, 50);
        maxHSV = cv::Scalar(0+iEcartTeinteRed, 255, 255);
        cv::inRange(frameHSV, minHSV, maxHSV, maskRed_1);
        minHSV = cv::Scalar(180-iEcartTeinteRed, 255-iPureteRed, 50);
        maxHSV = cv::Scalar(180, 255, 255);
        cv::inRange(frameHSV, minHSV, maxHSV, maskRed_2);
        maskRed=maskRed_1|maskRed_2;
        cv::bitwise_and(inputImage, inputImage, maskedRed, maskRed);

        maskRGB = maskGreen | maskRed;
        cv::bitwise_and(inputRGB, inputRGB, maskedRGB, maskRGB);

        //séquence qui contiendra les contours des éléments verts trouvés
        std::vector < std::vector<cv::Point> > contoursGreen;
        cv::findContours(maskGreen,contoursGreen,CV_RETR_CCOMP,CV_CHAIN_APPROX_SIMPLE);

        for (size_t contourIdx = 0; contourIdx < contoursGreen.size(); contourIdx++)
        {
            std::vector<cv::Point> approx;
            cv::approxPolyDP(contoursGreen[contourIdx], approx, 5, true);
            double aire_detectee = cv::contourArea(approx);
            if((aire_detectee>nb_pixels_minimum)&&(aire_detectee<nb_pixels_maximum))
            {
                //Rectangle circonscrit de la forme détectée
                cv::Rect RectCirconscrit=cv::boundingRect( approx );
                cv::rectangle(maskedRGB,RectCirconscrit,CV_RGB(0,255,0),5,8,0);

                //barycentre du rectangle
                //float centre_x=RectCirconscrit.x+RectCirconscrit.width/2;
                //float centre_y=RectCirconscrit.y+RectCirconscrit.height/2;
                //QPoint centre=QPoint((centre_x/m_iL)*100,(centre_y/m_iH)*100);
            }
        }

        //séquence qui contiendra les contours des éléments verts trouvés
        std::vector < std::vector<cv::Point> > contoursRed;
        cv::findContours(maskRed,contoursRed,CV_RETR_CCOMP,CV_CHAIN_APPROX_SIMPLE);

        for (size_t contourIdx = 0; contourIdx < contoursRed.size(); contourIdx++)
        {
            std::vector<cv::Point> approx;
            cv::approxPolyDP(contoursRed[contourIdx], approx, 5, true);
            double aire_detectee = cv::contourArea(approx);
            if((aire_detectee>nb_pixels_minimum)&&(aire_detectee<nb_pixels_maximum))
            {
                //Rectangle circonscrit de la forme détectée
                cv::Rect RectCirconscrit=cv::boundingRect( approx );
                cv::rectangle(maskedRGB,RectCirconscrit,CV_RGB(0,0,255),5,8,0);

                //barycentre du rectangle
                //float centre_x=RectCirconscrit.x+RectCirconscrit.width/2;
                //float centre_y=RectCirconscrit.y+RectCirconscrit.height/2;
                //QPoint centre=QPoint((centre_x/m_iL)*100,(centre_y/m_iH)*100);
            }
        }

        int fps=1000/(t.elapsed());
        result.m_fps=fps;

        QImage imgConst(m_iL,m_iH,QImage::Format_RGB888); //image du buffer video au format Qt

        //on affiche l'image traitée
        if (m_dbg_active)
        {
            //recuperation des donnees de la frame
            //et referencement dans l'image Qt
            //ATTENTION: pas de copie des donnees -> garbage collector

            //affichage overlay
            char str[200];
            sprintf(str,"%d fps",fps);
            cv::putText(maskedRGB, str, cv::Point2f(20,20), cv::FONT_HERSHEY_PLAIN, 2,  cv::Scalar(255,0,0,0));

            const uchar *qImageBuffer = (const uchar*)maskedRGB.data;
            QImage img(qImageBuffer, m_iL, m_iH, QImage::Format_RGB888);

            //copie complete de l'image pour eviter une desallocation sauvage de la frame
            imgConst=img.copy(QRect(0,0,m_iL,m_iH));
        }

        emit resultReady(result,imgConst);

        //QThread::msleep(5);
        inputImage.release();

       //opencv ne profite pas du garbage collector de Qt
        m_frame.release();
        m_frameCloned.release();
        inputImage.release();
        inputRGB.release();
        frameBlur.release();
        frameHSV.release();
        maskedRGB.release();
        maskedRed.release();
        maskedGreen.release();
        maskRGB.release();
        maskRed.release();
        maskRed_1.release();
        maskRed_2.release();
        maskGreen.release();
        frameAffichage.release();
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

void VideoWorker::_init_tResult(tVideoResult * parameter)
{
    parameter->m_fps=0;
    for(int i=0;i<20;i++)
        parameter->value[i]=0.;
}

#include <math.h>
#include <QDebug>

#include "lidar_data_filter_tracker.h"
#include "lidar_data.h"
#include "Lidar_utils.h"

CLidarDataFilterTracker::CLidarDataFilterTracker()
{

    //Paramétrage
    m_dot_size = m_data_manager.createData("dot_size", 20);
    m_d_dist_offset = m_data_manager.createData("d_dist_offset", 40.);
    m_i_MAX_BLANK = m_data_manager.createData("i_MAX_BLANK", 3);
    m_i_MIN_COUNT = m_data_manager.createData("i_MIN_COUNT", 2);
    m_i_MAX_COUNT = m_data_manager.createData("i_MAX_COUNT", 60);
    m_d_MAX_dist = m_data_manager.createData("d_MAX_dist", 3600.);
    m_d_MIN_dist = m_data_manager.createData("d_MIN_dist", 150.);
    m_d_seuil_filtrage_dist = m_data_manager.createData("d_seuil_filtrage_dist", 150.);
    m_i_seuil_filtrage_angle = m_data_manager.createData("i_seuil_filtrage_angle", 3);
    m_d_seuil_gradient = m_data_manager.createData("d_seuil_gradient", 150.);
    m_d_seuil_facteur_forme = m_data_manager.createData("d_seuil_facteur_forme", 1400.);
}

// _______________________________________________________________
const char *CLidarDataFilterTracker::get_name()
{
    return "Tracker";
}

// _______________________________________________________________
const char *CLidarDataFilterTracker::get_description()
{
    return "Un filtre tracker mis au point par le vénérable Laguiche :)";
}

/*!
  \brief filtre des données brutes du lidar en scannant les données sérialisées
  Le filtre traite les données les unes après les autres sur toute la zone angulaire de détection du Lidar
  Le but du filtre est de découper des nuages de points caractéristiques. Comme on les parcourt les uns après
  les autres, en représentation linéaire, ça formme des sortes de créneaux ==> CREC (CRéneau REception).
  Le filtre va utiliser différents traitements pour former ces CREC et s'affranchir de différents cas de parasitage des données.

  Traitements du filtre:

  * vue à l'infini: naturellement on fixe un horizon de détection (vision à l'infini: on ne distingue plus ce qui est trop loin).
  C'est le cas idéal=rien n'est détecté autour de l'objet détecté et une zone "blanche" se forme autour de lui.
  réglages à faire:
  m_d_MAX_dist->read().toDouble() qui est l'horizon de l'infini, toute mesure au dealà ne sera pas prise en compte
  m_i_MAX_BLANK->read().toInt() qui une sorte de contraste angulaire, la largeur de la zone "blanche" qui permet de distinguer chaque objet

  * problème de masquage:
  Rien ne ressemble plus à une mesure qu'une autre mesure, cependant on peut considérer quand l'écart de mesure
  est significatif et durable qu'on ne mesure plus le même objet
  On peut donc filtrer les forts gradients de distance pour distinguer le fond ou les objets lointains

  * filtrage de la taille de l'objet:
  Le support balise des robots est normé et ne peut donc excéder une certaine taille. De même les CREC trop petits sont des mesures parasites.

  * fusion des objets trop proches:
  Les robots ont une taille minimum donc deux objets détectés dans un rayons inférieur à cette taille minimum sont sûrement le même objet

  * facteur de forme:
  pour filtrer les gros objet apparents donc normalement proches mais pourtant lointains
  il y a une relation entre taille de l'objet et sa distance (obtenu par apprentissage pour un objet comme le support balise) c'est le facteur de forme;
  si le couple taille/distance s'éloigne trop du facteur de forme, il ne correspond pas à une balise
*/
// _______________________________________________________________
void CLidarDataFilterTracker::filter(const CLidarData *data_in, CLidarData *data_out)
{
    if (!data_in)   return;
    if (!data_out)  return;

    //on s'assure que les données sont rincées
    for(int i=0;i<data_out->m_measures_count;i++)
        data_out->m_dist_measures[i]=0.;

    //Variables internes algo    
    //pour gérer le CREC
    int i_min_CREC=0;
    int i_max_CREC=0;
    bool b_CREC=false;
    bool b_RESET=false;
    //pourgérer le blob de points
    int i_COUNT=0;
    double d_somme=0.;
    //pour détecter les découpes de points
    int i_COUNT_BLANK=0;
    //pour fusionner les blobs trop proches
    double old_dist=0.;
    int old_angle=0;
    bool toMerge=false;
    //pour gérer les forts gradients
    bool b_THRESHOLD=false;
    double d_Samples_Threshold[m_i_MAX_SAMPLES_THRESHOLD];
    for(int j=0;j<m_i_MAX_SAMPLES_THRESHOLD;j++)
        d_Samples_Threshold[j]=0.;
    int i_COUNT_THRESHOLD=0;
    double buffer_average=0.;
    double buffer_std_deviation=0.;

    //ALGO de détection
    for(int i=0;i<data_in->m_measures_count;i++)
    {
        if (i==data_in->m_measures_count-1)
        {
            //Fin du balayage, fermeture automatique du CREC
            b_RESET=true;
        }
        else
        {
            //Données enregistrables
            if((data_in->m_dist_measures[i]>m_d_MIN_dist->read().toDouble())&&(data_in->m_dist_measures[i]<m_d_MAX_dist->read().toDouble()))
            {
                //ENREGISTREMENT PENDANT CREC
                if (b_CREC)
                {
                    i_COUNT_BLANK=0;
                    i_COUNT++;
                    d_somme=d_somme+data_in->m_dist_measures[i];
                }
                else
                {
                    //OUVERTURE DU CREC
                    b_CREC=true;
                    i_min_CREC=i;
                    i_max_CREC=i;
                    i_COUNT=1;
                    d_somme=data_in->m_dist_measures[i];
                }

                //on remplit le buffer
                if(i_COUNT_THRESHOLD<m_i_MAX_SAMPLES_THRESHOLD-1)
                {
                    d_Samples_Threshold[i_COUNT_THRESHOLD]=data_in->m_dist_measures[i];
                    i_COUNT_THRESHOLD++;
                }
                //on peut vérifier le gradient
                else
                {
                    d_Samples_Threshold[i_COUNT_THRESHOLD]=data_in->m_dist_measures[i];
                    //le buffer est rempli on fait le traitement
                    buffer_average=0.;
                    buffer_std_deviation=0.;
                    for(int j=0;j<m_i_MAX_SAMPLES_THRESHOLD;j++)
                    {
                        buffer_average=buffer_average+d_Samples_Threshold[j];
                        buffer_std_deviation=buffer_std_deviation+d_Samples_Threshold[j]*d_Samples_Threshold[j];
                    }
                    buffer_average=buffer_average/m_i_MAX_SAMPLES_THRESHOLD;
                    buffer_std_deviation=sqrt((buffer_std_deviation/m_i_MAX_SAMPLES_THRESHOLD)-(buffer_average*buffer_average));

                    //décalage du buffer
                    for(int j=0;j<m_i_MAX_SAMPLES_THRESHOLD-1;j++)
                        d_Samples_Threshold[j]=d_Samples_Threshold[j+1];

                    //DETECTION FIN CREC: pb de masquage
                    if (buffer_std_deviation > m_d_seuil_gradient->read().toDouble())
                    {
                        b_RESET=true;
                        b_THRESHOLD=true;
                    }
                }
            }
            else if (b_CREC)
            {
                //DETECTION FIN CREC: vue à l'infini
                i_COUNT_BLANK++;
                if(i_COUNT_BLANK>=m_i_MAX_BLANK->read().toInt())
                    b_RESET=true;
            }

            if(b_CREC && b_RESET)
            {
                //MARQUAGE FIN CREC
                i_max_CREC=i;

                //MOYENNE DU CREC
                //Moyenne des données enregistrées lors d'un CREC
                //On ne fait la moyenne que s'il y a assez de points, et on ne la fait pas quand il y en a trop
                if((i_COUNT>=m_i_MIN_COUNT->read().toInt()) && (i_COUNT<=m_i_MAX_COUNT->read().toInt()))
                {
                    //on a détecté un fort gradient, on peut enlever au moins la dernière mesure
                    if(b_THRESHOLD)
                    {
                        d_somme=d_somme-data_in->m_dist_measures[i];
                        i_COUNT--;
                    }
                    double d_moyenne = d_somme/i_COUNT;
                    int i_moyenne = i_max_CREC - ((i_max_CREC-i_min_CREC)/2);


                    if((old_angle>0.) && (old_dist>0.))
                    {
                        //les deux points sont très proches en angle
                        if((fabs(i_moyenne-old_angle)<m_i_seuil_filtrage_angle->read().toInt()))
                        {
                            //si ils sont également très proche en distance on fusionne
                            if(fabs(d_moyenne-old_dist)<m_d_seuil_filtrage_dist->read().toDouble())
                                toMerge=true;
                            else
                                toMerge=false;

                        }
                        else
                            toMerge=false;
                    }


                    //vérification du facteur de forme
                    //f(x)=0.642905433457839*x*x-50.9818722727577*x+1085.44732653302
                    double facteur_forme=0.642905433457839*i_COUNT*i_COUNT-50.9818722727577*i_COUNT+1085.44732653302;

                    if(fabs(d_moyenne-facteur_forme)<m_d_seuil_facteur_forme->read().toDouble())
                    {

                        int i_recorded=0;
                        double dist_recorded=0.;
                        //qDebug() << "à fusionner "<<toMerge;
                        if(toMerge)
                        {
                            i_recorded=abs((i_moyenne+old_angle)/2);
                            dist_recorded=fabs((d_moyenne+old_dist)/2);
                            qDebug() <<"("<<old_angle<<","<<old_dist<<") et ("<<i_moyenne<<","<<d_moyenne<<") donne ("<<i_recorded<<","<<dist_recorded<<")";
                        }
                        else
                        {
                            i_recorded=i_moyenne;
                            dist_recorded=d_moyenne;
                        }

                        data_out->m_dist_measures[i_recorded]=dist_recorded-m_d_dist_offset->read().toDouble();

                       // data_out->m_dist_measures[i_moyenne]=d_moyenne-m_d_dist_offset->read().toDouble();
                        //mémorisation pour filtrage
                        old_angle=i_moyenne;
                        old_dist=d_moyenne;
                    }
                }

                //FERMETURE ET RESET DU CREC
                b_CREC=false;
                b_RESET=false;
                i_COUNT_BLANK=0;
                i_COUNT=0;
                d_somme=0.;
                i_COUNT_THRESHOLD=0;
                b_THRESHOLD=false;
            }
        }
    }

    //on s'assure que les données à 0 représentent un faux obstacle
    for(int i=0;i<data_out->m_measures_count;i++)
        if(data_out->m_dist_measures[i]==0.)
            data_out->m_dist_measures[i]=LidarUtils::NO_OBSTACLE;

}


#include <math.h>

#include "lidar_data_filter_tracker.h"
#include "lidar_data.h"

CLidarDataFilterTracker::CLidarDataFilterTracker()
{
    //Paramétrage
    m_d_dist_offset=40.;
    m_i_MAX_BLANK=3;
    m_i_MIN_COUNT=2;
    m_d_MAX_dist=1000.;
    m_d_MIN_dist=150.;
    m_d_seuil_filtrage_dist=150.;
    m_i_seuil_filtrage_angle=3;
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


// _______________________________________________________________
void CLidarDataFilterTracker::filter(const CLidarData *data_in, CLidarData *data_out)
{
    if (!data_in)   return;
    if (!data_out)  return;

    //on s'assure que les données sont rincées
    for(int i=0;i<data_out->m_measures_count;i++)
        data_out->m_dist_measures[i]=0.;

    //Variables internes algo
    // CREC = CRéneau REception
    int i_min_CREC=0;
    int i_max_CREC=0;
    bool b_CREC=false;
    int i_COUNT=0;
    double d_somme=0.;
    int i_COUNT_BLANK=0;
    double old_dist=0.;
    int old_angle=0;

    //ALGO de détection
    for(int i=0;i<data_in->m_measures_count;i++)
    {
        if (i==data_in->m_measures_count-1)
        {
            //Fin du balayage, fermeture automatique du CREC
            i_COUNT_BLANK=m_i_MAX_BLANK;
        }
        else
        {
            //Données enregistrables
            if((data_in->m_dist_measures[i]>m_d_MIN_dist)&&(data_in->m_dist_measures[i]<m_d_MAX_dist))
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
            }
            else if (b_CREC)
            {
                //POUR DETECTER FIN CREC
                i_COUNT_BLANK++;
            }

            //DETECTION FIN CREC
            if(b_CREC && (i_COUNT_BLANK>=m_i_MAX_BLANK))
            {
                //MARQUAGE FIN CREC
                i_max_CREC=i;

                //MOYENNE DU CREC
                //Moyenne des données enregistrées lors d'un CREC
                if(i_COUNT>=m_i_MIN_COUNT) //On ne fait la moyenne que s'il y a assez de points
                {
                    double d_moyenne = d_somme/i_COUNT;
                    int i_moyenne = i_max_CREC - ((i_max_CREC-i_min_CREC)/2);

                    //enregistrement des données filtrées
                    if((fabs(i_moyenne-old_angle)>m_i_seuil_filtrage_angle)&&(abs(d_moyenne-old_dist)>m_d_seuil_filtrage_dist))
                    {
                        data_out->m_dist_measures[i_moyenne]=d_moyenne-m_d_dist_offset;
                        //mémorisation pour filtrage
                        old_angle=i_moyenne;
                        old_dist=d_moyenne;
                    }
                }

                //FERMETURE ET RESET DU CREC
                b_CREC=false;
                i_COUNT_BLANK=0;
                i_COUNT=0;
                d_somme=0.;
            }
        }
    }
}


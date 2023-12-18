#include <QDebug>
#include <QMessageBox>
#include <QFileInfo>
#include <math.h>
#include "lidar_data_player.h"
#include "lidar_data.h"
#include "csvParser.h"

CLidarDataPlayer::CLidarDataPlayer(QObject *parent)
    : QObject(parent),
      m_state(PLAYER_STOP),
      m_current_step(-1)
{
    connect(&m_timer, SIGNAL(timeout()), this, SLOT(timer_tick()));
}

CLidarDataPlayer::~CLidarDataPlayer()
{
}

// _________________________________________________
int CLidarDataPlayer::get_step_count()
{
    return m_datas.size();
}

// _________________________________________________
bool CLidarDataPlayer::parse(QString pathfilename)
{
    clear();

    csvData data;
    csvParser parser;
    QStringList error_msg_lst, warn_msg_lst;

    parser.enableEmptyCells(false);
    parser.setSeparator(";");
    parser.setMinimumNumberOfExpectedColums(3);
    bool status = parser.parse(pathfilename, data, &error_msg_lst, &warn_msg_lst);
    if (!status) {
        if (error_msg_lst.size() > 0) {
            QString global_err_msg;
            foreach (QString msg, error_msg_lst) global_err_msg += "\n" + msg; // regroupe tous les messages en une seule chaine à afficher
            QMessageBox::critical(0, QFileInfo(pathfilename).fileName(), global_err_msg);
        }
        return false; // pas la peine d'aller plus loin, le fichier d'entrée n'est pas valide
    }

    // Il y a des warnings dans le parsing du fichier d'entrée
    if (warn_msg_lst.size() > 0) {
        QString global_warn_msg;
        foreach (QString msg, warn_msg_lst) global_warn_msg += "\n" + msg; // regroupe tous les messages en une seule chaine à afficher
        QMessageBox::warning(0, QFileInfo(pathfilename).fileName(), global_warn_msg);
    }

    // Vérifie que la colonne 1 est bien timestamp
    if (!data.m_header.at(0).simplified().toLower().contains("timestamp")) {
        QMessageBox::critical(0, QFileInfo(pathfilename).fileName(), QString("Colonne1 : attendu Timestamp / observe %1:").arg(data.m_header.at(0)));
        return false;
    }

    // Vérifie que toutes les autres colonnes correspondent à une mesure en degre
    // Test basé sur le nom de la colonne qui doit contenir le texte "distance"
    for (int column=1; column<data.m_header.size(); column++) {
        if (!data.m_header.at(column).simplified().toLower().contains("dist")) {
            QMessageBox::critical(0, QFileInfo(pathfilename).fileName(), QString("Colonne %1 : attendu \"Dist\" / observe:%2").arg(column+1).arg(data.m_header.at(column)));
            return false;
        }
    }

    // Recherche de la résolution
    // La résolution se trouve dans le nom de la 2ème et 3ème colonne
    // Exemple :
    //      Nom de la 2ème colonne : "Dist -45 deg [mm]"
    //      Nom de la 3ème colonne : "Dist -44 deg [mm]"
    //      => Résolution = |45 - 44| = 1 degre
    QString name_col2 = data.m_header.at(1);
    QString name_col3 = data.m_header.at(2);


    QString name2_value = name_col2.trimmed().simplified().toLower().remove("dist").remove("deg [mm]").trimmed();
    QString name3_value = name_col3.trimmed().simplified().toLower().remove("dist").remove("deg [mm]").trimmed();

    bool ok;
    double col2_val = name2_value.toDouble(&ok);
    if (!ok) {
        QMessageBox::critical(0, name_col2, QString("Impossible d'extraire la valeur numérique"));
        return false;
    }
    double col3_val = name3_value.toDouble(&ok);
    if (!ok) {
        QMessageBox::critical(0, name_col3, QString("Impossible d'extraire la valeur numérique"));
        return false;
    }

    double resolution = fabs(col2_val-col3_val);
    qDebug() << "Resolution du fichier : " << resolution;

    // Récupère toutes les lignes
    for (int line=0; line<data.m_datas.size(); line++) {
        CLidarData lidar_data;
        QStringList data_line = data.m_datas.at(line);
        lidar_data.m_timestamp = data_line.at(0).toULongLong();
        lidar_data.m_angle_step_resolution = resolution;
        lidar_data.m_start_angle = col2_val;
        lidar_data.m_measures_count = data_line.size() - 1; // -1 car la 1ère colonne est le timestamp
        int i=0;
        for (int column=1; column<data_line.size(); column++) {
            int distance;
            distance = data_line.at(column).toInt(&ok);
            if (!ok) {
                QMessageBox::critical(0, "Erreur de syntaxe", QString("Ligne %1 : impossible d'extraire la valeur numérique").arg(line+1));
                return false;
            }
            lidar_data.m_dist_measures[i++] = distance;
        }
        m_datas.append(lidar_data);   // ajoute la donée à la liste
    }

    qDebug() << QString("Parsing OK : Nombre d'angles=%1 / Resolution=%2 / Nombre de lignes=%3")
                .arg(data.m_header.size()-1)
                .arg(resolution)
                .arg(data.m_datas.size());

    return true;
}

// _________________________________________________
void CLidarDataPlayer::play(int step)
{
    if (step >=m_datas.size()) return;
    emit data_pending(step);

    CLidarData data;
    get_step(m_current_step, &data);
    emit new_data(m_datas.at(step));
}

// _________________________________________________
void CLidarDataPlayer::play_current_step()
{
    play(m_current_step);
}

// _________________________________________________
int CLidarDataPlayer::next_step()
{
    m_current_step++;
    if (m_current_step >=m_datas.size()) stop();
    return m_current_step;
}

// _________________________________________________
void CLidarDataPlayer::start()
{
    if (m_state == PLAYER_IN_PROGRESS) return;
    m_current_step = -1;
    m_state = PLAYER_IN_PROGRESS;
    m_timer.start(50);
}

// _________________________________________________
void CLidarDataPlayer::stop()
{
    if (m_state == PLAYER_STOP) return;
    m_timer.stop();
    m_current_step = -1;
    m_state = PLAYER_STOP;
    emit finished();
}

// _________________________________________________
void CLidarDataPlayer::pause()
{
    if (m_state != PLAYER_IN_PROGRESS) return;
    m_timer.stop();
    m_state = PLAYER_PAUSE;
    emit pause();
}

// _________________________________________________
void CLidarDataPlayer::goto_step(int step)
{
    if (m_current_step >=m_datas.size()) return;
    m_current_step = step;
}

// _________________________________________________
int CLidarDataPlayer::current_step()
{
    return m_current_step;
}

// _________________________________________________
void CLidarDataPlayer::get_step(int step, CLidarData *out_data)
{
    if (!out_data) return;
    if (step >= m_datas.size()) return;
    *out_data = m_datas.at(step);
}

// _________________________________________________
void CLidarDataPlayer::clear()
{
    stop();
    m_datas.clear();
}

// _________________________________________________
void CLidarDataPlayer::timer_tick()
{
    int step_to_play = next_step();  // si dernier step, le player est arrêté
    qDebug() << "Timer tick" << step_to_play;
    if (m_state == PLAYER_IN_PROGRESS) {
       play(step_to_play);
       m_timer.start(50);
    }
}

// FICHIER GENERE PAR L'OUTIL MESS2C_robot V1.0
// Date de génération : Tue May 05 22:41:21 2015
// PLATEFORME CIBLE : LABOTBOX
/*! \file CTrameFactory.cpp
 * A brief file description CPP.
 * A more elaborated file description.
 */

#include <QDebug>
#include "CDataManager.h"
#include "CDataManager.h"
#include "CMessagerieBot.h"

#include "CTrameFactory.h"
#include "data_encoder_decoder.h"

/*! \addtogroup DataManager
   * 
   *  @{
   */

// _____________________________________________________________________
/*!
* Constructeur
*
*/
CTrameFactory::CTrameFactory(CMessagerieBot *messagerie_bot, CDataManager *data_manager)
    : m_messagerie_bot(messagerie_bot),
      m_data_manager(data_manager)
{
  create();
}

// _____________________________________________________________________
/*!
* Destructeur
*
*/
CTrameFactory::~CTrameFactory()
{
  // Nettoie les trames créées dynamiquement
  for (int i=0; i<m_liste_trames.size(); i++) {
    if (m_liste_trames.at(i) != NULL) { delete m_liste_trames.at(i); }
  }
}



// _____________________________________________________________________
/*!
*  Point d'entrée lorsqu'une trame valide arrive
*
*  \param [in] trame la trame arrivée
*/
void CTrameFactory::Decode(tStructTrameBrute trame)
{
  // Vérifie pour chaque trame de la factory si l'identifiant
  // correspond à celui de la trame reçue
  for (int i=0; i<m_liste_trames_rx.size(); i++) {
    if (m_liste_trames_rx.at(i)->m_id == trame.ID) {
        m_liste_trames_rx.at(i)->Decode(&trame);
    }
  }
}
// _____________________________________________________________________
/*!
*  Construit et initialise les trames
*
*  \param [in] data la valeur de la donnée au format générique QVariant
*  \remarks le signal valueChanged est émis si la valeur de la donnée a changé
*  \remarks l'écriture de la donnée est réalisée en section critique pour assurer l'utilisation en multi-thread
*/
void CTrameFactory::create(void)
{
  // Trames en réception
 m_liste_trames_rx.append(new CTrame_ETAT_PID_ASSERVISSEMENT(m_messagerie_bot, m_data_manager));
 m_liste_trames_rx.append(new CTrame_ETAT_ASSERVISSEMENT(m_messagerie_bot, m_data_manager));
 m_liste_trames_rx.append(new CTrame_POSITION_CODEURS(m_messagerie_bot, m_data_manager));
 m_liste_trames_rx.append(new CTrame_POSITION_ABSOLUE_XY_TETA(m_messagerie_bot, m_data_manager));
 m_liste_trames_rx.append(new CTrame_ELECTROBOT_ETAT_CODEURS_1_2(m_messagerie_bot, m_data_manager));
 m_liste_trames_rx.append(new CTrame_ELECTROBOT_ETAT_TELEMETRES(m_messagerie_bot, m_data_manager));
 m_liste_trames_rx.append(new CTrame_ELECTROBOT_ETAT_CODEURS_3_4(m_messagerie_bot, m_data_manager));
 m_liste_trames_rx.append(new CTrame_ELECTROBOT_ETAT_CAPTEURS_2(m_messagerie_bot, m_data_manager));
 m_liste_trames_rx.append(new CTrame_ELECTROBOT_ETAT_CAPTEURS_1(m_messagerie_bot, m_data_manager));
 m_liste_trames_rx.append(new CTrame_ECRAN_ETAT_MATCH(m_messagerie_bot, m_data_manager));
 m_liste_trames_rx.append(new CTrame_ETAT_DETECTION_EVITEMENT_OBSTACLE(m_messagerie_bot, m_data_manager));
 m_liste_trames_rx.append(new CTrame_ETAT_RACK(m_messagerie_bot, m_data_manager));
 m_liste_trames_rx.append(new CTrame_COLOR_SENSOR(m_messagerie_bot, m_data_manager));
 m_liste_trames_rx.append(new CTrame_ETAT_POWER_ELECTROBOT(m_messagerie_bot, m_data_manager));
 m_liste_trames_rx.append(new CTrame_ETAT_SERVO_AX(m_messagerie_bot, m_data_manager));
 m_liste_trames_rx.append(new CTrame_ETAT_KMAR_GENERAL(m_messagerie_bot, m_data_manager));
 m_liste_trames_rx.append(new CTrame_FREE_STRING(m_messagerie_bot, m_data_manager));
 m_liste_trames_rx.append(new CTrame_ETAT_CHARGE_CPU(m_messagerie_bot, m_data_manager));
 m_liste_trames_rx.append(new CTrame_EEPROM_VALUE(m_messagerie_bot, m_data_manager));

 // Trames en émission
 m_liste_trames_tx.append(new CTrame_ELECTROBOT_CONFIG_SERVOS(m_messagerie_bot, m_data_manager));
 m_liste_trames_tx.append(new CTrame_ELECTROBOT_CDE_SERVOS_AX(m_messagerie_bot, m_data_manager));
 m_liste_trames_tx.append(new CTrame_ELECTROBOT_CDE_MOTEURS(m_messagerie_bot, m_data_manager));
 m_liste_trames_tx.append(new CTrame_ELECTROBOT_CDE_POWER_SWITCH(m_messagerie_bot, m_data_manager));
 m_liste_trames_tx.append(new CTrame_COMMANDE_MVT_XY(m_messagerie_bot, m_data_manager));
 m_liste_trames_tx.append(new CTrame_ASSERV_RESET(m_messagerie_bot, m_data_manager));
 m_liste_trames_tx.append(new CTrame_AUTOAPPRENTISSAGE_ASSERV(m_messagerie_bot, m_data_manager));
 m_liste_trames_tx.append(new CTrame_COMMANDE_REINIT_XY_TETA(m_messagerie_bot, m_data_manager));
 m_liste_trames_tx.append(new CTrame_COMMANDE_VITESSE_MVT(m_messagerie_bot, m_data_manager));
 m_liste_trames_tx.append(new CTrame_COMMANDE_REGUL_VITESSE(m_messagerie_bot, m_data_manager));
 m_liste_trames_tx.append(new CTrame_COMMANDE_DISTANCE_ANGLE(m_messagerie_bot, m_data_manager));
 m_liste_trames_tx.append(new CTrame_COMMANDE_MVT_XY_TETA(m_messagerie_bot, m_data_manager));
 m_liste_trames_tx.append(new CTrame_ASSERV_DIAG_WRITE_PARAM(m_messagerie_bot, m_data_manager));
 m_liste_trames_tx.append(new CTrame_ELECTROBOT_CDE_SERVOS(m_messagerie_bot, m_data_manager));
 m_liste_trames_tx.append(new CTrame_COMMANDE_MVT_MANUEL(m_messagerie_bot, m_data_manager));
 m_liste_trames_tx.append(new CTrame_ECRAN_ETAT_ECRAN(m_messagerie_bot, m_data_manager));
 m_liste_trames_tx.append(new CTrame_CONFIG_PERIODE_TRAME(m_messagerie_bot, m_data_manager));
 m_liste_trames_tx.append(new CTrame_COMMANDE_POWER_ELECTROBOT(m_messagerie_bot, m_data_manager));
 m_liste_trames_tx.append(new CTrame_COMMANDE_KMAR(m_messagerie_bot, m_data_manager));
 m_liste_trames_tx.append(new CTrame_ETAT_LIDAR(m_messagerie_bot, m_data_manager));
 m_liste_trames_tx.append(new CTrame_RESET_CPU(m_messagerie_bot, m_data_manager));
 m_liste_trames_tx.append(new CTrame_COMMANDE_MODE_FONCTIONNEMENT_CPU(m_messagerie_bot, m_data_manager));
 m_liste_trames_tx.append(new CTrame_READ_EEPROM_REQ(m_messagerie_bot, m_data_manager));
 m_liste_trames_tx.append(new CTrame_WRITE_EEPROM_REQ(m_messagerie_bot, m_data_manager));
 m_liste_trames_tx.append(new CTrame_ACTION_ROBOT(m_messagerie_bot, m_data_manager));

 // Crée une seule liste avec toutes les trames en émission et en réception
 for (int i=0; i<m_liste_trames_rx.size(); i++) {
    m_liste_trames.append(m_liste_trames_rx.at(i));
 }
 for (int i=0; i<m_liste_trames_tx.size(); i++) {
    m_liste_trames.append(m_liste_trames_tx.at(i));
 }

}

tListeTrames CTrameFactory::getListeTramesRx() const
{
    return m_liste_trames_rx;
}

tListeTrames CTrameFactory::getListeTramesTx() const
{
    return m_liste_trames_tx;
}

tListeTrames CTrameFactory::getListeTrames() const
{
    return m_liste_trames;
}

unsigned int CTrameFactory::name2ID(QString name)
{
    for (int i=0; i<m_liste_trames.count(); i++)
    {
        if (name == m_liste_trames[i]->m_name) return m_liste_trames[i]->m_id;
    }
    return 0;
}

QString CTrameFactory::ID2Name(unsigned int id)
{
    for (int i=0; i<m_liste_trames.count(); i++)
    {
        if (id == m_liste_trames[i]->m_id) return m_liste_trames[i]->m_name;
    }
    return "";
}

CTrameBot *CTrameFactory::getTrameFromName(QString name)
{
    return getTrameFromID(name2ID(name));
}

CTrameBot *CTrameFactory::getTrameFromID(unsigned int id)
{
    for (int i=0; i<m_liste_trames.count(); i++)
    {
        if (id == m_liste_trames[i]->m_id) return m_liste_trames[i];
    }
    return nullptr;

}


// ========================================================
//             TRAME ELECTROBOT_CONFIG_SERVOS
// ========================================================
CTrame_ELECTROBOT_CONFIG_SERVOS::CTrame_ELECTROBOT_CONFIG_SERVOS(CMessagerieBot *messagerie_bot, CDataManager *data_manager)
    : CTrameBot(messagerie_bot, data_manager)
{
 m_name = "ELECTROBOT_CONFIG_SERVOS";
 m_id = ID_ELECTROBOT_CONFIG_SERVOS;
 m_dlc = DLC_ELECTROBOT_CONFIG_SERVOS;
 m_liste_noms_signaux.append("valeur_commande_sd20");
 m_liste_noms_signaux.append("commande_sd20");
 m_liste_noms_signaux.append("num_servo_sd20");

 // Initialise les données de la messagerie
 valeur_commande_sd20 = 0;
 commande_sd20 = 0;
 num_servo_sd20 = 0;
 m_synchro_tx = 0;

 // S'assure que les données existent dans le DataManager
 data_manager->write("valeur_commande_sd20",  valeur_commande_sd20);
 data_manager->write("commande_sd20",  commande_sd20);
 data_manager->write("num_servo_sd20",  num_servo_sd20);
 data_manager->write("ELECTROBOT_CONFIG_SERVOS_TxSync",  m_synchro_tx);

 // Connexion avec le DataManager
 connect(data_manager->getData("valeur_commande_sd20"), SIGNAL(valueChanged(QVariant)), this, SLOT(valeur_commande_sd20_changed(QVariant)));
 connect(data_manager->getData("commande_sd20"), SIGNAL(valueChanged(QVariant)), this, SLOT(commande_sd20_changed(QVariant)));
 connect(data_manager->getData("num_servo_sd20"), SIGNAL(valueChanged(QVariant)), this, SLOT(num_servo_sd20_changed(QVariant)));
 connect(data_manager->getData("ELECTROBOT_CONFIG_SERVOS_TxSync"), SIGNAL(valueChanged(QVariant)), this, SLOT(Synchro_changed(QVariant)));

}
//___________________________________________________________________________
/*!
  \brief Fonction appelée lorsque la data est modifée
  \param val la nouvelle valeur de la data
*/
void CTrame_ELECTROBOT_CONFIG_SERVOS::valeur_commande_sd20_changed(QVariant val)
{
  valeur_commande_sd20 = val.toInt();
  if (m_synchro_tx == 0) { Encode(); }
}
//___________________________________________________________________________
/*!
  \brief Fonction appelée lorsque la data est modifée
  \param val la nouvelle valeur de la data
*/
void CTrame_ELECTROBOT_CONFIG_SERVOS::commande_sd20_changed(QVariant val)
{
  commande_sd20 = val.toInt();
  if (m_synchro_tx == 0) { Encode(); }
}
//___________________________________________________________________________
/*!
  \brief Fonction appelée lorsque la data est modifée
  \param val la nouvelle valeur de la data
*/
void CTrame_ELECTROBOT_CONFIG_SERVOS::num_servo_sd20_changed(QVariant val)
{
  num_servo_sd20 = val.toInt();
  if (m_synchro_tx == 0) { Encode(); }
}
//___________________________________________________________________________
/*!
  \brief Fonction appelée lorsque la data est modifée
  \param val la nouvelle valeur de la data
*/
void CTrame_ELECTROBOT_CONFIG_SERVOS::Synchro_changed(QVariant val)
{
  m_synchro_tx = val.toBool();
  if (m_synchro_tx == 0) { Encode(); }
}

//___________________________________________________________________________
/*!
  \brief Encode et envoie la trame
*/
void CTrame_ELECTROBOT_CONFIG_SERVOS::Encode(void)
{
  tStructTrameBrute trame;

  // Informations générales
  trame.ID = ID_ELECTROBOT_CONFIG_SERVOS;
  trame.DLC = DLC_ELECTROBOT_CONFIG_SERVOS;

 for (unsigned int i=0; i<m_dlc; i++) {
     trame.Data[i] = 0;
 }
  // Encode chacun des signaux de la trame
    trame.Data[4] |= (unsigned char)( ( (valeur_commande_sd20) & 0xFF) );
    trame.Data[3] |= (unsigned char)( ( (valeur_commande_sd20 >> 8) & 0xFF) );

    trame.Data[2] |= (unsigned char)( ( (commande_sd20) & 0xFF) );
    trame.Data[1] |= (unsigned char)( ( (commande_sd20 >> 8) & 0xFF) );

    trame.Data[0] |= (unsigned char)( ( (num_servo_sd20) & 0xFF) );

  // Envoie la trame
  m_messagerie_bot->SerialiseTrame(&trame);

  // Comptabilise le nombre de trames émises
  m_nombre_emis++;
}

// ========================================================
//             TRAME ELECTROBOT_CDE_SERVOS_AX
// ========================================================
CTrame_ELECTROBOT_CDE_SERVOS_AX::CTrame_ELECTROBOT_CDE_SERVOS_AX(CMessagerieBot *messagerie_bot, CDataManager *data_manager)
    : CTrameBot(messagerie_bot, data_manager)
{
 m_name = "ELECTROBOT_CDE_SERVOS_AX";
 m_id = ID_ELECTROBOT_CDE_SERVOS_AX;
 m_dlc = DLC_ELECTROBOT_CDE_SERVOS_AX;
 m_liste_noms_signaux.append("valeur_commande_ax");
 m_liste_noms_signaux.append("commande_ax");
 m_liste_noms_signaux.append("num_servo_ax");

 // Initialise les données de la messagerie
 valeur_commande_ax = 0;
 commande_ax = 0;
 num_servo_ax = 0;
 m_synchro_tx = 0;

 // S'assure que les données existent dans le DataManager
 data_manager->write("valeur_commande_ax",  valeur_commande_ax);
 data_manager->write("commande_ax",  commande_ax);
 data_manager->write("num_servo_ax",  num_servo_ax);
 data_manager->write("ELECTROBOT_CDE_SERVOS_AX_TxSync",  m_synchro_tx);

 // Connexion avec le DataManager
 connect(data_manager->getData("valeur_commande_ax"), SIGNAL(valueChanged(QVariant)), this, SLOT(valeur_commande_ax_changed(QVariant)));
 connect(data_manager->getData("commande_ax"), SIGNAL(valueChanged(QVariant)), this, SLOT(commande_ax_changed(QVariant)));
 connect(data_manager->getData("num_servo_ax"), SIGNAL(valueChanged(QVariant)), this, SLOT(num_servo_ax_changed(QVariant)));
 connect(data_manager->getData("ELECTROBOT_CDE_SERVOS_AX_TxSync"), SIGNAL(valueChanged(QVariant)), this, SLOT(Synchro_changed(QVariant)));

}
//___________________________________________________________________________
/*!
  \brief Fonction appelée lorsque la data est modifée
  \param val la nouvelle valeur de la data
*/
void CTrame_ELECTROBOT_CDE_SERVOS_AX::valeur_commande_ax_changed(QVariant val)
{
  valeur_commande_ax = val.toInt();
  if (m_synchro_tx == 0) { Encode(); }
}
//___________________________________________________________________________
/*!
  \brief Fonction appelée lorsque la data est modifée
  \param val la nouvelle valeur de la data
*/
void CTrame_ELECTROBOT_CDE_SERVOS_AX::commande_ax_changed(QVariant val)
{
  commande_ax = val.toInt();
  if (m_synchro_tx == 0) { Encode(); }
}
//___________________________________________________________________________
/*!
  \brief Fonction appelée lorsque la data est modifée
  \param val la nouvelle valeur de la data
*/
void CTrame_ELECTROBOT_CDE_SERVOS_AX::num_servo_ax_changed(QVariant val)
{
  num_servo_ax = val.toInt();
  if (m_synchro_tx == 0) { Encode(); }
}
//___________________________________________________________________________
/*!
  \brief Fonction appelée lorsque la data est modifée
  \param val la nouvelle valeur de la data
*/
void CTrame_ELECTROBOT_CDE_SERVOS_AX::Synchro_changed(QVariant val)
{
  m_synchro_tx = val.toBool();
  if (m_synchro_tx == 0) { Encode(); }
}

//___________________________________________________________________________
/*!
  \brief Encode et envoie la trame
*/
void CTrame_ELECTROBOT_CDE_SERVOS_AX::Encode(void)
{
  tStructTrameBrute trame;

  // Informations générales
  trame.ID = ID_ELECTROBOT_CDE_SERVOS_AX;
  trame.DLC = DLC_ELECTROBOT_CDE_SERVOS_AX;

 for (unsigned int i=0; i<m_dlc; i++) {
     trame.Data[i] = 0;
 }
  // Encode chacun des signaux de la trame
    trame.Data[4] |= (unsigned char)( ( (valeur_commande_ax) & 0xFF) );
    trame.Data[3] |= (unsigned char)( ( (valeur_commande_ax >> 8) & 0xFF) );

    trame.Data[2] |= (unsigned char)( ( (commande_ax) & 0xFF) );
    trame.Data[1] |= (unsigned char)( ( (commande_ax >> 8) & 0xFF) );

    trame.Data[0] |= (unsigned char)( ( (num_servo_ax) & 0xFF) );

  // Envoie la trame
  m_messagerie_bot->SerialiseTrame(&trame);

  // Comptabilise le nombre de trames émises
  m_nombre_emis++;
}

// ========================================================
//             TRAME ELECTROBOT_CDE_MOTEURS
// ========================================================
CTrame_ELECTROBOT_CDE_MOTEURS::CTrame_ELECTROBOT_CDE_MOTEURS(CMessagerieBot *messagerie_bot, CDataManager *data_manager)
    : CTrameBot(messagerie_bot, data_manager)
{
 m_name = "ELECTROBOT_CDE_MOTEURS";
 m_id = ID_ELECTROBOT_CDE_MOTEURS;
 m_dlc = DLC_ELECTROBOT_CDE_MOTEURS;
 m_liste_noms_signaux.append("cde_moteur_6");
 m_liste_noms_signaux.append("cde_moteur_5");
 m_liste_noms_signaux.append("cde_moteur_4");
 m_liste_noms_signaux.append("cde_moteur_3");
 m_liste_noms_signaux.append("cde_moteur_2");
 m_liste_noms_signaux.append("cde_moteur_1");

 // Initialise les données de la messagerie
 cde_moteur_6 = 0;
 cde_moteur_5 = 0;
 cde_moteur_4 = 0;
 cde_moteur_3 = 0;
 cde_moteur_2 = 0;
 cde_moteur_1 = 0;
 m_synchro_tx = 0;

 // S'assure que les données existent dans le DataManager
 data_manager->write("cde_moteur_6",  cde_moteur_6);
 data_manager->write("cde_moteur_5",  cde_moteur_5);
 data_manager->write("cde_moteur_4",  cde_moteur_4);
 data_manager->write("cde_moteur_3",  cde_moteur_3);
 data_manager->write("cde_moteur_2",  cde_moteur_2);
 data_manager->write("cde_moteur_1",  cde_moteur_1);
 data_manager->write("ELECTROBOT_CDE_MOTEURS_TxSync",  m_synchro_tx);

 // Connexion avec le DataManager
 connect(data_manager->getData("cde_moteur_6"), SIGNAL(valueChanged(QVariant)), this, SLOT(cde_moteur_6_changed(QVariant)));
 connect(data_manager->getData("cde_moteur_5"), SIGNAL(valueChanged(QVariant)), this, SLOT(cde_moteur_5_changed(QVariant)));
 connect(data_manager->getData("cde_moteur_4"), SIGNAL(valueChanged(QVariant)), this, SLOT(cde_moteur_4_changed(QVariant)));
 connect(data_manager->getData("cde_moteur_3"), SIGNAL(valueChanged(QVariant)), this, SLOT(cde_moteur_3_changed(QVariant)));
 connect(data_manager->getData("cde_moteur_2"), SIGNAL(valueChanged(QVariant)), this, SLOT(cde_moteur_2_changed(QVariant)));
 connect(data_manager->getData("cde_moteur_1"), SIGNAL(valueChanged(QVariant)), this, SLOT(cde_moteur_1_changed(QVariant)));
 connect(data_manager->getData("ELECTROBOT_CDE_MOTEURS_TxSync"), SIGNAL(valueChanged(QVariant)), this, SLOT(Synchro_changed(QVariant)));

}
//___________________________________________________________________________
/*!
  \brief Fonction appelée lorsque la data est modifée
  \param val la nouvelle valeur de la data
*/
void CTrame_ELECTROBOT_CDE_MOTEURS::cde_moteur_6_changed(QVariant val)
{
  //qDebug() << "value mot 6" << val;
  cde_moteur_6 = val.toInt();
  //qDebug() << "value mot 6 char" << val.toByteArray();
  //qDebug() << "value mot 6 char" << cde_moteur_6;
  if (m_synchro_tx == 0) { Encode(); }
}
//___________________________________________________________________________
/*!
  \brief Fonction appelée lorsque la data est modifée
  \param val la nouvelle valeur de la data
*/
void CTrame_ELECTROBOT_CDE_MOTEURS::cde_moteur_5_changed(QVariant val)
{
  cde_moteur_5 = val.toInt();
  if (m_synchro_tx == 0) { Encode(); }
}
//___________________________________________________________________________
/*!
  \brief Fonction appelée lorsque la data est modifée
  \param val la nouvelle valeur de la data
*/
void CTrame_ELECTROBOT_CDE_MOTEURS::cde_moteur_4_changed(QVariant val)
{
  cde_moteur_4 = val.toInt();
  if (m_synchro_tx == 0) { Encode(); }
}
//___________________________________________________________________________
/*!
  \brief Fonction appelée lorsque la data est modifée
  \param val la nouvelle valeur de la data
*/
void CTrame_ELECTROBOT_CDE_MOTEURS::cde_moteur_3_changed(QVariant val)
{
  cde_moteur_3 = val.toInt();
  if (m_synchro_tx == 0) { Encode(); }
}
//___________________________________________________________________________
/*!
  \brief Fonction appelée lorsque la data est modifée
  \param val la nouvelle valeur de la data
*/
void CTrame_ELECTROBOT_CDE_MOTEURS::cde_moteur_2_changed(QVariant val)
{
  cde_moteur_2 = val.toInt();
  if (m_synchro_tx == 0) { Encode(); }
}
//___________________________________________________________________________
/*!
  \brief Fonction appelée lorsque la data est modifée
  \param val la nouvelle valeur de la data
*/
void CTrame_ELECTROBOT_CDE_MOTEURS::cde_moteur_1_changed(QVariant val)
{
  cde_moteur_1 = val.toInt();
  if (m_synchro_tx == 0) { Encode(); }
}
//___________________________________________________________________________
/*!
  \brief Fonction appelée lorsque la data est modifée
  \param val la nouvelle valeur de la data
*/
void CTrame_ELECTROBOT_CDE_MOTEURS::Synchro_changed(QVariant val)
{
  m_synchro_tx = val.toBool();
  if (m_synchro_tx == 0) { Encode(); }
}

//___________________________________________________________________________
/*!
  \brief Encode et envoie la trame
*/
void CTrame_ELECTROBOT_CDE_MOTEURS::Encode(void)
{
  tStructTrameBrute trame;

  // Informations générales
  trame.ID = ID_ELECTROBOT_CDE_MOTEURS;
  trame.DLC = DLC_ELECTROBOT_CDE_MOTEURS;

 for (unsigned int i=0; i<m_dlc; i++) {
     trame.Data[i] = 0;
 }
  // Encode chacun des signaux de la trame
    trame.Data[5] |= (unsigned char)( ( (cde_moteur_6) & 0xFF) );

    trame.Data[4] |= (unsigned char)( ( (cde_moteur_5) & 0xFF) );

    trame.Data[3] |= (unsigned char)( ( (cde_moteur_4) & 0xFF) );

    trame.Data[2] |= (unsigned char)( ( (cde_moteur_3) & 0xFF) );

    trame.Data[1] |= (unsigned char)( ( (cde_moteur_2) & 0xFF) );

    trame.Data[0] |= (unsigned char)( ( (cde_moteur_1) & 0xFF) );

  // Envoie la trame
  m_messagerie_bot->SerialiseTrame(&trame);

  // Comptabilise le nombre de trames émises
  m_nombre_emis++;
}

// ========================================================
//             TRAME ELECTROBOT_CDE_POWER_SWITCH
// ========================================================
CTrame_ELECTROBOT_CDE_POWER_SWITCH::CTrame_ELECTROBOT_CDE_POWER_SWITCH(CMessagerieBot *messagerie_bot, CDataManager *data_manager)
    : CTrameBot(messagerie_bot, data_manager)
{
 m_name = "ELECTROBOT_CDE_POWER_SWITCH";
 m_id = ID_ELECTROBOT_CDE_POWER_SWITCH;
 m_dlc = DLC_ELECTROBOT_CDE_POWER_SWITCH;
 m_liste_noms_signaux.append("PowerSwitch_xt8");
 m_liste_noms_signaux.append("PowerSwitch_xt7");
 m_liste_noms_signaux.append("PowerSwitch_xt6");
 m_liste_noms_signaux.append("PowerSwitch_xt5");
 m_liste_noms_signaux.append("PowerSwitch_xt4");
 m_liste_noms_signaux.append("PowerSwitch_xt3");
 m_liste_noms_signaux.append("PowerSwitch_xt2");
 m_liste_noms_signaux.append("PowerSwitch_xt1");

 // Initialise les données de la messagerie
 PowerSwitch_xt8 = 0;
 PowerSwitch_xt7 = 0;
 PowerSwitch_xt6 = 0;
 PowerSwitch_xt5 = 0;
 PowerSwitch_xt4 = 0;
 PowerSwitch_xt3 = 0;
 PowerSwitch_xt2 = 0;
 PowerSwitch_xt1 = 0;
 m_synchro_tx = 0;

 // S'assure que les données existent dans le DataManager
 data_manager->write("PowerSwitch_xt8",  PowerSwitch_xt8);
 data_manager->write("PowerSwitch_xt7",  PowerSwitch_xt7);
 data_manager->write("PowerSwitch_xt6",  PowerSwitch_xt6);
 data_manager->write("PowerSwitch_xt5",  PowerSwitch_xt5);
 data_manager->write("PowerSwitch_xt4",  PowerSwitch_xt4);
 data_manager->write("PowerSwitch_xt3",  PowerSwitch_xt3);
 data_manager->write("PowerSwitch_xt2",  PowerSwitch_xt2);
 data_manager->write("PowerSwitch_xt1",  PowerSwitch_xt1);
 data_manager->write("ELECTROBOT_CDE_POWER_SWITCH_TxSync",  m_synchro_tx);

 // Connexion avec le DataManager
 connect(data_manager->getData("PowerSwitch_xt8"), SIGNAL(valueChanged(QVariant)), this, SLOT(PowerSwitch_xt8_changed(QVariant)));
 connect(data_manager->getData("PowerSwitch_xt7"), SIGNAL(valueChanged(QVariant)), this, SLOT(PowerSwitch_xt7_changed(QVariant)));
 connect(data_manager->getData("PowerSwitch_xt6"), SIGNAL(valueChanged(QVariant)), this, SLOT(PowerSwitch_xt6_changed(QVariant)));
 connect(data_manager->getData("PowerSwitch_xt5"), SIGNAL(valueChanged(QVariant)), this, SLOT(PowerSwitch_xt5_changed(QVariant)));
 connect(data_manager->getData("PowerSwitch_xt4"), SIGNAL(valueChanged(QVariant)), this, SLOT(PowerSwitch_xt4_changed(QVariant)));
 connect(data_manager->getData("PowerSwitch_xt3"), SIGNAL(valueChanged(QVariant)), this, SLOT(PowerSwitch_xt3_changed(QVariant)));
 connect(data_manager->getData("PowerSwitch_xt2"), SIGNAL(valueChanged(QVariant)), this, SLOT(PowerSwitch_xt2_changed(QVariant)));
 connect(data_manager->getData("PowerSwitch_xt1"), SIGNAL(valueChanged(QVariant)), this, SLOT(PowerSwitch_xt1_changed(QVariant)));
 connect(data_manager->getData("ELECTROBOT_CDE_POWER_SWITCH_TxSync"), SIGNAL(valueChanged(QVariant)), this, SLOT(Synchro_changed(QVariant)));

}
//___________________________________________________________________________
/*!
  \brief Fonction appelée lorsque la data est modifée
  \param val la nouvelle valeur de la data
*/
void CTrame_ELECTROBOT_CDE_POWER_SWITCH::PowerSwitch_xt8_changed(QVariant val)
{
  PowerSwitch_xt8 = val.toBool();
  if (m_synchro_tx == 0) { Encode(); }
}
//___________________________________________________________________________
/*!
  \brief Fonction appelée lorsque la data est modifée
  \param val la nouvelle valeur de la data
*/
void CTrame_ELECTROBOT_CDE_POWER_SWITCH::PowerSwitch_xt7_changed(QVariant val)
{
  PowerSwitch_xt7 = val.toBool();
  if (m_synchro_tx == 0) { Encode(); }
}
//___________________________________________________________________________
/*!
  \brief Fonction appelée lorsque la data est modifée
  \param val la nouvelle valeur de la data
*/
void CTrame_ELECTROBOT_CDE_POWER_SWITCH::PowerSwitch_xt6_changed(QVariant val)
{
  PowerSwitch_xt6 = val.toBool();
  if (m_synchro_tx == 0) { Encode(); }
}
//___________________________________________________________________________
/*!
  \brief Fonction appelée lorsque la data est modifée
  \param val la nouvelle valeur de la data
*/
void CTrame_ELECTROBOT_CDE_POWER_SWITCH::PowerSwitch_xt5_changed(QVariant val)
{
  PowerSwitch_xt5 = val.toBool();
  if (m_synchro_tx == 0) { Encode(); }
}
//___________________________________________________________________________
/*!
  \brief Fonction appelée lorsque la data est modifée
  \param val la nouvelle valeur de la data
*/
void CTrame_ELECTROBOT_CDE_POWER_SWITCH::PowerSwitch_xt4_changed(QVariant val)
{
  PowerSwitch_xt4 = val.toBool();
  if (m_synchro_tx == 0) { Encode(); }
}
//___________________________________________________________________________
/*!
  \brief Fonction appelée lorsque la data est modifée
  \param val la nouvelle valeur de la data
*/
void CTrame_ELECTROBOT_CDE_POWER_SWITCH::PowerSwitch_xt3_changed(QVariant val)
{
  PowerSwitch_xt3 = val.toBool();
  if (m_synchro_tx == 0) { Encode(); }
}
//___________________________________________________________________________
/*!
  \brief Fonction appelée lorsque la data est modifée
  \param val la nouvelle valeur de la data
*/
void CTrame_ELECTROBOT_CDE_POWER_SWITCH::PowerSwitch_xt2_changed(QVariant val)
{
  PowerSwitch_xt2 = val.toBool();
  if (m_synchro_tx == 0) { Encode(); }
}
//___________________________________________________________________________
/*!
  \brief Fonction appelée lorsque la data est modifée
  \param val la nouvelle valeur de la data
*/
void CTrame_ELECTROBOT_CDE_POWER_SWITCH::PowerSwitch_xt1_changed(QVariant val)
{
  PowerSwitch_xt1 = val.toBool();
  if (m_synchro_tx == 0) { Encode(); }
}
//___________________________________________________________________________
/*!
  \brief Fonction appelée lorsque la data est modifée
  \param val la nouvelle valeur de la data
*/
void CTrame_ELECTROBOT_CDE_POWER_SWITCH::Synchro_changed(QVariant val)
{
  m_synchro_tx = val.toBool();
  if (m_synchro_tx == 0) { Encode(); }
}

//___________________________________________________________________________
/*!
  \brief Encode et envoie la trame
*/
void CTrame_ELECTROBOT_CDE_POWER_SWITCH::Encode(void)
{
  tStructTrameBrute trame;

  // Informations générales
  trame.ID = ID_ELECTROBOT_CDE_POWER_SWITCH;
  trame.DLC = DLC_ELECTROBOT_CDE_POWER_SWITCH;

 for (unsigned int i=0; i<m_dlc; i++) {
     trame.Data[i] = 0;
 }
  // Encode chacun des signaux de la trame
    trame.Data[0] |= ( (PowerSwitch_xt1 << 0) |
                       (PowerSwitch_xt2 << 1) |
                       (PowerSwitch_xt3 << 2) |
                       (PowerSwitch_xt4 << 3) |
                       (PowerSwitch_xt5 << 4) |
                       (PowerSwitch_xt6 << 5) |
                       (PowerSwitch_xt7 << 6) |
                       (PowerSwitch_xt8 << 7) );

  // Envoie la trame
  m_messagerie_bot->SerialiseTrame(&trame);

  // Comptabilise le nombre de trames émises
  m_nombre_emis++;
}

// ========================================================
//             TRAME COMMANDE_POWER_ELECTROBOT
// ========================================================
CTrame_COMMANDE_POWER_ELECTROBOT::CTrame_COMMANDE_POWER_ELECTROBOT(CMessagerieBot *messagerie_bot, CDataManager *data_manager)
    : CTrameBot(messagerie_bot, data_manager)
{
 m_name = "COMMANDE_POWER_ELECTROBOT";
 m_id = ID_COMMANDE_POWER_ELECTROBOT;
 m_dlc = DLC_COMMANDE_POWER_ELECTROBOT;
 m_liste_noms_signaux.append("commande");
 m_liste_noms_signaux.append("value");

 // Initialise les données de la messagerie
 commande = 0;
 value = 0;
 m_synchro_tx = 0;

 // S'assure que les données existent dans le DataManager
 data_manager->write("PowerElectrobot.commande",  commande);
 data_manager->write("PowerElectrobot.value",  value);
 data_manager->write("COMMANDE_POWER_ELECTROBOT_TxSync",  m_synchro_tx);

 // Connexion avec le DataManager
 connect(data_manager->getData("PowerElectrobot.commande"), SIGNAL(valueChanged(QVariant)), this, SLOT(commande_changed(QVariant)));
 connect(data_manager->getData("PowerElectrobot.value"), SIGNAL(valueChanged(QVariant)), this, SLOT(value_changed(QVariant)));
 connect(data_manager->getData("COMMANDE_POWER_ELECTROBOT_TxSync"), SIGNAL(valueChanged(QVariant)), this, SLOT(Synchro_changed(QVariant)));
}
//___________________________________________________________________________
/*!
  \brief Fonction appelée lorsque la data est modifée
  \param val la nouvelle valeur de la data
*/
void CTrame_COMMANDE_POWER_ELECTROBOT::commande_changed(QVariant val)
{
  commande = val.toInt();
  if (m_synchro_tx == 0) { Encode(); }
}
//___________________________________________________________________________
/*!
  \brief Fonction appelée lorsque la data est modifée
  \param val la nouvelle valeur de la data
*/
void CTrame_COMMANDE_POWER_ELECTROBOT::value_changed(QVariant val)
{
  value = val.toInt();
  if (m_synchro_tx == 0) { Encode(); }
}
//___________________________________________________________________________
/*!
  \brief Fonction appelée lorsque la data est modifée
  \param val la nouvelle valeur de la data
*/
void CTrame_COMMANDE_POWER_ELECTROBOT::Synchro_changed(QVariant val)
{
  m_synchro_tx = val.toBool();
  if (m_synchro_tx == 0) { Encode(); }
}

//___________________________________________________________________________
/*!
  \brief Encode et envoie la trame
*/
void CTrame_COMMANDE_POWER_ELECTROBOT::Encode(void)
{
  tStructTrameBrute trame;

  // Informations générales
  trame.ID = ID_COMMANDE_POWER_ELECTROBOT;
  trame.DLC = DLC_COMMANDE_POWER_ELECTROBOT;

 for (unsigned int i=0; i<m_dlc; i++) {
     trame.Data[i] = 0;
 }
  // Encode chacun des signaux de la trame
  trame.Data[0] |= commande>>8;
  trame.Data[1] |= (unsigned char)(commande&0xFF);

  trame.Data[2] |= value>>8;
  trame.Data[3] |= (unsigned char)(value&0xFF);

  // Envoie la trame
  m_messagerie_bot->SerialiseTrame(&trame);

  // Comptabilise le nombre de trames émises
  m_nombre_emis++;
}

// ========================================================
//             TRAME ETAT_POWER_ELECTROBOT
// ========================================================
CTrame_ETAT_POWER_ELECTROBOT::CTrame_ETAT_POWER_ELECTROBOT(CMessagerieBot *messagerie_bot, CDataManager *data_manager)
    : CTrameBot(messagerie_bot, data_manager)
{
 m_name = "ETAT_POWER_ELECTROBOT";
 m_id = ID_ETAT_POWER_ELECTROBOT;
 m_dlc = DLC_ETAT_POWER_ELECTROBOT;
 m_liste_noms_signaux.append("PowerElectrobot.battery_voltage_mV");
 m_liste_noms_signaux.append("PowerElectrobot.global_current_mA");
 m_liste_noms_signaux.append("PowerElectrobot.current_out1_mA");
 m_liste_noms_signaux.append("PowerElectrobot.current_out2_mA");

 // S'assure que les données existent dans le DataManager
 data_manager->write("PowerElectrobot.battery_voltage", BRUTE2PHYS_battery_voltage(battery_voltage_mV));
 data_manager->write("PowerElectrobot.global_current", BRUTE2PHYS_global_current(global_current_mA));
 data_manager->write("PowerElectrobot.current_out1", BRUTE2PHYS_current_out1(current_out1_mA));
 data_manager->write("PowerElectrobot.current_out2", BRUTE2PHYS_current_out2(current_out2_mA));
}
//___________________________________________________________________________
/*!
  \brief Decode les signaux de la trame
  \param trameRecue la trame brute recue a decoder
*/
void CTrame_ETAT_POWER_ELECTROBOT::Decode(tStructTrameBrute *trameRecue)
{
    // Decode les signaux de la trame
    battery_voltage_mV = ( ( ((unsigned short)(trameRecue->Data[1])) & 0xFF) )  |  ( ( ((unsigned short)(trameRecue->Data[0])) & 0xFF) << 8 );
    global_current_mA = ( ( ((unsigned short)(trameRecue->Data[3])) & 0xFF) )  |  ( ( ((unsigned short)(trameRecue->Data[2])) & 0xFF) << 8 );
    current_out1_mA = ( ( ((unsigned short)(trameRecue->Data[5])) & 0xFF) )  |  ( ( ((unsigned short)(trameRecue->Data[4])) & 0xFF) << 8 );
    current_out2_mA = ( ( ((unsigned short)(trameRecue->Data[7])) & 0xFF) )  |  ( ( ((unsigned short)(trameRecue->Data[6])) & 0xFF) << 8 );

   // Envoie les données au data manager
   m_data_manager->write("PowerElectrobot.battery_voltage", BRUTE2PHYS_battery_voltage(battery_voltage_mV));
   m_data_manager->write("PowerElectrobot.global_current", BRUTE2PHYS_global_current(global_current_mA));
   m_data_manager->write("PowerElectrobot.current_out1", BRUTE2PHYS_current_out1(current_out1_mA));
   m_data_manager->write("PowerElectrobot.current_out2", BRUTE2PHYS_current_out2(current_out2_mA));
   // Comptabilise la reception de cette trame
   m_nombre_recue++;
}

// ========================================================
//             TRAME COMMANDE_MVT_XY
// ========================================================
CTrame_COMMANDE_MVT_XY::CTrame_COMMANDE_MVT_XY(CMessagerieBot *messagerie_bot, CDataManager *data_manager)
    : CTrameBot(messagerie_bot, data_manager)
{
 m_name = "COMMANDE_MVT_XY";
 m_id = ID_COMMANDE_MVT_XY;
 m_dlc = DLC_COMMANDE_MVT_XY;
 m_liste_noms_signaux.append("Y_consigne");
 m_liste_noms_signaux.append("X_consigne");
 m_liste_noms_signaux.append("Type_mouvement");

 // Initialise les données de la messagerie
 Y_consigne = 0;
 X_consigne = 0;
 Type_mouvement = 0;
 m_synchro_tx = 0;

 // S'assure que les données existent dans le DataManager
 data_manager->write("Y_consigne",  Y_consigne);
 data_manager->write("X_consigne",  X_consigne);
 data_manager->write("Type_mouvement",  Type_mouvement);
 data_manager->write("COMMANDE_MVT_XY_TxSync",  m_synchro_tx);

 // Connexion avec le DataManager
 connect(data_manager->getData("Y_consigne"), SIGNAL(valueChanged(QVariant)), this, SLOT(Y_consigne_changed(QVariant)));
 connect(data_manager->getData("X_consigne"), SIGNAL(valueChanged(QVariant)), this, SLOT(X_consigne_changed(QVariant)));
 connect(data_manager->getData("Type_mouvement"), SIGNAL(valueChanged(QVariant)), this, SLOT(Type_mouvement_changed(QVariant)));
 connect(data_manager->getData("COMMANDE_MVT_XY_TxSync"), SIGNAL(valueChanged(QVariant)), this, SLOT(Synchro_changed(QVariant)));

}
//___________________________________________________________________________
/*!
  \brief Fonction appelée lorsque la data est modifée
  \param val la nouvelle valeur de la data
*/
void CTrame_COMMANDE_MVT_XY::Y_consigne_changed(QVariant val)
{
  Y_consigne = val.toInt();
  if (m_synchro_tx == 0) { Encode(); }
}
//___________________________________________________________________________
/*!
  \brief Fonction appelée lorsque la data est modifée
  \param val la nouvelle valeur de la data
*/
void CTrame_COMMANDE_MVT_XY::X_consigne_changed(QVariant val)
{
  X_consigne = val.toInt();
  if (m_synchro_tx == 0) { Encode(); }
}
//___________________________________________________________________________
/*!
  \brief Fonction appelée lorsque la data est modifée
  \param val la nouvelle valeur de la data
*/
void CTrame_COMMANDE_MVT_XY::Type_mouvement_changed(QVariant val)
{
  Type_mouvement = val.toInt();
  if (m_synchro_tx == 0) { Encode(); }
}
//___________________________________________________________________________
/*!
  \brief Fonction appelée lorsque la data est modifée
  \param val la nouvelle valeur de la data
*/
void CTrame_COMMANDE_MVT_XY::Synchro_changed(QVariant val)
{
  m_synchro_tx = val.toBool();
  if (m_synchro_tx == 0) { Encode(); }
}

//___________________________________________________________________________
/*!
  \brief Encode et envoie la trame
*/
void CTrame_COMMANDE_MVT_XY::Encode(void)
{
  tStructTrameBrute trame;

  // Informations générales
  trame.ID = ID_COMMANDE_MVT_XY;
  trame.DLC = DLC_COMMANDE_MVT_XY;

 for (unsigned int i=0; i<m_dlc; i++) {
     trame.Data[i] = 0;
 }
  // Encode chacun des signaux de la trame
    trame.Data[3] |= (unsigned char)( ( (Y_consigne) & 0xFF) );
    trame.Data[2] |= (unsigned char)( ( (Y_consigne >> 8) & 0xFF) );

    trame.Data[1] |= (unsigned char)( ( (X_consigne) & 0xFF) );
    trame.Data[0] |= (unsigned char)( ( (X_consigne >> 8) & 0xFF) );

    trame.Data[4] |= (unsigned char)( ( (Type_mouvement) & 0xFF) );

  // Envoie la trame
  m_messagerie_bot->SerialiseTrame(&trame);

  // Comptabilise le nombre de trames émises
  m_nombre_emis++;
}

// ========================================================
//             TRAME ASSERV_RESET
// ========================================================
CTrame_ASSERV_RESET::CTrame_ASSERV_RESET(CMessagerieBot *messagerie_bot, CDataManager *data_manager)
    : CTrameBot(messagerie_bot, data_manager)
{
 m_name = "ASSERV_RESET";
 m_id = ID_ASSERV_RESET;
 m_dlc = DLC_ASSERV_RESET;
 m_liste_noms_signaux.append("SECURITE_RESET");

 // Initialise les données de la messagerie
 SECURITE_RESET = 0;
 m_synchro_tx = 0;

 // S'assure que les données existent dans le DataManager
 data_manager->write("SECURITE_RESET",  SECURITE_RESET);
 data_manager->write("ASSERV_RESET_TxSync",  m_synchro_tx);

 // Connexion avec le DataManager
 connect(data_manager->getData("SECURITE_RESET"), SIGNAL(valueChanged(QVariant)), this, SLOT(SECURITE_RESET_changed(QVariant)));
 connect(data_manager->getData("ASSERV_RESET_TxSync"), SIGNAL(valueChanged(QVariant)), this, SLOT(Synchro_changed(QVariant)));

}
//___________________________________________________________________________
/*!
  \brief Fonction appelée lorsque la data est modifée
  \param val la nouvelle valeur de la data
*/
void CTrame_ASSERV_RESET::SECURITE_RESET_changed(QVariant val)
{
  SECURITE_RESET = val.toInt();
  if (m_synchro_tx == 0) { Encode(); }
}
//___________________________________________________________________________
/*!
  \brief Fonction appelée lorsque la data est modifée
  \param val la nouvelle valeur de la data
*/
void CTrame_ASSERV_RESET::Synchro_changed(QVariant val)
{
  m_synchro_tx = val.toBool();
  if (m_synchro_tx == 0) { Encode(); }
}

//___________________________________________________________________________
/*!
  \brief Encode et envoie la trame
*/
void CTrame_ASSERV_RESET::Encode(void)
{
  tStructTrameBrute trame;

  // Informations générales
  trame.ID = ID_ASSERV_RESET;
  trame.DLC = DLC_ASSERV_RESET;

 for (unsigned int i=0; i<m_dlc; i++) {
     trame.Data[i] = 0;
 }
  // Encode chacun des signaux de la trame
    trame.Data[0] |= (unsigned char)( ( (SECURITE_RESET) & 0xFF) );

  // Envoie la trame
  m_messagerie_bot->SerialiseTrame(&trame);

  // Comptabilise le nombre de trames émises
  m_nombre_emis++;
}

// ========================================================
//             TRAME AUTOAPPRENTISSAGE_ASSERV
// ========================================================
CTrame_AUTOAPPRENTISSAGE_ASSERV::CTrame_AUTOAPPRENTISSAGE_ASSERV(CMessagerieBot *messagerie_bot, CDataManager *data_manager)
    : CTrameBot(messagerie_bot, data_manager)
{
 m_name = "AUTOAPPRENTISSAGE_ASSERV";
 m_id = ID_AUTOAPPRENTISSAGE_ASSERV;
 m_dlc = DLC_AUTOAPPRENTISSAGE_ASSERV;
 m_liste_noms_signaux.append("Type_autoapprentissage");

 // Initialise les données de la messagerie
 Type_autoapprentissage = 0;
 m_synchro_tx = 0;

 // S'assure que les données existent dans le DataManager
 data_manager->write("Type_autoapprentissage",  Type_autoapprentissage);
 data_manager->write("AUTOAPPRENTISSAGE_ASSERV_TxSync",  m_synchro_tx);

 // Connexion avec le DataManager
 connect(data_manager->getData("Type_autoapprentissage"), SIGNAL(valueChanged(QVariant)), this, SLOT(Type_autoapprentissage_changed(QVariant)));
 connect(data_manager->getData("AUTOAPPRENTISSAGE_ASSERV_TxSync"), SIGNAL(valueChanged(QVariant)), this, SLOT(Synchro_changed(QVariant)));

}
//___________________________________________________________________________
/*!
  \brief Fonction appelée lorsque la data est modifée
  \param val la nouvelle valeur de la data
*/
void CTrame_AUTOAPPRENTISSAGE_ASSERV::Type_autoapprentissage_changed(QVariant val)
{
  Type_autoapprentissage = val.toInt();
  if (m_synchro_tx == 0) { Encode(); }
}
//___________________________________________________________________________
/*!
  \brief Fonction appelée lorsque la data est modifée
  \param val la nouvelle valeur de la data
*/
void CTrame_AUTOAPPRENTISSAGE_ASSERV::Synchro_changed(QVariant val)
{
  m_synchro_tx = val.toBool();
  if (m_synchro_tx == 0) { Encode(); }
}

//___________________________________________________________________________
/*!
  \brief Encode et envoie la trame
*/
void CTrame_AUTOAPPRENTISSAGE_ASSERV::Encode(void)
{
  tStructTrameBrute trame;

  // Informations générales
  trame.ID = ID_AUTOAPPRENTISSAGE_ASSERV;
  trame.DLC = DLC_AUTOAPPRENTISSAGE_ASSERV;

 for (unsigned int i=0; i<m_dlc; i++) {
     trame.Data[i] = 0;
 }
  // Encode chacun des signaux de la trame
    trame.Data[0] |= (unsigned char)( ( (Type_autoapprentissage) & 0xFF) );

  // Envoie la trame
  m_messagerie_bot->SerialiseTrame(&trame);

  // Comptabilise le nombre de trames émises
  m_nombre_emis++;
}

// ========================================================
//             TRAME COMMANDE_REINIT_XY_TETA
// ========================================================
CTrame_COMMANDE_REINIT_XY_TETA::CTrame_COMMANDE_REINIT_XY_TETA(CMessagerieBot *messagerie_bot, CDataManager *data_manager)
    : CTrameBot(messagerie_bot, data_manager)
{
 m_name = "COMMANDE_REINIT_XY_TETA";
 m_id = ID_COMMANDE_REINIT_XY_TETA;
 m_dlc = DLC_COMMANDE_REINIT_XY_TETA;
 m_liste_noms_signaux.append("reinit_teta_pos");
 m_liste_noms_signaux.append("reinit_y_pos");
 m_liste_noms_signaux.append("reinit_x_pos");

 // Initialise les données de la messagerie
 reinit_teta_pos = 0;
 reinit_y_pos = 0;
 reinit_x_pos = 0;
 m_synchro_tx = 0;

 // S'assure que les données existent dans le DataManager
 data_manager->write("reinit_teta_pos",  reinit_teta_pos);
 data_manager->write("reinit_y_pos",  reinit_y_pos);
 data_manager->write("reinit_x_pos",  reinit_x_pos);
 data_manager->write("COMMANDE_REINIT_XY_TETA_TxSync",  m_synchro_tx);

 // Connexion avec le DataManager
 connect(data_manager->getData("reinit_teta_pos"), SIGNAL(valueChanged(QVariant)), this, SLOT(reinit_teta_pos_changed(QVariant)));
 connect(data_manager->getData("reinit_y_pos"), SIGNAL(valueChanged(QVariant)), this, SLOT(reinit_y_pos_changed(QVariant)));
 connect(data_manager->getData("reinit_x_pos"), SIGNAL(valueChanged(QVariant)), this, SLOT(reinit_x_pos_changed(QVariant)));
 connect(data_manager->getData("COMMANDE_REINIT_XY_TETA_TxSync"), SIGNAL(valueChanged(QVariant)), this, SLOT(Synchro_changed(QVariant)));

}
//___________________________________________________________________________
/*!
  \brief Fonction appelée lorsque la data est modifée
  \param val la nouvelle valeur de la data
*/
void CTrame_COMMANDE_REINIT_XY_TETA::reinit_teta_pos_changed(QVariant val)
{
  reinit_teta_pos = PHYS2BRUTE_reinit_teta_pos(val.toDouble());
  if (m_synchro_tx == 0) { Encode(); }
}
//___________________________________________________________________________
/*!
  \brief Fonction appelée lorsque la data est modifée
  \param val la nouvelle valeur de la data
*/
void CTrame_COMMANDE_REINIT_XY_TETA::reinit_y_pos_changed(QVariant val)
{
  reinit_y_pos = val.toInt();
  if (m_synchro_tx == 0) { Encode(); }
}
//___________________________________________________________________________
/*!
  \brief Fonction appelée lorsque la data est modifée
  \param val la nouvelle valeur de la data
*/
void CTrame_COMMANDE_REINIT_XY_TETA::reinit_x_pos_changed(QVariant val)
{
  reinit_x_pos = val.toInt();
  if (m_synchro_tx == 0) { Encode(); }
}
//___________________________________________________________________________
/*!
  \brief Fonction appelée lorsque la data est modifée
  \param val la nouvelle valeur de la data
*/
void CTrame_COMMANDE_REINIT_XY_TETA::Synchro_changed(QVariant val)
{
  m_synchro_tx = val.toBool();
  if (m_synchro_tx == 0) { Encode(); }
}

//___________________________________________________________________________
/*!
  \brief Encode et envoie la trame
*/
void CTrame_COMMANDE_REINIT_XY_TETA::Encode(void)
{
  tStructTrameBrute trame;

  // Informations générales
  trame.ID = ID_COMMANDE_REINIT_XY_TETA;
  trame.DLC = DLC_COMMANDE_REINIT_XY_TETA;

 for (unsigned int i=0; i<m_dlc; i++) {
     trame.Data[i] = 0;
 }
  // Encode chacun des signaux de la trame
    trame.Data[5] |= (unsigned char)( ( (reinit_teta_pos) & 0xFF) );
    trame.Data[4] |= (unsigned char)( ( (reinit_teta_pos >> 8) & 0xFF) );

    trame.Data[3] |= (unsigned char)( ( (reinit_y_pos) & 0xFF) );
    trame.Data[2] |= (unsigned char)( ( (reinit_y_pos >> 8) & 0xFF) );

    trame.Data[1] |= (unsigned char)( ( (reinit_x_pos) & 0xFF) );
    trame.Data[0] |= (unsigned char)( ( (reinit_x_pos >> 8) & 0xFF) );

  // Envoie la trame
  m_messagerie_bot->SerialiseTrame(&trame);

  // Comptabilise le nombre de trames émises
  m_nombre_emis++;
}

// ========================================================
//             TRAME COMMANDE_VITESSE_MVT
// ========================================================
CTrame_COMMANDE_VITESSE_MVT::CTrame_COMMANDE_VITESSE_MVT(CMessagerieBot *messagerie_bot, CDataManager *data_manager)
    : CTrameBot(messagerie_bot, data_manager)
{
 m_name = "COMMANDE_VITESSE_MVT";
 m_id = ID_COMMANDE_VITESSE_MVT;
 m_dlc = DLC_COMMANDE_VITESSE_MVT;
 m_liste_noms_signaux.append("indice_sportivite_decel");
 m_liste_noms_signaux.append("indice_sportivite_accel");
 m_liste_noms_signaux.append("vitesse_rotation_max");
 m_liste_noms_signaux.append("vitesse_avance_max");

 // Initialise les données de la messagerie
 indice_sportivite_decel = 0;
 indice_sportivite_accel = 0;
 vitesse_rotation_max = 0;
 vitesse_avance_max = 0;
 m_synchro_tx = 0;

 // S'assure que les données existent dans le DataManager
 data_manager->write("indice_sportivite_decel",  indice_sportivite_decel);
 data_manager->write("indice_sportivite_accel",  indice_sportivite_accel);
 data_manager->write("vitesse_rotation_max",  vitesse_rotation_max);
 data_manager->write("vitesse_avance_max",  vitesse_avance_max);
 data_manager->write("COMMANDE_VITESSE_MVT_TxSync",  m_synchro_tx);

 // Connexion avec le DataManager
 connect(data_manager->getData("indice_sportivite_decel"), SIGNAL(valueChanged(QVariant)), this, SLOT(indice_sportivite_decel_changed(QVariant)));
 connect(data_manager->getData("indice_sportivite_accel"), SIGNAL(valueChanged(QVariant)), this, SLOT(indice_sportivite_accel_changed(QVariant)));
 connect(data_manager->getData("vitesse_rotation_max"), SIGNAL(valueChanged(QVariant)), this, SLOT(vitesse_rotation_max_changed(QVariant)));
 connect(data_manager->getData("vitesse_avance_max"), SIGNAL(valueChanged(QVariant)), this, SLOT(vitesse_avance_max_changed(QVariant)));
 connect(data_manager->getData("COMMANDE_VITESSE_MVT_TxSync"), SIGNAL(valueChanged(QVariant)), this, SLOT(Synchro_changed(QVariant)));

}
//___________________________________________________________________________
/*!
  \brief Fonction appelée lorsque la data est modifée
  \param val la nouvelle valeur de la data
*/
void CTrame_COMMANDE_VITESSE_MVT::indice_sportivite_decel_changed(QVariant val)
{
  indice_sportivite_decel = val.toInt();
  if (m_synchro_tx == 0) { Encode(); }
}
//___________________________________________________________________________
/*!
  \brief Fonction appelée lorsque la data est modifée
  \param val la nouvelle valeur de la data
*/
void CTrame_COMMANDE_VITESSE_MVT::indice_sportivite_accel_changed(QVariant val)
{
  indice_sportivite_accel = val.toInt();
  if (m_synchro_tx == 0) { Encode(); }
}
//___________________________________________________________________________
/*!
  \brief Fonction appelée lorsque la data est modifée
  \param val la nouvelle valeur de la data
*/
void CTrame_COMMANDE_VITESSE_MVT::vitesse_rotation_max_changed(QVariant val)
{
  vitesse_rotation_max = PHYS2BRUTE_vitesse_rotation_max(val.toDouble());
  if (m_synchro_tx == 0) { Encode(); }
}
//___________________________________________________________________________
/*!
  \brief Fonction appelée lorsque la data est modifée
  \param val la nouvelle valeur de la data
*/
void CTrame_COMMANDE_VITESSE_MVT::vitesse_avance_max_changed(QVariant val)
{
  vitesse_avance_max = val.toInt();
  if (m_synchro_tx == 0) { Encode(); }
}
//___________________________________________________________________________
/*!
  \brief Fonction appelée lorsque la data est modifée
  \param val la nouvelle valeur de la data
*/
void CTrame_COMMANDE_VITESSE_MVT::Synchro_changed(QVariant val)
{
  m_synchro_tx = val.toBool();
  if (m_synchro_tx == 0) { Encode(); }
}

//___________________________________________________________________________
/*!
  \brief Encode et envoie la trame
*/
void CTrame_COMMANDE_VITESSE_MVT::Encode(void)
{
  tStructTrameBrute trame;

  // Informations générales
  trame.ID = ID_COMMANDE_VITESSE_MVT;
  trame.DLC = DLC_COMMANDE_VITESSE_MVT;

 for (unsigned int i=0; i<m_dlc; i++) {
     trame.Data[i] = 0;
 }
  // Encode chacun des signaux de la trame
    trame.Data[5] |= (unsigned char)( ( (indice_sportivite_decel) & 0xFF) );

    trame.Data[4] |= (unsigned char)( ( (indice_sportivite_accel) & 0xFF) );

    trame.Data[3] |= (unsigned char)( ( (vitesse_rotation_max) & 0xFF) );
    trame.Data[2] |= (unsigned char)( ( (vitesse_rotation_max >> 8) & 0xFF) );

    trame.Data[1] |= (unsigned char)( ( (vitesse_avance_max) & 0xFF) );
    trame.Data[0] |= (unsigned char)( ( (vitesse_avance_max >> 8) & 0xFF) );

  // Envoie la trame
  m_messagerie_bot->SerialiseTrame(&trame);

  // Comptabilise le nombre de trames émises
  m_nombre_emis++;
}

// ========================================================
//             TRAME COMMANDE_REGUL_VITESSE
// ========================================================
CTrame_COMMANDE_REGUL_VITESSE::CTrame_COMMANDE_REGUL_VITESSE(CMessagerieBot *messagerie_bot, CDataManager *data_manager)
    : CTrameBot(messagerie_bot, data_manager)
{
 m_name = "COMMANDE_REGUL_VITESSE";
 m_id = ID_COMMANDE_REGUL_VITESSE;
 m_dlc = DLC_COMMANDE_REGUL_VITESSE;
 m_liste_noms_signaux.append("consigne_vitesse_roue_D");
 m_liste_noms_signaux.append("consigne_vitesse_roue_G");

 // Initialise les données de la messagerie
 consigne_vitesse_roue_D = 0;
 consigne_vitesse_roue_G = 0;
 m_synchro_tx = 0;

 // S'assure que les données existent dans le DataManager
 data_manager->write("consigne_vitesse_roue_D",  consigne_vitesse_roue_D);
 data_manager->write("consigne_vitesse_roue_G",  consigne_vitesse_roue_G);
 data_manager->write("COMMANDE_REGUL_VITESSE_TxSync",  m_synchro_tx);

 // Connexion avec le DataManager
 connect(data_manager->getData("consigne_vitesse_roue_D"), SIGNAL(valueChanged(QVariant)), this, SLOT(consigne_vitesse_roue_D_changed(QVariant)));
 connect(data_manager->getData("consigne_vitesse_roue_G"), SIGNAL(valueChanged(QVariant)), this, SLOT(consigne_vitesse_roue_G_changed(QVariant)));
 connect(data_manager->getData("COMMANDE_REGUL_VITESSE_TxSync"), SIGNAL(valueChanged(QVariant)), this, SLOT(Synchro_changed(QVariant)));

}
//___________________________________________________________________________
/*!
  \brief Fonction appelée lorsque la data est modifée
  \param val la nouvelle valeur de la data
*/
void CTrame_COMMANDE_REGUL_VITESSE::consigne_vitesse_roue_D_changed(QVariant val)
{
  consigne_vitesse_roue_D = val.toInt();
  if (m_synchro_tx == 0) { Encode(); }
}
//___________________________________________________________________________
/*!
  \brief Fonction appelée lorsque la data est modifée
  \param val la nouvelle valeur de la data
*/
void CTrame_COMMANDE_REGUL_VITESSE::consigne_vitesse_roue_G_changed(QVariant val)
{
  consigne_vitesse_roue_G = val.toInt();
  if (m_synchro_tx == 0) { Encode(); }
}
//___________________________________________________________________________
/*!
  \brief Fonction appelée lorsque la data est modifée
  \param val la nouvelle valeur de la data
*/
void CTrame_COMMANDE_REGUL_VITESSE::Synchro_changed(QVariant val)
{
  m_synchro_tx = val.toBool();
  if (m_synchro_tx == 0) { Encode(); }
}

//___________________________________________________________________________
/*!
  \brief Encode et envoie la trame
*/
void CTrame_COMMANDE_REGUL_VITESSE::Encode(void)
{
  tStructTrameBrute trame;

  // Informations générales
  trame.ID = ID_COMMANDE_REGUL_VITESSE;
  trame.DLC = DLC_COMMANDE_REGUL_VITESSE;

 for (unsigned int i=0; i<m_dlc; i++) {
     trame.Data[i] = 0;
 }
  // Encode chacun des signaux de la trame
    trame.Data[3] |= (unsigned char)( ( (consigne_vitesse_roue_D) & 0xFF) );
    trame.Data[2] |= (unsigned char)( ( (consigne_vitesse_roue_D >> 8) & 0xFF) );

    trame.Data[1] |= (unsigned char)( ( (consigne_vitesse_roue_G) & 0xFF) );
    trame.Data[0] |= (unsigned char)( ( (consigne_vitesse_roue_G >> 8) & 0xFF) );

  // Envoie la trame
  m_messagerie_bot->SerialiseTrame(&trame);

  // Comptabilise le nombre de trames émises
  m_nombre_emis++;
}

// ========================================================
//             TRAME COMMANDE_DISTANCE_ANGLE
// ========================================================
CTrame_COMMANDE_DISTANCE_ANGLE::CTrame_COMMANDE_DISTANCE_ANGLE(CMessagerieBot *messagerie_bot, CDataManager *data_manager)
    : CTrameBot(messagerie_bot, data_manager)
{
 m_name = "COMMANDE_DISTANCE_ANGLE";
 m_id = ID_COMMANDE_DISTANCE_ANGLE;
 m_dlc = DLC_COMMANDE_DISTANCE_ANGLE;
 m_liste_noms_signaux.append("angle_consigne");
 m_liste_noms_signaux.append("distance_consigne");
 m_liste_noms_signaux.append("priorite_mouvement");

 // Initialise les données de la messagerie
 angle_consigne = 0;
 distance_consigne = 0;
 priorite_mouvement = 0;
 m_synchro_tx = 0;

 // S'assure que les données existent dans le DataManager
 data_manager->write("angle_consigne",  angle_consigne);
 data_manager->write("distance_consigne",  distance_consigne);
 data_manager->write("priorite_mouvement",  priorite_mouvement);
 data_manager->write("COMMANDE_DISTANCE_ANGLE_TxSync",  m_synchro_tx);

 // Connexion avec le DataManager
 connect(data_manager->getData("angle_consigne"), SIGNAL(valueChanged(QVariant)), this, SLOT(angle_consigne_changed(QVariant)));
 connect(data_manager->getData("distance_consigne"), SIGNAL(valueChanged(QVariant)), this, SLOT(distance_consigne_changed(QVariant)));
 connect(data_manager->getData("priorite_mouvement"), SIGNAL(valueChanged(QVariant)), this, SLOT(priorite_mouvement_changed(QVariant)));
 connect(data_manager->getData("COMMANDE_DISTANCE_ANGLE_TxSync"), SIGNAL(valueChanged(QVariant)), this, SLOT(Synchro_changed(QVariant)));

}
//___________________________________________________________________________
/*!
  \brief Fonction appelée lorsque la data est modifée
  \param val la nouvelle valeur de la data
*/
void CTrame_COMMANDE_DISTANCE_ANGLE::angle_consigne_changed(QVariant val)
{
  angle_consigne = PHYS2BRUTE_angle_consigne(val.toDouble());
  if (m_synchro_tx == 0) { Encode(); }
}
//___________________________________________________________________________
/*!
  \brief Fonction appelée lorsque la data est modifée
  \param val la nouvelle valeur de la data
*/
void CTrame_COMMANDE_DISTANCE_ANGLE::distance_consigne_changed(QVariant val)
{
  distance_consigne = val.toInt();
  if (m_synchro_tx == 0) { Encode(); }
}
//___________________________________________________________________________
/*!
  \brief Fonction appelée lorsque la data est modifée
  \param val la nouvelle valeur de la data
*/
void CTrame_COMMANDE_DISTANCE_ANGLE::priorite_mouvement_changed(QVariant val)
{
  priorite_mouvement = val.toInt();
  if (m_synchro_tx == 0) { Encode(); }
}
//___________________________________________________________________________
/*!
  \brief Fonction appelée lorsque la data est modifée
  \param val la nouvelle valeur de la data
*/
void CTrame_COMMANDE_DISTANCE_ANGLE::Synchro_changed(QVariant val)
{
  m_synchro_tx = val.toBool();
  if (m_synchro_tx == 0) { Encode(); }
}

//___________________________________________________________________________
/*!
  \brief Encode et envoie la trame
*/
void CTrame_COMMANDE_DISTANCE_ANGLE::Encode(void)
{
  tStructTrameBrute trame;

  // Informations générales
  trame.ID = ID_COMMANDE_DISTANCE_ANGLE;
  trame.DLC = DLC_COMMANDE_DISTANCE_ANGLE;

 for (unsigned int i=0; i<m_dlc; i++) {
     trame.Data[i] = 0;
 }
  // Encode chacun des signaux de la trame
    trame.Data[3] |= (unsigned char)( ( (angle_consigne) & 0xFF) );
    trame.Data[2] |= (unsigned char)( ( (angle_consigne >> 8) & 0xFF) );

    trame.Data[1] |= (unsigned char)( ( (distance_consigne) & 0xFF) );
    trame.Data[0] |= (unsigned char)( ( (distance_consigne >> 8) & 0xFF) );

    trame.Data[4] |= (unsigned char)( ( (priorite_mouvement) & 0xFF) );

  // Envoie la trame
  m_messagerie_bot->SerialiseTrame(&trame);

  // Comptabilise le nombre de trames émises
  m_nombre_emis++;
}

// ========================================================
//             TRAME COMMANDE_MVT_XY_TETA
// ========================================================
CTrame_COMMANDE_MVT_XY_TETA::CTrame_COMMANDE_MVT_XY_TETA(CMessagerieBot *messagerie_bot, CDataManager *data_manager)
    : CTrameBot(messagerie_bot, data_manager)
{
 m_name = "COMMANDE_MVT_XY_TETA";
 m_id = ID_COMMANDE_MVT_XY_TETA;
 m_dlc = DLC_COMMANDE_MVT_XY_TETA;
 m_liste_noms_signaux.append("XYT_angle_consigne");
 m_liste_noms_signaux.append("XYT_Y_consigne");
 m_liste_noms_signaux.append("XYT_X_consigne");
 m_liste_noms_signaux.append("XYT_Type_mouvement");

 // Initialise les données de la messagerie
 angle_consigne = 0;
 Y_consigne = 0;
 X_consigne = 0;
 Type_mouvement = 0;
 m_synchro_tx = 0;

 // S'assure que les données existent dans le DataManager
 data_manager->write("XYT_angle_consigne",  angle_consigne);
 data_manager->write("XYT_Y_consigne",  Y_consigne);
 data_manager->write("XYT_X_consigne",  X_consigne);
 data_manager->write("XYT_Type_mouvement",  Type_mouvement);
 data_manager->write("COMMANDE_MVT_XY_TETA_TxSync",  m_synchro_tx);

 // Connexion avec le DataManager
 connect(data_manager->getData("XYT_angle_consigne"), SIGNAL(valueChanged(QVariant)), this, SLOT(angle_consigne_changed(QVariant)));
 connect(data_manager->getData("XYT_Y_consigne"), SIGNAL(valueChanged(QVariant)), this, SLOT(Y_consigne_changed(QVariant)));
 connect(data_manager->getData("XYT_X_consigne"), SIGNAL(valueChanged(QVariant)), this, SLOT(X_consigne_changed(QVariant)));
 connect(data_manager->getData("XYT_Type_mouvement"), SIGNAL(valueChanged(QVariant)), this, SLOT(Type_mouvement_changed(QVariant)));
 connect(data_manager->getData("COMMANDE_MVT_XY_TETA_TxSync"), SIGNAL(valueChanged(QVariant)), this, SLOT(Synchro_changed(QVariant)));

}
//___________________________________________________________________________
/*!
  \brief Fonction appelée lorsque la data est modifée
  \param val la nouvelle valeur de la data
*/
void CTrame_COMMANDE_MVT_XY_TETA::angle_consigne_changed(QVariant val)
{
  angle_consigne = PHYS2BRUTE_angle_consigne(val.toDouble());
  if (m_synchro_tx == 0) { Encode(); }
}
//___________________________________________________________________________
/*!
  \brief Fonction appelée lorsque la data est modifée
  \param val la nouvelle valeur de la data
*/
void CTrame_COMMANDE_MVT_XY_TETA::Y_consigne_changed(QVariant val)
{
  Y_consigne = val.toInt();
  if (m_synchro_tx == 0) { Encode(); }
}
//___________________________________________________________________________
/*!
  \brief Fonction appelée lorsque la data est modifée
  \param val la nouvelle valeur de la data
*/
void CTrame_COMMANDE_MVT_XY_TETA::X_consigne_changed(QVariant val)
{
  X_consigne = val.toInt();
  if (m_synchro_tx == 0) { Encode(); }
}
//___________________________________________________________________________
/*!
  \brief Fonction appelée lorsque la data est modifée
  \param val la nouvelle valeur de la data
*/
void CTrame_COMMANDE_MVT_XY_TETA::Type_mouvement_changed(QVariant val)
{
  Type_mouvement = val.toInt();
  if (m_synchro_tx == 0) { Encode(); }
}
//___________________________________________________________________________
/*!
  \brief Fonction appelée lorsque la data est modifée
  \param val la nouvelle valeur de la data
*/
void CTrame_COMMANDE_MVT_XY_TETA::Synchro_changed(QVariant val)
{
  m_synchro_tx = val.toBool();
  if (m_synchro_tx == 0) { Encode(); }
}

//___________________________________________________________________________
/*!
  \brief Encode et envoie la trame
*/
void CTrame_COMMANDE_MVT_XY_TETA::Encode(void)
{
  tStructTrameBrute trame;

  // Informations générales
  trame.ID = ID_COMMANDE_MVT_XY_TETA;
  trame.DLC = DLC_COMMANDE_MVT_XY_TETA;

 for (unsigned int i=0; i<m_dlc; i++) {
     trame.Data[i] = 0;
 }
  // Encode chacun des signaux de la trame
    trame.Data[5] |= (unsigned char)( ( (angle_consigne) & 0xFF) );
    trame.Data[4] |= (unsigned char)( ( (angle_consigne >> 8) & 0xFF) );

    trame.Data[3] |= (unsigned char)( ( (Y_consigne) & 0xFF) );
    trame.Data[2] |= (unsigned char)( ( (Y_consigne >> 8) & 0xFF) );

    trame.Data[1] |= (unsigned char)( ( (X_consigne) & 0xFF) );
    trame.Data[0] |= (unsigned char)( ( (X_consigne >> 8) & 0xFF) );

    trame.Data[6] |= (unsigned char)( ( (Type_mouvement) & 0xFF) );

  // Envoie la trame
  m_messagerie_bot->SerialiseTrame(&trame);

  // Comptabilise le nombre de trames émises
  m_nombre_emis++;
}

// ========================================================
//             TRAME ASSERV_DIAG_WRITE_PARAM
// ========================================================
CTrame_ASSERV_DIAG_WRITE_PARAM::CTrame_ASSERV_DIAG_WRITE_PARAM(CMessagerieBot *messagerie_bot, CDataManager *data_manager)
    : CTrameBot(messagerie_bot, data_manager)
{
 m_name = "ASSERV_DIAG_WRITE_PARAM";
 m_id = ID_ASSERV_DIAG_WRITE_PARAM;
 m_dlc = DLC_ASSERV_DIAG_WRITE_PARAM;
 m_liste_noms_signaux.append("ASSERV_DIAG_WRITE_VALUE");
 m_liste_noms_signaux.append("ASSERV_DIAG_WRITE_PARAM");

 // Initialise les données de la messagerie
 ASSERV_DIAG_WRITE_VALUE = 0;
 ASSERV_DIAG_WRITE_PARAM = 0;
 m_synchro_tx = 0;

 // S'assure que les données existent dans le DataManager
 data_manager->write("ASSERV_DIAG_WRITE_VALUE",  ASSERV_DIAG_WRITE_VALUE);
 data_manager->write("ASSERV_DIAG_WRITE_PARAM",  ASSERV_DIAG_WRITE_PARAM);
 data_manager->write("ASSERV_DIAG_WRITE_PARAM_TxSync",  m_synchro_tx);

 // Connexion avec le DataManager
 connect(data_manager->getData("ASSERV_DIAG_WRITE_VALUE"), SIGNAL(valueChanged(QVariant)), this, SLOT(ASSERV_DIAG_WRITE_VALUE_changed(QVariant)));
 connect(data_manager->getData("ASSERV_DIAG_WRITE_PARAM"), SIGNAL(valueChanged(QVariant)), this, SLOT(ASSERV_DIAG_WRITE_PARAM_changed(QVariant)));
 connect(data_manager->getData("ASSERV_DIAG_WRITE_PARAM_TxSync"), SIGNAL(valueChanged(QVariant)), this, SLOT(Synchro_changed(QVariant)));

}
//___________________________________________________________________________
/*!
  \brief Fonction appelée lorsque la data est modifée
  \param val la nouvelle valeur de la data
*/
void CTrame_ASSERV_DIAG_WRITE_PARAM::ASSERV_DIAG_WRITE_VALUE_changed(QVariant val)
{
  ASSERV_DIAG_WRITE_VALUE = val.toInt();
  if (m_synchro_tx == 0) { Encode(); }
}
//___________________________________________________________________________
/*!
  \brief Fonction appelée lorsque la data est modifée
  \param val la nouvelle valeur de la data
*/
void CTrame_ASSERV_DIAG_WRITE_PARAM::ASSERV_DIAG_WRITE_PARAM_changed(QVariant val)
{
  ASSERV_DIAG_WRITE_PARAM = val.toInt();
  if (m_synchro_tx == 0) { Encode(); }
}
//___________________________________________________________________________
/*!
  \brief Fonction appelée lorsque la data est modifée
  \param val la nouvelle valeur de la data
*/
void CTrame_ASSERV_DIAG_WRITE_PARAM::Synchro_changed(QVariant val)
{
  m_synchro_tx = val.toBool();
  if (m_synchro_tx == 0) { Encode(); }
}

//___________________________________________________________________________
/*!
  \brief Encode et envoie la trame
*/
void CTrame_ASSERV_DIAG_WRITE_PARAM::Encode(void)
{
  tStructTrameBrute trame;

  // Informations générales
  trame.ID = ID_ASSERV_DIAG_WRITE_PARAM;
  trame.DLC = DLC_ASSERV_DIAG_WRITE_PARAM;

 for (unsigned int i=0; i<m_dlc; i++) {
     trame.Data[i] = 0;
 }
  // Encode chacun des signaux de la trame
    trame.Data[3] |= (unsigned char)( ( (ASSERV_DIAG_WRITE_VALUE) & 0xFF) );
    trame.Data[2] |= (unsigned char)( ( (ASSERV_DIAG_WRITE_VALUE >> 8) & 0xFF) );

    trame.Data[1] |= (unsigned char)( ( (ASSERV_DIAG_WRITE_PARAM) & 0xFF) );
    trame.Data[0] |= (unsigned char)( ( (ASSERV_DIAG_WRITE_PARAM >> 8) & 0xFF) );

  // Envoie la trame
  m_messagerie_bot->SerialiseTrame(&trame);

  // Comptabilise le nombre de trames émises
  m_nombre_emis++;
}

// ========================================================
//             TRAME ELECTROBOT_CDE_SERVOS
// ========================================================
CTrame_ELECTROBOT_CDE_SERVOS::CTrame_ELECTROBOT_CDE_SERVOS(CMessagerieBot *messagerie_bot, CDataManager *data_manager)
    : CTrameBot(messagerie_bot, data_manager)
{
 m_name = "ELECTROBOT_CDE_SERVOS";
 m_id = ID_ELECTROBOT_CDE_SERVOS;
 m_dlc = DLC_ELECTROBOT_CDE_SERVOS;
 m_liste_noms_signaux.append("PositionServoMoteur2");
 m_liste_noms_signaux.append("VitesseServoMoteur2");
 m_liste_noms_signaux.append("NumeroServoMoteur2");
 m_liste_noms_signaux.append("PositionServoMoteur1");
 m_liste_noms_signaux.append("VitesseServoMoteur1");
 m_liste_noms_signaux.append("NumeroServoMoteur1");

 // Initialise les données de la messagerie
 PositionServoMoteur2 = 0;
 VitesseServoMoteur2 = 0;
 NumeroServoMoteur2 = 0;
 PositionServoMoteur1 = 0;
 VitesseServoMoteur1 = 0;
 NumeroServoMoteur1 = 0;
 m_synchro_tx = 0;

 // S'assure que les données existent dans le DataManager
 data_manager->write("PositionServoMoteur2",  PositionServoMoteur2);
 data_manager->write("VitesseServoMoteur2",  VitesseServoMoteur2);
 data_manager->write("NumeroServoMoteur2",  NumeroServoMoteur2);
 data_manager->write("PositionServoMoteur1",  PositionServoMoteur1);
 data_manager->write("VitesseServoMoteur1",  VitesseServoMoteur1);
 data_manager->write("NumeroServoMoteur1",  NumeroServoMoteur1);
 data_manager->write("ELECTROBOT_CDE_SERVOS_TxSync",  m_synchro_tx);

 // Connexion avec le DataManager
 connect(data_manager->getData("PositionServoMoteur2"), SIGNAL(valueChanged(QVariant)), this, SLOT(PositionServoMoteur2_changed(QVariant)));
 connect(data_manager->getData("VitesseServoMoteur2"), SIGNAL(valueChanged(QVariant)), this, SLOT(VitesseServoMoteur2_changed(QVariant)));
 connect(data_manager->getData("NumeroServoMoteur2"), SIGNAL(valueChanged(QVariant)), this, SLOT(NumeroServoMoteur2_changed(QVariant)));
 connect(data_manager->getData("PositionServoMoteur1"), SIGNAL(valueChanged(QVariant)), this, SLOT(PositionServoMoteur1_changed(QVariant)));
 connect(data_manager->getData("VitesseServoMoteur1"), SIGNAL(valueChanged(QVariant)), this, SLOT(VitesseServoMoteur1_changed(QVariant)));
 connect(data_manager->getData("NumeroServoMoteur1"), SIGNAL(valueChanged(QVariant)), this, SLOT(NumeroServoMoteur1_changed(QVariant)));
 connect(data_manager->getData("ELECTROBOT_CDE_SERVOS_TxSync"), SIGNAL(valueChanged(QVariant)), this, SLOT(Synchro_changed(QVariant)));

}
//___________________________________________________________________________
/*!
  \brief Fonction appelée lorsque la data est modifée
  \param val la nouvelle valeur de la data
*/
void CTrame_ELECTROBOT_CDE_SERVOS::PositionServoMoteur2_changed(QVariant val)
{
  PositionServoMoteur2 = val.toInt();
  if (m_synchro_tx == 0) { Encode(); }
}
//___________________________________________________________________________
/*!
  \brief Fonction appelée lorsque la data est modifée
  \param val la nouvelle valeur de la data
*/
void CTrame_ELECTROBOT_CDE_SERVOS::VitesseServoMoteur2_changed(QVariant val)
{
  VitesseServoMoteur2 = val.toInt();
  if (m_synchro_tx == 0) { Encode(); }
}
//___________________________________________________________________________
/*!
  \brief Fonction appelée lorsque la data est modifée
  \param val la nouvelle valeur de la data
*/
void CTrame_ELECTROBOT_CDE_SERVOS::NumeroServoMoteur2_changed(QVariant val)
{
  NumeroServoMoteur2 = val.toInt();
  if (m_synchro_tx == 0) { Encode(); }
}
//___________________________________________________________________________
/*!
  \brief Fonction appelée lorsque la data est modifée
  \param val la nouvelle valeur de la data
*/
void CTrame_ELECTROBOT_CDE_SERVOS::PositionServoMoteur1_changed(QVariant val)
{
  PositionServoMoteur1 = val.toInt();
  if (m_synchro_tx == 0) { Encode(); }
}
//___________________________________________________________________________
/*!
  \brief Fonction appelée lorsque la data est modifée
  \param val la nouvelle valeur de la data
*/
void CTrame_ELECTROBOT_CDE_SERVOS::VitesseServoMoteur1_changed(QVariant val)
{
  VitesseServoMoteur1 = val.toInt();
  if (m_synchro_tx == 0) { Encode(); }
}
//___________________________________________________________________________
/*!
  \brief Fonction appelée lorsque la data est modifée
  \param val la nouvelle valeur de la data
*/
void CTrame_ELECTROBOT_CDE_SERVOS::NumeroServoMoteur1_changed(QVariant val)
{
  NumeroServoMoteur1 = val.toInt();
  if (m_synchro_tx == 0) { Encode(); }
}
//___________________________________________________________________________
/*!
  \brief Fonction appelée lorsque la data est modifée
  \param val la nouvelle valeur de la data
*/
void CTrame_ELECTROBOT_CDE_SERVOS::Synchro_changed(QVariant val)
{
  m_synchro_tx = val.toBool();
  if (m_synchro_tx == 0) { Encode(); }
}

//___________________________________________________________________________
/*!
  \brief Encode et envoie la trame
*/
void CTrame_ELECTROBOT_CDE_SERVOS::Encode(void)
{
  tStructTrameBrute trame;

  // Informations générales
  trame.ID = ID_ELECTROBOT_CDE_SERVOS;
  trame.DLC = DLC_ELECTROBOT_CDE_SERVOS;

 for (unsigned int i=0; i<m_dlc; i++) {
     trame.Data[i] = 0;
 }
  // Encode chacun des signaux de la trame
    trame.Data[7] |= (unsigned char)( ( (PositionServoMoteur2) & 0xFF) );
    trame.Data[6] |= (unsigned char)( ( (PositionServoMoteur2 >> 8) & 0xFF) );

    trame.Data[5] |= (unsigned char)( ( (VitesseServoMoteur2) & 0xFF) );

    trame.Data[4] |= (unsigned char)( ( (NumeroServoMoteur2) & 0xFF) );

    trame.Data[3] |= (unsigned char)( ( (PositionServoMoteur1) & 0xFF) );
    trame.Data[2] |= (unsigned char)( ( (PositionServoMoteur1 >> 8) & 0xFF) );

    trame.Data[1] |= (unsigned char)( ( (VitesseServoMoteur1) & 0xFF) );

    trame.Data[0] |= (unsigned char)( ( (NumeroServoMoteur1) & 0xFF) );

  // Envoie la trame
  m_messagerie_bot->SerialiseTrame(&trame);

  // Comptabilise le nombre de trames émises
  m_nombre_emis++;
}

// ========================================================
//             TRAME COMMANDE_MVT_MANUEL
// ========================================================
CTrame_COMMANDE_MVT_MANUEL::CTrame_COMMANDE_MVT_MANUEL(CMessagerieBot *messagerie_bot, CDataManager *data_manager)
    : CTrameBot(messagerie_bot, data_manager)
{
 m_name = "COMMANDE_MVT_MANUEL";
 m_id = ID_COMMANDE_MVT_MANUEL;
 m_dlc = DLC_COMMANDE_MVT_MANUEL;
 m_liste_noms_signaux.append("PuissanceMotD");
 m_liste_noms_signaux.append("PuissanceMotG");

 // Initialise les données de la messagerie
 PuissanceMotD = 0;
 PuissanceMotG = 0;
 m_synchro_tx = 0;

 // S'assure que les données existent dans le DataManager
 data_manager->write("PuissanceMotD",  PuissanceMotD);
 data_manager->write("PuissanceMotG",  PuissanceMotG);
 data_manager->write("COMMANDE_MVT_MANUEL_TxSync",  m_synchro_tx);

 // Connexion avec le DataManager
 connect(data_manager->getData("PuissanceMotD"), SIGNAL(valueChanged(QVariant)), this, SLOT(PuissanceMotD_changed(QVariant)));
 connect(data_manager->getData("PuissanceMotG"), SIGNAL(valueChanged(QVariant)), this, SLOT(PuissanceMotG_changed(QVariant)));
 connect(data_manager->getData("COMMANDE_MVT_MANUEL_TxSync"), SIGNAL(valueChanged(QVariant)), this, SLOT(Synchro_changed(QVariant)));

}
//___________________________________________________________________________
/*!
  \brief Fonction appelée lorsque la data est modifée
  \param val la nouvelle valeur de la data
*/
void CTrame_COMMANDE_MVT_MANUEL::PuissanceMotD_changed(QVariant val)
{
  PuissanceMotD = val.toInt();
  if (m_synchro_tx == 0) { Encode(); }
}
//___________________________________________________________________________
/*!
  \brief Fonction appelée lorsque la data est modifée
  \param val la nouvelle valeur de la data
*/
void CTrame_COMMANDE_MVT_MANUEL::PuissanceMotG_changed(QVariant val)
{
  PuissanceMotG = val.toInt();
  if (m_synchro_tx == 0) { Encode(); }
}
//___________________________________________________________________________
/*!
  \brief Fonction appelée lorsque la data est modifée
  \param val la nouvelle valeur de la data
*/
void CTrame_COMMANDE_MVT_MANUEL::Synchro_changed(QVariant val)
{
  m_synchro_tx = val.toBool();
  if (m_synchro_tx == 0) { Encode(); }
}

//___________________________________________________________________________
/*!
  \brief Encode et envoie la trame
*/
void CTrame_COMMANDE_MVT_MANUEL::Encode(void)
{
  tStructTrameBrute trame;

  // Informations générales
  trame.ID = ID_COMMANDE_MVT_MANUEL;
  trame.DLC = DLC_COMMANDE_MVT_MANUEL;

 for (unsigned int i=0; i<m_dlc; i++) {
     trame.Data[i] = 0;
 }
  // Encode chacun des signaux de la trame
    trame.Data[3] |= (unsigned char)( ( (PuissanceMotD) & 0xFF) );
    trame.Data[2] |= (unsigned char)( ( (PuissanceMotD >> 8) & 0xFF) );

    trame.Data[1] |= (unsigned char)( ( (PuissanceMotG) & 0xFF) );
    trame.Data[0] |= (unsigned char)( ( (PuissanceMotG >> 8) & 0xFF) );

  // Envoie la trame
  m_messagerie_bot->SerialiseTrame(&trame);

  // Comptabilise le nombre de trames émises
  m_nombre_emis++;
}

// ========================================================
//             TRAME ETAT_PID_ASSERVISSEMENT
// ========================================================
CTrame_ETAT_PID_ASSERVISSEMENT::CTrame_ETAT_PID_ASSERVISSEMENT(CMessagerieBot *messagerie_bot, CDataManager *data_manager)
    : CTrameBot(messagerie_bot, data_manager)
{
 m_name = "ETAT_PID_ASSERVISSEMENT";
 m_id = ID_ETAT_PID_ASSERVISSEMENT;
 m_dlc = DLC_ETAT_PID_ASSERVISSEMENT;
 m_liste_noms_signaux.append("consigne_vitesse_rotation_filt");
 m_liste_noms_signaux.append("vitesse_rotation_robot_filt");
 m_liste_noms_signaux.append("consigne_vitesse_avance_filt");
 m_liste_noms_signaux.append("vitesse_avance_robot_filt");

 // S'assure que les données existent dans le DataManager
 data_manager->write("consigne_vitesse_rotation_filt",  consigne_vitesse_rotation_filt);
 data_manager->write("vitesse_rotation_robot_filt",  vitesse_rotation_robot_filt);
 data_manager->write("consigne_vitesse_avance_filt",  consigne_vitesse_avance_filt);
 data_manager->write("vitesse_avance_robot_filt",  vitesse_avance_robot_filt);

}
//___________________________________________________________________________
/*!
  \brief Decode les signaux de la trame
  \param trameRecue la trame brute recue a decoder
*/
void CTrame_ETAT_PID_ASSERVISSEMENT::Decode(tStructTrameBrute *trameRecue)
{
   // Decode les signaux de la trame
   consigne_vitesse_rotation_filt = ( ( ((short)(trameRecue->Data[7])) & 0xFF) )  |  ( ( ((short)(trameRecue->Data[6])) & 0xFF) << 8 );

   vitesse_rotation_robot_filt = ( ( ((short)(trameRecue->Data[5])) & 0xFF) )  |  ( ( ((short)(trameRecue->Data[4])) & 0xFF) << 8 );

   consigne_vitesse_avance_filt = ( ( ((short)(trameRecue->Data[3])) & 0xFF) )  |  ( ( ((short)(trameRecue->Data[2])) & 0xFF) << 8 );

   vitesse_avance_robot_filt = ( ( ((short)(trameRecue->Data[1])) & 0xFF) )  |  ( ( ((short)(trameRecue->Data[0])) & 0xFF) << 8 );


   // Envoie les données au data manager
   m_data_manager->write("consigne_vitesse_rotation_filt", BRUTE2PHYS_consigne_vitesse_rotation_filt(consigne_vitesse_rotation_filt));
   m_data_manager->write("vitesse_rotation_robot_filt", BRUTE2PHYS_vitesse_rotation_robot_filt(vitesse_rotation_robot_filt));
   m_data_manager->write("consigne_vitesse_avance_filt", BRUTE2PHYS_consigne_vitesse_avance_filt(consigne_vitesse_avance_filt));
   m_data_manager->write("vitesse_avance_robot_filt", BRUTE2PHYS_vitesse_avance_robot_filt(vitesse_avance_robot_filt));
   // Comptabilise la reception de cette trame
   m_nombre_recue++;
}

// ========================================================
//             TRAME ETAT_ASSERVISSEMENT
// ========================================================
CTrame_ETAT_ASSERVISSEMENT::CTrame_ETAT_ASSERVISSEMENT(CMessagerieBot *messagerie_bot, CDataManager *data_manager)
    : CTrameBot(messagerie_bot, data_manager)
{
 m_name = "ETAT_ASSERVISSEMENT";
 m_id = ID_ETAT_ASSERVISSEMENT;
 m_dlc = DLC_ETAT_ASSERVISSEMENT;
 m_liste_noms_signaux.append("compteur_diag_blocage");
 m_liste_noms_signaux.append("ModeAsservissement");
 m_liste_noms_signaux.append("cde_moteur_D");
 m_liste_noms_signaux.append("cde_moteur_G");
 m_liste_noms_signaux.append("Convergence");

 // S'assure que les données existent dans le DataManager
 data_manager->write("compteur_diag_blocage",  compteur_diag_blocage);
 data_manager->write("ModeAsservissement",  ModeAsservissement);
 data_manager->write("cde_moteur_D",  cde_moteur_D);
 data_manager->write("cde_moteur_G",  cde_moteur_G);
 data_manager->write("Convergence",  Convergence);

}
//___________________________________________________________________________
/*!
  \brief Decode les signaux de la trame
  \param trameRecue la trame brute recue a decoder
*/
void CTrame_ETAT_ASSERVISSEMENT::Decode(tStructTrameBrute *trameRecue)
{
   // Decode les signaux de la trame
   compteur_diag_blocage = ( ( ((unsigned short)(trameRecue->Data[7])) & 0xFF) )  |  ( ( ((unsigned short)(trameRecue->Data[6])) & 0xFF) << 8 );

   ModeAsservissement = ( ( ((unsigned char)(trameRecue->Data[5])) & 0xFF) );

   cde_moteur_D = ( ( ((short)(trameRecue->Data[3])) & 0xFF) )  |  ( ( ((short)(trameRecue->Data[2])) & 0xFF) << 8 );

   cde_moteur_G = ( ( ((short)(trameRecue->Data[1])) & 0xFF) )  |  ( ( ((short)(trameRecue->Data[0])) & 0xFF) << 8 );

   Convergence = ( ( ((unsigned char)(trameRecue->Data[4])) & 0xFF) );


   // Envoie les données au data manager
   m_data_manager->write("compteur_diag_blocage", compteur_diag_blocage);
   m_data_manager->write("ModeAsservissement", ModeAsservissement);
   m_data_manager->write("cde_moteur_D", cde_moteur_D);
   m_data_manager->write("cde_moteur_G", cde_moteur_G);
   m_data_manager->write("Convergence", Convergence);
   // Comptabilise la reception de cette trame
   m_nombre_recue++;
}

// ========================================================
//             TRAME POSITION_CODEURS
// ========================================================
CTrame_POSITION_CODEURS::CTrame_POSITION_CODEURS(CMessagerieBot *messagerie_bot, CDataManager *data_manager)
    : CTrameBot(messagerie_bot, data_manager)
{
 m_name = "POSITION_CODEURS";
 m_id = ID_POSITION_CODEURS;
 m_dlc = DLC_POSITION_CODEURS;
 m_liste_noms_signaux.append("PosCodeurG");
 m_liste_noms_signaux.append("PosCodeurD");

 // S'assure que les données existent dans le DataManager
 data_manager->write("PosCodeurG",  PosCodeurG);
 data_manager->write("PosCodeurD",  PosCodeurD);

}
//___________________________________________________________________________
/*!
  \brief Decode les signaux de la trame
  \param trameRecue la trame brute recue a decoder
*/
void CTrame_POSITION_CODEURS::Decode(tStructTrameBrute *trameRecue)
{
   // Decode les signaux de la trame
   PosCodeurG = ( ( ((short)(trameRecue->Data[1])) & 0xFF) )  |  ( ( ((short)(trameRecue->Data[0])) & 0xFF) << 8 );

   PosCodeurD = ( ( ((short)(trameRecue->Data[3])) & 0xFF) )  |  ( ( ((short)(trameRecue->Data[2])) & 0xFF) << 8 );


   // Envoie les données au data manager
   m_data_manager->write("PosCodeurG", PosCodeurG);
   m_data_manager->write("PosCodeurD", PosCodeurD);
   // Comptabilise la reception de cette trame
   m_nombre_recue++;
}

// ========================================================
//             TRAME POSITION_ABSOLUE_XY_TETA
// ========================================================
CTrame_POSITION_ABSOLUE_XY_TETA::CTrame_POSITION_ABSOLUE_XY_TETA(CMessagerieBot *messagerie_bot, CDataManager *data_manager)
    : CTrameBot(messagerie_bot, data_manager)
{
 m_name = "POSITION_ABSOLUE_XY_TETA";
 m_id = ID_POSITION_ABSOLUE_XY_TETA;
 m_dlc = DLC_POSITION_ABSOLUE_XY_TETA;
 m_liste_noms_signaux.append("teta_pos");
 m_liste_noms_signaux.append("y_pos");
 m_liste_noms_signaux.append("x_pos");

 // S'assure que les données existent dans le DataManager
 data_manager->write("teta_pos",  teta_pos);
 data_manager->write("y_pos",  y_pos);
 data_manager->write("x_pos",  x_pos);

}
//___________________________________________________________________________
/*!
  \brief Decode les signaux de la trame
  \param trameRecue la trame brute recue a decoder
*/
void CTrame_POSITION_ABSOLUE_XY_TETA::Decode(tStructTrameBrute *trameRecue)
{
   // Decode les signaux de la trame
   teta_pos = ( ( ((short)(trameRecue->Data[5])) & 0xFF) )  |  ( ( ((short)(trameRecue->Data[4])) & 0xFF) << 8 );

   y_pos = ( ( ((short)(trameRecue->Data[3])) & 0xFF) )  |  ( ( ((short)(trameRecue->Data[2])) & 0xFF) << 8 );

   x_pos = ( ( ((short)(trameRecue->Data[1])) & 0xFF) )  |  ( ( ((short)(trameRecue->Data[0])) & 0xFF) << 8 );


   // Envoie les données au data manager
   m_data_manager->write("teta_pos", BRUTE2PHYS_teta_pos(teta_pos));
   m_data_manager->write("y_pos", BRUTE2PHYS_y_pos(y_pos));
   m_data_manager->write("x_pos", BRUTE2PHYS_x_pos(x_pos));
   // Comptabilise la reception de cette trame
   m_nombre_recue++;
}

// ========================================================
//             TRAME ELECTROBOT_ETAT_CODEURS_1_2
// ========================================================
CTrame_ELECTROBOT_ETAT_CODEURS_1_2::CTrame_ELECTROBOT_ETAT_CODEURS_1_2(CMessagerieBot *messagerie_bot, CDataManager *data_manager)
    : CTrameBot(messagerie_bot, data_manager)
{
 m_name = "ELECTROBOT_ETAT_CODEURS_1_2";
 m_id = ID_ELECTROBOT_ETAT_CODEURS_1_2;
 m_dlc = DLC_ELECTROBOT_ETAT_CODEURS_1_2;
 m_liste_noms_signaux.append("Codeur_2");
 m_liste_noms_signaux.append("Codeur_1");

 // S'assure que les données existent dans le DataManager
 data_manager->write("Codeur_2",  Codeur_2);
 data_manager->write("Codeur_1",  Codeur_1);

}
//___________________________________________________________________________
/*!
  \brief Decode les signaux de la trame
  \param trameRecue la trame brute recue a decoder
*/
void CTrame_ELECTROBOT_ETAT_CODEURS_1_2::Decode(tStructTrameBrute *trameRecue)
{
   // Decode les signaux de la trame
   Codeur_2 = ( ( ((long)(trameRecue->Data[7])) & 0xFF) )  |  ( ( ((long)(trameRecue->Data[6])) & 0xFF) << 8 )  |  ( ( ((long)(trameRecue->Data[5])) & 0xFF) << 16 )  |  ( ( ((long)(trameRecue->Data[4])) & 0xFF) << 24 );

   Codeur_1 = ( ( ((long)(trameRecue->Data[3])) & 0xFF) )  |  ( ( ((long)(trameRecue->Data[2])) & 0xFF) << 8 )  |  ( ( ((long)(trameRecue->Data[1])) & 0xFF) << 16 )  |  ( ( ((long)(trameRecue->Data[0])) & 0xFF) << 24 );


   // Envoie les données au data manager
   m_data_manager->write("Codeur_2", Codeur_2);
   m_data_manager->write("Codeur_1", Codeur_1);
   // Comptabilise la reception de cette trame
   m_nombre_recue++;
}

// ========================================================
//             TRAME ELECTROBOT_ETAT_TELEMETRES
// ========================================================
CTrame_ELECTROBOT_ETAT_TELEMETRES::CTrame_ELECTROBOT_ETAT_TELEMETRES(CMessagerieBot *messagerie_bot, CDataManager *data_manager)
    : CTrameBot(messagerie_bot, data_manager)
{
 m_name = "ELECTROBOT_ETAT_TELEMETRES";
 m_id = ID_ELECTROBOT_ETAT_TELEMETRES;
 m_dlc = DLC_ELECTROBOT_ETAT_TELEMETRES;
 m_liste_noms_signaux.append("Telemetre6");
 m_liste_noms_signaux.append("Telemetre5");
 m_liste_noms_signaux.append("Telemetre4");
 m_liste_noms_signaux.append("Telemetre3");
 m_liste_noms_signaux.append("Telemetre2");
 m_liste_noms_signaux.append("Telemetre1");

 // S'assure que les données existent dans le DataManager
 data_manager->write("Telemetre4",  Telemetre6);
 data_manager->write("Telemetre4",  Telemetre5);
 data_manager->write("Telemetre4",  Telemetre4);
 data_manager->write("Telemetre3",  Telemetre3);
 data_manager->write("Telemetre2",  Telemetre2);
 data_manager->write("Telemetre1",  Telemetre1);

}
//___________________________________________________________________________
/*!
  \brief Decode les signaux de la trame
  \param trameRecue la trame brute recue a decoder
*/
void CTrame_ELECTROBOT_ETAT_TELEMETRES::Decode(tStructTrameBrute *trameRecue)
{
   // Decode les signaux de la trame
   Telemetre6 = ( ( ((unsigned char)(trameRecue->Data[5])) & 0xFF) );

   Telemetre5 = ( ( ((unsigned char)(trameRecue->Data[4])) & 0xFF) );

   Telemetre4 = ( ( ((unsigned char)(trameRecue->Data[3])) & 0xFF) );

   Telemetre3 = ( ( ((unsigned char)(trameRecue->Data[2])) & 0xFF) );

   Telemetre2 = ( ( ((unsigned char)(trameRecue->Data[1])) & 0xFF) );

   Telemetre1 = ( ( ((unsigned char)(trameRecue->Data[0])) & 0xFF) );


   // Envoie les données au data manager
   m_data_manager->write("Telemetre6", Telemetre6);
   m_data_manager->write("Telemetre5", Telemetre5);
   m_data_manager->write("Telemetre4", Telemetre4);
   m_data_manager->write("Telemetre3", Telemetre3);
   m_data_manager->write("Telemetre2", Telemetre2);
   m_data_manager->write("Telemetre1", Telemetre1);
   // Comptabilise la reception de cette trame
   m_nombre_recue++;
}

// ========================================================
//             TRAME ELECTROBOT_ETAT_CODEURS_3_4
// ========================================================
CTrame_ELECTROBOT_ETAT_CODEURS_3_4::CTrame_ELECTROBOT_ETAT_CODEURS_3_4(CMessagerieBot *messagerie_bot, CDataManager *data_manager)
    : CTrameBot(messagerie_bot, data_manager)
{
 m_name = "ELECTROBOT_ETAT_CODEURS_3_4";
 m_id = ID_ELECTROBOT_ETAT_CODEURS_3_4;
 m_dlc = DLC_ELECTROBOT_ETAT_CODEURS_3_4;
 m_liste_noms_signaux.append("Codeur_4");
 m_liste_noms_signaux.append("Codeur_3");

 // S'assure que les données existent dans le DataManager
 data_manager->write("Codeur_4",  Codeur_4);
 data_manager->write("Codeur_3",  Codeur_3);

}
//___________________________________________________________________________
/*!
  \brief Decode les signaux de la trame
  \param trameRecue la trame brute recue a decoder
*/
void CTrame_ELECTROBOT_ETAT_CODEURS_3_4::Decode(tStructTrameBrute *trameRecue)
{
   // Decode les signaux de la trame
   Codeur_4 = ( ( ((long)(trameRecue->Data[7])) & 0xFF) )  |  ( ( ((long)(trameRecue->Data[6])) & 0xFF) << 8 )  |  ( ( ((long)(trameRecue->Data[5])) & 0xFF) << 16 )  |  ( ( ((long)(trameRecue->Data[4])) & 0xFF) << 24 );

   Codeur_3 = ( ( ((long)(trameRecue->Data[3])) & 0xFF) )  |  ( ( ((long)(trameRecue->Data[2])) & 0xFF) << 8 )  |  ( ( ((long)(trameRecue->Data[1])) & 0xFF) << 16 )  |  ( ( ((long)(trameRecue->Data[0])) & 0xFF) << 24 );


   // Envoie les données au data manager
   m_data_manager->write("Codeur_4", Codeur_4);
   m_data_manager->write("Codeur_3", Codeur_3);
   // Comptabilise la reception de cette trame
   m_nombre_recue++;
}

// ========================================================
//             TRAME ELECTROBOT_ETAT_CAPTEURS_2
// ========================================================
CTrame_ELECTROBOT_ETAT_CAPTEURS_2::CTrame_ELECTROBOT_ETAT_CAPTEURS_2(CMessagerieBot *messagerie_bot, CDataManager *data_manager)
    : CTrameBot(messagerie_bot, data_manager)
{
 m_name = "ELECTROBOT_ETAT_CAPTEURS_2";
 m_id = ID_ELECTROBOT_ETAT_CAPTEURS_2;
 m_dlc = DLC_ELECTROBOT_ETAT_CAPTEURS_2;
 m_liste_noms_signaux.append("Etor_PGED1_dsPIC2");
 m_liste_noms_signaux.append("Etor_PGED1_dsPIC1");
 m_liste_noms_signaux.append("Etor_PGEC1_dsPIC2");
 m_liste_noms_signaux.append("Etor_PGEC1_dsPIC1");
 m_liste_noms_signaux.append("Etor_Codeur4_B");
 m_liste_noms_signaux.append("Etor_Codeur4_A");
 m_liste_noms_signaux.append("Etor_Codeur3_B");
 m_liste_noms_signaux.append("Etor_Codeur3_A");
 m_liste_noms_signaux.append("Etor_CAN_TX");
 m_liste_noms_signaux.append("Etor_CAN_RX");
 m_liste_noms_signaux.append("Etor6");
 m_liste_noms_signaux.append("Etor5");
 m_liste_noms_signaux.append("Etor4");
 m_liste_noms_signaux.append("Etor3");
 m_liste_noms_signaux.append("Etor2");
 m_liste_noms_signaux.append("Etor1");
 m_liste_noms_signaux.append("Vbat");
 m_liste_noms_signaux.append("Eana13");
 m_liste_noms_signaux.append("Eana12");
 m_liste_noms_signaux.append("Eana11");
 m_liste_noms_signaux.append("Eana10");
 m_liste_noms_signaux.append("Eana9");

 // S'assure que les données existent dans le DataManager
 data_manager->write("Etor_PGED1_dsPIC2",  Etor_PGED1_dsPIC2);
 data_manager->write("Etor_PGED1_dsPIC1",  Etor_PGED1_dsPIC1);
 data_manager->write("Etor_PGEC1_dsPIC2",  Etor_PGEC1_dsPIC2);
 data_manager->write("Etor_PGEC1_dsPIC1",  Etor_PGEC1_dsPIC1);
 data_manager->write("Etor_Codeur4_B",  Etor_Codeur4_B);
 data_manager->write("Etor_Codeur4_A",  Etor_Codeur4_A);
 data_manager->write("Etor_Codeur3_B",  Etor_Codeur3_B);
 data_manager->write("Etor_Codeur3_A",  Etor_Codeur3_A);
 data_manager->write("Etor_CAN_TX",  Etor_CAN_TX);
 data_manager->write("Etor_CAN_RX",  Etor_CAN_RX);
 data_manager->write("Etor6",  Etor6);
 data_manager->write("Etor5",  Etor5);
 data_manager->write("Etor4",  Etor4);
 data_manager->write("Etor3",  Etor3);
 data_manager->write("Etor2",  Etor2);
 data_manager->write("Etor1",  Etor1);
 data_manager->write("Vbat",  Vbat);
 data_manager->write("Eana13",  Eana13);
 data_manager->write("Eana12",  Eana12);
 data_manager->write("Eana11",  Eana11);
 data_manager->write("Eana10",  Eana10);
 data_manager->write("Eana9",  Eana9);

}
//___________________________________________________________________________
/*!
  \brief Decode les signaux de la trame
  \param trameRecue la trame brute recue a decoder
*/
void CTrame_ELECTROBOT_ETAT_CAPTEURS_2::Decode(tStructTrameBrute *trameRecue)
{
   // Decode les signaux de la trame
   Etor_PGED1_dsPIC2 = ( ( ((unsigned char)(trameRecue->Data[7]) >> 7) & 0x1) );

   Etor_PGED1_dsPIC1 = ( ( ((unsigned char)(trameRecue->Data[7]) >> 6) & 0x1) );

   Etor_PGEC1_dsPIC2 = ( ( ((unsigned char)(trameRecue->Data[7]) >> 5) & 0x1) );

   Etor_PGEC1_dsPIC1 = ( ( ((unsigned char)(trameRecue->Data[7]) >> 4) & 0x1) );

   Etor_Codeur4_B = ( ( ((unsigned char)(trameRecue->Data[7]) >> 3) & 0x1) );

   Etor_Codeur4_A = ( ( ((unsigned char)(trameRecue->Data[7]) >> 2) & 0x1) );

   Etor_Codeur3_B = ( ( ((unsigned char)(trameRecue->Data[7]) >> 1) & 0x1) );

   Etor_Codeur3_A = ( ( ((unsigned char)(trameRecue->Data[7])) & 0x1) );

   Etor_CAN_TX = ( ( ((unsigned char)(trameRecue->Data[6]) >> 7) & 0x1) );

   Etor_CAN_RX = ( ( ((unsigned char)(trameRecue->Data[6]) >> 6) & 0x1) );

   Etor6 = ( ( ((unsigned char)(trameRecue->Data[6]) >> 5) & 0x1) );

   Etor5 = ( ( ((unsigned char)(trameRecue->Data[6]) >> 4) & 0x1) );

   Etor4 = ( ( ((unsigned char)(trameRecue->Data[6]) >> 3) & 0x1) );

   Etor3 = ( ( ((unsigned char)(trameRecue->Data[6]) >> 2) & 0x1) );

   Etor2 = ( ( ((unsigned char)(trameRecue->Data[6]) >> 1) & 0x1) );

   Etor1 = ( ( ((unsigned char)(trameRecue->Data[6])) & 0x1) );

   Vbat = ( ( ((unsigned char)(trameRecue->Data[5])) & 0xFF) );

   Eana13 = ( ( ((unsigned char)(trameRecue->Data[4])) & 0xFF) );

   Eana12 = ( ( ((unsigned char)(trameRecue->Data[3])) & 0xFF) );

   Eana11 = ( ( ((unsigned char)(trameRecue->Data[2])) & 0xFF) );

   Eana10 = ( ( ((unsigned char)(trameRecue->Data[1])) & 0xFF) );

   Eana9 = ( ( ((unsigned char)(trameRecue->Data[0])) & 0xFF) );


   // Envoie les données au data manager
   m_data_manager->write("Etor_PGED1_dsPIC2", Etor_PGED1_dsPIC2);
   m_data_manager->write("Etor_PGED1_dsPIC1", Etor_PGED1_dsPIC1);
   m_data_manager->write("Etor_PGEC1_dsPIC2", Etor_PGEC1_dsPIC2);
   m_data_manager->write("Etor_PGEC1_dsPIC1", Etor_PGEC1_dsPIC1);
   m_data_manager->write("Etor_Codeur4_B", Etor_Codeur4_B);
   m_data_manager->write("Etor_Codeur4_A", Etor_Codeur4_A);
   m_data_manager->write("Etor_Codeur3_B", Etor_Codeur3_B);
   m_data_manager->write("Etor_Codeur3_A", Etor_Codeur3_A);
   m_data_manager->write("Etor_CAN_TX", Etor_CAN_TX);
   m_data_manager->write("Etor_CAN_RX", Etor_CAN_RX);
   m_data_manager->write("Etor6", Etor6);
   m_data_manager->write("Etor5", Etor5);
   m_data_manager->write("Etor4", Etor4);
   m_data_manager->write("Etor3", Etor3);
   m_data_manager->write("Etor2", Etor2);
   m_data_manager->write("Etor1", Etor1);
   m_data_manager->write("Vbat", BRUTE2PHYS_Vbat(Vbat));
   m_data_manager->write("Eana13", BRUTE2PHYS_Eana13(Eana13));
   m_data_manager->write("Eana12", BRUTE2PHYS_Eana12(Eana12));
   m_data_manager->write("Eana11", BRUTE2PHYS_Eana11(Eana11));
   m_data_manager->write("Eana10", BRUTE2PHYS_Eana10(Eana10));
   m_data_manager->write("Eana9", BRUTE2PHYS_Eana9(Eana9));
   // Comptabilise la reception de cette trame
   m_nombre_recue++;
}

// ========================================================
//             TRAME ELECTROBOT_ETAT_CAPTEURS_1
// ========================================================
CTrame_ELECTROBOT_ETAT_CAPTEURS_1::CTrame_ELECTROBOT_ETAT_CAPTEURS_1(CMessagerieBot *messagerie_bot, CDataManager *data_manager)
    : CTrameBot(messagerie_bot, data_manager)
{
 m_name = "ELECTROBOT_ETAT_CAPTEURS_1";
 m_id = ID_ELECTROBOT_ETAT_CAPTEURS_1;
 m_dlc = DLC_ELECTROBOT_ETAT_CAPTEURS_1;
 m_liste_noms_signaux.append("Eana8");
 m_liste_noms_signaux.append("Eana7");
 m_liste_noms_signaux.append("Eana6");
 m_liste_noms_signaux.append("Eana5");
 m_liste_noms_signaux.append("Eana4");
 m_liste_noms_signaux.append("Eana3");
 m_liste_noms_signaux.append("Eana2");
 m_liste_noms_signaux.append("Eana1");

 // S'assure que les données existent dans le DataManager
 data_manager->write("Eana8",  Eana8);
 data_manager->write("Eana7",  Eana7);
 data_manager->write("Eana6",  Eana6);
 data_manager->write("Eana5",  Eana5);
 data_manager->write("Eana4",  Eana4);
 data_manager->write("Eana3",  Eana3);
 data_manager->write("Eana2",  Eana2);
 data_manager->write("Eana1",  Eana1);

}
//___________________________________________________________________________
/*!
  \brief Decode les signaux de la trame
  \param trameRecue la trame brute recue a decoder
*/
void CTrame_ELECTROBOT_ETAT_CAPTEURS_1::Decode(tStructTrameBrute *trameRecue)
{
   // Decode les signaux de la trame
   Eana8 = ( ( ((unsigned char)(trameRecue->Data[7])) & 0xFF) );

   Eana7 = ( ( ((unsigned char)(trameRecue->Data[6])) & 0xFF) );

   Eana6 = ( ( ((unsigned char)(trameRecue->Data[5])) & 0xFF) );

   Eana5 = ( ( ((unsigned char)(trameRecue->Data[4])) & 0xFF) );

   Eana4 = ( ( ((unsigned char)(trameRecue->Data[3])) & 0xFF) );

   Eana3 = ( ( ((unsigned char)(trameRecue->Data[2])) & 0xFF) );

   Eana2 = ( ( ((unsigned char)(trameRecue->Data[1])) & 0xFF) );

   Eana1 = ( ( ((unsigned char)(trameRecue->Data[0])) & 0xFF) );


   // Envoie les données au data manager
   m_data_manager->write("Eana8", BRUTE2PHYS_Eana8(Eana8));
   m_data_manager->write("Eana7", BRUTE2PHYS_Eana7(Eana7));
   m_data_manager->write("Eana6", BRUTE2PHYS_Eana6(Eana6));
   m_data_manager->write("Eana5", BRUTE2PHYS_Eana5(Eana5));
   m_data_manager->write("Eana4", BRUTE2PHYS_Eana4(Eana4));
   m_data_manager->write("Eana3", BRUTE2PHYS_Eana3(Eana3));
   m_data_manager->write("Eana2", BRUTE2PHYS_Eana2(Eana2));
   m_data_manager->write("Eana1", BRUTE2PHYS_Eana1(Eana1));
   // Comptabilise la reception de cette trame
   m_nombre_recue++;
}


// ========================================================
//             TRAME ECRAN_ETAT_MATCH
// ========================================================
CTrame_ECRAN_ETAT_MATCH::CTrame_ECRAN_ETAT_MATCH(CMessagerieBot *messagerie_bot, CDataManager *data_manager)
    : CTrameBot(messagerie_bot, data_manager)
{
 m_name = "ECRAN_ETAT_MATCH";
 m_id = ID_ECRAN_ETAT_MATCH;
 m_dlc = DLC_ECRAN_ETAT_MATCH;
 ModeFonctionnement=2;
 m_liste_noms_signaux.append("ObstacleDetecte");
 m_liste_noms_signaux.append("OrigineDetectionObstacle");
 m_liste_noms_signaux.append("DiagBlocage");
 m_liste_noms_signaux.append("ConvergenceAsserv");
 m_liste_noms_signaux.append("ModeFonctionnement");
 m_liste_noms_signaux.append("CouleurEquipe");
 m_liste_noms_signaux.append("TempsMatch");
 m_liste_noms_signaux.append("Score");

 // S'assure que les données existent dans le DataManager
 data_manager->write("ObstacleDetecte",  ObstacleDetecte);
 data_manager->write("OrigineDetectionObstacle",  ObstacleDetecte);
 data_manager->write("DiagBlocage",  DiagBlocage);
 data_manager->write("ConvergenceAsserv",  ConvergenceAsserv);
 data_manager->write("ModeFonctionnement",  ModeFonctionnement);
 data_manager->write("CouleurEquipe",  CouleurEquipe);
 data_manager->write("TempsMatch",  TempsMatch);
 data_manager->write("Score",  Score);

}
//___________________________________________________________________________
/*!
  \brief Decode les signaux de la trame
  \param trameRecue la trame brute recue a decoder
*/
void CTrame_ECRAN_ETAT_MATCH::Decode(tStructTrameBrute *trameRecue)
{
   // Decode les signaux de la trame
   NumStrategie = ( ( ((unsigned char)(trameRecue->Data[5])) & 0xFF) );

   Score=( ( ((unsigned short)(trameRecue->Data[4])) & 0xFF) )  |  ( ( ((unsigned short)(trameRecue->Data[3])) & 0xFF) << 8 );

   OrigineDetectionObstacle = ( ( ((unsigned char)(trameRecue->Data[2])) >> 7) & 0x1 );

   ObstacleDetecte = ( ( ((unsigned char)(trameRecue->Data[2])) >> 6) & 0x1 );

   DiagBlocage = ( ( ((unsigned char)(trameRecue->Data[2])) >> 5) & 0x1 );

   ConvergenceAsserv = ( ( ((unsigned char)(trameRecue->Data[2])) >> 4) & 0x1 );

   ModeFonctionnement = ( ( ((unsigned char)(trameRecue->Data[2])) & 0xF) );

   CouleurEquipe=( ( ((unsigned char)(trameRecue->Data[1])) & 0xFF) );

   TempsMatch = ( ( ((unsigned char)(trameRecue->Data[0])) & 0xFF) );

   // Envoie les données au data manager
   m_data_manager->write("OrigineDetectionObstacle", BRUTE2PHYS_OrigineDetectionObstacle(OrigineDetectionObstacle));
   m_data_manager->write("ObstacleDetecte", BRUTE2PHYS_ObstacleDetecte(ObstacleDetecte));
   m_data_manager->write("DiagBlocage", BRUTE2PHYS_DiagBlocage(DiagBlocage));
   m_data_manager->write("ConvergenceAsserv", BRUTE2PHYS_ConvergenceAsserv(ConvergenceAsserv));
   m_data_manager->write("ModeFonctionnement", BRUTE2PHYS_ModeFonctionnement(ModeFonctionnement));
   m_data_manager->write("CouleurEquipe", CouleurEquipe);
   m_data_manager->write("TempsMatch", BRUTE2PHYS_TempsMatch(TempsMatch));
   m_data_manager->write("Score", Score);
   m_data_manager->write("NumStrategie", NumStrategie);
   // Comptabilise la reception de cette trame
   m_nombre_recue++;
}

// ========================================================
//             TRAME ETAT_DETECTION_EVITEMENT_OBSTACLE
// ========================================================
CTrame_ETAT_DETECTION_EVITEMENT_OBSTACLE::CTrame_ETAT_DETECTION_EVITEMENT_OBSTACLE(CMessagerieBot *messagerie_bot, CDataManager *data_manager)
    : CTrameBot(messagerie_bot, data_manager)
{
 m_name = "ETAT_DETECTION_EVITEMENT_OBSTACLE";
 m_id = ID_ETAT_DETECTION_EVITEMENT_OBSTACLE;
 m_dlc = DLC_ETAT_DETECTION_EVITEMENT_OBSTACLE;

 m_liste_noms_signaux.append(QString("%1.x_robot").arg(prefix_detection));
 m_liste_noms_signaux.append(QString("%1.y_robot").arg(prefix_detection));
 m_liste_noms_signaux.append(QString("%1.teta_robot").arg(prefix_detection));
 m_liste_noms_signaux.append(QString("%1.SensDeplacement").arg(prefix_detection));
 m_liste_noms_signaux.append(QString("%1.ObstacleBitfield").arg(prefix_detection));
 m_liste_noms_signaux.append(QString("%1.NombreObstaclesPresents").arg(prefix_detection));
 m_liste_noms_signaux.append(QString("%1.AnglePremierObstacleDetecte").arg(prefix_detection));
 m_liste_noms_signaux.append(QString("%1.DistancePremierObstacleDetecte").arg(prefix_detection));
 m_liste_noms_signaux.append(QString("%1.ObstacleDetecte").arg(prefix_detection));
 m_liste_noms_signaux.append(QString("%1.ObstacleAVD").arg(prefix_detection));
 m_liste_noms_signaux.append(QString("%1.ObstacleAVG").arg(prefix_detection));
 m_liste_noms_signaux.append(QString("%1.ObstacleARD").arg(prefix_detection));
 m_liste_noms_signaux.append(QString("%1.ObstacleARG").arg(prefix_detection));
 m_liste_noms_signaux.append(QString("%1.ObstacleInhibe").arg(prefix_detection));
 m_liste_noms_signaux.append(QString("%1.ForcageDetectObstacleSansPosition").arg(prefix_detection));
 m_liste_noms_signaux.append(QString("%1.OrigineDetectionObstacle").arg(prefix_detection));
 m_liste_noms_signaux.append(QString("%1.SeuilDetectionObstacleLidar").arg(prefix_detection));
 m_liste_noms_signaux.append(QString("%1.SeuilDetectionObstacleTelemetre").arg(prefix_detection));

 m_liste_noms_signaux.append(QString("%1.NumeroEtape").arg(prefix_evitement));
 m_liste_noms_signaux.append(QString("%1.NombreTentatives").arg(prefix_evitement));
 m_liste_noms_signaux.append(QString("%1.ChoixStrategieEvitement").arg(prefix_evitement));
 m_liste_noms_signaux.append(QString("%1.EvitementEnCours").arg(prefix_evitement));


 // S'assure que les données existent dans le DataManager
 data_manager->write((QString("%1.x_robot").arg(prefix_detection)),  x_robot);
 data_manager->write((QString("%1.y_robot").arg(prefix_detection)),  y_robot);
 data_manager->write((QString("%1.teta_robot").arg(prefix_detection)),  teta_robot);
 data_manager->write((QString("%1.SensDeplacement").arg(prefix_detection)),  SensDeplacement);
 data_manager->write((QString("%1.ObstacleBitfield").arg(prefix_detection)),  ObstacleBitfield);
 data_manager->write((QString("%1.NombreObstaclesPresents").arg(prefix_detection)),  NombreObstaclesPresents);
 data_manager->write((QString("%1.AnglePremierObstacleDetecte").arg(prefix_detection)),  AnglePremierObstacleDetecte);
 data_manager->write((QString("%1.DistancePremierObstacleDetecte").arg(prefix_detection)),  DistancePremierObstacleDetecte);
 data_manager->write((QString("%1.ObstacleDetecte").arg(prefix_detection)),  ObstacleDetecte);
 data_manager->write((QString("%1.ObstacleAVD").arg(prefix_detection)),  ObstacleAVD);
 data_manager->write((QString("%1.ObstacleAVG").arg(prefix_detection)),  ObstacleAVG);
 data_manager->write((QString("%1.ObstacleARD").arg(prefix_detection)),  ObstacleARD);
 data_manager->write((QString("%1.ObstacleARG").arg(prefix_detection)),  ObstacleARG);
 data_manager->write((QString("%1.ObstacleInhibe").arg(prefix_detection)),  ObstacleInhibe);
 data_manager->write((QString("%1.ForcageDetectObstacleSansPosition").arg(prefix_detection)),  ForcageDetectObstacleSansPosition);
 data_manager->write((QString("%1.OrigineDetectionObstacle").arg(prefix_detection)),  ForcageDetectObstacleSansPosition);
 data_manager->write((QString("%1.SeuilDetectionObstacleLidar").arg(prefix_detection)),  SeuilDetectionObstacleLidar);
 data_manager->write((QString("%1.SeuilDetectionObstacleTelemetre").arg(prefix_detection)),  SeuilDetectionObstacleTelemetre);

 data_manager->write((QString("%1.NumeroEtape").arg(prefix_evitement)),  NumeroEtape);
 data_manager->write((QString("%1.NombreTentatives").arg(prefix_evitement)),  NombreTentatives);
 data_manager->write((QString("%1.ChoixStrategieEvitement").arg(prefix_evitement)),  ChoixStrategieEvitement);
 data_manager->write((QString("%1.EvitementEnCours").arg(prefix_evitement)),  EvitementEnCours);

}
//___________________________________________________________________________
/*!
  \brief Decode les signaux de la trame
  \param trameRecue la trame brute recue a decoder
*/
void CTrame_ETAT_DETECTION_EVITEMENT_OBSTACLE::Decode(tStructTrameBrute *trameRecue)
{
   // Decode les signaux de la trame

    // Encode chacun des signaux de la trame
    x_robot =                           CDataEncoderDecoder::decode_int16(trameRecue->Data, 0);
    y_robot =                           CDataEncoderDecoder::decode_int16(trameRecue->Data, 2);
    teta_robot =                        (float)CDataEncoderDecoder::decode_int16(trameRecue->Data, 4) / 100.;
    SensDeplacement =                   CDataEncoderDecoder::decode_int8(trameRecue->Data,  6);
    ObstacleBitfield =                  CDataEncoderDecoder::decode_uint8(trameRecue->Data, 7);
    NombreObstaclesPresents =           CDataEncoderDecoder::decode_uint8(trameRecue->Data, 8);
    AnglePremierObstacleDetecte =       (signed short)CDataEncoderDecoder::decode_int16(trameRecue->Data, 9);
    DistancePremierObstacleDetecte =    CDataEncoderDecoder::decode_uint16(trameRecue->Data, 11);
    ObstacleDetecte =                   CDataEncoderDecoder::decode_bit(trameRecue->Data,   13, 0);
    ObstacleAVD =                       CDataEncoderDecoder::decode_bit(trameRecue->Data,   13, 1);
    ObstacleAVG =                       CDataEncoderDecoder::decode_bit(trameRecue->Data,   13, 2);
    ObstacleARD =                       CDataEncoderDecoder::decode_bit(trameRecue->Data,   13, 3);
    ObstacleARG =                       CDataEncoderDecoder::decode_bit(trameRecue->Data,   13, 4);
    ObstacleInhibe =                    CDataEncoderDecoder::decode_bit(trameRecue->Data,   13, 5);
    ForcageDetectObstacleSansPosition = CDataEncoderDecoder::decode_bit(trameRecue->Data,   13, 6);
    OrigineDetectionObstacle =          CDataEncoderDecoder::decode_bit(trameRecue->Data,   13, 7);
    SeuilDetectionObstacleLidar =       CDataEncoderDecoder::decode_uint8(trameRecue->Data, 14);
    SeuilDetectionObstacleTelemetre =   CDataEncoderDecoder::decode_uint8(trameRecue->Data, 15);

    NumeroEtape =                       CDataEncoderDecoder::decode_uint8(trameRecue->Data, 16);
    NombreTentatives =                  CDataEncoderDecoder::decode_uint8(trameRecue->Data, 17);
    ChoixStrategieEvitement =           CDataEncoderDecoder::decode_uint8(trameRecue->Data, 18);
    EvitementEnCours =                  CDataEncoderDecoder::decode_bit(trameRecue->Data,   19, 0);

   // Envoie les données au data manager
   // S'assure que les données existent dans le DataManager
   m_data_manager->write((QString("%1.x_robot").arg(prefix_detection)),  x_robot);
   m_data_manager->write((QString("%1.y_robot").arg(prefix_detection)),  y_robot);
   m_data_manager->write((QString("%1.teta_robot").arg(prefix_detection)),  teta_robot);
   m_data_manager->write((QString("%1.SensDeplacement").arg(prefix_detection)),  SensDeplacement);
   m_data_manager->write((QString("%1.ObstacleBitfield").arg(prefix_detection)),  ObstacleBitfield);
   m_data_manager->write((QString("%1.NombreObstaclesPresents").arg(prefix_detection)),  NombreObstaclesPresents);
   m_data_manager->write((QString("%1.AnglePremierObstacleDetecte").arg(prefix_detection)),  AnglePremierObstacleDetecte);
   m_data_manager->write((QString("%1.DistancePremierObstacleDetecte").arg(prefix_detection)),  DistancePremierObstacleDetecte);
   m_data_manager->write((QString("%1.ObstacleDetecte").arg(prefix_detection)),  ObstacleDetecte);
   m_data_manager->write((QString("%1.ObstacleAVD").arg(prefix_detection)),  ObstacleAVD);
   m_data_manager->write((QString("%1.ObstacleAVG").arg(prefix_detection)),  ObstacleAVG);
   m_data_manager->write((QString("%1.ObstacleARD").arg(prefix_detection)),  ObstacleARD);
   m_data_manager->write((QString("%1.ObstacleARG").arg(prefix_detection)),  ObstacleARG);
   m_data_manager->write((QString("%1.ObstacleInhibe").arg(prefix_detection)),  ObstacleInhibe);
   m_data_manager->write((QString("%1.ForcageDetectObstacleSansPosition").arg(prefix_detection)),  ForcageDetectObstacleSansPosition);
   m_data_manager->write((QString("%1.SeuilDetectionObstacleLidar").arg(prefix_detection)),  SeuilDetectionObstacleLidar);
   m_data_manager->write((QString("%1.SeuilDetectionObstacleTelemetre").arg(prefix_detection)),  SeuilDetectionObstacleTelemetre);

   m_data_manager->write((QString("%1.NumeroEtape").arg(prefix_evitement)),  NumeroEtape);
   m_data_manager->write((QString("%1.NombreTentatives").arg(prefix_evitement)),  NombreTentatives);
   m_data_manager->write((QString("%1.ChoixStrategieEvitement").arg(prefix_evitement)),  ChoixStrategieEvitement);
   m_data_manager->write((QString("%1.EvitementEnCours").arg(prefix_evitement)),  EvitementEnCours);

   // Comptabilise la reception de cette trame
   m_nombre_recue++;
}

// ========================================================
//             TRAME ECRAN_ETAT_ECRAN
// ========================================================
CTrame_ECRAN_ETAT_ECRAN::CTrame_ECRAN_ETAT_ECRAN(CMessagerieBot *messagerie_bot, CDataManager *data_manager)
    : CTrameBot(messagerie_bot, data_manager)
{
 m_name = "ECRAN_ETAT_ECRAN";
 m_id = ID_ECRAN_ETAT_ECRAN;
 m_dlc = DLC_ECRAN_ETAT_ECRAN;
 m_liste_noms_signaux.append("valeur_etat_ecran");
 m_liste_noms_signaux.append("commande_etat_ecran");

 // Initialise les données de la messagerie
 Valeur = 0;
 CodeCommande = 0;

 // S'assure que les données existent dans le DataManager
 data_manager->write("valeur_etat_ecran",  Valeur);
 data_manager->write("commande_etat_ecran",  CodeCommande);
 data_manager->write("ECRAN_ETAT_ECRAN_TxSync",  m_synchro_tx);

 // Connexion avec le DataManager
 connect(data_manager->getData("valeur_etat_ecran"), SIGNAL(valueChanged(QVariant)), this, SLOT(Valeur_etat_ecran_changed(QVariant)));
 connect(data_manager->getData("commande_etat_ecran"), SIGNAL(valueChanged(QVariant)), this, SLOT(CodeCommande_etat_ecran_changed(QVariant)));
 connect(data_manager->getData("ECRAN_ETAT_ECRAN_TxSync"), SIGNAL(valueChanged(QVariant)), this, SLOT(Synchro_changed(QVariant)));

}
//___________________________________________________________________________
/*!
  \brief Fonction appelée lorsque la data est modifée
  \param val la nouvelle valeur de la data
*/
void CTrame_ECRAN_ETAT_ECRAN::Valeur_etat_ecran_changed(QVariant val)
{
  Valeur = val.toInt();
  if (m_synchro_tx == 0) { Encode(); }
}
//___________________________________________________________________________
/*!
  \brief Fonction appelée lorsque la data est modifée
  \param val la nouvelle valeur de la data
*/
void CTrame_ECRAN_ETAT_ECRAN::CodeCommande_etat_ecran_changed(QVariant val)
{
  CodeCommande = val.toInt();
  if (m_synchro_tx == 0) { Encode(); }
}
//___________________________________________________________________________
/*!
  \brief Fonction appelée lorsque la data est modifée
  \param val la nouvelle valeur de la data
*/
void CTrame_ECRAN_ETAT_ECRAN::Synchro_changed(QVariant val)
{
  m_synchro_tx = val.toBool();
  if (m_synchro_tx == 0) { Encode(); }
}

//___________________________________________________________________________
/*!
  \brief Encode et envoie la trame
*/
void CTrame_ECRAN_ETAT_ECRAN::Encode(void)
{
  tStructTrameBrute trame;

  // Informations générales
  trame.ID = ID_ECRAN_ETAT_ECRAN;
  trame.DLC = DLC_ECRAN_ETAT_ECRAN;

 for (unsigned int i=0; i<m_dlc; i++) {
     trame.Data[i] = 0;
 }
 // Encode chacun des signaux de la trame
   trame.Data[3] |= (unsigned char)( ( (Valeur) & 0xFF) );
   trame.Data[2] |= (unsigned char)( ( (Valeur >> 8) & 0xFF) );

   trame.Data[1] |= (unsigned char)( ( (CodeCommande) & 0xFF) );
   trame.Data[0] |= (unsigned char)( ( (CodeCommande >> 8) & 0xFF) );

  //qDebug() << CodeCommande << Valeur;
  // Envoie la trame
  m_messagerie_bot->SerialiseTrame(&trame);

  // Comptabilise le nombre de trames émises
  m_nombre_emis++;
}

// ========================================================
//             TRAME ETAT_RACK
// ========================================================
CTrame_ETAT_RACK::CTrame_ETAT_RACK(CMessagerieBot *messagerie_bot, CDataManager *data_manager)
    : CTrameBot(messagerie_bot, data_manager)
{
 m_name = "ETAT_RACK";
 m_id = ID_ETAT_RACK;
 m_dlc = DLC_ETAT_RACK;
 m_liste_noms_signaux.append("rack_reserve");
 m_liste_noms_signaux.append("rack_modeAsservissement");
 m_liste_noms_signaux.append("rack_cde_moteur");
 m_liste_noms_signaux.append("rack_consigne_moteur");
 m_liste_noms_signaux.append("rack_convergence");

 // S'assure que les données existent dans le DataManager
 data_manager->write("rack_reserve",  rack_reserve);
 data_manager->write("rack_modeAsservissement",  rack_modeAsservissement);
 data_manager->write("rack_cde_moteur",  rack_cde_moteur);
 data_manager->write("rack_consigne_moteur",  rack_consigne_moteur);
 data_manager->write("rack_convergence",  rack_convergence);

}
//___________________________________________________________________________
/*!
  \brief Decode les signaux de la trame
  \param trameRecue la trame brute recue a decoder
*/
void CTrame_ETAT_RACK::Decode(tStructTrameBrute *trameRecue)
{
   // Decode les signaux de la trame
   rack_reserve = ( ( ((short)(trameRecue->Data[7])) & 0xFF) )  |  ( ( ((short)(trameRecue->Data[6])) & 0xFF) << 8 );

   rack_modeAsservissement = ( ( ((unsigned char)(trameRecue->Data[5])) & 0xFF) );

   rack_cde_moteur = ( ( ((short)(trameRecue->Data[3])) & 0xFF) )  |  ( ( ((short)(trameRecue->Data[2])) & 0xFF) << 8 );

   rack_consigne_moteur = ( ( ((short)(trameRecue->Data[1])) & 0xFF) )  |  ( ( ((short)(trameRecue->Data[0])) & 0xFF) << 8 );

   rack_convergence = ( ( ((unsigned char)(trameRecue->Data[4])) & 0xFF) );


   // Envoie les données au data manager
   m_data_manager->write("rack_reserve", rack_reserve);
   m_data_manager->write("rack_modeAsservissement", rack_modeAsservissement);
   m_data_manager->write("rack_cde_moteur", rack_cde_moteur);
   m_data_manager->write("rack_consigne_moteur", rack_consigne_moteur);
   m_data_manager->write("rack_convergence", rack_convergence);
   // Comptabilise la reception de cette trame
   m_nombre_recue++;
}

// ========================================================
//             TRAME COLOR SENSOR
// ========================================================
CTrame_COLOR_SENSOR::CTrame_COLOR_SENSOR(CMessagerieBot *messagerie_bot, CDataManager *data_manager)
    : CTrameBot(messagerie_bot, data_manager)
{
 m_name = "COLOR_SENSOR";
 m_id = ID_ELECTROBOT_COLOR_SENSOR;
 m_dlc = DLC_ELECTROBOT_COLOR_SENSOR;
 m_liste_noms_signaux.append("color_sensor_R");
 m_liste_noms_signaux.append("color_sensor_G");
 m_liste_noms_signaux.append("color_sensor_B");


 // S'assure que les données existent dans le DataManager
 data_manager->write("color_sensor_R", R);
 data_manager->write("color_sensor_G", G);
 data_manager->write("color_sensor_B", B);

}
//___________________________________________________________________________
/*!
  \brief Decode les signaux de la trame
  \param trameRecue la trame brute recue a decoder
*/
void CTrame_COLOR_SENSOR::Decode(tStructTrameBrute *trameRecue)
{
   // Decode les signaux de la trame
    R = ( ( ((short)(trameRecue->Data[1])) & 0xFF) )  |  ( ( ((short)(trameRecue->Data[0])) & 0xFF) << 8 );
    G = ( ( ((short)(trameRecue->Data[3])) & 0xFF) )  |  ( ( ((short)(trameRecue->Data[2])) & 0xFF) << 8 );
    B = ( ( ((short)(trameRecue->Data[5])) & 0xFF) )  |  ( ( ((short)(trameRecue->Data[4])) & 0xFF) << 8 );

   // Envoie les données au data manager
   m_data_manager->write("color_sensor_R", R);
   m_data_manager->write("color_sensor_G", G);
   m_data_manager->write("color_sensor_B", B);

   // Comptabilise la reception de cette trame
   m_nombre_recue++;
}
// ========================================================
//             TRAME CONFIG_PERIODE_TRAME
// ========================================================
CTrame_CONFIG_PERIODE_TRAME::CTrame_CONFIG_PERIODE_TRAME(CMessagerieBot *messagerie_bot, CDataManager *data_manager)
    : CTrameBot(messagerie_bot, data_manager)
{
 m_name = "CONFIG_PERIODE_TRAME";
 m_id = ID_CONFIG_PERIODE_TRAME;
 m_dlc = DLC_CONFIG_PERIODE_TRAME;
 m_liste_noms_signaux.append("CONFIG_PERIODE_TRAME_Periode");
 m_liste_noms_signaux.append("CONFIG_PERIODE_TRAME_ID");

 // Initialise les données de la messagerie
 CONFIG_PERIODE_TRAME_Periode = 0;
 CONFIG_PERIODE_TRAME_ID = 0;
 m_synchro_tx = 0;

 // S'assure que les données existent dans le DataManager
 data_manager->write("CONFIG_PERIODE_TRAME_Periode",  CONFIG_PERIODE_TRAME_Periode);
 data_manager->write("CONFIG_PERIODE_TRAME_ID",  CONFIG_PERIODE_TRAME_ID);
 data_manager->write("CONFIG_PERIODE_TRAME_TxSync",  m_synchro_tx);

 // Connexion avec le DataManager
 connect(data_manager->getData("CONFIG_PERIODE_TRAME_Periode"), SIGNAL(valueChanged(QVariant)), this, SLOT(CONFIG_PERIODE_TRAME_Periode_changed(QVariant)));
 connect(data_manager->getData("CONFIG_PERIODE_TRAME_ID"), SIGNAL(valueChanged(QVariant)), this, SLOT(CONFIG_PERIODE_TRAME_ID_changed(QVariant)));
 connect(data_manager->getData("CONFIG_PERIODE_TRAME_TxSync"), SIGNAL(valueChanged(QVariant)), this, SLOT(Synchro_changed(QVariant)));

}
//___________________________________________________________________________
/*!
  \brief Fonction appelée lorsque la data est modifée
  \param val la nouvelle valeur de la data
*/
void CTrame_CONFIG_PERIODE_TRAME::CONFIG_PERIODE_TRAME_Periode_changed(QVariant val)
{
  CONFIG_PERIODE_TRAME_Periode = val.toInt();
  if (m_synchro_tx == 0) { Encode(); }
}
//___________________________________________________________________________
/*!
  \brief Fonction appelée lorsque la data est modifée
  \param val la nouvelle valeur de la data
*/
void CTrame_CONFIG_PERIODE_TRAME::CONFIG_PERIODE_TRAME_ID_changed(QVariant val)
{
  CONFIG_PERIODE_TRAME_ID = val.toInt();
  if (m_synchro_tx == 0) { Encode(); }
}
//___________________________________________________________________________
/*!
  \brief Fonction appelée lorsque la data est modifée
  \param val la nouvelle valeur de la data
*/
void CTrame_CONFIG_PERIODE_TRAME::Synchro_changed(QVariant val)
{
  m_synchro_tx = val.toBool();
  if (m_synchro_tx == 0) { Encode(); }
}

//___________________________________________________________________________
/*!
  \brief Encode et envoie la trame
*/
void CTrame_CONFIG_PERIODE_TRAME::Encode(void)
{
  tStructTrameBrute trame;

  // Informations générales
  trame.ID = ID_CONFIG_PERIODE_TRAME;
  trame.DLC = DLC_CONFIG_PERIODE_TRAME;

 for (unsigned int i=0; i<m_dlc; i++) {
     trame.Data[i] = 0;
 }
  // Encode chacun des signaux de la trame
    trame.Data[3] |= (unsigned char)( ( (CONFIG_PERIODE_TRAME_Periode) & 0xFF) );
    trame.Data[2] |= (unsigned char)( ( (CONFIG_PERIODE_TRAME_Periode >> 8) & 0xFF) );

    trame.Data[1] |= (unsigned char)( ( (CONFIG_PERIODE_TRAME_ID) & 0xFF) );
    trame.Data[0] |= (unsigned char)( ( (CONFIG_PERIODE_TRAME_ID >> 8) & 0xFF) );

  // Envoie la trame
  m_messagerie_bot->SerialiseTrame(&trame);

  // Comptabilise le nombre de trames émises
  m_nombre_emis++;
}

// ========================================================
//             TRAME CPU_CMDE
// ========================================================
CTrame_CPU_CMDE::CTrame_CPU_CMDE(CMessagerieBot *messagerie_bot, CDataManager *data_manager)
    : CTrameBot(messagerie_bot, data_manager)
{
 m_name = "CPU_CMDE";
 m_id = ID_CPU_CMDE;
 m_dlc = DLC_CPU_CMDE;
 m_liste_noms_signaux.append("valeur_cpu_cmde_01");
 m_liste_noms_signaux.append("valeur_cpu_cmde_02");
 m_liste_noms_signaux.append("valeur_cpu_cmde_03");
 m_liste_noms_signaux.append("valeur_cpu_cmde_04");
 m_liste_noms_signaux.append("code_cpu_cmde");

 // Initialise les données de la messagerie
 Valeur_01 = 0;
 Valeur_02 = 0;
 Valeur_03 = 0;
 Valeur_04 = 0;
 CodeCommande = 0;

 // S'assure que les données existent dans le DataManager
 data_manager->write("valeur_cpu_cmde_01",  Valeur_01);
 data_manager->write("valeur_cpu_cmde_02",  Valeur_02);
 data_manager->write("valeur_cpu_cmde_03",  Valeur_03);
 data_manager->write("valeur_cpu_cmde_04",  Valeur_04);
 data_manager->write("code_cpu_cmde",  CodeCommande);
 data_manager->write("CPU_CMDE_TxSync",  m_synchro_tx);

 // Connexion avec le DataManager
 connect(data_manager->getData("valeur_cpu_cmde_01"), SIGNAL(valueChanged(QVariant)), this, SLOT(Valeur_cpu_cmde_01_changed(QVariant)));
 connect(data_manager->getData("valeur_cpu_cmde_02"), SIGNAL(valueChanged(QVariant)), this, SLOT(Valeur_cpu_cmde_02_changed(QVariant)));
 connect(data_manager->getData("valeur_cpu_cmde_03"), SIGNAL(valueChanged(QVariant)), this, SLOT(Valeur_cpu_cmde_03_changed(QVariant)));
 connect(data_manager->getData("valeur_cpu_cmde_04"), SIGNAL(valueChanged(QVariant)), this, SLOT(Valeur_cpu_cmde_04_changed(QVariant)));
 connect(data_manager->getData("code_cpu_cmde"), SIGNAL(valueChanged(QVariant)), this, SLOT(Code_cpu_cmde_changed(QVariant)));
 connect(data_manager->getData("CPU_CMDE_TxSync"), SIGNAL(valueChanged(QVariant)), this, SLOT(Synchro_changed(QVariant)));

}
//___________________________________________________________________________
/*!
  \brief Fonction appelée lorsque la data est modifée
  \param val la nouvelle valeur de la data
*/
void CTrame_CPU_CMDE::Valeur_cpu_cmde_01_changed(QVariant val)
{
  Valeur_01 = val.toInt();
  if (m_synchro_tx == 0) { Encode(); }
}
//___________________________________________________________________________
/*!
  \brief Fonction appelée lorsque la data est modifée
  \param val la nouvelle valeur de la data
*/
void CTrame_CPU_CMDE::Valeur_cpu_cmde_02_changed(QVariant val)
{
  Valeur_02 = val.toInt();
  if (m_synchro_tx == 0) { Encode(); }
}
//___________________________________________________________________________
/*!
  \brief Fonction appelée lorsque la data est modifée
  \param val la nouvelle valeur de la data
*/
void CTrame_CPU_CMDE::Valeur_cpu_cmde_03_changed(QVariant val)
{
  Valeur_03 = val.toInt();
  if (m_synchro_tx == 0) { Encode(); }
}
//___________________________________________________________________________
/*!
  \brief Fonction appelée lorsque la data est modifée
  \param val la nouvelle valeur de la data
*/
void CTrame_CPU_CMDE::Valeur_cpu_cmde_04_changed(QVariant val)
{
  Valeur_04 = val.toInt();
  if (m_synchro_tx == 0) { Encode(); }
}
//___________________________________________________________________________
/*!
  \brief Fonction appelée lorsque la data est modifée
  \param val la nouvelle valeur de la data
*/
void CTrame_CPU_CMDE::Code_cpu_cmde_changed(QVariant val)
{
  CodeCommande = val.toInt();
  if (m_synchro_tx == 0) { Encode(); }
}
//___________________________________________________________________________
/*!
  \brief Fonction appelée lorsque la data est modifée
  \param val la nouvelle valeur de la data
*/
void CTrame_CPU_CMDE::Synchro_changed(QVariant val)
{
  m_synchro_tx = val.toBool();
  if (m_synchro_tx == 0) { Encode(); }
}

//___________________________________________________________________________
/*!
  \brief Encode et envoie la trame
*/
void CTrame_CPU_CMDE::Encode(void)
{
  tStructTrameBrute trame;

  // Informations générales
  trame.ID = ID_CPU_CMDE;
  trame.DLC = DLC_CPU_CMDE;

 for (unsigned int i=0; i<m_dlc; i++) {
     trame.Data[i] = 0;
 }
 // Encode chacun des signaux de la trame
     trame.Data[7] |= (unsigned char)( ( (Valeur_04) & 0xFF) );
     trame.Data[6] |= (unsigned char)( ( (Valeur_03) & 0xFF) );

     trame.Data[5] |= (unsigned char)( ( (Valeur_02) & 0xFF) );
     trame.Data[4] |= (unsigned char)( ( (Valeur_02 >> 8) & 0xFF) );

   trame.Data[3] |= (unsigned char)( ( (Valeur_01) & 0xFF) );
   trame.Data[2] |= (unsigned char)( ( (Valeur_01 >> 8) & 0xFF) );

   trame.Data[1] |= (unsigned char)( ( (CodeCommande) & 0xFF) );
   trame.Data[0] |= (unsigned char)( ( (CodeCommande >> 8) & 0xFF) );

  //qDebug() << CodeCommande << Valeur;
  // Envoie la trame
  m_messagerie_bot->SerialiseTrame(&trame);

  // Comptabilise le nombre de trames émises
  m_nombre_emis++;
}

// ========================================================
//             TRAME CPU_ETAT
// ========================================================
CTrame_CPU_ETAT::CTrame_CPU_ETAT(CMessagerieBot *messagerie_bot, CDataManager *data_manager)
    : CTrameBot(messagerie_bot, data_manager)
{
 m_name = "CPU_ETAT";
 m_id = ID_CPU_ETAT;
 m_dlc = DLC_CPU_ETAT;
 m_liste_noms_signaux.append("Valeur_cpu_etat_01");
 m_liste_noms_signaux.append("Valeur_cpu_etat_02");
 m_liste_noms_signaux.append("Valeur_cpu_etat_03");
 m_liste_noms_signaux.append("Valeur_cpu_etat_04");
 m_liste_noms_signaux.append("Code_cpu_etat");

 // S'assure que les données existent dans le DataManager
 data_manager->write("Valeur_cpu_etat_01",  Valeur_cpu_etat_01);
 data_manager->write("Valeur_cpu_etat_02",  Valeur_cpu_etat_02);
 data_manager->write("Valeur_cpu_etat_03",  Valeur_cpu_etat_03);
 data_manager->write("Valeur_cpu_etat_04",  Valeur_cpu_etat_04);
 data_manager->write("Code_cpu_etat",  Cde_cpu_etat);

}
//___________________________________________________________________________
/*!
  \brief Decode les signaux de la trame
  \param trameRecue la trame brute recue a decoder
*/
void CTrame_CPU_ETAT::Decode(tStructTrameBrute *trameRecue)
{
   // Decode les signaux de la trame

    Valeur_cpu_etat_04 = ( ((short)(trameRecue->Data[7])) & 0xFF);
   Valeur_cpu_etat_03 = ( ((short)(trameRecue->Data[6])) & 0xFF) ;

   Valeur_cpu_etat_02 = ( ( ((short)(trameRecue->Data[5])) & 0xFF) )  |  ( ( ((short)(trameRecue->Data[4])) & 0xFF) << 8 );

   Valeur_cpu_etat_01 = ( ( ((short)(trameRecue->Data[3])) & 0xFF) )  |  ( ( ((short)(trameRecue->Data[2])) & 0xFF) << 8 );

   Cde_cpu_etat = ( ( ((unsigned short)(trameRecue->Data[1])) & 0xFF) )  |  ( ( ((unsigned short)(trameRecue->Data[0])) & 0xFF) << 8 );


   // Envoie les données au data manager
   m_data_manager->write("Valeur_cpu_etat_01", Valeur_cpu_etat_01);
   m_data_manager->write("Valeur_cpu_etat_02", Valeur_cpu_etat_02);
   m_data_manager->write("Valeur_cpu_etat_03", Valeur_cpu_etat_03);
   m_data_manager->write("Valeur_cpu_etat_04", Valeur_cpu_etat_04);
   m_data_manager->write("Code_cpu_etat", Cde_cpu_etat);
   // Comptabilise la reception de cette trame
   m_nombre_recue++;
}

// ========================================================
//             TRAME ETAT_SERVO_AX
// ========================================================
CTrame_ETAT_SERVO_AX::CTrame_ETAT_SERVO_AX(CMessagerieBot *messagerie_bot, CDataManager *data_manager)
    : CTrameBot(messagerie_bot, data_manager)
{
 m_name = "ETAT_SERVO_AX";
 m_id = ID_ETAT_SERVO_AX;
 m_dlc = DLC_ETAT_SERVO_AX;

 // Cas particulier pour les servos AX.
 // Les données dans le DataManager sont renseignées au fur et à mesure de l'arrivée des messages.
 // Car le nom des data contiennent le numéro du servo et Labotbox ne connais pas à priori les
 // ID des servos utilisés dans l'application embarquée.

}
//___________________________________________________________________________
/*!
  \brief Decode les signaux de la trame
  \param trameRecue la trame brute recue a decoder
*/
void CTrame_ETAT_SERVO_AX::Decode(tStructTrameBrute *trameRecue)
{
   // Decode les signaux de la trame
   num_servo_ax =       CDataEncoderDecoder::decode_int8(trameRecue->Data,  0);
   position =           CDataEncoderDecoder::decode_int16(trameRecue->Data, 1);
   temperature =        CDataEncoderDecoder::decode_int8(trameRecue->Data,  3);
   couple =             CDataEncoderDecoder::decode_int16(trameRecue->Data, 4);
   mouvement_en_cours = CDataEncoderDecoder::decode_int8(trameRecue->Data,  6);

   // Envoie les données au data manager
   m_data_manager->write(QString("ServoAX_%1.Position").arg(num_servo_ax), position);
   m_data_manager->write(QString("ServoAX_%1.Temperature").arg(num_servo_ax), temperature);
   m_data_manager->write(QString("ServoAX_%1.Torque").arg(num_servo_ax), couple);
   m_data_manager->write(QString("ServoAX_%1.Moving").arg(num_servo_ax), mouvement_en_cours);
   // Comptabilise la reception de cette trame
   m_nombre_recue++;
}

// ========================================================
//             TRAME COMMANDE_KMAR
// ========================================================
CTrame_COMMANDE_KMAR::CTrame_COMMANDE_KMAR(CMessagerieBot *messagerie_bot, CDataManager *data_manager)
    : CTrameBot(messagerie_bot, data_manager)
{
 m_name = "COMMANDE_KMAR";
 m_id = ID_COMMANDE_KMAR;
 m_dlc = DLC_COMMANDE_KMAR;
 m_liste_noms_signaux.append("value_cmd_kmar");
 m_liste_noms_signaux.append("cmd_kmar");
 m_liste_noms_signaux.append("num_kmar");

 // Initialise les données de la messagerie
 value_cmd_kmar = 0;
 cmd_kmar = 0;
 num_kmar = 0;
 m_synchro_tx = 0;

 // S'assure que les données existent dans le DataManager
 data_manager->write("value_cmd_kmar",  value_cmd_kmar);
 data_manager->write("cmd_kmar",  cmd_kmar);
 data_manager->write("num_kmar",  num_kmar);
 data_manager->write("COMMANDE_KMAR_TxSync",  m_synchro_tx);

 // Connexion avec le DataManager
 connect(data_manager->getData("value_cmd_kmar"), SIGNAL(valueChanged(QVariant)), this, SLOT(value_cmd_kmar_changed(QVariant)));
 connect(data_manager->getData("cmd_kmar"), SIGNAL(valueChanged(QVariant)), this, SLOT(cmd_kmar_changed(QVariant)));
 connect(data_manager->getData("num_kmar"), SIGNAL(valueChanged(QVariant)), this, SLOT(num_kmar_changed(QVariant)));
 connect(data_manager->getData("COMMANDE_KMAR_TxSync"), SIGNAL(valueChanged(QVariant)), this, SLOT(Synchro_changed(QVariant)));

}
//___________________________________________________________________________
/*!
  \brief Fonction appelée lorsque la data est modifée
  \param val la nouvelle valeur de la data
*/
void CTrame_COMMANDE_KMAR::value_cmd_kmar_changed(QVariant val)
{
  value_cmd_kmar = val.toInt();
  if (m_synchro_tx == 0) { Encode(); }
}
//___________________________________________________________________________
/*!
  \brief Fonction appelée lorsque la data est modifée
  \param val la nouvelle valeur de la data
*/
void CTrame_COMMANDE_KMAR::cmd_kmar_changed(QVariant val)
{
  cmd_kmar = val.toInt();
  if (m_synchro_tx == 0) { Encode(); }
}
//___________________________________________________________________________
/*!
  \brief Fonction appelée lorsque la data est modifée
  \param val la nouvelle valeur de la data
*/
void CTrame_COMMANDE_KMAR::num_kmar_changed(QVariant val)
{
  num_kmar = val.toInt();
  if (m_synchro_tx == 0) { Encode(); }
}
//___________________________________________________________________________
/*!
  \brief Fonction appelée lorsque la data est modifée
  \param val la nouvelle valeur de la data
*/
void CTrame_COMMANDE_KMAR::Synchro_changed(QVariant val)
{
  m_synchro_tx = val.toBool();
  if (m_synchro_tx == 0) { Encode(); }
}

//___________________________________________________________________________
/*!
  \brief Encode et envoie la trame
*/
void CTrame_COMMANDE_KMAR::Encode(void)
{
  tStructTrameBrute trame;

  // Informations générales
  trame.ID = ID_COMMANDE_KMAR;
  trame.DLC = DLC_COMMANDE_KMAR;

 for (unsigned int i=0; i<m_dlc; i++) {
     trame.Data[i] = 0;
 }
  // Encode chacun des signaux de la trame
 CDataEncoderDecoder::encode_int8(trame.Data,   0, num_kmar);
 CDataEncoderDecoder::encode_int16(trame.Data,  1, cmd_kmar);
 CDataEncoderDecoder::encode_int16(trame.Data,  3, value_cmd_kmar);

  // Envoie la trame
  m_messagerie_bot->SerialiseTrame(&trame);

  // Comptabilise le nombre de trames émises
  m_nombre_emis++;
}

// ========================================================
//             TRAME ETAT_KMAR_GENERAL
// ========================================================
CTrame_ETAT_KMAR_GENERAL::CTrame_ETAT_KMAR_GENERAL(CMessagerieBot *messagerie_bot, CDataManager *data_manager)
    : CTrameBot(messagerie_bot, data_manager)
{
 m_name = "ETAT_KMAR_GENERAL";
 m_id = ID_ETAT_KMAR_GENERAL;
 m_dlc = DLC_ETAT_KMAR_GENERAL;

 // Cas particulier pour les servos AX.
 // Les données dans le DataManager sont renseignées au fur et à mesure de l'arrivée des messages.
 // Car le nom des data contiennent le numéro du Kmar et Labotbox ne connais pas à priori le nombre
 // de Kmar utilisés sur le robot.

}
//___________________________________________________________________________
/*!
  \brief Decode les signaux de la trame
  \param trameRecue la trame brute recue a decoder
*/
void CTrame_ETAT_KMAR_GENERAL::Decode(tStructTrameBrute *trameRecue)
{
    // Encode chacun des signaux de la trame
    num_kmar =                  CDataEncoderDecoder::decode_int8(trameRecue->Data, 0);
    status =                    CDataEncoderDecoder::decode_int8(trameRecue->Data, 1);
    num_mouvement_en_cours =    CDataEncoderDecoder::decode_int8(trameRecue->Data, 2);
    moving =                    CDataEncoderDecoder::decode_bit(trameRecue->Data, 3, 0);
    axis1_moving =              CDataEncoderDecoder::decode_bit(trameRecue->Data, 3, 1);
    axis2_moving =              CDataEncoderDecoder::decode_bit(trameRecue->Data, 3, 2);
    axis3_moving =              CDataEncoderDecoder::decode_bit(trameRecue->Data, 3, 3);
    axis4_moving =              CDataEncoderDecoder::decode_bit(trameRecue->Data, 3, 4);
    object_catched =            CDataEncoderDecoder::decode_bit(trameRecue->Data, 3, 5);
    axis1_position =            CDataEncoderDecoder::decode_int16(trameRecue->Data,  4);
    axis2_position =            CDataEncoderDecoder::decode_int16(trameRecue->Data,  6);
    axis3_position =            CDataEncoderDecoder::decode_int16(trameRecue->Data,  8);
    axis4_position =            CDataEncoderDecoder::decode_int16(trameRecue->Data,  10);

   // Envoie les données au data manager
   m_data_manager->write(QString("Kmar%1.status").arg(num_kmar), status);
   m_data_manager->write(QString("Kmar%1.num_mouvement_en_cours").arg(num_kmar), num_mouvement_en_cours);
   m_data_manager->write(QString("Kmar%1.moving").arg(num_kmar), moving);
   m_data_manager->write(QString("Kmar%1.axis1.moving").arg(num_kmar), axis1_moving);
   m_data_manager->write(QString("Kmar%1.axis2.moving").arg(num_kmar), axis2_moving);
   m_data_manager->write(QString("Kmar%1.axis3.moving").arg(num_kmar), axis3_moving);
   m_data_manager->write(QString("Kmar%1.axis4.moving").arg(num_kmar), axis4_moving);
   m_data_manager->write(QString("Kmar%1.object_catched").arg(num_kmar), object_catched);
   m_data_manager->write(QString("Kmar%1.axis1.position").arg(num_kmar), axis1_position);
   m_data_manager->write(QString("Kmar%1.axis2.position").arg(num_kmar), axis2_position);
   m_data_manager->write(QString("Kmar%1.axis3.position").arg(num_kmar), axis3_position);
   m_data_manager->write(QString("Kmar%1.axis4.position").arg(num_kmar), axis4_position);

   // Comptabilise la reception de cette trame
   m_nombre_recue++;
}

// ========================================================
//             TRAME ETAT_LIDAR
// ========================================================
CTrame_ETAT_LIDAR::CTrame_ETAT_LIDAR(CMessagerieBot *messagerie_bot, CDataManager *data_manager)
    : CTrameBot(messagerie_bot, data_manager)
{
 m_name = "ETAT_LIDAR";
 m_id = ID_ETAT_LIDAR;
 m_dlc = DLC_ETAT_LIDAR;

 // Initialise les données de la messagerie
 m_status = LidarUtils::LIDAR_DISCONNECTED;
 m_synchro_tx = 0;

 // S'assure que les données existent dans le DataManager
  data_manager->write("ETAT_LIDAR_TxSync",  m_synchro_tx);

 // Connexion avec le DataManager
 connect(data_manager->getData("ETAT_LIDAR_TxSync"), SIGNAL(valueChanged(QVariant)), this, SLOT(Synchro_changed(QVariant)));
}
//___________________________________________________________________________
/*!
  \brief Fonction appelée lorsque la data est modifée
  \param val la nouvelle valeur de la data
*/
void CTrame_ETAT_LIDAR::Synchro_changed(QVariant val)
{
  m_synchro_tx = val.toBool();
  if (m_synchro_tx == 0) { Encode(); }
}

//___________________________________________________________________________
/*!
  \brief Encode et envoie la trame
*/
void CTrame_ETAT_LIDAR::Encode(void)
{
  tStructTrameBrute trame;

  // Informations générales
  trame.ID = ID_ETAT_LIDAR;
  trame.DLC = DLC_ETAT_LIDAR;

 for (unsigned int i=0; i<m_dlc; i++) {
     trame.Data[i] = 0;
 }

 CDataEncoderDecoder::encode_int8(trame.Data,  0, m_status);

 // 24 bits pour vehiculer angle (non signe [0;360deg] sur 12 bits) et distance (sur 12 bits)
 // 12 bits MSB : angle / 12 bits LSB : distance
 //   octet 1 |  octet2  |  octet3 |       octet4 |  octet5  |  octet6     ...
 //  xxxx xxxx xxxx  xxxx xxxx xxxx       xxxx xxxx xxxx  xxxx xxxx xxxx    ...
 // <     ANGLE1   ><   DISTANCE1  >     <     ANGLE2   ><   DISTANCE2  >   ...

 int _byte_num = 1;
 for (int i=0; i<LidarUtils::NBRE_MAX_OBSTACLES; i++) {
     unsigned short angle_0_360 = 180 + m_obstacles[i].angle;  // ramene l'angle dans la plage [0;360]
     unsigned long ulval_24bits = ( (angle_0_360 & 0xFFF) << 12) |
                                 (m_obstacles[i].distance & 0xFFF);

     CDataEncoderDecoder::encode_uint24(trame.Data,  _byte_num, ulval_24bits);
     _byte_num+=3;
 }

  // Envoie la trame
  m_messagerie_bot->SerialiseTrame(&trame);

  // Comptabilise le nombre de trames émises
  m_nombre_emis++;
}

// ========================================================
//             TRAME FREE_STRING
// ========================================================
CTrame_FREE_STRING::CTrame_FREE_STRING(CMessagerieBot *messagerie_bot, CDataManager *data_manager)
    : CTrameBot(messagerie_bot, data_manager)
{
 m_name = "FREE_STRING";
 m_id = ID_FREE_STRING;
 m_dlc = DLC_FREE_STRING;

 m_liste_noms_signaux.append("FREE_STRING");

 // S'assure que les données existent dans le DataManager
 data_manager->write("FREE_STRING",  "");
}
//___________________________________________________________________________
/*!
  \brief Decode les signaux de la trame
  \param trameRecue la trame brute recue a decoder
*/
void CTrame_FREE_STRING::Decode(tStructTrameBrute *trameRecue)
{
   // Decode les signaux de la trame
   for (int i=0; i<64; i++) {
        m_str[i] = trameRecue->Data[i];
   }

   // Envoie les données au data manager
   m_data_manager->write("FREE_STRING", m_str);
   // Comptabilise la reception de cette trame
   m_nombre_recue++;
}

// ========================================================
//             TRAME ETAT_CHARGE_CPU
// ========================================================
CTrame_ETAT_CHARGE_CPU::CTrame_ETAT_CHARGE_CPU(CMessagerieBot *messagerie_bot, CDataManager *data_manager)
    : CTrameBot(messagerie_bot, data_manager)
{
 m_name = "ETAT_CHARGE_CPU";
 m_id = ID_ETAT_CHARGE_CPU;
 m_dlc = DLC_ETAT_CHARGE_CPU;
 m_liste_noms_signaux.append("CPU.IndicateurSurcharge");

 // S'assure que les données existent dans le DataManager
 cpu_overload_counter = 99999;
 task_real_period_usec = 99999;
 data_manager->write("CPU.OverloadCounter",  cpu_overload_counter);
 data_manager->write("CPU.TaskRealPeriod_us",  task_real_period_usec);
}
//___________________________________________________________________________
/*!
  \brief Decode les signaux de la trame
  \param trameRecue la trame brute recue a decoder
*/
void CTrame_ETAT_CHARGE_CPU::Decode(tStructTrameBrute *trameRecue)
{
   // Decode les signaux de la trame
   cpu_overload_counter = CDataEncoderDecoder::decode_uint32(trameRecue->Data, 0);
   task_real_period_usec = CDataEncoderDecoder::decode_uint32(trameRecue->Data, 4);

   // Envoie les données au data manager
   m_data_manager->write("CPU.OverloadCounter", cpu_overload_counter);
   m_data_manager->write("CPU.TaskRealPeriod_us", task_real_period_usec);

   // Comptabilise la reception de cette trame
   m_nombre_recue++;
}

// ========================================================
//             TRAME RESET_CPU
// ========================================================
CTrame_RESET_CPU::CTrame_RESET_CPU(CMessagerieBot *messagerie_bot, CDataManager *data_manager)
    : CTrameBot(messagerie_bot, data_manager)
{
 m_name = "RESET_CPU";
 m_id = ID_RESET_CPU;
 m_dlc = DLC_RESET_CPU;
 m_liste_noms_signaux.append("SECURITE_RESET_CPU");

 // Initialise les données de la messagerie
 SECURITE_RESET_CPU = 0;
 m_synchro_tx = 0;

 // S'assure que les données existent dans le DataManager
 data_manager->write("SECURITE_RESET_CPU",  SECURITE_RESET_CPU);
 data_manager->write("RESET_CPU_TxSync",  m_synchro_tx);

 // Connexion avec le DataManager
 connect(data_manager->getData("SECURITE_RESET_CPU"), SIGNAL(valueChanged(QVariant)), this, SLOT(SECURITE_RESET_CPU_changed(QVariant)));
 connect(data_manager->getData("RESET_CPU_TxSync"), SIGNAL(valueChanged(QVariant)), this, SLOT(Synchro_changed(QVariant)));

}
//___________________________________________________________________________
/*!
  \brief Fonction appelée lorsque la data est modifée
  \param val la nouvelle valeur de la data
*/
void CTrame_RESET_CPU::SECURITE_RESET_CPU_changed(QVariant val)
{
  SECURITE_RESET_CPU = val.toInt();
  if (m_synchro_tx == 0) { Encode(); }
}
//___________________________________________________________________________
/*!
  \brief Fonction appelée lorsque la data est modifée
  \param val la nouvelle valeur de la data
*/
void CTrame_RESET_CPU::Synchro_changed(QVariant val)
{
  m_synchro_tx = val.toBool();
  if (m_synchro_tx == 0) { Encode(); }
}

//___________________________________________________________________________
/*!
  \brief Encode et envoie la trame
*/
void CTrame_RESET_CPU::Encode(void)
{
  tStructTrameBrute trame;

  // Informations générales
  trame.ID = ID_RESET_CPU;
  trame.DLC = DLC_RESET_CPU;

 for (unsigned int i=0; i<m_dlc; i++) {
     trame.Data[i] = 0;
 }
  // Encode chacun des signaux de la trame
    trame.Data[0] |= (unsigned char)( ( (SECURITE_RESET_CPU) & 0xFF) );

  // Envoie la trame
  m_messagerie_bot->SerialiseTrame(&trame);

  // Comptabilise le nombre de trames émises
  m_nombre_emis++;
}

// ========================================================
//             TRAME COMMANDE_MODE_FONCTIONNEMENT
// ========================================================
CTrame_COMMANDE_MODE_FONCTIONNEMENT_CPU::CTrame_COMMANDE_MODE_FONCTIONNEMENT_CPU(CMessagerieBot *messagerie_bot, CDataManager *data_manager)
    : CTrameBot(messagerie_bot, data_manager)
{
 m_name = "COMMANDE_MODE_FONCTIONNEMENT";
 m_id = ID_COMMANDE_MODE_FONCTIONNEMENT_CPU;
 m_dlc = DLC_COMMANDE_MODE_FONCTIONNEMENT_CPU;
 m_liste_noms_signaux.append("CommandeModeFonctionnementCPU");

 // Initialise les données de la messagerie
 CommandeModeFonctionnementCPU = 0;
 m_synchro_tx = 0;

 // S'assure que les données existent dans le DataManager
 data_manager->write("CommandeModeFonctionnementCPU",  CommandeModeFonctionnementCPU);
 data_manager->write("COMMANDE_MODE_FONCTIONNEMENT_CPU_TxSync",  m_synchro_tx);

 // Connexion avec le DataManager
 connect(data_manager->getData("CommandeModeFonctionnementCPU"), SIGNAL(valueChanged(QVariant)), this, SLOT(CommandeModeFonctionnementCPU_changed(QVariant)));
 connect(data_manager->getData("COMMANDE_MODE_FONCTIONNEMENT_CPU_TxSync"), SIGNAL(valueChanged(QVariant)), this, SLOT(Synchro_changed(QVariant)));

}
//___________________________________________________________________________
/*!
  \brief Fonction appelée lorsque la data est modifée
  \param val la nouvelle valeur de la data
*/
void CTrame_COMMANDE_MODE_FONCTIONNEMENT_CPU::CommandeModeFonctionnementCPU_changed(QVariant val)
{
  CommandeModeFonctionnementCPU = val.toInt();
  if (m_synchro_tx == 0) { Encode(); }
}
//___________________________________________________________________________
/*!
  \brief Fonction appelée lorsque la data est modifée
  \param val la nouvelle valeur de la data
*/
void CTrame_COMMANDE_MODE_FONCTIONNEMENT_CPU::Synchro_changed(QVariant val)
{
  m_synchro_tx = val.toBool();
  if (m_synchro_tx == 0) { Encode(); }
}

//___________________________________________________________________________
/*!
  \brief Encode et envoie la trame
*/
void CTrame_COMMANDE_MODE_FONCTIONNEMENT_CPU::Encode(void)
{
  tStructTrameBrute trame;

  // Informations générales
  trame.ID = ID_COMMANDE_MODE_FONCTIONNEMENT_CPU;
  trame.DLC = DLC_COMMANDE_MODE_FONCTIONNEMENT_CPU;

 for (unsigned int i=0; i<m_dlc; i++) {
     trame.Data[i] = 0;
 }
  // Encode chacun des signaux de la trame
    CDataEncoderDecoder::encode_uint16(trame.Data,  0, CommandeModeFonctionnementCPU);

  // Envoie la trame
  m_messagerie_bot->SerialiseTrame(&trame);

  // Comptabilise le nombre de trames émises
  m_nombre_emis++;
}


// ========================================================
//             TRAME READ_EEPROM_REQ
// ========================================================
CTrame_READ_EEPROM_REQ::CTrame_READ_EEPROM_REQ(CMessagerieBot *messagerie_bot, CDataManager *data_manager)
    : CTrameBot(messagerie_bot, data_manager)
{
 m_name = "READ_EEPROM_REQ";
 m_id = ID_READ_EEPROM_REQ;
 m_dlc = DLC_READ_EEPROM_REQ;

 // Initialise les données de la messagerie
 m_synchro_tx = 0;

 // S'assure que les données existent dans le DataManager
 data_manager->write("READ_EEPROM_REQ_TxSync",  m_synchro_tx);

 // Connexion avec le DataManager
 connect(data_manager->getData("READ_EEPROM_REQ_TxSync"), SIGNAL(valueChanged(QVariant)), this, SLOT(Synchro_changed(QVariant)));
}
//___________________________________________________________________________
/*!
  \brief Fonction appelée lorsque la data est modifée
  \param val la nouvelle valeur de la data
*/
void CTrame_READ_EEPROM_REQ::Synchro_changed(QVariant val)
{
  m_synchro_tx = val.toBool();
  if (m_synchro_tx == 0) { Encode(); }
}

//___________________________________________________________________________
/*!
  \brief Encode et envoie la trame
*/
void CTrame_READ_EEPROM_REQ::Encode(void)
{
  tStructTrameBrute trame;

  // Informations générales
  trame.ID = ID_READ_EEPROM_REQ;
  trame.DLC = DLC_READ_EEPROM_REQ;

 for (unsigned int i=0; i<m_dlc; i++) {
     trame.Data[i] = 0;
 }
  // Encode chacun des signaux de la trame
    CDataEncoderDecoder::encode_uint32(trame.Data,  0, start_address);
    CDataEncoderDecoder::encode_uint32(trame.Data,  4, count);

  // Envoie la trame
  m_messagerie_bot->SerialiseTrame(&trame);

  // Comptabilise le nombre de trames émises
  m_nombre_emis++;
}

// ========================================================
//             TRAME WRITE_EEPROM_REQ
// ========================================================
CTrame_WRITE_EEPROM_REQ::CTrame_WRITE_EEPROM_REQ(CMessagerieBot *messagerie_bot, CDataManager *data_manager)
    : CTrameBot(messagerie_bot, data_manager)
{
 m_name = "WRITE_EEPROM_REQ";
 m_id = ID_WRITE_EEPROM_REQ;
 m_dlc = DLC_WRITE_EEPROM_REQ;

 // Initialise les données de la messagerie
 m_synchro_tx = 0;

 // S'assure que les données existent dans le DataManager
 data_manager->write("WRITE_EEPROM_REQ_TxSync",  m_synchro_tx);

 // Connexion avec le DataManager
 connect(data_manager->getData("WRITE_EEPROM_REQ_TxSync"), SIGNAL(valueChanged(QVariant)), this, SLOT(Synchro_changed(QVariant)));
}
//___________________________________________________________________________
/*!
  \brief Fonction appelée lorsque la data est modifée
  \param val la nouvelle valeur de la data
*/
void CTrame_WRITE_EEPROM_REQ::Synchro_changed(QVariant val)
{
  m_synchro_tx = val.toBool();
  if (m_synchro_tx == 0) { Encode(); }
}

//___________________________________________________________________________
/*!
  \brief Encode et envoie la trame
*/
void CTrame_WRITE_EEPROM_REQ::Encode(void)
{
  tStructTrameBrute trame;

  // Informations générales
  trame.ID = ID_WRITE_EEPROM_REQ;
  trame.DLC = DLC_WRITE_EEPROM_REQ;

 for (unsigned int i=0; i<m_dlc; i++) {
     trame.Data[i] = 0;
 }
  // Encode chacun des signaux de la trame
    CDataEncoderDecoder::encode_uint32(trame.Data,  0, address);
    CDataEncoderDecoder::encode_uint32(trame.Data,  4, value);

  // Envoie la trame
  m_messagerie_bot->SerialiseTrame(&trame);

  // Comptabilise le nombre de trames émises
  m_nombre_emis++;
}

// ========================================================
//             TRAME EEPROM_VALUE
// ========================================================
CTrame_EEPROM_VALUE::CTrame_EEPROM_VALUE(CMessagerieBot *messagerie_bot, CDataManager *data_manager)
    : CTrameBot(messagerie_bot, data_manager)
{
    m_name = "EEPROM_VALUE";
    m_id = ID_EEPROM_VALUE;
    m_dlc = DLC_EEPROM_VALUE;
    m_liste_noms_signaux.append("EEPROM_CPU.Address");
    m_liste_noms_signaux.append("EEPROM_CPU.Value");

    // S'assure que les données existent dans le DataManager
    address = 0;
    value = 0;
    data_manager->write("EEPROM_CPU.Address",  (unsigned int)address);
    data_manager->write("EEPROM_CPU.Value",  (unsigned int)value);
}
//___________________________________________________________________________
/*!
  \brief Decode les signaux de la trame
  \param trameRecue la trame brute recue a decoder
*/
void CTrame_EEPROM_VALUE::Decode(tStructTrameBrute *trameRecue)
{
    // Decode les signaux de la trame
    address = CDataEncoderDecoder::decode_uint32(trameRecue->Data, 0);
    value = CDataEncoderDecoder::decode_uint32(trameRecue->Data, 4);

    // Envoie les données au data manager
    m_data_manager->write("EEPROM_CPU.Address", (unsigned int)address);
    m_data_manager->write("EEPROM_CPU.Value", (unsigned int)value);

    // Comptabilise la reception de cette trame
    m_nombre_recue++;

    emit receive_value(address, value);
}


// ========================================================
//             TRAME ACTION_ROBOT
// ========================================================
CTrame_ACTION_ROBOT::CTrame_ACTION_ROBOT(CMessagerieBot *messagerie_bot, CDataManager *data_manager)
    : CTrameBot(messagerie_bot, data_manager)
{
 m_name = "ACTION_ROBOT";
 m_id = ID_ACTION_ROBOT;
 m_dlc = DLC_ACTION_ROBOT;

 // Initialise les données de la messagerie
 m_synchro_tx = 0;

 // S'assure que les données existent dans le DataManager
 data_manager->write("ACTION_ROBOT_TxSync",  m_synchro_tx);

 // Connexion avec le DataManager
 connect(data_manager->getData("ACTION_ROBOT_TxSync"), SIGNAL(valueChanged(QVariant)), this, SLOT(Synchro_changed(QVariant)));
}
//___________________________________________________________________________
/*!
  \brief Fonction appelée lorsque la data est modifée
  \param val la nouvelle valeur de la data
*/
void CTrame_ACTION_ROBOT::Synchro_changed(QVariant val)
{
  m_synchro_tx = val.toBool();
  if (m_synchro_tx == 0) { Encode(); }
}

//___________________________________________________________________________
/*!
  \brief Encode et envoie la trame
*/
void CTrame_ACTION_ROBOT::Encode(void)
{
  tStructTrameBrute trame;

  // Informations générales
  trame.ID = ID_ACTION_ROBOT;
  trame.DLC = DLC_ACTION_ROBOT;

 for (unsigned int i=0; i<m_dlc; i++) {
     trame.Data[i] = 0;
 }
  // Encode chacun des signaux de la trame
    CDataEncoderDecoder::encode_uint32(trame.Data,  0, command);
    CDataEncoderDecoder::encode_uint32(trame.Data,  4, value);

  // Envoie la trame
  m_messagerie_bot->SerialiseTrame(&trame);

  // Comptabilise le nombre de trames émises
  m_nombre_emis++;
}

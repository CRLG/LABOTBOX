/*! \file CMessagerieBot.cpp
 * A brief file description CPP.
 * A more elaborated file description.
 */
#include <QDebug>
#include "CMessagerieBot.h"
#include "CLaBotBox.h"
#include "CPrintView.h"
#include "CMainWindow.h"
#include "CEEPROM.h"
#include "CDataManager.h"
#include "CRS232.h"
#include "CTrameFactory.h"

/*! \addtogroup Module_Test2
   * 
   *  @{
   */
	   
// _____________________________________________________________________
/*!
*  Constructeur
*
*/
CMessagerieBot::CMessagerieBot(const char *plugin_name)
    :CPluginModule(plugin_name, VERSION_MessagerieBot, AUTEUR_MessagerieBot, INFO_MessagerieBot),
     m_rs232(NULL),
    // m_trame_factory(NULL),
     m_etatReconst(cETAT_INIT),
     m_numero_data(0),
     m_transfert_avec_checksum(true),
     m_cpt_trames_recues_diag(0),
     m_connected_to_robot(false)
{
}


// _____________________________________________________________________
/*!
*  Destructeur
*
*/
CMessagerieBot::~CMessagerieBot()
{

}


// _____________________________________________________________________
/*!
*  Initialisation du module
*
*/
void CMessagerieBot::init(CLaBotBox *application)
{
  CModule::init(application);
  setGUI(&m_ihm); // indique � la classe de base l'IHM

  // Restore la taille de la fen�tre
  QVariant val;
  val = m_application->m_eeprom->read(getName(), "geometry", QRect(50, 50, 150, 150));
  m_ihm.setGeometry(val.toRect());
  // Restore le fait que la fen�tre est visible ou non
  val = m_application->m_eeprom->read(getName(), "visible", QVariant(true));
  if (val.toBool()) { m_ihm.show(); }
  else              { m_ihm.hide(); }
  // Restore le niveau d'affichage
  val = m_application->m_eeprom->read(getName(), "niveau_trace", QVariant(MSG_TOUS));
  setNiveauTrace(val.toUInt());

  // Pour les essais uniquement.
  // A supprimer et association � r�aliser au niveau LaBotBox pour que le module de Messagerie
  // soit r�utilisable si besoin
  setRS232(m_application->m_RS232_robot);

  //
  m_trame_factory = new CTrameFactory(this, m_application->m_data_center);
  connect(this, SIGNAL(frameReceived(tStructTrameBrute)), m_trame_factory, SLOT(Decode(tStructTrameBrute)));

  connect(this, SIGNAL(frameReceived(tStructTrameBrute)), this, SLOT(DecodeFrame(tStructTrameBrute)));
  connect(m_rs232, SIGNAL(readyBytes(QByteArray)), this, SLOT(Reconstitution(QByteArray)));


  // Diagnostic de perte de communication
  connect(&m_timer_diag_comm, SIGNAL(timeout()), this, SLOT(DiagPerteComm()));
}


// _____________________________________________________________________
/*!
*  Fermeture du module
*
*/
void CMessagerieBot::close(void)
{
  // M�morise en EEPROM l'�tat de la fen�tre
  m_application->m_eeprom->write(getName(), "geometry", QVariant(m_ihm.geometry()));
  m_application->m_eeprom->write(getName(), "visible", QVariant(m_ihm.isVisible()));
  m_application->m_eeprom->write(getName(), "niveau_trace", QVariant((unsigned int)getNiveauTrace()));
}



// =======================================================
//                  TRAMES EN RECEPTION
// =======================================================


//___________________________________________________________________________
 /*!
   \brief Point d'entree du module.

    - Fonction appelee lorsqu'une donnee arrive sur la liaison serie
    -   Implemente la machine d'etat de reconstitution d'une trame, de detection d'erreur, d'interception de messages specifiques

   \param newData la donnee nouvellement recue
   \return --
   */
void CMessagerieBot::Reconstitution(unsigned char newData)
{
   switch(m_etatReconst)
   {
    // ----------------------------------------- ETATS PRIMAIRE D'AIGUILLAGE DU TYPE D'INFO RECUES
    case  cETAT_INIT :
        // Initialise les champs d'une pr�c�dente r�ception
        Init_Reconstitution();

        // Le message est une trame
        if (newData == 'T') {
            m_etatReconst = cETAT_ID_MSB;
        }
        else {
            m_application->m_print_view->print_error(this, "R�ception d'entete de trame inconnu");
        }
    break;
    // ----------------------------------------- ETATS LIES A LA RECEPTION DE TRAMES
    case  cETAT_ID_MSB :
        m_trameCourante.ID = (newData << 8);
        m_etatReconst = cETAT_ID_LSB;
    break;
    // -----------------------------------------
    case cETAT_ID_LSB :
        m_trameCourante.ID += (newData&0xFF);
        m_etatReconst = cETAT_DLC;
    break;
    // -----------------------------------------
    case cETAT_DLC :
        m_trameCourante.DLC = newData;
        if (newData > C_NOMBRE_DATA_TRAMES_ROBOT) {
            m_etatReconst = cETAT_INIT;
            m_application->m_print_view->print_warning(this, "Reception d'une trame trop grande - trame rejetee");
        }
        else if (newData > 0) {
            m_etatReconst = cETAT_DATA_i;
        }
        else { // Aucune donn�e
            if (m_transfert_avec_checksum)  { m_etatReconst = cETAT_CHECKSUM; }
            else                            { emit frameReceived(m_trameCourante);  m_etatReconst = cETAT_INIT; }
        }
    break;
    // ----------------------------------------- Les DLC donn�es
    case cETAT_DATA_i :
        m_trameCourante.Data[m_numero_data] = newData;
        m_numero_data++;
        if (m_trameCourante.DLC > m_numero_data)
        {  /* ne rien faire : il reste des donn�es � recevoir */ }
        else {
            if (m_transfert_avec_checksum)  { m_etatReconst = cETAT_CHECKSUM; }
            else                            { emit frameReceived(m_trameCourante);  m_etatReconst = cETAT_INIT;}
        }
    break;
    // ----------------------------------------- Le CHECKSUM
    case cETAT_CHECKSUM :
        if (newData == getCheckSumTrame(&m_trameCourante)) {
            GestionConnexionRobot();
            emit frameReceived(m_trameCourante);
        }
        else {
            m_application->m_print_view->print_error(this, "Trame recue avec un mauvais checksum");
        }
        m_etatReconst = cETAT_INIT;
    break;
   }
}



// _____________________________________________________________________
/*!
*  Reconstitution d'une trame
*
*/
void CMessagerieBot::Reconstitution(QByteArray data)
{
  for (int i=0; i<data.length(); i++) {
    Reconstitution(data.at(i));
  }
}

//___________________________________________________________________________
 /*!
   \brief Calcul le checksum de la trame

    - Calcul le checksum de la trame courante et le compare au parametre d'entree de la fonction

   \param trameBrute la trame pour laquelle calculer le checksum
   \return 	le checksum sur 8 bits
   */
unsigned char CMessagerieBot::getCheckSumTrame(tStructTrameBrute *trameBrute)
{
 unsigned char checksum = 0;
 unsigned char i=0;

 checksum += ((trameBrute->ID)&0xFF)>>8;
 checksum += ((trameBrute->ID)&0xFF);
 checksum += trameBrute->DLC;
 for(i=0; i<trameBrute->DLC; i++) {
    checksum += trameBrute->Data[i];
 }
 return(checksum);
}



//___________________________________________________________________________
 /*!
   \brief Initialisation du module Reconstitution

    - Initialise tous les champs de la structure de donnee courante

   \param --
   \return --
   */
void CMessagerieBot::Init_Reconstitution(void)
{
  m_numero_data = 0;
  // Initialise les champs de la trame courante
  m_trameCourante.ID = 0xFFF;
  m_trameCourante.DLC = 0xFF;
  for (unsigned int i=0; i<C_NOMBRE_DATA_TRAMES_ROBOT; i++) {
      m_trameCourante.Data[i] = 0xFF;
  }
}



//___________________________________________________________________________
/*!
   \brief Point d'entree du module.

    - Fonction appelee lorsqu'une trame valide est recue
    - Implemente le necessaire pour transformer une trame brute en signaux de la structure dediee

   \param trameRecue la trame brute recue
   \return --
   */
void CMessagerieBot::DecodeFrame(tStructTrameBrute trameRecue)
{
  m_application->m_print_view->print_debug(this, TrameBruteToString(&trameRecue));
}



//___________________________________________________________________________
/*!
   \brief Diagnostic de perte de communication avec le robot.
    - Fonction appelee p�riodiquement
    - Si aucune trame valide n'a �t� re�ue dans l'interval de diagnostic, c'est que la communication est perdue
   \param trameRecue la trame brute recue
   \return --
*/
void CMessagerieBot::DiagPerteComm(void)
{
 if ( (m_connected_to_robot==true) && (m_cpt_trames_recues_diag==0) ) {
    m_connected_to_robot = false;
    m_application->m_print_view->print_info(this, "Perte de connexion avec le robot");
    emit disconnected();
    m_timer_diag_comm.stop();  // la connexion �tant perdue, pas la peine de poursuivre le diagnostic de perte de communication
 }
 m_cpt_trames_recues_diag = 0;
}

//___________________________________________________________________________
/*!
   \brief Diagnostic de perte de communication avec le robot.
    - Fonction appelee � chaque r�ception
   \param trameRecue la trame brute recue
   \return --
*/
void CMessagerieBot::GestionConnexionRobot(void)
{
  // Gestion de la connexion/deconnexion avec le robot
  m_cpt_trames_recues_diag++;
  if (m_connected_to_robot == false) {
      m_connected_to_robot = true;
      m_application->m_print_view->print_info(this, "Connextion etablie avec le robot");
      emit connected();
      m_timer_diag_comm.start(C_PERIODE_DIAG_PERTE_COM); // la connexion est �tablie, lance le timer de diagnostic
  }
}

// =======================================================
//                  GENERALITES
// =======================================================

//___________________________________________________________________________
/*!
   \brief Convertit une trame en une chaine affichable.
*/
QString CMessagerieBot::TrameBruteToString(tStructTrameBrute *trame)
{
  QString str;
  QString str2;

  str.sprintf("0x%x [%d]   ", trame->ID, trame->DLC);
  for (unsigned int i=0; i<trame->DLC; i++) {
      str2.sprintf("0x%x", trame->Data[i]);
      str = str + str2 + " ";
  }
  return(str);
}


//___________________________________________________________________________
/*!
   \brief Associe le module de messagerie � une RS232.
*/
void CMessagerieBot::setRS232(CRS232 *serial)
{
  m_rs232 = serial;
}



// =======================================================
//                  TRAMES EN EMISSION
// =======================================================


//___________________________________________________________________________
 /*!
   \brief Point d'entree du module pour la serialisation de la trame

    - Fonction appelee pour demander la transformation d'une trame en serie d'octet envoyes sur la liaison serie
    - Prend les donnees une par une et les envoie sur la liaison serie
   \param trameBrute la trame a envoyer avec ses octets bruts
   \return --
   */
void CMessagerieBot::SerialiseTrame(tStructTrameBrute *trameBrute)
{
  unsigned char i=0;
  QByteArray byteArray;

  // Met en forme la trame
  byteArray.append('T');
  byteArray.append(trameBrute->ID>>8);
  byteArray.append(trameBrute->ID);
  byteArray.append(trameBrute->DLC);

  for(i=0; i<trameBrute->DLC; i++) {
    byteArray.append(trameBrute->Data[i]);
  }
  byteArray.append(getCheckSumTrame(trameBrute));

  // Envoie la trame
  m_rs232->write(byteArray);
}

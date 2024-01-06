/*! \file CMessagerieBot.h
 * A brief file description.
 * A more elaborated file description.
 */
#ifndef _CPLUGIN_MODULE_MessagerieBot_H_
#define _CPLUGIN_MODULE_MessagerieBot_H_

#include <QMainWindow>
#include <QTimer>

#include "CPluginModule.h"
#include "ui_ihm_MessagerieBot.h"
#include "CTrameBot.h"

class CRS232;
class CTrameFactory;
class CData;

 class Cihm_MessagerieBot : public QMainWindow
{
    Q_OBJECT
public:
    Cihm_MessagerieBot(QWidget *parent = 0)  : QMainWindow(parent) { ui.setupUi(this); }
    ~Cihm_MessagerieBot() { }

    Ui::ihm_MessagerieBot ui;

    CApplication *m_application;
 };



 /*! \addtogroup MessagerieBot
   * 
   *  @{
   */

	   
/*! @brief class CMessagerieBot
 */	   
class CMessagerieBot : public CPluginModule
{
    Q_OBJECT
#define     VERSION_MessagerieBot   "1.0"
#define     AUTEUR_MessagerieBot    "Nico"
#define     INFO_MessagerieBot      "Messages echanges avec le robot"

public:
    CMessagerieBot(const char *plugin_name);
    ~CMessagerieBot();

    Cihm_MessagerieBot *getIHM(void) { return(&m_ihm); }

    virtual void init(CApplication *application);
    virtual void close(void);
    virtual bool hasGUI(void)           { return(true); }
    virtual QIcon getIcon(void)         { return(QIcon(":/icons/edit_add.png")); }  // Précise l'icône qui représente le module
    virtual QString getMenuName(void)   { return("PluginModule"); }                 // Précise le nom du menu de la fenêtre principale dans lequel le module apparaît

private:
    Cihm_MessagerieBot m_ihm;

private slots :
    void onRightClicGUI(QPoint pos);
    void onArreterToutesTrames();
    void onToutesTrames200ms();
    void onSendConfigPeriodeTrame();
    void onConfigSelectTrame(QString tramename);
    void onConfigSelectID();


// =======================================================
//                  GENERALITES
// =======================================================
public :
    //! Convertit une trame en une chaine affichable
    QString TrameBruteToString(tStructTrameBrute *trame);
    //! Associe le module de messagerie à une RS232
    void setRS232(CRS232 *serial);
    CTrameFactory *getTrameFactory();

private :
    CRS232          *m_rs232;
    CTrameFactory   *m_trame_factory;
    CData           *m_data_robot_connected;

// =======================================================
//                  TRAMES EN RECEPTION
// =======================================================

//! Enumere les differents etats de la machine d'etat de l'automate.
//! Cet enumere contient toutes les valeurs prises par la machine d'etat de reconstitution des donnees
typedef enum {
  cETAT_INIT = 0,
  cETAT_ERREUR,
  cETAT_ID_MSB,
  cETAT_ID_LSB,
  cETAT_DLC,
  cETAT_DATA_i,
  cETAT_CHECKSUM
} tETATS_RECONST;

//! Codes d'erreurs retournes par le module Reconstitution.
//! La liste des codes d'erreurs partages entre le module Reconstitution (producteur d'un message d'erreur ) et les autres modules consommateurs
typedef enum {
  cERREUR_ENTETE_INCONNU = 0,
  cERREUR_CHECKSUM,
  cERREUR_DLC_INCORRECT
}eERREUR_RECONSTITUTION;

#define C_TIMEOUT_PERTE_COM 1000 // [msec]

private slots :
    //! Reconstitue une trame à partir de données entrantes
    void Reconstitution(QByteArray data);
    //! Reconstitue une trame à partir de données entrantes
    void Reconstitution(unsigned char data);
    //! Diagnostic de perte de communication avec le robot
    void TimeoutPerteComm(void);

public slots :
    //! Recherche et lance le decodage de la trame arrivee
    void DecodeFrame(tStructTrameBrute trameRecue);
signals :
    //! Signal de réception de trame
    void frameReceived(tStructTrameBrute trame);
    //! Signal de diagnostic de communication avec le robot pour indiquer que la connexion avec le robot est établie/perdue
    void connected(bool state);

private :
    //! Initialisation des donnees liees a la reconstitution de la trame
    void Init_Reconstitution(void);
    //! Checksum de trame brut
    unsigned char getCheckSumTrame(tStructTrameBrute *trameBrute);
    //! Gestion de connexion robot
    void GestionConnexionRobot(void);

private :
    //! Etat de la machine d'etat de reconstitution
    unsigned char m_etatReconst;
    //! Numéro d'octet reçu
    unsigned int m_numero_data;
    //! Trame en cours de reconstruction
    tStructTrameBrute m_trameCourante;
    //! Indique si le transfert se fait avec ou sans checksum de fin
    bool m_transfert_avec_checksum;

    //! Timer de diagnostic de perte de communication
    QTimer  m_timer_diag_comm;
    //! Indique si la connexion est établie ou non
    bool m_connected_to_robot;

// =======================================================
//                  TRAMES EN EMISSION
// =======================================================
public :
    //! Sérialise une trame
    void SerialiseTrame(tStructTrameBrute *trameBrute);
};

#endif // _CPLUGIN_MODULE_MessagerieBot_H_

/*! @} */


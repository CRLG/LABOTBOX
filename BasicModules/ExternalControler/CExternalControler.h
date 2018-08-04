/*! \file CExternalControler.h
 * A brief file description.
 * A more elaborated file description.
 */
#ifndef _CBASIC_MODULE_ExternalControler_H_
#define _CBASIC_MODULE_ExternalControler_H_

#include <QMainWindow>

#include "CBasicModule.h"
#include "ui_ihm_ExternalControler.h"
#include "CNetworkServerInterface.h"

class CInstructionsSet;

 class Cihm_ExternalControler : public QMainWindow
{
    Q_OBJECT
public:
    Cihm_ExternalControler(QWidget *parent = 0)  : QMainWindow(parent) { ui.setupUi(this); }
    ~Cihm_ExternalControler() { }

    Ui::ihm_ExternalControler ui;

    CApplication *m_application;
 };



 /*! \addtogroup ExternalControler
   * 
   *  @{
   */

	   
/*! @brief class CExternalControler
 */	   
class CExternalControler : public CBasicModule
{
    Q_OBJECT
#define     VERSION_ExternalControler   "1.0"
#define     AUTEUR_ExternalControler    "Nico"
#define     INFO_ExternalControler      "Contrôle de l'outil par une application externe"

public:
    CExternalControler(const char *plugin_name);
    ~CExternalControler();

    Cihm_ExternalControler *getIHM(void) { return(&m_ihm); }

    virtual void init(CApplication *application);
    virtual void close(void);
    virtual bool hasGUI(void)           { return(true); }
    virtual QIcon getIcon(void)         { return(QIcon(":/icons/edit_add.png")); }  // Précise l'icône qui représente le module
    virtual QString getMenuName(void)   { return("BasicModule"); }                  // Précise le nom du menu de la fenêtre principale dans lequel le module apparaît

private:
    Cihm_ExternalControler m_ihm;

    CNetworkServerInterface *m_server;
    tNtwConfig m_server_config;

    CInstructionsSet *m_instructions_set;


private slots :
    void onRightClicGUI(QPoint pos);
    void srv_bytesWritten(int client_id, qint64 bytes);

// From Network Serveur
private slots :
    void srv_newConnection(int client_id);
    void srv_receivedData(int client_id, QByteArray data);
    void srv_opened(QString msg);
    void srv_errorMessage(QString msg);
};

#endif // _CBASIC_MODULE_ExternalControler_H_

/*! @} */


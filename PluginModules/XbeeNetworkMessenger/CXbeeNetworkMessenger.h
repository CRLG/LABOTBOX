/*! \file CXbeeNetworkMessenger.h
 * A brief file description.
 * A more elaborated file description.
 */
#ifndef _CPLUGIN_MODULE_XbeeNetworkMessenger_H_
#define _CPLUGIN_MODULE_XbeeNetworkMessenger_H_

#include <QMainWindow>
#include <QTimer>

#include "CPluginModule.h"
#include "ui_ihm_XbeeNetworkMessenger.h"
#include "xbeemessenger.h"

 class Cihm_XbeeNetworkMessenger : public QMainWindow
{
    Q_OBJECT
public:
    Cihm_XbeeNetworkMessenger(QWidget *parent = 0)  : QMainWindow(parent) { ui.setupUi(this); }
    ~Cihm_XbeeNetworkMessenger() { }

    Ui::ihm_XbeeNetworkMessenger ui;

    CApplication *m_application;
 };



 /*! \addtogroup XbeeNetworkMessenger
   * 
   *  @{
   */

	   
/*! @brief class CXbeeNetworkMessenger
 */	   
class CXbeeNetworkMessenger : public CPluginModule
{
    Q_OBJECT
#define     VERSION_XbeeNetworkMessenger   "1.0"
#define     AUTEUR_XbeeNetworkMessenger    "Nico"
#define     INFO_XbeeNetworkMessenger      "Messages du réseau XBEE"

public:
    CXbeeNetworkMessenger(const char *plugin_name);
    ~CXbeeNetworkMessenger();

    Cihm_XbeeNetworkMessenger *getIHM(void) { return(&m_ihm); }

    virtual void init(CApplication *application);
    virtual void close(void);
    virtual bool hasGUI(void)           { return(true); }
    virtual QIcon getIcon(void)         { return(QIcon(":/icons/edit_add.png")); }  // Précise l'icône qui représente le module
    virtual QString getMenuName(void)   { return("PluginModule"); }                 // Précise le nom du menu de la fenêtre principale dans lequel le module apparaît

    void messengerEncodeRequest(unsigned char *buff_data, unsigned short buff_size, unsigned short dest_address);
    void messengerDisplayAnalyzer(QString str_msg);

private:
    Cihm_XbeeNetworkMessenger m_ihm;
    XbeeMessenger m_xbee_messenger;

    QTimer m_timer;
    bool m_trace_active;

    void initDiagPresencePage();
    void initGeneratorPage();

private slots :
    void onRightClicGUI(QPoint pos);
    void Xbee_readyBytes(QByteArray data, unsigned short src_addr);

    void on_clear_trace();
    void on_timer_tick();
    void on_node_com_status_changed();
    void on_message_name_changed(QString msg);
    void on_message_id_changed();
    void on_send_message();
    void on_trace_active_changed(bool val);
};

#endif // _CPLUGIN_MODULE_XbeeNetworkMessenger_H_

/*! @} */


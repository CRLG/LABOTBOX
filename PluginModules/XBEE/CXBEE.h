/*! \file CXBEE.h
 * A brief file description.
 * A more elaborated file description.
 */
#ifndef _CPLUGIN_MODULE_XBEE_H_
#define _CPLUGIN_MODULE_XBEE_H_

#include <QMainWindow>

#include "CPluginModule.h"
#include "ui_ihm_XBEE.h"
#include "xbeedriver.h"

 class Cihm_XBEE : public QMainWindow
{
    Q_OBJECT
public:
    Cihm_XBEE(QWidget *parent = 0)  : QMainWindow(parent) { ui.setupUi(this); }
    ~Cihm_XBEE() { }

    Ui::ihm_XBEE ui;

    CApplication *m_application;
 };



 /*! \addtogroup XBEE
   * 
   *  @{
   */

	   
/*! @brief class CXBEE
 */	   
class CXBEE : public CPluginModule
{
    Q_OBJECT
#define     VERSION_XBEE   "1.0"
#define     AUTEUR_XBEE    "Nico"
#define     INFO_XBEE      "Module de communication XBEE"

public:
    CXBEE(const char *plugin_name);
    ~CXBEE();

    Cihm_XBEE *getIHM(void) { return(&m_ihm); }

    virtual void init(CApplication *application);
    virtual void close(void);
    virtual bool hasGUI(void)           { return(true); }
    virtual QIcon getIcon(void)         { return(QIcon(":/icons/edit_add.png")); }  // Précise l'icône qui représente le module
    virtual QString getMenuName(void)   { return("PluginModule"); }                 // Précise le nom du menu de la fenêtre principale dans lequel le module apparaît


    void XbeeEvt_readyBytes(unsigned char *buff_data, unsigned char buff_size, unsigned short source_id);
    void XbeeEvt_write(unsigned char *buff_data, unsigned char buff_size);

private:
    Cihm_XBEE m_ihm;
    XbeeDriver m_xbee_driver;
    tXbeeSettings m_xbee_settings;
    bool m_auto_init_xbee;
    bool m_trace_active;

private slots :
    void onRightClicGUI(QPoint pos);
    void on_auto_config_changed(bool val);
    void on_send_config();
    void on_trace_active_changed(bool val);
    void on_send_data();

    void serialReadyBytes(QByteArray data);

public slots :
    void encode(QByteArray data, unsigned short dest_addr);

signals :
    void readyBytes(QByteArray data, unsigned short src_addr);
};

#endif // _CPLUGIN_MODULE_XBEE_H_

/*! @} */


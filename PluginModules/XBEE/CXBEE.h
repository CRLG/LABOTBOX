/*! \file CXBEE.h
 * A brief file description.
 * A more elaborated file description.
 */
#ifndef _CPLUGIN_MODULE_XBEE_H_
#define _CPLUGIN_MODULE_XBEE_H_

#include <QMainWindow>

#include "CPluginModule.h"
#include "ui_ihm_XBEE.h"
#include "messenger.h"

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
#define     AUTEUR_XBEE    "Valentin"
#define     INFO_XBEE      "Communication XBEE"

public:
    CXBEE(const char *plugin_name);
    ~CXBEE();

    Cihm_XBEE *getIHM(void) { return(&m_ihm); }

    virtual void init(CApplication *application);
    virtual void close(void);
    virtual bool hasGUI(void)           { return(true); }
    virtual QIcon getIcon(void)         { return(QIcon(":/icons/edit_add.png")); }  // Précise l'icône qui représente le module
    virtual QString getMenuName(void)   { return("PluginModule"); }                 // Précise le nom du menu de la fenêtre principale dans lequel le module apparaît

private:
    Cihm_XBEE m_ihm;

    RobNetworkMessenger m_messenger;

private slots :
    void onRightClicGUI(QPoint pos);
    void test();
    void initXbee();
    void receiveSerialData(QByteArray datas);
};

#endif // _CPLUGIN_MODULE_XBEE_H_

/*! @} */


/*! \file CMessengerNetwork2019.h
 * A brief file description.
 * A more elaborated file description.
 */
#ifndef _CPLUGIN_MODULE_MessengerNetwork2019_H_
#define _CPLUGIN_MODULE_MessengerNetwork2019_H_

#include <QMainWindow>

#include "CPluginModule.h"
#include "ui_ihm_MessengerNetwork2019.h"
#include "MessengerXbeeNetwork2019.h"

 class Cihm_MessengerNetwork2019 : public QMainWindow
{
    Q_OBJECT
public:
    Cihm_MessengerNetwork2019(QWidget *parent = 0)  : QMainWindow(parent) { ui.setupUi(this); }
    ~Cihm_MessengerNetwork2019() { }

    Ui::ihm_MessengerNetwork2019 ui;

    CApplication *m_application;
 };



 /*! \addtogroup MessengerNetwork2019
   * 
   *  @{
   */

	   
/*! @brief class CMessengerNetwork2019
 */	   
class CMessengerNetwork2019 : public CPluginModule
{
    Q_OBJECT
#define     VERSION_MessengerNetwork2019   "1.0"
#define     AUTEUR_MessengerNetwork2019    "Nico"
#define     INFO_MessengerNetwork2019      "Test d'un messenger"

public:
    CMessengerNetwork2019(const char *plugin_name);
    ~CMessengerNetwork2019();

    Cihm_MessengerNetwork2019 *getIHM(void) { return(&m_ihm); }

    virtual void init(CApplication *application);
    virtual void close(void);
    virtual bool hasGUI(void)           { return(true); }
    virtual QIcon getIcon(void)         { return(QIcon(":/icons/edit_add.png")); }  // Précise l'icône qui représente le module
    virtual QString getMenuName(void)   { return("PluginModule"); }                 // Précise le nom du menu de la fenêtre principale dans lequel le module apparaît

private:
    Cihm_MessengerNetwork2019 m_ihm;

    MessengerXbeeNetwork m_messenger;

private slots :
    void onRightClicGUI(QPoint pos);
    void test();
    void receiveSerialData(QByteArray datas);
};

#endif // _CPLUGIN_MODULE_MessengerNetwork2019_H_

/*! @} */


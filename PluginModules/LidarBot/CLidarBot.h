/*! \file CLidarBot.h
 * A brief file description.
 * A more elaborated file description.
 */
#ifndef _CPLUGIN_MODULE_LidarBot_H_
#define _CPLUGIN_MODULE_LidarBot_H_

#include <QMainWindow>

#include "CPluginModule.h"
#include "ui_ihm_LidarBot.h"

 class Cihm_LidarBot : public QMainWindow
{
    Q_OBJECT
public:
    Cihm_LidarBot(QWidget *parent = 0)  : QMainWindow(parent) { ui.setupUi(this); }
    ~Cihm_LidarBot() { }

    Ui::ihm_LidarBot ui;

    CApplication *m_application;
 };



 /*! \addtogroup LidarBot
   * 
   *  @{
   */

	   
/*! @brief class CLidarBot
 */	   
class CLidarBot : public CPluginModule
{
    Q_OBJECT
#define     VERSION_LidarBot   "1.0"
#define     AUTEUR_LidarBot    "Laguiche"
#define     INFO_LidarBot      "Manage LIDAR beacon of the bot"

public:
    CLidarBot(const char *plugin_name);
    ~CLidarBot();

    Cihm_LidarBot *getIHM(void) { return(&m_ihm); }

    virtual void init(CApplication *application);
    virtual void close(void);
    virtual bool hasGUI(void)           { return(true); }
    virtual QIcon getIcon(void)         { return(QIcon(":/icons/edit_add.png")); }  // Précise l'icône qui représente le module
    virtual QString getMenuName(void)   { return("PluginModule"); }                 // Précise le nom du menu de la fenêtre principale dans lequel le module apparaît

private:
    Cihm_LidarBot m_ihm;

private slots :
    void onRightClicGUI(QPoint pos);
};

#endif // _CPLUGIN_MODULE_LidarBot_H_

/*! @} */


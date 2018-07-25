/*! \file CMainWindow.h
 * A brief file description.
 * A more elaborated file description.
 */
#ifndef _CBASIC_MODULE_MAIN_WINDOW_H_
#define _CBASIC_MODULE_MAIN_WINDOW_H_

#include <QMainWindow>
#include <QDialog>
#include <QMap>
#include <QMenu>

#include "CBasicModule.h"
#include "ui_ihm_main_window.h"
#include "ui_ihm_infos_modules.h"



 class Cihm_main_window : public QMainWindow
{
    Q_OBJECT
public:
    Cihm_main_window(QWidget *parent = 0)  : QMainWindow(parent) { ui.setupUi(this); }
    ~Cihm_main_window() { }

    Ui::ihm_main_window ui;

    CLaBotBox *m_application;

    void closeEvent(QCloseEvent *event)
    {
        Q_UNUSED(event)
        emit DdeFinApplication();
    }
signals :
    void DdeFinApplication(void);
 };



 class Cihm_infos_module : public QDialog
{
    Q_OBJECT
public:
    Cihm_infos_module(QWidget *parent = 0)  : QDialog(parent) { ui.setupUi(this); }
    ~Cihm_infos_module() { }

    Ui::ihm_infos_module ui;
 };


 

 /*! \addtogroup PrintView
   * 
   *  @{
   */

	   
/*! @brief class CMainWindow in @link PrintView basic module.
 */	   
class CMainWindow : public CBasicModule
{
    Q_OBJECT
#define     VERSION_MAIN_WINDOW     "1.0"
#define     AUTEUR_MAIN_WINDOW      "Nico"
#define     INFO_MAIN_WINDOW        "Main Window"

typedef QMap<QString, QMenu *>tListeMenu;


//! Niveau des messages de trace (à déplacer dans le module Logger)

public:
    CMainWindow(const char *plugin_name);
    ~CMainWindow();

    Cihm_main_window *getIHM(void) { return(&m_ihm); }

    virtual void init(CLaBotBox *application);
    virtual void close(void);
    virtual bool hasGUI(void) { return(true); }
    virtual QIcon getIcon(void) { return(QIcon(":/icons/peertopeer.png")); }

    // Fournit l'accès à la barre de menu pour qu'un module puisse ajouter des menus, actions, ... en fonction du besoin
    QMenuBar    *getMenuBar(void)   { return(m_ihm.menuBar()); }
    QStatusBar  *getStatusBar(void) { return(m_ihm.statusBar()); }
    QMenu       *getMenu(QString name); // TODO à implémenter
private:
    Cihm_main_window m_ihm;

    tListeMenu m_liste_menu;

    void createMenuModules(void);
    void createMenuHelp(void);

public slots :
    void About(void);
    void InfoModules(void);
};

#endif // _CBASIC_MODULE_MAIN_WINDOW_H_

/*! @} */


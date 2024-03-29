/*! \file CMainWindow.cpp
 * A brief file description CPP.
 * A more elaborated file description.
 */
#include <QDebug>
#include <QFile>
#include <QFileInfo>
#include "CMainWindow.h"
#include "CApplication.h"
#include "CEEPROM.h"



/*! \addtogroup PluginModule_Test
   * 
   *  @{
   */
	   
// _____________________________________________________________________
/*!
*  Constructeur
*
*/
CMainWindow::CMainWindow(const char *plugin_name)
    :CBasicModule(plugin_name, VERSION_MAIN_WINDOW, AUTEUR_MAIN_WINDOW, INFO_MAIN_WINDOW)
{
}


// _____________________________________________________________________
/*!
*  Destructeur
*
*/
CMainWindow::~CMainWindow()
{

}

// _____________________________________________________________________
/*!
*  Initialisation du module
*
*/
void CMainWindow::init(CApplication *application)
{
  CBasicModule::init(application);

  m_ihm.m_application = m_application;
  QStatusBar *statusBar = new QStatusBar();
  m_ihm.setStatusBar(statusBar);
  statusBar->showMessage(tr("Hello status bar"));

  m_ihm.setWindowTitle(getName());
  m_ihm.setWindowIcon(getIcon());

  // Restore la taille de la fenï¿½tre
  QVariant val;
  val = m_application->m_eeprom->read(getName(), "geometry", QRect(20, 20, 350, 150));
  m_ihm.setGeometry(val.toRect());
  // Restore le fait que la fenï¿½tre est visible ou non
  val = m_application->m_eeprom->read(getName(), "visible", QVariant(true));
  if (val.toBool()) { m_ihm.show(); }
  else              { m_ihm.hide(); }


  connect(&m_ihm, SIGNAL(DdeFinApplication()), m_application, SLOT(TermineApplication()));
  connect(m_ihm.ui.actionQuit, SIGNAL(triggered()), m_application, SLOT(TermineApplication()));
  connect(m_ihm.ui.actionInfo_modules, SIGNAL(triggered()), this, SLOT(InfoModules()));
  connect(m_ihm.ui.actionAbout, SIGNAL(triggered()), this, SLOT(About()));

  createMenuModules();
  createMenuHelp();  // l'appel de cette méthode doit etre etre en dernier pour que le menu "?" soit placé tout à droite
}


// _____________________________________________________________________
/*!
*  Fermeture du module
*
*/
void CMainWindow::close(void)
{
 // Mï¿½morise en EEPROM l'ï¿½tat de la fenï¿½tre
 m_application->m_eeprom->write(getName(), "geometry", QVariant(m_ihm.geometry()));
 m_application->m_eeprom->write(getName(), "visible", QVariant(m_ihm.isVisible()));
}



// _____________________________________________________________________
/*!
*  Menu About
*
*/
void CMainWindow::About(void)
{

}


void  CMainWindow::InfoModules(void)
{
  Cihm_infos_module *dlg = new Cihm_infos_module();
  //dlg->ui.table_modules->insertColumn(4);
  dlg->ui.table_modules->setRowCount(m_application->m_list_modules.size());

  dlg->ui.table_modules->setColumnWidth(0, 120);
  dlg->ui.table_modules->setColumnWidth(1, 40);
  dlg->ui.table_modules->setColumnWidth(2, 300);

  for (int i=0; i<m_application->m_list_modules.size(); i++) {
    dlg->ui.table_modules->setItem(i, 0, new QTableWidgetItem(m_application->m_list_modules[i]->getName()));
    dlg->ui.table_modules->setItem(i, 1, new QTableWidgetItem(m_application->m_list_modules[i]->getVersion()));
    dlg->ui.table_modules->setItem(i, 2, new QTableWidgetItem(m_application->m_list_modules[i]->getDescription()));
    dlg->ui.table_modules->setRowHeight(i, 20);
  }

  dlg->exec();
}


// _____________________________________________________________________
/*!
*   Renvoie un pointeur sur le menu s'il existe ou null sinon
*
*/
QMenu *CMainWindow::getExistingMenu(QString name)
{
    tListeMenu::iterator it1 = m_liste_menu.find(name);
    if (it1 != m_liste_menu.end()) {
        return it1.value();
    }
    return nullptr;
}

// _____________________________________________________________________
/*!
*  Crée le menu s'il n'existe pas
*  Exemple :    BasicModule         -> Créé le menu BasicModule
*               BasicModule/Data    -> Crée le menu BasicModule et le sous menu Data en dessous
*               N1/N2/N3            -> Crée le menu N1, son sous module N2 et un sous module N3 de N2
*
*/
QMenu *CMainWindow::getMenu(QString name)
{
    // Le menu existe déjà
    QMenu *menu = getExistingMenu(name);
    if (menu) return menu;

    QStringList split_names = name.split("/");
    QString tree_name="";
    QMenu *parent_menu=nullptr;
    for (int i=0; i < split_names.size(); i++) {
        if (i != 0) tree_name.append("/");
        tree_name.append(split_names.at(i));

        menu = getExistingMenu(tree_name);
        if (!menu) { // Le menu n'existe pas encore -> le créé
            if (i==0)               menu = m_ihm.menuBar()->addMenu(split_names.at(i));   // le menu de plus haut niveau s'attache à la barre principale
            else if (parent_menu)   menu = parent_menu->addMenu(split_names.at(i));       // les sous-menus s'attachent au menu parent
        }
        parent_menu = menu;
        m_liste_menu.insert(tree_name, menu); // ajoute le nouveau menu à la liste
    }
    return(menu);
}


// _____________________________________________________________________
/*!
* Crï¿½e les menus
*
*/
void CMainWindow::createMenuModules(void)
{
    // Ajoute pour chaque module possï¿½dant une IHM un menu une action
    // permattant l'affichage du module
    for (int i=0; i<m_application->m_list_modules.size(); i++) {
      // Le module CMainWindow lui mï¿½me n'as pas besoin d'apparaitre dans le menu car il est toujours visible
      if (m_application->m_list_modules[i] != this) {
        if(m_application->m_list_modules[i]->hasGUI()) {
            QAction *Act = new QAction(m_application->m_list_modules[i]->getIcon(), m_application->m_list_modules[i]->getName(), this);
            Act->setStatusTip(m_application->m_list_modules[i]->getDescription());
            // ajoute ce module ï¿½ la barre de menu
            getMenu(m_application->m_list_modules[i]->getMenuName())->addAction(Act);
            connect(Act, SIGNAL(triggered()), m_application->m_list_modules[i], SLOT(setVisible()));  // Rend visible le panel du module lorsqu'il est rappelï¿½ depuis le menu
        } // if le module possï¿½de une IHM
      } // if le module n'est pas le module CMainWindow
    } // pour tous les basic modules
}


// _____________________________________________________________________
/*!
* Crée le menu Help
*
*/
void CMainWindow::createMenuHelp(void)
{
    QMenu *menu = getMenu("?");
    QAction *act = NULL;

    act = new QAction("Info modules", this);
    menu->addAction(act);
    connect(act, SIGNAL(triggered()), this, SLOT(InfoModules()));

    act = new QAction("About", this);
    menu->addAction(act);
    connect(act, SIGNAL(triggered()), this, SLOT(About()));
}

/*! @} */

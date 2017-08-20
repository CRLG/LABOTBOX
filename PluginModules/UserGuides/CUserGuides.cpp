/*! \file CUserGuides.cpp
 * A brief file description CPP.
 * A more elaborated file description.
 */
#include <QDebug>
#include "CUserGuides.h"
#include "CLaBotBox.h"
#include "CPrintView.h"
#include "CMainWindow.h"
#include "CEEPROM.h"
#include "CDataManager.h"

#include "htmltexteditor.h"



/*! \addtogroup Module_Test2
   * 
   *  @{
   */
	   
// _____________________________________________________________________
/*!
*  Constructeur
*
*/
CUserGuides::CUserGuides(const char *plugin_name)
    :CPluginModule(plugin_name, VERSION_UserGuides, AUTEUR_UserGuides, INFO_UserGuides),
     m_html_text_editor(NULL)
{
}


// _____________________________________________________________________
/*!
*  Destructeur
*
*/
CUserGuides::~CUserGuides()
{
    if (!m_html_text_editor)  {
        delete m_html_text_editor;
    }
}


// _____________________________________________________________________
/*!
*  Initialisation du module
*
*/
void CUserGuides::init(CLaBotBox *application)
{
  CModule::init(application);
  setGUI(&m_ihm); // indique à la classe de base l'IHM
  m_ihm.ui.menubar->setVisible(true);

  // Gère les actions sur clic droit sur le panel graphique du module
  m_ihm.setContextMenuPolicy(Qt::CustomContextMenu);
  connect(&m_ihm, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(onRightClicGUI(QPoint)));

  QVariant val;
  // Restore le niveau d'affichage
  val = m_application->m_eeprom->read(getName(), "niveau_trace", QVariant(MSG_TOUS));
  setNiveauTrace(val.toUInt());

  // Restore la couleur de fond
  val = m_application->m_eeprom->read(getName(), "background_color", QVariant(DEFAULT_MODULE_COLOR));
  setBackgroundColor(val.value<QColor>());

  connect(m_ihm.ui.userguides_list, SIGNAL(currentTextChanged(QString)), this, SLOT(onUserGuideSelectedChange(QString)));
  m_ihm.ui.useguides_textedit->setReadOnly(true);
  connect(m_ihm.ui.actionUserGuideEditor, SIGNAL(triggered()), this, SLOT(onUserGuideEditorSelected()));
  initListUserGuides();
}


// _____________________________________________________________________
/*!
*  Fermeture du module
*
*/
void CUserGuides::close(void)
{
  // Mémorise en EEPROM l'état de la fenêtre
  m_application->m_eeprom->write(getName(), "niveau_trace", QVariant((unsigned int)getNiveauTrace()));
  m_application->m_eeprom->write(getName(), "background_color", QVariant(getBackgroundColor()));
}

// _____________________________________________________________________
/*!
*  Création des menus sur clic droit sur la fenêtre du module
*
*/
void CUserGuides::onRightClicGUI(QPoint pos)
{
  QMenu *menu = new QMenu();

  menu->addAction("Select background color", this, SLOT(selectBackgroundColor()));
  menu->exec(m_ihm.mapToGlobal(pos));
}


// _____________________________________________________________________
/*!
*  Création des menus sur clic droit sur la fenêtre du module
*
*/
void CUserGuides::initListUserGuides()
{
    QStringList list;
    for (int i=0; i<m_application->m_list_modules.size(); i++) {
      if (m_application->m_list_modules[i]->hasUserGuide()) {
        list << m_application->m_list_modules[i]->getName();
      }
    }
    list.sort();
    m_ihm.ui.userguides_list->addItems(list);
}


// _____________________________________________________________________
/*!
*  Affiche le user guide du module sélectionné
*
*/
void CUserGuides::onUserGuideSelectedChange(QString name)
{
    for (int i=0; i<m_application->m_list_modules.size(); i++) {
      if (m_application->m_list_modules[i]->getName() == name) {
        m_ihm.ui.useguides_textedit->setText(m_application->m_list_modules[i]->getUserGuide());
      }
    }
}

// _____________________________________________________________________
/*!
*  Lance l'éditeur html de user guide
*
*/
void CUserGuides::onUserGuideEditorSelected()
{
  if (m_html_text_editor == NULL)  {
      m_html_text_editor = new HtmlTextEditor();
  }
  m_html_text_editor->show();
}

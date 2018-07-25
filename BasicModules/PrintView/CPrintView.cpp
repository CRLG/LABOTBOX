/*! \file CPrintView.cpp
 * A brief file description CPP.
 * A more elaborated file description.
 */
#include <QDebug>
#include <QFile>
#include <QDateTime>
#include <QDate>
#include "CMainWindow.h"
#include "CPrintView.h"
#include "NiveauTraceModules.h"


/*! \addtogroup PrintView
   * 
   *  @{
   */
	   
// _____________________________________________________________________
/*!
*  Constructeur
*
*/
CPrintView::CPrintView(const char *plugin_name)
    :   CBasicModule(plugin_name, VERSION, AUTEUR, INFO),
        m_trace_enable(true)
{
}


// _____________________________________________________________________
/*!
*  Destructeur
*
*/
CPrintView::~CPrintView()
{
  if (m_ihm_print_view) { delete m_ihm_print_view; }
}

// _____________________________________________________________________
/*!
*  Initialisation du module
*
*/
void CPrintView::init(CLaBotBox *application)
{
  CBasicModule::init(application);

  //m_dialog = new QDialog(application->m_main_windows->m_father);
  m_dialog = new QDialog();
  m_ihm_print_view  = new Ui::ihm_print_view();
  m_ihm_print_view->setupUi(m_dialog);

  // signaux et slots
  connect(m_ihm_print_view->pb_parametrer_modules, SIGNAL(clicked()), this, SLOT(ConfigurerNiveauxModules()));
  connect(m_ihm_print_view->pb_effacer_liste, SIGNAL(clicked()), m_ihm_print_view->liste_trace, SLOT(clear()));
  connect(m_ihm_print_view->activer_trace, SIGNAL(clicked(bool)), this, SLOT(setEnableTrace(bool)));
  connect(m_ihm_print_view->pb_enregistrer_trace, SIGNAL(clicked()), this, SLOT(enregistreTrace()));

  m_dialog->setModal(false);
  m_dialog->setWindowTitle(getName());
  m_dialog->show();


  // Met en cohérence la checkbox avec la valeur d'init d'autorisation de la trace
  m_ihm_print_view->activer_trace->setChecked(m_trace_enable);
//  m_dialog->setWindowIcon(getIcon());
//  m_dialog->setStatusTip(getDescription());

  setGUI(m_dialog);

}


// _____________________________________________________________________
/*!
*  Fermeture du module
*
*/
void CPrintView::close(void)
{
  //TODO : enregistrer les infos en EEPROM
}

// _____________________________________________________________________
/*!
*  Ecrit une  valeur dans la data
*
*  \param [in] msg le message a logger
*/
void CPrintView::print(QString msg)
{
  if (!m_trace_enable) { return; }
  m_ihm_print_view->liste_trace->insertPlainText(msg + "\n");
  m_ihm_print_view->liste_trace->moveCursor(QTextCursor::End);  // Place le curseur à la fin pour s'assurer que le dernier message est celui lisible par l'utilisateur
}


// _____________________________________________________________________
/*!
*  Ecrit une  valeur dans la data
*
*  \param [in] msg le message a logger
*/
void CPrintView::print(CModule *module, QString msg, unsigned long type_trace)
{
  QColor couleur;

  if (!m_trace_enable) { return; }

  // Est-ce que ce type de message est autorisé à être affiché ?
  if( (module->getNiveauTrace() & type_trace) || (type_trace==MSG_ERREUR) ){
     if         (type_trace == MSG_ERREUR)  {  couleur = Qt::red; }
     else if    (type_trace&MSG_WARNING)    {  couleur = Qt::darkGreen; }
     else                                   {  couleur = Qt::black; }

     m_ihm_print_view->liste_trace->setTextColor(Qt::darkBlue);
     m_ihm_print_view->liste_trace->setFontItalic(true);
     m_ihm_print_view->liste_trace->insertPlainText(module->getName() + "> ");
     m_ihm_print_view->liste_trace->setTextColor(couleur);
     m_ihm_print_view->liste_trace->setFontItalic(false);
     m_ihm_print_view->liste_trace->insertPlainText(msg + "\n");

     m_ihm_print_view->liste_trace->moveCursor(QTextCursor::End);  // Place le curseur à la fin pour s'assurer que le dernier message est celui lisible par l'utilisateur
  }
}



// _____________________________________________________________________
/*!
*
* Ouvre une fenêtre permettant à l'utilisateur de modifier
*  le niveau d'affichage de chaque module
*/
void CPrintView::ConfigurerNiveauxModules(void)
{
  CNiveauTraceModules *dialog;
  CNiveauTraceModules::t_map_niv_aff map_niv_aff;

  dialog = new CNiveauTraceModules();

  // Construit la liste et la communique au module graphique dédié
  // On passe le nom de chaque module ainsi que le niveau d'affichage associé
  for (int i=0; i<m_application->m_list_modules.size(); i++) {
      map_niv_aff.insert(m_application->m_list_modules[i]->getName(),
                         m_application->m_list_modules[i]->getNiveauTrace() );
  }
  dialog->setListeModules(&map_niv_aff);
  dialog->exec();

  // en sortie d'exec(), le map contient pour chaque module
  // le niveau d'afichage attendu qu'l faut restituer à chaque module

  // Met à jour pour chaque module le niveau d'affichage sélectionné par l'utilisateur
  for (int i=0; i<m_application->m_list_modules.size(); i++) {
      CNiveauTraceModules::t_map_niv_aff::const_iterator it;
      it = map_niv_aff.find(m_application->m_list_modules[i]->getName());
      if (it != map_niv_aff.end()) {  // le module a une correspondance dans la liste issue des modifications utilisateurs
         m_application->m_list_modules[i]->setNiveauTrace(it.value());
      }
  }
  if (dialog) { delete dialog; }
}


// _____________________________________________________________________
/*!
*
*
*/
void CPrintView::setEnableTrace(bool on_off)
{
    m_trace_enable = on_off;
}



// _____________________________________________________________________
/*!
* Memorise le contenu de la fenêtre de trace dans un fichier
*
*/
void CPrintView::enregistreTrace()
{
  QString pathfilename;
  QDateTime date_time = QDateTime::currentDateTime();

  // Construit une chaine de type :
  //    CheminEnregistrement/<NomModule>_<DateHeure>.html
  pathfilename =    m_application->m_pathname_log_file +
                    "/" +
                    QString(getName()).replace(" ", "") + // Supprime les espaces dans le nom généré
                    "_" +
                    date_time.toString("yyyy_MM_dd_hh'h'mm'm'ss's'") +
                    ".html";
  QFile file(pathfilename);
  if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
      print_error(this, "Impossible d'ouvrir en écriture le fichier " + pathfilename);
      return;
  }

  QTextStream out(&file);
  out <<  m_ihm_print_view->liste_trace->toHtml();
  file.close();

  print_debug(this, "Fichier de trace sauvegardé: " + pathfilename);
}


// _____________________________________________________________________
/*!
*  Affichage du module
*
*/
//void CPrintView::setVisible(void)
//{
  //m_ihm.show();
//    m_dialog->show();
//}

/*! @} */

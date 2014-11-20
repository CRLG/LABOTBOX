/*! \file CModuleDesigner.cpp
 * A brief file description CPP.
 * A more elaborated file description.
 */
#include <QDebug>
#include <QFile>
#include <QDir>
#include <QFileDialog>
#include <QMessageBox>
#include "CModuleDesigner.h"
#include "CLaBotBox.h"
#include "CPrintView.h"
#include "CMainWindow.h"
#include "CEEPROM.h"
#include "CDataManager.h"



/*! \addtogroup PluginModule_Test2
   * 
   *  @{
   */
	   
// _____________________________________________________________________
/*!
*  Constructeur
*
*/
CModuleDesigner::CModuleDesigner(const char *plugin_name)
    :CPluginModule(plugin_name, VERSION_MODULE_CREATOR, AUTEUR_MODULE_CREATOR, INFO_MODULE_CREATOR)
{
}


// _____________________________________________________________________
/*!
*  Destructeur
*
*/
CModuleDesigner::~CModuleDesigner()
{

}


// _____________________________________________________________________
/*!
*  Affichage du module
*
*/
//void CModuleDesigner::setVisible(void)
//{
//  m_ihm.show();
//}


// _____________________________________________________________________
/*!
*  Initialisation du module
*
*/
void CModuleDesigner::init(CLaBotBox *application)
{
  CPluginModule::init(application);
  setGUI(&m_ihm); // indique à la classe de base l'IHM
  setNiveauTrace(MSG_TOUS);

  // Restore la taille de la fenêtre
/*  QVariant val;
  val = m_application->m_eeprom->read(getName(), "geometry", 0);
  m_ihm.setGeometry(val.toRect());
  // Restore le fait que la fenêtre est visible ou non
  val = m_application->m_eeprom->read(getName(), "visible", QVariant(true));
  if (val.toBool()) { m_ihm.show(); }
  else              { m_ihm.hide(); }
*/
  // Restaure le chemin de génération du module
  QVariant val;
  val = m_application->m_eeprom->read(getName(), "repertoire_projet", "./"); 
  m_ihm.ui.repertoire_projet->setText(val.toString());

  connect(m_ihm.ui.PB_generate, SIGNAL(clicked()), this, SLOT(genererModule()));
  connect(m_ihm.ui.PB_choix_repertoire, SIGNAL(clicked()), this, SLOT(choixRepertoireSortie()));

  connect(m_ihm.ui.PB_integrer_module_existant, SIGNAL(clicked()), this, SLOT(integrerModuleExistantAuProjet()));
  connect(m_ihm.ui.PB_choix_repertoire_module_existant, SIGNAL(clicked()), this, SLOT(choixRepertoireModuleExistant()));

  connect(m_ihm.ui.PB_desintegrer_module, SIGNAL(clicked()), this, SLOT(desintegrerModuleExistantDuProjet()));
  connect(m_ihm.ui.PB_choix_repertoire_module_desintegration, SIGNAL(clicked()), this, SLOT(choixRepertoireModuleDesintegration()));
  
}


// _____________________________________________________________________
/*!
*  Fermeture du module
*
*/
void CModuleDesigner::close(void)
{
  // Mémorise en EEPROM l'état de la fenêtre
  m_application->m_eeprom->write(getName(), "geometry", QVariant(m_ihm.geometry()));
  m_application->m_eeprom->write(getName(), "visible", QVariant(m_ihm.isVisible()));
  m_application->m_eeprom->write(getName(), "repertoire_projet", QVariant(m_ihm.ui.repertoire_projet->text()));
}



// _____________________________________________________________________
/*!
*  Génère du module
*
*/
void CModuleDesigner::genererModule(void)
{
  bool status = true;
  
  status = creerModule();
  if (status == false) { return; }

  if (m_ihm.ui.chbx_integrer_modume_au_projet->isChecked()) {
    status = integrerNouveauModuleCreeAuProjet();
  }
}


// _____________________________________________________________________
/*!
*  Crée du module
*
*/
bool CModuleDesigner::creerModule(void)
{
  bool status = true;
  QString module_template_cpp = "";
  QString module_template_h = "";
  QString module_template_ui = "";
  QString nom_module_a_generer = m_ihm.ui.nom_module->text();

  m_application->m_print_view->print_info(this, "GenererModule");
  
  QString type_module="";
  if (m_ihm.ui.ModuleTypeBasic->isChecked()) {
    type_module = "Basic";
  }
  else {
    type_module = "Plugin";
  }
  
  m_application->m_print_view->print_info(this, type_module + " / " +
                                                nom_module_a_generer  + " / " +
                                                m_ihm.ui.auteur->text() + " / " +
                                                m_ihm.ui.description->text() +  " / " +
                                                QString::number(m_ihm.ui.chbx_interface_graphique->isChecked())
                                               );

  // Lit les fichiers templates adaptés au type de module à générer
  if (m_ihm.ui.ModuleTypeBasic->isChecked()) { // BasicModule
      if (m_ihm.ui.chbx_interface_graphique->isChecked()) {  // Avec IHM
        module_template_cpp = readFile(QString(PREFIXE_QRC_TEMPLATE_CODE) + "BasicModuleAvecIHM/CModuleTemplate.tpl.cpp");
        module_template_h = readFile(QString(PREFIXE_QRC_TEMPLATE_CODE) + "BasicModuleAvecIHM/CModuleTemplate.tpl.h");
        module_template_ui = readFile(QString(PREFIXE_QRC_TEMPLATE_CODE) + "BasicModuleAvecIHM/ihm_ModuleTemplate.tpl.ui");
      }
      else {  // Sans IHM
        module_template_cpp = readFile(QString(PREFIXE_QRC_TEMPLATE_CODE) + "BasicModuleSansIHM/CModuleTemplate.tpl.cpp");
        module_template_h = readFile(QString(PREFIXE_QRC_TEMPLATE_CODE) + "BasicModuleSansIHM/CModuleTemplate.tpl.h");
        module_template_ui = "";
      }
  }
  else { // PluginModule
      if (m_ihm.ui.chbx_interface_graphique->isChecked()) {  // Avec IHM
        module_template_cpp = readFile(QString(PREFIXE_QRC_TEMPLATE_CODE) + "PluginModuleAvecIHM/CModuleTemplate.tpl.cpp");
        module_template_h = readFile(QString(PREFIXE_QRC_TEMPLATE_CODE) + "PluginModuleAvecIHM/CModuleTemplate.tpl.h");
        module_template_ui = readFile(QString(PREFIXE_QRC_TEMPLATE_CODE) + "PluginModuleAvecIHM/ihm_ModuleTemplate.tpl.ui");
      }
      else {  // Sans IHM
        module_template_cpp = readFile(QString(PREFIXE_QRC_TEMPLATE_CODE) + "PluginModuleSansIHM/CModuleTemplate.tpl.cpp");
        module_template_h = readFile(QString(PREFIXE_QRC_TEMPLATE_CODE) + "PluginModuleSansIHM/CModuleTemplate.tpl.h");
        module_template_ui = "";
      }
  }
  
  // Test sur la cohérence des fichiers templates
  if ((module_template_cpp.length() == 0) || (module_template_h.length() == 0) ) {
      m_application->m_print_view->print_error(this, "Probleme d'ouverture des fichiers template");
      return(false);
  }

  // Remplace les balises dans les fichiers templates
  module_template_cpp.replace("ModuleTemplate", nom_module_a_generer);
  module_template_h.replace("ModuleTemplate", nom_module_a_generer);
  module_template_ui.replace("ModuleTemplate", nom_module_a_generer);
  module_template_h.replace("##AUTEUR##", m_ihm.ui.auteur->text());
  module_template_h.replace("##DESCRIPTION##", m_ihm.ui.description->text());

  // Crée le répertoire de sortie
  QString output_dirname; 
  if (m_ihm.ui.ModuleTypeBasic->isChecked())    { output_dirname = m_ihm.ui.repertoire_projet->text() + "/BasicModules/" + nom_module_a_generer; }
  else                                          { output_dirname = m_ihm.ui.repertoire_projet->text() + "/PluginModules/" + nom_module_a_generer; }
  bool enable_creation=true;
  QDir directory;
  if (directory.exists(output_dirname)) { // Demande confirmation à l'utilisateur si le répertoir existe déjà
     QMessageBox msgBox;
     msgBox.setText("Le répertoire existe déjà: " + output_dirname);
     msgBox.setInformativeText("Voulez vous l'écraser ?");
     msgBox.setIcon(QMessageBox::Warning);
     msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
     msgBox.setDefaultButton(QMessageBox::Yes);
     int ret = msgBox.exec();  
     if (ret == QMessageBox::No) { 
         enable_creation = false; 
         m_application->m_print_view->print_warning(this, "Annulation de la création du module: " + nom_module_a_generer);
     }
  }
  
  if (enable_creation) {
      // Crée le répertoire s'il n'existe pas déjà
      if (directory.exists(output_dirname) == false) {
          if (directory.mkdir(output_dirname) == false) { 
              m_application->m_print_view->print_error(this, "Impossible de créer le répertoire: " + output_dirname); 
              return(false);
          }
      }
      if (module_template_cpp != "")    { writeFile(output_dirname, "C"+m_ihm.ui.nom_module->text()+".cpp", module_template_cpp); }
      if (module_template_h != "")      { writeFile(output_dirname, "C"+m_ihm.ui.nom_module->text()+".h", module_template_h); }
      if (module_template_ui != "")     { writeFile(output_dirname, "ihm_"+m_ihm.ui.nom_module->text()+".ui", module_template_ui); }
      m_application->m_print_view->print_info(this, "Creation du module: " + nom_module_a_generer);
  }
  else {
      status = false;
  }
  return(status);
}


// _____________________________________________________________________
/*!
*  Intègre le module nouvellement créé au projet
*
*/
bool CModuleDesigner::integrerNouveauModuleCreeAuProjet(void)
{
  QString typeModuleString="";

 if (m_ihm.ui.ModuleTypeBasic->isChecked()) { typeModuleString = "BASIC"; }
 else                                       { typeModuleString = "PLUGIN"; }

 return(integrerModuleAuProjet( typeModuleString, 
                                m_ihm.ui.repertoire_projet->text(),
                                m_ihm.ui.nom_module->text())
       );
}




// _____________________________________________________________________
/*!
*  Modifie certains fichiers du projet pour intégrer le nouveau module
*
*/
bool CModuleDesigner::integrerModuleAuProjet(QString type_module,
                                            QString repertoire_projet,
                                            QString nom_module)
{
  bool status = true;
  QString pathfilename="";
  QString replaceString="";
  QString tagName="";

  // _______________________________________________
  // Ouvre le fichier LaBotBox.pro et ajoute le nouveau module
  pathfilename = repertoire_projet + "/LaBotBox.pro";
  QString pro_file = readFile(pathfilename); 
  if (pro_file == "") {
    m_application->m_print_view->print_error(this, "Fichier inexistant : " + pathfilename);
    return(false);
  }
  // Ajoute le module
  tagName = "# ##_NEW_"+ type_module + "_MODULE_NAME_HERE_##";
  replaceString = nom_module + " \\ \r\n" + "        " + tagName;
  pro_file.replace(tagName, replaceString);
  m_application->m_print_view->print_debug(this, pro_file);
  writeFile(pathfilename, pro_file);

  // _______________________________________________
  // Ouvre le fichier LaBotBox.h et ajoute le nouveau module
  pathfilename = repertoire_projet + "/CLaBotBox.h";
  QString header_file = readFile(pathfilename); 
  if (header_file == "") {
    m_application->m_print_view->print_error(this, "Fichier inexistant : " + pathfilename);
    return(false);
  }
  // Remplace les balises
  tagName = "//##_NEW_CLASS_"+ type_module + "_MODULE_HERE_##";
  replaceString = "class C" + nom_module + ";\r\n" + tagName;
  header_file.replace(tagName, replaceString);
  
  tagName = "//##_NEW_"+ type_module + "_MODULE_CLASS_POINTER_DEFINITION_##";  // //##_NEW_BASIC_MODULE_CLASS_POINTER_DEFINITION_##
  replaceString = "   C" + nom_module + "     *m_" + nom_module + ";\r\n" + tagName;  
  header_file.replace(tagName, replaceString);

  // Ecrit le fichier (remplace l'ancien)
  writeFile(pathfilename, header_file);

  // _______________________________________________
  // Ouvre le fichier LaBotBox.cpp et ajoute le nouveau module
  pathfilename = repertoire_projet + "/CLaBotBox.cpp";
  QString cpp_file = readFile(pathfilename); 
  if (header_file == "") {
    m_application->m_print_view->print_error(this, "Fichier inexistant : " + pathfilename);
    return(false);
  }
  // Remplace la balise //_##NEW_INCLUDE_BASIC_MODULE_HERE_## -> #include "CnewModule.h"
  tagName = "//_##NEW_INCLUDE_"+ type_module + "_MODULE_HERE_##";
  replaceString = "#include \"C" + nom_module + ".h\"\r\n" + tagName;
  cpp_file.replace(tagName, replaceString);

  // Remplace la balise // ##_NEW_BASIC_MODULE_INSTANCIATION_HERE_## -> instanciation du module
  tagName = "// ##_NEW_"+ type_module + "_MODULE_INSTANCIATION_HERE_##";
  replaceString =  "  m_" + nom_module + "     = new C" + nom_module + "(\"" + nom_module + "\");\r\n" ;
  replaceString += "  m_list_" + type_module.toLower() + "_modules.append(m_" + nom_module + ");\r\n"; 
  replaceString += "  m_list_modules.append(m_" + nom_module + ");\r\n";
  replaceString += "\r\n";
  replaceString += tagName; 
  cpp_file.replace(tagName, replaceString);

  // Ecrit le fichier (remplace l'ancien)
  writeFile(pathfilename, cpp_file);

  return(status);
}



// _____________________________________________________________________
/*!
*  Modifie certains fichiers du projet pour désintégrer un module
*  Après cette opération, les fichiers du projet sont nettoyé des
*  appels au module
*/
#define C_RIEN_DU_TOUT ""
bool CModuleDesigner::desintegrerModuleDuProjet(QString type_module,
                                            QString repertoire_projet,
                                            QString nom_module)
{
  bool status = true;
  QString pathfilename="";
  QString replaceString="";
 
  // _______________________________________________
  // Ouvre le fichier LaBotBox.pro et supprime les références au module
  pathfilename = repertoire_projet + "/LaBotBox.pro";
  QString pro_file = readFile(pathfilename); 
  if (pro_file == "") {
    m_application->m_print_view->print_error(this, "Fichier inexistant : " + pathfilename);
    return(false);
  }
  // Supprime le texte ajouté automatiquement à la création du module
  replaceString = "        " + nom_module + " \\ \r\n";
  pro_file.replace(replaceString, C_RIEN_DU_TOUT);
  m_application->m_print_view->print_info(this, pro_file);
  writeFile(pathfilename, pro_file);

  // _______________________________________________
  // Ouvre le fichier LaBotBox.h et supprime les références au module
  pathfilename = repertoire_projet + "/CLaBotBox.h";
  QString header_file = readFile(pathfilename); 
  if (header_file == "") {
    m_application->m_print_view->print_error(this, "Fichier inexistant : " + pathfilename);
    return(false);
  }
  // Supprime le texte ajouté automatiquement à la création du module
  replaceString = "class C" + nom_module + ";\r\n";
  header_file.replace(replaceString, C_RIEN_DU_TOUT);
  
  replaceString = "   C" + nom_module + "     *m_" + nom_module + ";\r\n";  
  header_file.replace(replaceString, C_RIEN_DU_TOUT);

  // Ecrit le fichier (remplace l'ancien)
  writeFile(pathfilename, header_file);

  // _______________________________________________
  // Ouvre le fichier LaBotBox.cpp et supprime les références au module
  pathfilename = repertoire_projet + "/CLaBotBox.cpp";
  QString cpp_file = readFile(pathfilename); 
  if (header_file == "") {
    m_application->m_print_view->print_error(this, "Fichier inexistant : " + pathfilename);
    return(false);
  }
  replaceString = "#include \"C" + nom_module + ".h\"\r\n";
  cpp_file.replace(replaceString, C_RIEN_DU_TOUT);

  // Supprime le texte ajouté automatiquement à la création du module
  replaceString =  "  m_" + nom_module + "     = new C" + nom_module + "(\"" + nom_module + "\");\r\n" ;
  replaceString += "  m_list_" + type_module.toLower() + "_modules.append(m_" + nom_module + ");\r\n"; 
  replaceString += "  m_list_modules.append(m_" + nom_module + ");\r\n";
  replaceString += "\r\n";
  cpp_file.replace(replaceString, C_RIEN_DU_TOUT);

  // Ecrit le fichier (remplace l'ancien)
  writeFile(pathfilename, cpp_file);

  return(status);
}



// _____________________________________________________________________
/*!
*  Lit le contenu d'un fichier
*
*/
QString CModuleDesigner::readFile(QString pathfilename)
{
  QFile file(pathfilename);
  if (file.open(QFile::ReadOnly) == NULL) {
      m_application->m_print_view->print_error(this, "Impossible d'ouvrir le fichier : " + pathfilename);
  }
  QTextStream txtstream(&file);
  txtstream.setCodec("ISO 8859-1");
  QString contenu = txtstream.readAll();
  file.close();
  return(contenu);
}


// _____________________________________________________________________
/*!
*  Ecrit dans un fichier le contenu de la string passée
*
*/
void CModuleDesigner::writeFile(QString path, QString filename, const QString &value)
{
  writeFile(path + "/" + filename, value);
}
// _____________________________________________________________________
/*!
*  Ecrit dans un fichier le contenu de la string passée
*
*/
void CModuleDesigner::writeFile(QString pathfilename, const QString &value)
{
  QFile file(pathfilename);
  if (file.open(QFile::WriteOnly) == NULL) {
      m_application->m_print_view->print_error(this, "Impossible d'ouvrir le fichier : " + pathfilename);
  }
  QTextStream txtstream(&file);
  txtstream.setCodec("ISO 8859-1");
  txtstream << value;
  file.close();
}


// _____________________________________________________________________
/*!
*  Choisit le répertoire dans lequel le module va être créé
*
*/
void CModuleDesigner::choixRepertoireSortie(void)
{
 QFileDialog dialog;
 QString rep = QFileDialog::getExistingDirectory(   &dialog, tr("Choix du répertoire de génération du module"),
                                                    m_ihm.ui.repertoire_projet->text(),
                                                    QFileDialog::ShowDirsOnly);
 if (rep != "") { m_ihm.ui.repertoire_projet->setText(rep); }
}




// ============ PAGE INTEGRATION D'UN MODULE EXISTANT =======================
// _____________________________________________________________________
/*!
*  Intègre un module déjà existant au projet
*
*/
bool CModuleDesigner::integrerModuleExistantAuProjet(void)
{
 QString rep_module_existant = m_ihm.ui.repertoire_module_existant->text();
 const QString SEPARATEUR = "/";
 // Extrait du répertoire : 
 // Le type de module : BASIC ou PLUGIN
 // Le nom du module
 // Le nom du répertoire du projet
 m_application->m_print_view->print_info(this, rep_module_existant);

 // Découpe le répertoire suivant le séparateur "/" (windows et linux)
 QStringList liste_rep = rep_module_existant.split(SEPARATEUR, QString::SkipEmptyParts);
 if (liste_rep.size() <= 2) { 
     m_application->m_print_view->print_error(this, "Repertoire du module invalide");
     return(false);
 }

 // Si le découpage s'est bien passé : 
 //     - Le dernier nom de la liste est le nom du module
 //     - L'avant dernier nom indique s'il s'agit d'un Basic ou Plugin module
 //     - Le nom du répertoire du projet s'obtient en recomposant les noms de la liste sans le dernier
 // Exemple : D:\workarea\LaBotBox\LabBotBox\BasicModules\DataView
 //           |-----------------------------|------------|-------------|
 //           |  nom du répertoire projet   | type module| nom module  |

 QString nom_module = liste_rep.at(liste_rep.size()-1);

 QString typeModuleString="";
 if (liste_rep.at(liste_rep.size()-2) == "BasicModules") { typeModuleString = "BASIC"; }
 else                                                   { typeModuleString = "PLUGIN"; }

 QString rep_projet = "";
 for (int i=0; i<liste_rep.size()-2; i++) {
    rep_projet += liste_rep.at(i) + SEPARATEUR;
 }

return(integrerModuleAuProjet(typeModuleString, rep_projet, nom_module));
}


// _____________________________________________________________________
/*!
*  Choisit le répertoire dans lequel le module va être créé
*
*/
void CModuleDesigner::choixRepertoireModuleExistant(void)
{
 QFileDialog dialog;
 QString rep = QFileDialog::getExistingDirectory(   &dialog, tr("Emplacement du module"),
                                                    m_ihm.ui.repertoire_module_existant->text(),
                                                    QFileDialog::ShowDirsOnly);
 if (rep != "") { m_ihm.ui.repertoire_module_existant->setText(rep); }
}




// ============ PAGE DESINTEGRATION D'UN MODULE DU PROJET =======================
bool CModuleDesigner::desintegrerModuleExistantDuProjet(void)
{
 QString rep_module_a_desintegrer = m_ihm.ui.repertoire_module_desintegration->text();
 const QString SEPARATEUR = "/";
 // Extrait du répertoire : 
 // Le type de module : BASIC ou PLUGIN
 // Le nom du module
 // Le nom du répertoire du projet
 m_application->m_print_view->print_info(this, rep_module_a_desintegrer);

 // Découpe le répertoire suivant le séparateur "/" (windows et linux)
 QStringList liste_rep = rep_module_a_desintegrer.split(SEPARATEUR, QString::SkipEmptyParts);
 if (liste_rep.size() <= 2) { 
     m_application->m_print_view->print_error(this, "Repertoire du module invalide");
     return(false);
 }

 // Si le découpage s'est bien passé : 
 //     - Le dernier nom de la liste est le nom du module
 //     - L'avant dernier nom indique s'il s'agit d'un Basic ou Plugin module
 //     - Le nom du répertoire du projet s'obtient en recomposant les noms de la liste sans le dernier
 // Exemple : D:\workarea\LaBotBox\LabBotBox\BasicModules\DataView
 //           |-----------------------------|------------|-------------|
 //           |  nom du répertoire projet   | type module| nom module  |

 QString nom_module = liste_rep.at(liste_rep.size()-1);

 QString typeModuleString="";
 if (liste_rep.at(liste_rep.size()-2) == "BasicModules") { typeModuleString = "BASIC"; }
 else                                                   { typeModuleString = "PLUGIN"; }

 QString rep_projet = "";
 for (int i=0; i<liste_rep.size()-2; i++) {
    rep_projet += liste_rep.at(i) + SEPARATEUR;
 }

return(desintegrerModuleDuProjet(typeModuleString, rep_projet, nom_module));
}



// _____________________________________________________________________
/*!
*  Choisit le répertoire dans lequel le module va être créé
*
*/
void CModuleDesigner::choixRepertoireModuleDesintegration(void)
{
 QFileDialog dialog;
 QString rep = QFileDialog::getExistingDirectory(   &dialog, tr("Emplacement du module"),
                                                    m_ihm.ui.repertoire_module_desintegration->text(),
                                                    QFileDialog::ShowDirsOnly);
 if (rep != "") { m_ihm.ui.repertoire_module_desintegration->setText(rep); }
}

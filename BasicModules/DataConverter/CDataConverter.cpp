/*! \file CDataConverter.cpp
 * A brief file description CPP.
 * A more elaborated file description.
 */
#include <QDebug>
#include <QQueue>
#include <QFileDialog>
#include "CDataConverter.h"
#include "CApplication.h"
#include "CPrintView.h"
#include "CMainWindow.h"
#include "CEEPROM.h"
#include "CDataManager.h"
#include "datatransformer.h"
#include "datatransformer_factory.h"
#include "csvParser.h"


/*! \addtogroup Module_DataConverter
   * 
   *  @{
   */
	   
// _____________________________________________________________________
/*!
*  Constructeur
*
*/
CDataConverter::CDataConverter(const char *plugin_name)
    :CBasicModule(plugin_name, VERSION_DataConverter, AUTEUR_DataConverter, INFO_DataConverter)
{
}


// _____________________________________________________________________
/*!
*  Destructeur
*
*/
CDataConverter::~CDataConverter()
{

}


// _____________________________________________________________________
/*!
*  Initialisation du module
*
*/
void CDataConverter::init(CApplication *application)
{
  CModule::init(application);
  setGUI(&m_ihm); // indique à la classe de base l'IHM

  // Gère les actions sur clic droit sur le panel graphique du module
  m_ihm.setContextMenuPolicy(Qt::CustomContextMenu);
  connect(&m_ihm, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(onRightClicGUI(QPoint)));
  
  // Restore la taille de la fenêtre
  QVariant val;
  val = m_application->m_eeprom->read(getName(), "geometry", QRect(50, 50, 150, 150));
  m_ihm.setGeometry(val.toRect());
  // Restore le fait que la fenêtre est visible ou non
  val = m_application->m_eeprom->read(getName(), "visible", QVariant(true));
  if (val.toBool()) { m_ihm.show(); }
  else              { m_ihm.hide(); }
  // Restore le niveau d'affichage
  val = m_application->m_eeprom->read(getName(), "niveau_trace", QVariant(MSG_TOUS));
  setNiveauTrace(val.toUInt());
  // Restore la couleur de fond
  val = m_application->m_eeprom->read(getName(), "background_color", QVariant(DEFAULT_MODULE_COLOR));
  setBackgroundColor(val.value<QColor>());

  // onglet 1
  connect(m_ihm.ui.transformer_list, SIGNAL(currentIndexChanged(int)), this, SLOT(transformer_selection_changed(int)));
  connect(m_ihm.ui.pb_delete_transformer, SIGNAL(clicked(bool)), this, SLOT(delete_transformer()));
  connect(m_ihm.ui.pb_delete_all_tranformers, SIGNAL(clicked(bool)), this, SLOT(delete_all_transformers()));

  // onglet 2
  connect(m_ihm.ui.actionLoad, SIGNAL(triggered(bool)), this, SLOT(loadTransformerListFromFile()));
  connect(m_ihm.ui.actionsave, SIGNAL(triggered(bool)), this, SLOT(saveTransformerListToFile()));
  connect(m_ihm.ui.pb_apply, SIGNAL(clicked(bool)), this, SLOT(apply_new_transformer()));
  connect(m_ihm.ui.pb_add_input, SIGNAL(clicked(bool)), this, SLOT(add_data_to_input()));
  connect(m_ihm.ui.pb_add_outuput, SIGNAL(clicked(bool)), this, SLOT(add_data_to_output()));
  connect(m_ihm.ui.filtre_noms_variables, SIGNAL(textChanged(QString)), this, SLOT(onDataFilterChanged(QString)));
  connect(m_ihm.ui.select_algo, SIGNAL(currentTextChanged(QString)), this, SLOT(algo_changed(QString)));

  connect(m_application->m_data_center, SIGNAL(dataCreated(CData*)), this, SLOT(refreshListeVariables()));

  // Recharge les transformers précédent
  val = m_application->m_eeprom->read(getName(), "reload_file_at_startup", QVariant(false));
  if (val.toBool()) {
      m_last_pathfilename = m_application->m_eeprom->read(getName(), "last_input_filename", QVariant("")).toString();
      if (m_last_pathfilename !="") loadTransformerListFromFile(m_last_pathfilename);
  }

  // met à jour la liste des algo possibles
  m_ihm.ui.select_algo->addItems(CDataTransformerFactory::getAvailableAlgo());

  refreshTransformerList();
}


// _____________________________________________________________________
/*!
*  Fermeture du module
*
*/
void CDataConverter::close(void)
{
  // Mémorise en EEPROM l'état de la fenêtre
  m_application->m_eeprom->write(getName(), "geometry", QVariant(m_ihm.geometry()));
  m_application->m_eeprom->write(getName(), "visible", QVariant(m_ihm.isVisible()));
  m_application->m_eeprom->write(getName(), "niveau_trace", QVariant((unsigned int)getNiveauTrace()));
  m_application->m_eeprom->write(getName(), "background_color", QVariant(getBackgroundColor()));
  if (m_last_pathfilename!="") m_application->m_eeprom->write(getName(), "last_input_filename", QVariant(m_last_pathfilename));
}

// _____________________________________________________________________
/*!
*  Création des menus sur clic droit sur la fenêtre du module
*
*/
void CDataConverter::onRightClicGUI(QPoint pos)
{
  QMenu *menu = new QMenu();

  menu->addAction("Select background color", this, SLOT(selectBackgroundColor()));
  menu->exec(m_ihm.mapToGlobal(pos));
}

// _____________________________________________________________________
/*!
*  Filtre les variables sur le nom
*/
void CDataConverter::onDataFilterChanged(QString filter_name)
{
    QStringList items_to_match;
    items_to_match = filter_name.toLower().simplified().split(" ");

    for (int i=0; i<m_ihm.ui.liste_variables->count(); i++) {
        m_ihm.ui.liste_variables->item(i)->setHidden(true);
        bool found = true;
        foreach (QString item_to_match, items_to_match) {
            if (!m_ihm.ui.liste_variables->item(i)->text().toLower().contains(item_to_match)) {
                found = false; // si tous les mots clés du filtre ne sont pas présents dans le nom du script, le script est rejeté
            }
        }
        m_ihm.ui.liste_variables->item(i)->setHidden(!found);
    }
}


// _____________________________________________________________________
/*!
 * \brief Applique le nouveau transformer et l'ajoute à la liste
 */
void CDataConverter::apply_new_transformer()
{
    QString input_data_list = m_ihm.ui.input_datanames->text();
    QString output_data = m_ihm.ui.output_dataname->text();
    QString refresh_method= "updated";
    if (m_ihm.ui.changed_method->isChecked()) refresh_method = "changed";
    QString algo = m_ihm.ui.select_algo->currentText();
    QString params = m_ihm.ui.params->text();
    QString description = m_ihm.ui.description->text();


    CDataTransformer *transformer = CDataTransformerFactory::createInstance(input_data_list,
                                                                            output_data,
                                                                            m_application->m_data_center,
                                                                            refresh_method,
                                                                            algo,
                                                                            params,
                                                                            description,
                                                                            this
                                                                            );
    if (transformer) m_transformer_list.append(transformer);
    refreshTransformerList();
}
// _____________________________________________________________________
/*!
 * \brief Ajoute la data pointée à la liste de data d'entrées du transformer
 * Les données d'entrées sont séparées par une ","
 */
void CDataConverter::add_data_to_input()
{
    if (m_ihm.ui.liste_variables->currentItem() == Q_NULLPTR) return; // cas où l'IHM ne pointe sur aucun élément de la liste
    QString current_text = m_ihm.ui.input_datanames->text();
    if ((current_text!="") && (!current_text.endsWith(","))) current_text +=",";
    m_ihm.ui.input_datanames->setText(current_text + m_ihm.ui.liste_variables->currentItem()->text());
}

// _____________________________________________________________________
/*!
 * \brief La data pointée devient la data de sortie
 */
void CDataConverter::add_data_to_output()
{
    if (m_ihm.ui.liste_variables->currentItem() == Q_NULLPTR) return; // cas où l'IHM ne pointe sur aucun élément de la liste
    m_ihm.ui.output_dataname->setText(m_ihm.ui.liste_variables->currentItem()->text());
}

// _____________________________________________________________________
void CDataConverter::algo_changed(QString algo)
{
    m_ihm.ui.params->setText(CDataTransformerFactory::getParamsTemplate(algo));
    m_ihm.ui.formula->setText(CDataTransformerFactory::getFormula(algo));
    m_ihm.ui.select_algo->setToolTip(CDataTransformerFactory::getHelp(algo));
}

// _____________________________________________________________________
/*!
*  Met à jour la liste des variables dans la fenêtre de gauche
*
*/
void CDataConverter::refreshListeVariables(void)
{
    QStringList var_list;
    m_application->m_data_center->getListeVariablesName(var_list);

    m_ihm.ui.liste_variables->clear();
    m_ihm.ui.liste_variables->addItems(var_list);
}


// _______________________________________________________
void CDataConverter::loadTransformerListFromFile()
{
    QString pathfilename = QFileDialog::getOpenFileName(&m_ihm, tr("Open File"), ".");
    if (pathfilename.isEmpty()) return;
    loadTransformerListFromFile(pathfilename);
}

// _______________________________________________________
/*!
 * \brief Charge un fichier csv à 6 colonnes (séparateur ";")
 *  1ère ligne contient la description des colonnes
 *  1 ligne par data à conversion de data à créer
 * Exemple de fichier d'entrée :
    DataIn;DataOut;Refresh method;Algo;Parameters;Description
    Toto_SRC;Toto_SINUS;updated;sin;{2.0, 1.0, 0.0};y=A*Sin(a*x+b)
    {Toto_SRC, Joystick0_Axis2};Toto_ADD2D;changed;addition2d;{1.0, 0.0, 4.0, 1.0};y=(a1*x1+b1)+(a2*x2+b2)
    Joystick0_Axis2;Joystick0_Axis2.MAX;updated;max;--;y=Max(x)
    Joystick0_Axis2;Joystick0_Axis2.MIN;updated;min;--;y=Min(x)
 * \param pathfilename le nom complet du fichier à ouvrir
 */
void CDataConverter::loadTransformerListFromFile(QString pathfilename)
{
    csvData data;
    csvParser parser;
    QStringList error_msg, warn_msg;

    parser.enableEmptyCells(false);
    parser.setMinimumNumberOfExpectedColums(6);
    parser.setSeparator(CSV_COLUMN_SEPARATOR);
    bool status = parser.parse(pathfilename, data, &error_msg, &warn_msg);
    if (status) {
        if (warn_msg.size() > 0) {
            qDebug() << "!! Warnings during parsing";
            foreach (QString msg, warn_msg) {
                qDebug() << "  -" << msg;
            }
        }
        else {
            m_ihm.ui.statusbar->showMessage("Parsing OK", 4000);
        }
        data.debug();
    }
    else {
        if (error_msg.size() > 0) {
            m_ihm.ui.statusbar->showMessage("ERRORS during parsing", 4000);
            foreach (QString msg, error_msg) {
                qDebug() << "  -" << msg;
            }
        }
        return; // pas la peine d'aller plus loin, le fichier n'est pas valide
    }

    // Créer un Transformer par ligne du fichier csv
    for (int line=0; line<data.m_datas.size(); line++) {
        QStringList data_line = data.m_datas.at(line);
        CDataTransformer *transformer = CDataTransformerFactory::createInstance(data_line.at(0),
                                                                                data_line.at(1),
                                                                                m_application->m_data_center,
                                                                                data_line.at(2),
                                                                                data_line.at(3),
                                                                                data_line.at(4),
                                                                                data_line.at(5),
                                                                                this
                                                                                );
        if (transformer) m_transformer_list.append(transformer);
    }
    refreshTransformerList();
    m_last_pathfilename = pathfilename;
}

// _______________________________________________________
/*!
 * \brief CDataConverter::saveTransformerListToFile
 */
void CDataConverter::saveTransformerListToFile()
{
    QString fileName = QFileDialog::getSaveFileName(&m_ihm, tr("Save File"), ".");
    if (fileName.isEmpty()) return;

    saveTransformerListToFile(fileName);
}

// _______________________________________________________
/*!
 * \brief Sauvegarde la liste des transformers dans un fichier texte
 * \param pathfilename le nom complet du fichier de sortie
 */
void CDataConverter::saveTransformerListToFile(QString pathfilename)
{
    QFile file;
    file.setFileName(pathfilename);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return;
    }
    QTextStream out(&file);

     out << "DataIn;DataOut;Refresh method;Algo;Parameters;Description" << CSV_LINE_SEPARATOR; // header
    foreach(CDataTransformer *transformer, m_transformer_list) {
        QString input_datas = transformer->getInputDataName().join(",");
        if (!input_datas.startsWith("{")) input_datas.prepend("{");
        if (!input_datas.endsWith("}")) input_datas.append("}");
        out << input_datas << CSV_COLUMN_SEPARATOR;

        out << transformer->getOutputDataName() << CSV_COLUMN_SEPARATOR;
        out << transformer->getEventConnection() << CSV_COLUMN_SEPARATOR;
        out << transformer->getShortAlgoName() << CSV_COLUMN_SEPARATOR;

        QString params = transformer->getParams();
        if (!params.startsWith("{")) params.prepend("{");
        if (!params.endsWith("}")) params.append("}");
        out << params << CSV_COLUMN_SEPARATOR;

        out << transformer->getDescription();
        out << CSV_LINE_SEPARATOR;
    }
    file.close();
}

// _______________________________________________________
void CDataConverter::refreshTransformerList()
{
    m_ihm.ui.description_transformer->clear();
    m_ihm.ui.transformer_list->clear();
    // Ajoute dans le combobox la liste des données de sortie
    for (int i=0; i<m_transformer_list.size(); i++) {
        m_ihm.ui.transformer_list->addItem(m_transformer_list[i]->getOutputDataName());
    }

    // Grise les boutons delete si la liste est vide
    if (m_transformer_list.size() > 0)  {
        m_ihm.ui.pb_delete_transformer->setEnabled(true);
        m_ihm.ui.pb_delete_all_tranformers->setEnabled(true);
    }
    else {
        m_ihm.ui.pb_delete_transformer->setEnabled(false);
        m_ihm.ui.pb_delete_all_tranformers->setEnabled(false);
    }
}

// _______________________________________________________
/*!
 * \brief Méthode appelée sur changement de sélection du Transformer
 * \param index l'index sélectionnée dans la liste
 * \remark affiche le détail du transformer sélectionné dans le champ dédié
 * \warning l'index du combobox est supposé être le même que l'index de la list m_transformer_list
 */
void CDataConverter::transformer_selection_changed(int index)
{
    if ((index<0) || (index > m_transformer_list.size())) return;

    QString str;
    str += QString("Input Data : %1").arg(m_transformer_list[index]->getInputDataName().join(", ")) + "\n";
    str += QString("Output Data : %1").arg(m_transformer_list[index]->getOutputDataName()) + "\n";
    str += QString("Algo : %1").arg(m_transformer_list[index]->getShortAlgoName()) + "\n";
    str += QString("Formula : %1").arg(m_transformer_list[index]->getFormula()) + "\n";
    str += QString("Help : %1").arg(m_transformer_list[index]->getHelp()) + "\n";
    str += QString("Parameters : %1").arg(m_transformer_list[index]->getParams()) + "\n";
    str += QString("Description : %1").arg(m_transformer_list[index]->getDescription());

    m_ihm.ui.description_transformer->setText(str);
}

// _______________________________________________________
/*!
 * Efface le transformer pointée sur l'IHM
 * \warning l'index du combobox est supposé être le même que l'index de la list m_transformer_list
 */
void CDataConverter::delete_transformer()
{
    int index = m_ihm.ui.transformer_list->currentIndex();
    delete_transformer(index);
}

// _______________________________________________________
/*!
 * Efface un transformer de la liste
 * \param index l'index de la liste m_transformer_list à supprimer
 */
void CDataConverter::delete_transformer(int index)
{
    if (index >= m_transformer_list.size()) return;

    CDataTransformer *transformer = m_transformer_list.at(index);
    if (transformer) delete transformer;
    m_transformer_list.remove(index);
    refreshTransformerList();
}

// _______________________________________________________
/*!
 * Efface tous les transformers de la liste
 */
void CDataConverter::delete_all_transformers()
{
    while (m_transformer_list.size()) {
        delete_transformer(0);
    }
}


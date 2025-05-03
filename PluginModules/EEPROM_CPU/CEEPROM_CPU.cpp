/*! \file CEEPROM_CPU.cpp
 * A brief file description CPP.
 * A more elaborated file description.
 */
#include <QDebug>
#include <QFileDialog>
#include <QProgressDialog>
#include "CEEPROM_CPU.h"
#include "CApplication.h"
#include "CPrintView.h"
#include "CMainWindow.h"
#include "CEEPROM.h"
#include "CDataManager.h"
#include "CMessagerieBot.h"
#include "CTrameFactory.h"
#include "csvParser.h"


/*! \addtogroup Module_EEPROM_CPU
   * 
   *  @{
   */
	   
// _____________________________________________________________________
/*!
*  Constructeur
*
*/
CEEPROM_CPU::CEEPROM_CPU(const char *plugin_name)
    :CPluginModule(plugin_name, VERSION_EEPROM_CPU, AUTEUR_EEPROM_CPU, INFO_EEPROM_CPU)
{
}


// _____________________________________________________________________
/*!
*  Destructeur
*
*/
CEEPROM_CPU::~CEEPROM_CPU()
{

}


// _____________________________________________________________________
/*!
*  Initialisation du module
*
*/
void CEEPROM_CPU::init(CApplication *application)
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

  val = m_application->m_eeprom->read(getName(), "automatic_load_mapping", true);
  m_automatic_load_mapping = val.toBool();

  val = m_application->m_eeprom->read(getName(), "eep_mapping_pathfilename", "");
  m_eep_mapping_pathfilename= val.toString();

  connect(m_ihm.ui.pb_read_eeprom, SIGNAL(clicked(bool)), this, SLOT(read_eeprom_to_table()));
  connect(m_ihm.ui.actionReadEEPROM, SIGNAL(triggered(bool)), this, SLOT(read_eeprom_to_table()));
  connect(m_ihm.ui.actionWriteEEPROM, SIGNAL(triggered(bool)), this, SLOT(write_table_to_eeprom()));

  connect(m_ihm.ui.actionImportMapping, SIGNAL(triggered(bool)), this, SLOT(import_mapping()));
  connect(m_ihm.ui.actionExportMapping, SIGNAL(triggered(bool)), this, SLOT(export_mapping()));

  connect(m_ihm.ui.actionImportValues, SIGNAL(triggered(bool)), this, SLOT(import_values()));
  connect(m_ihm.ui.actionExportValues, SIGNAL(triggered(bool)), this, SLOT(export_values()));


  CTrameBot *trame = m_application->m_MessagerieBot->getTrameFactory()->getTrameFromID(ID_EEPROM_VALUE);
  CTrame_EEPROM_VALUE *trame_eeprom_req = (CTrame_EEPROM_VALUE *)trame;
  connect(trame_eeprom_req, SIGNAL(receive_value(unsigned long, unsigned long)), this, SLOT(onRxValue(unsigned long, unsigned long)));

  // Menu clic droit sur la table de valeurs
  m_ihm.ui.table_eeprom->setContextMenuPolicy(Qt::CustomContextMenu);
  connect(m_ihm.ui.table_eeprom, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(onRightClicTable(QPoint)));

  connect(m_ihm.ui.actionSave, SIGNAL(triggered(bool)), this, SLOT(save_table()));
  connect(m_ihm.ui.actionOpen, SIGNAL(triggered(bool)), this, SLOT(open_table()));
  connect(m_ihm.ui.actionCleanTable, SIGNAL(triggered(bool)), this, SLOT(clean()));

  connect(m_ihm.ui.filter_txt, SIGNAL(textChanged(QString)), this, SLOT(onDataFilterChanged(QString)));


  import_mapping(m_eep_mapping_pathfilename);
}


// _____________________________________________________________________
/*!
*  Fermeture du module
*
*/
void CEEPROM_CPU::close(void)
{
  // Mémorise en EEPROM l'état de la fenêtre
  m_application->m_eeprom->write(getName(), "geometry", QVariant(m_ihm.geometry()));
  m_application->m_eeprom->write(getName(), "visible", QVariant(m_ihm.isVisible()));
  m_application->m_eeprom->write(getName(), "niveau_trace", QVariant((unsigned int)getNiveauTrace()));
  m_application->m_eeprom->write(getName(), "background_color", QVariant(getBackgroundColor()));
}

// _____________________________________________________________________
/*!
*  Création des menus sur clic droit sur la fenêtre du module
*
*/
void CEEPROM_CPU::onRightClicGUI(QPoint pos)
{
  QMenu *menu = new QMenu();

  menu->addAction("Select background color", this, SLOT(selectBackgroundColor()));
  menu->exec(m_ihm.mapToGlobal(pos));
}

// _____________________________________________________________________
/*!
 * \brief Filtre de recherche sur le nom des variables
 * \param filter_name le filtre
 */
void CEEPROM_CPU::onDataFilterChanged(QString filter_name)
{
    QStringList items_to_match;
    items_to_match = filter_name.toLower().simplified().split(" ");

    for (int i=0; i<m_ihm.ui.table_eeprom->rowCount(); i++) {
        m_ihm.ui.table_eeprom->hideRow(i);
        bool found = true;
        foreach (QString item_to_match, items_to_match) {
            if (!m_ihm.ui.table_eeprom->item(i, COL_NAME)->text().trimmed().toLower().contains(item_to_match)) {
                found = false; // si tous les mots clés du filtre ne sont pas présents dans le nom du script, le script est rejeté
            }
        }
        if (found) m_ihm.ui.table_eeprom->showRow(i);
    }
    m_ihm.ui.table_eeprom->resizeColumnsToContents();
}

// _____________________________________________________________________
/*!
 * \brief Nettoie toute la table
 */
void CEEPROM_CPU::clean()
{
    m_ihm.ui.table_eeprom->clearContents();
    m_ihm.ui.table_eeprom->setRowCount(0);
}

// _____________________________________________________________________
void CEEPROM_CPU::onRxValue(unsigned long address, unsigned long value)
{
    qDebug() << address << value;
    m_ihm.statusBar()->showMessage(QString("Receive @%1=%2").arg(address).arg(value), 1000);

    // 1. Vérifie si la ligne n'existe pas déjà, si c'est le cas, il suffit de la mettre à jour
    for (int row=0; row<m_ihm.ui.table_eeprom->rowCount(); row++) {
      unsigned long current_add = m_ihm.ui.table_eeprom->item(row, COL_ADDRESS)->text().toUInt();
      if (current_add == address) { // met à jour la ligne existante
          QComboBox *box = (QComboBox*)(m_ihm.ui.table_eeprom->cellWidget(row, COL_TYPE));
          QString type_str = box->currentText();
          m_ihm.ui.table_eeprom->item(row, COL_VALUE)->setText( value2Display(value, type_str) );
          m_ihm.ui.table_eeprom->resizeColumnsToContents();
          return; // pas la peine d'aller plus loin
      }
    }

    //   La ligne n'existe pas -> la crée avec le format d'affichage par défaut hex
    createLine(address, "", value2Display(value, "hex"), "");
}

// _____________________________________________________________________
/*!
 * \brief Charge le fichier de mapping de l'EEPROMM
 * \param pathfilename le chemin + nom du fichier de mapping
 * \remark le format du fichier de mappint est un .csv 3 colonnes :
 *      Adresse;Nom;Type
 */
void CEEPROM_CPU::import_mapping(QString pathfilename)
{
    csvData data;
    csvParser parser;
    QStringList err_msg;

    parser.setNumberOfExpectedColums(3);
    if (!parser.parse(pathfilename, data, &err_msg)) {
        qDebug() << err_msg;
        m_application->m_print_view->print_error(this, err_msg.join('\n'));
        return;
    }

    //qDebug() << data.m_header;               // display columnn description if exists
    //qDebug() << data.getColumnsCount();      // display the number of columns detected
    //qDebug() << data.getLinesCount();        // display the number of lines detected
    for (int line=0; line<data.m_datas.size(); line++) {
        QStringList data_line = data.m_datas.at(line);
        for (int column=0; column<data_line.size(); column++) {
            //qDebug() << data_line.at(column);
        }
        apply_mapping(data_line.at(0).toULong(), data_line.at(1), data_line.at(2));
    }
}

// _____________________________________________________________________
void CEEPROM_CPU::import_mapping()
{
    QString pathfilename = QFileDialog::getOpenFileName(&m_ihm, tr("Open File"), ".csv");
    if (pathfilename.isEmpty()) return;

    import_mapping(pathfilename);
}

// _____________________________________________________________________
/*!
 * \brief Sauvegarde le mapping mémoire dans un fichier .csv 3 colonnes
 */
void CEEPROM_CPU::export_mapping()
{
    QString pathfilename = QFileDialog::getSaveFileName(&m_ihm, tr("Save File"), ".csv");
    if (pathfilename.isEmpty()) return;

    QFile file;
    file.setFileName(pathfilename);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return;
    }
    QTextStream out(&file);
    out << "Adress;Name;Type" << LINE_SEPARATOR;
    for (int row=0; row<m_ihm.ui.table_eeprom->rowCount(); row++) {
        out << tableIndex2Adress(row)   << CSV_SEPARATOR
            << tableIndex2Name(row)     << CSV_SEPARATOR
            << tableIndex2Type(row)
            << LINE_SEPARATOR;
    }
    file.close();
}

// _____________________________________________________________________
/*!
 * \brief Charge les valeurs depuis un fichier .csv 2 colonnes : adress;valeur
 */
void CEEPROM_CPU::import_values()
{
    QString pathfilename = QFileDialog::getOpenFileName(&m_ihm, tr("Open File"), ".csv");
    if (pathfilename.isEmpty()) return;

    csvData data;
    csvParser parser;
    QStringList err_msg;

    parser.setNumberOfExpectedColums(2);
    if (!parser.parse(pathfilename, data, &err_msg)) {
        qDebug() << err_msg;
        m_application->m_print_view->print_error(this, err_msg.join('\n'));
        return;
    }

    //qDebug() << data.m_header;               // display columnn description if exists
    //qDebug() << data.getColumnsCount();      // display the number of columns detected
    //qDebug() << data.getLinesCount();        // display the number of lines detected
    for (int line=0; line<data.m_datas.size(); line++) {
        QStringList data_line = data.m_datas.at(line);
        for (int column=0; column<data_line.size(); column++) {
            //qDebug() << data_line.at(column);
        }
        apply_value(data_line.at(0), data_line.at(1));
    }
}

// _____________________________________________________________________
/*!
 * \brief Exporte les valeurs dans un fichier .csv 2 colonnes : name;value
 */
void CEEPROM_CPU::export_values()
{
    QString pathfilename = QFileDialog::getSaveFileName(&m_ihm, tr("Save File"), ".csv");
    if (pathfilename.isEmpty()) return;

    QFile file;
    file.setFileName(pathfilename);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return;
    }
    QTextStream out(&file);
    out << "Name;Value" << LINE_SEPARATOR;
    for (int row=0; row<m_ihm.ui.table_eeprom->rowCount(); row++) {
        out << tableIndex2Name(row)     << CSV_SEPARATOR
            << tableIndex2Value(row)
            << LINE_SEPARATOR;
    }
    file.close();
}

// _____________________________________________________________________
void CEEPROM_CPU::onRightClicTable(QPoint pos)
{
    QMenu *menu = new QMenu();

    menu->addAction("Clean", this, SLOT(clean()));
    menu->addAction("Read selected", this, SLOT(read_selected_index_to_table()));
    menu->addAction("Write selected", this, SLOT(write_selected_index_to_table()));
    menu->exec(m_ihm.ui.table_eeprom->mapToGlobal(pos));
}

// _____________________________________________________________________
void CEEPROM_CPU::save_table()
{
    QString pathfilename = QFileDialog::getSaveFileName(&m_ihm, tr("Save File"), ".csv");
    if (pathfilename.isEmpty()) return;

    QFile file;
    file.setFileName(pathfilename);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return;
    }
    QTextStream out(&file);
    out << "Adress;Name;Value;Type" << LINE_SEPARATOR;
    for (int row=0; row<m_ihm.ui.table_eeprom->rowCount(); row++) {
        out << tableIndex2Adress(row)   << CSV_SEPARATOR
            << tableIndex2Name(row)     << CSV_SEPARATOR
            << tableIndex2Value(row)    << CSV_SEPARATOR
            << tableIndex2Type(row)
            << LINE_SEPARATOR;
    }
    file.close();
}

// _____________________________________________________________________
/*!
 * \brief Charge un fichier .csv 4 colonnes : Adress;Name;Value;Type
 *
 */
void CEEPROM_CPU::open_table()
{
    QString pathfilename = QFileDialog::getOpenFileName(&m_ihm, tr("Open File"), ".csv");
    if (pathfilename.isEmpty()) return;

    // efface la table
    clean();

    csvData data;
    csvParser parser;
    QStringList err_msg;

    parser.setNumberOfExpectedColums(4);
    parser.enableEmptyCells(true);
    if (!parser.parse(pathfilename, data, &err_msg)) {
        qDebug() << err_msg;
        m_application->m_print_view->print_error(this, err_msg.join('\n'));
        return;
    }

    //qDebug() << data.m_header;               // display columnn description if exists
    //qDebug() << data.getColumnsCount();      // display the number of columns detected
    //qDebug() << data.getLinesCount();        // display the number of lines detected
    for (int line=0; line<data.m_datas.size(); line++) {
        QStringList data_line = data.m_datas.at(line);
        for (int column=0; column<data_line.size(); column++) {
            //qDebug() << data_line.at(column);
        }
        createLine(data_line.at(0).toULong(), data_line.at(1), data_line.at(2), data_line.at(3));
    }
}

// _____________________________________________________________________
/*!
 * \brief Lit toute l'EEPROM et affiche les informations dans la table
 */
void CEEPROM_CPU::read_eeprom_to_table()
{
    clean(); // commence par effacer la table
    import_mapping(m_eep_mapping_pathfilename);  // charge le mapping
    CTrameBot *trame = m_application->m_MessagerieBot->getTrameFactory()->getTrameFromID(ID_READ_EEPROM_REQ);
    CTrame_READ_EEPROM_REQ *trame_eeprom_req = (CTrame_READ_EEPROM_REQ *)trame;
    trame_eeprom_req->start_address = 0;
    trame_eeprom_req->count = 0xFFFF; // valeur maximum possible (c'est le CPU qui limitera l'envoi aux données réellement existantes)
    trame->Encode();
}

// _____________________________________________________________________
/*!
 * \brief Ecrit la table vers l'EEPROM
 */
void CEEPROM_CPU::write_table_to_eeprom()
{
    QProgressDialog progress("Send parameters...", QString(), 0, m_ihm.ui.table_eeprom->rowCount(), &m_ihm);

    QApplication::setOverrideCursor(Qt::WaitCursor);
    for (int row=0; row<m_ihm.ui.table_eeprom->rowCount(); row++) {
        progress.setWindowModality(Qt::WindowModal);
        progress.setValue(row);
        write_1_value(row);
        delay_between_cpu_commands();
    }
    QApplication::restoreOverrideCursor();
    progress.setValue(m_ihm.ui.table_eeprom->rowCount());
}

// _____________________________________________________________________
/*!
 * \brief Lit les valeurs EEPROM de chaque adresse sélectionnée
 */
void CEEPROM_CPU::read_selected_index_to_table()
{
    for (int row=0; row<m_ihm.ui.table_eeprom->rowCount(); row++) {
        QTableWidgetItem *item = m_ihm.ui.table_eeprom->item(row, COL_ADDRESS);
        if (item->isSelected()) {
            read_1_value(item->row());
            delay_between_cpu_commands();
        }
    }
}

// _____________________________________________________________________
/*!
 * \brief Ecrit les valeurs EEPROM de chaque adresse sélectionnée
 */
void CEEPROM_CPU::write_selected_index_to_table()
{
    for (int row=0; row<m_ihm.ui.table_eeprom->rowCount(); row++) {
        QTableWidgetItem *item = m_ihm.ui.table_eeprom->item(row, COL_ADDRESS);
        if (item->isSelected()) {
            write_1_value(item->row());
            delay_between_cpu_commands();
        }
    }
}

// _____________________________________________________________________
/*!
 * \brief Crée la ligne de mapping (où la met à jour si elle existe)
 * \param address
 * \param name
 * \param type
 */
void CEEPROM_CPU::apply_mapping(unsigned long address, QString name, QString type)
{
    // 1. Vérifie si la ligne n'existe pas déjà, si c'est le cas, il suffit de la mettre à jour
    for (int row=0; row<m_ihm.ui.table_eeprom->rowCount(); row++) {
      unsigned long current_add = m_ihm.ui.table_eeprom->item(row, COL_ADDRESS)->text().toUInt();
      if (current_add == address) { // met à jour la ligne existante
          m_ihm.ui.table_eeprom->itemAt(row, COL_NAME)->setText(name);
          m_ihm.ui.table_eeprom->setCellWidget(row, COL_TYPE, type2Combobox(type));
          m_ihm.ui.table_eeprom->resizeColumnsToContents();
          return; // pas la peine d'aller plus loin
      }
    }

    // 2. Si on arrive ici, c'est que la ligne n'a pas été trouvée -> la crée avec les infos connues
    createLine( address,
                name,
                QString("?"),
                type);
}

// _____________________________________________________________________
/*!
 * \brief Met à jour la table avec la valeur de la variable si elle existe
 * \param name le nom de la variable
 * \param value sa valeur
 */
void CEEPROM_CPU::apply_value(QString name, QString value)
{
    // Vérifie si la variable existe, si c'est le cas, il suffit de la mettre à jour le champ valeur
    for (int row=0; row<m_ihm.ui.table_eeprom->rowCount(); row++) {
        QString current_name = m_ihm.ui.table_eeprom->item(row, COL_NAME)->text().toLower();
        if (current_name == name.toLower()) { // met à jour la ligne existante
            m_ihm.ui.table_eeprom->item(row, COL_VALUE)->setText(value);
            m_ihm.ui.table_eeprom->resizeColumnsToContents();
            return; // pas la peine d'aller plus loin, on a trouvé
        }
    }
}



// _____________________________________________________________________
/*!
 * \brief Crée une list Combobox et lui affecte la valeur correspondant au type
 * \param type le type
 * \return
 */
QWidget* CEEPROM_CPU::type2Combobox(QString type)
{
    QComboBox* _list = new QComboBox();
    _list->addItem("?");
    _list->addItem("UINT32");
    _list->addItem("INT32");
    _list->addItem("FLOAT");
    _list->addItem("HEX");
    _list->addItem("BIN");
    _list->setEnabled(false);
    QString type_simplified = type.simplified().toLower();
    if (type_simplified == "uint32")        _list->setCurrentIndex(1);
    else if (type_simplified == "int32")    _list->setCurrentIndex(2);
    else if (type_simplified == "float")    _list->setCurrentIndex(3);
    else if (type_simplified == "hex")      _list->setCurrentIndex(4);
    else if (type_simplified == "bin")      _list->setCurrentIndex(5);
    else                                    _list->setCurrentIndex(0);

    return _list;
}


// _____________________________________________________________________
/*!
 * \brief Crée une nouvelle ligne en fin de tableau
 * \param address
 * \param name
 * \param value
 * \param type
 */
void CEEPROM_CPU::createLine(unsigned long address, QString name, QString value, QString type)
{
    int new_row = m_ihm.ui.table_eeprom->rowCount();
    m_ihm.ui.table_eeprom->setRowCount(new_row+1);
    QTableWidgetItem *newItem = new QTableWidgetItem();
    m_ihm.ui.table_eeprom->setItem(new_row, COL_ADDRESS, newItem);
    newItem->setText(QString("%1").arg(address, 4, 10, QChar('0')));
    newItem->setTextAlignment(Qt::AlignCenter);

    newItem = new QTableWidgetItem();
    m_ihm.ui.table_eeprom->setItem(new_row, COL_NAME, newItem);
    newItem->setText(QString("%1").arg(name));
    newItem->setTextAlignment(Qt::AlignCenter);

    newItem = new QTableWidgetItem();
    m_ihm.ui.table_eeprom->setItem(new_row, COL_VALUE, newItem);
    newItem->setText(value);
    newItem->setTextAlignment(Qt::AlignCenter);

    newItem = new QTableWidgetItem();
    m_ihm.ui.table_eeprom->setItem(new_row, COL_TYPE, newItem);
    m_ihm.ui.table_eeprom->setCellWidget(new_row, 3, (QWidget*)type2Combobox(type));

    m_ihm.ui.table_eeprom->resizeColumnsToContents();
}

// _____________________________________________________________________
/*!
 * \brief Crée un texte à afficher dans la bonne base et dans le bon format
 * \param value la valeur d'entrée (4 octets)
 * \param type_str le type de représentation au format chaine de caractère
 * \return la chaine représentant la valeur dans le bon format
 */
QString CEEPROM_CPU::value2Display(unsigned long value, QString type_str)
{
    QString ret;
    uEE eep;
    eep.ulval = value;
    type_str = type_str.simplified().toLower();

    if (type_str == "uint32")       ret = QString("%1").arg(eep.ulval);
    else if (type_str == "int32")   ret = QString("%1").arg(eep.sval);
    else if (type_str == "float")   ret = QString("%1").arg(eep.fval);
    else if (type_str == "bin")     ret = QString("0b%1").arg(eep.ulval, 32, 2, QChar('0'));
    else                            ret = QString("0x%1").arg(eep.ulval, 8, 16, QChar('0'));

    return ret;
}

// _____________________________________________________________________
void CEEPROM_CPU::read_1_value(unsigned long address)
{
    CTrameBot *trame = m_application->m_MessagerieBot->getTrameFactory()->getTrameFromID(ID_READ_EEPROM_REQ);
    CTrame_READ_EEPROM_REQ *trame_eeprom_req = (CTrame_READ_EEPROM_REQ *)trame;
    trame_eeprom_req->start_address = address;
    trame_eeprom_req->count = 1;
    trame->Encode();
}

// _____________________________________________________________________
void CEEPROM_CPU::write_1_value(unsigned long address, unsigned long value)
{
    CTrameBot *trame = m_application->m_MessagerieBot->getTrameFactory()->getTrameFromID(ID_WRITE_EEPROM_REQ);
    CTrame_WRITE_EEPROM_REQ *trame_eeprom_req = (CTrame_WRITE_EEPROM_REQ *)trame;
    trame_eeprom_req->address = address;
    trame_eeprom_req->value = value;
    trame->Encode();
}

// _____________________________________________________________________
/*!
 * \brief Envoie vers l'EEPROM la valeur de la table pointée par l'index
 * \param table_index l'index de la table
 * \remark contrôle la convertion string -> valeur numérique avant d'envoyer une mauvaise valeur au CPU
 */
void CEEPROM_CPU::write_1_value(unsigned long table_index)
{
    bool ok;
    unsigned long address = tableIndex2Adress(table_index);
    unsigned long raw_value = tableIndex2RawValue(table_index, &ok);
    if (ok) write_1_value(address, raw_value); // pas d'envoi si la conversion n'a pas aboutit
}

// _____________________________________________________________________
/*!
 * \brief Convertit la valeur de la table en une valeur brute unsigned long prêt à envoyer
 * \param table_index
 * \return
 */
unsigned long CEEPROM_CPU::tableIndex2RawValue(unsigned long table_index, bool *ok)
{
    QString type_str = tableIndex2Type(table_index).simplified().toLower();
    QString val_str = tableIndex2Value(table_index);
    uEE eeval;
    bool _ok;

    if (type_str == "uint32")       eeval.ulval = val_str.toUInt(&_ok);
    else if (type_str == "int32")   eeval.sval = val_str.toInt(&_ok);
    else if (type_str == "float")   eeval.fval = val_str.toFloat(&_ok);
    else if (type_str == "bin")     eeval.ulval = val_str.remove("0b").toUInt(&_ok, 2);  // il faut supprimer le "0b" car non reconnu par la conversion "toUInt"
    // hex et cas par défaut
    else                            eeval.ulval = val_str.toUInt(&_ok, 16);

    if (ok) *ok = _ok;

    return eeval.ulval;
}

// _____________________________________________________________________
/*!
 * \brief Récupère l'adresse EEPROM à partir de la ligne de la table
 * \param table_index numéro de ligne de la table
 * \return adresse EEPROM correspondante
 */
QString CEEPROM_CPU::tableIndex2Value(unsigned long table_index)
{
    QString val;
    QTableWidgetItem *item = m_ihm.ui.table_eeprom->item(table_index, COL_VALUE);
    if (item) val = item->text();
    return val;
}

// _____________________________________________________________________
/*!
 * \brief Récupère le nom de la donnéeà partir de la ligne de la table
 * \param table_index numéro de ligne de la table
 * \return le nom
 */
QString CEEPROM_CPU::tableIndex2Name(unsigned long table_index)
{
    QString val;
    QTableWidgetItem *item = m_ihm.ui.table_eeprom->item(table_index, COL_NAME);
    if (item) val = item->text();
    return val;
}

// _____________________________________________________________________
/*!
 * \brief Récupère l'adresse EEPROM à partir de la ligne de la table
 * \param table_index numéro de ligne de la table
 * \return adresse EEPROM correspondante
 */
unsigned long CEEPROM_CPU::tableIndex2Adress(unsigned long table_index)
{
    unsigned long val = 0;
    QTableWidgetItem *item = m_ihm.ui.table_eeprom->item(table_index, COL_ADDRESS);
    if (item) val = item->text().toInt();
    return val;
}

// _____________________________________________________________________
QString CEEPROM_CPU::tableIndex2Type(unsigned long table_index)
{
    QString type_str;
    QComboBox *box = (QComboBox*)(m_ihm.ui.table_eeprom->cellWidget(table_index, COL_TYPE));
    if (box) type_str = box->currentText();
    return type_str;
}

// _____________________________________________________________________
/*!
 * \brief Crée un délai utilisé entre 2 envois de commands au CPU
 * \param delay_msec valeur du délai en msec
 */
void CEEPROM_CPU::delay_between_cpu_commands(unsigned int delay_msec)
{
    QTime dieTime= QTime::currentTime().addMSecs(delay_msec);
    while (QTime::currentTime() < dieTime)
        QCoreApplication::processEvents(QEventLoop::AllEvents, 100);  // s'assure de rendre la main à la boucle infinie pourque les messages RS232 soient gérés en lecture et écriture
}

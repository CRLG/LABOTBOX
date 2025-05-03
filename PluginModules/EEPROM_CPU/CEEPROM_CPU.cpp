/*! \file CEEPROM_CPU.cpp
 * A brief file description CPP.
 * A more elaborated file description.
 */
#include <QDebug>
#include "CEEPROM_CPU.h"
#include "CApplication.h"
#include "CPrintView.h"
#include "CMainWindow.h"
#include "CEEPROM.h"
#include "CDataManager.h"
#include "CMessagerieBot.h"
#include "CTrameFactory.h"
#include "csvParser.h"


/*! \addtogroup Module_Test2
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

  connect(m_ihm.ui.pb_read_eeprom, SIGNAL(clicked(bool)), this, SLOT(onTest()));

  CTrameBot *trame = m_application->m_MessagerieBot->getTrameFactory()->getTrameFromID(ID_EEPROM_VALUE);
  CTrame_EEPROM_VALUE *trame_eeprom_req = (CTrame_EEPROM_VALUE *)trame;
  connect(trame_eeprom_req, SIGNAL(receive_value(unsigned long, unsigned long)), this, SLOT(onRxValue(unsigned long, unsigned long)));

  // Menu clic droit sur la table de valeurs
  m_ihm.ui.table_eeprom->setContextMenuPolicy(Qt::CustomContextMenu);
  connect(m_ihm.ui.table_eeprom, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(onRightClicTable(QPoint)));

  connect(m_ihm.ui.filter_txt, SIGNAL(textChanged(QString)), this, SLOT(onDataFilterChanged(QString)));

  load_mapping(m_eep_mapping_pathfilename);
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
 * \brief Filtre sur le nom des variables
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
}


// _____________________________________________________________________
void CEEPROM_CPU::onTest()
{

    CTrameBot *trame = m_application->m_MessagerieBot->getTrameFactory()->getTrameFromID(ID_READ_EEPROM_REQ);
    CTrame_READ_EEPROM_REQ *trame_eeprom_req = (CTrame_READ_EEPROM_REQ *)trame;
    trame_eeprom_req->start_address = 0;
    trame_eeprom_req->count = 100;
    trame->Encode();

/*
    // cree les elements de la table
    m_ihm.ui.table_eeprom->setRowCount(9);
    for (int row=0; row<LidarUtils::NBRE_MAX_OBSTACLES; row++) {
        QTableWidgetItem *newItem = new QTableWidgetItem();
        m_ihm.ui.table_eeprom->setItem(row, 0, newItem);
        newItem->setText(QString("%1.0").arg(row));

        newItem = new QTableWidgetItem();
        m_ihm.ui.table_eeprom->setItem(row, 1, newItem);
        newItem->setText(QString("%1.1").arg(row));

        QComboBox* _list = new QComboBox();
              _list->addItem("UINT32");
              _list->addItem("INT32");
              _list->addItem("FLOAT");
              m_ihm.ui.table_eeprom->setCellWidget(row, 3, (QWidget*)_list);
    }
*/

}

// _____________________________________________________________________
void CEEPROM_CPU::onRxValue(unsigned long address, unsigned long value)
{
    qDebug() << address << value;

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
void CEEPROM_CPU::load_mapping(QString pathfilename)
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

    qDebug() << data.m_header;               // display columnn description if exists
    qDebug() << data.getColumnsCount();      // display the number of columns detected
    qDebug() << data.getLinesCount();        // display the number of lines detected
    for (int line=0; line<data.m_datas.size(); line++) {
        QStringList data_line = data.m_datas.at(line);
        for (int column=0; column<data_line.size(); column++) {
            qDebug() << data_line.at(column);
        }
        apply_mapping(data_line.at(0).toULong(), data_line.at(1), data_line.at(2));
    }
}

// _____________________________________________________________________
void CEEPROM_CPU::onRightClicTable(QPoint pos)
{
    //QMenu *menu = new QMenu();

    QTableWidgetItem *item = m_ihm.ui.table_eeprom->itemAt(pos);
    if (item) {
        qDebug() << "Clic sur" << item->row()<< item->column();
        unsigned long address = m_ihm.ui.table_eeprom->item(item->row(), COL_ADDRESS)->text().toInt();
        //read_1_value(m_ihm.ui.table_eeprom->item(item->row(), COL_ADDRESS)->text().toInt());
        //uEE eeval;
        //eeval.fval = 2.345;
        write_1_value(tableIndex2Adress(item->row()), tableIndex2RawValue(address));
    }

    //menu->addAction("Read", this, SLOT(selectBackgroundColor()));
    //menu->addAction("Write", this, SLOT(selectBackgroundColor()));
    //menu->exec(m_ihm.mapToGlobal(pos));

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
 * \brief Convertit la valeur de la table en une valeur brute unsigned long prêt à envoyer
 * \param table_index
 * \return
 */
unsigned long CEEPROM_CPU::tableIndex2RawValue(unsigned long table_index)
{
    QString type_str = tableIndex2Type(table_index).simplified().toLower();
    QString val_str = tableIndex2Value(table_index);
    uEE eeval;

    if (type_str == "uint32")       eeval.ulval = val_str.toUInt();
    else if (type_str == "int32")   eeval.sval = val_str.toInt();
    else if (type_str == "float")   eeval.fval = val_str.toFloat();
    else if (type_str == "bin")     eeval.ulval = val_str.remove("0b").toUInt(nullptr, 2);  // il faut supprimer le "0b" car non reconnu par la conversion "toUInt"
    // hex et cas par défaut
    else                            eeval.ulval = val_str.toUInt(nullptr, 16);

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


// TODO :
//   Trie du tableau par nom
//   Trie du tableau par adresse
//   Ajout d'un filtre de recherche de nom

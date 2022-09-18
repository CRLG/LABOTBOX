/*! \file CDataExchanger.cpp
 * A brief file description CPP.
 * A more elaborated file description.
 */
#include <QDebug>
#include <QFileDialog>

#include "CDataExchanger.h"
#include "CApplication.h"
#include "CPrintView.h"
#include "CMainWindow.h"
#include "CEEPROM.h"
#include "CDataManager.h"
#include "CDataListOpenSave.h"



/*! \addtogroup Module_DataExchanger
   *
   *  @{
   */

// _____________________________________________________________________
/*!
*  Constructeur
*
*/
CDataExchanger::CDataExchanger(const char *plugin_name)
    :CBasicModule(plugin_name, VERSION_DataExchanger, AUTEUR_DataExchanger, INFO_DataExchanger),
      m_server(Q_NULLPTR),
      m_client(Q_NULLPTR),
      m_exchanger(Q_NULLPTR)
{
}


// _____________________________________________________________________
/*!
*  Destructeur
*
*/
CDataExchanger::~CDataExchanger()
{
    if (m_client) delete m_client;
    if (m_server) delete m_server;
    m_server->deleteLater();
}


// _____________________________________________________________________
/*!
*  Initialisation du module
*
*/
void CDataExchanger::init(CApplication *application)
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

    //Restore les dernières variables loggées (créé la data dans le DataManager + l'ajoute sur l'IHM + coche la case)
    val = m_application->m_eeprom->read(getName(), "datalist_logged", "");
    QStringList datalist_memo = val.toStringList();
    addVariablesObserver(datalist_memo);

    // Restore l'adresse IP et le port
    val = m_application->m_eeprom->read(getName(), "exchanger_ip", "127.0.0.1");
    QString ip = val.toString().trimmed();

    val = m_application->m_eeprom->read(getName(), "exchanger_port", 2468);
    int port = val.toInt();

    // Restore le type de d'Exchanger (client ou serveur) et l'initialise
    val = m_application->m_eeprom->read(getName(), "exchanger_type", "");
    QString exchanger_type = val.toString().trimmed().toLower();
    if (exchanger_type.contains("serv")) {
        init_gateway_as_server(port);
        m_ihm.setWindowTitle(getName() + "<SERVER>"); // indique dans le titre de la fenêtre si le module est un client ou un serveur
    }
    else if (exchanger_type.contains("client")) {
        init_gateway_as_client(ip, port);
        m_ihm.setWindowTitle(getName() + "<CLIENT>");
    }
    else { // ne rien faire (on ne sait pas si ça doit être un client ou un serveur -> laisse le module off)
        m_ihm.ui.statusbar->showMessage("Exchanger is OFF");
    }

    if (m_exchanger) { // si le module est OFF, pas de connexion
        connect(m_exchanger, SIGNAL(receive_double(double)), this, SLOT(onReceiveDouble(double)));
        connect(m_exchanger, SIGNAL(receive_int32(qint32)), this, SLOT(onReceiveInt32(int)));
        connect(m_exchanger, SIGNAL(receive_uint8(quint8)), this, SLOT(onReceiveUInt8(quint8)));
        connect(m_exchanger, SIGNAL(receive_variant(QVariant)), this, SLOT(onReceiveVariant(QVariant)));
        connect(m_exchanger, SIGNAL(receive_raw_buffer(QByteArray)), this, SLOT(onReceiveRawData(QByteArray)));
        connect(m_exchanger, SIGNAL(receive_data(QString,QVariant)), this, SLOT(onReceiveData(QString,QVariant)));
        connect(m_exchanger, SIGNAL(receive_datas(QHash<QString,QVariant>)), this, SLOT(onReceiveDatas(QHash<QString,QVariant>)));
    }

    connect(m_ihm.ui.pb_test1, SIGNAL(clicked(bool)), this, SLOT(onPbTest1()));
    connect(m_ihm.ui.pb_test2, SIGNAL(clicked(bool)), this, SLOT(onPbTest2()));
    connect(m_ihm.ui.pb_send_all_data_manager, SIGNAL(clicked(bool)), this, SLOT(sendAllDataManager()));
    connect(m_ihm.ui.pb_send_selection, SIGNAL(clicked(bool)), this, SLOT(sendSelectedData()));

    connect(m_application->m_data_center, SIGNAL(dataCreated(CData*)), this, SLOT(refreshListeVariables()));

    connect(m_ihm.ui.filtre_noms_variables, SIGNAL(textChanged(QString)), this, SLOT(onDataFilterChanged(QString)));
    connect(m_ihm.ui.pb_open_variables_liste, SIGNAL(clicked(bool)), this, SLOT(loadListeVariablesObservees()));
    connect(m_ihm.ui.pb_save_variables_liste, SIGNAL(clicked(bool)), this, SLOT(saveListeVariablesObservees()));
    connect(m_ihm.ui.pb_decoche_tout, SIGNAL(clicked(bool)), this, SLOT(decocheToutesVariables()));
    connect(m_ihm.ui.pb_coche_tout, SIGNAL(clicked(bool)), this, SLOT(cocheToutesVariables()));

    connect(m_ihm.ui.active_gateway, SIGNAL(clicked(bool)), this, SLOT(activeGateway(bool)));
}

// _____________________________________________________________________
void CDataExchanger::gateway_is_connected()
{
    QString msg = "Connected to host";
    m_application->m_print_view->print_info(this, msg);
    m_ihm.ui.statusbar->showMessage(msg);
    m_ihm.ui.led_exchanger_connected->setValue(true);
    activeGateway(true);
}

// _____________________________________________________________________
void CDataExchanger::gateway_is_disconnected()
{
    QString msg = "Disconnected from host";
    m_application->m_print_view->print_info(this, msg);
    m_ihm.ui.statusbar->showMessage(msg);
    m_ihm.ui.led_exchanger_connected->setValue(false);
    activeGateway(false);
}

// _____________________________________________________________________
/*!
*  Fermeture du module
*
*/
void CDataExchanger::close(void)
{
    // Mémorise en EEPROM l'état de la fenêtre
    m_application->m_eeprom->write(getName(), "geometry", QVariant(m_ihm.geometry()));
    m_application->m_eeprom->write(getName(), "visible", QVariant(m_ihm.isVisible()));
    m_application->m_eeprom->write(getName(), "niveau_trace", QVariant((unsigned int)getNiveauTrace()));
    m_application->m_eeprom->write(getName(), "background_color", QVariant(getBackgroundColor()));

    // mémorise en EEPROM la liste des variables sous surveillance (cochées sur l'IHM)
    m_application->m_eeprom->write(getName(), "datalist_logged", QVariant(getListeVariablesObservees()));
}

// _____________________________________________________________________
/*!
 * \brief Configure le module Exchanger comme un client et l'initialise
 *        Le module étant un client, il doit se connecter à un serveur
 * \param ip_add Adresse du serveur sur lequel le client doit se connecter
 * \param port Numéro de port du serveur
 */
void CDataExchanger::init_gateway_as_client(QString ip_add, int port)
{
    if (m_ihm.ui.active_gateway->isChecked()) activeGateway(false);
    if (m_server) delete m_server;
    if (m_client) delete m_client;
    m_exchanger = Q_NULLPTR;
    m_ihm.ui.statusbar->clearMessage();

    m_client = new DataClient();
    if (!m_client) {
        m_application->m_print_view->print_error(this, "Impossible de créer un DataClient()");
        return;
    }
    m_client->connectToHost(ip_add, port, true);
    m_exchanger = &m_client->m_exchanger;
    connect(m_client, SIGNAL(connected()), this, SLOT(gateway_is_connected()));
    connect(m_client, SIGNAL(disconnected()), this, SLOT(gateway_is_disconnected()));
}

// _____________________________________________________________________
/*!
 * \brief Configure le module Exchanger comme un serveur et l'initialise
 *        Le module étant un serveur, il s'ouvre et attend une connexion d'un client
 * \param port Numéro de port du serveur sur lequel le client pourra se connecter
 */
void CDataExchanger::init_gateway_as_server(int port)
{
    if (m_ihm.ui.active_gateway->isChecked()) activeGateway(false);
    if (m_server) delete m_server;
    if (m_client) delete m_client;
    m_exchanger = Q_NULLPTR;
    m_ihm.ui.statusbar->clearMessage();
    m_server = new DataServer();
    if (!m_server) {
        m_application->m_print_view->print_error(this, "Impossible de créer un DataServer()");
        return;
    }
    m_exchanger = &m_server->m_exchanger;

    QString msg;
    if (m_server->start(port)) {
        msg = QString ("Server started on port %1").arg(port);
        m_ihm.ui.statusbar->showMessage(msg);
        m_application->m_print_view->print_info(this, msg);
        connect(m_server, SIGNAL(connected()), this, SLOT(gateway_is_connected()));
        connect(m_server, SIGNAL(disconnected()), this, SLOT(gateway_is_disconnected()));
    }
    else {
        msg = QString ("Impossible d'initialiser le serveur sur le port %1").arg(port);
        m_application->m_print_view->print_error(this, msg);
    }
}


// _____________________________________________________________________
/*!
*  Met à jour la liste des variables dans la fenêtre de gauche
*
*/
void CDataExchanger::refreshListeVariables(void)
{
    if (!m_ihm.ui.liste_variables->isEnabled()) return;

    // Mémorise d'abord la liste des variables qui étaient à observer (cochées)
    QStringList liste_variables_observees;
    for (int i=0; i<m_ihm.ui.liste_variables->count(); i++) {
        if (m_ihm.ui.liste_variables->item(i)->checkState() == Qt::Checked) {
            liste_variables_observees.append(m_ihm.ui.liste_variables->item(i)->text());
        }
    } // for toutes les variables sous surveillances

    QStringList var_list;
    m_application->m_data_center->getListeVariablesName(var_list);

    m_ihm.ui.liste_variables->clear();
    m_ihm.ui.liste_variables->addItems(var_list);
    for (int i=0; i<m_ihm.ui.liste_variables->count(); i++) {
        QString data_name = m_ihm.ui.liste_variables->item(i)->text();
        m_ihm.ui.liste_variables->item(i)->setFlags(Qt::ItemIsUserCheckable|Qt::ItemIsEnabled);
        // Restitue le fait que la case était cochée ou décochée avant la mise à jour de liste
        m_ihm.ui.liste_variables->item(i)->setCheckState(liste_variables_observees.contains(data_name) ? Qt::Checked : Qt::Unchecked);
    }
}


// _____________________________________________________________________
/*!
*  Ajout des varibales à observer
*/
void CDataExchanger::addVariablesObserver(QStringList liste_variables)
{
    for (int i=0; i<liste_variables.count(); i++) {
        QString dataname = liste_variables.at(i);
        if (dataname.simplified() != "") {
            bool force_creation = true;
            m_application->m_data_center->getData(dataname, force_creation); //crée la donnée si elle n'existe pas. Si elle existe, conserve sa valeur
            m_ihm.ui.liste_variables->addItem(dataname);
        }
    }
    for (int i=0; i<m_ihm.ui.liste_variables->count(); i++) {
        if (liste_variables.contains(m_ihm.ui.liste_variables->item(i)->text())) {
            m_ihm.ui.liste_variables->item(i)->setCheckState(Qt::Checked);
        }
    }
    refreshListeVariables();
}

// _____________________________________________________________________
/*!
*  Récupère la liste des variables à observer
*/
QStringList CDataExchanger::getListeVariablesObservees()
{
    QStringList liste_variables_observees;
    for (int i=0; i<m_ihm.ui.liste_variables->count(); i++) {
        if (m_ihm.ui.liste_variables->item(i)->checkState() == Qt::Checked) {
            liste_variables_observees.append(m_ihm.ui.liste_variables->item(i)->text());
        }
    }
    return liste_variables_observees;
}

// _____________________________________________________________________
/*!
*  Décoche toutes les variables
*/
void CDataExchanger::decocheToutesVariables()
{
    for (int i=0; i<m_ihm.ui.liste_variables->count(); i++) {
        m_ihm.ui.liste_variables->item(i)->setCheckState(Qt::Unchecked);
    }
}

// _____________________________________________________________________
/*!
*  Coche toutes les variables
*/
void CDataExchanger::cocheToutesVariables()
{
    for (int i=0; i<m_ihm.ui.liste_variables->count(); i++) {
        m_ihm.ui.liste_variables->item(i)->setCheckState(Qt::Checked);
    }
}

// _____________________________________________________________________
/*!
*  Active la passerelle
*/
void CDataExchanger::activeGateway(bool state)
{
    connectDisconnectData(state);

    // vérouille les objets pendant l'enregistrement
    m_ihm.ui.liste_variables->setEnabled(!state);
    m_ihm.ui.PB_refresh_liste->setEnabled(!state);
    m_ihm.ui.pb_decoche_tout->setEnabled(!state);
    m_ihm.ui.pb_open_variables_liste->setEnabled(!state);

    m_ihm.ui.active_gateway->setChecked(state);
}

// _____________________________________________________________________
/*!
 * Envoie d'un seul coup toutes les données contenues dans le DataManager
 */
void CDataExchanger::sendAllDataManager()
{
    QHash<QString, QVariant> datas;
    QVector<CData *> data_list;

    m_application->m_data_center->getDataList(data_list);
    foreach(CData *data, data_list) {
        datas.insert(data->getName(), data->read());
    }
    m_exchanger->send_datas(datas);

}

// _____________________________________________________________________
/*!
 * Envoie d'un seul coup toutes les données sélectionnées
 */
void CDataExchanger::sendSelectedData()
{
    QHash<QString, QVariant> datas;
    for (int i=0; i<m_ihm.ui.liste_variables->count(); i++) {
        if (m_ihm.ui.liste_variables->item(i)->checkState() == Qt::Checked) {
            CData *_data = m_application->m_data_center->getData(m_ihm.ui.liste_variables->item(i)->text());
            if (!_data) continue; // la data existe dans la liste mais n'existe pas dans le DataManager
            datas.insert(_data->getName(), _data->read());
        }
    }
    m_exchanger->send_datas(datas);
}

// _____________________________________________________________________
/*!
*  Actions à effecter lors de l'activation de l'enregistrement des data en mode "Temporel"
*
*  Crée une connexion entre toutes les variables à surveiller et l'affichage de la valeur
*  choix : 0=deconnexion / 1=connexion
*  only_diff : n'affiche que si la variable a changé de valeur
*/
void CDataExchanger::connectDisconnectData(bool choix)
{
    // Recopie toutes les variables à surveiller
    for (int i=0; i<m_ihm.ui.liste_variables->count(); i++) {
        if (m_ihm.ui.liste_variables->item(i)->checkState() == Qt::Checked) {
            CData *_data = m_application->m_data_center->getData(m_ihm.ui.liste_variables->item(i)->text());
            if (!_data) continue; // la data existe dans la liste mais n'existe pas dans le DataManager
            if (choix) {
                connect(_data,
                        SIGNAL(valueUpdated(QVariant, quint64)),
                        this,
                        SLOT(variableChangedUpdated(QVariant, quint64))
                        );
            }
            else {
                disconnect(_data,
                           SIGNAL(valueUpdated(QVariant, quint64)),
                           this,
                           SLOT(variableChangedUpdated(QVariant, quint64))
                           );
            }
        }
    }
}

// _____________________________________________________________________
/*!
*  Charge une liste de variables à observer
*/
void CDataExchanger::loadListeVariablesObservees(void)
{
    QString fileName = QFileDialog::getOpenFileName(&m_ihm, tr("Open File"), ".");
    if (fileName.isEmpty()) return;

    QStringList liste_variables;
    CDataListOpenSave::open(fileName, liste_variables);

    addVariablesObserver(liste_variables);
}

// _____________________________________________________________________
/*!
*  Sauvegarde la liste des variables observées
*/
void CDataExchanger::saveListeVariablesObservees(void)
{
    QString fileName = QFileDialog::getSaveFileName(&m_ihm, tr("Save File"), ".");
    if (fileName.isEmpty()) return;

    QStringList liste_variables = getListeVariablesObservees();
    CDataListOpenSave::save(fileName, liste_variables);
}


// _____________________________________________________________________
/*!
*  Création des menus sur clic droit sur la fenêtre du module
*
*/
void CDataExchanger::onRightClicGUI(QPoint pos)
{
    QMenu *menu = new QMenu();

    menu->addAction("Select background color", this, SLOT(selectBackgroundColor()));
    menu->exec(m_ihm.mapToGlobal(pos));
}

// _____________________________________________________________________
/*!
*  Filtre les variables sur le nom
*/
void CDataExchanger::onDataFilterChanged(QString filter_name)
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
*  Point d'entrée lorsqu'une data du DataManager configurée comme devant être transmise
*    via Exchanger a été mise à jour
*
*/
void CDataExchanger::variableChangedUpdated(QVariant val, quint64 update_time)
{
    Q_UNUSED(update_time)
    CData *data = qobject_cast<CData*>(sender()); // récupère l'objet émetteur du signal (pour pouvoir récupérer son nom)
    if (data && m_exchanger) m_exchanger->send_data(data->getName(), val);
}


void CDataExchanger::onReceiveDocDesigner(QString doc)
{
    qDebug() << "CDataExchanger::onReceiveDocDesigner" << doc;
}

void CDataExchanger::onReceiveDouble(double val)
{
    qDebug() << "CDataExchanger::onReceiveDouble" << val;
}

void CDataExchanger::onReceiveUInt8(quint8 val)
{
    qDebug() << "CDataExchanger::onReceiveUInt8" << val;
}

void CDataExchanger::onReceiveInt32(int val)
{
    qDebug() << "CDataExchanger::onReceiveInt32" << val;
}

void CDataExchanger::onReceiveVariant(const QVariant &val)
{
    qDebug() << "CDataExchanger::onReceiveVariant" << val;
}

void CDataExchanger::onReceiveRawData(const QByteArray &raw_data)
{
    //qDebug() << "CDataExchanger::onReceiveRawData" << raw_data;
    qDebug() << "CDataExchanger::onReceiveRawData / Size : " << raw_data.size();
}

// ------------------------------------------------------------
/*!
 * \brief Une data a été reçu via Exchanger -> la publie dans le DataManager
 * \param name
 * \param val
 */
void CDataExchanger::onReceiveData(QString name, QVariant val)
{
    m_application->m_data_center->write(name, val);
}

// ------------------------------------------------------------
/*!
 * \brief Une liste de data a été reçu via Exchanger -> les publie dans le DataManager
 * \param datas la liste de couples { Nom;Valeur }
 */
void CDataExchanger::onReceiveDatas(QHash<QString, QVariant> datas)
{
    qDebug() << "Reception de plusieurs datas" << datas.size();
    QHashIterator<QString, QVariant> i(datas);
    while (i.hasNext()) {
        i.next();
        m_application->m_data_center->write(i.key(), i.value());
    }
}

void CDataExchanger::onPbTest1()
{
    double ullval = 123.456;
    m_exchanger->send_variant(ullval);

    QHash<QString, QVariant> datas;

    CData * data_posX = m_application->m_data_center->getData("PosX_robot");
    //if (data_posX) m_exchanger->send_data(data_posX->getName(), data_posX->read());

    CData * data_posY = m_application->m_data_center->getData("PosY_robot");
    //if (data_posY) m_exchanger->send_data(data_posY->getName(), data_posY->read());

    datas.insert(data_posX->getName(), data_posX->read());
    datas.insert(data_posY->getName(), data_posY->read());
    m_exchanger->send_datas(datas);
}

void CDataExchanger::onPbTest2()
{
    DocDesigner doc;

    for (int i=0; i<80; i++)
    {
        doc.appendCRLF();
        doc.appendChapterTitle(1, "Nom du chapitre 5");
        doc.appendText("Une liste à puces");
        QStringList lst;
        lst << "Bulle1" << "Bulle2" << "Bulle3";
        doc.appendBulletList(lst);
        doc.appendText("Du texte normal juste en dessous de la liste à puces", true);
        doc.appendText("La même liste à puce mais en plus gros et en couleur !", true);
        doc.setFont(DocFont(doc.getDefaultFamilyFont(), 24, true, true, true, Qt::darkBlue));
        doc.appendBulletList(lst);
    }
    m_exchanger->send_string(doc.toRawText());
}


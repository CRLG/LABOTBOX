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

    // Restore le port de communication (utilisé pour le client ou le serveur)
    val = m_application->m_eeprom->read(getName(), "exchanger_port", 2468);
    int port = val.toInt();

    enableDisabelGuiOnConnectionDeconnection(false);

    // Restore le type de d'Exchanger (client ou serveur) et l'initialise
    val = m_application->m_eeprom->read(getName(), "exchanger_type", "");
    QString exchanger_type = val.toString().trimmed().toLower();
    if (exchanger_type.contains("serv")) {
        init_gateway_as_server(port);
        m_ihm.setWindowTitle(getName() + "<SERVER>"); // indique dans le titre de la fenêtre si le module est un client ou un serveur
    }
    else if (exchanger_type.contains("client")) {
        QString ip = m_application->m_eeprom->read(getName(), "exchanger_ip", "127.0.0.1").toString().trimmed();
        bool autoreconnect = m_application->m_eeprom->read(getName(), "autoreconnect", true).toBool();

        init_gateway_as_client(ip, port, autoreconnect);
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
        connect(m_exchanger, SIGNAL(receive_start_stop_request(bool)), this, SLOT(onReceiveStartStopTransmissionRequest(bool)));
        connect(m_exchanger, SIGNAL(receive_add_data_request(QString)), this, SLOT(onReceiveAddDataRequest(QString)));
        connect(m_exchanger, SIGNAL(receive_remove_data_request(QString)), this, SLOT(onReceiveRemoveDataRequest(QString)));
    }

    connect(m_ihm.ui.pb_send_all_data_manager, SIGNAL(clicked(bool)), this, SLOT(sendAllDataManager()));
    connect(m_ihm.ui.pb_send_selection, SIGNAL(clicked(bool)), this, SLOT(sendSelectedData()));
    connect(m_ihm.ui.pb_start_request, SIGNAL(clicked(bool)), this, SLOT(onStartRequest()));
    connect(m_ihm.ui.pb_stop_request, SIGNAL(clicked(bool)), this, SLOT(onStopRequest()));
    connect(m_ihm.ui.pb_add_data_request, SIGNAL(clicked(bool)), this, SLOT(onAddDataRequest()));
    connect(m_ihm.ui.pb_remove_data_request, SIGNAL(clicked(bool)), this, SLOT(onRemoveDataRequest()));

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
    enableDisabelGuiOnConnectionDeconnection(true);

    activeGateway(true);
}

// _____________________________________________________________________
void CDataExchanger::gateway_is_disconnected()
{
    QString msg = "Disconnected from host";
    m_application->m_print_view->print_info(this, msg);
    m_ihm.ui.statusbar->showMessage(msg);
    m_ihm.ui.led_exchanger_connected->setValue(false);
    enableDisabelGuiOnConnectionDeconnection(false);
    activeGateway(false);
}

// _____________________________________________________________________
void CDataExchanger::enableDisabelGuiOnConnectionDeconnection(bool state)
{
    m_ihm.ui.active_gateway->setEnabled(state);
    m_ihm.ui.pb_send_all_data_manager->setEnabled(state);
    m_ihm.ui.pb_send_selection->setEnabled(state);
    m_ihm.ui.pb_start_request->setEnabled(state);
    m_ihm.ui.pb_stop_request->setEnabled(state);
    m_ihm.ui.pb_add_data_request->setEnabled(state);
    m_ihm.ui.pb_remove_data_request->setEnabled(state);
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
 * \param autoreconect active ou non les tentatives de reconnexion automatique en cas de déconnexion
 */
void CDataExchanger::init_gateway_as_client(QString ip_add, int port, bool autoreconect)
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
    m_client->connectToHost(ip_add, port, autoreconect);
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
        if (!m_ihm.ui.liste_variables->item(i)->isHidden()) {
            m_ihm.ui.liste_variables->item(i)->setCheckState(Qt::Unchecked);
        }
    }
}

// _____________________________________________________________________
/*!
*  Coche toutes les variables
*/
void CDataExchanger::cocheToutesVariables()
{
    for (int i=0; i<m_ihm.ui.liste_variables->count(); i++) {
        if (!m_ihm.ui.liste_variables->item(i)->isHidden()) {
            m_ihm.ui.liste_variables->item(i)->setCheckState(Qt::Checked);
        }
    }
}

// _____________________________________________________________________
/*!
*  Active la passerelle
*/
void CDataExchanger::activeGateway(bool state)
{
    connectDisconnectDatas(state);

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
*  Connecte ou déconnecte toutes les data sélectionnées en vue du transfert
*
*  Crée une connexion entre toutes les variables cochées
*  choix : 0=deconnexion / 1=connexion
*  only_diff : n'affiche que si la variable a changé de valeur
*/
void CDataExchanger::connectDisconnectDatas(bool choix)
{
    for (int i=0; i<m_ihm.ui.liste_variables->count(); i++) {
        if (m_ihm.ui.liste_variables->item(i)->checkState() == Qt::Checked) {
            CData *_data = m_application->m_data_center->getData(m_ihm.ui.liste_variables->item(i)->text());
            if (!_data) continue; // la data existe dans la liste mais n'existe pas dans le DataManager
            connectDisconnectData(_data, choix);
        }
    }
}

// _____________________________________________________________________
/*!
 * \brief Crée une connexion ou deconnexion de la data
 * \param data la data à connecter/déconnecter
 * \param choix : 0=deconnexion / 1=connexion
 */
void CDataExchanger::connectDisconnectData(CData *data, bool choix)
{
    if (!data) return; // la data existe dans la liste mais n'existe pas dans le DataManager
    if (choix) {
        connect(data,
                SIGNAL(valueUpdated(QVariant, quint64)),
                this,
                SLOT(variableChangedUpdated(QVariant, quint64))
                );
    }
    else {
        disconnect(data,
                   SIGNAL(valueUpdated(QVariant, quint64)),
                   this,
                   SLOT(variableChangedUpdated(QVariant, quint64))
                   );
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
    QHashIterator<QString, QVariant> i(datas);
    while (i.hasNext()) {
        i.next();
        m_application->m_data_center->write(i.key(), i.value());
    }
}

// ------------------------------------------------------------
/*!
 * \brief Une demande d'ajout de donnée à la liste des data à publier à été reçue
 * \param name le nom de la data à ajouter à la liste des data à publier
 */
void CDataExchanger::onReceiveAddDataRequest(QString name)
{
    if (getListeVariablesObservees().contains(name)) return;    // Si la donnée existe déjà et déjà cochée, ne fait rien
    bool isGatewayActive = m_ihm.ui.active_gateway->isChecked();
    if (isGatewayActive) activeGateway(false);  // Désactive temporairement la passerelle le temps d'ajouter la donnée
    addVariablesObserver(QStringList(name));
    if (isGatewayActive) activeGateway(true);   // Réactive la passerelle si elle était active juste avant
}

// ------------------------------------------------------------
/*!
 * \brief Une demande d'arrêt de transfert de donnée de la liste des data à publier à été reçue
 * \param name le nom de la data à arrêter de transférer
 */
void CDataExchanger::onReceiveRemoveDataRequest(QString name)
{
    if (!getListeVariablesObservees().contains(name))   return;     // Si la donnée n'est pas cochée, ne fait rien
    if (!m_application->m_data_center->getData(name))   return;     // Si la donnée n'existe pas dans le DataManager, ne rien faire
    bool isGatewayActive = m_ihm.ui.active_gateway->isChecked();
    if (isGatewayActive) activeGateway(false);  // Désactive temporairement la passerelle le temps d'ajouter la donnée
    for (int i=0; i<m_ihm.ui.liste_variables->count(); i++) {  // Décoche l'élément
        if (m_ihm.ui.liste_variables->item(i)->text() == name) {
            m_ihm.ui.liste_variables->item(i)->setCheckState(Qt::Unchecked);
        }
    }
    if (isGatewayActive) activeGateway(true);   // Réactive la passerelle si elle était active juste avant
}

// ------------------------------------------------------------
/*!
 * \brief Demande de commencement ou d'arrêt de transmission des données
 * \param start_stop 0=stop / 1=start
 */
void CDataExchanger::onReceiveStartStopTransmissionRequest(bool start_stop)
{
    activeGateway(start_stop);
}

// ------------------------------------------------------------
/*!
 * \brief Demande au distant de démarrer le transfert
 */
void CDataExchanger::onStartRequest()
{
    m_exchanger->send_start_stop_transmission_request(true);
}

// ------------------------------------------------------------
/*!
 * \brief Demande au distant d'arrêter le transfert
 */
void CDataExchanger::onStopRequest()
{
    m_exchanger->send_start_stop_transmission_request(false);
}

// ------------------------------------------------------------
/*!
 * \brief Demande au distant d'ajouter une data au transfert
 */
void CDataExchanger::onAddDataRequest()
{
    QString data_name = m_ihm.ui.distant_data_name->text();
    m_exchanger->send_add_data_request(data_name);
}

// ------------------------------------------------------------
/*!
 * \brief Demande au distant de supprimer une une data au transfert
 */
void CDataExchanger::onRemoveDataRequest()
{
    QString data_name = m_ihm.ui.distant_data_name->text();
    m_exchanger->send_remove_data_request(data_name);
}

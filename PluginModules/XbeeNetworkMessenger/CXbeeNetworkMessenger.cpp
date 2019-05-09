/*! \file CXbeeNetworkMessenger.cpp
 * A brief file description CPP.
 * A more elaborated file description.
 */
#include <QDebug>
#include <QStringList>
#include <QMessageBox>
#include "CXbeeNetworkMessenger.h"
#include "CApplication.h"
#include "CPrintView.h"
#include "CMainWindow.h"
#include "CEEPROM.h"
#include "CDataManager.h"
#include "CXBEE.h"

#include "qled.h"

#define MESSENGER_REFRESH_PERIOD 100

/*! \addtogroup Module_XbeeNetworkMessenger
   * 
   *  @{
   */
	   
// _____________________________________________________________________
/*!
*  Constructeur
*
*/
CXbeeNetworkMessenger::CXbeeNetworkMessenger(const char *plugin_name)
    :CPluginModule(plugin_name, VERSION_XbeeNetworkMessenger, AUTEUR_XbeeNetworkMessenger, INFO_XbeeNetworkMessenger)
{
}


// _____________________________________________________________________
/*!
*  Destructeur
*
*/
CXbeeNetworkMessenger::~CXbeeNetworkMessenger()
{

}


// _____________________________________________________________________
/*!
*  Initialisation du module
*
*/
void CXbeeNetworkMessenger::init(CApplication *application)
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

  val = m_application->m_eeprom->read(getName(), "trace_active", QVariant(false));
  m_trace_active = val.toBool();
  m_ihm.ui.trace_active->setChecked(m_trace_active);

  m_xbee_messenger.initApp(m_application);

  connect(m_application->m_XBEE, SIGNAL(readyBytes(QByteArray,unsigned short)), this, SLOT(Xbee_readyBytes(QByteArray,unsigned short)));
  connect(m_ihm.ui.clear_trace, SIGNAL(released()), this, SLOT(on_clear_trace()));

  connect(m_ihm.ui.message_list, SIGNAL(currentIndexChanged(QString)), this, SLOT(on_message_name_changed(QString)));
  connect(m_ihm.ui.ID, SIGNAL(editingFinished()), this, SLOT(on_message_id_changed()));
  connect(m_ihm.ui.send, SIGNAL(released()), this, SLOT(on_send_message()));
  connect(m_ihm.ui.trace_active, SIGNAL(clicked(bool)), this, SLOT(on_trace_active_changed(bool)));

  initDiagPresencePage();
  initGeneratorPage();

  connect(&m_timer, SIGNAL(timeout()), this, SLOT(on_timer_tick()));
  m_timer.start(MESSENGER_REFRESH_PERIOD);
}


// _____________________________________________________________________
/*!
*  Fermeture du module
*
*/
void CXbeeNetworkMessenger::close(void)
{
  // Mémorise en EEPROM l'état de la fenêtre
  m_application->m_eeprom->write(getName(), "geometry", QVariant(m_ihm.geometry()));
  m_application->m_eeprom->write(getName(), "visible", QVariant(m_ihm.isVisible()));
  m_application->m_eeprom->write(getName(), "niveau_trace", QVariant((unsigned int)getNiveauTrace()));
  m_application->m_eeprom->write(getName(), "background_color", QVariant(getBackgroundColor()));
  m_application->m_eeprom->write(getName(), "trace_active", QVariant(m_trace_active));
}

// _____________________________________________________________________
/*!
*  Création des menus sur clic droit sur la fenêtre du module
*
*/
void CXbeeNetworkMessenger::onRightClicGUI(QPoint pos)
{
  QMenu *menu = new QMenu();

  menu->addAction("Select background color", this, SLOT(selectBackgroundColor()));
  menu->exec(m_ihm.mapToGlobal(pos));
}

// _____________________________________________________________________
DatabaseXbeeNetwork2019* CXbeeNetworkMessenger::getDatabase()
{
    return &m_xbee_messenger.m_database;
}

// _____________________________________________________________________
/*!
*  Traitement des messages valides en provenance du XBEE
*  Refirige les données vers le decodeur du messenger
*/
void CXbeeNetworkMessenger::Xbee_readyBytes(QByteArray data, unsigned short src_addr)
{
    for (int i=0; i<data.size(); i++) {
        m_xbee_messenger.decode(data.at(i), src_addr);
    }
}


// =====================================================================
// =====================================================================

// _____________________________________________________________________
/*!
*  Traitement de la requête du messenger pour encoder les données.
*  Refirige les données vers l'encodeur du module XBEE
*/
void CXbeeNetworkMessenger::messengerEncodeRequest(unsigned char *buff_data, unsigned short buff_size, unsigned short dest_address)
{
    QByteArray data((const char*)buff_data, buff_size);
    m_application->m_XBEE->encode(data, dest_address);
}

// =====================================================================
// =====================================================================

// _____________________________________________________________________
void CXbeeNetworkMessenger::on_timer_tick()
{
    m_xbee_messenger.m_database.checkAndSendPeriodicMessages();
    m_xbee_messenger.m_database.checkNodeCommunication();
}


// _____________________________________________________________________
// Construit automatiquement la page de diagnostique de présenc
// en fonction des informations renseignées sur les nodes dans la database
void CXbeeNetworkMessenger::initDiagPresencePage()
{
    int ligne=0;
    NodeBase **node_list = m_xbee_messenger.m_database.getNodesList();
    for (int i=0; i<m_xbee_messenger.m_database.getNodeCount(); i++)
    {
        NodeBase *node = node_list[i];
        if (node) {
            QString data_name= "XbeeMsngNodePresent." + QString(node->getName());
            m_application->m_data_center->write(data_name, QVariant(node->isPresent()));
            CData *data = m_application->m_data_center->getData(data_name);
            if (data) {
                connect(data, SIGNAL(valueChanged(QVariant)), this, SLOT(on_node_com_status_changed()));
            }

            QLed *led;
            led = new QLed(m_ihm.ui.tab_diag);
            led->setObjectName("led_" + data_name);
            led->setMaximumSize(QSize(30, 30));
            led->setMinimumSize(QSize(20, 20));
            led->setValue(node->isPresent());
            led->setOnColor(QLed::Green);
            led->setOffColor(QLed::Grey);
            led->setShape(QLed::Circle);
            led->setToolTip(data_name);
            m_ihm.ui.layout_led_presence->addWidget(led, ligne, 0, 1, 1);

            QLabel *label = new QLabel(m_ihm.ui.tab_diag);
            label->setObjectName("lbl_" + data_name);
            label->setText(node->getName());
            label->setToolTip("Node ID=" + QString::number(node->getID()));
            m_ihm.ui.layout_led_presence->addWidget(label, ligne, 1, 1, 1);

            ligne++;
        }
    }
}

// _____________________________________________________________________
// Méthode appelée à chaque fois qu'un noeud change de status de communication
// (passe de OK à KO ou l'inverse)
void CXbeeNetworkMessenger::on_node_com_status_changed()
{
    NodeBase **node_list = m_xbee_messenger.m_database.getNodesList();
    for (int i=0; i<m_xbee_messenger.m_database.getNodeCount(); i++) {
        NodeBase *node = node_list[i];
        if (node) {
            QString led_name= "led_XbeeMsngNodePresent." + QString(node->getName());
            QLed *led = m_ihm.findChild<QLed *>(led_name);  // recherche l'objet LED par son nom
            if (led) {
                led->setValue(node->isPresent());
            }
        }
    }
}


// =====================================================================
//                  PAGE GENERATOR
// =====================================================================

// _____________________________________________________________________
void CXbeeNetworkMessenger::initGeneratorPage()
{
    // Construit la liste des messages
    MessageBase **message_list = m_xbee_messenger.m_database.getMessagesList();
    QStringList str_msg_list;
    for (int i=0; i<m_xbee_messenger.m_database.getMessageCount(); i++) {
        if (message_list[i]) str_msg_list.append(message_list[i]->getName());
    }
    str_msg_list.sort();
    m_ihm.ui.message_list->addItems(str_msg_list);

    // Construit la liste des nodes
    NodeBase **node_list = m_xbee_messenger.m_database.getNodesList();
    QStringList str_node_list;
    for (int i=0; i<m_xbee_messenger.m_database.getNodeCount(); i++) {
        if (node_list[i]) str_node_list.append(node_list[i]->getName());
    }
    str_node_list.append("0xFFFF_BROADCAST");
    str_node_list.sort();
    m_ihm.ui.node_list->addItems(str_node_list);
}

// _____________________________________________________________________
// Lorsque le nom du message est changé, met à jour l'identifiant du message et la longueur
void CXbeeNetworkMessenger::on_message_name_changed(QString msg)
{
    MessageBase **message_list = m_xbee_messenger.m_database.getMessagesList();
    for (int i=0; i<m_xbee_messenger.m_database.getMessageCount(); i++) {
        if (message_list[i]) {
            if (msg == message_list[i]->getName()) {
                m_ihm.ui.ID->setValue(message_list[i]->getID());
                m_ihm.ui.dlc->setValue(message_list[i]->getDLC());
                break;
            }
        }
    }
}

// _____________________________________________________________________
// Lorsque l'ID du message est changé, met à jour le nom et la longueur
void CXbeeNetworkMessenger::on_message_id_changed()
{
    int ID = m_ihm.ui.ID->value();
    MessageBase **message_list = m_xbee_messenger.m_database.getMessagesList();
    for (int i=0; i<m_xbee_messenger.m_database.getMessageCount(); i++) {
        if (message_list[i]) {
            if (ID == message_list[i]->getID()) {
                m_ihm.ui.message_list->setCurrentText(message_list[i]->getName());
                m_ihm.ui.dlc->setValue(message_list[i]->getDLC());
                break;
            }
        }
    }
}

// _____________________________________________________________________
void CXbeeNetworkMessenger::on_send_message()
{
    tMessengerFrame frame;

    frame.DLC = m_ihm.ui.dlc->value();
    frame.ID = m_ihm.ui.ID->value();

    // Les datas
    QString raw_data = m_ihm.ui.datas->text();
    raw_data = raw_data.simplified(); // néttoyage des double espaces, tabulation, ...
    QStringList list = raw_data.split(" ");
    for (int i=0; i<list.size(); i++) {
        QString str_val = list.at(i);
        bool ok;
        int data = str_val.toInt(&ok, 0);  // "0" = détection automatique de la base 10 ou 16 ou ...
        if (!ok) {  // conversion en valeur numérique échouée ?
            QMessageBox msgBox;
            msgBox.setIcon(QMessageBox::Warning);
            msgBox.setText(str_val + " is invalid");
            msgBox.exec();
            return;  // si une seule valeur est KO, pas la peine d'aller plus loin
        }
        frame.Data[i] = data;
    }
    // Adresse de destination du message
    frame.DestinationAddress = XBEE_BROADCAST_ID;  // valeur par défaut
    NodeBase **node_list = m_xbee_messenger.m_database.getNodesList();
    for (int i=0; i<m_xbee_messenger.m_database.getNodeCount(); i++) {
        if (node_list[i]) {
            if (m_ihm.ui.node_list->currentText() == node_list[i]->getName()) {
                frame.DestinationAddress = node_list[i]->getID();
                break;
            }
        }
    }
    m_xbee_messenger.m_transporter.encode(&frame);
}

// =====================================================================
//                  PAGE ANALYZER
// =====================================================================

// _____________________________________________________________________
void CXbeeNetworkMessenger::messengerDisplayAnalyzer(QString str_msg)
{
    if (m_trace_active) {
        m_ihm.ui.analyser->append(str_msg);
    }
}

// _____________________________________________________________________
void CXbeeNetworkMessenger::on_clear_trace()
{
    m_ihm.ui.analyser->clear();
}

// _____________________________________________________________________
void CXbeeNetworkMessenger::on_trace_active_changed(bool val)
{
    m_trace_active = val;
}


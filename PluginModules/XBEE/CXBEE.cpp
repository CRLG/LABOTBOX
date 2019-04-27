/*! \file CXBEE.cpp
 * A brief file description CPP.
 * A more elaborated file description.
 */
#include <QDebug>
#include <QStringList>
#include "CXBEE.h"
#include "CApplication.h"
#include "CPrintView.h"
#include "CMainWindow.h"
#include "CEEPROM.h"
#include "CDataManager.h"
#include "CRS232.h"


/*! \addtogroup Module_XBEE
   * 
   *  @{
   */
	   
// _____________________________________________________________________
/*!
*  Constructeur
*
*/
CXBEE::CXBEE(const char *plugin_name)
    :CPluginModule(plugin_name, VERSION_XBEE, AUTEUR_XBEE, INFO_XBEE)
{
}


// _____________________________________________________________________
/*!
*  Destructeur
*
*/
CXBEE::~CXBEE()
{

}


// _____________________________________________________________________
/*!
*  Initialisation du module
*
*/
void CXBEE::init(CApplication *application)
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

  m_xbee_driver.setApplication(m_application);

  val = m_application->m_eeprom->read(getName(), "XBEE.APIMODE", QVariant("1"));
  m_xbee_settings.APIMODE = val.toByteArray().at(0);
  m_ihm.ui.api_mode->setText(val.toString());

  // Attention, les tableaux de caractère n'ont pas la place pour accueillir '\0', donc pas possible d'utiliser strcpy
  val = m_application->m_eeprom->read(getName(), "XBEE.CHANNEL", QVariant("0E"));
  for (int i=0; i<val.toByteArray().size(); i++) {
    m_xbee_settings.CHANNEL[i] = val.toByteArray().at(i);
  }
  m_ihm.ui.channel->setText(val.toString());

  val = m_application->m_eeprom->read(getName(), "XBEE.COORDINATOR", QVariant("1"));
  m_xbee_settings.COORDINATOR = val.toByteArray().at(0);
  m_ihm.ui.coordinator->setText(val.toString());

  val = m_application->m_eeprom->read(getName(), "XBEE.COORDINATOR_OPTION", QVariant(0x04));
  m_xbee_settings.COORDINATOR_OPTION = val.toInt();
  m_ihm.ui.coordinator_option->setText(val.toString());

  val = m_application->m_eeprom->read(getName(), "XBEE.PANID", QVariant("3321"));
  for (int i=0; i<val.toByteArray().size(); i++) {
      m_xbee_settings.PANID[i] = val.toByteArray().at(i);
  }
  m_ihm.ui.panid->setText(val.toString());

  val = m_application->m_eeprom->read(getName(), "XBEE.KEY", QVariant("6910DEA76FC0328DEBB4307854EDFC42"));
  for (int i=0; i<val.toByteArray().size(); i++) {
      m_xbee_settings.KEY[i] = val.toByteArray().at(i);
  }
  m_ihm.ui.key->setText(val.toString());

  val = m_application->m_eeprom->read(getName(), "XBEE.ID", QVariant("1"));
  m_xbee_settings.ID = val.toByteArray().at(0);
  m_ihm.ui.id->setText(val.toString());

  val = m_application->m_eeprom->read(getName(), "XBEE.SECURITY", QVariant("1"));
  m_xbee_settings.SECURITY = val.toByteArray().at(0);
  m_ihm.ui.security->setText(val.toString());

  val = m_application->m_eeprom->read(getName(), "auto_init_xbee", QVariant(false));
  m_auto_init_xbee = val.toBool();
  m_ihm.ui.auto_config_startup->setChecked(m_auto_init_xbee);

  val = m_application->m_eeprom->read(getName(), "trace_active", QVariant(false));
  m_trace_active = val.toBool();
  m_ihm.ui.trace_active->setChecked(m_trace_active);

  val = m_application->m_eeprom->read(getName(), "data_to_send", QVariant(""));
  m_ihm.ui.line_generator->setText(val.toString());

  val = m_application->m_eeprom->read(getName(), "destination_address", QVariant(XBEE_BROADCAST_ID));
  m_ihm.ui.destination_address->setValue(val.toInt());

  if (m_auto_init_xbee) {
      m_xbee_driver.init(m_xbee_settings);
  }

  connect(m_ihm.ui.auto_config_startup, SIGNAL(clicked(bool)), this, SLOT(on_auto_config_changed(bool)));
  connect(m_ihm.ui.send_config, SIGNAL(released()), this, SLOT(on_send_config()));
  connect(m_ihm.ui.trace_active, SIGNAL(clicked(bool)), this, SLOT(on_trace_active_changed(bool)));
  connect(m_ihm.ui.clear_trace, SIGNAL(released()), m_ihm.ui.xbee_trace, SLOT(clear()));
  connect(m_ihm.ui.send_data, SIGNAL(released()), this, SLOT(on_send_data()));

  connect(m_application->m_RS232_xbee, SIGNAL(readyBytes(QByteArray)), this, SLOT(serialReadyBytes(QByteArray)));
}


// _____________________________________________________________________
/*!
*  Fermeture du module
*
*/
void CXBEE::close(void)
{
  // Mémorise en EEPROM l'état de la fenêtre
  m_application->m_eeprom->write(getName(), "geometry", QVariant(m_ihm.geometry()));
  m_application->m_eeprom->write(getName(), "visible", QVariant(m_ihm.isVisible()));
  m_application->m_eeprom->write(getName(), "niveau_trace", QVariant((unsigned int)getNiveauTrace()));
  m_application->m_eeprom->write(getName(), "background_color", QVariant(getBackgroundColor()));

  // Attention, les tableaux de caractère ne se terminent pas par '\0'
  m_application->m_eeprom->write(getName(), "XBEE.APIMODE", QString(m_xbee_settings.APIMODE));
  m_application->m_eeprom->write(getName(), "XBEE.CHANNEL", QString(QByteArray(m_xbee_settings.CHANNEL,2)));
  m_application->m_eeprom->write(getName(), "XBEE.COORDINATOR", QString(m_xbee_settings.COORDINATOR));
  m_application->m_eeprom->write(getName(), "XBEE.COORDINATOR_OPTION", QVariant(m_xbee_settings.COORDINATOR_OPTION));
  m_application->m_eeprom->write(getName(), "XBEE.PANID", QString(QByteArray(m_xbee_settings.PANID, 4)));
  m_application->m_eeprom->write(getName(), "XBEE.KEY", QString(QByteArray(m_xbee_settings.KEY, 32)));
  m_application->m_eeprom->write(getName(), "XBEE.ID", QString(m_xbee_settings.ID));
  m_application->m_eeprom->write(getName(), "XBEE.SECURITY", QString(m_xbee_settings.SECURITY));

  m_application->m_eeprom->write(getName(), "auto_init_xbee", QVariant(m_auto_init_xbee));
  m_application->m_eeprom->write(getName(), "trace_active", QVariant(m_trace_active));
  m_application->m_eeprom->write(getName(), "data_to_send", QVariant(m_ihm.ui.line_generator->text()));
  m_application->m_eeprom->write(getName(), "destination_address", QVariant(m_ihm.ui.destination_address->value()));
}

// _____________________________________________________________________
/*!
*  Création des menus sur clic droit sur la fenêtre du module
*
*/
void CXBEE::onRightClicGUI(QPoint pos)
{
  QMenu *menu = new QMenu();

  menu->addAction("Select background color", this, SLOT(selectBackgroundColor()));
  menu->exec(m_ihm.mapToGlobal(pos));
}

// _____________________________________________________________________
void CXBEE::on_auto_config_changed(bool val)
{
    m_auto_init_xbee = val;
}

// _____________________________________________________________________
void CXBEE::on_send_config()
{
    m_xbee_settings.APIMODE = m_ihm.ui.api_mode->text().at(0).toLatin1();
    for (int i=0; i<m_ihm.ui.channel->text().size(); i++) {
        m_xbee_settings.CHANNEL[i] = m_ihm.ui.channel->text().at(i).toLatin1();
    }
    m_xbee_settings.COORDINATOR = m_ihm.ui.coordinator->text().at(0).toLatin1();
    m_xbee_settings.COORDINATOR_OPTION = m_ihm.ui.coordinator_option->text().toInt();
    for (int i=0; i<m_ihm.ui.panid->text().size(); i++) {
        m_xbee_settings.PANID[i] = m_ihm.ui.panid->text().at(i).toLatin1();
    }
    for (int i=0; i<m_ihm.ui.key->text().size(); i++) {
        m_xbee_settings.KEY[i] = m_ihm.ui.key->text().at(i).toLatin1();
    }
    m_xbee_settings.ID = m_ihm.ui.id->text().at(0).toLatin1();
    m_xbee_settings.SECURITY = m_ihm.ui.security->text().at(0).toLatin1();

    m_xbee_driver.init(m_xbee_settings);
}

// _____________________________________________________________________
void CXBEE::on_trace_active_changed(bool val)
{
    m_trace_active = val;
}

// _____________________________________________________________________
// Format attendu de la ligne : décimal, hexa, mixte avec séparation par des expaces ou tabulation
//    Exemple de ligne valide : 100 0xab 12 245 0xef 0x9F 45
void CXBEE::on_send_data()
{
    QString raw_data = m_ihm.ui.line_generator->text();
    int dest_address = m_ihm.ui.destination_address->value();

    raw_data = raw_data.simplified(); // néttoyage des double espaces, tabulation, ...
    QStringList list = raw_data.split(" ");
    QByteArray data_to_send;
    for (int i=0; i<list.size(); i++) {
        QString str_val = list.at(i);
        bool ok;
        int data = str_val.toInt(&ok, 0);  // "0" = détection automatique de la base 10 ou 16 ou ...
        if (!ok) {  // conversion en valeur numérique échouée ?
            QMessageBox msgBox;
            msgBox.setIcon(QMessageBox::Warning);
            msgBox.setText(str_val + " is invalid");
            msgBox.exec();
            break;  // si une seule valeur est KO, pas la peine d'aller plus loin
        }
        data_to_send.append(data);
    }
    if (data_to_send.size() > 0) {
        encode(data_to_send, dest_address);
    }
}

// ====================================================================
// ====================================================================
// _____________________________________________________________________
/*!
*  Traitement de l'évènement readyBytes remonté par le driver XBEE lorsqu'un message valide est reçu
*  Affiche le message et envoie un signal avec les données reçues pour les autres modules
*/
void CXBEE::XbeeEvt_readyBytes(unsigned char *buff_data, unsigned char buff_size, unsigned short source_id)
{
    QByteArray data((const char*)buff_data, buff_size);
    if (m_trace_active) {
        QString str;
        for (int i=0; i<buff_size; i++) {
            str += " 0x" + QString::number(buff_data[i], 16);
        }
        m_ihm.ui.xbee_trace->append("[" + QString::number(source_id) + "]>" + str);
    }
    emit readyBytes(data, source_id);
}

// _____________________________________________________________________
/*!
*  Traitement de la reqûete write envoyée par le driver XBEE
*  Affiche les données et transfert les données vers la liaison série physique
*/
void CXBEE::XbeeEvt_write(unsigned char *buff_data, unsigned char buff_size)
{
    QByteArray data((const char*)buff_data, buff_size);
    if (m_trace_active) {
        QString str;
        for (int i=0; i<buff_size; i++) {
            str += " 0x" + QString::number(buff_data[i], 16);
        }
        m_ihm.ui.xbee_trace->append("<" + str);
    }
    m_application->m_RS232_xbee->write(data);
}


// ====================================================================
// ====================================================================
// _____________________________________________________________________
/*!
*  Données brutes reçues de la RS232 XBEE
*  Transmet les données au décodeur xbee
*
*/
void CXBEE::serialReadyBytes(QByteArray data)
{
    qDebug() << "serialReadyBytes" << data;
    for (int i=0; i<data.size(); i++) {
        m_xbee_driver.decode(data[i]);
    }
}

// _____________________________________________________________________
/*!
*  Slot point d'entrée pour les données à encoder au format XBEE et à transmettre vers le XBEE
*
*/
void CXBEE::encode(QByteArray data, unsigned short dest_addr)
{
    m_xbee_driver.encode((unsigned char*)data.toStdString().c_str(), data.size(), dest_addr);
}

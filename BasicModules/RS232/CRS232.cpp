/*! \file CRS232.cpp
 * A brief file description CPP.
 * A more elaborated file description.
 */
#include <QDebug>
#include "CRS232.h"
#include "CLaBotBox.h"
#include "CPrintView.h"
#include "CMainWindow.h"
#include "CEEPROM.h"
#include "CDataManager.h"
#include "CToolBox.h"
#include "console.h"


/*! \addtogroup Module_Test2
   * 
   *  @{
   */
	   
// _____________________________________________________________________
/*!
*  Constructeur
*
*/
CRS232::CRS232(const char *plugin_name)
    :   CBasicModule(plugin_name, VERSION_RS232, AUTEUR_RS232, INFO_RS232),
        m_terminal_ON(false),
        m_hex_edit_ON(false),
        m_hex_edit_data(NULL),
        m_hex_edit_writer(NULL),
        m_simu_rx_tx_ON(false),
        m_simuTx_data(NULL),
        m_simuTx_writer(NULL)
{
}


// _____________________________________________________________________
/*!
*  Destructeur
*
*/
CRS232::~CRS232()
{
}


// _____________________________________________________________________
/*!
*  Initialisation du module
*
*/
void CRS232::init(CLaBotBox *application)
{
  CModule::init(application);
  setGUI(&m_ihm); // indique à la classe de base l'IHM

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

  // Restore les informations sur la RS232
  val = m_application->m_eeprom->read(getName(), "portname", "");
  QString portname = val.toString();
  tConfigRS232 config;
  val = m_application->m_eeprom->read(getName(), "baudrate", 19200);
  config.baudrate = (QSerialPort::BaudRate)val.toInt();
  val = m_application->m_eeprom->read(getName(), "databits", 8);
  config.databits = (QSerialPort::DataBits)val.toInt();
  val = m_application->m_eeprom->read(getName(), "flowcontrol", 0);
  config.flowcontrol = (QSerialPort::FlowControl)val.toInt();
  val = m_application->m_eeprom->read(getName(), "parity", 0);
  config.parity = (QSerialPort::Parity)val.toInt();
  val = m_application->m_eeprom->read(getName(), "stopbits", 0);
  config.stopbits = (QSerialPort::StopBits)val.toInt();
  val = m_application->m_eeprom->read(getName(), "echo_enabled", 0);
  config.echo_enabled = val.toInt();

  procFillingOptions(); // Remplit toutes les valeurs possibles de l'IHM
  serialConfigToIHM(portname, config); // Met en cohérence l'IHM avec la configuration du port

  connect(m_ihm.ui.OpenButton, SIGNAL(clicked()), this, SLOT(openButtonClick()));
  connect(m_ihm.ui.CloseButton, SIGNAL(clicked()), this, SLOT(closeButtonClick()));
  connect(m_ihm.ui.portCOM, SIGNAL(currentIndexChanged(int)), this, SLOT(portCOMChanged(int)));

  connect(m_ihm.ui.TX_send, SIGNAL(clicked()), this, SLOT(TX_send()));


  connect(&m_rs232_listener, SIGNAL(readyBytes(QByteArray)), this, SLOT(reveive_rx_data(QByteArray)));
  connect(&m_rs232_listener, SIGNAL(connected()), this, SLOT(serialConnected()));
  connect(&m_rs232_listener, SIGNAL(disconnected()), this, SLOT(serialDisconnected()));

  // ré-émet les signaux de m_rs232_listener
  connect(&m_rs232_listener, SIGNAL(readyBytes(QByteArray)), this, SIGNAL(readyBytes(QByteArray)));
  connect(&m_rs232_listener, SIGNAL(connected()), this, SIGNAL(connected()));
  connect(&m_rs232_listener, SIGNAL(disconnected()), this, SIGNAL(disconnected()));
  connect(&m_rs232_listener, SIGNAL(error()), this, SIGNAL(error()));


  // Onglet Terminal
  m_terminal = new Console;
  m_ihm.ui.console_terminal->addWidget(m_terminal);
  m_terminal->setReadOnly(true);
  m_terminal->setEnabled(true);
  m_terminal->setTextInteractionFlags(Qt::TextEditorInteraction);
  connect(m_ihm.ui.actionTerminal_OnOff, SIGNAL(toggled(bool)), this, SLOT(Terminal_OnOff(bool)));
  connect(m_ihm.ui.actionTerminal_Clear, SIGNAL(triggered()), this, SLOT(Terminal_Clear()));
  connect(m_ihm.ui.actionTerminal_EchoLocal, SIGNAL(toggled(bool)), this, SLOT(Terminal_EchoLocal(bool)));
  connect(m_ihm.ui.actionTerminal_Sauvegarder, SIGNAL(triggered()), this, SLOT(Terminal_Sauvegarder()));
  connect(m_ihm.ui.actionTerminal_EnvoyerFichierTexte, SIGNAL(triggered()), this, SLOT(Terminal_EnvoyerFichierTexte()));
  Terminal_OnOff(false);  // fonction inhibée par défaut

  // Onglet HexEdit
  connect(m_ihm.ui.actionHexEdit_OnOff, SIGNAL(toggled(bool)), this, SLOT(HexEdit_OnOff(bool)));
  connect(m_ihm.ui.actionHexEdit_Clear, SIGNAL(triggered()), this, SLOT(HexEdit_Clear()));
  connect(m_ihm.ui.actionHexEdit_SauvegarderBrut, SIGNAL(triggered()), this, SLOT(HexEdit_SauvegarderBrut()));
  connect(m_ihm.ui.actionHexEdit_SauvegarderHexASCII, SIGNAL(triggered()), this, SLOT(HexEdit_SauvegarderHexASCII()));
  connect(m_ihm.ui.actionHexEdit_CopyBrut, SIGNAL(triggered()), this, SLOT(HexEdit_CopyBrut()));
  connect(m_ihm.ui.actionHexEdit_CopyHexASCII, SIGNAL(triggered()), this, SLOT(HexEdit_CopyHexASCII()));
  connect(m_ihm.ui.actionHexEdit_EnvoyerFichierFormatBrut, SIGNAL(triggered()), this, SLOT(HexEdit_EnvoyerFichierFormatBrut()));
  connect(m_ihm.ui.actionHexEdit_EnvoyerFichierFormatHexASCII, SIGNAL(triggered()), this, SLOT(HexEdit_EnvoyerFichierFormatHexASCII()));
  HexEdit_OnOff(false); // fonction inhibée par défaut

  // Onglet SimuRxTx
  connect(m_ihm.ui.actionSimuRxTx_OnOff, SIGNAL(toggled(bool)), this, SLOT(SimuRxTx_OnOff(bool)));
  connect(m_ihm.ui.actionVisuTx_Clear, SIGNAL(triggered()), this, SLOT(VisuTx_Clear()));
  connect(m_ihm.ui.actionSimuRx_DonneesEntrantesFichierBrut, SIGNAL(triggered()), this, SLOT(SimuRx_DonneesEntrantesFichierBrut()));
  connect(m_ihm.ui.actionSimuRx_DonneesEntrantesFichierHexASCII, SIGNAL(triggered()), this, SLOT(SimuRx_DonneesEntrantesFichierHexASCII()));
  connect(m_ihm.ui.SimuRX_send, SIGNAL(clicked()), this, SLOT(SimuRX_send()));
  connect(m_ihm.ui.actionVisuTx_SauvegarderBrut, SIGNAL(triggered()), this, SLOT(VisuTx_SauvegarderBrut()));
  connect(m_ihm.ui.actionVisuTx_SauvegarderHexASCII, SIGNAL(triggered()), this, SLOT(VisuTx_SauvegarderHexASCII()));
  SimuRxTx_OnOff(false);    // fonction inhibée par défaut


  // ouvre le port RS232 s'il était ouvert à la précédente session
  val = m_application->m_eeprom->read(getName(), "connected", 0);
  if (val.toInt()) {
    m_rs232_listener.startCommunication(portname, config);  // Ouvre le port de communication
  }

}


// _____________________________________________________________________
/*!
*  Fermeture du module
*
*/
void CRS232::close(void)
{
  // Mémorise en EEPROM l'état de la fenêtre
  m_application->m_eeprom->write(getName(), "geometry", QVariant(m_ihm.geometry()));
  m_application->m_eeprom->write(getName(), "visible", QVariant(m_ihm.isVisible()));
  m_application->m_eeprom->write(getName(), "niveau_trace", QVariant((unsigned int)getNiveauTrace()));

  m_application->m_eeprom->write(getName(), "connected", QVariant((int)m_rs232_listener.isOpen()));
  // Si le port était ouvert, c'est que la configuration est bonne -> la mémorise
  if (m_rs232_listener.isOpen()) {
    // récupère la config
    tConfigRS232 config;
    QString portname;
    serialIHMToConfig(portname, config);

    m_application->m_eeprom->write(getName(), "portname", QVariant(portname));
    m_application->m_eeprom->write(getName(), "baudrate", QVariant((int)config.baudrate));
    m_application->m_eeprom->write(getName(), "databits", QVariant((int)config.databits));
    m_application->m_eeprom->write(getName(), "flowcontrol", QVariant((int)config.flowcontrol));
    m_application->m_eeprom->write(getName(), "parity", QVariant((int)config.parity));
    m_application->m_eeprom->write(getName(), "stopbits", QVariant((int)config.stopbits));
    m_application->m_eeprom->write(getName(), "echo_enabled", QVariant((int)config.echo_enabled));
  }

  m_rs232_listener.stopCommunication();
}






// =======================================================
//          MANIPULATION DE LA LIAISON SERIE
// =======================================================




// =======================================================
//      RECEPTION ET EMISSION SUR LA LIAISON SERIE
// =======================================================


// _____________________________________________________________________
/*!
*
*/
void CRS232::write(const QByteArray &data)
{
 m_rs232_listener.send(data);

 // Affiche les données sortantes dans le menu des données entrantes et sortantes
 if (m_simu_rx_tx_ON) {
    m_simuTx_writer->append(data);
 }
}

// _____________________________________________________________________
/*!
*
*/
void CRS232::write(const char *data, qint64 size)
{
  write(QByteArray(data, size));
}

// _____________________________________________________________________
/*!
* Empile une donnée dans la FIFO des data à transmettre sur la RS232
* ainsi que dans la FIFO des données pour l'IHM (affichage ou enregistrement dans un fichier)
*/
void CRS232::write(char data)
{
  write(QByteArray(1, data));
}

// _____________________________________________________________________
/*!
* Données reçues de la RS232 physique
*/
void CRS232::reveive_rx_data(QByteArray data)
{
  if (m_terminal_ON)    { m_terminal->putData(data); }
  if (m_hex_edit_ON)    { m_hex_edit_writer->append(data); }
}



// =======================================================
//       IHM DE CONFIGURATION DES PARAMETRES RS232
// =======================================================

// _____________________________________________________________________
/*!
*
*
*/
void CRS232::procFillingOptions(void)
{
    m_ihm.ui.baudBox->addItem(tr("1200"), QSerialPort::Baud1200);
    m_ihm.ui.baudBox->addItem(tr("2400"), QSerialPort::Baud2400);
    m_ihm.ui.baudBox->addItem(tr("4800"), QSerialPort::Baud4800);
    m_ihm.ui.baudBox->addItem(tr("9600"), QSerialPort::Baud9600);
    m_ihm.ui.baudBox->addItem(tr("19200"), QSerialPort::Baud19200);
    m_ihm.ui.baudBox->addItem(tr("38400"), QSerialPort::Baud38400);
    m_ihm.ui.baudBox->addItem(tr("57600"), QSerialPort::Baud57600);
    m_ihm.ui.baudBox->addItem(tr("115200"), QSerialPort::Baud115200);
    m_ihm.ui.baudBox->addItem(tr("Unknown"), QSerialPort::UnknownBaud);

    m_ihm.ui.dataBox->addItem(tr("5"), QSerialPort::Data5);
    m_ihm.ui.dataBox->addItem(tr("6"), QSerialPort::Data6);
    m_ihm.ui.dataBox->addItem(tr("7"), QSerialPort::Data7);
    m_ihm.ui.dataBox->addItem(tr("8"), QSerialPort::Data8);
    m_ihm.ui.dataBox->addItem(tr("Unknown"), QSerialPort::UnknownDataBits);

    m_ihm.ui.parityBox->addItem(tr("None"), QSerialPort::NoParity);
    m_ihm.ui.parityBox->addItem(tr("Even"), QSerialPort::EvenParity);
    m_ihm.ui.parityBox->addItem(tr("Odd"), QSerialPort::OddParity);
    m_ihm.ui.parityBox->addItem(tr("Mark"), QSerialPort::MarkParity);
    m_ihm.ui.parityBox->addItem(tr("Space"), QSerialPort::SpaceParity);
    m_ihm.ui.parityBox->addItem(tr("Unknown"), QSerialPort::UnknownParity);

    m_ihm.ui.stopBox->addItem(tr("1"), QSerialPort::OneStop);
    m_ihm.ui.stopBox->addItem(tr("1.5"), QSerialPort::OneAndHalfStop);
    m_ihm.ui.stopBox->addItem(tr("2"), QSerialPort::TwoStop);
    m_ihm.ui.stopBox->addItem(tr("Unknown"), QSerialPort::UnknownStopBits);

    m_ihm.ui.flowBox->addItem(tr("Off"), QSerialPort::NoFlowControl);
    m_ihm.ui.flowBox->addItem(tr("Hardware"), QSerialPort::HardwareControl);
    m_ihm.ui.flowBox->addItem(tr("Software"), QSerialPort::SoftwareControl);
    m_ihm.ui.flowBox->addItem(tr("Unknown"), QSerialPort::UnknownFlowControl);

    m_ihm.ui.policyBox->addItem(tr("Skip"), QSerialPort::SkipPolicy);
    m_ihm.ui.policyBox->addItem(tr("PassZero"), QSerialPort::PassZeroPolicy);
    m_ihm.ui.policyBox->addItem(tr("Ignore"), QSerialPort::IgnorePolicy);
    m_ihm.ui.policyBox->addItem(tr("StopReceiving"), QSerialPort::StopReceivingPolicy);
    m_ihm.ui.policyBox->addItem(tr("Unknown"), QSerialPort::UnknownPolicy);

    m_ihm.ui.echoBox->addItem(tr("Off"), 0);
    m_ihm.ui.echoBox->addItem(tr("On"), 1);


    m_list = QSerialPortInfo::availablePorts();
    foreach(QSerialPortInfo info, m_list) {
         m_ihm.ui.portCOM->addItem(info.portName());
    }
    m_ihm.ui.portCOM->addItem(C_VIRTUAL_PORT_COM); // ajoute un port virtuel pour la simulation

}


// _____________________________________________________________________
/*!
*
*
*/
void CRS232::serialConfigToIHM(QString portname, tConfigRS232 config)
{
int value = 0;

   int count = m_ihm.ui.portCOM->count();
   for (int i = 0; i < count; ++i) {
       if (m_ihm.ui.portCOM->itemText(i) == portname) {
           m_ihm.ui.portCOM->setCurrentIndex(i);
           portCOMChanged(i);
           break;
       }
   }

   value = config.baudrate;
   switch (value) {
   case QSerialPort::Baud9600:
   case QSerialPort::Baud19200:
   case QSerialPort::Baud38400:
   case QSerialPort::Baud57600:
   case QSerialPort::Baud115200:
       break;
   default: value = QSerialPort::UnknownBaud;
   }
   count = m_ihm.ui.baudBox->count();
   for (int i = 0; i < count; ++i) {
       bool ok;
       if (m_ihm.ui.baudBox->itemData(i).toInt(&ok) == value) {
           m_ihm.ui.baudBox->setCurrentIndex(i);
           break;
       }
   }

   value = config.databits;
   switch (value) {
   case QSerialPort::Data5:
   case QSerialPort::Data6:
   case QSerialPort::Data7:
   case QSerialPort::Data8:
       break;
   default: value = QSerialPort::UnknownDataBits;
   }
   count = m_ihm.ui.dataBox->count();
   for (int i = 0; i < count; ++i) {
       bool ok;
       if (m_ihm.ui.dataBox->itemData(i).toInt(&ok) == value) {
           m_ihm.ui.dataBox->setCurrentIndex(i);
           break;
       }
   }

   value = config.parity;
   switch (value) {
   case QSerialPort::NoParity:
   case QSerialPort::EvenParity:
   case QSerialPort::OddParity:
   case QSerialPort::MarkParity:
   case QSerialPort::SpaceParity:
       break;
   default: value = QSerialPort::UnknownParity;
   }
   count = m_ihm.ui.parityBox->count();
   for (int i = 0; i < count; ++i) {
       bool ok;
       if (m_ihm.ui.parityBox->itemData(i).toInt(&ok) == value) {
           m_ihm.ui.parityBox->setCurrentIndex(i);
           break;
       }
   }

   value = config.stopbits;
   switch (value) {
   case QSerialPort::OneStop:
   case QSerialPort::OneAndHalfStop:
   case QSerialPort::TwoStop:
       break;
   default: value = QSerialPort::UnknownStopBits;
   }
   count = m_ihm.ui.stopBox->count();
   for (int i = 0; i < count; ++i) {
       bool ok;
       if (m_ihm.ui.stopBox->itemData(i).toInt(&ok) == value) {
           m_ihm.ui.stopBox->setCurrentIndex(i);
           break;
       }
   }

   value = config.flowcontrol;
   switch (value) {
   case QSerialPort::NoFlowControl:
   case QSerialPort::HardwareControl:
   case QSerialPort::SoftwareControl:
       break;
   default: value = QSerialPort::UnknownFlowControl;
   }
   count = m_ihm.ui.flowBox->count();
   for (int i = 0; i < count; ++i) {
       bool ok;
       if (m_ihm.ui.flowBox->itemData(i).toInt(&ok) == value) {
           m_ihm.ui.flowBox->setCurrentIndex(i);
           break;
       }
   }

   value = config.dataerrorpolicy;
   switch (value) {
   case QSerialPort::PassZeroPolicy:
   case QSerialPort::IgnorePolicy:
   case QSerialPort::StopReceivingPolicy:
       break;
   default: value = QSerialPort::UnknownPolicy;
   }
   count = m_ihm.ui.policyBox->count();
   for (int i = 0; i < count; ++i) {
       bool ok;
       if (m_ihm.ui.policyBox->itemData(i).toInt(&ok) == value) {
           m_ihm.ui.policyBox->setCurrentIndex(i);
           break;
       }
   }

   value = config.echo_enabled;
   m_ihm.ui.echoBox->setCurrentIndex(value);
}

// _____________________________________________________________________
/*! Recupère les données de configuration du port de l'IHM
*
*
*/
void CRS232::serialIHMToConfig(QString &portname, tConfigRS232 &config)
{
 bool ok;
 int val;

 portname  = m_ihm.ui.portCOM->currentText();

 val = m_ihm.ui.baudBox->itemData(m_ihm.ui.baudBox->currentIndex()).toInt(&ok);
 config.baudrate = QSerialPort::BaudRate(val);

 val = m_ihm.ui.dataBox->itemData(m_ihm.ui.dataBox->currentIndex()).toInt(&ok);
 config.databits = QSerialPort::DataBits(val);

 val = m_ihm.ui.parityBox->itemData(m_ihm.ui.parityBox->currentIndex()).toInt(&ok);
 config.parity = QSerialPort::Parity(val);

 val = m_ihm.ui.stopBox->itemData(m_ihm.ui.stopBox->currentIndex()).toInt(&ok);
 config.stopbits = QSerialPort::StopBits(val);

 val = m_ihm.ui.flowBox->itemData(m_ihm.ui.flowBox->currentIndex()).toInt(&ok);
 config.flowcontrol = QSerialPort::FlowControl(val);

 val = m_ihm.ui.policyBox->itemData(m_ihm.ui.policyBox->currentIndex()).toInt(&ok);
 config.dataerrorpolicy = QSerialPort::DataErrorPolicy(val);

 val = m_ihm.ui.echoBox->itemData(m_ihm.ui.echoBox->currentIndex()).toInt(&ok);
 config.echo_enabled = val;
}

// _____________________________________________________________
void CRS232::openButtonClick(void)
{
 tConfigRS232 config;
 QString portname;

 // récupère la config
 serialIHMToConfig(portname, config);

 // ouvre le port de communication
 m_rs232_listener.startCommunication(portname, config);
}

// _____________________________________________________________
void CRS232::closeButtonClick(void)
{
 m_rs232_listener.stopCommunication();
}
// _____________________________________________________________
void CRS232::portCOMChanged(int index)
{
  QString portname  = m_ihm.ui.portCOM->currentText();

  if (portname == C_VIRTUAL_PORT_COM) {
    m_ihm.ui.portDescription->setText("Port COM virtuel (simulation)");
  }
  else { // c'est un vrai port COM
    m_ihm.ui.portDescription->setText(m_list.at(index).description());
  }
}

// _____________________________________________________________
// La liaison série est connectée, grise les boutons
void CRS232::serialConnected(void)
{
 m_ihm.ui.groupConfigPortCom->setEnabled(false);
 m_ihm.ui.CloseButton->setEnabled(true);
 m_ihm.ui.OpenButton->setEnabled(false);

 m_ihm.ui.HexEditorRxTx->setEnabled(true);
 m_ihm.ui.Terminal->setEnabled(true);
 //m_ihm.ui.SimuRxTx->setEnabled(true);

 // Efface le conteu
 terminalInit();

 // Initialise la zone HexEdit
 hexEditInit();
 simuRxTxInit();

 //
 m_ihm.ui.menuSimulation->setEnabled(true);

 m_ihm.ui.statusbar->showMessage("Connected");
}
// _____________________________________________________________
void CRS232::serialDisconnected(void)
{
 m_ihm.ui.groupConfigPortCom->setEnabled(true);
 m_ihm.ui.CloseButton->setEnabled(false);
 m_ihm.ui.OpenButton->setEnabled(true);

 m_ihm.ui.HexEditorRxTx->setEnabled(false);
 m_ihm.ui.Terminal->setEnabled(false);
 //m_ihm.ui.SimuRxTx->setEnabled(false);

 // Inhibe les fonctions terminal / HexEdit et Simulation
 m_ihm.ui.actionTerminal_OnOff->setChecked(false);
 m_ihm.ui.actionHexEdit_OnOff->setChecked(false);
 m_ihm.ui.actionSimuRxTx_OnOff->setChecked(false);

 //
 m_ihm.ui.menuSimulation->setEnabled(false);

 m_ihm.ui.statusbar->showMessage("Disconnected");
}


// =======================================================
//                      ONGLET TERMINAL
// =======================================================

// _____________________________________________________________
void CRS232::terminalInit(void)
{
 // Efface le contenu de la zone
 m_terminal->clear();
}

// _____________________________________________________________
void CRS232::Terminal_OnOff(bool state)
{
 // Grise / dégris l'onglet
 m_ihm.ui.Terminal->setEnabled(state);
 m_terminal_ON = state;

 if (state) {
    terminalInit();
    m_ihm.ui.tabWidget->setCurrentIndex(1);  // se positionne automatiquement sur l'onglet
    connect(m_terminal, SIGNAL(getData(QByteArray)), this, SLOT(write(QByteArray)));
 }
 else {
    disconnect(m_terminal, SIGNAL(getData(QByteArray)), this, SLOT(write(QByteArray)));
 }
}

// _____________________________________________________________
void CRS232::Terminal_Clear(void)
{
 // Efface le contenu de la zone
 m_terminal->clear();
}


// _____________________________________________________________
// active l'écho local : lorsque l'utilisateur saisit une donnée,
// elle est affichée dans la console
void CRS232::Terminal_EchoLocal(bool state)
{
  m_terminal->setLocalEchoEnabled(state);
}



// _____________________________________________________________
// Enregistre les données du terminal dans un fichier texte
void CRS232::Terminal_Sauvegarder(void)
{
  QString pathfilename;

  // Construit une chaine de type :
  //    CheminEnregistrement/<NomModule>_Terminal_<DateHeure>.txt
  pathfilename =    m_application->m_pathname_log_file +
                      "/" +
                      QString(getName()).replace(" ", "") + // Supprime les espaces dans le nom généré
                      "_Terminal_" +
                      CToolBox::getDateTime() +
                      ".txt";
  QFile file(pathfilename);
  if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        m_application->m_print_view->print_error(this, "Impossible d'ouvrir en écriture le fichier " + pathfilename);
        return;
  }

  QTextStream out(&file);
  out <<  m_terminal->toPlainText();
  file.close();

  m_ihm.ui.statusbar->showMessage("Fichier sauvegarde: " + pathfilename);
}

// _____________________________________________________________
void CRS232::Terminal_EnvoyerFichierTexte(void)
{
 QString fileName = QFileDialog::getOpenFileName(&m_ihm,
     tr("Fichier texte a transmettre"), "./", "", 0, 0);

 if (fileName != "") {
     // Récupère le contenu du fichier et l'envoie
     QFile data(fileName);
     if (data.open(QFile::ReadOnly)) {
         QTextStream in(&data);
         write(in.readAll().toLocal8Bit()); // envoie des données vers le port
     }
     data.close();
 }
}

// =======================================================
//                      ONGLET HexEdit
// =======================================================

// _____________________________________________________________
void CRS232::HexEdit_OnOff(bool state)
{
 // Grise / dégris l'onglet
 m_ihm.ui.HexEditorRxTx->setEnabled(state);
 m_hex_edit_ON = state;

 if (state) {
    hexEditInit();
    m_ihm.ui.tabWidget->setCurrentIndex(2);  // se positionne automatiquement sur l'onglet
 }

 m_application->m_print_view->print_warning(this, "HexEdit_OnOff");
}


// _____________________________________________________________
void CRS232::HexEdit_Clear(void)
{
  hexEditInit();
  m_application->m_print_view->print_warning(this, "HexEdit_Clear");

}



// _____________________________________________________________
void CRS232::TX_send(void)
{
  QByteArray byteArray;

  // La ligne est directement la chaine à envoyer
  if (m_ihm.ui.TX_select_String->isChecked()) {
    byteArray.append(m_ihm.ui.TX_value->text());
  }
  // Convertit la ligne en valeurs numériques
  else if (m_ihm.ui.TX_select_Values->isChecked()) {
    QStringList list = m_ihm.ui.TX_value->text().split(" ");
    for (int i=0; i<list.size(); i++) {
        bool ok = false;
        char val = list.at(i).toInt(&ok, 0);
        if (ok) {
            byteArray.append(val);
        }
    } // for toutes les valeurs à écrire
  } // if la ligne doit être considérée comme valeur numérique
  write(byteArray);
}



// _____________________________________________________________
void CRS232::hexEditInit(void)
 {
   m_hex_edit_byte_array.clear();
   if (m_hex_edit_writer != NULL)       { delete m_hex_edit_writer; }

   m_hex_edit_data = QHexEditData::fromMemory(m_hex_edit_byte_array);
   m_hex_edit_writer = new QHexEditDataWriter(m_hex_edit_data);
   m_ihm.ui.hexEdit->setData(m_hex_edit_data);
}



// _____________________________________________________________
// Sauvegarde le contenu du HexEdit dans un fichier texte ASCII
void CRS232::HexEdit_SauvegarderBrut(void)
{
  QString pathfilename;
  QHexEditDataReader reader(m_hex_edit_data);

  // Construit une chaine de type :
  //    CheminEnregistrement/<NomModule>_Terminal_<DateHeure>.txt
  pathfilename =    m_application->m_pathname_log_file +
                        "/" +
                        QString(getName()).replace(" ", "") + // Supprime les espaces dans le nom généré
                        "_HexEditBrut" +
                        CToolBox::getDateTime() +
                        ".txt";
  QFile file(pathfilename);
  if (!file.open(QIODevice::WriteOnly)) {
          m_application->m_print_view->print_error(this, "Impossible d'ouvrir en écriture le fichier " + pathfilename);
          return;
  }
  QByteArray byteArray = reader.read(0, m_hex_edit_data->length());
  file.write(byteArray);

  m_ihm.ui.statusbar->showMessage("Fichier sauvegarde: " + pathfilename + " (" + QString::number((byteArray.size())) + " octets)");
}

// _____________________________________________________________
// Sauvegarde le contenu du HexEdit dans un fichier texte Hexa
void CRS232::HexEdit_SauvegarderHexASCII(void)
{
  QString pathfilename;
  QHexEditDataReader reader(m_hex_edit_data);

  // Construit une chaine de type :
  //    CheminEnregistrement/<NomModule>_Terminal_<DateHeure>.txt
  pathfilename =    m_application->m_pathname_log_file +
                          "/" +
                          QString(getName()).replace(" ", "") + // Supprime les espaces dans le nom généré
                          "_HexEditHEXA_" +
                          CToolBox::getDateTime() +
                          ".txt";
  QFile file(pathfilename);
  if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            m_application->m_print_view->print_error(this, "Impossible d'ouvrir en écriture le fichier " + pathfilename);
            return;
  }

  QTextStream out(&file);
  QByteArray byteArray;
  for (qint64 i=0; i<m_hex_edit_data->length(); i++) {
      if ( (i!=0) && ((i%16) == 0) ) { out << "\n"; }
      byteArray = reader.read(i, 1);
      out << QString(byteArray.toHex().toUpper()) << " ";
  }

  file.close();
  m_ihm.ui.statusbar->showMessage("Fichier sauvegarde: " + pathfilename);
}


// _____________________________________________________________
// Copie dans le presse papier les données brutes
void CRS232::HexEdit_CopyBrut(void)
{
  m_ihm.ui.hexEdit->copy();
}

// _____________________________________________________________
// Copie dans le presse papier les données au format hexa affichable
void CRS232::HexEdit_CopyHexASCII(void)
{
  m_ihm.ui.hexEdit->copyHex();
}


// _____________________________________________________________
// Envoie sur le port un fichier brut
void CRS232::HexEdit_EnvoyerFichierFormatBrut(void)
{
 QString fileName = QFileDialog::getOpenFileName(&m_ihm,
    tr("Fichier brut a transmettre"), "./", "", 0, 0);

 if (fileName != "") {
    // Récupère le contenu du fichier et l'envoie
    QFile data(fileName);
    if (data.open(QFile::ReadOnly)) {
        QTextStream in(&data);
        QByteArray byteArray = in.readAll().toLocal8Bit();
        write(byteArray); // envoie des données vers le port
        m_ihm.ui.statusbar->showMessage(QString::number(byteArray.size()) + " octets envoyes");
    }
    data.close();
 }
}

// _____________________________________________________________
// Envoie sur le port un fichier au format Hexa ASCII :
//  0x12 0x01 0x56 ....
void CRS232::HexEdit_EnvoyerFichierFormatHexASCII(void)
{
  bool ok;
  char val;
  QByteArray byte_array;
  QString fileName = QFileDialog::getOpenFileName(&m_ihm,
       tr("Fichier brut a transmettre"), "./", "", 0, 0);

  if (fileName != "") {
       // Récupère le contenu du fichier et l'envoie
       QFile data(fileName);
       if (data.open(QFile::ReadOnly)) {
           QTextStream in(&data);
           QStringList str_list = in.readAll().split(QRegularExpression("\\s+"));  // découpe en éliminants espaces, tabulation, retours chariots
            for (int i=0; i<str_list.size(); i++) {
            val = str_list.at(i).toInt(&ok, 16);
            if (ok) { byte_array.append(val); }
           } // for tous les octets lus
           qDebug() << "Array : " << byte_array;
           write(byte_array);
       }
       data.close();
       m_ihm.ui.statusbar->showMessage(QString::number(byte_array.size()) + " octets envoyes");
  }
}



// =======================================================
//                      ONGLET SIMU RX TX
// =======================================================


// _____________________________________________________________
void CRS232::simuRxTxInit(void)
 {
   m_simuTx_byte_array.clear();
   if (m_simuTx_writer != NULL)       { delete m_simuTx_writer; }

   m_simuTx_data = QHexEditData::fromMemory(m_simuTx_byte_array);
   m_simuTx_writer = new QHexEditDataWriter(m_simuTx_data);
   m_ihm.ui.simuTx_hexEdit->setData(m_simuTx_data);
}



// _____________________________________________________________
void CRS232::SimuRxTx_OnOff(bool state)
{
  // Grise / dégris l'onglet
  m_ihm.ui.SimuRxTx->setEnabled(state);
  m_simu_rx_tx_ON = state;

  if (state) {
    simuRxTxInit();
    m_ihm.ui.tabWidget->setCurrentIndex(3);  // se positionne automatiquement sur l'onglet Simu
  }
}

// _____________________________________________________________
void CRS232::VisuTx_Clear(void)
{
  simuRxTxInit();
}

// _____________________________________________________________
// Simule des données entrantes vers l'applicatif.
// Les données sont récupérées d'un fichier brut
void CRS232::SimuRx_DonneesEntrantesFichierBrut(void)
{
  QString fileName = QFileDialog::getOpenFileName(&m_ihm,
       tr("Fichier brut a transmettre"), "./", "", 0, 0);

  if (fileName != "") {
       // Récupère le contenu du fichier et l'envoie
       QFile data(fileName);
       if (data.open(QFile::ReadOnly)) {
           QTextStream in(&data);
           QByteArray byteArray = in.readAll().toLocal8Bit();
           emit readyBytes(byteArray); // envoie les données vers l'applicatif
           m_ihm.ui.statusbar->showMessage(QString::number(byteArray.size()) + " octets entrants simules");
       }
       data.close();
  }
}


// _____________________________________________________________
// Simule des données entrantes vers l'applicatif.
// Les données sont récupérées d'un fichier Hex ASCII
// 0x12 0xfe ...
void CRS232::SimuRx_DonneesEntrantesFichierHexASCII(void)
{
  bool ok;
  char val;
  QByteArray byte_array;
  QString fileName = QFileDialog::getOpenFileName(&m_ihm,
       tr("Fichier brut a transmettre"), "./", "", 0, 0);

  if (fileName != "") {
       // Récupère le contenu du fichier et l'envoie
       QFile data(fileName);
       if (data.open(QFile::ReadOnly)) {
           QTextStream in(&data);
           QStringList str_list = in.readAll().split(QRegularExpression("\\s+"));
            for (int i=0; i<str_list.size(); i++) {
            val = str_list.at(i).toInt(&ok, 16);
            if (ok) { byte_array.append(val); }
           } // for tous les octets lus
           qDebug() << "Array : " << byte_array;
           emit readyBytes(byte_array);
       }
       data.close();
       m_ihm.ui.statusbar->showMessage(QString::number(byte_array.size()) + " octets entrants simules");
  }
}

// _____________________________________________________________
void CRS232::VisuTx_SauvegarderBrut(void)
{
    QString pathfilename;
    QHexEditDataReader reader(m_simuTx_data);

    // Construit une chaine de type :
    //    CheminEnregistrement/<NomModule>_VisuTxBrut<DateHeure>.txt
    pathfilename =    m_application->m_pathname_log_file +
                          "/" +
                          QString(getName()).replace(" ", "") + // Supprime les espaces dans le nom généré
                          "_VisuTxBrut_" +
                          CToolBox::getDateTime() +
                          ".txt";
    QFile file(pathfilename);
    if (!file.open(QIODevice::WriteOnly)) {
            m_application->m_print_view->print_error(this, "Impossible d'ouvrir en écriture le fichier " + pathfilename);
            return;
    }
    QByteArray byteArray = reader.read(0, m_simuTx_data->length());
    file.write(byteArray);
    file.close();

    m_ihm.ui.statusbar->showMessage("Fichier sauvegarde: " + pathfilename + " (" + QString::number((byteArray.size())) + " octets)");
}

// _____________________________________________________________
void CRS232::VisuTx_SauvegarderHexASCII(void)
{
    QString pathfilename;
    QHexEditDataReader reader(m_simuTx_data);

    // Construit une chaine de type :
    //    CheminEnregistrement/<NomModule>_Terminal_<DateHeure>.txt
    pathfilename =    m_application->m_pathname_log_file +
                            "/" +
                            QString(getName()).replace(" ", "") + // Supprime les espaces dans le nom généré
                            "_VisuTxHEXA_" +
                            CToolBox::getDateTime() +
                            ".txt";
    QFile file(pathfilename);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
              m_application->m_print_view->print_error(this, "Impossible d'ouvrir en écriture le fichier " + pathfilename);
              return;
    }

    QTextStream out(&file);
    QByteArray byteArray;
    for (qint64 i=0; i<m_simuTx_data->length(); i++) {
        if ( (i!=0) && ((i%16) == 0) ) { out << "\n"; }
        byteArray = reader.read(i, 1);
        out << QString(byteArray.toHex().toUpper()) << " ";
    }

    file.close();
    m_ihm.ui.statusbar->showMessage("Fichier sauvegarde: " + pathfilename);
}


// _____________________________________________________________
void CRS232::SimuRX_send(void)
{
  QByteArray byteArray;

  // La ligne est directement la chaine à envoyer
  if (m_ihm.ui.SimuRX_select_String->isChecked()) {
    byteArray.append(m_ihm.ui.SimuRX_value->text());
  }
  // Convertit la ligne en valeurs numériques
  else if (m_ihm.ui.SimuRX_select_Values->isChecked()) {
    QStringList list = m_ihm.ui.SimuRX_value->text().split(" ");
    for (int i=0; i<list.size(); i++) {
        bool ok = false;
        char val = list.at(i).toInt(&ok, 0);
        if (ok) {
            byteArray.append(val);
        }
    } // for toutes les valeurs à écrire
  } // if la ligne doit être considérée comme valeur numérique
  emit readyBytes(byteArray); // simule les données entrantes vers l'applicatif
}





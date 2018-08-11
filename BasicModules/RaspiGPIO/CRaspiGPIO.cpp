/*! \file CRaspiGPIO.cpp
 * A brief file description CPP.
 * A more elaborated file description.
 */
#include <QDebug>

#include "CRaspiGPIO.h"
#include "CApplication.h"
#include "CPrintView.h"
#include "CMainWindow.h"
#include "CEEPROM.h"
#include "CDataManager.h"

#include <wiringPi.h>



/*! \addtogroup RASPI_C
   * 
   *  @{
   */
	   
// _____________________________________________________________________
/*!
*  Constructeur
*
*/
CRaspiGPIO::CRaspiGPIO(const char *plugin_name)
    :CBasicModule(plugin_name, VERSION_RaspiGPIO, AUTEUR_RaspiGPIO, INFO_RaspiGPIO)
{
}


// _____________________________________________________________________
/*!
*  Destructeur
*
*/
CRaspiGPIO::~CRaspiGPIO()
{

}


// _____________________________________________________________________
/*!
*  Initialisation du module
*
*/
void CRaspiGPIO::init(CApplication *application)
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

  wiringPiSetupGpio(); // Utilise la numérotation de pin BCM

  for (unsigned int i=0; i<MAX_GPIO_COUNT; i++) {
      // Recherche dans le fichier EEPROM une configuration de type "gpio_<i>=input pull down"
      // gpio_17 = input
      QString eeprom_config_name = "gpio_" + QString::number(i);
      val = m_application->m_eeprom->read(getName(), eeprom_config_name, QVariant());
      if (val != QVariant()) {  // le paramètre a été trouvé -> le GPIO est utilisé et doit être configuré
          configPinMode(i, val.toString());
      }

      // Initialise la liste des GPIO dans la liste déroulante de configuration
      // on part des contrôles affichés sur l'interface et on récupère les numéros
      // Rensigne dans la liste déroulante des GPIO tous les numéros possibles de GPIO
      QLabel *lbl = m_ihm.findChild<QLabel*>(PREFIX_MODE_GPIO + QString::number(i)); // "mode_GPIO_<i>" c'est le nom de tous les labels des GPIO sur l'IHM pour indiquer si c'est une entrée / sortie / ...
      if (lbl) {
          m_ihm.ui.configChoixGPIO->addItem("GPIO " + QString::number(i));
      }
      // connexion signal/slot avec toutes les checkbox qui permettent d'écrire sur le port
      // on repert toutes les checkbox par leur nom "write_GPIO_<i>"
      QCheckBox *checkbox = m_ihm.findChild<QCheckBox*>(PREFIX_CHECKBOX_WRITE + QString::number(i));
      if (checkbox) {
          connect(checkbox, SIGNAL(clicked(bool)), this, SLOT(onWritePinChange(bool)));
      }
  }

  // Renseigne dans la liste déroulante des configurations possibles
  QStringList list_modes;
  list_modes << "" << "Input" << "Input pull down" << "Input pull up" << "Output" << "PWM Output";
  m_ihm.ui.configChoixMode->addItems(list_modes);

  connect(m_ihm.ui.configChoixGPIO, SIGNAL(activated(int)), this, SLOT(onChoixConfigGPIO()));
  connect(m_ihm.ui.configChoixMode, SIGNAL(activated(QString)), this, SLOT(onChoixConfigMode(QString)));
  connect(&m_timer_read, SIGNAL(timeout()), this, SLOT(readAllInputs()));
  m_timer_read.start(100);
}


// _____________________________________________________________________
/*!
*  Fermeture du module
*
*/
void CRaspiGPIO::close(void)
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
void CRaspiGPIO::onRightClicGUI(QPoint pos)
{
  QMenu *menu = new QMenu();

  menu->addAction("Select background color", this, SLOT(selectBackgroundColor()));
  menu->exec(m_ihm.mapToGlobal(pos));
}


// _____________________________________________________________________
void CRaspiGPIO::configPinMode(unsigned int gpio_num, QString mode)
{
    if (mode.toLower() == "output") {
        setPinModeOutput(gpio_num, 0);
        m_application->m_print_view->print_debug(this, "GPIO " + QString::number(gpio_num) + " is Digital Output");
    }
    else if (mode.toLower() == "pwm output") {
        setPinModePWMOutput(gpio_num, 0);
        m_application->m_print_view->print_debug(this, "GPIO " + QString::number(gpio_num) + " is PWM Outpur");
    }
    else if (mode.toLower() == "input pull up") {
        setPinModeInput(gpio_num, PUD_UP);
        m_application->m_print_view->print_debug(this, "GPIO " + QString::number(gpio_num) + " is Input with Pull up");
    }
    else if (mode.toLower() == "input pull down") {
        setPinModeInput(gpio_num, PUD_DOWN);
        m_application->m_print_view->print_debug(this, "GPIO " + QString::number(gpio_num) + " is Input with Pull down");
    }
    else if (mode.toLower() == "input") {
        setPinModeInput(gpio_num, PUD_OFF);
        m_application->m_print_view->print_debug(this, "GPIO " + QString::number(gpio_num) + " is Input");
    }
    else { // mode inconnu, laisse le GPIO dans son état courant
        m_application->m_print_view->print_error(this, "Unknown mode \"" + mode + "\" for GPIO " + QString::number(gpio_num));
    }
}

// _____________________________________________________________________
void CRaspiGPIO::setPinModeInput(unsigned int gpio_num, unsigned int pullup_down)
{
    pinMode(gpio_num, INPUT);
    pullUpDnControl(gpio_num, pullup_down);
    QString txt;
    QString tooltip;
    if (pullup_down == PUD_UP) {
        txt+="Iu";
        tooltip = "Input with pull UP";
    }
    else if (pullup_down == PUD_DOWN) {
        txt+="Id";
        tooltip = "Input with pull DOWN";
    }
    else {
        txt+="I";
        tooltip = "Digital Input";
    }
    QLabel *lbl = m_ihm.findChild<QLabel*>(PREFIX_MODE_GPIO + QString::number(gpio_num));
    if (lbl) {
        lbl->setText(txt);
        lbl->setToolTip(tooltip);
        lbl->setEnabled(true);
    }
    QLabel *lbl_name = m_ihm.findChild<QLabel*>(PREFIX_GPIO_NAME + QString::number(gpio_num));
    if (lbl_name) {
        lbl_name->setEnabled(true);
    }

    // On ne peut pas forcer l'état sur une entrée
    QCheckBox *checkbox = m_ihm.findChild<QCheckBox*>(PREFIX_CHECKBOX_WRITE + QString::number(gpio_num));
    if (checkbox) {
        checkbox->setEnabled(false);
    }

    m_raspi_pins_config[gpio_num] = tRaspiPinConfig { INPUT, pullup_down, 0};
    readPin(gpio_num);
}

// _____________________________________________________________________
void CRaspiGPIO::setPinModeOutput(unsigned int gpio_num, unsigned int init_state)
{
    pinMode(gpio_num, OUTPUT);
    digitalWrite(gpio_num, init_state);

    QString txt = "O";
    QString tooltip = "Digital Output";
    QLabel *lbl = m_ihm.findChild<QLabel*>(PREFIX_MODE_GPIO + QString::number(gpio_num));
    if (lbl) {
        lbl->setText(txt);
        lbl->setToolTip(tooltip);
        lbl->setEnabled(true);
    }
    QLabel *lbl_name = m_ihm.findChild<QLabel*>(PREFIX_GPIO_NAME + QString::number(gpio_num));
    if (lbl_name) {
        lbl_name->setEnabled(true);
    }

    QCheckBox *checkbox = m_ihm.findChild<QCheckBox*>(PREFIX_CHECKBOX_WRITE + QString::number(gpio_num));
    if (checkbox) {
        checkbox->setEnabled(true);
    }
    m_raspi_pins_config[gpio_num] = tRaspiPinConfig { OUTPUT, PUD_OFF, init_state};

    // Crée la data dans le data manager et l'associe à une callback pour déclencher l'écriture sur le port sur changement de valeur de la data
    QString dataname = PREFIX_RASPI_OUT_DATANAME + QString::number(gpio_num);
    m_application->m_data_center->write(dataname, init_state);
    CData *data = m_application->m_data_center->getData(dataname);
    if (data) {
        connect (data, SIGNAL(valueUpdated(QVariant)), this, SLOT(onDataChangeWrite(QVariant)));
    }
}

// _____________________________________________________________________
void CRaspiGPIO::setPinModePWMOutput(unsigned int gpio_num, unsigned int init_pwm)
{
    pinMode(gpio_num, PWM_OUTPUT);

    QString txt = "P";
    QString tooltip = "PWM Output";
    QLabel *lbl = m_ihm.findChild<QLabel*>(PREFIX_MODE_GPIO + QString::number(gpio_num));
    if (lbl) {
        lbl->setText(txt);
        lbl->setToolTip(tooltip);
    }
    QLabel *lbl_name = m_ihm.findChild<QLabel*>(PREFIX_GPIO_NAME + QString::number(gpio_num));
    if (lbl_name) {
        lbl_name->setEnabled(true);
    }
    m_raspi_pins_config[gpio_num] = tRaspiPinConfig { PWM_OUTPUT, PUD_OFF, init_pwm};

    // Crée la data dans le data manager et l'associe à une callback pour déclencher l'écriture sur le port sur changement de valeur de la data
    writePwmPin(gpio_num, init_pwm);
    QString dataname = PREFIX_RASPI_OUT_DATANAME + QString::number(gpio_num);
    m_application->m_data_center->write(dataname, init_pwm);
    CData *data = m_application->m_data_center->getData(dataname);
    if (data) {
        connect (data, SIGNAL(valueChanged(double)), this, SLOT(onDataChangeWritePWM(double)));
    }
}

// _____________________________________________________________________
bool CRaspiGPIO::readPin(unsigned int gpio_num)
{
    bool input_state = digitalRead(gpio_num);
    // refresh Data Manager
    QString dataname = PREFIX_RASPI_DATANAME + QString::number(gpio_num);
    m_application->m_data_center->write(dataname, input_state);

    // refresh GUI
    QLed *led = m_ihm.findChild<QLed*>(PREFIX_STATE_GPIO + QString::number(gpio_num));
    if (led) {
        led->setValue(input_state);
    }

    return input_state;
}

// _____________________________________________________________________
void CRaspiGPIO::readAllInputs()
{
    for (tListPins::const_iterator it=m_raspi_pins_config.constBegin(); it!=m_raspi_pins_config.constEnd() ; it++) {
        readPin(it.key());
    }
}

// _____________________________________________________________________
void CRaspiGPIO::writePin(unsigned int gpio_num, bool state)
{
    digitalWrite(gpio_num, state);

/*
    // met en cohérene l'IHM et le data manager avec la nouvelle valeur
    // met à jour la data correspondante dans le data manager
    QString dataname = PREFIX_RASPI_OUT_DATANAME + QString::number(gpio_num);
    CData* data = m_application->m_data_center->getData(dataname);
    if (data) {
        data->write(state);
    }

    // met à jour la checkbox sur l'IHM
    QCheckBox *checkbox = m_ihm.findChild<QCheckBox*>(PREFIX_CHECKBOX_WRITE + QString::number(gpio_num));
    if (checkbox) {
        checkbox->setChecked(state);
    }
    */
}
// _____________________________________________________________________
void CRaspiGPIO::writePwmPin(unsigned int gpio_num, float percent)
{
    pwmWrite(gpio_num, percent);
}


// _____________________________________________________________________
void CRaspiGPIO::onChoixConfigGPIO()
{
    m_ihm.ui.configChoixMode->setCurrentText("");
}

// _____________________________________________________________________
void CRaspiGPIO::onChoixConfigMode(QString mode)
{
    unsigned int gpio_num = m_ihm.ui.configChoixGPIO->currentText().remove("gpio", Qt::CaseInsensitive).toInt();
    configPinMode(gpio_num, mode);
}

// _____________________________________________________________________
void CRaspiGPIO::onWritePinChange(bool state)
{
    QCheckBox* checkbox = qobject_cast<QCheckBox*>(sender());
    if( checkbox != NULL )
    {
        QString objectName = checkbox->objectName();
        bool ok;
        unsigned int gpio_num = objectName.remove(PREFIX_CHECKBOX_WRITE).toInt(&ok);
        if (ok) {
            writePin(gpio_num, state);
        }
    }
}


// _____________________________________________________________________
// Cette callback permet d'effectuer une écriture sur le port lorsqu'une data "RaspiOut_GPIO_<i>"
// est écrite.
void CRaspiGPIO::onDataChangeWrite(QVariant state)
{
    // récupère la data et par son nom, on connaitra le numéro de GPIO concerné
    CData* data = qobject_cast<CData*>(sender());
    if (!data) return;

    bool ok;
    unsigned int gpio_num = data->getName().remove(PREFIX_RASPI_OUT_DATANAME).toInt(&ok);
    if (ok) {
        writePin(gpio_num, state.toBool());
    }
}

// _____________________________________________________________________
void CRaspiGPIO::onDataChangeWritePWM(double percent)
{
    // récupère la data et par son nom, on connaitra le numéro de GPIO concerné
    CData* data = qobject_cast<CData*>(sender());
    if (!data) return;

    bool ok;
    unsigned int gpio_num = data->getName().remove(PREFIX_RASPI_OUT_DATANAME).toInt(&ok);
    if (ok) {
        writePwmPin(gpio_num, (float)percent);
    }
}

/*! \file CJoystick.cpp
 * A brief file description CPP.
 * A more elaborated file description.
 */
#include <QDebug>
#include "CJoystick.h"
#include "CApplication.h"
#include "CPrintView.h"
#include "CMainWindow.h"
#include "CEEPROM.h"
#include "CDataManager.h"



/*! \addtogroup Module_Test2
   * 
   *  @{
   */
	   
// _____________________________________________________________________
/*!
*  Constructeur
*
*/
CJoystick::CJoystick(const char *plugin_name)
    :CBasicModule(plugin_name, VERSION_Joystick, AUTEUR_Joystick, INFO_Joystick)
{
}


// _____________________________________________________________________
/*!
*  Destructeur
*
*/
CJoystick::~CJoystick()
{

}


// _____________________________________________________________________
/*!
*  Initialisation du module
*
*/
void CJoystick::init(CApplication *application)
{
  CModule::init(application);

  // Restore le niveau d'affichage
  QVariant val = m_application->m_eeprom->read(getName(), "niveau_trace", QVariant(MSG_TOUS));
  setNiveauTrace(val.toUInt());

  memset(&m_AxisPosition_old, 0, sizeof(m_AxisPosition_old));
  memset(&m_ButtonState_old, 0, sizeof(m_ButtonState_old));
  memset(&m_connected, 0, sizeof(m_connected));

  // Crée les variables liées au joystick dans le data manager
  QString data_name = "";
  for (unsigned int joy=0; joy<MAX_JOYSTICK_NUMBER; joy++) {
      data_name = QString("Joystick%1_Connected").arg(joy);
      m_application->m_data_center->write(data_name, m_connected[joy]);

      for (unsigned int axis=0; axis<MAX_AXIS_NUMBER; axis++) {
          data_name = QString("Joystick%1_Axis%2").arg(joy).arg(axis);
          m_application->m_data_center->write(data_name, m_AxisPosition_old[joy][axis]);
      }

      for (unsigned int button=0; button<MAX_BUTTON_NUMBER; button++) {
          data_name = QString("Joystick%1_Button%2").arg(joy).arg(button);
          m_application->m_data_center->write(data_name, m_ButtonState_old[joy][button]);
      }
  }

  connect(&m_refresh_timer, SIGNAL(timeout()), this, SLOT(refreshJoysticksState()));
  m_refresh_timer.start(SEARCH_JOYSTICK_PERIOD);
}


// _____________________________________________________________________
/*!
*  Fermeture du module
*
*/
void CJoystick::close(void)
{
    m_application->m_eeprom->write(getName(), "niveau_trace", QVariant((unsigned int)getNiveauTrace()));
}



// _____________________________________________________________________
/*!
*  Mise à jour des états des joysticks
*
*/
void CJoystick::refreshJoysticksState()
{
    int numberjoystickconnected=0;

    // mise à jour sfml
    sf::Joystick::update();

    for (unsigned int joy=0; joy<MAX_JOYSTICK_NUMBER; joy++) {
        // ____________________________________
        // recherche des joysticks nouvellement connectés ou déconnectés
        if (m_connected[joy] != sf::Joystick::isConnected(joy)) {
            m_connected[joy] = sf::Joystick::isConnected(joy);
            QString data_name = QString("Joystick%1_Connected").arg(joy);
            m_application->m_data_center->write(data_name, m_connected[joy]);
            m_application->m_print_view->print_info(this, QString("%1 = %2").arg(data_name).arg(m_connected[joy]));
        }
        // ____________________________________
        // valeur pour chaque axe
        for (unsigned int axis=0; axis<MAX_AXIS_NUMBER; axis++) {
            double position = sf::Joystick::getAxisPosition(joy, (sf::Joystick::Axis)axis);
            if (m_AxisPosition_old[joy][axis] != position) {
                m_AxisPosition_old[joy][axis] = position;
                QString data_name = QString("Joystick%1_Axis%2").arg(joy).arg(axis);
                m_application->m_data_center->write(data_name, position);
                m_application->m_print_view->print_debug(this, QString("%1 = %2").arg(data_name).arg(position));
            }
        }
        // ____________________________________
        // valeur pour chaque bouton
        for (unsigned int button=0; button<MAX_BUTTON_NUMBER; button++) {
            bool button_pressed = sf::Joystick::isButtonPressed(joy, button);
            if (m_ButtonState_old[joy][button] != button_pressed) {
                m_ButtonState_old[joy][button] = button_pressed;
                QString data_name = QString("Joystick%1_Button%2").arg(joy).arg(button);
                m_application->m_data_center->write(data_name, button_pressed);
                m_application->m_print_view->print_debug(this, QString("%1 = %2").arg(data_name).arg(button_pressed));
            }
        }
        // ____________________________________
        // comptabilise le nombre de joystick connectés au moment présent
        numberjoystickconnected += sf::Joystick::isConnected(joy);
    } // for tous les joysticks possibles

    // si au moins un joystick est connecté, augmente la fréquence de lecture du jostick
    m_refresh_timer.setInterval(numberjoystickconnected!=0?REFRESH_PERIOD:SEARCH_JOYSTICK_PERIOD);
}





/*! \file CJoystick.h
 * A brief file description.
 * A more elaborated file description.
 */
#ifndef _CBASIC_MODULE_Joystick_H_
#define _CBASIC_MODULE_Joystick_H_

#include <QMainWindow>
#include <QTimer>

#include "CBasicModule.h"

#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>

 /*! \addtogroup Joystick
   * 
   *  @{
   */

	   
/*! @brief class CJoystick
 */	   
class CJoystick : public CBasicModule
{
    Q_OBJECT
#define     VERSION_Joystick   "1.0"
#define     AUTEUR_Joystick    "Nico"
#define     INFO_Joystick      "Lecture des états des joysticks"

#define     MAX_JOYSTICK_NUMBER 2       // Nombre maximum de joytick gérés par le driver (voir sf::Joystick::Count pour la limite gérée par la lib sfml)
#define     MAX_AXIS_NUMBER     8       // Nombre maximum d'axes gérés par un joystick
#define     MAX_BUTTON_NUMBER   12      // Nombre maximi de bouttons gérés par joystick

#define     REFRESH_PERIOD          50      // msec : période de rafraichissement des données lorsqu'au moins un joystick est connecté
#define     SEARCH_JOYSTICK_PERIOD  1000    // msec : période de recherche des nouveaux joyticks connectés ou déconnectés

public:
    CJoystick(const char *plugin_name);
    ~CJoystick();

    virtual void init(CApplication *application);
    virtual void close(void);
    virtual bool hasGUI(void) { return(false); }
    virtual QIcon getIcon(void) { return(QIcon(":/icons/edit_add.png")); }

private :
    bool   m_connected[MAX_JOYSTICK_NUMBER];
    double m_AxisPosition_old[MAX_JOYSTICK_NUMBER][MAX_AXIS_NUMBER];
    bool   m_ButtonState_old[MAX_JOYSTICK_NUMBER][MAX_BUTTON_NUMBER];

    QTimer m_refresh_timer; // lecture périodique des états du joystick

private slots :
    void refreshJoysticksState();
};

#endif // _CBASIC_MODULE_Joystick_H_

/*! @} */


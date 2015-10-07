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
#define     INFO_Joystick      "Lecture des �tats des joysticks"

#define     MAX_JOYSTICK_NUMBER 2       // Nombre maximum de joytick g�r�s par le driver (voir sf::Joystick::Count pour la limite g�r�e par la lib sfml)
#define     MAX_AXIS_NUMBER     8       // Nombre maximum d'axes g�r�s par un joystick
#define     MAX_BUTTON_NUMBER   12      // Nombre maximi de bouttons g�r�s par joystick

#define     REFRESH_PERIOD          50      // msec : p�riode de rafraichissement des donn�es lorsqu'au moins un joystick est connect�
#define     SEARCH_JOYSTICK_PERIOD  1000    // msec : p�riode de recherche des nouveaux joyticks connect�s ou d�connect�s

public:
    CJoystick(const char *plugin_name);
    ~CJoystick();

    virtual void init(CLaBotBox *application);
    virtual void close(void);
    virtual bool hasGUI(void) { return(false); }
    virtual QIcon getIcon(void) { return(QIcon(":/icons/edit_add.png")); }

private :
    bool   m_connected[MAX_JOYSTICK_NUMBER];
    double m_AxisPosition_old[MAX_JOYSTICK_NUMBER][MAX_AXIS_NUMBER];
    bool   m_ButtonState_old[MAX_JOYSTICK_NUMBER][MAX_BUTTON_NUMBER];

    QTimer m_refresh_timer; // lecture p�riodique des �tats du joystick

private slots :
    void refreshJoysticksState();
};

#endif // _CBASIC_MODULE_Joystick_H_

/*! @} */


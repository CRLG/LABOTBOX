/*! \file CRaspiGPIO.h
 * A brief file description.
 * A more elaborated file description.
 */
#ifndef _CBASIC_MODULE_RaspiGPIO_H_
#define _CBASIC_MODULE_RaspiGPIO_H_

#include <QMainWindow>
#include <QTimer>

#include "CBasicModule.h"
#include "ui_ihm_RaspiGPIO.h"

 class Cihm_RaspiGPIO : public QMainWindow
{
    Q_OBJECT
public:
    Cihm_RaspiGPIO(QWidget *parent = 0)  : QMainWindow(parent) { ui.setupUi(this); }
    ~Cihm_RaspiGPIO() { }

    Ui::ihm_RaspiGPIO ui;

    CApplication *m_application;
 };



 /*! \addtogroup RaspiGPIO
   * 
   *  @{
   */

	   
/*! @brief class CRaspiGPIO
 */	   
class CRaspiGPIO : public CBasicModule
{
    typedef struct {
        unsigned short input_output_mode;  // INPUT / OUTPUT / PWM
        unsigned short pull_up_down;
        bool initial_output_value;
    }tRaspiPinConfig;

    typedef QMap<int, tRaspiPinConfig>tListPins;

    Q_OBJECT
#define     VERSION_RaspiGPIO   "1.0"
#define     AUTEUR_RaspiGPIO    "Nico"
#define     INFO_RaspiGPIO      "Contrôle des GPIO sur Raspberry Pi"

#define MAX_GPIO_COUNT 40
#define PREFIX_RASPI_DATANAME       "RaspiGPIO_"
#define PREFIX_RASPI_OUT_DATANAME   "RaspiOutGPIO_"
#define PREFIX_CHECKBOX_WRITE       "write_GPIO_"
#define PREFIX_MODE_GPIO            "mode_GPIO_"
#define PREFIX_STATE_GPIO           "state_GPIO_"
#define PREFIX_GPIO_NAME            "label_GPIO_"

public:
    CRaspiGPIO(const char *plugin_name);
    ~CRaspiGPIO();

    Cihm_RaspiGPIO *getIHM(void) { return(&m_ihm); }

    virtual void init(CApplication *application);
    virtual void close(void);
    virtual bool hasGUI(void)           { return(true); }
    virtual QIcon getIcon(void)         { return(QIcon(":/icons/raspberrypi_logo.png")); }  // Précise l'icône qui représente le module
    virtual QString getMenuName(void)   { return("BasicModule"); }                  // Précise le nom du menu de la fenêtre principale dans lequel le module apparaît

private:
    Cihm_RaspiGPIO m_ihm;
    tListPins m_raspi_pins_config;
    QTimer m_timer_read;

public slots :
    void configPinMode(unsigned int gpio_num, QString mode);
    void setPinModeInput(unsigned int gpio_num, unsigned int pullup_down);
    void setPinModeOutput(unsigned int gpio_num, unsigned int init_state);
    void setPinModePWMOutput(unsigned int gpio_num, unsigned int init_pwm);

    bool readPin(unsigned int gpio_num);
    void writePin(unsigned int gpio_num, bool state);
    void writePwmPin(unsigned int gpio_num, float percent);

private slots :
    void onRightClicGUI(QPoint pos);
    void onChoixConfigMode(QString mode);
    void onChoixConfigGPIO();
    void onWritePinChange(bool state);
    void readAllInputs();
    void onDataChangeWrite(QVariant state);
    void onDataChangeWritePWM(double percent);
};

#endif // _CBASIC_MODULE_RaspiGPIO_H_

/*! @} */


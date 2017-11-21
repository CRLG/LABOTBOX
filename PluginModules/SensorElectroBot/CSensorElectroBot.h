/*! \file CSensorElectroBot.h
 * A brief file description.
 * A more elaborated file description.
 */
#ifndef _CPLUGIN_MODULE_SensorElectroBot_H_
#define _CPLUGIN_MODULE_SensorElectroBot_H_

#include <QMainWindow>

#include "CPluginModule.h"
#include "ui_ihm_SensorElectroBot.h"

 class Cihm_SensorElectroBot : public QMainWindow
{
    Q_OBJECT
public:
    Cihm_SensorElectroBot(QWidget *parent = 0)  : QMainWindow(parent) { ui.setupUi(this); }
    ~Cihm_SensorElectroBot() { }

    Ui::ihm_SensorElectroBot ui;

    CLaBotBox *m_application;
 };



 /*! \addtogroup SensorElectroBot
   * 
   *  @{
   */

	   
/*! @brief class CSensorElectroBot
 */	   
class CSensorElectroBot : public CPluginModule
{
    Q_OBJECT
#define     VERSION_SensorElectroBot   "1.0"
#define     AUTEUR_SensorElectroBot    "Nico"
#define     INFO_SensorElectroBot      "Etat des capteurs d'Electrobot"

public:
    CSensorElectroBot(const char *plugin_name);
    ~CSensorElectroBot();

    Cihm_SensorElectroBot *getIHM(void) { return(&m_ihm); }

    virtual void init(CLaBotBox *application);
    virtual void close(void);
    virtual bool hasGUI(void)           { return(true); }
    virtual QIcon getIcon(void)         { return(QIcon(":/icons/webcam.png")); }  // Précise l'icône qui représente le module
    virtual QString getMenuName(void)   { return("PluginModule"); }                 // Précise le nom du menu de la fenêtre principale dans lequel le module apparaît

private:
    Cihm_SensorElectroBot m_ihm;

private slots :
    void onRightClicGUI(QPoint pos);

private :
    void updateAliasLabels(void);
    int color_now;
    int color_prec;
    int color_conf;
    int color_count;

private slots :
    void Etor1_changed(QVariant val);
    void Etor2_changed(QVariant val);
    void Etor3_changed(QVariant val);
    void Etor4_changed(QVariant val);
    void Etor5_changed(QVariant val);
    void Etor6_changed(QVariant val);
    void Etor_PGED1_dsPIC2_changed(QVariant val);
    void Etor_PGED1_dsPIC1_changed(QVariant val);
    void Etor_PGEC1_dsPIC2_changed(QVariant val);
    void Etor_PGEC1_dsPIC1_changed(QVariant val);
    void Etor_Codeur4_B_changed(QVariant val);
    void Etor_Codeur4_A_changed(QVariant val);
    void Etor_Codeur3_B_changed(QVariant val);
    void Etor_Codeur3_A_changed(QVariant val);
    void Etor_CAN_TX_changed(QVariant val);
    void Etor_CAN_RX_changed(QVariant val);

    void Eana1_changed(QVariant val);
    void Eana2_changed(QVariant val);
    void Eana3_changed(QVariant val);
    void Eana4_changed(QVariant val);
    void Eana5_changed(QVariant val);
    void Eana6_changed(QVariant val);
    void Eana7_changed(QVariant val);
    void Eana8_changed(QVariant val);
    void Eana9_changed(QVariant val);
    void Eana10_changed(QVariant val);
    void Eana11_changed(QVariant val);
    void Eana12_changed(QVariant val);
    void Eana13_changed(QVariant val);
    void Vbat_changed(QVariant val);

    void Codeur_1_changed(QVariant val);
    void Codeur_2_changed(QVariant val);
    void Codeur_3_changed(QVariant val);
    void Codeur_4_changed(QVariant val);

    void R_changed(QVariant val);
    void G_changed(QVariant val);
    void B_changed(QVariant val);
};

#endif // _CPLUGIN_MODULE_SensorElectroBot_H_

/*! @} */


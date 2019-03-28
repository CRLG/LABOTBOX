/*! \file CEcran.h
 * A brief file description.
 * A more elaborated file description.
 */
#ifndef _CBASIC_MODULE_Ecran_H_
#define _CBASIC_MODULE_Ecran_H_

#include <QMainWindow>

#include "CPluginModule.h"
#include "ui_ihm_Ecran.h"
#include "CData.h"

 class Cihm_Ecran : public QMainWindow
{
    Q_OBJECT
public:
    Cihm_Ecran(QWidget *parent = 0)  : QMainWindow(parent) { ui.setupUi(this); }
    ~Cihm_Ecran() { }

    Ui::ihm_Ecran ui;

    CApplication *m_application;
 };



 /*! \addtogroup Ecran
   * 
   *  @{
   */

	   
/*! @brief class CEcran
 */	   
class CEcran : public CPluginModule
{
    Q_OBJECT
#define     VERSION_Ecran   "1.0"
#define     AUTEUR_Ecran    "Laguiche"
#define     INFO_Ecran      "IHM du robot pour écran sur raspberry pi"

public:
    CEcran(const char *plugin_name);
    ~CEcran();

    Cihm_Ecran *getIHM(void) { return(&m_ihm); }

    //int m_isMaximized;

    virtual void init(CApplication *application);
    virtual void close(void);
    virtual bool hasGUI(void)           { return(true); }
    virtual QIcon getIcon(void)         { return(QIcon(":/icons/edit_add.png")); }  // Pr�cise l'ic�ne qui repr�sente le module
    virtual QString getMenuName(void)   { return("PluginModule"); }                  // Pr�cise le nom du menu de la fen�tre principale dans lequel le module appara�t

private:
    Cihm_Ecran m_ihm;
    QColor initColor;

private slots :
    void onRightClicGUI(QPoint pos);
    void onClicColorButton(void);
    void CouleurEquipe_changed(QVariant val);
    void ModeFonctionnement_changed(QVariant val);
    void Vbat_changed(QVariant val);
    void TpsMatch_changed(QVariant val);
    void Telemetre1_changed(QVariant val);
    void Telemetre2_changed(QVariant val);
    void Telemetre3_changed(QVariant val);
    void Telemetre4_changed(QVariant val);
    void onRPI_Shutdown();
    void onRPI_Reboot();
};

#endif // _CBASIC_MODULE_Ecran_H_

/*! @} */


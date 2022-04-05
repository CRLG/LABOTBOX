/*! \file CKMAR.h
 * A brief file description.
 * A more elaborated file description.
 */
#ifndef _CPLUGIN_MODULE_KMAR_H_
#define _CPLUGIN_MODULE_KMAR_H_

#include <QMainWindow>

#include "CPluginModule.h"
#include "ui_ihm_KMAR.h"

 class Cihm_KMAR : public QMainWindow
{
    Q_OBJECT
public:
    Cihm_KMAR(QWidget *parent = 0)  : QMainWindow(parent) { ui.setupUi(this); }
    ~Cihm_KMAR() { }

    Ui::ihm_KMAR ui;

    CApplication *m_application;
 };



 /*! \addtogroup KMAR
   * 
   *  @{
   */

	   
/*! @brief class CKMAR
 */	   
class CKMAR : public CPluginModule
{
    Q_OBJECT
#define     VERSION_KMAR   "1.0"
#define     AUTEUR_KMAR    "Nico"
#define     INFO_KMAR      "Bras robotisé KMAR"

public:
    CKMAR(const char *plugin_name);
    ~CKMAR();

    Cihm_KMAR *getIHM(void) { return(&m_ihm); }

    virtual void init(CApplication *application);
    virtual void close(void);
    virtual bool hasGUI(void)           { return(true); }
    virtual QIcon getIcon(void)         { return(QIcon(":/icons/edit_add.png")); }  // Précise l'icône qui représente le module
    virtual QString getMenuName(void)   { return("PluginModule"); }                 // Précise le nom du menu de la fenêtre principale dans lequel le module apparaît

private:
    Cihm_KMAR m_ihm;

private slots :
    void onRightClicGUI(QPoint pos);

    void send_mouvement_clicked(void);
    void send_vitesse_clicked(void);
    void send_disarm_clicked(void);
};

#endif // _CPLUGIN_MODULE_KMAR_H_

/*! @} */


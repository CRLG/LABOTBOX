/*! \file CBalise.h
 * A brief file description.
 * A more elaborated file description.
 */
#ifndef _CPLUGIN_MODULE_Balise_H_
#define _CPLUGIN_MODULE_Balise_H_

#include <QMainWindow>
#include <QList>
#include <QTimer>
#include <QElapsedTimer>

#include "CPluginModule.h"
#include "ui_ihm_Balise.h"
#include "CDataLogger.h"

 class Cihm_Balise : public QMainWindow
{
    Q_OBJECT
public:
    Cihm_Balise(QWidget *parent = 0)  : QMainWindow(parent) { ui.setupUi(this); }
    ~Cihm_Balise() { }

    Ui::ihm_Balise ui;

    CApplication *m_application;
 };

class CData;

 class DatabaseXbeeNetwork2019;

 /*! \addtogroup Balise
   * 
   *  @{
   */

	   
/*! @brief class CBalise
 */	   
class CBalise : public CPluginModule
{
    Q_OBJECT
#define     VERSION_Balise   "1.0"
#define     AUTEUR_Balise    "Nico"
#define     INFO_Balise      "Gestion de la balise de détection de robots sur le terrain"

public:
    CBalise(const char *plugin_name);
    ~CBalise();

    Cihm_Balise *getIHM(void) { return(&m_ihm); }

    virtual void init(CApplication *application);
    virtual void close(void);
    virtual bool hasGUI(void)           { return(true); }
    virtual QIcon getIcon(void)         { return(QIcon(":/icons/edit_add.png")); }  // Précise l'icône qui représente le module
    virtual QString getMenuName(void)   { return("PluginModule"); }                 // Précise le nom du menu de la fenêtre principale dans lequel le module apparaît

private:
    Cihm_Balise m_ihm;

    DatabaseXbeeNetwork2019 *m_xbee_ntw_database;

    CDataLogger m_data_logger;
    static const int LOGGER_PERIOD = 100; // [msec]
    static const QString BASE_LOGGER_FILENAME;
    void initDataLogger();
    QString getLogFilename();

private slots :
    void onRightClicGUI(QPoint pos);

    void Robot1_DetectedPosition_changed();
    void Robot2_DetectedPosition_changed();
    void Robot3_DetectedPosition_changed();
    void Robot4_DetectedPosition_changed();

    void Grosbot_Position_changed();
    void Minibot_Position_changed();

    void video_state_changed(QVariant val);
    void grosbot_present_changed(QVariant val);

    void onRPI_Shutdown();
    void onRPI_Reboot();
};

#endif // _CPLUGIN_MODULE_Balise_H_

/*! @} */


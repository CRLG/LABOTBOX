/*! \file CTelemetryServer.h
 * A brief file description.
 * A more elaborated file description.
 */
#ifndef _CPLUGIN_MODULE_TelemetryServer_H_
#define _CPLUGIN_MODULE_TelemetryServer_H_

#include <QMainWindow>

#include "CPluginModule.h"
#include "ui_ihm_TelemetryServer.h"
#include "telemetry_server.h"

 class Cihm_TelemetryServer : public QMainWindow
{
    Q_OBJECT
public:
    Cihm_TelemetryServer(QWidget *parent = 0)  : QMainWindow(parent) { ui.setupUi(this); }
    ~Cihm_TelemetryServer() { }

    Ui::ihm_TelemetryServer ui;

    CApplication *m_application;
 };



 /*! \addtogroup TelemetryServer
   * 
   *  @{
   */

	   
/*! @brief class CTelemetryServer
 */	   
class CTelemetryServer : public CPluginModule
{
    Q_OBJECT
#define     VERSION_TelemetryServer   "1.0"
#define     AUTEUR_TelemetryServer    "Nico"
#define     INFO_TelemetryServer      "Module de télémétrie côté serveur"

public:
    CTelemetryServer(const char *plugin_name);
    ~CTelemetryServer();

    Cihm_TelemetryServer *getIHM(void) { return(&m_ihm); }

    virtual void init(CApplication *application);
    virtual void close(void);
    virtual bool hasGUI(void)           { return(true); }
    virtual QIcon getIcon(void)         { return(QIcon(":/icons/edit_add.png")); }  // Précise l'icône qui représente le module
    virtual QString getMenuName(void)   { return("PluginModule"); }                 // Précise le nom du menu de la fenêtre principale dans lequel le module apparaît

private:
    Cihm_TelemetryServer m_ihm;
    TelemetryServer m_exchanger;

private slots :
    void onRightClicGUI(QPoint pos);

    void onReceiveDouble(double val);
    void onReceiveUInt8(quint8 val);
    void onReceiveInt32(int val);
    void onReceiveVariant(const QVariant &val);
    void onReceiveDocDesigner(QString doc);
    void onReceiveRawData(const QByteArray &raw_data);

    void onPbTest1();
    void onPbTest2();
};

#endif // _CPLUGIN_MODULE_TelemetryServer_H_

/*! @} */


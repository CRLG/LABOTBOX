/*! \file CTelemetryClient.h
 * A brief file description.
 * A more elaborated file description.
 */
#ifndef _CPLUGIN_MODULE_TelemetryClient_H_
#define _CPLUGIN_MODULE_TelemetryClient_H_

#include <QMainWindow>
#include <QTimer>

#include "CPluginModule.h"
#include "ui_ihm_TelemetryClient.h"
#include "telemetry_client.h"

 class Cihm_TelemetryClient : public QMainWindow
{
    Q_OBJECT
public:
    Cihm_TelemetryClient(QWidget *parent = 0)  : QMainWindow(parent) { ui.setupUi(this); }
    ~Cihm_TelemetryClient() { }

    Ui::ihm_TelemetryClient ui;

    CApplication *m_application;
 };



 /*! \addtogroup TelemetryClient
   * 
   *  @{
   */

	   
/*! @brief class CTelemetryClient
 */	   
class CTelemetryClient : public CPluginModule
{
    Q_OBJECT
#define     VERSION_TelemetryClient   "1.0"
#define     AUTEUR_TelemetryClient    "Nico"
#define     INFO_TelemetryClient      "Module de télémétrie côté client"

public:
    CTelemetryClient(const char *plugin_name);
    ~CTelemetryClient();

    Cihm_TelemetryClient *getIHM(void) { return(&m_ihm); }

    virtual void init(CApplication *application);
    virtual void close(void);
    virtual bool hasGUI(void)           { return(true); }
    virtual QIcon getIcon(void)         { return(QIcon(":/icons/edit_add.png")); }  // Précise l'icône qui représente le module
    virtual QString getMenuName(void)   { return("PluginModule"); }                 // Précise le nom du menu de la fenêtre principale dans lequel le module apparaît

private:
    Cihm_TelemetryClient m_ihm;
    QTimer m_timer;

    TelemetryClient     m_telemetry_client;

    void init_socket(QString ip_add, int port);

private slots :
    void onRightClicGUI(QPoint pos);

    void onReceiveDouble(double val);
    void onReceiveInt32(int);
    void onReceiveVariant(const QVariant &val);
    void onReceiveDocDesigner(QString doc);
    void onReceiveRawData(const QByteArray &raw_data);

    void onPbTest1();
    void onPbTest2();

    void connected();
    void disconnected();
};

#endif // _CPLUGIN_MODULE_TelemetryClient_H_

/*! @} */


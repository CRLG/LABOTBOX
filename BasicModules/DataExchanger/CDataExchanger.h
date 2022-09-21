/*! \file CDataExchanger.h
 * A brief file description.
 * A more elaborated file description.
 */
#ifndef _CBASIC_MODULE_DataExchanger_H_
#define _CBASIC_MODULE_DataExchanger_H_

#include <QMainWindow>

#include "CBasicModule.h"
#include "ui_ihm_DataExchanger.h"
#include "data_server.h"
#include "data_client.h"

class Cihm_DataExchanger : public QMainWindow
{
    Q_OBJECT
public:
    Cihm_DataExchanger(QWidget *parent = 0)  : QMainWindow(parent) { ui.setupUi(this); }
    ~Cihm_DataExchanger() { }

    Ui::ihm_DataExchanger ui;

    CApplication *m_application;
};



/*! \addtogroup DataExchanger
   *
   *  @{
   */


class CData;
/*! @brief class CDataExchanger
 */
class CDataExchanger : public CBasicModule
{
    Q_OBJECT
#define     VERSION_DataExchanger   "1.0"
#define     AUTEUR_DataExchanger    "Nico"
#define     INFO_DataExchanger      "Module d'echange de data"

public:
    CDataExchanger(const char *plugin_name);
    ~CDataExchanger();

    Cihm_DataExchanger *getIHM(void) { return(&m_ihm); }

    virtual void init(CApplication *application);
    virtual void close(void);
    virtual bool hasGUI(void)           { return(true); }
    virtual QIcon getIcon(void)         { return(QIcon(":/icons/edit_add.png")); }  // Précise l'icône qui représente le module
    virtual QString getMenuName(void)   { return("BasicModule"); }                 // Précise le nom du menu de la fenêtre principale dans lequel le module apparaît

private:
    Cihm_DataExchanger m_ihm;
    DataServer *m_server;
    DataClient *m_client;
    CExchangerData *m_exchanger;

    void init_gateway_as_client(QString ip_add, int port);
    void init_gateway_as_server(int port);

    void connectDisconnectDatas(bool choix);
    void connectDisconnectData(CData *data, bool choix);
    void addVariablesObserver(QStringList liste_variables);
    QStringList getListeVariablesObservees();
    void enableDisabelGuiOnConnectionDeconnection(bool state);

public slots :
    void variableChangedUpdated(QVariant val, quint64 update_time);
    void refreshListeVariables(void);
    void loadListeVariablesObservees(void);
    void saveListeVariablesObservees(void);
    void decocheToutesVariables();
    void cocheToutesVariables();
    void activeGateway(bool state);
    void sendAllDataManager();
    void sendSelectedData();

private slots :
    void onRightClicGUI(QPoint pos);
    void onDataFilterChanged(QString filter_name);

    void gateway_is_connected();
    void gateway_is_disconnected();

    void onReceiveDouble(double val);
    void onReceiveUInt8(quint8 val);
    void onReceiveInt32(int val);
    void onReceiveVariant(const QVariant &val);
    void onReceiveRawData(const QByteArray &raw_data);
    void onReceiveData(QString name, QVariant val);
    void onReceiveDatas(QHash<QString, QVariant> datas);
    void onReceiveAddDataRequest(QString name);
    void onReceiveRemoveDataRequest(QString name);
    void onReceiveStartStopTransmissionRequest(bool start_stop);

    void onStartRequest();
    void onStopRequest();
};

#endif // _CBASIC_MODULE_DataExchanger_H_

/*! @} */


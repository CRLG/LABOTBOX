/*! \file CBlockBotLab.h
 * A brief file description.
 * A more elaborated file description.
 */
#ifndef _CPLUGIN_MODULE_BlockBotLab_H_
#define _CPLUGIN_MODULE_BlockBotLab_H_

#include <QMainWindow>
#include <QWebEngineView>
#include <QProcess>
#include <QTimer>
#include <QUrl>
#include <QWebChannel>
#include <QComboBox>
#include <QWebEngineDownloadItem>

#include "CPluginModule.h"
#include "ui_ihm_BlockBotLab.h"


 class Cihm_BlockBotLab : public QMainWindow
{
    Q_OBJECT
public:
    Cihm_BlockBotLab(QWidget *parent = 0)  : QMainWindow(parent) { ui.setupUi(this); }
    ~Cihm_BlockBotLab() { }

    Ui::ihm_BlockBotLab ui;

    CApplication *m_application;
 };



 /*! \addtogroup BlockBotLab
   * 
   *  @{
   */

	   
/*! @brief class CBlockBotLab
 */	   
class CBlockBotLab : public CPluginModule
{
    Q_OBJECT
#define     VERSION_BlockBotLab   "1.0"
#define     AUTEUR_BlockBotLab    "Laguiche / Nico"
#define     INFO_BlockBotLab      "Création du module de gestion de Blocky"

public:
    CBlockBotLab(const char *plugin_name);
    ~CBlockBotLab();

    Cihm_BlockBotLab *getIHM(void) { return(&m_ihm); }

    virtual void init(CApplication *application);
    virtual void close(void);
    virtual bool hasGUI(void)           { return(true); }
    virtual QIcon getIcon(void)         { return(QIcon(":/icons/edit_add.png")); }  // Précise l'icône qui représente le module
    virtual QString getMenuName(void)   { return("PluginModule"); }                 // Précise le nom du menu de la fenêtre principale dans lequel le module apparaît

private:
    Cihm_BlockBotLab m_ihm;
    QString m_generated_pathfilename;
    QString m_launch_and_program_command;
    QProcess m_build_target_process;
    QTimer m_timer_close_build_logs_delayed;

    QString m_blockbotPath;
    QString m_blockbotPort;
    bool m_blockbotDevMode;
    bool m_blockbotStarted;
    bool m_blockbotBuilt;
    //Ui::RoboticsMainWindow *ui;
    QWebEngineView* m_blockbotWebView;
    QProcess* m_blockbotProcess;
    void startBlockBot();
    void loadBlockbotInWebView();
    QString readFile(QString pathfilename);
    //void loadBlocklyInWebView(const QString& port);

    QWebChannel* webChannel;

    QComboBox *modeChoice;

signals:
    //signal pour envoyer une commande et un paramètre à BlockBot
    void executeCommand(const QString &Cmd, const QString &params);

private slots :
    void onRightClicGUI(QPoint pos);

public slots :
    //bool processData(const QString& code);
    bool processData(QString code, QString nomStrategie, QString listeEtatsJSON);
    bool buildTargetAndUpload();
    void buildStarted();
    void buildFinished(int exitcode);
    void buildError();
    void buildOutput();
    void setBuildLogsVisibility(bool visible);
    void closeBuildLogs();
    void send2BlockBot();
    void logJS(const QString& message);
    void onDownloadRequested(QWebEngineDownloadItem *download);

};

#endif // _CPLUGIN_MODULE_BlockBotLab_H_

/*! @} */


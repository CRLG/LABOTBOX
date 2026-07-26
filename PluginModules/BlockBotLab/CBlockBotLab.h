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
#include <QCheckBox>
#include <QWebEngineDownloadItem>

#include "CPluginModule.h"
#include "ui_ihm_BlockBotLab.h"
#include "CHILEngine.h"


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
    QString m_config_specifique_coupe_path;
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
    QCheckBox *showCode;

    //! Moteur d'exécution HIL (Hardware In the Loop)
    CHILEngine *m_hilEngine;

signals:
    //signal pour envoyer une commande et un paramètre à BlockBot
    void executeCommand(const QString &Cmd, const QString &params);

private slots :
    void onRightClicGUI(QPoint pos);
    //! Appele a chaque fin de chargement de la page BlockBot (mode dev comme mode indus).
    //! Reapplique dans BlockBot l'etat de l'IHM LaBotBox qui, sinon, ne lui serait jamais
    //! transmis : BlockBot redemarre sur ses propres valeurs par defaut a chaque chargement.
    void onBlockBotLoaded(bool ok);
    //! Pousse le mode courant du combo (debutant/expert) vers BlockBot.
    void pushCurrentModeToBlockBot();

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
    // Déclenché par CSimuBot::setSequence() (double-clic simulation, checkbox cochée).
    // Lit x_pos / y_pos / teta_pos et envoie la commande de création de blocs à BlockBot.
    void Slot_SetPosFromSimu(double angle, double distance);

    // ── HIL (Hardware In the Loop) ──────────────────────────────────────
    //! Reçoit la description HIL exportée depuis BlockBot (JSON).
    //! Appelé par JS en réponse à get_hil_start_state, get_hil_state ou export_hil_single_action.
    //! hilType : "start_state", "state" ou "single_action"
    //! hilJson : description JSON (vide si rien de valide)
    void processHILExport(const QString &hilType, const QString &hilJson);

    //! Connecté à actionPlayHIL — demande l'export puis lance CHILEngine
    void Slot_PlayHIL();

    //! Connecté à actionStopHIL — arrêt de sécurité + nettoyage
    void Slot_StopHIL();

    //! Connecté à actionPlayOnlyOneHIL — demande l'export action unique puis l'exécute
    void Slot_PlaySingleActionHIL();

    //! Connecté à actionMergeProject — ouvre un fichier JSON et fusionne les blocs dans le workspace courant.
    //! Contrairement à actionOpen (load_project), le workspace existant n'est pas effacé.
    void Slot_MergeProject();

};

#endif // _CPLUGIN_MODULE_BlockBotLab_H_

/*! @} */


/*! \file CDataPlayer.h
 * A brief file description.
 * A more elaborated file description.
 */
#ifndef _CBASIC_MODULE_DataPlayer_H_
#define _CBASIC_MODULE_DataPlayer_H_

#include <QMainWindow>
#include <QMap>

#include "CBasicModule.h"
#include "ui_ihm_DataPlayer.h"
#include "CTracePlayer.h"

class CSignalGenerator;
class CTracePlayer;

 class Cihm_DataPlayer : public QMainWindow
{
    Q_OBJECT
public:
    Cihm_DataPlayer(QWidget *parent = 0)  : QMainWindow(parent) { ui.setupUi(this); }
    ~Cihm_DataPlayer() { }

    Ui::ihm_DataPlayer ui;

    CApplication *m_application;
 };




typedef QMap<QString, CSignalGenerator*>tListeVariables;    // Pour les générateurs de signaux
typedef QMap<QString, CTracePlayer*>tListePlayers;          // Pour les players

 /*! \addtogroup DataPlayer
   * 
   *  @{
   */

	   
/*! @brief class CDataPlayer
 */	   
class CDataPlayer : public CBasicModule
{
    Q_OBJECT
#define     VERSION_DataPlayer   "1.0"
#define     AUTEUR_DataPlayer    "Nico"
#define     INFO_DataPlayer      "Générateur de signaux pour data"

public:
    CDataPlayer(const char *plugin_name);
    ~CDataPlayer();

    Cihm_DataPlayer *getIHM(void) { return(&m_ihm); }

    virtual void init(CApplication *application);
    virtual void close(void);
    virtual bool hasGUI(void)           { return(true); }
    virtual QIcon getIcon(void)         { return(QIcon(":/icons/signal.png")); }  // Précise l'icône qui représente le module
    virtual QString getMenuName(void)   { return("BasicModule"); }                  // Précise le nom du menu de la fenêtre principale dans lequel le module apparaît
    virtual void setVisible(void);

private slots :
    void onRightClicGUI(QPoint pos);

private:
    Cihm_DataPlayer m_ihm;
    tListeVariables m_liste_generateurs;
    QString m_default_signal_path;  // répertoire par défaut des des signaux

    tListePlayers   m_liste_players;


    void refreshListeVariables(void);
    void miseEnCoherenceListeVariables(void);
    CSignalGenerator *selectedVariableToSignalGenerator(void);

    // Gestion des traces
    void newPlayer(QString player_name);
    CTracePlayer *selectedPlayerNameToPlayer(void);
    void refreshIHMGenerateur(void);



public slots :
    // Générateur de signaux
    void signalGenerationFinished(QString var_name);
    void on_PB_refresh_liste_clicked(void);
    void on_PB_choixSignal_clicked(void);
    void on_PB_StartGeneration_clicked(void);
    void on_PB_StopGeneration_clicked(void);
    void on_dureeEchantillon_editingFinished(void);
    void on_nombreCycles_editingFinished(void);
    void on_selectVariable(void);
    void on_checkVariable(QListWidgetItem* item);
    void refreshGeneratorProperties(void);

    // Gestion des traces
    void on_PB_choixTrace_clicked(void);
    void on_PB_StepBackwardTrace_clicked(void);
    void on_PB_StartTrace_clicked(void);
    void on_PB_StopTrace_clicked(void);
    void on_PB_StepForwardTrace_clicked(void);
    void on_playerNameListe_changed(QString player_name);
    void on_PB_AjoutPlayer_clicked(void);
    void on_TracePlayerStateChanged(QString player_name);
    void on_dureeStepTrace_editingFinished(void);
    void on_nombreCyclesTrace_editingFinished(void);

    void on_PB_StartAllTraces_Synchro_clicked(void);
    void on_PB_StopAllTraces_Synchro_clicked(void);
    void on_sliderStepNumTrace_moved(int val);
};

#endif // _CBASIC_MODULE_DataPlayer_H_

/*! @} */


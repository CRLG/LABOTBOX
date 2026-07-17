/*! \file IRobotLogic.h
    \brief Interface stable entre le shell Simulia (CSimulia) et le plugin de logique robot.

    POC hot-reload (cf. CLAUDE.md a la racine du clone GROSBOT_STM32_test_simulia).

    Contrat impose par le spike :
      - le shell ne reference AUCUN symbole du plugin en direct ;
      - il ne tient qu'un IRobotLogic* re-obtenu a chaque chargement (createRobotLogic) ;
      - le plugin est charge en RTLD_LOCAL (isolation) -> unload/reload effectif.

    Regles :
      - les APPELS de methode (qui lieraient un symbole plugin cote shell) passent par
        des methodes virtuelles de cette interface ;
      - les ACCES aux champs des structs partagees (offset, sans symbole) restent directs
        via les accesseurs inputs()/datas()/debug() qui renvoient des pointeurs ;
      - AUCUN type Qt-widget dans la logique : seuls quelques passages de pointeurs de widgets
        sont tolerees pour le lidar (choix pragmatique POC, cf. CLAUDE.md principe P3).
*/
#ifndef _IROBOTLOGIC_H_
#define _IROBOTLOGIC_H_

// Structs metier partagees (memes headers, meme toolchain -> layout stable des deux cotes).
#include "sm_inputsinterface.h"
#include "sm_datasinterface.h"
#include "sm_debuginterface.h"

class CApplication;      // classe generique LABOTBOX (cote shell)
class QTableWidget;      // widgets IHM : le plugin ne fait que les lire/ecrire (Qt charge une seule fois)
class QComboBox;
class QStringList;

// _____________________________________________________________________
//! Interface de la logique robot rechargeable a chaud.
class IRobotLogic
{
public:
    virtual ~IRobotLogic() {}

    // ---- cycle de vie ----
    //! Lie les ~17 sous-systemes a l'application (rejoue les Application.xxx.init(app)).
    virtual void initSubsystems(CApplication* application) = 0;
    //! Reset complet de la simulation (equivalent de l'ancien CSimulia::on_pb_init_all).
    virtual void resetSimulation() = 0;
    virtual void setStrategie(int strategie) = 0;
    virtual void setCouleurEquipe(int couleur) = 0;
    virtual void startMain() = 0;              //!< m_modelia.m_sm_main.start()
    virtual void stopAll() = 0;                //!< m_modelia.stopAllStateMachines()
    virtual void installDefaultDebugger() = 0; //!< m_modelia.setDebugger(new SM_DebugQDebug())

    // ---- tick (boucle chaude, encapsule simu_task_sequencer + publications DataManager) ----
    //! Execute un pas de simulation. m_step est mis a jour cote plugin ; expose par currentStep().
    virtual void step() = 0;
    virtual int  currentStep() const = 0;
    virtual void resetStep() = 0;

    // ---- entrees IHM -> modele ----
    virtual void setTestActionneurs(int val) = 0;
    virtual void setDebugFlags(bool start, bool stop, bool onEntry, bool onExit, bool interruptEvit) = 0;
    virtual void setTelemetresFromGui(float avg, float avd, float arg, float ard, float argCentre, float ardCentre) = 0;
    virtual void setOrigineTelemetre(int origine) = 0;
    virtual void setDetectionFromGui(bool avg, bool avd, bool arg, bool ard) = 0;
    virtual void setOrigineDetection(int origine) = 0;
    virtual void setGlobalCurrent(float current) = 0;
    virtual void setPositionXYTeta(float x, float y, float teta) = 0;
    virtual void addCodeurSteps(int steps_G, int steps_D) = 0;

    // ---- lidar (passages de widgets toleres, POC) ----
    virtual void lidarInitGui(QTableWidget* table, QComboBox* statusCombo) = 0;
    virtual void lidarRefreshGui(QTableWidget* table, QComboBox* statusCombo) = 0;
    virtual void lidarSetObstaclesFromGui(QTableWidget* table) = 0;
    virtual void lidarReset() = 0;               //!< m_lidar.Init()
    virtual void setOrigineLidar(int origine) = 0;
    virtual int  lidarOrigine() const = 0;
    virtual void lidarOrigines(QStringList& out) const = 0; //!< remplit la liste des origines pour le combo

    // ---- acces donnees (pointeurs de struct : lecture/ecriture de champs sans symbole) ----
    virtual SM_InputsInterface* inputs() = 0;
    virtual SM_DatasInterface*  datas()  = 0;
    virtual SM_DebugInterface*  debug()  = 0;
};

// ABI C d'entree du plugin (paire symetrique).
extern "C" IRobotLogic* createRobotLogic();
extern "C" void         destroyRobotLogic(IRobotLogic* p);

#endif

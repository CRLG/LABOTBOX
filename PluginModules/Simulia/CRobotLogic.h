/*! \file CRobotLogic.h
    \brief Implementation cote plugin de l'interface IRobotLogic.

    CRobotLogic encapsule le CGlobaleSimule global (interne au plugin, isole par RTLD_LOCAL)
    et delegue chaque operation aux sous-systemes Application.xxx. Le code metier (Modelia,
    _simu, CppRobLib) reste inchange et continue d'utiliser le global Application.

    C'est ici que descend la logique auparavant dans CSimulia :
      - la sequence d'init des sous-systemes,
      - le reset simulation (ancien on_pb_init_all),
      - le tick multi-cadences (ancien simu_task_sequencer),
      - les publications de sorties dans le DataManager.
*/
#ifndef _CROBOTLOGIC_H_
#define _CROBOTLOGIC_H_

#include "IRobotLogic.h"

class CApplication;

class CRobotLogic : public IRobotLogic
{
public:
    CRobotLogic();
    virtual ~CRobotLogic();

    // ---- cycle de vie ----
    void initSubsystems(CApplication* application) override;
    void resetSimulation() override;
    void setStrategie(int strategie) override;
    void setCouleurEquipe(int couleur) override;
    void startMain() override;
    void stopAll() override;
    void installDefaultDebugger() override;

    // ---- tick ----
    void step() override;
    int  currentStep() const override { return m_step; }
    void resetStep() override { m_step = 0; }

    // ---- entrees IHM -> modele ----
    void setTestActionneurs(int val) override;
    void setDebugFlags(bool start, bool stop, bool onEntry, bool onExit, bool interruptEvit) override;
    void setTelemetresFromGui(float avg, float avd, float arg, float ard, float argCentre, float ardCentre) override;
    void setOrigineTelemetre(int origine) override;
    void setDetectionFromGui(bool avg, bool avd, bool arg, bool ard) override;
    void setOrigineDetection(int origine) override;
    void setGlobalCurrent(float current) override;
    void setPositionXYTeta(float x, float y, float teta) override;
    void addCodeurSteps(int steps_G, int steps_D) override;

    // ---- lidar ----
    void lidarInitGui(QTableWidget* table, QComboBox* statusCombo) override;
    void lidarRefreshGui(QTableWidget* table, QComboBox* statusCombo) override;
    void lidarSetObstaclesFromGui(QTableWidget* table) override;
    void lidarReset() override;
    void setOrigineLidar(int origine) override;
    int  lidarOrigine() const override;
    void lidarOrigines(QStringList& out) const override;

    // ---- acces donnees ----
    SM_InputsInterface* inputs() override;
    SM_DatasInterface*  datas()  override;
    SM_DebugInterface*  debug()  override;

private:
    //! Ordonnanceur multi-cadences (ancien CSimulia::simu_task_sequencer).
    void taskSequencer();
    //! Publie les sorties du modele dans le DataManager (ancien bas de step_sequencer).
    void publishOutputs(int oldStep);

    CApplication* m_application;
    int m_step;

    // Compteurs de l'ordonnanceur : desormais membres (donc reinitialises au reload,
    // contrairement aux static de l'ancienne implementation cote shell).
    unsigned int m_cpt10msec;
    unsigned int m_cpt20msec;
    unsigned int m_cpt50msec;
    unsigned int m_cpt100msec;
    unsigned int m_cpt200msec;
    unsigned int m_cpt500msec;
    unsigned int m_cpt1sec;
};

#endif

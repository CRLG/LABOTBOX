// Banc logique robot : charge la logique robot compilee pour Simulia (librobotlogic_*.so) HORS de
// Simulia et la pilote par son interface IRobotLogic, pour verifier des scenarios de facon scriptee
// et reproductible (sans ecran, sans SimuBot).
//
// Ce qui est reel : TOUTE la logique robot (Modelia, CppRobLib, classes _simu, modele de roues
// interne du plugin). Ce qui est remplace : le shell Simulia, dont le plugin n'utilise que le
// DataManager (cf. banc_data_manager.h) et le pointeur m_data_center de CApplication.
//
// Ne l'utiliser que sur un plugin compile depuis LE MEME etat des sources que le banc : le banc
// lit directement SM_InputsInterface et SM_DatasInterface, dont la disposition doit correspondre.
#ifndef _BANC_H_
#define _BANC_H_

#include <QString>
#include "banc_data_manager.h"
#include "IRobotLogic.h"

class CApplication;

class Banc
{
public:
    explicit Banc(const QString &chemin_plugin);
    ~Banc();
    bool estCharge() const { return m_logique != nullptr; }

    // ---- pilotage
    void reinitialiser();                              // reset complet + lidar alimente par le DataManager
    void demarrerMatch(int strategie, int couleur);    // strategie, couleur, SM_Main, retrait de la tirette
    void pas(int n = 1);                               // n pas de 10 ms (IA::step tourne un pas sur deux)
    void passagesModele(int n) { pas(2 * n); }         // n passages de IA::step (20 ms chacun)
    void obstacle(int index, int distance_mm, int angle_deg);   // index 1..10, comme Lidar.ObstacleN
    void aucunObstacle();
    void statutLidar(int statut);                      // LidarUtils::LIDAR_OK / _DISCONNECTED / _ERROR

    // ---- observation
    SM_InputsInterface *entrees()  { return m_logique->inputs(); }
    SM_DatasInterface  *donnees()  { return m_logique->datas(); }
    CDataManager       &dataManager() { return m_dm; }

    // ---- verification
    void verifier(bool condition, const QString &libelle);
    void titre(const QString &texte);
    int  echecs() const { return m_echecs; }
    int  verifications() const { return m_verifications; }

private:
    void           *m_handle;
    IRobotLogic    *m_logique;
    void          (*m_detruire)(IRobotLogic *);
    CDataManager    m_dm;
    CApplication   *m_app;
    int             m_echecs;
    int             m_verifications;
};

#endif // _BANC_H_

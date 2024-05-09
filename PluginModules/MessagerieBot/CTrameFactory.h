// FICHIER GENERE PAR L'OUTIL MESS2C_robot V1.0
// Date de génération : Tue May 05 22:41:21 2015
// PLATEFORME CIBLE : LABOTBOX
/*! \file CTrameFactory.h
 * A brief file description header.
 * A more elaborated file description.
 */

#ifndef _CTRAME_FACTORY_H_
#define _CTRAME_FACTORY_H_

#include <QObject>
#include <QVector>
#include <QVariant>
#include "CTrameBot.h"
#include "Lidar_utils.h"

/*! \addtogroup TrameFactory
   *  Additional documentation for group TrameFactory
   *  @{
   */

//! Enumere des codes commandes de l'ecran vers le MBED
typedef enum {
    // Generique pour toutes les annees
    LBB_CMDE_INVALIDE = 0,
    LBB_CMDE_CHOIX_EQUIPE,
    LBB_CMDE_CHOIX_NUMERO_STRATEGIE,
    LBB_CMDE_RAZ_CODEURS_ROUES,
    LBB_CMDE_RAZ_CODEUR_3,
    LBB_CMDE_RAZ_CODEUR_4,
    LBB_CMDE_RESET_MODELE_SIMULINK_MATCH,
    LBB_CMDE_TEST_ACTIONNEURS,
    LBB_CMDE_INIT_ACTIONNEURS
}tCodeCommandeLabotBox;

typedef QVector<CTrameBot *>tListeTrames;

class CMessagerieBot;
class CDataManager;

/*! @brief class CTrameFactory in @link TrameFactory.
 */
class CTrameFactory : public QObject
{
    Q_OBJECT

public:
    CTrameFactory(CMessagerieBot *messagerie_bot, CDataManager *data_manager);
    ~CTrameFactory();

    void create(void);

    tListeTrames getListeTramesRx() const;
    tListeTrames getListeTramesTx() const;
    tListeTrames getListeTrames() const;
    unsigned int name2ID(QString name);
    QString ID2Name(unsigned int id);
    CTrameBot *getTrameFromName(QString name);
    CTrameBot *getTrameFromID(unsigned int id);

private :
    CMessagerieBot *m_messagerie_bot;
    CDataManager *m_data_manager;
    //! Liste des trames
    tListeTrames m_liste_trames;
    //! Liste des trames en émission
    tListeTrames m_liste_trames_tx;
    //! Liste des trames en réception
    tListeTrames m_liste_trames_rx;

public slots :
    void Decode(tStructTrameBrute trame);

};


// ========================================================
//             TRAME ELECTROBOT_CDE_SERVOS_SD20
// ========================================================
#define ID_ELECTROBOT_CDE_SERVOS_SD20 0x53
#define DLC_ELECTROBOT_CDE_SERVOS_SD20 5
#define BRUTE2PHYS_valeur_commande_sd20(val) ( ((float)val * (1.000000)) + (0.000000) )
#define PHYS2BRUTE_valeur_commande_sd20(val) (unsigned short)( (val - (0.000000)) / (1.000000) )
#define BRUTE2PHYS_commande_sd20(val) ( ((float)val * (1.000000)) + (0.000000) )
#define PHYS2BRUTE_commande_sd20(val) (unsigned short)( (val - (0.000000)) / (1.000000) )
#define BRUTE2PHYS_num_servo_sd20(val) ( ((float)val * (1.000000)) + (0.000000) )
#define PHYS2BRUTE_num_servo_sd20(val) (unsigned char)( (val - (0.000000)) / (1.000000) )
class CTrame_ELECTROBOT_CDE_SERVOS_SD20 : public CTrameBot
{
   Q_OBJECT
public :
    CTrame_ELECTROBOT_CDE_SERVOS_SD20(CMessagerieBot *messagerie_bot, CDataManager *data_manager);
    ~CTrame_ELECTROBOT_CDE_SERVOS_SD20() { }
    /*virtual*/ void Encode(void);

 private :
    unsigned short valeur_commande_sd20;
    unsigned short commande_sd20;
    unsigned char num_servo_sd20;
    bool m_synchro_tx;

private slots :
    void valeur_commande_sd20_changed(QVariant val);
    void commande_sd20_changed(QVariant val);
    void num_servo_sd20_changed(QVariant val);
    void Synchro_changed(QVariant val);
};


// ========================================================
//             TRAME ELECTROBOT_CDE_SERVOS_AX
// ========================================================
#define ID_ELECTROBOT_CDE_SERVOS_AX 0x52
#define DLC_ELECTROBOT_CDE_SERVOS_AX 5
#define BRUTE2PHYS_valeur_commande_ax(val) ( ((float)val * (1.000000)) + (0.000000) )
#define PHYS2BRUTE_valeur_commande_ax(val) (unsigned short)( (val - (0.000000)) / (1.000000) )
#define BRUTE2PHYS_commande_ax(val) ( ((float)val * (1.000000)) + (0.000000) )
#define PHYS2BRUTE_commande_ax(val) (unsigned short)( (val - (0.000000)) / (1.000000) )
#define BRUTE2PHYS_num_servo_ax(val) ( ((float)val * (1.000000)) + (0.000000) )
#define PHYS2BRUTE_num_servo_ax(val) (unsigned char)( (val - (0.000000)) / (1.000000) )
class CTrame_ELECTROBOT_CDE_SERVOS_AX : public CTrameBot
{
   Q_OBJECT
public :
    CTrame_ELECTROBOT_CDE_SERVOS_AX(CMessagerieBot *messagerie_bot, CDataManager *data_manager);
    ~CTrame_ELECTROBOT_CDE_SERVOS_AX() { }
    /*virtual*/ void Encode(void);

 private :
    unsigned short valeur_commande_ax;
    unsigned short commande_ax;
    unsigned char num_servo_ax;
    bool m_synchro_tx;

private slots :
    void valeur_commande_ax_changed(QVariant val);
    void commande_ax_changed(QVariant val);
    void num_servo_ax_changed(QVariant val);
    void Synchro_changed(QVariant val);
};


// ========================================================
//             TRAME ELECTROBOT_CDE_MOTEURS
// ========================================================
#define ID_ELECTROBOT_CDE_MOTEURS 0x50
#define DLC_ELECTROBOT_CDE_MOTEURS 6
#define BRUTE2PHYS_cde_moteur_6(val) ( ((float)val * (1.000000)) + (0.000000) )
#define PHYS2BRUTE_cde_moteur_6(val) (char)( (val - (0.000000)) / (1.000000) )
#define BRUTE2PHYS_cde_moteur_5(val) ( ((float)val * (1.000000)) + (0.000000) )
#define PHYS2BRUTE_cde_moteur_5(val) (char)( (val - (0.000000)) / (1.000000) )
#define BRUTE2PHYS_cde_moteur_4(val) ( ((float)val * (1.000000)) + (0.000000) )
#define PHYS2BRUTE_cde_moteur_4(val) (char)( (val - (0.000000)) / (1.000000) )
#define BRUTE2PHYS_cde_moteur_3(val) ( ((float)val * (1.000000)) + (0.000000) )
#define PHYS2BRUTE_cde_moteur_3(val) (char)( (val - (0.000000)) / (1.000000) )
#define BRUTE2PHYS_cde_moteur_2(val) ( ((float)val * (1.000000)) + (0.000000) )
#define PHYS2BRUTE_cde_moteur_2(val) (char)( (val - (0.000000)) / (1.000000) )
#define BRUTE2PHYS_cde_moteur_1(val) ( ((float)val * (1.000000)) + (0.000000) )
#define PHYS2BRUTE_cde_moteur_1(val) (char)( (val - (0.000000)) / (1.000000) )
class CTrame_ELECTROBOT_CDE_MOTEURS : public CTrameBot
{
   Q_OBJECT
public :
    CTrame_ELECTROBOT_CDE_MOTEURS(CMessagerieBot *messagerie_bot, CDataManager *data_manager);
    ~CTrame_ELECTROBOT_CDE_MOTEURS() { }
    /*virtual*/ void Encode(void);

 private :
    char cde_moteur_6;
    char cde_moteur_5;
    char cde_moteur_4;
    char cde_moteur_3;
    char cde_moteur_2;
    char cde_moteur_1;
    bool m_synchro_tx;

private slots :
    void cde_moteur_6_changed(QVariant val);
    void cde_moteur_5_changed(QVariant val);
    void cde_moteur_4_changed(QVariant val);
    void cde_moteur_3_changed(QVariant val);
    void cde_moteur_2_changed(QVariant val);
    void cde_moteur_1_changed(QVariant val);
    void Synchro_changed(QVariant val);
};

// ========================================================
//             TRAME ELECTROBOT_CDE_POWER_SWITCH
// ========================================================
#define ID_ELECTROBOT_CDE_POWER_SWITCH 0x54
#define DLC_ELECTROBOT_CDE_POWER_SWITCH 1
class CTrame_ELECTROBOT_CDE_POWER_SWITCH : public CTrameBot
{
   Q_OBJECT
public :
    CTrame_ELECTROBOT_CDE_POWER_SWITCH(CMessagerieBot *messagerie_bot, CDataManager *data_manager);
    ~CTrame_ELECTROBOT_CDE_POWER_SWITCH() { }
    /*virtual*/ void Encode(void);

 private :
    bool PowerSwitch_xt1;
    bool PowerSwitch_xt2;
    bool PowerSwitch_xt3;
    bool PowerSwitch_xt4;
    bool PowerSwitch_xt5;
    bool PowerSwitch_xt6;
    bool PowerSwitch_xt7;
    bool PowerSwitch_xt8;
    bool m_synchro_tx;

private slots :
    void PowerSwitch_xt1_changed(QVariant val);
    void PowerSwitch_xt2_changed(QVariant val);
    void PowerSwitch_xt3_changed(QVariant val);
    void PowerSwitch_xt4_changed(QVariant val);
    void PowerSwitch_xt5_changed(QVariant val);
    void PowerSwitch_xt6_changed(QVariant val);
    void PowerSwitch_xt7_changed(QVariant val);
    void PowerSwitch_xt8_changed(QVariant val);

    void Synchro_changed(QVariant val);
};

// ========================================================
//             TRAME COMMANDE_POWER_ELECTROBOT
// ========================================================
#define ID_COMMANDE_POWER_ELECTROBOT 0x60
#define DLC_COMMANDE_POWER_ELECTROBOT 4
class CTrame_COMMANDE_POWER_ELECTROBOT : public CTrameBot
{
   Q_OBJECT
public :
    CTrame_COMMANDE_POWER_ELECTROBOT(CMessagerieBot *messagerie_bot, CDataManager *data_manager);
    ~CTrame_COMMANDE_POWER_ELECTROBOT() { }
    /*virtual*/ void Encode(void);

private :
    unsigned short commande;
    unsigned short value;
    bool m_synchro_tx;

private slots :
    void commande_changed(QVariant val);
    void value_changed(QVariant val);

    void Synchro_changed(QVariant val);
};

// ========================================================
//             TRAME ETAT_POWER_ELECTROBOT
// ========================================================
#define ID_ETAT_POWER_ELECTROBOT 0x62
#define DLC_ETAT_POWER_ELECTROBOT 8
#define BRUTE2PHYS_battery_voltage(val) ( ((float)val * (0.001000)) + (0.000000) )
#define PHYS2BRUTE_battery_voltage(val) (unsigned char)( (val - (0.000000)) / (0.001000) )
#define BRUTE2PHYS_global_current(val) ( ((float)val * (0.001000)) + (0.000000) )
#define PHYS2BRUTE_global_current(val) (unsigned char)( (val - (0.000000)) / (0.001000) )
#define BRUTE2PHYS_current_out1(val) ( ((float)val * (0.001000)) + (0.000000) )
#define PHYS2BRUTE_current_out1(val) (unsigned char)( (val - (0.000000)) / (0.001000) )
#define BRUTE2PHYS_current_out2(val) ( ((float)val * (0.001000)) + (0.000000) )
#define PHYS2BRUTE_current_out2(val) (unsigned char)( (val - (0.000000)) / (0.001000) )
class CTrame_ETAT_POWER_ELECTROBOT : public CTrameBot
{
   Q_OBJECT
public :
    CTrame_ETAT_POWER_ELECTROBOT(CMessagerieBot *messagerie_bot, CDataManager *data_manager);
    ~CTrame_ETAT_POWER_ELECTROBOT() { }
    /*virtual*/ void Decode(tStructTrameBrute *trameRecue);

 private :
    unsigned short battery_voltage_mV;
    unsigned short global_current_mA;
    unsigned short current_out1_mA;
    unsigned short current_out2_mA;
private slots :
};

// ========================================================
//             TRAME COMMANDE_MVT_XY
// ========================================================
#define ID_COMMANDE_MVT_XY 0x102
#define DLC_COMMANDE_MVT_XY 5
#define BRUTE2PHYS_Y_consigne(val) ( ((float)val * (1.000000)) + (0.000000) )
#define PHYS2BRUTE_Y_consigne(val) (short)( (val - (0.000000)) / (1.000000) )
#define BRUTE2PHYS_X_consigne(val) ( ((float)val * (1.000000)) + (0.000000) )
#define PHYS2BRUTE_X_consigne(val) (short)( (val - (0.000000)) / (1.000000) )
#define BRUTE2PHYS_Type_mouvement(val) ( ((float)val * (1.000000)) + (0.000000) )
#define PHYS2BRUTE_Type_mouvement(val) (unsigned char)( (val - (0.000000)) / (1.000000) )
class CTrame_COMMANDE_MVT_XY : public CTrameBot
{
   Q_OBJECT
public :
    CTrame_COMMANDE_MVT_XY(CMessagerieBot *messagerie_bot, CDataManager *data_manager);
    ~CTrame_COMMANDE_MVT_XY() { }
    /*virtual*/ void Encode(void);

 private :
    short Y_consigne;
    short X_consigne;
    unsigned char Type_mouvement;
    bool m_synchro_tx;

private slots :
    void Y_consigne_changed(QVariant val);
    void X_consigne_changed(QVariant val);
    void Type_mouvement_changed(QVariant val);
    void Synchro_changed(QVariant val);
};


// ========================================================
//             TRAME ASSERV_RESET
// ========================================================
#define ID_ASSERV_RESET 0x132
#define DLC_ASSERV_RESET 1
#define BRUTE2PHYS_SECURITE_RESET(val) ( ((float)val * (1.000000)) + (0.000000) )
#define PHYS2BRUTE_SECURITE_RESET(val) (unsigned char)( (val - (0.000000)) / (1.000000) )
class CTrame_ASSERV_RESET : public CTrameBot
{
   Q_OBJECT
public :
    CTrame_ASSERV_RESET(CMessagerieBot *messagerie_bot, CDataManager *data_manager);
    ~CTrame_ASSERV_RESET() { }
    /*virtual*/ void Encode(void);

 private :
    unsigned char SECURITE_RESET;
    bool m_synchro_tx;

private slots :
    void SECURITE_RESET_changed(QVariant val);
    void Synchro_changed(QVariant val);
};


// ========================================================
//             TRAME AUTOAPPRENTISSAGE_ASSERV
// ========================================================
#define ID_AUTOAPPRENTISSAGE_ASSERV 0x107
#define DLC_AUTOAPPRENTISSAGE_ASSERV 1
#define BRUTE2PHYS_Type_autoapprentissage(val) ( ((float)val * (1.000000)) + (0.000000) )
#define PHYS2BRUTE_Type_autoapprentissage(val) (unsigned char)( (val - (0.000000)) / (1.000000) )
class CTrame_AUTOAPPRENTISSAGE_ASSERV : public CTrameBot
{
   Q_OBJECT
public :
    CTrame_AUTOAPPRENTISSAGE_ASSERV(CMessagerieBot *messagerie_bot, CDataManager *data_manager);
    ~CTrame_AUTOAPPRENTISSAGE_ASSERV() { }
    /*virtual*/ void Encode(void);

 private :
    unsigned char Type_autoapprentissage;
    bool m_synchro_tx;

private slots :
    void Type_autoapprentissage_changed(QVariant val);
    void Synchro_changed(QVariant val);
};


// ========================================================
//             TRAME COMMANDE_REINIT_XY_TETA
// ========================================================
#define ID_COMMANDE_REINIT_XY_TETA 0x106
#define DLC_COMMANDE_REINIT_XY_TETA 6
#define BRUTE2PHYS_reinit_teta_pos(val) ( ((float)val * (0.001000)) + (0.000000) )
#define PHYS2BRUTE_reinit_teta_pos(val) (short)( (val - (0.000000)) / (0.001000) )
#define BRUTE2PHYS_reinit_y_pos(val) ( ((float)val * (1.000000)) + (0.000000) )
#define PHYS2BRUTE_reinit_y_pos(val) (short)( (val - (0.000000)) / (1.000000) )
#define BRUTE2PHYS_reinit_x_pos(val) ( ((float)val * (1.000000)) + (0.000000) )
#define PHYS2BRUTE_reinit_x_pos(val) (short)( (val - (0.000000)) / (1.000000) )
class CTrame_COMMANDE_REINIT_XY_TETA : public CTrameBot
{
   Q_OBJECT
public :
    CTrame_COMMANDE_REINIT_XY_TETA(CMessagerieBot *messagerie_bot, CDataManager *data_manager);
    ~CTrame_COMMANDE_REINIT_XY_TETA() { }
    /*virtual*/ void Encode(void);

 private :
    short reinit_teta_pos;
    short reinit_y_pos;
    short reinit_x_pos;
    bool m_synchro_tx;

private slots :
    void reinit_teta_pos_changed(QVariant val);
    void reinit_y_pos_changed(QVariant val);
    void reinit_x_pos_changed(QVariant val);
    void Synchro_changed(QVariant val);
};


// ========================================================
//             TRAME COMMANDE_VITESSE_MVT
// ========================================================
#define ID_COMMANDE_VITESSE_MVT 0x105
#define DLC_COMMANDE_VITESSE_MVT 6
#define BRUTE2PHYS_indice_sportivite_decel(val) ( ((float)val * (1.000000)) + (0.000000) )
#define PHYS2BRUTE_indice_sportivite_decel(val) (unsigned char)( (val - (0.000000)) / (1.000000) )
#define BRUTE2PHYS_indice_sportivite_accel(val) ( ((float)val * (1.000000)) + (0.000000) )
#define PHYS2BRUTE_indice_sportivite_accel(val) (unsigned char)( (val - (0.000000)) / (1.000000) )
#define BRUTE2PHYS_vitesse_rotation_max(val) ( ((float)val * (0.001000)) + (0.000000) )
#define PHYS2BRUTE_vitesse_rotation_max(val) (unsigned short)( (val - (0.000000)) / (0.001000) )
#define BRUTE2PHYS_vitesse_avance_max(val) ( ((float)val * (1.000000)) + (0.000000) )
#define PHYS2BRUTE_vitesse_avance_max(val) (unsigned short)( (val - (0.000000)) / (1.000000) )
class CTrame_COMMANDE_VITESSE_MVT : public CTrameBot
{
   Q_OBJECT
public :
    CTrame_COMMANDE_VITESSE_MVT(CMessagerieBot *messagerie_bot, CDataManager *data_manager);
    ~CTrame_COMMANDE_VITESSE_MVT() { }
    /*virtual*/ void Encode(void);

 private :
    unsigned char indice_sportivite_decel;
    unsigned char indice_sportivite_accel;
    unsigned short vitesse_rotation_max;
    unsigned short vitesse_avance_max;
    bool m_synchro_tx;

private slots :
    void indice_sportivite_decel_changed(QVariant val);
    void indice_sportivite_accel_changed(QVariant val);
    void vitesse_rotation_max_changed(QVariant val);
    void vitesse_avance_max_changed(QVariant val);
    void Synchro_changed(QVariant val);
};


// ========================================================
//             TRAME COMMANDE_REGUL_VITESSE
// ========================================================
#define ID_COMMANDE_REGUL_VITESSE 0x104
#define DLC_COMMANDE_REGUL_VITESSE 4
#define BRUTE2PHYS_consigne_vitesse_roue_D(val) ( ((float)val * (1.000000)) + (0.000000) )
#define PHYS2BRUTE_consigne_vitesse_roue_D(val) (short)( (val - (0.000000)) / (1.000000) )
#define BRUTE2PHYS_consigne_vitesse_roue_G(val) ( ((float)val * (1.000000)) + (0.000000) )
#define PHYS2BRUTE_consigne_vitesse_roue_G(val) (short)( (val - (0.000000)) / (1.000000) )
class CTrame_COMMANDE_REGUL_VITESSE : public CTrameBot
{
   Q_OBJECT
public :
    CTrame_COMMANDE_REGUL_VITESSE(CMessagerieBot *messagerie_bot, CDataManager *data_manager);
    ~CTrame_COMMANDE_REGUL_VITESSE() { }
    /*virtual*/ void Encode(void);

 private :
    short consigne_vitesse_roue_D;
    short consigne_vitesse_roue_G;
    bool m_synchro_tx;

private slots :
    void consigne_vitesse_roue_D_changed(QVariant val);
    void consigne_vitesse_roue_G_changed(QVariant val);
    void Synchro_changed(QVariant val);
};


// ========================================================
//             TRAME COMMANDE_DISTANCE_ANGLE
// ========================================================
#define ID_COMMANDE_DISTANCE_ANGLE 0x103
#define DLC_COMMANDE_DISTANCE_ANGLE 5
#define BRUTE2PHYS_angle_consigne(val) ( ((float)val * (0.001000)) + (0.000000) )
#define PHYS2BRUTE_angle_consigne(val) (short)( (val - (0.000000)) / (0.001000) )
#define BRUTE2PHYS_distance_consigne(val) ( ((float)val * (1.000000)) + (0.000000) )
#define PHYS2BRUTE_distance_consigne(val) (short)( (val - (0.000000)) / (1.000000) )
#define BRUTE2PHYS_priorite_mouvement(val) ( ((float)val * (1.000000)) + (0.000000) )
#define PHYS2BRUTE_priorite_mouvement(val) (unsigned char)( (val - (0.000000)) / (1.000000) )
class CTrame_COMMANDE_DISTANCE_ANGLE : public CTrameBot
{
   Q_OBJECT
public :
    CTrame_COMMANDE_DISTANCE_ANGLE(CMessagerieBot *messagerie_bot, CDataManager *data_manager);
    ~CTrame_COMMANDE_DISTANCE_ANGLE() { }
    /*virtual*/ void Encode(void);

 private :
    short angle_consigne;
    short distance_consigne;
    unsigned char priorite_mouvement;
    bool m_synchro_tx;

private slots :
    void angle_consigne_changed(QVariant val);
    void distance_consigne_changed(QVariant val);
    void priorite_mouvement_changed(QVariant val);
    void Synchro_changed(QVariant val);
};


// ========================================================
//             TRAME COMMANDE_MVT_XY_TETA
// ========================================================
#define ID_COMMANDE_MVT_XY_TETA 0x101
#define DLC_COMMANDE_MVT_XY_TETA 7
#define BRUTE2PHYS_angle_consigne(val) ( ((float)val * (0.001000)) + (0.000000) )
#define PHYS2BRUTE_angle_consigne(val) (short)( (val - (0.000000)) / (0.001000) )
#define BRUTE2PHYS_Y_consigne(val) ( ((float)val * (1.000000)) + (0.000000) )
#define PHYS2BRUTE_Y_consigne(val) (short)( (val - (0.000000)) / (1.000000) )
#define BRUTE2PHYS_X_consigne(val) ( ((float)val * (1.000000)) + (0.000000) )
#define PHYS2BRUTE_X_consigne(val) (short)( (val - (0.000000)) / (1.000000) )
#define BRUTE2PHYS_Type_mouvement(val) ( ((float)val * (1.000000)) + (0.000000) )
#define PHYS2BRUTE_Type_mouvement(val) (unsigned char)( (val - (0.000000)) / (1.000000) )
class CTrame_COMMANDE_MVT_XY_TETA : public CTrameBot
{
   Q_OBJECT
public :
    CTrame_COMMANDE_MVT_XY_TETA(CMessagerieBot *messagerie_bot, CDataManager *data_manager);
    ~CTrame_COMMANDE_MVT_XY_TETA() { }
    /*virtual*/ void Encode(void);

 private :
    short angle_consigne;
    short Y_consigne;
    short X_consigne;
    unsigned char Type_mouvement;
    bool m_synchro_tx;

private slots :
    void angle_consigne_changed(QVariant val);
    void Y_consigne_changed(QVariant val);
    void X_consigne_changed(QVariant val);
    void Type_mouvement_changed(QVariant val);
    void Synchro_changed(QVariant val);
};


// ========================================================
//             TRAME ASSERV_DIAG_WRITE_PARAM
// ========================================================
#define ID_ASSERV_DIAG_WRITE_PARAM 0x131
#define DLC_ASSERV_DIAG_WRITE_PARAM 4
#define BRUTE2PHYS_ASSERV_DIAG_WRITE_VALUE(val) ( ((float)val * (1.000000)) + (0.000000) )
#define PHYS2BRUTE_ASSERV_DIAG_WRITE_VALUE(val) (short)( (val - (0.000000)) / (1.000000) )
#define BRUTE2PHYS_ASSERV_DIAG_WRITE_PARAM(val) ( ((float)val * (1.000000)) + (0.000000) )
#define PHYS2BRUTE_ASSERV_DIAG_WRITE_PARAM(val) (unsigned short)( (val - (0.000000)) / (1.000000) )
class CTrame_ASSERV_DIAG_WRITE_PARAM : public CTrameBot
{
   Q_OBJECT
public :
    CTrame_ASSERV_DIAG_WRITE_PARAM(CMessagerieBot *messagerie_bot, CDataManager *data_manager);
    ~CTrame_ASSERV_DIAG_WRITE_PARAM() { }
    /*virtual*/ void Encode(void);

 private :
    short ASSERV_DIAG_WRITE_VALUE;
    unsigned short ASSERV_DIAG_WRITE_PARAM;
    bool m_synchro_tx;

private slots :
    void ASSERV_DIAG_WRITE_VALUE_changed(QVariant val);
    void ASSERV_DIAG_WRITE_PARAM_changed(QVariant val);
    void Synchro_changed(QVariant val);
};


// ========================================================
//             TRAME ELECTROBOT_CDE_SERVOS
// ========================================================
#define ID_ELECTROBOT_CDE_SERVOS 0x51
#define DLC_ELECTROBOT_CDE_SERVOS 8
#define BRUTE2PHYS_PositionServoMoteur2(val) ( ((float)val * (1.000000)) + (0.000000) )
#define PHYS2BRUTE_PositionServoMoteur2(val) (unsigned short)( (val - (0.000000)) / (1.000000) )
#define BRUTE2PHYS_VitesseServoMoteur2(val) ( ((float)val * (1.000000)) + (0.000000) )
#define PHYS2BRUTE_VitesseServoMoteur2(val) (unsigned char)( (val - (0.000000)) / (1.000000) )
#define BRUTE2PHYS_NumeroServoMoteur2(val) ( ((float)val * (1.000000)) + (0.000000) )
#define PHYS2BRUTE_NumeroServoMoteur2(val) (unsigned char)( (val - (0.000000)) / (1.000000) )
#define BRUTE2PHYS_PositionServoMoteur1(val) ( ((float)val * (1.000000)) + (0.000000) )
#define PHYS2BRUTE_PositionServoMoteur1(val) (unsigned short)( (val - (0.000000)) / (1.000000) )
#define BRUTE2PHYS_VitesseServoMoteur1(val) ( ((float)val * (1.000000)) + (0.000000) )
#define PHYS2BRUTE_VitesseServoMoteur1(val) (unsigned char)( (val - (0.000000)) / (1.000000) )
#define BRUTE2PHYS_NumeroServoMoteur1(val) ( ((float)val * (1.000000)) + (0.000000) )
#define PHYS2BRUTE_NumeroServoMoteur1(val) (unsigned char)( (val - (0.000000)) / (1.000000) )
class CTrame_ELECTROBOT_CDE_SERVOS : public CTrameBot
{
   Q_OBJECT
public :
    CTrame_ELECTROBOT_CDE_SERVOS(CMessagerieBot *messagerie_bot, CDataManager *data_manager);
    ~CTrame_ELECTROBOT_CDE_SERVOS() { }
    /*virtual*/ void Encode(void);

 private :
    unsigned short PositionServoMoteur2;
    unsigned char VitesseServoMoteur2;
    unsigned char NumeroServoMoteur2;
    unsigned short PositionServoMoteur1;
    unsigned char VitesseServoMoteur1;
    unsigned char NumeroServoMoteur1;
    bool m_synchro_tx;

private slots :
    void PositionServoMoteur2_changed(QVariant val);
    void VitesseServoMoteur2_changed(QVariant val);
    void NumeroServoMoteur2_changed(QVariant val);
    void PositionServoMoteur1_changed(QVariant val);
    void VitesseServoMoteur1_changed(QVariant val);
    void NumeroServoMoteur1_changed(QVariant val);
    void Synchro_changed(QVariant val);
};


// ========================================================
//             TRAME COMMANDE_MVT_MANUEL
// ========================================================
#define ID_COMMANDE_MVT_MANUEL 0x100
#define DLC_COMMANDE_MVT_MANUEL 4
#define BRUTE2PHYS_PuissanceMotD(val) ( ((float)val * (1.000000)) + (0.000000) )
#define PHYS2BRUTE_PuissanceMotD(val) (short)( (val - (0.000000)) / (1.000000) )
#define BRUTE2PHYS_PuissanceMotG(val) ( ((float)val * (1.000000)) + (0.000000) )
#define PHYS2BRUTE_PuissanceMotG(val) (short)( (val - (0.000000)) / (1.000000) )
class CTrame_COMMANDE_MVT_MANUEL : public CTrameBot
{
   Q_OBJECT
public :
    CTrame_COMMANDE_MVT_MANUEL(CMessagerieBot *messagerie_bot, CDataManager *data_manager);
    ~CTrame_COMMANDE_MVT_MANUEL() { }
    /*virtual*/ void Encode(void);

 private :
    short PuissanceMotD;
    short PuissanceMotG;
    bool m_synchro_tx;

private slots :
    void PuissanceMotD_changed(QVariant val);
    void PuissanceMotG_changed(QVariant val);
    void Synchro_changed(QVariant val);
};


// ========================================================
//             TRAME ETAT_PID_ASSERVISSEMENT
// ========================================================
#define ID_ETAT_PID_ASSERVISSEMENT 0x153
#define DLC_ETAT_PID_ASSERVISSEMENT 8
#define BRUTE2PHYS_consigne_vitesse_rotation_filt(val) ( ((float)val * (0.000100)) + (0.000000) )
#define PHYS2BRUTE_consigne_vitesse_rotation_filt(val) (short)( (val - (0.000000)) / (0.000100) )
#define BRUTE2PHYS_vitesse_rotation_robot_filt(val) ( ((float)val * (0.000100)) + (0.000000) )
#define PHYS2BRUTE_vitesse_rotation_robot_filt(val) (short)( (val - (0.000000)) / (0.000100) )
#define BRUTE2PHYS_consigne_vitesse_avance_filt(val) ( ((float)val * (0.01000)) + (0.000000) )
#define PHYS2BRUTE_consigne_vitesse_avance_filt(val) (short)( (val - (0.000000)) / (0.01000) )
#define BRUTE2PHYS_vitesse_avance_robot_filt(val) ( ((float)val * (0.01000)) + (0.000000) )
#define PHYS2BRUTE_vitesse_avance_robot_filt(val) (short)( (val - (0.000000)) / (0.01000) )
class CTrame_ETAT_PID_ASSERVISSEMENT : public CTrameBot
{
   Q_OBJECT
public :
    CTrame_ETAT_PID_ASSERVISSEMENT(CMessagerieBot *messagerie_bot, CDataManager *data_manager);
    ~CTrame_ETAT_PID_ASSERVISSEMENT() { }
    /*virtual*/ void Decode(tStructTrameBrute *trameRecue);

 private :
    short consigne_vitesse_rotation_filt;
    short vitesse_rotation_robot_filt;
    short consigne_vitesse_avance_filt;
    short vitesse_avance_robot_filt;

private slots :
};


// ========================================================
//             TRAME ETAT_ASSERVISSEMENT
// ========================================================
#define ID_ETAT_ASSERVISSEMENT 0x150
#define DLC_ETAT_ASSERVISSEMENT 8
#define BRUTE2PHYS_compteur_diag_blocage(val) ( ((float)val * (1.000000)) + (0.000000) )
#define PHYS2BRUTE_compteur_diag_blocage(val) (unsigned short)( (val - (0.000000)) / (1.000000) )
#define BRUTE2PHYS_ModeAsservissement(val) ( ((float)val * (1.000000)) + (0.000000) )
#define PHYS2BRUTE_ModeAsservissement(val) (unsigned char)( (val - (0.000000)) / (1.000000) )
#define BRUTE2PHYS_cde_moteur_D(val) ( ((float)val * (1.000000)) + (0.000000) )
#define PHYS2BRUTE_cde_moteur_D(val) (short)( (val - (0.000000)) / (1.000000) )
#define BRUTE2PHYS_cde_moteur_G(val) ( ((float)val * (1.000000)) + (0.000000) )
#define PHYS2BRUTE_cde_moteur_G(val) (short)( (val - (0.000000)) / (1.000000) )
#define BRUTE2PHYS_Convergence(val) ( ((float)val * (1.000000)) + (0.000000) )
#define PHYS2BRUTE_Convergence(val) (unsigned char)( (val - (0.000000)) / (1.000000) )
class CTrame_ETAT_ASSERVISSEMENT : public CTrameBot
{
   Q_OBJECT
public :
    CTrame_ETAT_ASSERVISSEMENT(CMessagerieBot *messagerie_bot, CDataManager *data_manager);
    ~CTrame_ETAT_ASSERVISSEMENT() { }
    /*virtual*/ void Decode(tStructTrameBrute *trameRecue);

 private :
    unsigned short compteur_diag_blocage;
    unsigned char ModeAsservissement;
    short cde_moteur_D;
    short cde_moteur_G;
    unsigned char Convergence;

private slots :
};


// ========================================================
//             TRAME POSITION_CODEURS
// ========================================================
#define ID_POSITION_CODEURS 0x152
#define DLC_POSITION_CODEURS 8
#define BRUTE2PHYS_PosCodeurG(val) ( ((float)val * (1.000000)) + (0.000000) )
#define PHYS2BRUTE_PosCodeurG(val) (short)( (val - (0.000000)) / (1.000000) )
#define BRUTE2PHYS_PosCodeurD(val) ( ((float)val * (1.000000)) + (0.000000) )
#define PHYS2BRUTE_PosCodeurD(val) (short)( (val - (0.000000)) / (1.000000) )
class CTrame_POSITION_CODEURS : public CTrameBot
{
   Q_OBJECT
public :
    CTrame_POSITION_CODEURS(CMessagerieBot *messagerie_bot, CDataManager *data_manager);
    ~CTrame_POSITION_CODEURS() { }
    /*virtual*/ void Decode(tStructTrameBrute *trameRecue);

 private :
    short PosCodeurG;
    short PosCodeurD;

private slots :
};


// ========================================================
//             TRAME POSITION_ABSOLUE_XY_TETA
// ========================================================
#define ID_POSITION_ABSOLUE_XY_TETA 0x151
#define DLC_POSITION_ABSOLUE_XY_TETA 6
#define BRUTE2PHYS_teta_pos(val) ( ((float)val * (0.000100)) + (0.000000) )
#define PHYS2BRUTE_teta_pos(val) (short)( (val - (0.000000)) / (0.000100) )
#define BRUTE2PHYS_y_pos(val) ( ((float)val * (0.100000)) + (0.000000) )
#define PHYS2BRUTE_y_pos(val) (short)( (val - (0.000000)) / (0.100000) )
#define BRUTE2PHYS_x_pos(val) ( ((float)val * (0.100000)) + (0.000000) )
#define PHYS2BRUTE_x_pos(val) (short)( (val - (0.000000)) / (0.100000) )
class CTrame_POSITION_ABSOLUE_XY_TETA : public CTrameBot
{
   Q_OBJECT
public :
    CTrame_POSITION_ABSOLUE_XY_TETA(CMessagerieBot *messagerie_bot, CDataManager *data_manager);
    ~CTrame_POSITION_ABSOLUE_XY_TETA() { }
    /*virtual*/ void Decode(tStructTrameBrute *trameRecue);

 private :
    short teta_pos;
    short y_pos;
    short x_pos;

private slots :
};


// ========================================================
//             TRAME ELECTROBOT_ETAT_CODEURS_1_2
// ========================================================
#define ID_ELECTROBOT_ETAT_CODEURS_1_2 0x30
#define DLC_ELECTROBOT_ETAT_CODEURS_1_2 8
#define BRUTE2PHYS_Codeur_2(val) ( ((float)val * (1.000000)) + (0.000000) )
#define PHYS2BRUTE_Codeur_2(val) (long)( (val - (0.000000)) / (1.000000) )
#define BRUTE2PHYS_Codeur_1(val) ( ((float)val * (1.000000)) + (0.000000) )
#define PHYS2BRUTE_Codeur_1(val) (long)( (val - (0.000000)) / (1.000000) )
class CTrame_ELECTROBOT_ETAT_CODEURS_1_2 : public CTrameBot
{
   Q_OBJECT
public :
    CTrame_ELECTROBOT_ETAT_CODEURS_1_2(CMessagerieBot *messagerie_bot, CDataManager *data_manager);
    ~CTrame_ELECTROBOT_ETAT_CODEURS_1_2() { }
    /*virtual*/ void Decode(tStructTrameBrute *trameRecue);

 private :
    int Codeur_2;
    int Codeur_1;

private slots :
};


// ========================================================
//             TRAME ELECTROBOT_ETAT_TELEMETRES
// ========================================================
#define ID_ELECTROBOT_ETAT_TELEMETRES 0x40
#define DLC_ELECTROBOT_ETAT_TELEMETRES 6
#define BRUTE2PHYS_Telemetre5(val) ( ((float)val * (1.000000)) + (0.000000) )
#define PHYS2BRUTE_Telemetre5(val) (unsigned char)( (val - (0.000000)) / (1.000000) )
#define BRUTE2PHYS_Telemetre6(val) ( ((float)val * (1.000000)) + (0.000000) )
#define PHYS2BRUTE_Telemetre6(val) (unsigned char)( (val - (0.000000)) / (1.000000) )
#define BRUTE2PHYS_Telemetre4(val) ( ((float)val * (1.000000)) + (0.000000) )
#define PHYS2BRUTE_Telemetre4(val) (unsigned char)( (val - (0.000000)) / (1.000000) )
#define BRUTE2PHYS_Telemetre3(val) ( ((float)val * (1.000000)) + (0.000000) )
#define PHYS2BRUTE_Telemetre3(val) (unsigned char)( (val - (0.000000)) / (1.000000) )
#define BRUTE2PHYS_Telemetre2(val) ( ((float)val * (1.000000)) + (0.000000) )
#define PHYS2BRUTE_Telemetre2(val) (unsigned char)( (val - (0.000000)) / (1.000000) )
#define BRUTE2PHYS_Telemetre1(val) ( ((float)val * (1.000000)) + (0.000000) )
#define PHYS2BRUTE_Telemetre1(val) (unsigned char)( (val - (0.000000)) / (1.000000) )
class CTrame_ELECTROBOT_ETAT_TELEMETRES : public CTrameBot
{
   Q_OBJECT
public :
    CTrame_ELECTROBOT_ETAT_TELEMETRES(CMessagerieBot *messagerie_bot, CDataManager *data_manager);
    ~CTrame_ELECTROBOT_ETAT_TELEMETRES() { }
    /*virtual*/ void Decode(tStructTrameBrute *trameRecue);

 private :
    unsigned char Telemetre6;
    unsigned char Telemetre5;
    unsigned char Telemetre4;
    unsigned char Telemetre3;
    unsigned char Telemetre2;
    unsigned char Telemetre1;

private slots :
};


// ========================================================
//             TRAME ELECTROBOT_ETAT_CODEURS_3_4
// ========================================================
#define ID_ELECTROBOT_ETAT_CODEURS_3_4 0x31
#define DLC_ELECTROBOT_ETAT_CODEURS_3_4 8
#define BRUTE2PHYS_Codeur_4(val) ( ((float)val * (1.000000)) + (0.000000) )
#define PHYS2BRUTE_Codeur_4(val) (long)( (val - (0.000000)) / (1.000000) )
#define BRUTE2PHYS_Codeur_3(val) ( ((float)val * (1.000000)) + (0.000000) )
#define PHYS2BRUTE_Codeur_3(val) (long)( (val - (0.000000)) / (1.000000) )
class CTrame_ELECTROBOT_ETAT_CODEURS_3_4 : public CTrameBot
{
   Q_OBJECT
public :
    CTrame_ELECTROBOT_ETAT_CODEURS_3_4(CMessagerieBot *messagerie_bot, CDataManager *data_manager);
    ~CTrame_ELECTROBOT_ETAT_CODEURS_3_4() { }
    /*virtual*/ void Decode(tStructTrameBrute *trameRecue);

 private :
    int Codeur_4;
    int Codeur_3;

private slots :
};


// ========================================================
//             TRAME ELECTROBOT_ETAT_CAPTEURS_2
// ========================================================
#define ID_ELECTROBOT_ETAT_CAPTEURS_2 0x20
#define DLC_ELECTROBOT_ETAT_CAPTEURS_2 8
#define BRUTE2PHYS_Etor_PGED1_dsPIC2(val) ( ((float)val * (1.000000)) + (0.000000) )
#define PHYS2BRUTE_Etor_PGED1_dsPIC2(val) (unsigned char)( (val - (0.000000)) / (1.000000) )
#define BRUTE2PHYS_Etor_PGED1_dsPIC1(val) ( ((float)val * (1.000000)) + (0.000000) )
#define PHYS2BRUTE_Etor_PGED1_dsPIC1(val) (unsigned char)( (val - (0.000000)) / (1.000000) )
#define BRUTE2PHYS_Etor_PGEC1_dsPIC2(val) ( ((float)val * (1.000000)) + (0.000000) )
#define PHYS2BRUTE_Etor_PGEC1_dsPIC2(val) (unsigned char)( (val - (0.000000)) / (1.000000) )
#define BRUTE2PHYS_Etor_PGEC1_dsPIC1(val) ( ((float)val * (1.000000)) + (0.000000) )
#define PHYS2BRUTE_Etor_PGEC1_dsPIC1(val) (unsigned char)( (val - (0.000000)) / (1.000000) )
#define BRUTE2PHYS_Etor_Codeur4_B(val) ( ((float)val * (1.000000)) + (0.000000) )
#define PHYS2BRUTE_Etor_Codeur4_B(val) (unsigned char)( (val - (0.000000)) / (1.000000) )
#define BRUTE2PHYS_Etor_Codeur4_A(val) ( ((float)val * (1.000000)) + (0.000000) )
#define PHYS2BRUTE_Etor_Codeur4_A(val) (unsigned char)( (val - (0.000000)) / (1.000000) )
#define BRUTE2PHYS_Etor_Codeur3_B(val) ( ((float)val * (1.000000)) + (0.000000) )
#define PHYS2BRUTE_Etor_Codeur3_B(val) (unsigned char)( (val - (0.000000)) / (1.000000) )
#define BRUTE2PHYS_Etor_Codeur3_A(val) ( ((float)val * (1.000000)) + (0.000000) )
#define PHYS2BRUTE_Etor_Codeur3_A(val) (unsigned char)( (val - (0.000000)) / (1.000000) )
#define BRUTE2PHYS_Etor_CAN_TX(val) ( ((float)val * (1.000000)) + (0.000000) )
#define PHYS2BRUTE_Etor_CAN_TX(val) (unsigned char)( (val - (0.000000)) / (1.000000) )
#define BRUTE2PHYS_Etor_CAN_RX(val) ( ((float)val * (1.000000)) + (0.000000) )
#define PHYS2BRUTE_Etor_CAN_RX(val) (unsigned char)( (val - (0.000000)) / (1.000000) )
#define BRUTE2PHYS_Etor6(val) ( ((float)val * (1.000000)) + (0.000000) )
#define PHYS2BRUTE_Etor6(val) (unsigned char)( (val - (0.000000)) / (1.000000) )
#define BRUTE2PHYS_Etor5(val) ( ((float)val * (1.000000)) + (0.000000) )
#define PHYS2BRUTE_Etor5(val) (unsigned char)( (val - (0.000000)) / (1.000000) )
#define BRUTE2PHYS_Etor4(val) ( ((float)val * (1.000000)) + (0.000000) )
#define PHYS2BRUTE_Etor4(val) (unsigned char)( (val - (0.000000)) / (1.000000) )
#define BRUTE2PHYS_Etor3(val) ( ((float)val * (1.000000)) + (0.000000) )
#define PHYS2BRUTE_Etor3(val) (unsigned char)( (val - (0.000000)) / (1.000000) )
#define BRUTE2PHYS_Etor2(val) ( ((float)val * (1.000000)) + (0.000000) )
#define PHYS2BRUTE_Etor2(val) (unsigned char)( (val - (0.000000)) / (1.000000) )
#define BRUTE2PHYS_Etor1(val) ( ((float)val * (1.000000)) + (0.000000) )
#define PHYS2BRUTE_Etor1(val) (unsigned char)( (val - (0.000000)) / (1.000000) )
#define BRUTE2PHYS_Vbat(val) ( ((float)val * (0.100000)) + (0.000000) )
#define PHYS2BRUTE_Vbat(val) (unsigned char)( (val - (0.000000)) / (0.100000) )
#define BRUTE2PHYS_Eana13(val) ( ((float)val * (0.100000)) + (0.000000) )
#define PHYS2BRUTE_Eana13(val) (unsigned char)( (val - (0.000000)) / (0.100000) )
#define BRUTE2PHYS_Eana12(val) ( ((float)val * (0.100000)) + (0.000000) )
#define PHYS2BRUTE_Eana12(val) (unsigned char)( (val - (0.000000)) / (0.100000) )
#define BRUTE2PHYS_Eana11(val) ( ((float)val * (0.100000)) + (0.000000) )
#define PHYS2BRUTE_Eana11(val) (unsigned char)( (val - (0.000000)) / (0.100000) )
#define BRUTE2PHYS_Eana10(val) ( ((float)val * (0.100000)) + (0.000000) )
#define PHYS2BRUTE_Eana10(val) (unsigned char)( (val - (0.000000)) / (0.100000) )
#define BRUTE2PHYS_Eana9(val) ( ((float)val * (0.100000)) + (0.000000) )
#define PHYS2BRUTE_Eana9(val) (unsigned char)( (val - (0.000000)) / (0.100000) )
class CTrame_ELECTROBOT_ETAT_CAPTEURS_2 : public CTrameBot
{
   Q_OBJECT
public :
    CTrame_ELECTROBOT_ETAT_CAPTEURS_2(CMessagerieBot *messagerie_bot, CDataManager *data_manager);
    ~CTrame_ELECTROBOT_ETAT_CAPTEURS_2() { }
    /*virtual*/ void Decode(tStructTrameBrute *trameRecue);

 private :
    unsigned char Etor_PGED1_dsPIC2;
    unsigned char Etor_PGED1_dsPIC1;
    unsigned char Etor_PGEC1_dsPIC2;
    unsigned char Etor_PGEC1_dsPIC1;
    unsigned char Etor_Codeur4_B;
    unsigned char Etor_Codeur4_A;
    unsigned char Etor_Codeur3_B;
    unsigned char Etor_Codeur3_A;
    unsigned char Etor_CAN_TX;
    unsigned char Etor_CAN_RX;
    unsigned char Etor6;
    unsigned char Etor5;
    unsigned char Etor4;
    unsigned char Etor3;
    unsigned char Etor2;
    unsigned char Etor1;
    unsigned char Vbat;
    unsigned char Eana13;
    unsigned char Eana12;
    unsigned char Eana11;
    unsigned char Eana10;
    unsigned char Eana9;

private slots :
};


// ========================================================
//             TRAME ELECTROBOT_ETAT_CAPTEURS_1
// ========================================================
#define ID_ELECTROBOT_ETAT_CAPTEURS_1 0x10
#define DLC_ELECTROBOT_ETAT_CAPTEURS_1 8
#define BRUTE2PHYS_Eana8(val) ( ((float)val * (0.100000)) + (0.000000) )
#define PHYS2BRUTE_Eana8(val) (unsigned char)( (val - (0.000000)) / (0.100000) )
#define BRUTE2PHYS_Eana7(val) ( ((float)val * (0.100000)) + (0.000000) )
#define PHYS2BRUTE_Eana7(val) (unsigned char)( (val - (0.000000)) / (0.100000) )
#define BRUTE2PHYS_Eana6(val) ( ((float)val * (0.100000)) + (0.000000) )
#define PHYS2BRUTE_Eana6(val) (unsigned char)( (val - (0.000000)) / (0.100000) )
#define BRUTE2PHYS_Eana5(val) ( ((float)val * (0.100000)) + (0.000000) )
#define PHYS2BRUTE_Eana5(val) (unsigned char)( (val - (0.000000)) / (0.100000) )
#define BRUTE2PHYS_Eana4(val) ( ((float)val * (0.100000)) + (0.000000) )
#define PHYS2BRUTE_Eana4(val) (unsigned char)( (val - (0.000000)) / (0.100000) )
#define BRUTE2PHYS_Eana3(val) ( ((float)val * (0.100000)) + (0.000000) )
#define PHYS2BRUTE_Eana3(val) (unsigned char)( (val - (0.000000)) / (0.100000) )
#define BRUTE2PHYS_Eana2(val) ( ((float)val * (0.100000)) + (0.000000) )
#define PHYS2BRUTE_Eana2(val) (unsigned char)( (val - (0.000000)) / (0.100000) )
#define BRUTE2PHYS_Eana1(val) ( ((float)val * (0.100000)) + (0.000000) )
#define PHYS2BRUTE_Eana1(val) (unsigned char)( (val - (0.000000)) / (0.100000) )
class CTrame_ELECTROBOT_ETAT_CAPTEURS_1 : public CTrameBot
{
   Q_OBJECT
public :
    CTrame_ELECTROBOT_ETAT_CAPTEURS_1(CMessagerieBot *messagerie_bot, CDataManager *data_manager);
    ~CTrame_ELECTROBOT_ETAT_CAPTEURS_1() { }
    /*virtual*/ void Decode(tStructTrameBrute *trameRecue);

 private :
    unsigned char Eana8;
    unsigned char Eana7;
    unsigned char Eana6;
    unsigned char Eana5;
    unsigned char Eana4;
    unsigned char Eana3;
    unsigned char Eana2;
    unsigned char Eana1;

private slots :
};

// ========================================================
//             TRAME ECRAN_ETAT_MATCH
// ========================================================
#define ID_ECRAN_ETAT_MATCH 0x41
#define DLC_ECRAN_ETAT_MATCH 6
#define BRUTE2PHYS_ObstacleDetecte(val) ( ((float)val * (1.000000)) + (0.000000) )
#define PHYS2BRUTE_ObstacleDetecte(val) (unsigned char)( (val - (0.000000)) / (1.000000) )
#define BRUTE2PHYS_OrigineDetectionObstacle(val) ( ((float)val * (1.000000)) + (0.000000) )
#define PHYS2BRUTE_OrigineDetectionObstacle(val) (unsigned char)( (val - (0.000000)) / (1.000000) )
#define BRUTE2PHYS_DiagBlocage(val) ( ((float)val * (1.000000)) + (0.000000) )
#define PHYS2BRUTE_DiagBlocage(val) (unsigned char)( (val - (0.000000)) / (1.000000) )
#define BRUTE2PHYS_ConvergenceAsserv(val) ( ((float)val * (1.000000)) + (0.000000) )
#define PHYS2BRUTE_ConvergenceAsserv(val) (unsigned char)( (val - (0.000000)) / (1.000000) )
#define BRUTE2PHYS_ModeFonctionnement(val) ( ((float)val * (1.000000)) + (0.000000) )
#define PHYS2BRUTE_ModeFonctionnement(val) (unsigned char)( (val - (0.000000)) / (1.000000) )
#define BRUTE2PHYS_CouleurEquipe(val) ( ((float)val * (1.000000)) + (0.000000) )
#define PHYS2BRUTE_CouleurEquipe(val) (unsigned char)( (val - (0.000000)) / (1.000000) )
#define BRUTE2PHYS_TempsMatch(val) ( ((float)val * (1.000000)) + (0.000000) )
#define PHYS2BRUTE_TempsMatch(val) (unsigned char)( (val - (0.000000)) / (1.000000) )
#define BRUTE2PHYS_Score(val) ( ((float)val * (1.000000)) + (0.000000) )
#define PHYS2BRUTE_Score(val) (unsigned short)( (val - (0.000000)) / (1.000000) )
class CTrame_ECRAN_ETAT_MATCH : public CTrameBot
{
   Q_OBJECT
public :
    CTrame_ECRAN_ETAT_MATCH(CMessagerieBot *messagerie_bot, CDataManager *data_manager);
    ~CTrame_ECRAN_ETAT_MATCH() { }
    /*virtual*/ void Decode(tStructTrameBrute *trameRecue);

 private :
    // Encode chacun des signaux de la trame
    unsigned char ObstacleDetecte;
    unsigned char OrigineDetectionObstacle; // 0=Ultrasons / 1=LIDAR
    unsigned char DiagBlocage;
    unsigned char ConvergenceAsserv;
    unsigned char ModeFonctionnement;
    unsigned char CouleurEquipe;
    unsigned char NumStrategie;
    unsigned char TempsMatch;
    unsigned short Score;

private slots :
};

// ========================================================
//             TRAME ETAT_EVITEMENT_OBSTACLE
// ========================================================
#define ID_ETAT_EVITEMENT_OBSTACLE 0x42
#define DLC_ETAT_EVITEMENT_OBSTACLE 8
class CTrame_ETAT_EVITEMENT_OBSTACLE : public CTrameBot
{
   Q_OBJECT
public :
    CTrame_ETAT_EVITEMENT_OBSTACLE(CMessagerieBot *messagerie_bot, CDataManager *data_manager);
    ~CTrame_ETAT_EVITEMENT_OBSTACLE() { }
    /*virtual*/ void Decode(tStructTrameBrute *trameRecue);

 private :
    signed char SensDeplacement;
    unsigned char ObstacleBitfield;
    unsigned char NumeroEtape;
    unsigned char NombreTentatives;
    unsigned char ChoixStrategieEvitement;
    bool EvitementEnCours;
    bool ObstacleDetecte;
    bool ObstacleInhibe;
    bool ForcageDetectObstacleSansPosition;

private slots :
};

// ========================================================
//             TRAME CONFIG_PERIODE_TRAME
// ========================================================
#define ID_CONFIG_PERIODE_TRAME 0x108
#define DLC_CONFIG_PERIODE_TRAME 4

class CTrame_CONFIG_PERIODE_TRAME : public CTrameBot
{
   Q_OBJECT
public :
    CTrame_CONFIG_PERIODE_TRAME(CMessagerieBot *messagerie_bot, CDataManager *data_manager);
    ~CTrame_CONFIG_PERIODE_TRAME() { }
    /*virtual*/ void Encode(void);

 private :
    short CONFIG_PERIODE_TRAME_Periode;
    unsigned short CONFIG_PERIODE_TRAME_ID;
    bool m_synchro_tx;

private slots :
    void CONFIG_PERIODE_TRAME_Periode_changed(QVariant val);
    void CONFIG_PERIODE_TRAME_ID_changed(QVariant val);
    void Synchro_changed(QVariant val);
};

// ========================================================
//             TRAME ECRAN_ETAT_ECRAN
// ========================================================
#define ID_ECRAN_ETAT_ECRAN 0x91
#define DLC_ECRAN_ETAT_ECRAN 4
#define BRUTE2PHYS_Valeur(val) ( ((float)val * (1.000000)) + (0.000000) )
#define PHYS2BRUTE_Valeur(val) (short)( (val - (0.000000)) / (1.000000) )
#define BRUTE2PHYS_CodeCommande(val) ( ((float)val * (1.000000)) + (0.000000) )
#define PHYS2BRUTE_CodeCommande(val) (unsigned short)( (val - (0.000000)) / (1.000000) )
class CTrame_ECRAN_ETAT_ECRAN : public CTrameBot
{
   Q_OBJECT
public :
    CTrame_ECRAN_ETAT_ECRAN(CMessagerieBot *messagerie_bot, CDataManager *data_manager);
    ~CTrame_ECRAN_ETAT_ECRAN() { }
    /*virtual*/ void Encode(void);

 private :
    // Encode chacun des signaux de la trame
    short Valeur;
    unsigned short CodeCommande;
    bool m_synchro_tx;

private slots :
    void Valeur_etat_ecran_changed(QVariant val);
    void CodeCommande_etat_ecran_changed(QVariant val);
    void Synchro_changed(QVariant val);
};

// ========================================================
//             TRAME ETAT_RACK
// ========================================================
#define ID_ETAT_RACK 0x154
#define DLC_ETAT_RACK 8
#define BRUTE2PHYS_rack_reserve(val) ( ((float)val * (1.000000)) + (0.000000) )
#define PHYS2BRUTE_rack_reserve(val) (short)( (val - (0.000000)) / (1.000000) )
#define BRUTE2PHYS_rack_modeAsservissement(val) ( ((float)val * (1.000000)) + (0.000000) )
#define PHYS2BRUTE_rack_modeAsservissement(val) (unsigned char)( (val - (0.000000)) / (1.000000) )
#define BRUTE2PHYS_rack_cde_moteur(val) ( ((float)val * (1.000000)) + (0.000000) )
#define PHYS2BRUTE_rack_cde_moteur(val) (short)( (val - (0.000000)) / (1.000000) )
#define BRUTE2PHYS_rack_consigne_moteur(val) ( ((float)val * (1.000000)) + (0.000000) )
#define PHYS2BRUTE_rack_consigne_moteur(val) (short)( (val - (0.000000)) / (1.000000) )
#define BRUTE2PHYS_rack_convergence(val) ( ((float)val * (1.000000)) + (0.000000) )
#define PHYS2BRUTE_rack_convergence(val) (unsigned char)( (val - (0.000000)) / (1.000000) )
class CTrame_ETAT_RACK : public CTrameBot
{
   Q_OBJECT
public :
    CTrame_ETAT_RACK(CMessagerieBot *messagerie_bot, CDataManager *data_manager);
    ~CTrame_ETAT_RACK() { }
    /*virtual*/ void Decode(tStructTrameBrute *trameRecue);

 private :
    short rack_reserve;
    unsigned char rack_modeAsservissement;
    short rack_cde_moteur;
    short rack_consigne_moteur;
    unsigned char rack_convergence;

private slots :
};

// ========================================================
//             TRAME COLOR_SENSOR
// ========================================================
#define ID_ELECTROBOT_COLOR_SENSOR 0x21
#define DLC_ELECTROBOT_COLOR_SENSOR 6
class CTrame_COLOR_SENSOR : public CTrameBot
{
   Q_OBJECT
public :
    CTrame_COLOR_SENSOR(CMessagerieBot *messagerie_bot, CDataManager *data_manager);
    ~CTrame_COLOR_SENSOR() { }
    /*virtual*/ void Decode(tStructTrameBrute *trameRecue);

 private :
    short R;
    short G;
    short B;

};


// ========================================================
//             TRAME MBED_CMDE
// ========================================================
#define ID_MBED_CMDE 0x95
#define DLC_MBED_CMDE 8
#define BRUTE2PHYS_cde_mbed_char(val) ( ((float)val * (1.000000)) + (0.000000) )
#define PHYS2BRUTE_cde_mbed_char(val) (char)( (val - (0.000000)) / (1.000000) )
class CTrame_MBED_CMDE : public CTrameBot
{
   Q_OBJECT
public :
    CTrame_MBED_CMDE(CMessagerieBot *messagerie_bot, CDataManager *data_manager);
    ~CTrame_MBED_CMDE() { }
    /*virtual*/ void Encode(void);

 private :
    // Encode chacun des signaux de la trame
    short Valeur_01;
    short Valeur_02;
    char Valeur_03;
    char Valeur_04;
    unsigned short CodeCommande;
    bool m_synchro_tx;

private slots :
    void Valeur_mbed_cmde_01_changed(QVariant val);
    void Valeur_mbed_cmde_02_changed(QVariant val);
    void Valeur_mbed_cmde_03_changed(QVariant val);
    void Valeur_mbed_cmde_04_changed(QVariant val);
    void Code_mbed_cmde_changed(QVariant val);
    void Synchro_changed(QVariant val);
};

// ========================================================
//             TRAME MBED_ETAT
// ========================================================
#define ID_MBED_ETAT 0x96
#define DLC_MBED_ETAT 8
#define BRUTE2PHYS_code_mbed_etat(val) ( ((float)val * (1.000000)) + (0.000000) )
#define PHYS2BRUTE_code_mbed_etat(val) (unsigned short)( (val - (0.000000)) / (1.000000) )
#define BRUTE2PHYS_valeur_mbed_etat(val) ( ((float)val * (1.000000)) + (0.000000) )
#define PHYS2BRUTE_valeur_mbed_etat(val) (short)( (val - (0.000000)) / (1.000000) )
class CTrame_MBED_ETAT : public CTrameBot
{
   Q_OBJECT
public :
    CTrame_MBED_ETAT(CMessagerieBot *messagerie_bot, CDataManager *data_manager);
    ~CTrame_MBED_ETAT() { }
    /*virtual*/ void Decode(tStructTrameBrute *trameRecue);

 private :
    short Valeur_mbed_etat_01;
    short Valeur_mbed_etat_02;
    char Valeur_mbed_etat_03;
    char Valeur_mbed_etat_04;
    unsigned short Cde_mbed_etat;

private slots :
};


// ========================================================
//             TRAME ETAT_SERVO_AX
// ========================================================
#define ID_ETAT_SERVO_AX 0x97
#define DLC_ETAT_SERVO_AX 8
class CTrame_ETAT_SERVO_AX : public CTrameBot
{
   Q_OBJECT
public :
    CTrame_ETAT_SERVO_AX(CMessagerieBot *messagerie_bot, CDataManager *data_manager);
    ~CTrame_ETAT_SERVO_AX() { }
    /*virtual*/ void Decode(tStructTrameBrute *trameRecue);

 private :
    unsigned char num_servo_ax;
    unsigned short position;
    unsigned char temperature;
    unsigned short couple;
    unsigned char mouvement_en_cours;

private slots :
};

// ========================================================
//             TRAME COMMANDE_KMAR
// ========================================================
#define ID_COMMANDE_KMAR 0x55
#define DLC_COMMANDE_KMAR 5
class CTrame_COMMANDE_KMAR : public CTrameBot
{
   Q_OBJECT
public :
    CTrame_COMMANDE_KMAR(CMessagerieBot *messagerie_bot, CDataManager *data_manager);
    ~CTrame_COMMANDE_KMAR() { }
    /*virtual*/ void Encode(void);

 private :
    unsigned short value_cmd_kmar; // [0;300]
    unsigned short cmd_kmar;
    unsigned char num_kmar;
    bool m_synchro_tx;

private slots :
    void value_cmd_kmar_changed(QVariant val);
    void cmd_kmar_changed(QVariant val);
    void num_kmar_changed(QVariant val);
    void Synchro_changed(QVariant val);
};

// ========================================================
//             TRAME ETAT_KMAR_GENERAL
// ========================================================
#define ID_ETAT_KMAR_GENERAL 0x98
#define DLC_ETAT_KMAR_GENERAL 12
class CTrame_ETAT_KMAR_GENERAL : public CTrameBot
{
   Q_OBJECT
public :
    CTrame_ETAT_KMAR_GENERAL(CMessagerieBot *messagerie_bot, CDataManager *data_manager);
    ~CTrame_ETAT_KMAR_GENERAL() { }
    /*virtual*/ void Decode(tStructTrameBrute *trameRecue);

 private :
     unsigned char num_kmar;
    unsigned char status;
    unsigned char num_mouvement_en_cours;
    bool moving;
    bool axis1_moving;
    bool axis2_moving;
    bool axis3_moving;
    bool axis4_moving;
    bool object_catched;
    unsigned short axis1_position;
    unsigned short axis2_position;
    unsigned short axis3_position;
    unsigned short axis4_position;

private slots :
};


// ========================================================
//             TRAME ETAT_LIDAR
// ========================================================
#define ID_ETAT_LIDAR 0x99
#define DLC_ETAT_LIDAR 64
class CTrame_ETAT_LIDAR : public CTrameBot
{
   Q_OBJECT
public :
    CTrame_ETAT_LIDAR(CMessagerieBot *messagerie_bot, CDataManager *data_manager);
    ~CTrame_ETAT_LIDAR() { }
    /*virtual*/ void Encode(void);

    unsigned char m_status;         // voir enum "eLidarStatus"
    LidarUtils::tLidarObstacles m_obstacles;  // tableaux

    bool m_synchro_tx;

private slots :
    void Synchro_changed(QVariant val);
};

// ========================================================
//             TRAME FREE_STRING
// ========================================================
#define ID_FREE_STRING 0x10A
#define DLC_FREE_STRING 64
class CTrame_FREE_STRING : public CTrameBot
{
   Q_OBJECT
public :
    CTrame_FREE_STRING(CMessagerieBot *messagerie_bot, CDataManager *data_manager);
    ~CTrame_FREE_STRING() { }
    /*virtual*/ void Decode(tStructTrameBrute *trameRecue);

    char m_str[64];
private :

private slots :
};

// ========================================================
//             TRAME ETAT_CHARGE_CPU
// ========================================================
#define ID_ETAT_CHARGE_CPU 0x10B
#define DLC_ETAT_CHARGE_CPU 8
class CTrame_ETAT_CHARGE_CPU : public CTrameBot
{
   Q_OBJECT
public :
    CTrame_ETAT_CHARGE_CPU(CMessagerieBot *messagerie_bot, CDataManager *data_manager);
    ~CTrame_ETAT_CHARGE_CPU() { }
    /*virtual*/ void Decode(tStructTrameBrute *trameRecue);

    unsigned long long cpu_overload_counter;     // un compteur qui indique la derive de la charge CPU
    unsigned long long task_real_period_usec;        // le delta T entre les 2 derniers appels de la tache surveillee
};


// ========================================================
//             TRAME RESET_CPU
// ========================================================
#define ID_RESET_CPU 0x10C
#define DLC_RESET_CPU 1
class CTrame_RESET_CPU : public CTrameBot
{
   Q_OBJECT
public :
    CTrame_RESET_CPU(CMessagerieBot *messagerie_bot, CDataManager *data_manager);
    ~CTrame_RESET_CPU() { }
    /*virtual*/ void Encode(void);

    unsigned char SECURITE_RESET_CPU;         // voir enum "eLidarStatus"

    bool m_synchro_tx;

private slots :
    void Synchro_changed(QVariant val);
    void SECURITE_RESET_CPU_changed(QVariant val);

};


#endif // _CTRAME_FACTORY_H_

/*! @} */


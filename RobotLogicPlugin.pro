#-------------------------------------------------
# RobotLogicPlugin.pro  --  POC hot-reload Simulia
#
# Compile TOUTE la logique robot (Modelia + CppRobLib + classes _simu + CGlobaleSimule)
# dans une bibliotheque dynamique horodatee : librobotlogic_AAAAMMJJ_HHMMSS.so
#
# Chargee a chaud par le shell Simulia (CSimulia) via QLibrary en RTLD_LOCAL.
# Le shell ne referme QUE createRobotLogic()/destroyRobotLogic() (ABI C, cf. IRobotLogic.h).
#
# IMPORTANT : ce .pro ne touche PAS Soft_STM32 (chaine de build/flash STM32 preservee).
#-------------------------------------------------

QT       += core gui xml serialport network
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

# Meme define que Simulia : build AVEC la vraie logique robot (cf. Simulia.pro).
DEFINES += SIMUBOT_ROBOT_LOGIC
DEFINES += MESSENGER_FULL

# CRITIQUE : le plugin recoit un CApplication* construit par le shell. Le layout de CApplication
# depend des MODULE_* (champs m_data_center, m_eeprom... sous #ifdef). Le plugin DOIT compiler
# CApplication.h avec EXACTEMENT la meme liste de MODULE_* que Simulia.pro, sinon les offsets
# des membres divergent -> acces memoire errone au runtime. Liste identique a celle generee
# par Simulia.pro (LIST_BASIC_MODULES + LIST_PLUGIN_MODULES + LIST_TOOLS).
DEFINES += MODULE_DataManager MODULE_MainWindow MODULE_PrintView MODULE_EEPROM \
           MODULE_DataView MODULE_DataGraph MODULE_DataPlayer MODULE_csvDataLogger \
           MODULE_Joystick MODULE_ModuleDesigner MODULE_UserGuides MODULE_ExternalControler \
           MODULE_SimuBot MODULE_ActuatorSequencer MODULE_BlockBotLab MODULE_Simulia \
           MODULE_CustomPlot MODULE_HtmlTextEditor MODULE_DocDesigner MODULE_NetworkServer \
           MODULE_ExternalControlerClient MODULE_DataHandler MODULE_CsvParser

TEMPLATE = lib
CONFIG  += plugin
CONFIG  += c++11
CONFIG  -= app_bundle

# Nom horodate : chaque build produit une lib distincte -> selection/chargement par date.
TARGET = robotlogic_$$system(date +%Y%m%d_%H%M%S)

# Repertoire de depot de la lib generee, lu dans l'environnement AU MOMENT DU QMAKE.
# Positionne par tools/build_robot_logic.sh, lui-meme alimente par la clef EEPROM
# [Simulia] robot_logic_lib_path quand le build est lance depuis BlockBotLab : la lib atterrit
# ainsi directement dans le repertoire scanne par Simulia (aucune copie manuelle).
# Variable absente/vide -> depot dans le repertoire de build (comportement historique du POC).
ROBOT_LOGIC_DEST_ENV = $$(ROBOT_LOGIC_DEST)
!isEmpty(ROBOT_LOGIC_DEST_ENV) {
    DESTDIR = $$ROBOT_LOGIC_DEST_ENV
}

# __________________________________________________
# Chemins (identiques a Simulia.pro, ce .pro vit dans le meme repertoire Simulia/)
PATH_SOFT_CPU =         ../Soft_STM32
PATH_CPPROBLIB =        $$PATH_SOFT_CPU/ext/CppRobLib
PATH_COMMON_ROB =       $$PATH_CPPROBLIB/common-rob
PATH_CPU_COMMON_ROB =   $$PATH_SOFT_CPU/ext/common-rob
PATH_MODELIA_COMMON =   $$PATH_COMMON_ROB/Modelia
PATH_MODELIA_ROBOT =    $$PATH_SOFT_CPU/CM7/Modelia

# __________________________________________________
# Modules CppRobLib (meme liste que Simulia.pro)
LIST_EXT_CPPROBLIB += \
        ServosAX \
        PowerElectrobot \
        mcp23017 \
        Lidar \
        Servomoteurs \
        common-rob \
        Communication/Messenger \
        Communication/Messenger/MessagesGeneric \
        Communication/Messenger/DatabaseXbeeNetwork2019 \

INCLUDEPATH += ./ext/CppRobLib
for(i, LIST_EXT_CPPROBLIB) {
    INCLUDEPATH += $$_PRO_FILE_PWD_/../Soft_STM32/ext/CppRobLib/$${i}
    SOURCES     += $$files($$_PRO_FILE_PWD_/../Soft_STM32/ext/CppRobLib/$${i}/*.cpp)
    HEADERS     += $$files($$_PRO_FILE_PWD_/../Soft_STM32/ext/CppRobLib/$${i}/*.h)
}

# __________________________________________________
# Modelia (strategie + machines a etats) + includes robot
# IMPORTANT : ./PluginModules/Simulia AVANT CM7/Includes pour que #include "CGlobale.h"
# resolve vers le CGlobaleSimule de Simulia et NON vers le CGlobale du firmware
# (Soft_STM32/CM7/Includes/CGlobale.h, qui tire RessourcesHardware.h -> main.h STM32).
# Meme ordre que Simulia.pro (les PluginModules sont ajoutes avant les paths Modelia).
INCLUDEPATH += ./PluginModules/Simulia \
               ./PluginModules/Simulia/simu_moteurs \
               $$PATH_MODELIA_COMMON \
               $$PATH_MODELIA_ROBOT \
               $$PATH_SOFT_CPU/CM7/Includes \
               $$PATH_CPU_COMMON_ROB/Includes \

# Classes generiques LABOTBOX referencees par les _simu (CApplication, CDataManager, CData...).
# Seuls headers necessaires : le shell fournit les definitions a l'execution (le plugin ne
# reimplemente pas ces classes -> resolues dans le binaire Simulia via l'objet CApplication passe).
INCLUDEPATH += $$_PRO_FILE_PWD_ \
               $$_PRO_FILE_PWD_/BasicModules \
               $$_PRO_FILE_PWD_/BasicModules/DataManager \
               $$_PRO_FILE_PWD_/BasicModules/EEPROM \

SOURCES += $$PATH_MODELIA_COMMON/*.cpp \
           $$PATH_MODELIA_ROBOT/*.cpp \
           ./PluginModules/Simulia/simu_moteurs/plateformer_robot.cpp \

HEADERS += $$PATH_MODELIA_COMMON/*.h \
           $$PATH_MODELIA_ROBOT/*.h \

# __________________________________________________
# Classes de simulation (_simu) + agregat CGlobaleSimule + wrapper interface.
# NB : CSimulia.cpp N'EST PAS ici (il reste dans le shell Simulia.pro).
SOURCES += \
    PluginModules/Simulia/CAscenseur_simu.cpp \
    PluginModules/Simulia/CAsservissementChariot_simu.cpp \
    PluginModules/Simulia/CAsservissement_simu.cpp \
    PluginModules/Simulia/CCapteurs_simu.cpp \
    PluginModules/Simulia/CDetectionObstacles_simu.cpp \
    PluginModules/Simulia/CGlobale.cpp \
    PluginModules/Simulia/CLed_simu.cpp \
    PluginModules/Simulia/CLidar_simu.cpp \
    PluginModules/Simulia/CRoues_simu.cpp \
    PluginModules/Simulia/CServoMoteurAX_simu.cpp \
    PluginModules/Simulia/CServoMoteurSD20_simu.cpp \
    PluginModules/Simulia/CServomoteurs_simu.cpp \
    PluginModules/Simulia/CTelemetres_simu.cpp \
    PluginModules/Simulia/MessengerXbeeNetwork_simu.cpp \
    PluginModules/Simulia/powerelectrobotsimu.cpp \
    PluginModules/Simulia/sm_debugqdebug.cpp \
    PluginModules/Simulia/CRobotLogic.cpp \
    PluginModules/Simulia/CRobotLogic_entry.cpp \

HEADERS += \
    PluginModules/Simulia/CGlobale.h \
    PluginModules/Simulia/IRobotLogic.h \
    PluginModules/Simulia/CRobotLogic.h \
    PluginModules/Simulia/CAscenseur_simu.h \
    PluginModules/Simulia/CAsservissementChariot_simu.h \
    PluginModules/Simulia/CAsservissement_simu.h \
    PluginModules/Simulia/CCapteurs_simu.h \
    PluginModules/Simulia/CDetectionObstacles_simu.h \
    PluginModules/Simulia/CLed_simu.h \
    PluginModules/Simulia/CLidar_simu.h \
    PluginModules/Simulia/CRoues_simu.h \
    PluginModules/Simulia/CServoMoteurAX_simu.h \
    PluginModules/Simulia/CServoMoteurSD20_simu.h \
    PluginModules/Simulia/CServomoteurs_simu.h \
    PluginModules/Simulia/CTelemetres_simu.h \
    PluginModules/Simulia/MessengerXbeeNetwork_simu.h \
    PluginModules/Simulia/powerelectrobotsimu.h \
    PluginModules/Simulia/sm_debugqdebug.h \

# NB : CLidar_simu.h est un QObject (Q_OBJECT) -> DOIT etre dans HEADERS pour que moc genere son
# metaobjet + vtable (sinon "vtable for CLidarSimu" indefini -> crash a la construction du plugin).

# __________________________________________________
# Le lidar simule (CLidarSimu) est un QObject -> moc necessaire (fourni par CONFIG plugin + QT widgets)

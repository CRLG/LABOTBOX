#-------------------------------------------------
#
# Project created by QtCreator 2014-07-21T12:19:41
#
#-------------------------------------------------
QT       += core gui testlib xml printsupport serialport network websockets multimedia
# webenginewidgets + webchannel : requis par le plugin BlockBotLab (pont Qt <-> BlockBot/Blockly),
# ajoute a Simulia pour pouvoir tester le HIL directement dans le simulateur (etape 6 migration v2).
QT       += webenginewidgets webchannel

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

# POC hot-reload : SIMUBOT_ROBOT_LOGIC RETIRE du shell.
# Ce define faisait utiliser a CSimuBot le VRAI CAsservissementBase (classe robot-logic) pour
# reconstruire la pose "l'asserv fait foi". Or CAsservissementBase.cpp utilise le global Application
# et part desormais dans le plugin (librobotlogic_*.so) -> le shell ne peut plus le lier directement.
# Sans ce define, CSimuBot bascule sur CPoseReconstructeurStandalone (copie autonome de CalculXY,
# chemin LaBotBox), sans dependance a la logique robot. C'est le seul couplage shell->robot HORS
# IRobotLogic ; le decoupler proprement (via une methode IRobotLogic) serait une evolution ulterieure.
#DEFINES += SIMUBOT_ROBOT_LOGIC

TARGET = Simulia
TEMPLATE = app

SOURCES +=  main.cpp\
            CApplication.cpp


HEADERS  += CApplication.h \
            CModule.h

RESOURCES+= icons.qrc \
            code_template.qrc
# __________________________________________________
# Ajouter ici les basic modules (nom des répertoires)
LIST_BASIC_MODULES+= \
        DataManager \
        MainWindow \
        PrintView \
        EEPROM \
        DataView \ 
        DataGraph \
        DataPlayer \
        csvDataLogger \
        #RS232 \
        Joystick \
        ModuleDesigner \
        UserGuides \
        ExternalControler \ 
        # ##_NEW_BASIC_MODULE_NAME_HERE_##

# __________________________________________________
# Ajouter ici les plugin modules (nom des répertoires)
LIST_PLUGIN_MODULES+= \
        SimuBot \
        ActuatorSequencer \
        BlockBotLab \
        # POC hot-reload : Simulia RETIRE du glob automatique. La logique robot (_simu, CGlobale,
        # Modelia, CppRobLib) part dans RobotLogicPlugin.pro (librobotlogic_*.so). Seul CSimulia.cpp
        # reste dans le shell et est ajoute explicitement plus bas (section "POC hot-reload").
        # ##_NEW_PLUGIN_MODULE_NAME_HERE_##

# __________________________________________________
# Ajouter ici les utilitaires communs "Tools" (nom des répertoires)
LIST_TOOLS+= CustomPlot\
             HtmlTextEditor \
             DocDesigner \
             NetworkServer \
             ExternalControlerClient \
             DataHandler \
             CsvParser \

# __________________________________________________
# Ajouter ici les modules externes CppRobLib
LIST_EXT_CPPROBLIB+= \
        ServosAX \
        PowerElectrobot \
        mcp23017 \
        Lidar \
        Servomoteurs \
        common-rob \
        Communication/Messenger \
        Communication/Messenger/MessagesGeneric \
        Communication/Messenger/DatabaseXbeeNetwork2019 \

# __________________________________________________
# Gestion des basic modules
INCLUDEPATH +=  ./BasicModules
HEADERS += $$_PRO_FILE_PWD_/BasicModules/*.h

for(i, LIST_BASIC_MODULES) {
    DEFINES+= MODULE_$${i}
    INCLUDEPATH+= $$_PRO_FILE_PWD_/BasicModules/$${i}
    SOURCES+= $$files($$_PRO_FILE_PWD_/BasicModules/$${i}/*.cpp)
    HEADERS+= $$files($$_PRO_FILE_PWD_/BasicModules/$${i}/*.h)
    FORMS+= $$files($$_PRO_FILE_PWD_/BasicModules/$${i}/*.ui)
    RESOURCES+= $$files($$_PRO_FILE_PWD_/BasicModules/$${i}/*.qrc)
}
# __________________________________________________
# Gestion des plugin modules
INCLUDEPATH +=  ./PluginModules
HEADERS += $$_PRO_FILE_PWD_/PluginModules/*.h

for(i, LIST_PLUGIN_MODULES) {
    DEFINES+= MODULE_$${i}
    INCLUDEPATH+= $$_PRO_FILE_PWD_/PluginModules/$${i}
    SOURCES+= $$files($$_PRO_FILE_PWD_/PluginModules/$${i}/*.cpp)
    HEADERS+= $$files($$_PRO_FILE_PWD_/PluginModules/$${i}/*.h)
    FORMS+= $$files($$_PRO_FILE_PWD_/PluginModules/$${i}/*.ui)
    RESOURCES+= $$files($$_PRO_FILE_PWD_/PluginModules/$${i}/*.qrc)
}
# __________________________________________________
# Gestion des utilitaires "Tools"
INCLUDEPATH +=  ./Tools
HEADERS += $$_PRO_FILE_PWD_/Tools/*.h

for(i, LIST_TOOLS) {
    DEFINES+= MODULE_$${i}
    INCLUDEPATH+= $$_PRO_FILE_PWD_/Tools/$${i}
    SOURCES+= $$files($$_PRO_FILE_PWD_/Tools/$${i}/*.cpp)
    HEADERS+= $$files($$_PRO_FILE_PWD_/Tools/$${i}/*.h)
    FORMS+= $$files($$_PRO_FILE_PWD_/Tools/$${i}/*.ui)
    RESOURCES+= $$files($$_PRO_FILE_PWD_/Tools/$${i}/*.qrc)
}
# __________________________________________________
# Gestion des modules CppRobLib
INCLUDEPATH +=  ./ext/CppRobLib

for(i, LIST_EXT_CPPROBLIB) {
    INCLUDEPATH+= $$_PRO_FILE_PWD_/../Soft_STM32/ext/CppRobLib/$${i}
    # POC hot-reload : les .cpp CppRobLib partent dans RobotLogicPlugin.pro. Le shell garde
    # UNIQUEMENT les INCLUDEPATH/HEADERS (pour que CSimulia compile contre les declarations).
    #SOURCES+= $$files($$_PRO_FILE_PWD_/../Soft_STM32/ext/CppRobLib/$${i}/*.cpp)
    HEADERS+= $$files($$_PRO_FILE_PWD_/../Soft_STM32/ext/CppRobLib/$${i}/*.h)
    FORMS+= $$files($$_PRO_FILE_PWD_/../Soft_STM32/ext/CppRobLib/$${i}/*.ui)
    RESOURCES+= $$files($$_PRO_FILE_PWD_/../Soft_STM32/ext/CppRobLib/$${i}/*.qrc)
}

# __________________________________________________
# box2d retire en etape 3 de la migration SimuBot v2 (cf. rapport_simubot.md) :
# la simulation utilise desormais CKinematicEngine (cinematique analytique) et
# les collisions geometriques maison (CCollisionEngine). Plus aucun symbole Box2D
# ne doit subsister dans le binaire.

# __________________________________________________
# Spécifiquement pour Simulia
PATH_SOFT_CPU =         ../Soft_STM32
PATH_CPPROBLIB =        $$PATH_SOFT_CPU/ext/CppRobLib
PATH_COMMON_ROB =       $$PATH_CPPROBLIB/common-rob
PATH_CPU_COMMON_ROB =   $$PATH_SOFT_CPU/ext/common-rob
PATH_MODELIA_COMMON =   $$PATH_COMMON_ROB/Modelia
PATH_MODELIA_ROBOT =    $$PATH_SOFT_CPU/CM7/Modelia
DEFINES+= MESSENGER_FULL
# POC hot-reload : ./PluginModules/Simulia AVANT CM7/Includes (resolution de CGlobale.h vers
# le CGlobaleSimule de Simulia si un fichier shell l'inclut ; cf. meme regle dans RobotLogicPlugin.pro).
INCLUDEPATH +=  ./PluginModules/Simulia \
                $$PATH_MODELIA_COMMON \
                $$PATH_MODELIA_ROBOT \
                $$PATH_SOFT_CPU/CM7/Includes \
                $$PATH_CPU_COMMON_ROB/Includes \
                ./PluginModules/Simulia/simu_moteurs \

# POC hot-reload : les .cpp Modelia + plateformer_robot partent dans RobotLogicPlugin.pro.
# Le shell garde les INCLUDEPATH/HEADERS pour compiler CSimulia contre les declarations
# (structs d'interface, enums _simu) sans lier les definitions (fournies par le plugin/shell).
#SOURCES +=      $$PATH_MODELIA_COMMON/*.cpp \
#                $$PATH_MODELIA_ROBOT/*.cpp \
#                ./PluginModules/Simulia/simu_moteurs/plateformer_robot.cpp \

HEADERS +=      $$PATH_MODELIA_COMMON/*.h \
                $$PATH_MODELIA_ROBOT/*.h \

CONFIG += plugins_designer

# __________________________________________________
# POC hot-reload : module Simulia gere explicitement (retire du glob LIST_PLUGIN_MODULES).
# Seul le shell CSimulia reste dans l'application ; il dialogue avec la logique robot
# via l'interface IRobotLogic (plugin librobotlogic_*.so charge a chaud).
DEFINES     += MODULE_Simulia
INCLUDEPATH += $$_PRO_FILE_PWD_/PluginModules/Simulia
SOURCES     += PluginModules/Simulia/CSimulia.cpp
HEADERS     += PluginModules/Simulia/CSimulia.h \
               PluginModules/Simulia/IRobotLogic.h
FORMS       += PluginModules/Simulia/ihm_Simulia.ui
RESOURCES   += PluginModules/Simulia/Simulia.qrc

# -rdynamic : exporte les symboles de l'executable Simulia (CDataManager::write, CApplication...)
# pour que le plugin librobotlogic_*.so (charge en RTLD_LOCAL) resolve ses appels VERS le shell.
# Direction stable (le shell ne se recharge pas) -> pas le probleme d'interposition du spike.
QMAKE_LFLAGS += -rdynamic

# __________________________________________________
# Placé en premier, link en priorité avec les librairies contenues dans le répertoire d'installation de Qt (cas de plusieurs versions de librairies Qt installées sur la même machine dans des répertoires différents)
LIBS += -L$$[QT_INSTALL_LIBS]
# __________________________________________________
# Gestion du joystick
LIBS += -lsfml-graphics -lsfml-window -lsfml-system
# __________________________________________________

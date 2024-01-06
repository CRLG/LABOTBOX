#-------------------------------------------------
#
# Project created by QtCreator 2014-07-21T12:19:41
#
#-------------------------------------------------
QT       += core gui testlib xml printsupport serialport network websockets multimedia

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

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
        Simulia\
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
    INCLUDEPATH+= $$_PRO_FILE_PWD_/../Soft_MBED/ext/CppRobLib/$${i}
    SOURCES+= $$files($$_PRO_FILE_PWD_/../Soft_MBED/ext/CppRobLib/$${i}/*.cpp)
    HEADERS+= $$files($$_PRO_FILE_PWD_/../Soft_MBED/ext/CppRobLib/$${i}/*.h)
    FORMS+= $$files($$_PRO_FILE_PWD_/../Soft_MBED/ext/CppRobLib/$${i}/*.ui)
    RESOURCES+= $$files($$_PRO_FILE_PWD_/../Soft_MBED/ext/CppRobLib/$${i}/*.qrc)
}

# __________________________________________________
# box2d
INCLUDEPATH+= $$_PRO_FILE_PWD_/ext/box2d/include
linux {
    contains(QMAKE_HOST.arch, arm.*):{
        # Raspberry
        LIBS += -L$$_PRO_FILE_PWD_/ext/box2d/libs/arm64
    }else{
        # x86-64
        LIBS += -L$$_PRO_FILE_PWD_/ext/box2d/libs/ubuntu_x86_64
    }
}
LIBS += -lbox2d

# __________________________________________________
# Spécifiquement pour Simulia
PATH_SOFT_MBED =        ../Soft_MBED
PATH_CPPROBLIB =        $$PATH_SOFT_MBED/ext/CppRobLib
PATH_COMMON_ROB =       $$PATH_CPPROBLIB/common-rob
PATH_MBED_COMMON_ROB =  $$PATH_SOFT_MBED/ext/mbed-common-rob
PATH_MODELIA_COMMON =   $$PATH_COMMON_ROB/Modelia
PATH_MODELIA_ROBOT =    $$PATH_SOFT_MBED/Modelia
DEFINES+= MESSENGER_FULL
INCLUDEPATH +=  $$PATH_MODELIA_COMMON \
                $$PATH_MODELIA_ROBOT \
                $$PATH_SOFT_MBED/Includes \
                $$PATH_MBED_COMMON_ROB/Includes \
                ./PluginModules/Simulia/simu_moteurs \

SOURCES +=      $$PATH_MODELIA_COMMON/*.cpp \
                $$PATH_MODELIA_ROBOT/*.cpp \
                ./PluginModules/Simulia/simu_moteurs/plateformer_robot.cpp \

HEADERS +=      $$PATH_MODELIA_COMMON/*.h \
                $$PATH_MODELIA_ROBOT/*.h \

CONFIG += plugins_designer

# __________________________________________________
# Placé en premier, link en priorité avec les librairies contenues dans le répertoire d'installation de Qt (cas de plusieurs versions de librairies Qt installées sur la même machine dans des répertoires différents)
LIBS += -L$$[QT_INSTALL_LIBS]
# __________________________________________________
# Gestion du joystick
LIBS += -lsfml-graphics -lsfml-window -lsfml-system
# __________________________________________________

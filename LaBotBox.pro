#-------------------------------------------------
#
# Project created by QtCreator 2014-07-21T12:19:41
#
#-------------------------------------------------
QT       += core gui testlib xml printsupport serialport network websockets multimedia

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = LaBotBox
TEMPLATE = app

#CONFIG += RASPBERRY_PI

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
        RS232 \ 
        Joystick \
        ModuleDesigner \
        UserGuides \
        ExternalControler \ 
        # ##_NEW_BASIC_MODULE_NAME_HERE_##

# __________________________________________________
# Ajouter ici les plugin modules (nom des répertoires)
LIST_PLUGIN_MODULES+= \
        TestUnitaire \ 
        SimuBot \ 
        StrategyDesigner \ 
        MessagerieBot \ 
        SensorElectroBot \ 
        ActuatorElectrobot \ 
        SensorView \ 
        Asserv \ 
        ActuatorSequencer \
        #BotCam \
        #Ecran \
        ImageProcessing \ 
        XBEE \ 
        XbeeNetworkMessenger \ 
        #Balise \ 
        PowerElectrobot \ 
        # ##_NEW_PLUGIN_MODULE_NAME_HERE_##

# __________________________________________________
# Ajouter ici les utilitaires communs "Tools" (nom des répertoires)
LIST_TOOLS+= CustomPlot\
             HtmlTextEditor \
             DocDesigner \
             NetworkServer \
             ExternalControlerClient \
             DataLogger \
             CsvParser

# __________________________________________________
# Ajouter ici les modules externes CppRobLib
LIST_EXT_CPPROBLIB+= \
        ServosAX \
        Communication/Messenger \
        Communication/Messenger/MessagesGeneric \
        Communication/Messenger/DatabaseXbeeNetwork2019 \
        Communication/XBEE \

DEFINES+= MESSENGER_FULL
# __________________________________________________
# Ajouter ici les modules spécifiques à RaspberyPi
RASPBERRY_PI {
    #LIST_PLUGIN_MODULES+= RaspiServoAX
    LIST_BASIC_MODULES+=  RaspiGPIO
    DEFINES += RASPBERRY_PI
    LIBS += -L/usr/lib -lwiringPi
}
		
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
    INCLUDEPATH+= $$_PRO_FILE_PWD_/ext/CppRobLib/$${i}
    SOURCES+= $$files($$_PRO_FILE_PWD_/ext/CppRobLib/$${i}/*.cpp)
    HEADERS+= $$files($$_PRO_FILE_PWD_/ext/CppRobLib/$${i}/*.h)
    FORMS+= $$files($$_PRO_FILE_PWD_/ext/CppRobLib/$${i}/*.ui)
    RESOURCES+= $$files($$_PRO_FILE_PWD_/ext/CppRobLib/$${i}/*.qrc)
}

CONFIG += plugins_designer

# __________________________________________________
# Placé en premier, link en priorité avec les librairies contenues dans le répertoire d'installation de Qt (cas de plusieurs versions de librairies Qt installées sur la même machine dans des répertoires différents)
LIBS += -L$$[QT_INSTALL_LIBS]
# __________________________________________________
# Gestion du joystick
LIBS += -lsfml-graphics -lsfml-window -lsfml-system
# __________________________________________________
# Gestion des webcam et traitements video
win32 {
    LIBS += -LC:/win_progs/Qt/5.2.1/mingw48_32/plugins/designer -lqhexeditplugin -lqledplugin -lq7segplugin
    LIBS += -LC:/win_progs/OpenCV/opencv_2_4_9/bin -llibopencv_core249 -llibopencv_highgui249 -llibopencv_imgproc249
    INCLUDEPATH += "C:/win_progs/OpenCV/opencv_2_4_9/include"
}

linux {
    LIBS +=`pkg-config opencv --libs`  # Ligne de commande qui renvoie toutes les librairies opencv sous la forme "-lopencv... -lopencv..."
}

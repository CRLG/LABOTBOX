#-------------------------------------------------
#
# Project created by QtCreator 2014-07-21T12:19:41
#
#-------------------------------------------------
QT       += core gui testlib xml printsupport serialport network websockets

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

RASPBERRY_PI {
    LIST_BASIC_MODULES+=RaspiGPIO
    DEFINES += RASPBERRY_PI
    LIBS += -L/usr/lib -lwiringPi
}

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
        BotCam \
        Ecran \
        # ##_NEW_PLUGIN_MODULE_NAME_HERE_##

# __________________________________________________
# Ajouter ici les utilitaires communs "Tools" (nom des répertoires)
LIST_TOOLS+= CustomPlot\
             HtmlTextEditor \
             NetworkServer

		
# __________________________________________________
# Gestion des basic modules
INCLUDEPATH +=  ./BasicModules
HEADERS += $$_PRO_FILE_PWD_/BasicModules/*.h

for(i, LIST_BASIC_MODULES) {
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
    INCLUDEPATH+= $$_PRO_FILE_PWD_/Tools/$${i}
    SOURCES+= $$files($$_PRO_FILE_PWD_/Tools/$${i}/*.cpp)
    HEADERS+= $$files($$_PRO_FILE_PWD_/Tools/$${i}/*.h)
    FORMS+= $$files($$_PRO_FILE_PWD_/Tools/$${i}/*.ui)
    RESOURCES+= $$files($$_PRO_FILE_PWD_/Tools/$${i}/*.qrc)
}

CONFIG += plugins_designer

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
    LIBS += -L/usr/lib/x86_64-linux-gnu -lopencv_core -lopencv_highgui -lopencv_imgproc
    INCLUDEPATH += /usr/include/opencv2
}


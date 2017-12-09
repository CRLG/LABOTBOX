#-------------------------------------------------
#
# Project created by QtCreator 2014-07-21T12:19:41
#
#-------------------------------------------------
QT       += core gui testlib xml printsupport serialport multimedia multimediawidgets

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

LIBS += -LD:/win_progs/Qt_5_2_1/5.2.1/mingw48_32/plugins/designer -lqhexeditplugin -lqledplugin -lq7segplugin
LIBS += -LD:/win_progs/OpenCV/opencv_2_4_9/bin -llibopencv_core249 -llibopencv_highgui249 -llibopencv_imgproc249
INCLUDEPATH += "D:/win_progs/OpenCV/opencv_2_4_9/include"
#LIBS += -LD:/win_progs/SFML-2.3.2/bin -lsfml-graphics-2 -lsfml-window-2 -lsfml-system-2
#INCLUDEPATH += "D:/win_progs/SFML-2.3.2/include"
TARGET = LaBotBox
TEMPLATE = app

SOURCES +=  main.cpp\
            CLaBotBox.cpp


HEADERS  += CLaBotBox.h \
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
        DataPlayer \ 
        RS232 \ 
        Ecran \
#        Joystick \ 
        # ##_NEW_BASIC_MODULE_NAME_HERE_##

# __________________________________________________
# Ajouter ici les plugin modules (nom des répertoires)
LIST_PLUGIN_MODULES+= \
        ModuleDesigner \
        TestUnitaire \ 
        DataGraph \ 
        SimuBot \ 
        StrategyDesigner \ 
        MessagerieBot \ 
        SensorElectroBot \ 
        ActuatorElectrobot \ 
        SensorView \ 
        Asserv \ 
        UserGuides \ 
        ActuatorSequencer \
        BotCam \
        # ##_NEW_PLUGIN_MODULE_NAME_HERE_##

# __________________________________________________
# Ajouter ici les utilitaires communs "Tools" (nom des répertoires)
LIST_TOOLS+= CustomPlot\
             HtmlTextEditor

		
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
#LIBS += -lsfml-graphics -lsfml-window -lsfml-system

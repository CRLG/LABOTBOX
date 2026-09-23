#-------------------------------------------------
# banc_logique_robot : banc de test de la logique robot compilee pour Simulia
# (librobotlogic_*.so), charge hors de Simulia. Voir banc.h.
#
#   mkdir -p build_banc && cd build_banc && qmake ../Simulia/tools/banc_logique_robot && make
#   ./banc_logique_robot <chemin/librobotlogic_xxx.so>
#-------------------------------------------------
QT      += core
QT      -= gui
CONFIG  += console c++11
CONFIG  -= app_bundle
TEMPLATE = app
TARGET   = banc_logique_robot

# Le plugin resout CDataManager/CData dans l'executable : symboles exportes.
QMAKE_LFLAGS += -rdynamic
LIBS         += -ldl

# CRITIQUE : meme liste de MODULE_* que Simulia.pro et RobotLogicPlugin.pro. La position de
# m_data_center dans CApplication en depend (membres sous #ifdef MODULE_xxx).
DEFINES += SIMUBOT_ROBOT_LOGIC MESSENGER_FULL
DEFINES += MODULE_DataManager MODULE_MainWindow MODULE_PrintView MODULE_EEPROM \
           MODULE_DataView MODULE_DataGraph MODULE_DataPlayer MODULE_csvDataLogger \
           MODULE_Joystick MODULE_ModuleDesigner MODULE_UserGuides MODULE_ExternalControler \
           MODULE_SimuBot MODULE_ActuatorSequencer MODULE_BlockBotLab MODULE_Simulia \
           MODULE_CustomPlot MODULE_HtmlTextEditor MODULE_DocDesigner MODULE_NetworkServer \
           MODULE_ExternalControlerClient MODULE_DataHandler MODULE_CsvParser

SIMULIA  = $$_PRO_FILE_PWD_/../..
SOFT_CPU = $$SIMULIA/../Soft_STM32

# Ordre identique au plugin : PluginModules/Simulia AVANT CM7/Includes (CGlobale.h simule)
INCLUDEPATH += $$SIMULIA \
               $$SIMULIA/PluginModules \
               $$SIMULIA/BasicModules/DataManager \
               $$SIMULIA/PluginModules/Simulia \
               $$SOFT_CPU/ext/CppRobLib/common-rob/Modelia \
               $$SOFT_CPU/CM7/Modelia \
               $$SOFT_CPU/CM7/Includes \
               $$SOFT_CPU/ext/CppRobLib/common-rob \
               $$SOFT_CPU/ext/CppRobLib/Lidar

SOURCES += main.cpp banc.cpp banc_data_manager.cpp scenarios_etape0.cpp scenarios_etape1.cpp scenarios_etape2.cpp scenarios_etape3.cpp \
           $$SIMULIA/BasicModules/DataManager/CData.cpp
HEADERS += banc.h banc_data_manager.h scenarios.h \
           $$SIMULIA/BasicModules/DataManager/CData.h

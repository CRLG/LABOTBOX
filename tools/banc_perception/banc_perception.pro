#-------------------------------------------------
# banc_filtre_lidar : banc de test du filtre lidar « tracker » de CppRobLib, sur des balayages
# synthetiques. Aucune dependance : ni Qt, ni Simulia, ni plugin.
#
# Deliberement SEPARE de banc_logique_robot : ce dernier charge le plugin, qui contient deja une
# copie du filtre. Compiler le filtre dans le meme executable ferait primer la copie de
# l'executable sur celle du plugin (interposition de symboles) et masquerait silencieusement un
# ecart entre les deux.
#
#   mkdir -p build_banc_filtre && cd build_banc_filtre
#   qmake ../Simulia/tools/banc_filtre_lidar && make && ./banc_filtre_lidar
#-------------------------------------------------
QT      -= core gui
CONFIG  += console c++11
CONFIG  -= app_bundle qt
TEMPLATE = app
TARGET   = banc_filtre_lidar

LIDAR = $$_PRO_FILE_PWD_/../../../Soft_STM32/ext/CppRobLib/Lidar
INCLUDEPATH += $$LIDAR
SOURCES += main.cpp \
           $$LIDAR/lidar_data.cpp \
           $$LIDAR/Lidar_utils.cpp \
           $$LIDAR/lidar_data_filter_tracker.cpp
HEADERS += $$LIDAR/lidar_data.h $$LIDAR/lidar_blob.h $$LIDAR/lidar_data_filter_tracker.h

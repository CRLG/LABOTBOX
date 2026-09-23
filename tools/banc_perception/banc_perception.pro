#-------------------------------------------------
# banc_perception : banc de test des couches 1 (filtre « tracker ») et 2 (suivi temporel des
# objets) de l'evitement, sur donnees synthetiques. Aucune dependance : ni Qt, ni Simulia, ni plugin.
#
# Deliberement SEPARE de banc_logique_robot : ce dernier charge le plugin, qui contient deja une
# copie du filtre. Compiler le filtre dans le meme executable ferait primer la copie de
# l'executable sur celle du plugin (interposition de symboles) et masquerait silencieusement un
# ecart entre les deux.
#
#   mkdir -p build_banc_perception && cd build_banc_perception
#   qmake ../Simulia/tools/banc_perception && make && ./banc_perception
#-------------------------------------------------
QT      -= core gui
CONFIG  += console c++11
CONFIG  -= app_bundle qt
TEMPLATE = app
TARGET   = banc_perception

LIDAR = $$_PRO_FILE_PWD_/../../../Soft_STM32/ext/CppRobLib/Lidar
COMMONROB = $$_PRO_FILE_PWD_/../../../Soft_STM32/ext/CppRobLib/common-rob
INCLUDEPATH += $$LIDAR $$COMMONROB
SOURCES += main.cpp tests_filtre.cpp tests_suivi.cpp tests_tactique.cpp \
           $$LIDAR/lidar_data.cpp \
           $$LIDAR/Lidar_utils.cpp \
           $$LIDAR/lidar_data_filter_tracker.cpp \
           $$LIDAR/CObstacleTracker.cpp \
           $$COMMONROB/CTacticalEvaluator.cpp
HEADERS += outils_banc.h \
           $$LIDAR/lidar_data.h $$LIDAR/lidar_blob.h $$LIDAR/lidar_data_filter_tracker.h \
           $$LIDAR/CObstacleTracker.h $$COMMONROB/CTacticalEvaluator.h

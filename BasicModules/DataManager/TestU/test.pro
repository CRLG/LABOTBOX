QT += widgets testlib
INSTALLS += target

INCLUDEPATH +=  ./  \ # répertoire des scripts de test
                ../ \   # répertoire des sources
                ../../  # répertoire des classes mères
 _________________________________________
# Les sources à tester
SOURCES +=  ../CData.cpp
#            ../CDataManager.cpp
HEADERS +=  ../CData.h
#            ../../CBasicModule.h \
#            ../CDataManager.h
# _________________________________________
# Les tests
SOURCES +=  *.cpp    # (tous les fichiers sources de test)
HEADERS +=  *.h

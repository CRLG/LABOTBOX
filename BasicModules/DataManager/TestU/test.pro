QT += widgets testlib
INSTALLS += target

INCLUDEPATH +=  ./  \ # r�pertoire des scripts de test
                ../ \   # r�pertoire des sources
                ../../  # r�pertoire des classes m�res
 _________________________________________
# Les sources � tester
SOURCES +=  ../CData.cpp
#            ../CDataManager.cpp
HEADERS +=  ../CData.h
#            ../../CBasicModule.h \
#            ../CDataManager.h
# _________________________________________
# Les tests
SOURCES +=  *.cpp    # (tous les fichiers sources de test)
HEADERS +=  *.h

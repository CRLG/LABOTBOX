/*! \file CcsvDataLogger.h
 * A brief file description.
 * A more elaborated file description.
 */
#ifndef _CBASIC_MODULE_csvDataLogger_H_
#define _CBASIC_MODULE_csvDataLogger_H_

#include <QMainWindow>
#include <QTimer>
#include <QElapsedTimer>
#include <QTextStream>
#include <QFile>
#include <QVector>

#include "CBasicModule.h"
#include "ui_ihm_csvDataLogger.h"
#include "CDataLogger.h"

 class Cihm_csvDataLogger : public QMainWindow
{
    Q_OBJECT
public:
    Cihm_csvDataLogger(QWidget *parent = 0)  : QMainWindow(parent) { ui.setupUi(this); }
    ~Cihm_csvDataLogger() { }

    Ui::ihm_csvDataLogger ui;

    CApplication *m_application;
 };



 /*! \addtogroup csvDataLogger
   * 
   *  @{
   */

class CData;
/*! @brief class CcsvDataLogger
 */	   
class CcsvDataLogger : public CBasicModule
{
    Q_OBJECT
#define     VERSION_csvDataLogger   "1.0"
#define     AUTEUR_csvDataLogger    "NSA"
#define     INFO_csvDataLogger      "Enregistreur de données au format csv"

public:
    CcsvDataLogger(const char *plugin_name);
    ~CcsvDataLogger();

    Cihm_csvDataLogger *getIHM(void) { return(&m_ihm); }

    virtual void init(CApplication *application);
    virtual void close(void);
    virtual bool hasGUI(void)           { return(true); }
    virtual QIcon getIcon(void)         { return(QIcon(":/icons/edit_add.png")); }  // Précise l'icône qui représente le module
    virtual QString getMenuName(void)   { return("BasicModule"); }                  // Précise le nom du menu de la fenêtre principale dans lequel le module apparaît

private:
    Cihm_csvDataLogger m_ihm;
    void addVariablesObserver(QStringList liste_variables);
    QStringList getListeVariablesObservees();
    QString getDefaultPathfilename();

    CDataLogger m_data_logger;


    void log_once(qint64 timestamp);

    QString m_output_pathfilename;

private slots :
    void onRightClicGUI(QPoint pos);
    void onDataFilterChanged(QString filter_name);
    void onLoggerStatusChanged(int status);
    void onLoggerStarted(QString filename);
    void onLoggerFinished(QString filename);
    void onSelectFile();

public slots :
    void refreshListeVariables(void);
    void loadListeVariablesObservees(void);
    void saveListeVariablesObservees(void);
    void decocheToutesVariables();
    void afficheVariablesCocheesUniquement();

    void start_logger();
    void stop_logger();
    void pause_logger();
    void log_once();
};

#endif // _CBASIC_MODULE_csvDataLogger_H_

/*! @} */


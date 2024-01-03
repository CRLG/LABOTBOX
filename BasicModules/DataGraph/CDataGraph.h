/*! \file CDataGraph.h
 * A brief file description.
 * A more elaborated file description.
 */
#ifndef _CBASIC_MODULE_DataGraph_H_
#define _CBASIC_MODULE_DataGraph_H_

#include <QMainWindow>
#include <QTimer>

#include "CBasicModule.h"
#include "ui_ihm_DataGraph.h"
#include "qcustomplot.h"

 class Cihm_DataGraph : public QMainWindow
{
    Q_OBJECT
public:
    Cihm_DataGraph(QWidget *parent = 0)  : QMainWindow(parent) { ui.setupUi(this); }
    ~Cihm_DataGraph() { }

    Ui::ihm_DataGraph ui;

    CApplication *m_application;
 };

 /*! \addtogroup DataGraph
   * 
   *  @{
   */

	   
/*! @brief class CDataGraph
 */	   
class CDataGraph : public CBasicModule
{
    Q_OBJECT
#define     VERSION_DataGraph   "1.1"
#define     AUTEUR_DataGraph    "Laguiche"
#define     INFO_DataGraph      "Trace les variables du datacenter"

public:
    CDataGraph(const char *plugin_name);
    ~CDataGraph();

    Cihm_DataGraph *getIHM(void) { return(&m_ihm); }

    virtual void init(CApplication *application);
    virtual void close(void);
    virtual bool hasGUI(void)           { return(true); }
    virtual QIcon getIcon(void)         { return(QIcon(":/icons/plot.png")); }  // Précise l'icône qui représente le module
    //virtual QString getMenuName(void)   { return("BasicModule"); }
    virtual void setVisible(void);

private :
    const int PERIODE_ECHANTILLONNAGE_VARIABLES = 30; // msec

    //void activeInspectorTemporel(void);
    void playGraph(void);
    //void finInspectorTemporel(void);
    void stopGraph(void);
    //void connectDiscconnectVariablesTemporel(bool choix);
    void addTraceVariable(QString name, QString value = "");
    void ManageCursor(QCPItemLine *cursor, double xPosition, int columTableWidget);
    void ResetCursor(void);
    QColor variousColor();
    QComboBox* widgetCouleur(int indexRow);
    void addVariablesObserver(QStringList liste_variables);
    QStringList getListeVariablesObservees();

	QFile* sauvFic;
	QTextStream sauvStream;
    
private slots :
    void onRightClicGUI(QPoint pos);
	void Slot_save(bool toggleState);
    void onDataFilterChanged(QString filter_name);

private:
    Cihm_DataGraph m_ihm;

    QTimer m_timer_lecture_variables;

    QCustomPlot *customPlot;
    QCPItemLine * cursor1;
    double nearDistanceCursor1;
    double nearDistanceCursor2;
    bool isCursor1Selected;
    bool isCursor2Selected;
    int fineDistanceCursorSelection;
    QCPItemLine * cursor2;
    bool graphEnabled;
    bool cursorEnabled;
    QHash<QString, int> m_liste_graph;
    uint graphColor;
    const QColor t_predefinedColor[8]={Qt::black,Qt::red,Qt::green,Qt::blue,Qt::cyan,Qt::magenta,Qt::yellow,Qt::gray};
    QStringList t_predefinedColorName;
    QCPRange* storedRange;
    QCPRange* storedRangeAxe1;
    QCPRange* storedRangeAxe2;
    int dureeObservee;
    bool backupEnabled;


public slots :
    //void variableChanged(QVariant val);
    void refreshListeVariables(void);
    void activeGraph(bool state);
    void refreshValeursVariables(void);
    void upVariable(void);
    void downVariable(void);
    void effacerListeVariablesValues(void);
    //void editingFinishedWriteValueInstantane(void);
    void resumeGraph(bool varToggle);
    void changeGraphCouleur(int indexCouleur);
    void itemRescaleGraph(QTableWidgetItem* item);
    void userRescaleGraph(void);
    void userZoomGraph(int value);
    void rescaleGraph(double newMaxRange, double newMaxRange2);
    void loadListeVariablesObservees(void);
    void saveListeVariablesObservees(void);
    void decocheToutesVariables();

private slots:
    void mousePress(QMouseEvent *event);
    void mouseMove(QMouseEvent *event);
    void mouseRelease(QMouseEvent* event);
    void resetZoom(void);
    void authorizePlay(QListWidgetItem *senderItem);
    void enableCursor(bool varToggle);
};

#endif // _CBASIC_MODULE_DataGraph_H_

/*! @} */


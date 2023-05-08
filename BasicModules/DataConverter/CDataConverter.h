/*! \file CDataConverter.h
 * A brief file description.
 * A more elaborated file description.
 */
#ifndef _CBASIC_MODULE_DataConverter_H_
#define _CBASIC_MODULE_DataConverter_H_

#include <QMainWindow>

#include "CBasicModule.h"
#include "ui_ihm_DataConverter.h"

 class Cihm_DataConverter : public QMainWindow
{
    Q_OBJECT
public:
    Cihm_DataConverter(QWidget *parent = 0)  : QMainWindow(parent) { ui.setupUi(this); }
    ~Cihm_DataConverter() { }

    Ui::ihm_DataConverter ui;

    CApplication *m_application;
 };


class CDataTransformer;
 /*! \addtogroup DataConverter
   * 
   *  @{
   */

	   
/*! @brief class CDataConverter
 */	   
class CDataConverter : public CBasicModule
{
    Q_OBJECT
#define     VERSION_DataConverter   "1.0"
#define     AUTEUR_DataConverter    "Nico"
#define     INFO_DataConverter      "Convertit une donnée en une autre en appliquant un algorithme"

public:
    CDataConverter(const char *plugin_name);
    ~CDataConverter();

    Cihm_DataConverter *getIHM(void) { return(&m_ihm); }

    virtual void init(CApplication *application);
    virtual void close(void);
    virtual bool hasGUI(void)           { return(true); }
    virtual QIcon getIcon(void)         { return(QIcon(":/icons/edit_add.png")); }  // Précise l'icône qui représente le module
    virtual QString getMenuName(void)   { return("BasicModule"); }                  // Précise le nom du menu de la fenêtre principale dans lequel le module apparaît

private:
    Cihm_DataConverter m_ihm;
    QVector<CDataTransformer*> m_transformer_list;
    const QString CSV_COLUMN_SEPARATOR = ";";
    const QString CSV_LINE_SEPARATOR = "\n";
    QString m_last_pathfilename;

private slots :
    void onRightClicGUI(QPoint pos);
    void onDataFilterChanged(QString filter_name);
    void apply_new_transformer();
    void add_data_to_input();
    void add_data_to_output();
    void algo_changed(QString algo);
    void transformer_selection_changed(int index);
    void delete_transformer();
    void delete_transformer(int index);
    void delete_all_transformers();

public slots :
    void refreshListeVariables(void);
    void loadTransformerListFromFile();
    void loadTransformerListFromFile(QString pathfilename);
    void saveTransformerListToFile();
    void saveTransformerListToFile(QString pathfilename);
    void refreshTransformerList();
};

#endif // _CBASIC_MODULE_DataConverter_H_

/*! @} */


/*! \file CEEPROM_CPU.h
 * A brief file description.
 * A more elaborated file description.
 */
#ifndef _CPLUGIN_MODULE_EEPROM_CPU_H_
#define _CPLUGIN_MODULE_EEPROM_CPU_H_

#include <QMainWindow>

#include "CPluginModule.h"
#include "ui_ihm_EEPROM_CPU.h"

 class Cihm_EEPROM_CPU : public QMainWindow
{
    Q_OBJECT
public:
    Cihm_EEPROM_CPU(QWidget *parent = 0)  : QMainWindow(parent) { ui.setupUi(this); }
    ~Cihm_EEPROM_CPU() { }

    Ui::ihm_EEPROM_CPU ui;

    CApplication *m_application;
 };



 /*! \addtogroup EEPROM_CPU
   * 
   *  @{
   */

	   
/*! @brief class CEEPROM_CPU
 */	   
class CEEPROM_CPU : public CPluginModule
{
    Q_OBJECT
#define     VERSION_EEPROM_CPU   "1.0"
#define     AUTEUR_EEPROM_CPU    "Nico"
#define     INFO_EEPROM_CPU      "Lecture / Ecriture EEPROM interne CPU"

public:
    CEEPROM_CPU(const char *plugin_name);
    ~CEEPROM_CPU();

    Cihm_EEPROM_CPU *getIHM(void) { return(&m_ihm); }

    virtual void init(CApplication *application);
    virtual void close(void);
    virtual bool hasGUI(void)           { return(true); }
    virtual QIcon getIcon(void)         { return(QIcon(":/icons/edit_add.png")); }  // Précise l'icône qui représente le module
    virtual QString getMenuName(void)   { return("PluginModule"); }                 // Précise le nom du menu de la fenêtre principale dans lequel le module apparaît

private:
    Cihm_EEPROM_CPU m_ihm;

    typedef enum {
        COL_ADDRESS = 0,
        COL_NAME,
        COL_VALUE,
        COL_TYPE
    }tTABLE_COL;

    typedef union {

        quint32 ulval;
        qint32  sval;
        float   fval;
        quint32 ulbuff[1];
        quint16 usbuff[2];
        quint8  ucbuff[4];
    }uEE;

    bool m_automatic_load_mapping;
    QString m_eep_mapping_pathfilename;

    const QString LINE_SEPARATOR = "\n";
    const QString CSV_SEPARATOR = ";";

    void apply_mapping(unsigned long address, QString name, QString type);
    void apply_value(QString name, QString value);
    QWidget *type2Combobox(QString type);
    void createLine(unsigned long address, QString name, QString value, QString type);
    QString value2Display(unsigned long value, QString type_str);
    void read_1_value(unsigned long address);
    void write_1_value(unsigned long address, unsigned long value);
    void write_1_value(unsigned long table_index);
    unsigned long tableIndex2RawValue(unsigned long table_index, bool *ok=nullptr);
    QString tableIndex2Value(unsigned long table_index);
    QString tableIndex2Name(unsigned long table_index);
    unsigned long tableIndex2Adress(unsigned long table_index);
    QString tableIndex2Type(unsigned long table_index);

    void delay_between_cpu_commands(unsigned int delay_msec=100);

private slots :
    void onRightClicGUI(QPoint pos);
    void onRightClicTable(QPoint p);
    void onDataFilterChanged(QString filter_name);

    void clean();

    void onRxValue(unsigned long address, unsigned long value);

    void import_mapping(QString pathfilename);
    void import_mapping();
    void export_mapping();

    void import_values();
    void export_values();

    void save_table();
    void open_table();

    void read_eeprom_to_table();
    void write_table_to_eeprom();

    void read_selected_index_to_table();
    void write_selected_index_to_table();
};

#endif // _CPLUGIN_MODULE_EEPROM_CPU_H_

/*! @} */


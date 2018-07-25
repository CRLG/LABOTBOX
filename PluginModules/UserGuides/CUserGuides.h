/*! \file CUserGuides.h
 * A brief file description.
 * A more elaborated file description.
 */
#ifndef _CPLUGIN_MODULE_UserGuides_H_
#define _CPLUGIN_MODULE_UserGuides_H_

#include <QMainWindow>

#include "CPluginModule.h"
#include "ui_ihm_UserGuides.h"

 class Cihm_UserGuides : public QMainWindow
{
    Q_OBJECT
public:
    Cihm_UserGuides(QWidget *parent = 0)  : QMainWindow(parent) { ui.setupUi(this); }
    ~Cihm_UserGuides() { }

    Ui::ihm_UserGuides ui;

    CLaBotBox *m_application;
 };



 /*! \addtogroup UserGuides
   * 
   *  @{
   */
class HtmlTextEditor;
	   
/*! @brief class CUserGuides
 */	   
class CUserGuides : public CPluginModule
{
    Q_OBJECT
#define     VERSION_UserGuides   "1.0"
#define     AUTEUR_UserGuides    "Nico"
#define     INFO_UserGuides      "Manipulation des guides d'utilisation des modules"

public:
    CUserGuides(const char *plugin_name);
    ~CUserGuides();

    Cihm_UserGuides *getIHM(void) { return(&m_ihm); }

    virtual void init(CLaBotBox *application);
    virtual void close(void);
    virtual bool hasGUI(void)           { return(true); }
    virtual QIcon getIcon(void)         { return(QIcon(":/icons/question.png")); }  // Précise l'icône qui représente le module
    virtual QString getMenuName(void)   { return("?"); }                 // Précise le nom du menu de la fenêtre principale dans lequel le module apparaît

private:
    Cihm_UserGuides m_ihm;
    HtmlTextEditor *m_html_text_editor;

    void initListUserGuides();

private slots :
    void onRightClicGUI(QPoint pos);
    void onUserGuideSelectedChange(QString name);
    void onUserGuideEditorSelected();
};

#endif // _CPLUGIN_MODULE_UserGuides_H_

/*! @} */


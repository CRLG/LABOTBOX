/*! \file CPrintView.h
 * A brief file description.
 * A more elaborated file description.
 */
#ifndef _CTRACE_LOGGER_H_
#define _CTRACE_LOGGER_H_

#include "CBasicModule.h"
#include "ui_PrintView.h"

 /*! \addtogroup PrintView
   * 
   *  @{
   */


//! Niveau des messages de trace (à déplacer dans le module Logger)
typedef enum {
    MSG_ERREUR              = 0x00000000,
    MSG_WARNING_N1          = 0x00000001,
    MSG_WARNING_N2          = 0x00000002,
    MSG_WARNING_N3          = 0x00000004,
    MSG_DEBUG_N1            = 0x00000008,
    MSG_DEBUG_N2            = 0x00000010,
    MSG_DEBUG_N3            = 0x00000020,
    MSG_INFO_N1             = 0x00000040,
    MSG_INFO_N2             = 0x00000080,
    MSG_INFO_N3             = 0x00000100,

    MSG_WARNING             = (MSG_WARNING_N1|MSG_WARNING_N2|MSG_WARNING_N3),
    MSG_DEBUG               = (MSG_DEBUG_N1|MSG_DEBUG_N2|MSG_DEBUG_N3),
    MSG_INFO                = (MSG_INFO_N1|MSG_INFO_N2|MSG_INFO_N3),
    MSG_TOUS                = 0xFFFFFFFF
}tNiveauTraceAutorises;

/*! @brief class CPrintView in @link PrintView basic module.
 */	   
class CPrintView : public CBasicModule
{
    Q_OBJECT

private :
#define     VERSION         "1.0"
#define     AUTEUR          "Nico"
#define     INFO            "Logger de traces"

public:
    CPrintView(const char *plugin_name);
    ~CPrintView();

    void print(QString msg);
    void print(CModule *module, QString msg, unsigned long type_trace);
    void print_error(CModule *module, QString msg)    { print(module, msg, MSG_ERREUR); }
    void print_warning(CModule *module, QString msg)  { print(module, msg, MSG_WARNING); }
    void print_info(CModule *module, QString msg)     { print(module, msg, MSG_INFO); }
    void print_debug(CModule *module, QString msg)    { print(module, msg, MSG_DEBUG); }

    virtual void init(CLaBotBox *application);
    virtual void close(void);
    virtual bool hasGUI(void) { return(true); }
    virtual QIcon getIcon(void) { return(QIcon(":/icons/bloc_note.png")); }

private:
    Ui::ihm_print_view *m_ihm_print_view;
    QDialog *m_dialog;

    //! Autorisation d'affichage des traces
    bool m_trace_enable;


public slots :
    void ConfigurerNiveauxModules(void);
    void setEnableTrace(bool on_off);
    void enregistreTrace(void);
    //void setVisible(void);

};

#endif // _CTRACE_LOGGER_H_

/*! @} */


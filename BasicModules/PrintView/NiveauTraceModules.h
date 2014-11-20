/*! \file CNiveauTraceModules.h
 * A brief file description.
 * A more elaborated file description.
 */
#ifndef _NIVEAU_TRACE_MODULES_
#define _NIVEAU_TRACE_MODULES_

#include <QDialog>
#include <QMap>
#include "ui_NiveauTraceModules.h"

 /*! \addtogroup PrintView
   * 
   *  @{
   */


/*! @brief class CNiveauTraceModules in @link PrintView basic module.
 */	   
class CNiveauTraceModules : public QDialog
{
    Q_OBJECT

public :
typedef QMap<QString, unsigned long> t_map_niv_aff;


public:
    CNiveauTraceModules(QWidget *parent = 0);
    ~CNiveauTraceModules();

    void setListeModules(t_map_niv_aff *liste_modules_niveaux);

private : 
        Ui::NiveauTraceModules ui;
        //! Liste des modules avec leur niveau d'affichage associé
        t_map_niv_aff *m_liste_modules_niveaux;

        //! Indique si tous les éléments du tableau sont créés
        bool m_table_created;

public slots :
        void selectAll(void);
        void unselectAll(void);
        void refreshMemory(int row, int column);
        void refreshLine(int row, int column);
 };

#endif // _NIVEAU_TRACE_MODULES_

/*! @} */


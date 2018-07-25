/*! \file CNiveauTraceModules.cpp
 * A brief file description CPP.
 * A more elaborated file description.
 */
#include <QDebug>
#include "NiveauTraceModules.h"
#include "CMainWindow.h"
#include "CPrintView.h"


/*! \addtogroup PrintView
   * 
   *  @{
   */
	   
// _____________________________________________________________________
/*!
*  Constructeur
*
*/
CNiveauTraceModules::CNiveauTraceModules(QWidget *parent)
    : QDialog(parent),
      m_table_created(false)
{
   ui.setupUi(this);
   ui.table_niveau_aff_modules->setColumnWidth(0, 155);

   connect(this->ui.pb_select_all, SIGNAL(clicked()), this, SLOT(selectAll()));
   connect(this->ui.pb_deselect_all, SIGNAL(clicked()), this, SLOT(unselectAll()));
   connect(this->ui.table_niveau_aff_modules, SIGNAL(cellChanged(int,int)), this, SLOT(refreshMemory(int,int)));
   connect(this->ui.table_niveau_aff_modules, SIGNAL(cellDoubleClicked(int,int)), this, SLOT(refreshLine(int,int)));
}


// _____________________________________________________________________
/*!
*  Destructeur
*
*/
CNiveauTraceModules::~CNiveauTraceModules()
{
}



// _____________________________________________________________________
/*!
*
*
*/
void CNiveauTraceModules::setListeModules(t_map_niv_aff *liste_modules_niveaux)
{
  int i;
  t_map_niv_aff::const_iterator it;

  // mémorise la liste
  m_liste_modules_niveaux = liste_modules_niveaux;

  // met à jour l'IHM avec cette nouvelle liste
  //ui.table_niveau_aff_modules->clear();

  ui.table_niveau_aff_modules->setRowCount(liste_modules_niveaux->count());
  i=0;
  for (it=liste_modules_niveaux->constBegin();  it!=liste_modules_niveaux->constEnd(); it++) {
      for (int j=0; j<ui.table_niveau_aff_modules->columnCount(); j++) {
          ui.table_niveau_aff_modules->setItem(i, j, new QTableWidgetItem());
          if (j!=0) {
              ui.table_niveau_aff_modules->item(i, j)->setFlags(Qt::ItemIsUserCheckable|Qt::ItemIsEnabled);
              ui.table_niveau_aff_modules->item(i,j)->setCheckState(Qt::Unchecked);
          }
          else { // cas spécial pour la cellule contenant le nom du module
              ui.table_niveau_aff_modules->item(i, j)->setFlags(Qt::ItemIsEnabled);
              ui.table_niveau_aff_modules->item(i, j)->setToolTip(tr("Double clic sur le nom du module pour cocher/decocher toute la ligne"));
          }
      } // for toutes les cellules du tablea

      // Renseigne les valeurs du tableau
      ui.table_niveau_aff_modules->item(i, 0)->setText(it.key());
      ui.table_niveau_aff_modules->item(i, 1)->setCheckState((it.value()&MSG_WARNING_N1)?Qt::Checked: Qt::Unchecked);
      ui.table_niveau_aff_modules->item(i, 2)->setCheckState((it.value()&MSG_WARNING_N2)?Qt::Checked: Qt::Unchecked);
      ui.table_niveau_aff_modules->item(i, 3)->setCheckState((it.value()&MSG_WARNING_N3)?Qt::Checked: Qt::Unchecked);
      ui.table_niveau_aff_modules->item(i, 4)->setCheckState((it.value()&MSG_INFO_N1)?Qt::Checked: Qt::Unchecked);
      ui.table_niveau_aff_modules->item(i, 5)->setCheckState((it.value()&MSG_INFO_N2)?Qt::Checked: Qt::Unchecked);
      ui.table_niveau_aff_modules->item(i, 6)->setCheckState((it.value()&MSG_INFO_N3)?Qt::Checked: Qt::Unchecked);
      ui.table_niveau_aff_modules->item(i, 7)->setCheckState((it.value()&MSG_DEBUG_N1)?Qt::Checked: Qt::Unchecked);
      ui.table_niveau_aff_modules->item(i, 8)->setCheckState((it.value()&MSG_DEBUG_N2)?Qt::Checked: Qt::Unchecked);
      ui.table_niveau_aff_modules->item(i, 9)->setCheckState((it.value()&MSG_DEBUG_N3)?Qt::Checked: Qt::Unchecked);

      i++;
  }

  m_table_created = true;
}


// _____________________________________________________________________
/*!
*
*
*/
void CNiveauTraceModules::selectAll(void)
{
  int i,j;

  for (i=0; i<ui.table_niveau_aff_modules->rowCount(); i++) {
    for (j=1; j<ui.table_niveau_aff_modules->columnCount(); j++) {
        ui.table_niveau_aff_modules->item(i, j)->setCheckState(Qt::Checked);
    } // for toutes les colonnes checkable
  } // for toutes les lignes
}

// _____________________________________________________________________
/*!
*
*
*/
void CNiveauTraceModules::unselectAll(void)
{
  int i,j;

  for (i=0; i<ui.table_niveau_aff_modules->rowCount(); i++) {
    for (j=1; j<ui.table_niveau_aff_modules->columnCount(); j++) {
        ui.table_niveau_aff_modules->item(i, j)->setCheckState(Qt::Unchecked);
    } // for toutes les colonnes checkable
  } // for toutes les lignes
}


// _____________________________________________________________________
/*!
* Met à jour la mémoire lorsqu'une case change d'état
*
*/
void CNiveauTraceModules::refreshMemory(int row, int column)
{
 Q_UNUSED(column)

 QString module_name;
 unsigned long niveau;

 if (m_table_created == false) { return; }

 module_name = ui.table_niveau_aff_modules->item(row, 0)->text();
 niveau = (
          ((ui.table_niveau_aff_modules->item(row, 1)->checkState()==Qt::Checked?(int)MSG_WARNING_N1:0))
        | ((ui.table_niveau_aff_modules->item(row, 2)->checkState()==Qt::Checked?(int)MSG_WARNING_N2:0))
        | ((ui.table_niveau_aff_modules->item(row, 3)->checkState()==Qt::Checked?(int)MSG_WARNING_N3:0))
        | ((ui.table_niveau_aff_modules->item(row, 4)->checkState()==Qt::Checked?(int)MSG_INFO_N1:0))
        | ((ui.table_niveau_aff_modules->item(row, 5)->checkState()==Qt::Checked?(int)MSG_INFO_N2:0))
        | ((ui.table_niveau_aff_modules->item(row, 6)->checkState()==Qt::Checked?(int)MSG_INFO_N3:0))
        | ((ui.table_niveau_aff_modules->item(row, 7)->checkState()==Qt::Checked?(int)MSG_DEBUG_N1:0))
        | ((ui.table_niveau_aff_modules->item(row, 8)->checkState()==Qt::Checked?(int)MSG_DEBUG_N2:0))
        | ((ui.table_niveau_aff_modules->item(row, 9)->checkState()==Qt::Checked?(int)MSG_DEBUG_N3:0))
      );

 m_liste_modules_niveaux->insert(module_name, niveau);
}


/*!
* Change toute la ligne d'un seul coup.
*
*/
void CNiveauTraceModules::refreshLine(int row, int column)
{
 QString module_name;
 unsigned long cpt_checked=0;

 if (m_table_created == false) { return; }
 if (column != 0) { return; } // L'évènement n'a un effet que s'il est fait sur le nom du module

 // Comptabilise le nombre d'éléments cochés sur la ligne
 for (unsigned long i=1; i<=9; i++) {
    cpt_checked+= ui.table_niveau_aff_modules->item(row, i)->checkState()==Qt::Checked;
 }
 // Si aucune case n'était cochée -> les coche toutes
 Qt::CheckState checkstate;
 if (cpt_checked == 0)  { checkstate = Qt::Checked; }
 else                   { checkstate = Qt::Unchecked; }
 for (unsigned long i=1; i<=9; i++) {
     ui.table_niveau_aff_modules->item(row, i)->setCheckState(checkstate);
 }

 // La mise à jour de la mémoire est faite automatiquement
 // grace au fait les cellules sont connectées au slot refreshMemory
 // lorsque leur valeur change
}


/*! @} */

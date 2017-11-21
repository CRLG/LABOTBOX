/*! \file CStrategyDesigner.cpp
 * A brief file description CPP.
 * A more elaborated file description.
 */
#include <QDebug>
#include "CStrategyDesigner.h"
#include "CLaBotBox.h"
#include "CPrintView.h"
#include "CMainWindow.h"
#include "CEEPROM.h"
#include "CDataManager.h"



/*! \addtogroup PluginModule_Test2
   * 
   *  @{
   */
	   
// _____________________________________________________________________
/*!
*  Constructeur
*
*/
CStrategyDesigner::CStrategyDesigner(const char *plugin_name)
    :CPluginModule(plugin_name, VERSION_StrategyDesigner, AUTEUR_StrategyDesigner, INFO_StrategyDesigner)
{
    bPlay=false;
    bResume=false;
    bStop=false;
}


// _____________________________________________________________________
/*!
*  Destructeur
*
*/
CStrategyDesigner::~CStrategyDesigner()
{

}


// _____________________________________________________________________
/*!
*  Initialisation du module
*
*/
void CStrategyDesigner::init(CLaBotBox *application)
{
    CPluginModule::init(application);
    setGUI(&m_ihm); // indique à la classe de base l'IHM
    setNiveauTrace(MSG_TOUS);

    // Gère les actions sur clic droit sur le panel graphique du module
    m_ihm.setContextMenuPolicy(Qt::CustomContextMenu);
    connect(&m_ihm, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(onRightClicGUI(QPoint)));

    // Restore la taille de la fenêtre
    QVariant val;
    val = m_application->m_eeprom->read(getName(), "geometry", QRect(50, 50, 150, 150));
    m_ihm.setGeometry(val.toRect());
    // Restore le fait que la fenêtre est visible ou non
    val = m_application->m_eeprom->read(getName(), "visible", QVariant(true));
    if (val.toBool()) { m_ihm.show(); }
    else              { m_ihm.hide(); }
    // Restore la couleur de fond
    val = m_application->m_eeprom->read(getName(), "background_color", QVariant(DEFAULT_MODULE_COLOR));
    setBackgroundColor(val.value<QColor>());

    //récupération des valeurs servos
    QVariant valList;
    valList=m_application->m_eeprom->read(getName(),"list_servo_name",QStringList());
    servoName=valList.toStringList();
    if(servoName.isEmpty())
        servoName << "DEFAULT_NAME";
    valList=m_application->m_eeprom->read(getName(),"list_servo_value",QStringList());
    servoValue=valList.toStringList();
    if(servoValue.isEmpty())
        servoValue << "DEFAULT_VALUE";
    //récupération des valeurs moteur
    valList=m_application->m_eeprom->read(getName(),"list_moteur_name",QStringList());
    moteurName=valList.toStringList();
    if(moteurName.isEmpty())
        moteurName << "DEFAULT_NAME";
    valList=m_application->m_eeprom->read(getName(),"list_moteur_value",QStringList());
    moteurValue=valList.toStringList();
    if(moteurValue.isEmpty())
        moteurValue << "DEFAULT_VALUE";

    lineEdit_X=m_ihm.findChild<QLineEdit*>("lineEdit_x");
    lineEdit_Y=m_ihm.findChild<QLineEdit*>("lineEdit_y");
    lineEdit_Theta=m_ihm.findChild<QLineEdit*>("lineEdit_theta");
    lineEdit_Angle=m_ihm.findChild<QLineEdit*>("lineEdit_angle");
    lineEdit_Distance=m_ihm.findChild<QLineEdit*>("lineEdit_distance");

    tableWidget_SM=m_ihm.findChild<QTableWidget*>("tableWidget_SM");
    tableWidget_SM->setColumnCount(6);
    QStringList QS_Labels;
    QS_Labels << "Type" << "Fonction" << "Données 1" << "Données 2" << "Données 3" << "Commentaires";
    tableWidget_SM->setHorizontalHeaderLabels(QS_Labels);
    blistFirstFilled=false;
    connect(tableWidget_SM, SIGNAL(cellEntered(int,int)), this, SLOT(cellEnteredSlot(int,int)));



    QComboBox *comboBox_Deplacement=m_ihm.findChild<QComboBox*>("comboBox_Deplacement");
    comboBox_Deplacement->addItem("X Y", QVariant(X_Y));
    comboBox_Deplacement->addItem("X Y Theta", QVariant(X_Y_Teta));
    comboBox_Deplacement->addItem("Distance", QVariant(Distance));
    comboBox_Deplacement->addItem("Distance Angle", QVariant(Distance_Angle));
    comboBox_Deplacement->addItem("Angle", QVariant(Angle));
    comboBox_Deplacement->addItem("Boucle ouverte", QVariant(boucleOuverte));

    QPushButton *pushButton_Deplacement_plus=m_ihm.findChild<QPushButton*>("pushButton_deplacement_plus");
    pushButton_Deplacement_plus->setIcon(QIcon(":/icons/edit_add.png"));
    connect(pushButton_Deplacement_plus,SIGNAL(clicked()),this,SLOT(Slot_AddDeplacement()));

    QComboBox *comboBox_Transition=m_ihm.findChild<QComboBox*>("comboBox_Transition");
    comboBox_Transition->addItem("isFinMouvement");
    comboBox_Transition->addItem("isFinMouvementRapide");
    comboBox_Transition->addItem("Tempo");

    QPushButton *pushButton_Transition_plus=m_ihm.findChild<QPushButton*>("pushButton_transition_plus");
    pushButton_Transition_plus->setIcon(QIcon(":/icons/edit_add.png"));
    connect(pushButton_Transition_plus,SIGNAL(clicked()),this,SLOT(Slot_AddTransition()));

    QComboBox *comboBox_Action=m_ihm.findChild<QComboBox*>("comboBox_Action");
    comboBox_Action->addItem("PositionServo");
    comboBox_Action->addItem("CommandeMoteur");

    QComboBox *comboBox_Action_name=m_ihm.findChild<QComboBox*>("comboBox_Action_name");
    comboBox_Action_name->addItems(servoName);
    QComboBox *comboBox_Action_value=m_ihm.findChild<QComboBox*>("comboBox_Action_value");
    comboBox_Action_value->addItems(servoValue);
    connect(comboBox_Action,SIGNAL(currentTextChanged(QString)),this,SLOT(Slot_ChoseAction(QString)));

    QPushButton *pushButton_Action_plus=m_ihm.findChild<QPushButton*>("pushButton_action_plus");
    pushButton_Action_plus->setIcon(QIcon(":/icons/edit_add.png"));
    connect(pushButton_Action_plus,SIGNAL(clicked()),this,SLOT(Slot_AddAction()));

    QPushButton *pushButton_Moins=m_ihm.findChild<QPushButton*>("pushButton_moins");
    pushButton_Moins->setIcon(QIcon(":/icons/edit_remove.png"));
    connect(pushButton_Moins,SIGNAL(clicked()),this,SLOT(Slot_Remove()));


    QPushButton *pushButton_Generate=m_ihm.findChild<QPushButton*>("pushButton_generate");
    pushButton_Generate->setIcon(QIcon(":/icons/matlab.png"));
    connect(pushButton_Generate,SIGNAL(clicked()),this,SLOT(Slot_GenerateMFile()));

    QPushButton *pushButton_save=m_ihm.findChild<QPushButton*>("pushButton_save");
    pushButton_save->setIcon(QIcon(":/icons/save.png"));
    connect(pushButton_save,SIGNAL(clicked()),this,SLOT(Slot_Save()));

    QPushButton *pushButton_load=m_ihm.findChild<QPushButton*>("pushButton_load");
    pushButton_load->setIcon(QIcon(":/icons/open.png"));
    connect(pushButton_load,SIGNAL(clicked()),this,SLOT(Slot_Load()));

    QPushButton *pushButton_Play=m_ihm.findChild<QPushButton*>("pushButton_play");
    pushButton_Play->setIcon(QIcon(":/icons/player_play.png"));
    connect(pushButton_Play,SIGNAL(clicked()),this,SLOT(Slot_Play()));

    QPushButton *pushButton_Resume=m_ihm.findChild<QPushButton*>("pushButton_resume");
    pushButton_Resume->setIcon(QIcon(":/icons/player_pause.png"));
    connect(pushButton_Resume,SIGNAL(clicked()),this,SLOT(Slot_Resume()));

    QPushButton *pushButton_Stop=m_ihm.findChild<QPushButton*>("pushButton_stop");
    pushButton_Stop->setIcon(QIcon(":/icons/player_stop.png"));
    connect(pushButton_Stop,SIGNAL(clicked()),this,SLOT(Slot_Stop()));

    connect(m_application->m_data_center,SIGNAL(valueChanged(CData*)),this,SLOT(Slot_CoordChanged(CData*)));
}


// _____________________________________________________________________
/*!
*  Fermeture du module
*
*/
void CStrategyDesigner::close(void)
{
  // Mémorise en EEPROM l'état de la fenêtre
  m_application->m_eeprom->write(getName(), "geometry", QVariant(m_ihm.geometry()));
  m_application->m_eeprom->write(getName(), "visible", QVariant(m_ihm.isVisible()));
  m_application->m_eeprom->write(getName(), "background_color", QVariant(getBackgroundColor()));
}

// _____________________________________________________________________
/*!
*  Création des menus sur clic droit sur la fenêtre du module
*
*/
void CStrategyDesigner::onRightClicGUI(QPoint pos)
{
  QMenu *menu = new QMenu();

  menu->addAction("Select background color", this, SLOT(selectBackgroundColor()));
  menu->exec(m_ihm.mapToGlobal(pos));
}

//void CStrategyDesigner::Slot_X(QVariant val){lineEdit_X->setText(val.toString());}

//void CStrategyDesigner::Slot_Y(QVariant val){lineEdit_Y->setText(val.toString());}

//void CStrategyDesigner::Slot_Theta(QVariant val){lineEdit_Theta->setText(val.toString());}

void CStrategyDesigner::Slot_CoordChanged(CData* data)
{
    QString dataName=data->getName();
    QString str;
    if(dataName.compare("PosX_robot")==0)
        lineEdit_X->setText(str.number(data->read().toDouble(),'f',2));
    if(dataName.compare("PosY_robot")==0)
        lineEdit_Y->setText(str.number(data->read().toDouble(),'f',2));
    if(dataName.compare("PosTeta_robot")==0)
        lineEdit_Theta->setText(str.number(data->read().toDouble(),'f',4));
    if(dataName.compare("DirDistance_robot")==0)
        lineEdit_Distance->setText(str.number(data->read().toDouble(),'f',2));
    if(dataName.compare("DirAngle_robot")==0)
        lineEdit_Angle->setText(str.number(data->read().toDouble(),'f',4));

}

void CStrategyDesigner::Slot_AddDeplacement()
{
    int nbArgs=0;
    QComboBox *comboBox_Deplacement=m_ihm.findChild<QComboBox*>("comboBox_Deplacement");
    int choixDeplacement=comboBox_Deplacement->currentData().toInt();
    QTableWidgetItem *newItem = new QTableWidgetItem();
    QTableWidgetItem *newItem_Data1 = new QTableWidgetItem();
    QTableWidgetItem *newItem_Data2 = new QTableWidgetItem();
    QTableWidgetItem *newItem_Data3 = new QTableWidgetItem();
    QTableWidgetItem *newItem_Comments = new QTableWidgetItem();


    switch (choixDeplacement)
    {
        case X_Y:
        nbArgs=2;
        newItem->setText("Mouvement_XY");
        newItem_Data1->setText(lineEdit_X->text());
        newItem_Data2->setText(lineEdit_Y->text());
        break;

        case X_Y_Teta:
        nbArgs=3;
        newItem->setText("Mouvement_XYTeta");
        newItem_Data1->setText(lineEdit_X->text());
        newItem_Data2->setText(lineEdit_Y->text());
        newItem_Data3->setText(lineEdit_Theta->text());
        break;

        case Distance_Angle:
        nbArgs=2;
        newItem->setText("Mouvement_DistanceAngle");
        newItem_Data1->setText(lineEdit_X->text());
        newItem_Data2->setText(lineEdit_Y->text());
        break;

        case Angle:
        nbArgs=1;
        newItem->setText("Mouvement_Angle");
        newItem_Data1->setText(lineEdit_X->text());
        break;

        case Distance:
        nbArgs=1;
        newItem->setText("Mouvement_Distance");
        newItem_Data1->setText(lineEdit_X->text());
        break;

        case boucleOuverte:
        nbArgs=2;
        newItem->setText("Mouvement_Manuel");
        newItem_Data1->setText(lineEdit_X->text());
        newItem_Data2->setText(lineEdit_Y->text());
        break;

        default:
        break;

    }

    if(sender()->objectName().compare("pushButton_deplacement_plus")==0)
    {
        //isFinMouvement    isFinMouvementRapide
        QTableWidgetItem *newItem_Type = new QTableWidgetItem("Deplacement");

        int indexAdd=tableWidget_SM->rowCount();
        tableWidget_SM->insertRow(indexAdd);
        tableWidget_SM->setItem(indexAdd, 0, newItem_Type);
        tableWidget_SM->setItem(indexAdd, 1, newItem);
        tableWidget_SM->setItem(indexAdd, 2, newItem_Data1);
        tableWidget_SM->setItem(indexAdd, 3, newItem_Data2);
        tableWidget_SM->setItem(indexAdd, 4, newItem_Data3);
        tableWidget_SM->setItem(indexAdd, 5, newItem_Comments);
        //qDebug() << "inclusion de "<<choixDeplacement << " à " <<indexAdd;
    }

    //TODO: mettre l'action pour le bouton de remplacement d'action - garde-t-on les mêmes paramètres
//    if(sender()->objectName().compare("pushButton_deplacement_remplace")==0)
//    {

//    }

}

void CStrategyDesigner::Slot_AddAction()
{
    QComboBox *comboBox_Action=m_ihm.findChild<QComboBox*>("comboBox_Action");
    QComboBox *comboBox_Action_name=m_ihm.findChild<QComboBox*>("comboBox_Action_name");
    QComboBox *comboBox_Action_value=m_ihm.findChild<QComboBox*>("comboBox_Action_value");
    QString choixAction=comboBox_Action->currentText();
    QString commande;
    if(choixAction.compare("PositionServo")==0)
    {
        commande="setServo";
    }
    if(choixAction.compare("CommandeMoteur")==0)
    {
        commande="setPWM";
    }
    QTableWidgetItem *newItem = new QTableWidgetItem(commande);
    QTableWidgetItem *newItem_Type = new QTableWidgetItem("Action");
    QTableWidgetItem *newItem_Data1 = new QTableWidgetItem(comboBox_Action_name->currentText());
    QTableWidgetItem *newItem_Data2 = new QTableWidgetItem(comboBox_Action_value->currentText());
    QTableWidgetItem *newItem_Data3 = new QTableWidgetItem();
    QTableWidgetItem *newItem_Comments = new QTableWidgetItem();

    int indexAdd=tableWidget_SM->rowCount();
    tableWidget_SM->insertRow(indexAdd);
    tableWidget_SM->setItem(indexAdd, 0, newItem_Type);
    tableWidget_SM->setItem(indexAdd, 1, newItem);
    tableWidget_SM->setItem(indexAdd, 2, newItem_Data1);
    tableWidget_SM->setItem(indexAdd, 3, newItem_Data2);
    tableWidget_SM->setItem(indexAdd, 4, newItem_Data3);
    tableWidget_SM->setItem(indexAdd, 5, newItem_Comments);
    //qDebug() << "inclusion de "<<choixTransition << " à " <<indexAdd;
}

void CStrategyDesigner::Slot_AddTransition()
{
    QComboBox *comboBox_Transition=m_ihm.findChild<QComboBox*>("comboBox_Transition");
    QString choixTransition=comboBox_Transition->currentText();
    QTableWidgetItem *newItem = new QTableWidgetItem(choixTransition);
    QTableWidgetItem *newItem_Type = new QTableWidgetItem("Transition");
    QTableWidgetItem *newItem_Data1 = new QTableWidgetItem("5");
    QTableWidgetItem *newItem_Data2 = new QTableWidgetItem();
    QTableWidgetItem *newItem_Data3 = new QTableWidgetItem();
    QTableWidgetItem *newItem_Comments = new QTableWidgetItem();

    int indexAdd=tableWidget_SM->rowCount();
    tableWidget_SM->insertRow(indexAdd);
    tableWidget_SM->setItem(indexAdd, 0, newItem_Type);
    tableWidget_SM->setItem(indexAdd, 1, newItem);
    tableWidget_SM->setItem(indexAdd, 2, newItem_Data1);
    tableWidget_SM->setItem(indexAdd, 3, newItem_Data2);
    tableWidget_SM->setItem(indexAdd, 4, newItem_Data3);
    tableWidget_SM->setItem(indexAdd, 5, newItem_Comments);
    //qDebug() << "inclusion de "<<choixTransition << " à " <<indexAdd;
}

void CStrategyDesigner::Slot_GenerateMFile(){

    int indexItem=0;
    int yEtape=80;
    int Etape1=-1;
    int Etape2=-1;
    bool bAttenteTransition=false;
    QString cetteEtape;

    QString cetteTransition=("\tdtA = Stateflow.Transition(objArray); \n\
                \tdtA.Destination = SA0; \n\
                \tdtA.DestinationOClock = 0; \n\
                \txsource = SA0.Position(1)+SA0.Position(3)/2-10; \n\
                \tysource = SA0.Position(2)-20; \n\
                \tdtA.SourceEndPoint = [xsource ysource];\n");

    QString sEtapePart1=("\tSA%1=Stateflow.State(objArray); \n\
                    \tSA%1.Name=\'ACTION_%1\'; \n\
                    \tSA%1.Position=[80 %2 200 100]; \n\
                    \taction=[\'ACTION_%1\',10,\'entry:\'");
    QString sActionTemplate=(",10,\'%1");
    QString sActionComment=(",\'/*%1*/\'");
    QString sEtapePart2=("]; \n\
                    \tSA%1.LabelString=action; \n\
                    \tSA%1.IsGrouped=true; \n\
                    \tSA%1.FontSize=9; \n\
                    ");

    QString sTransition=("\ttA%1A%2 = Stateflow.Transition(objArray); \n\
                         \ttA%1A%2.Source = SA%1; \n\
                         \ttA%1A%2.Destination = SA%2; \n\
                         \ttA%1A%2.SourceOClock = 6; \n\
                         \ttA%1A%2.DestinationOClock = 12; \n\
                        \ttransition=\'[%3(%4/Te)]\'; \n\
                        \ttA%1A%2.LabelString = transition; \n\
                      ");
    QString sAction1arg=(",10,\'%1(%2);\'");
    QString sAction2arg=(",10,\'%1(%2,%3);\'");
    QString sAction3arg=(",10,\'%1(%2,%3,%4);\'");

    //lecture du template
    QFile fichier(":/Templates/Stateflow_script.m");
    fichier.open(QIODevice::ReadOnly | QIODevice::Text);
    QTextStream flux(&fichier);
    QString canevas=flux.readAll();
    //qDebug()<<flux.readAll();
    fichier.close();

    // Création d'un objet QFile
    QString ficName("draftStateflow_");
    QString temps2=QTime::currentTime().toString("hh_mm");
    QString temps1 = QDate::currentDate().toString("yyyy_MM_dd_at_");
    ficName.append(temps1);
    ficName.append(temps2);
    ficName.append(".m");
    //qDebug() << ficName;

    QFile fichier2(ficName);
    // On ouvre notre fichier en lecture seule et on vérifie l'ouverture
    if (fichier2.open(QIODevice::ReadWrite | QIODevice::Text))
    {
        // Création d'un objet QTextStream à partir de notre objet QFile
        QTextStream flux2(&fichier2);
        // On choisit le codec correspondant au jeu de caractère que l'on souhaite ; ici, UTF-8
        flux2.setCodec("UTF-8");
        // Écriture du canevas dans le fichier
        flux2 << canevas << endl;

        for (indexItem=0;indexItem<tableWidget_SM->rowCount();indexItem++)
        {

            QString sAction=tableWidget_SM->item(indexItem,1)->text();
            QString typeLigne=tableWidget_SM->item(indexItem,0)->text();

            //on attend en premier lieu un déplacement et on saute les autres (il ne peut y avoir qu'un
            //déplacement par transition)
            //une fois le déplacement enregistré on marque l'attente d'une transition
            if((typeLigne.compare("Deplacement")==0) && (!bAttenteTransition))
            {
                //EtapeInit=indexItem;
                //on crée l'étape et on enregistre son numéro
                yEtape=yEtape+140;
                cetteEtape=sEtapePart1.arg(QString::number(indexItem),QString::number(yEtape));
                QString sActionTotale=sActionTemplate.arg(sAction);
                sActionTotale.append('(');
                for(int j=2;j<5;j++)
                {
                    QString sData=tableWidget_SM->item(indexItem,j)->text();
                    if(!sData.isEmpty())
                    {
                        sActionTotale.append(sData);
                        sActionTotale.append(',');
                    }
                }
                sActionTotale.remove(sActionTotale.lastIndexOf(','),1);
                sActionTotale.append("); \'");
                cetteEtape.append(sActionTotale);
                QString extractComment=tableWidget_SM->item(indexItem,5)->text();
                if (!extractComment.isEmpty())
                    cetteEtape.append(sActionComment.arg(extractComment));

                //Recherche d'actions qui complètent l'étape
                int k=indexItem;
                QString nextTypeLigne;
                QList<int> indexActions;
                do
                {
                    k++;
                    if(k<(tableWidget_SM->rowCount())-1)
                    {
                    nextTypeLigne=tableWidget_SM->item(k,0)->text();
                    if(nextTypeLigne.compare("Action")==0) //une action se trouve avant la transition
                        //on enregistre laligne
                        indexActions << k;
                    }
                } while((k<((tableWidget_SM->rowCount())-1))&&(nextTypeLigne.compare("Transition")!=0));
                //on ajoute les actions
                for(k=0;k<indexActions.size();k++)
                {
                    QString arg1,arg2,arg3;
                    int nbActionsArgs=0;
                    if(!tableWidget_SM->item(indexActions.at(k),2)->text().isEmpty()){
                        arg1=tableWidget_SM->item(indexActions.at(k),2)->text(); nbActionsArgs++;}
                    if(!tableWidget_SM->item(indexActions.at(k),3)->text().isEmpty()){
                        arg2=tableWidget_SM->item(indexActions.at(k),3)->text(); nbActionsArgs++;}
                    if(!tableWidget_SM->item(indexActions.at(k),4)->text().isEmpty()){
                        arg3=tableWidget_SM->item(indexActions.at(k),4)->text(); nbActionsArgs++;}
                    QString actionEnPlus;

                    switch(nbActionsArgs)
                    {
                        case 1:
                            actionEnPlus=sAction1arg.arg(tableWidget_SM->item(indexActions.at(k),1)->text(),arg1);
                            break;
                        case 2:
                            actionEnPlus=sAction2arg.arg(tableWidget_SM->item(indexActions.at(k),1)->text(),arg1,arg2);
                            break;
                        case 3:
                            actionEnPlus=sAction3arg.arg(tableWidget_SM->item(indexActions.at(k),1)->text(),arg1,arg2,arg3);
                            break;
                        default:
                            //TODO: une action sans paramètre on a?
                            break;
                    }

                    if(!tableWidget_SM->item(indexActions.at(k),5)->text().isEmpty())
                        actionEnPlus.append(sActionComment.arg(tableWidget_SM->item(indexActions.at(k),5)->text()));
                    cetteEtape.append(actionEnPlus);
                }


                cetteEtape.append(sEtapePart2.arg(QString::number(indexItem)));

                flux2 << cetteEtape << endl;
                Etape1=indexItem;
                Etape2=-1;

                flux2 << cetteTransition << endl;

                bAttenteTransition=true;

                //  TODO: enrichir l'étape avec une action

            }
            if((typeLigne.compare("Transition")==0) && (bAttenteTransition))
            {

                //on cherche l'index du déplacement qui suit
                int indexCurrent=indexItem;
                //on s'assure que la suite est bien décrite, sinon on n'écrit pas la transition
                if(indexCurrent==tableWidget_SM->rowCount()-1)
                {
                    qDebug() << "pas d'action après la dernière transition, elle ne sera pas prise en compte!";
                }
                else
                {
                    int j=indexCurrent;
                    QString nextTypeLigne;
                    do
                    {
                        j++;
                        nextTypeLigne=tableWidget_SM->item(j,0)->text();
                        if(nextTypeLigne.compare("Deplacement")==0)
                            //on enregistre laligne
                            Etape2=j;
                    } while((j<((tableWidget_SM->rowCount())-1))&&(nextTypeLigne.compare("Deplacement")!=0));

                    QString sData=tableWidget_SM->item(indexItem,2)->text();
                    cetteTransition=sTransition.arg(QString::number(Etape1),QString::number(Etape2),sAction,sData);

                    //reinit
                    bAttenteTransition=false;
                    Etape1=Etape2;
                    Etape2=-1;
                }
            }
        }
        flux2 << "\n\tclear all;\n" << endl;
        fichier2.close();
        setNiveauTrace(MSG_TOUS);

        m_application->m_print_view->print(this, "Script de stratégie généré", MSG_INFO_N1);
     }
    else
       qDebug() << "fichier non ouvert";

}

/*!
 * \brief CStrategyDesigner::cellEnteredSlot pour le drag and drop des lignes
 * \param row
 * \param column
 */
void CStrategyDesigner::cellEnteredSlot(int row, int column)
{
  Q_UNUSED(column)
    int colCount=tableWidget_SM->columnCount();

    int rowsel;

    if(tableWidget_SM->currentIndex().row()<row) rowsel=row-1; //down
    else if(tableWidget_SM->currentIndex().row()>row) rowsel=row+1; //up
    else return;

    QList<QTableWidgetItem*> rowItems,rowItems1;
    for (int col = 0; col < colCount; ++col)
    {
        rowItems << tableWidget_SM->takeItem(row, col);
        rowItems1 << tableWidget_SM->takeItem(rowsel, col);
    }

    for (int cola = 0; cola < colCount; ++cola)
    {
        tableWidget_SM->setItem(rowsel, cola, rowItems.at(cola));
        tableWidget_SM->setItem(row, cola, rowItems1.at(cola));
    }

}

/*!
 * \brief CStrategyDesigner::Slot_Play
 */
void CStrategyDesigner::Slot_Play()
{
    bStop=false;
    bPlay=true;
    bResume=false;

    qreal x_sauv=m_application->m_data_center->read("PosX_robot").toDouble();
    qreal y_sauv=m_application->m_data_center->read("PosY_robot").toDouble();
    qreal theta_sauv=m_application->m_data_center->read("PosTeta_robot").toDouble();

    qreal x_new=x_sauv;
    qreal y_new=y_sauv;
    double theta_new=theta_sauv;
    double theta_new_final=theta_sauv;
    double dTheta=0;
    double dX=0.0;
    double dY=0.0;
    double distance_restante=0.0;
    double distance_new=0;
    double m=1;
    double p=0;
    double nb_pas=1;

    float speed=40.0;
    float angleSpeed=1.5;

    bool isPI2=false;

    int indexItem=0;
    bool bAttenteTransition=false;

    QString sAction;
    QString typeLigne;

    for (indexItem=0;indexItem<tableWidget_SM->rowCount();indexItem++){

        sAction=tableWidget_SM->item(indexItem,1)->text();
        typeLigne=tableWidget_SM->item(indexItem,0)->text();

        //on attend en premier lieu un déplacement et on saute les autres (il ne peut y avoir qu'un
        //déplacement par transition)
        //une fois le déplacement enregistré on marque l'attente d'une transition
        if((typeLigne.compare("Deplacement")==0) && (!bAttenteTransition))
        {
            x_sauv=m_application->m_data_center->read("PosX_robot").toDouble();
            y_sauv=m_application->m_data_center->read("PosY_robot").toDouble();
            theta_sauv=m_application->m_data_center->read("PosTeta_robot").toDouble();

            if(sAction.compare("Mouvement_XY")==0)
            {
                x_new=tableWidget_SM->item(indexItem,2)->text().toDouble();
                y_new=tableWidget_SM->item(indexItem,3)->text().toDouble();
            }

            if(sAction.compare("Mouvement_XYTeta")==0)
            {
                x_new=tableWidget_SM->item(indexItem,2)->text().toDouble();
                y_new=tableWidget_SM->item(indexItem,3)->text().toDouble();
                theta_new_final=tableWidget_SM->item(indexItem,4)->text().toDouble();
            }

//            if(sAction.compare("Mouvement_Manuel")==0)
//            {
//                x_new=tableWidget_SM->item(indexItem,2)->text().toDouble();
//                y_new=tableWidget_SM->item(indexItem,3)->text().toDouble();
//            }


            if((sAction.compare("Mouvement_XY")==0) || (sAction.compare("Mouvement_XYTeta")==0))
            {
                //gestion du PI/2
                if(x_new==x_sauv)
                {
                    m=0;
                    p=0;
                    isPI2=true;
                }
                else
                {
                    m=((y_new-y_sauv)/(x_new-x_sauv));
                    p=(y_new-m*x_new);
                    isPI2=false;
                }

                if(isPI2)
                    theta_new=copysign(1.0,(y_new-y_sauv))*1.57;
                else
                    theta_new=atan(m);
                theta_new_final=theta_new;



                //correction de l'angle
                nb_pas=((theta_new-theta_sauv)/angleSpeed)/0.025;
                dTheta=(theta_new-theta_sauv)/nb_pas;
                for (int j=1;j<nb_pas;j++)
                {
                    m_application->m_data_center->write("PosTeta_robot", theta_sauv+j*dTheta);
                    if (!bResume)
                        QTest::qWait(25);
                    else
                        while(bResume)
                            QTest::qWait(25);

                    if (bStop)
                        return;

                }
                m_application->m_data_center->write("PosTeta_robot", theta_new);

                //déplacement linéaire
                distance_restante=sqrt(pow((x_new-x_sauv),2)+pow((y_new-y_sauv),2));
                nb_pas=(distance_restante/speed)/0.025;
                dX=(x_new-x_sauv)/nb_pas;
                dY=(y_new-y_sauv)/nb_pas;
                for (int i=1;i<nb_pas;i++)
                {
                    if(isPI2)
                    {
                        m_application->m_data_center->write("PosY_robot", y_sauv+i*dY);
                    }
                    else
                    {
                        m_application->m_data_center->write("PosX_robot", x_sauv+i*dX);
                        m_application->m_data_center->write("PosY_robot", (m*(x_sauv+i*dX)+p));
                    }
                    if (!bResume)
                        QTest::qWait(25);
                    else
                        while(bResume)
                            QTest::qWait(25);

                    if (bStop)
                        return;
                }
                m_application->m_data_center->write("PosX_robot", x_new);
                m_application->m_data_center->write("PosY_robot", y_new);

                if(sAction.compare("Mouvement_XYTeta")==0)
                    theta_new_final=tableWidget_SM->item(indexItem,4)->text().toDouble();

                //correction de l'angle final
                if(theta_new_final!=theta_new)
                {
                    nb_pas=((theta_new_final-theta_new)/angleSpeed)/0.025;
                    dTheta=(theta_new_final-theta_new)/nb_pas;
                    for (int k=1;k<nb_pas;k++)
                    {
                        m_application->m_data_center->write("PosTeta_robot", theta_new+k*dTheta);
                        if (!bResume)
                            QTest::qWait(25);
                        else
                            while(bResume)
                                QTest::qWait(25);

                        if (bStop)
                            return;
                    }
                    m_application->m_data_center->write("PosTeta_robot", theta_new_final);
                }
            }

            if((sAction.compare("Mouvement_DistanceAngle")==0) || (sAction.compare("Mouvement_Angle")==0) || (sAction.compare("Mouvement_Distance")==0))
            {
                distance_new=0.0;
                //on détermine theta et distance
                if(sAction.compare("Mouvement_Distance")==0)
                {
                    theta_new=theta_sauv;
                    distance_new=tableWidget_SM->item(indexItem,2)->text().toDouble();
                }
                if(sAction.compare("Mouvement_DistanceAngle")==0)
                {
                    distance_new=tableWidget_SM->item(indexItem,2)->text().toDouble();
                    theta_new=tableWidget_SM->item(indexItem,3)->text().toDouble();
                }
                if (sAction.compare("Mouvement_Angle")==0)
                    theta_new=tableWidget_SM->item(indexItem,2)->text().toDouble();

                //correction de l'angle
                if(theta_new!=theta_sauv)
                {
                    nb_pas=((theta_new-theta_sauv)/angleSpeed)/0.025;
                    dTheta=(theta_new-theta_sauv)/nb_pas;
                    for (int l=1;l<nb_pas;l++)
                    {
                        m_application->m_data_center->write("PosTeta_robot", theta_sauv+l*dTheta);
                        if (!bResume)
                            QTest::qWait(25);
                        else
                            while(bResume)
                                QTest::qWait(25);

                        if (bStop)
                            return;

                    }
                    m_application->m_data_center->write("PosTeta_robot", theta_new);
                }

                //déplacement linéaire
                nb_pas=fabs(distance_new/speed)/0.025;
                //gestion du PI/2
//                if(fabs(theta_new)==1.57)
//                {
//                    dX=0;
//                    dY=copysign(1.0,theta_new)*distance_new/nb_pas;
//                }
//                else
//                {
                    dX=distance_new*cos(theta_new)/nb_pas;
                    dY=distance_new*sin(theta_new)/nb_pas;
//                }

                for (int n=1;n<nb_pas;n++)
                {
                        m_application->m_data_center->write("PosX_robot", x_sauv+n*dX);
                        m_application->m_data_center->write("PosY_robot", y_sauv+n*dY);

                    if (!bResume)
                        QTest::qWait(25);
                    else
                        while(bResume)
                            QTest::qWait(25);

                    if (bStop)
                        return;
                }
                m_application->m_data_center->write("PosX_robot", x_sauv+distance_new*cos(theta_new));
                m_application->m_data_center->write("PosY_robot", y_sauv+distance_new*sin(theta_new));

            }

            bAttenteTransition=true;
        }
        if((typeLigne.compare("Transition")==0) && (bAttenteTransition))
                bAttenteTransition=false;
    }
}

void CStrategyDesigner::Slot_Resume()
{
    bResume=!bResume;
}

void CStrategyDesigner::Slot_Stop()
{
    bStop=true;
    bPlay=false;
    bResume=false;
}

void CStrategyDesigner::Slot_Save()
{
    int indexItem=0;

    // Création d'un objet XML
    QString ficName("savedStrategie_");
    QString temps2=QTime::currentTime().toString("hh_mm");
    QString temps1 = QDate::currentDate().toString("yyyy_MM_dd_at_");
    ficName.append(temps1);
    ficName.append(temps2);
    ficName.append(".xml");
    //qDebug() << ficName;

    QDomDocument doc("strategie_xml");
    // root node
    QDomElement strategieNode = doc.createElement("strategie");
    doc.appendChild(strategieNode);

    //commentaires
    QDomElement commentsNode = doc.createElement("commentaires");
    QTextEdit *textEdit_comment=m_ihm.findChild<QTextEdit*>("textEdit_comment");
    commentsNode.appendChild(doc.createTextNode(textEdit_comment->toPlainText()));
    strategieNode.appendChild(commentsNode);

    for (indexItem=0;indexItem<tableWidget_SM->rowCount();indexItem++)
    {
        QDomElement ligneNode = doc.createElement("ligne");
        ligneNode.setAttribute("type",tableWidget_SM->item(indexItem,0)->text());
        ligneNode.setAttribute("macro",tableWidget_SM->item(indexItem,1)->text());
        ligneNode.setAttribute("data1",tableWidget_SM->item(indexItem,2)->text());
        ligneNode.setAttribute("data2",tableWidget_SM->item(indexItem,3)->text());
        ligneNode.setAttribute("data3",tableWidget_SM->item(indexItem,4)->text());
        ligneNode.setAttribute("commentaires",tableWidget_SM->item(indexItem,5)->text());
        strategieNode.appendChild(ligneNode);
    }

    QFile fichier(ficName);
    if(fichier.open(QIODevice::ReadWrite | QIODevice::Text))
    {
        QTextStream stream(&fichier);
        QString contenuXML=doc.toString();
        stream << contenuXML;
        fichier.close();
    }
    else
        fichier.close();
}

void CStrategyDesigner::Slot_Load()
{
    int indexItem=0;
    int nb_lignes=tableWidget_SM->rowCount();
    for (indexItem=nb_lignes-1;indexItem>=0;indexItem--)
        tableWidget_SM->removeRow(indexItem);
    QString caption("Open XML Strategie File");
    QString filter("XML Files (*.xml)");
    QString fileName = QFileDialog::getOpenFileName(&m_ihm,caption, "",filter);
    indexItem=0;
    if(fileName.isEmpty()==false)
    {
        QFile fichier(fileName);
        if(fichier.open(QIODevice::ReadOnly | QIODevice::Text))
        {
            QDomDocument doc("strategie_xml");
            if (doc.setContent(&fichier)) {
                QDomElement docElem = doc.documentElement();
                QDomNode n = docElem.firstChild();
                while(!n.isNull()) {
                    QDomElement e = n.toElement();
                    if(!e.isNull())
                    {
                        //qDebug() << e.tagName();
                        if(e.tagName().compare("commentaires")==0)
                        {
                            QTextEdit *textEdit_comment=m_ihm.findChild<QTextEdit*>("textEdit_comment");
                            textEdit_comment->setPlainText(e.text());

                        }

                        if(e.tagName().compare("ligne")==0)
                        {
                            QTableWidgetItem *newItem_Type = new QTableWidgetItem(e.attribute("type","Deplacement"));
                            QTableWidgetItem *newItem = new QTableWidgetItem(e.attribute("macro","Mouvement_XY"));
                            QTableWidgetItem *newItem_Data1 = new QTableWidgetItem(e.attribute("data1","0"));
                            QTableWidgetItem *newItem_Data2 = new QTableWidgetItem(e.attribute("data2","0"));
                            QTableWidgetItem *newItem_Data3 = new QTableWidgetItem(e.attribute("data3",""));
                            QTableWidgetItem *newItem_Comments = new QTableWidgetItem(e.attribute("commentaires",""));
                            tableWidget_SM->insertRow(indexItem);
                            tableWidget_SM->setItem(indexItem, 0, newItem_Type);
                            tableWidget_SM->setItem(indexItem, 1, newItem);
                            tableWidget_SM->setItem(indexItem, 2, newItem_Data1);
                            tableWidget_SM->setItem(indexItem, 3, newItem_Data2);
                            tableWidget_SM->setItem(indexItem, 4, newItem_Data3);
                            tableWidget_SM->setItem(indexItem, 5, newItem_Comments);
                            indexItem++;
                        }
                    }
                    n = n.nextSibling();
                }
            }
            fichier.close();
        }
        else
            fichier.close();
    }
}

/*!
 * \brief CStrategyDesigner::Slot_Remove Retire la ligne sélectionnée du tableau de la stratégie. Si plusieurs
 * lignes sont sélectionnées, seule la première est retirée.
 */
void CStrategyDesigner::Slot_Remove()
{
    QList<QTableWidgetItem*> rowItems;

    //récupère toutes les cases sélectionnées du tableau
    rowItems=tableWidget_SM->selectedItems();

    int selectedRow=0;

    //s'il y a bien des cases sélectionnées
    if(rowItems.size()>0)
    {
        //on prend l'indice de ligne de la première case sélectionnée
        selectedRow=rowItems.at(0)->row();
        //et on la supprime
        tableWidget_SM->removeRow(selectedRow);
    }
}

void CStrategyDesigner::Slot_ChoseAction(QString actionChoisie)
{
    QComboBox *comboBox_Action_name=m_ihm.findChild<QComboBox*>("comboBox_Action_name");
    QComboBox *comboBox_Action_value=m_ihm.findChild<QComboBox*>("comboBox_Action_value");

    if(actionChoisie.compare("PositionServo")==0)
    {
        for(int i=comboBox_Action_name->count()-1;i>=0;i--)
            comboBox_Action_name->removeItem(i);
        comboBox_Action_name->addItems(servoName);
        for(int j=comboBox_Action_value->count()-1;j>=0;j--)
            comboBox_Action_value->removeItem(j);
        comboBox_Action_value->addItems(servoValue);
    }
    if(actionChoisie.compare("CommandeMoteur")==0)
    {
        for(int k=comboBox_Action_name->count()-1;k>=0;k--)
            comboBox_Action_name->removeItem(k);
        comboBox_Action_name->addItems(moteurName);
        for(int l=comboBox_Action_value->count()-1;l>=0;l--)
            comboBox_Action_value->removeItem(l);
        comboBox_Action_value->addItems(moteurValue);
    }


}

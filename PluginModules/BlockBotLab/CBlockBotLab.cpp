/*! \file CBlockBotLab.cpp
 * A brief file description CPP.
 * A more elaborated file description.
 */
#include <signal.h>
#include <unistd.h>
#include <QDebug>
#include <QProcessEnvironment>
#include <QDir>
#include "CBlockBotLab.h"
#include "CApplication.h"
#include "CPrintView.h"
#include "CMainWindow.h"
#include "CEEPROM.h"
#include "CDataManager.h"


/*! \addtogroup Module_Test2
   * 
   *  @{
   */
	   
// _____________________________________________________________________
/*!
*  Constructeur
*
*/
CBlockBotLab::CBlockBotLab(const char *plugin_name)
    :CPluginModule(plugin_name, VERSION_BlockBotLab, AUTEUR_BlockBotLab, INFO_BlockBotLab)
{
}


// _____________________________________________________________________
/*!
*  Destructeur
*
*/
CBlockBotLab::~CBlockBotLab()
{
    //fermeture de blockbot (qui est un serveur webpack démarré par QProcess)
    if (blockbotProcess && blockbotProcess->state() != QProcess::NotRunning) {
        // Tuer tout le groupe de process pour éviter les processus fantome
        pid_t pid = blockbotProcess->processId();
        if (pid > 0) {
            ::kill(-pid, SIGTERM); // envoie SIGTERM à tout le groupe
        }
        blockbotProcess->waitForFinished(3000);
    }
}


// _____________________________________________________________________
/*!
*  Initialisation du module
*
*/
void CBlockBotLab::init(CApplication *application)
{
  CModule::init(application);
  setGUI(&m_ihm); // indique à la classe de base l'IHM

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
  // Restore le niveau d'affichage
  val = m_application->m_eeprom->read(getName(), "niveau_trace", QVariant(MSG_TOUS));
  setNiveauTrace(val.toUInt());
  // Restore la couleur de fond
  val = m_application->m_eeprom->read(getName(), "background_color", QVariant(DEFAULT_MODULE_COLOR));
  setBackgroundColor(val.value<QColor>());

  blockbotWebView=m_ihm.ui.ui_webView;

  httpServer = new RoboticsHttpServer(this);
  httpServer->listen(3001); // Démarre le serveur web Qt sur le port 3001

  //chemin où trouver blockbot - plus tard à mettre en .ini ou alors inclure directement blockbot
  //dans l'arborescence LaBotBox
  blockbotPath="/home/crlg/workspace_robot_sans_code/BlockBot";

  //port normalement par défaut de webpack
  blockbotPort="8080";

  //variable d'état de fonctionnement de Blockly
  blockbotStarted=false;
  blockbotBuilt=false;

  //Démarrer Blockly
  startBlockBot();

}


// _____________________________________________________________________
/*!
*  Fermeture du module
*
*/
void CBlockBotLab::close(void)
{
  // Mémorise en EEPROM l'état de la fenêtre
  m_application->m_eeprom->write(getName(), "geometry", QVariant(m_ihm.geometry()));
  m_application->m_eeprom->write(getName(), "visible", QVariant(m_ihm.isVisible()));
  m_application->m_eeprom->write(getName(), "niveau_trace", QVariant((unsigned int)getNiveauTrace()));
  m_application->m_eeprom->write(getName(), "background_color", QVariant(getBackgroundColor()));
}

// _____________________________________________________________________
/*!
*  Création des menus sur clic droit sur la fenêtre du module
*
*/
void CBlockBotLab::onRightClicGUI(QPoint pos)
{
  QMenu *menu = new QMenu();

  menu->addAction("Select background color", this, SLOT(selectBackgroundColor()));
  menu->exec(m_ihm.mapToGlobal(pos));
}


// _____________________________________________________________________
/**
 * @brief Fonction de lancement de BlockBot
 *
 * Cette fonction paramètre un QProcess pour lancer un bash (non interactif) qui lui même lancera BlockBot.
 * Le bash est utilisé pour éviter que Node.js (npm) se mélange les pinceaux entre les binaires et lib d'Ubuntu (obsolètes)
 * et ceux gérés par nvm (disponible pour l'utilisateur).
 * Si le démarrage de BlockBot réussi, la fonction appelle la fonction d'affichage de BlockBot dans l'interface graphique
 */
void CBlockBotLab::startBlockBot() {
    //processus qui va accueillir le bash chargé de démarrer blockbot
    blockbotProcess = new QProcess(this);

    //commande de mise en place de nvm dans le bash qu'on va appeler
    const QString nvmInit = "export NVM_DIR=\"$HOME/.nvm\"; . \"$NVM_DIR/nvm.sh\"";
    //complément de commande (avec la mise en place de nvm) pour lancer blockbot (avec npm start) dans le bash qu'on va appeler
    const QString cmd = " exec setsid npm start";

    //répertoire de travail de blockbot (cf le constructeur ou l'init)
    blockbotProcess->setWorkingDirectory(blockbotPath);

    //Etant donné qu'on va appeler un bash, on s'assure de bien configurer l'environnement en rapatriant les variables d'environnement
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    env.insert("HOME", QDir::homePath());
    blockbotProcess->setProcessEnvironment(env);

    //On regourpe tous les processus enfants avec le processus père, comme cela quand on ferme le processus père
    //linux ferme tous les autres automatiquement évitant, tant que faire se peut, les processus zombie
    blockbotProcess->setProcessChannelMode(QProcess::MergedChannels);

    //trouver les infos utiles (port, bonne compilation,...) dans la sortie standard du processus
    //l'info utile peut être dans stderr ou stdout suivant les cas

    //Gérer les messages de la sortie standard STDERR
    connect(blockbotProcess, &QProcess::readyReadStandardError, [this]() {
        //lecture de la sortie standard
        QByteArray data = blockbotProcess->readAllStandardError();
        QString output = QString::fromUtf8(data);
        // pour le debug, affichage de la sortie standard
        qDebug() << "STDERR:" << output;

        //Si BlockBot est mal fermé, le serveur webpack devient un process zombie et bloque son port
        //Pas de panique, webpack à la prochaine instance repère le port bloqué et incrémente ce numéro de 1 pour avoir un nouveau port libre
        // bien entendu il faut un moyen pour repĉher l'info de ce port dans la sortie standard pour être certain
        //d'afficher blockbot à coup sûr

        //expression régulière pour choper l'info dans la sortie standard
        QRegularExpression portRegex("Loopback: http://localhost:(\\d+)/");
        QRegularExpressionMatch match = portRegex.match(output);
        if (match.hasMatch()) {
            blockbotPort = match.captured(1);
            qDebug() << "Port webpack détecté:" << blockbotPort;
        }

        // Détecter que le serveur webpack est démarré
        if (output.contains("Project is running at:")) {
            blockbotStarted = true;
            qDebug() << "Serveur webpack démarré";
        }
    });

    //Gérer les messages de la sortie standard STDOUT
    connect(blockbotProcess, &QProcess::readyReadStandardOutput, [this]() {
        //lecture de la sortie standard
        QByteArray data = blockbotProcess->readAllStandardOutput();
        QString output = QString::fromUtf8(data);
        // pour le debug, affichage de la sortie standard
        qDebug() << "STDOUT:" << output;

        //Si BlockBot est mal fermé, le serveur webpack devient un process zombie et bloque son port
        //Pas de panique, webpack à la prochaine instance repère le port bloqué et incrémente ce numéro de 1 pour avoir un nouveau port libre
        // bien entendu il faut un moyen pour repĉher l'info de ce port dans la sortie standard pour être certain
        //d'afficher blockbot à coup sûr

        //expression régulière pour choper l'info dans la sortie standard
        QRegularExpression portRegex("Loopback: http://localhost:(\\d+)/");
        QRegularExpressionMatch match = portRegex.match(output);
        if (match.hasMatch()) {
            blockbotPort = match.captured(1);
            qDebug() << "Port webpack détecté:" << blockbotPort;
        }

        // Détecter que le serveur webpack est démarré si dans STDOUT
        if (output.contains("Project is running at:")) {
            blockbotStarted = true;
            qDebug() << "Serveur webpack démarré";
        }

        // Détecter la fin de compilation réussie de webpack. Dans ce cas là, tout s'est bien passé (on aura forcément un serveur
        // démarré et un port) et on autorise l'affichage de BlockBot
        if (output.contains("webpack") && output.contains("compiled successfully")) {
            blockbotBuilt = true;
            qDebug() << "Compilation webpack terminée avec succès";

            // Vérifier si on peut charger BlockBot (a priori si c'est compilé on a tout ce qu'il faut)
            if (blockbotStarted && !blockbotPort.isEmpty()) {
                //Petit délai pour s'assurer que le serveur est vraiment prêt ;-)
                QTimer::singleShot(1000, this, &CBlockBotLab::loadBlockbotInWebView);
            }
        }
    });

    //Pour le debug: surveillance des signaux de fin de processus
    /*
    connect(blockbotProcess, &QProcess::errorOccurred, this, [](QProcess::ProcessError error){
        qDebug() << "Process error:" << error;
    });
    connect(blockbotProcess, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, [](int exitCode, QProcess::ExitStatus status){
        qDebug() << "Process finished:" << exitCode << status;
    });
    */


    //On a paramétré le processus, les sorties standard sont outillées pour récupérer les infos utiles,
    //on est donc prêt à démarrer le processus :-)
    qDebug() << "Démarrage de BlockBot (webpack) avec bash et npm...";
    blockbotProcess->start("bash", {"-lc", cmd});


    if(!blockbotProcess->waitForStarted()){
        qCritical() << "Impossible de démarrer BlockBot, vous pouvez décommenter le code de surveillance des sorties standard ou des signaux de fin de process pour plus d'infos";
    }
}

// _____________________________________________________________________
/**
 * @brief Fonction d'affichage de blockblot dans un widget Qt
 *
 * Cette fonction affiche l'instance de webpack dans un QWebEngineView de l'ui.
 * Elle dépend du port trouvé lors de lancement de webpack
 */
void CBlockBotLab::loadBlockbotInWebView() {
    QString url = QString("http://localhost:%1").arg(blockbotPort);
    qDebug() << "Affichage de BlockBot (serveur webpack à l'URL: " << url <<").";
    // Charger la page BlockBot
    blockbotWebView->load(QUrl(url));
}



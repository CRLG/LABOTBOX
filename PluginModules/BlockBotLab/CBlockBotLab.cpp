/*! \file CBlockBotLab.cpp
 * A brief file description CPP.
 * A more elaborated file description.
 */
#include <signal.h>
#include <unistd.h>
#include <QDebug>
#include <QProcessEnvironment>
#include <QDir>
#include <QWebEnginePage>
#include <QWebEngineProfile>
#include "CBlockBotLab.h"
#include "CApplication.h"
#include "CPrintView.h"
#include "CMainWindow.h"
#include "CEEPROM.h"
#include "CDataManager.h"
#include <QJsonDocument>
#include <QJsonArray>
#include <QStringList>
#include <QFileDialog>
#include <QStandardPaths>
#include <QJsonObject>
#include <QLabel>

#include "CActuatorSequencer.h"
#include "CSimuBot.h"
#include "CHILEngine.h"

// Delai laisse a la page BlockBot pour etablir son pont QWebChannel avant de lui pousser l'etat
// initial de l'IHM. Cote JS, la connexion est faite 100 ms apres DOMContentLoaded ; on garde une
// marge confortable, le geste n'etant joue qu'une fois par chargement de page.
static const int BLOCKBOT_WEBCHANNEL_HANDSHAKE_DELAY_MS = 500;

// Developpe un "~" de tete en repertoire home de l'utilisateur courant. Permet de configurer en
// EEPROM un chemin situe dans le home (p.ex. le cache QtWebEngine) sans y ecrire le nom d'un
// utilisateur : la meme configuration fonctionne sur n'importe quel poste. Tout autre chemin
// (absolu ou relatif) est renvoye inchange.
static QString resolveHomePath(const QString& path)
{
    if (path == "~")            return QDir::homePath();
    if (path.startsWith("~/"))  return QDir::cleanPath(QDir::homePath() + path.mid(1));
    return path;
}


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
    if (m_blockbotProcess && m_blockbotProcess->state() != QProcess::NotRunning) {
        // Tuer tout le groupe de process pour éviter les processus fantome
        pid_t pid = m_blockbotProcess->processId();
        if (pid > 0) {
            ::kill(-pid, SIGTERM); // envoie SIGTERM à tout le groupe
        }
        m_blockbotProcess->waitForFinished(3000);
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

  m_blockbotWebView=m_ihm.ui.ui_webView;

  // Le mode restaure depuis l'EEPROM n'etait applique qu'au combo de la barre d'outils : BlockBot,
  // lui, redemarre systematiquement sur son propre mode par defaut (debutant), car aucune commande
  // set_mode ne lui est envoyee au chargement de la page. La configuration affichait donc "expert"
  // alors que l'IHM BlockBot restait en debutant. On rejoue l'etat de l'IHM a chaque fin de
  // chargement de la page (cf. onBlockBotLoaded), en mode developpeur comme en mode industriel.
  connect(m_blockbotWebView, SIGNAL(loadFinished(bool)), this, SLOT(onBlockBotLoaded(bool)));

  m_generated_pathfilename = m_application->m_eeprom->read(getName(), "generated_pathfilename", ".").toString();
  m_launch_and_program_command = m_application->m_eeprom->read(getName(), "launch_and_program_command", "make install -C -j4 /home/crlg/workspace/GROSBOT_STM32/Soft_STM32/CM7/").toString();
  m_config_specifique_coupe_path = m_application->m_eeprom->read(getName(), "config_specifique_coupe_path", "/home/crlg/workspace/GROSBOT_STM32/Soft_STM32/CM7/Includes/ConfigSpecifiqueCoupe.h").toString();

  /*int httpServerPort = m_application->m_eeprom->read(getName(), "http_server_port", 3001).toInt();
  m_httpServer = new RoboticsHttpServer(this);
  if (m_httpServer) {
      m_httpServer->listen(httpServerPort); // Démarre le serveur web Qt sur le port (3001 par défaut)
      connect(m_httpServer, SIGNAL(processData(QString)), this, SLOT(processData(QString)));
  }*/

  //chemin où trouver blockbot
  m_blockbotPath = m_application->m_eeprom->read(getName(), "blockbot_path", "/home/crlg/workspace_robot_sans_code/BlockBot").toString();

  //port normalement par défaut de webpack
  m_blockbotPort=m_application->m_eeprom->read(getName(), "blockbot_port", "8080").toString();

  //mode développeur ou en prod pour BlockBot
  m_blockbotDevMode=m_application->m_eeprom->read(getName(), "blockbot_devmode", false).toBool();

    // Création de la combobox permettant de choisir entre les modes "débutant" et "expert"
    modeChoice=new QComboBox();
    modeChoice->addItem("debutant");
    modeChoice->addItem("expert");
    // Restaure le mode depuis EEPROM avant la connexion du signal pour ne pas déclencher send2BlockBot()
    modeChoice->setCurrentText(m_application->m_eeprom->read(getName(), "blockbot_mode", "debutant").toString());
    modeChoice->setObjectName("setMode");
    modeChoice->setMaximumWidth(150);
    // Ajout dans la toolbar
    m_ihm.ui.toolBar->addWidget(modeChoice);
    // Création de la checkbox permettant de choisir d'afficher ou non le code généré
    QLabel* l_showCode=new QLabel();
    l_showCode->setText("Montrer le code");
    m_ihm.ui.toolBar->addWidget(l_showCode);
    showCode=new QCheckBox();
    showCode->setChecked(false);
    showCode->setObjectName("showCode");
    // Ajout dans la toolbar
    m_ihm.ui.toolBar->addWidget(showCode);


  //variable d'état de fonctionnement de Blockly
  m_blockbotStarted=false;
  m_blockbotBuilt=false;

  // Le process de compilation
  connect(&m_build_target_process, SIGNAL(started()), this, SLOT(buildStarted()));
  connect(&m_build_target_process, SIGNAL(finished(int)), this, SLOT(buildFinished(int)));
  connect(&m_build_target_process, SIGNAL(readyReadStandardOutput()), this, SLOT(buildOutput()));
  connect(&m_build_target_process, SIGNAL(readyReadStandardError()), this, SLOT(buildError()));
  connect(m_ihm.ui.actionafficheBuildLog, SIGNAL(triggered(bool)), this, SLOT(setBuildLogsVisibility(bool)));
  connect(m_ihm.ui.actionCompilAndDownload, SIGNAL(triggered(bool)), this, SLOT(buildTargetAndUpload()));
  connect(&m_timer_close_build_logs_delayed, SIGNAL(timeout()), this, SLOT(closeBuildLogs()));
  setBuildLogsVisibility(false);

  m_timer_close_build_logs_delayed.setInterval(2000);

  // Nettoie le cache
  // "~/..." developpe en repertoire home de l'utilisateur courant : la configuration livree ne
  // contient donc aucun nom d'utilisateur et fonctionne sur n'importe quel poste.
  QString clean_cache_directory = resolveHomePath(m_application->m_eeprom->read(getName(), "clean_cache_directory", "").toString());
  if (!clean_cache_directory.trimmed().isEmpty()) {
      QDir dir(clean_cache_directory);
      if (dir.removeRecursively()) {
          m_application->m_print_view->print_debug(this, QString("Clean cache directory: %1").arg(clean_cache_directory));
      }
  }

  //gère la sauvegarde
  connect(m_ihm.ui.ui_webView->page()->profile(),  &QWebEngineProfile::downloadRequested,this, &CBlockBotLab::onDownloadRequested);

  // Configuration du lien entre LaBotBox et BlockBot
    // Créer le canal web et association du pont
    webChannel = new QWebChannel(this);
    webChannel->registerObject("BlockBotLab",this);
    // Associer le canal configuré à l'affichage (navigateur) de BlockBot
    m_blockbotWebView->page()->setWebChannel(webChannel);
    //Connection du bouton avec la commande à envoyer à BlockBot
    connect(m_ihm.ui.actionBlockbotGenerate, SIGNAL(triggered(bool)), this, SLOT(send2BlockBot()));
    connect(modeChoice, SIGNAL(currentTextChanged(QString)),this, SLOT(send2BlockBot()));
    connect(showCode, SIGNAL(stateChanged(int)),this, SLOT(send2BlockBot()));
    connect(m_ihm.ui.actionOpen, SIGNAL(triggered(bool)), this, SLOT(send2BlockBot()));
    connect(m_ihm.ui.actionSave, SIGNAL(triggered(bool)), this, SLOT(send2BlockBot()));
    connect(m_ihm.ui.actionContext, SIGNAL(triggered(bool)), this, SLOT(send2BlockBot()));
    // Double-clic sur la simulation (CSimuBot) → création automatique d'un état XYTheta dans BlockBot
    connect(m_application->m_SimuBot, SIGNAL(setSequence(double, double)), this, SLOT(Slot_SetPosFromSimu(double, double)));

    // ── HIL (Hardware In the Loop) ──────────────────────────────────────
    m_hilEngine = new CHILEngine(this);
    m_hilEngine->setApplication(m_application);

    // Connexion des boutons toolbar HIL aux slots
    connect(m_ihm.ui.actionPlayHIL, SIGNAL(triggered(bool)), this, SLOT(Slot_PlayHIL()));
    connect(m_ihm.ui.actionStopHIL, SIGNAL(triggered(bool)), this, SLOT(Slot_StopHIL()));
    connect(m_ihm.ui.actionPlayOnlyOneHIL, SIGNAL(triggered(bool)), this, SLOT(Slot_PlaySingleActionHIL()));
    connect(m_ihm.ui.actionMergeProject, SIGNAL(triggered(bool)), this, SLOT(Slot_MergeProject()));

    // Quand CHILEngine demande un état → on le demande à BlockBot via QWebChannel
    connect(m_hilEngine, &CHILEngine::requestState, this, [this](const QString &nomEtat) {
        emit executeCommand("get_hil_state", nomEtat);
    });

    // Quand CHILEngine change d'état → surlignage dans BlockBot
    connect(m_hilEngine, &CHILEngine::stateChanged, this, [this](const QString &nomEtat) {
        emit executeCommand("highlight_hil_state", nomEtat);
    });

    // Quand CHILEngine termine → effacer le surlignage
    connect(m_hilEngine, &CHILEngine::hilFinished, this, [this]() {
        emit executeCommand("clear_hil_highlight", "");
        // Etape 6 (correctif) : fin/arret HIL (stop() est appele sur Stop utilisateur ET sur
        // FIN_MISSION, tous deux emettant hilFinished). En VISU/SIMU sans robot, on fige aussi
        // l'asserv de la sim interne, sinon le sprite continue vers sa derniere cible non atteinte.
        if (m_application->m_SimuBot) m_application->m_SimuBot->stopInternalSim();
    });

    // si_vrai_expert : demande d'init de la condition booléenne côté JS
    connect(m_hilEngine, &CHILEngine::requestLogicInit, this, [this](const QString &blockId) {
        emit executeCommand("hil_logic_init", blockId);
    });

    // si_vrai_expert : envoi du snapshot DataManager pour le tick d'évaluation
    connect(m_hilEngine, &CHILEngine::requestLogicTick, this, [this](const QString &kvMapJson) {
        emit executeCommand("hil_logic_tick", kvMapJson);
    });

    // Messages de log HIL → printView
    connect(m_hilEngine, &CHILEngine::logMessage, this, [this](const QString &msg) {
        m_application->m_print_view->print_info(this, msg);
    });

  //Démarrer Blockly
    if(m_blockbotDevMode)
    {
        qDebug() << "[BlockBotLab] Démarrage de blockbot en mode dev!";
        startBlockBot();
    }
    else
    {
        qDebug() << "[BlockBotLab] Démarrage de blockbot en mode indus!";
        QString appDir = QCoreApplication::applicationDirPath();
        QString filePath = appDir + "/BlockBot_static/index.html";
        m_blockbotWebView->setUrl(QUrl::fromLocalFile(filePath));
    }
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
  m_application->m_eeprom->write(getName(), "config_specifique_coupe_path", QVariant(m_config_specifique_coupe_path));
  m_application->m_eeprom->write(getName(), "blockbot_mode", QVariant(modeChoice->currentText()));
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
    m_blockbotProcess = new QProcess(this);

    //commande de mise en place de nvm dans le bash qu'on va appeler
    const QString nvmInit = "export NVM_DIR=\"$HOME/.nvm\"; . \"$NVM_DIR/nvm.sh\"";
    //complément de commande (avec la mise en place de nvm) pour lancer blockbot (avec npm start) dans le bash qu'on va appeler
    const QString cmd = nvmInit +
            " && nvm use 20"           // adapte la version si besoin
            " && which node && node -v"
            " && which npm && npm -v"
            " && exec setsid npm start";

    //répertoire de travail de blockbot (cf le constructeur ou l'init)
    m_blockbotProcess->setWorkingDirectory(m_blockbotPath);

    //Etant donné qu'on va appeler un bash, on s'assure de bien configurer l'environnement en rapatriant les variables d'environnement
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    env.insert("HOME", QDir::homePath());
    m_blockbotProcess->setProcessEnvironment(env);

    //On regourpe tous les processus enfants avec le processus père, comme cela quand on ferme le processus père
    //linux ferme tous les autres automatiquement évitant, tant que faire se peut, les processus zombie
    m_blockbotProcess->setProcessChannelMode(QProcess::MergedChannels);

    //trouver les infos utiles (port, bonne compilation,...) dans la sortie standard du processus
    //l'info utile peut être dans stderr ou stdout suivant les cas

    //Gérer les messages de la sortie standard STDERR
    connect(m_blockbotProcess, &QProcess::readyReadStandardError, [this]() {
        //lecture de la sortie standard
        QByteArray data = m_blockbotProcess->readAllStandardError();
        QString output = QString::fromUtf8(data);
        // pour le debug, affichage de la sortie standard
        if (!output.isEmpty()) m_application->m_print_view->print_error(this, QString("STDERR: %1").arg(output));

        //Si BlockBot est mal fermé, le serveur webpack devient un process zombie et bloque son port
        //Pas de panique, webpack à la prochaine instance repère le port bloqué et incrémente ce numéro de 1 pour avoir un nouveau port libre
        // bien entendu il faut un moyen pour repĉher l'info de ce port dans la sortie standard pour être certain
        //d'afficher blockbot à coup sûr

        //expression régulière pour choper l'info dans la sortie standard
        QRegularExpression portRegex("Loopback: http://localhost:(\\d+)/");
        QRegularExpressionMatch match = portRegex.match(output);
        if (match.hasMatch()) {
            m_blockbotPort = match.captured(1);
            m_application->m_print_view->print_info(this, QString("Port webpack détecté: %1").arg(m_blockbotPort));
        }

        // Détecter que le serveur webpack est démarré
        if (output.contains("Project is running at:")) {
            m_blockbotStarted = true;
            m_application->m_print_view->print_debug(this, "Serveur webpack démarré");
        }
    });

    //Gérer les messages de la sortie standard STDOUT
    connect(m_blockbotProcess, &QProcess::readyReadStandardOutput, [this]() {
        //lecture de la sortie standard
        QByteArray data = m_blockbotProcess->readAllStandardOutput();
        QString output = QString::fromUtf8(data);
        // pour le debug, affichage de la sortie standard
        if (!output.isEmpty()) m_application->m_print_view->print_debug(this, QString("STDOUT: %1").arg(output));

        //Si BlockBot est mal fermé, le serveur webpack devient un process zombie et bloque son port
        //Pas de panique, webpack à la prochaine instance repère le port bloqué et incrémente ce numéro de 1 pour avoir un nouveau port libre
        // bien entendu il faut un moyen pour repĉher l'info de ce port dans la sortie standard pour être certain
        //d'afficher blockbot à coup sûr

        //expression régulière pour choper l'info dans la sortie standard
        QRegularExpression portRegex("Loopback: http://localhost:(\\d+)/");
        QRegularExpressionMatch match = portRegex.match(output);
        if (match.hasMatch()) {
            m_blockbotPort = match.captured(1);
            m_application->m_print_view->print_info(this, QString("Port webpack détecté: %1").arg(m_blockbotPort));
        }

        // Détecter que le serveur webpack est démarré si dans STDOUT
        if (output.contains("Project is running at:")) {
            m_blockbotStarted = true;
            m_application->m_print_view->print_debug(this, "Serveur webpack démarré");
        }

        // Détecter la fin de compilation réussie de webpack. Dans ce cas là, tout s'est bien passé (on aura forcément un serveur
        // démarré et un port) et on autorise l'affichage de BlockBot
        if (output.contains("webpack") && output.contains("compiled successfully")) {
            m_blockbotBuilt = true;
            m_application->m_print_view->print_debug(this, "Compilation webpack terminée avec succès");

            // Vérifier si on peut charger BlockBot (a priori si c'est compilé on a tout ce qu'il faut)
            if (m_blockbotStarted && !m_blockbotPort.isEmpty()) {
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
    m_application->m_print_view->print_debug(this, "Démarrage de BlockBot (webpack) avec bash et npm...");

    m_blockbotProcess->start("bash", {"-lc", cmd});


    if(!m_blockbotProcess->waitForStarted()){
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
    QString url = QString("http://localhost:%1").arg(m_blockbotPort);
    m_application->m_print_view->print_debug(this, QString("Affichage de BlockBot (serveur webpack à l'URL: %1).").arg(url));

    // Charger la page BlockBot
    m_blockbotWebView->load(QUrl(url));
}

// _____________________________________________________________________
/*!
 * \brief fin de chargement de la page BlockBot : lui reapplique l'etat de l'IHM LaBotBox
 *
 * BlockBot repart de ses propres valeurs par defaut a chaque chargement de page. Tout etat
 * restaure cote LaBotBox (ici le mode debutant/expert lu en EEPROM) doit donc lui etre pousse
 * explicitement, sinon les deux divergent silencieusement.
 *
 * Le delai est necessaire : cote JS, le pont QWebChannel n'est etabli que 100 ms apres
 * DOMContentLoaded (temporisation de index.js), donc apres ce signal. Une commande emise avant
 * cet etablissement n'a aucun abonne et est purement et simplement perdue.
 */
void CBlockBotLab::onBlockBotLoaded(bool ok)
{
    if (!ok) {
        qWarning() << "[BlockBotLab] chargement de la page BlockBot en echec : mode non applique";
        return;
    }
    QTimer::singleShot(BLOCKBOT_WEBCHANNEL_HANDSHAKE_DELAY_MS, this, SLOT(pushCurrentModeToBlockBot()));
}

// _____________________________________________________________________
/*!
 * \brief pousse vers BlockBot le mode courant du combo (debutant / expert)
 *
 * Meme commande que celle emise quand l'utilisateur change le combo a la main (cf. send2BlockBot,
 * objet "setMode") : un seul chemin de bascule de mode, donc pas de comportement divergent entre
 * le mode initial et un changement en cours de session.
 */
void CBlockBotLab::pushCurrentModeToBlockBot()
{
    if (!modeChoice) return;
    emit executeCommand("set_mode", modeChoice->currentText());
    // Meme trace que la bascule manuelle (send2BlockBot), pour pouvoir distinguer dans les logs
    // le mode applique au chargement de celui choisi ensuite a la main.
    qDebug() << "[BlockBotLab] set_mode au chargement de la page :" << modeChoice->currentText();
    m_application->m_print_view->print_debug(this, QString("Mode BlockBot applique au demarrage : %1").arg(modeChoice->currentText()));
}


// _____________________________________________________________________
/**
 * @brief Traite les données envoyées par BlockBot
 * @param code Code généré complet par BlockBot
 * @return true si la sauvegarde a réussi, false en cas d'erreur
 *
 * Pour l'instant sauvegarde le code dans "programmeBlockBot.cpp" et crée une copie
 * avec timestamp pour l'historique (à fin de debug)
 */
bool CBlockBotLab::processData(QString code, QString nomStrategie, QString listeEtatsJSON)
{
    QString timestamp = QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss");
    QString msg;
    qDebug() << "génération des fichiers";
    qDebug() << modeChoice->currentText();

    if(modeChoice->currentText()=="expert")
    {
        // Parser le JSON pour récupérer la liste des états
        QJsonDocument jsonDoc = QJsonDocument::fromJson(listeEtatsJSON.toUtf8());
        QStringList listeEtats;
        if (jsonDoc.isArray()) {
            QJsonArray jsonArray = jsonDoc.array();
            for (int i = 0; i < jsonArray.size(); i++)
            {
                listeEtats.append(jsonArray[i].toString());
            }
        }

        /**
         * Vérification des doublons des noms dans les états
         * on prévient l'utilisateur dans ce cas
         */

        // Vérification des doublons de noms d'états
        // Détecter les doublons
        QStringList doublons;
        QStringList vus;
        for (const QString& nom : listeEtats) {
            if (vus.contains(nom)) {
                if (!doublons.contains(nom))
                    doublons.append(nom);
            } else {
                vus.append(nom);
            }
        }

        if (!doublons.isEmpty()) {
            QString msgErreur = QString("⚠️ ERREUR : Les états suivants ont des noms en double :\n  • %1\n\nCorrigez les noms avant de générer le code.")
                                .arg(doublons.join("\n  • "));
            m_application->m_print_view->print_error(this, msgErreur);
            //m_ihm.ui.statusbar->showMessage("Erreur : noms d'états en double détectés !", 6000);
        }

        /**
         * La présence de doublon n'est pas une erreur bloquante, on continue donc les traitements
         */

        //Nom include
        QString nomIncludeSM = "SM_"+nomStrategie.toUpper()+"_H";
        //Nom du fichier
        QString nomFicSM = "sm_"+nomStrategie.toLower();
        //Nom classe
        QString nomClasse = "SM_" + nomStrategie;
        //List états vers noms et définition
        QString listeEtatsNoms;
        QString listeEtatsDef;
        if(!listeEtats.isEmpty())
        {
            for(int i=0; i<listeEtats.size();i++)
            {
                //liste des états pour l'entete
                if(i==0)
                    listeEtatsDef.append("\t"+listeEtats.at(i)+" = SM_StateMachineBase::SM_FIRST_STATE,\n");
                else
                    listeEtatsDef.append("\t"+listeEtats.at(i)+",\n");
                //liste des états pour le cpp
                listeEtatsNoms.append("\t\tcase "+listeEtats.at(i)+" :\treturn \""+listeEtats.at(i)+"\";\n");
            }
        }

        //------------------------------------------------------
        // Génération de l'entete
        //------------------------------------------------------
        // Le fichier .h template est embarqué dans l'exécutable Labotbox.
        //Ouvre le template et remplace les balises pour générer le code final
        QString file_content_h = readFile(":/templates/Templates/sm_blockly_expert.tpl.h");
        file_content_h.replace("##GENERATED_DATE_TIME##", timestamp);
        file_content_h.replace("##INCLUDE_DEF##",nomIncludeSM);
        file_content_h.replace("##NOM_CLASSE##",nomClasse);
        file_content_h.replace("##LISTE_ETAT_DEF##",listeEtatsDef);

        // Sauvegarder le code généré dans un fichier pour Arduino/STM32
        QString sFicName_h=nomFicSM+".h";
        QString fileNameHGenerated=m_generated_pathfilename+"/"+sFicName_h;
        QFile headerFile(fileNameHGenerated);
        if (!headerFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
            msg = QString("Impossible d'ouvrir le fichier de code en écriture: %1").arg(headerFile.errorString());
            m_application->m_print_view->print_error(this, msg);
            m_ihm.ui.statusbar->showMessage(msg, 4000);
            return false;
        }

        QTextStream codeStream_h(&headerFile);
        codeStream_h << file_content_h;
        headerFile.close();

        qDebug() << "header OK:" << headerFile.fileName();

        //------------------------------------------------------
        // Génération du CPP
        //------------------------------------------------------
        // Le fichier .cpp template est embarqué dans l'exécutable Labotbox.
        //Ouvre le template et remplace les balises pour générer le code final
        //QString file_content = readFile(":/templates/Templates/sm_blockly_debutant.tpl.cpp");
        QString file_content = readFile(":/templates/Templates/sm_blockly_expert.tpl.cpp");
        file_content.replace("##GENERATED_DATE_TIME##", timestamp);
        file_content.replace("##NOM_FIC##",nomFicSM);
        file_content.replace("##NOM_CLASSE##",nomClasse);
        file_content.replace("##LISTE_ETAT_NOM##",listeEtatsNoms);
        file_content.replace("##FUNCTION_STEP##", code);

        // Sauvegarder le code généré dans un fichier pour Arduino/STM32
        //QFile codeFile(m_generated_pathfilename);
        QString sFicName=nomFicSM+".cpp";
        QString fileNameGenerated=m_generated_pathfilename+"/"+sFicName;
        QFile codeFile(fileNameGenerated);
        if (!codeFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
            msg = QString("Impossible d'ouvrir le fichier de code en écriture: %1").arg(codeFile.errorString());
            m_application->m_print_view->print_error(this, msg);
            m_ihm.ui.statusbar->showMessage(msg, 4000);
            return false;
        }

        QTextStream codeStream(&codeFile);
        codeStream << file_content;
        codeFile.close();

         qDebug() << "cpp OK:" << codeFile.fileName();

        msg = QString("Code généré par BlockBot sauvegardé dans: %1").arg(codeFile.fileName());
        m_application->m_print_view->print_debug(this, msg);
        m_ihm.ui.statusbar->showMessage(msg, 4000);
    }
    else if(modeChoice->currentText()=="debutant")
    {
        //------------------------------------------------------
        // Génération du CPP
        //------------------------------------------------------
        // Le fichier .cpp template est embarqué dans l'exécutable Labotbox.
        //Ouvre le template et remplace les balises pour générer le code final
        QString file_content = readFile(":/templates/Templates/sm_blockly_debutant.tpl.cpp");
        file_content.replace("##GENERATED_DATE_TIME##", timestamp);
        file_content.replace("##FUNCTION_STEP##", code);

        QString fileNameGenerated=m_generated_pathfilename+"/"+"sm_blockly_debutant.cpp";

        // Sauvegarder le code généré dans un fichier pour Arduino/STM32
        QFile codeFile(fileNameGenerated);
        if (!codeFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
            msg = QString("Impossible d'ouvrir le fichier de code en écriture: %1").arg(codeFile.errorString());
            m_application->m_print_view->print_error(this, msg);
            m_ihm.ui.statusbar->showMessage(msg, 4000);
            return false;
        }

        QTextStream codeStream(&codeFile);
        codeStream << file_content;
        codeFile.close();

        msg = QString("Code généré par BlockBot sauvegardé dans: %1").arg(codeFile.fileName());
        m_application->m_print_view->print_debug(this, msg);
        m_ihm.ui.statusbar->showMessage(msg, 4000);

        // Optionnel: sauvegarde avec un timestamp pour l'historique car à chaque fois on écrase le fichier
        //même mécanisme que CActuatorSequencer
        QString timestampFilename = QString("programmeBlockBot%1.cpp")
                                   .arg(timestamp);
        QFile timestampFile(timestampFilename);
        if (timestampFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream timestampStream(&timestampFile);
            timestampStream << code;
            timestampFile.close();
            msg = QString("Copie sauvegardée avec timestamp:: %1").arg(timestampFilename);
            m_application->m_print_view->print_debug(this, msg);
        }

    }
    // Lance la compilation et le téléchargement de la ciblee
    buildTargetAndUpload();

    return true;
}

// _____________________________________________________________________
/*!
*  Lit le contenu d'un fichier
*
*/
QString CBlockBotLab::readFile(QString pathfilename)
{
  pathfilename.replace("//", "/");
  QFile file(pathfilename);
  if (file.open(QFile::ReadOnly) == false) {
      m_application->m_print_view->print_error(this, "Impossible d'ouvrir le fichier : " + pathfilename);
      return "";
  }
  QTextStream txtstream(&file);
  txtstream.setCodec("UTF-8");
  QString contenu = txtstream.readAll();
  file.close();
  return(contenu);
}


// ===================================================================
//                GESTION DU PROCESS DE COMPILATION - TELECHARGEMENT
// ===================================================================
// _____________________________________________________________________
/*!
 * \brief compile le code généré et lance la programmation de la cible
 * \return
 */
bool CBlockBotLab::buildTargetAndUpload()
{
    if (m_launch_and_program_command.isEmpty()) return false;
    m_build_target_process.start(m_launch_and_program_command);
    return true;
}

// _____________________________________________________________________
void CBlockBotLab::buildStarted()
{
    qDebug() << "CBlockBotLab::buildStarted";
    m_ihm.ui.build_logs->clear();
    m_ihm.ui.build_logs->setVisible(true);
    m_ihm.ui.actionafficheBuildLog->setChecked(true);
}

// _____________________________________________________________________
void CBlockBotLab::buildFinished(int exitcode)
{
    qDebug() << "CBlockBotLab::buildFinished: " << exitcode;
    m_timer_close_build_logs_delayed.start();  // décale la fermeture de la fenêtre pour laisser le temps de voir les derniers messages
}

// _____________________________________________________________________
void CBlockBotLab::buildError()
{
    QString txt = m_build_target_process.readAllStandardError();
    m_ihm.ui.build_logs->appendPlainText(txt);
}

// _____________________________________________________________________
void CBlockBotLab::buildOutput()
{
    QString txt = m_build_target_process.readAllStandardOutput();
    m_ihm.ui.build_logs->appendPlainText(txt);
}

// _____________________________________________________________________
void CBlockBotLab::setBuildLogsVisibility(bool visible)
{
    m_ihm.ui.build_logs->setVisible(visible);
    m_ihm.ui.actionafficheBuildLog->setChecked(visible);
    m_timer_close_build_logs_delayed.stop();
}

// _____________________________________________________________________
void CBlockBotLab::closeBuildLogs()
{
    setBuildLogsVisibility(false);
}

// _____________________________________________________________________
void CBlockBotLab::send2BlockBot()
{
    //récupération du pointeur de l'action ayant envoyé son signal
    QObject *obj = sender();
    if (!obj)
        return;
    //si l'action existe, choix de la fonctionnalité à activer côté blockly

    //Action de bascule entre les modes expert et débutant
    //pas de retour capturé par un slot, tout se passe ensuite dans blockly (masquage de la toolbox idoine)
    if (obj->objectName()=="setMode")
    {
        emit executeCommand("set_mode",modeChoice->currentText());
        qDebug() << "set_mode : " <<modeChoice->currentText();
    }
    //Action de montrer ou non le code généré dans BlockBot
    if (obj->objectName()=="showCode")
    {
        emit executeCommand("show_code",(showCode->isChecked()?"true":"false"));
    }
    //Action de génération des fichiers modélia à partir du code généré par blockly
    //cf slot processData
    if (obj->objectName()=="actionBlockbotGenerate")
    {
        emit executeCommand("upload_code","no_param");
    }
    //Action de sauvegarde de workspace (lance une logique de téléchargement dans blockly récupéré par Qt
    //cf slot onDownloadRequested
    if (obj->objectName()=="actionSave")
    {
        emit executeCommand("save_project","no_param");
    }
    //Action de restauration du workspace, blockly ayant une logique "navigateur", il ne peut pas ouvrir
    //de fichier directement, donc LaBotBox doit le faire à sa place et lui envoyer le workspace désérialisé à restaurer
    if (obj->objectName() == "actionOpen")
    {
        QString fileName = QFileDialog::getOpenFileName(
                m_ihm.ui.centralwidget,
                tr("Ouvrir un workspace"),
                QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation),
                tr("Fichier Blockly (*.json)")
            );

        if(fileName.isEmpty())
            return;

        QFile file(fileName);
        if(file.open(QIODevice::ReadOnly))
        {
            QString json = file.readAll();
            file.close();

            emit executeCommand("load_project",json);
        }
    }

    //Action d'enrichissement du contexte
    if (obj->objectName() == "actionContext")
    {
        QString str_FicCoupe;
        str_FicCoupe = m_config_specifique_coupe_path;
        const QFileInfo outputDir(str_FicCoupe);

        QJsonArray jsonArray_servo;
        QJsonArray jsonArray_servo_values;
        QJsonArray jsonArray_motor;
        QJsonArray jsonArray_ax;
        QJsonArray jsonArray_ax_values;
        QJsonArray jsonArray_state_machine;
        QJsonArray jsonArray_switch;

        //valeurs par défaut pour nommer les servos
        QString servoName="SERVO_%1";
        for (int i=1;i<8;i++)
        {
            QJsonObject obj_json;
            obj_json["nom"] = servoName.arg(i);
            obj_json["valeur"] = i;
            jsonArray_servo.append(obj_json);
        }
        //Valeurs par défaut pour nommer les switches (Switch_1 à Switch_10)
        QString switchName="Switch_%1";
        for (int i=1;i<=10;i++)
        {
            QJsonObject obj_json;
            obj_json["nom"] = switchName.arg(i);
            obj_json["valeur"] = i-1;  // index physique 0-based (eATTRIBUTION_POWER_ELECTROBOT)
            jsonArray_switch.append(obj_json);
        }
        //Valeurs par défaut pour nommer les moteurs
        QString motorName="MOTOR_%1";
        for (int i=1;i<7;i++)
        {
            QJsonObject obj_json;
            obj_json["nom"] = motorName.arg(i);
            obj_json["valeur"] = i;
            jsonArray_motor.append(obj_json);
        }
        //Valeurs par défaut pour nommer les ax
        QString axName="AX_%1";
        for (int i=1;i<10;i++)
        {
            QJsonObject obj_json;
            obj_json["nom"] = axName.arg(i);
            obj_json["valeur"] = i;
            jsonArray_ax.append(obj_json);
        }

        //pour les tests liste statique de machine à état
        jsonArray_state_machine.append("Autotest");
        jsonArray_state_machine.append("ChasseNeige");
        jsonArray_state_machine.append("Centre");
        jsonArray_state_machine.append("Curseur");
        jsonArray_state_machine.append("PetiteBordure");
        jsonArray_state_machine.append("GrandeBordure");
        jsonArray_state_machine.append("RetourZoneDepart");


        //on verifie si le chemin par defaut du fichier d'entete est valide
        if ((outputDir.exists()) || (outputDir.isReadable()))
        {
            qDebug() << "[CBlockBotLab] Conf Specifique coupe (" << str_FicCoupe << " ) exists and is readable";
            QList<EnumDefinition> enums=CActuatorSequencer::getEnum(str_FicCoupe);

            //mise à jour avec les valeurs prédéfinies
            for (const EnumDefinition &enumDef : enums) {
                if (enumDef.enumName == "eVALUES_SERVOS") {
                    for (const auto& servo_values : enumDef.values)
                    {
                        QJsonObject obj_json;
                        obj_json["nom"] = servo_values.name;
                        obj_json["valeur"] = servo_values.value;
                        jsonArray_servo_values.append(obj_json);
                    }
                }
                else if (enumDef.enumName == "eVALUES_SERVOS_AX") {
                    for (const auto& ax_values : enumDef.values)
                    {
                        QJsonObject obj_json;
                        obj_json["nom"] = ax_values.name;
                        obj_json["valeur"] = ax_values.value;
                        jsonArray_ax_values.append(obj_json);
                    }
                }
                else if (enumDef.enumName == "eATTRIBUTION_SERVOS_AX") {
                    jsonArray_ax={};
                    for (const auto& ax : enumDef.values)
                    {
                        QJsonObject obj_json;
                        obj_json["nom"] = ax.name;
                        obj_json["valeur"] = ax.value;
                        jsonArray_ax.append(obj_json);
                    }
                }
                else if (enumDef.enumName == "eATTRIBUTION_SERVOS") {
                    jsonArray_servo={};
                    for (const auto& servo : enumDef.values)
                    {
                        QJsonObject obj_json;
                        obj_json["nom"] = servo.name;
                        obj_json["valeur"] = servo.value;
                        jsonArray_servo.append(obj_json);
                    }
                }
                else if (enumDef.enumName == "eATTRIBUTION_MOTEURS") {
                    jsonArray_motor={};
                    for (const auto& motor : enumDef.values)
                    {
                        QJsonObject obj_json;
                        obj_json["nom"] = motor.name;
                        obj_json["valeur"] = motor.value;
                        jsonArray_motor.append(obj_json);
                    }
                }
                else if (enumDef.enumName == "eATTRIBUTION_POWER_ELECTROBOT") {
                    jsonArray_switch={};
                    for (const auto& sw : enumDef.values)
                    {
                        QJsonObject obj_json;
                        obj_json["nom"] = sw.name;
                        obj_json["valeur"] = sw.value;
                        jsonArray_switch.append(obj_json);
                    }
                }
                // Noms des machines à états disponibles.
                // Format : tableau de strings (pas de valeur numérique associée).
                // Adapter enumName selon la définition dans ConfigSpecifiqueCoupe.h.
                else if (enumDef.enumName == "eSTATE_MACHINES") {
                    jsonArray_state_machine = {};
                    for (const auto& sm : enumDef.values)
                        jsonArray_state_machine.append(sm.name);
                }

            }
        }

        //on envoie les valeurs pour enrichir le contexte de BlockBot
        emit executeCommand("servos", QJsonDocument(jsonArray_servo).toJson());
        emit executeCommand("values_servos", QJsonDocument(jsonArray_servo_values).toJson());
        emit executeCommand("moteurs", QJsonDocument(jsonArray_motor).toJson());
        emit executeCommand("servos_ax", QJsonDocument(jsonArray_ax).toJson());
        emit executeCommand("values_servos_ax", QJsonDocument(jsonArray_ax_values).toJson());
        emit executeCommand("state_machine", QJsonDocument(jsonArray_state_machine).toJson());
        emit executeCommand("switch", QJsonDocument(jsonArray_switch).toJson());

    }
}

void CBlockBotLab::logJS(const QString& message)
{
    qDebug() << "[log JS] : " << message;
}

// _____________________________________________________________________
/*!
*  Slot_SetPosFromSimu
*
*  Déclenché par le signal setSequence() de CSimuBot (double-clic sur la
*  simulation quand la checkbox "setSequence" est cochée).
*
*  En mode expert : envoie "add_state_simu" pour créer un triplet
*    (etat_expert + set_pos XYTheta + convergence_expert) connecté au
*    dernier état libre de la chaîne.
*
*  En mode débutant : envoie "add_pos_simu_debutant" pour créer un bloc
*    se_deplacer_en_position (X, Y, Teta) connecté au dernier bloc libre
*    du bloc description_debutant.
*    Le mode n'est PAS forcé en expert.
*/
void CBlockBotLab::Slot_SetPosFromSimu(double angle, double distance)
{
    QString currentMode = modeChoice->currentText();

    if (currentMode == "expert") {
        // ── Mode expert : comportement inchangé ───────────────────────────
        // Lecture des coordonnées simulées depuis le DataCenter
        int    x     = m_application->m_data_center->read("x_pos").toInt();
        int    y     = m_application->m_data_center->read("y_pos").toInt();
        double theta = m_application->m_data_center->read("teta_pos").toDouble();

        // Construction du JSON de paramètres transmis à BlockBot
        QJsonObject params;
        params["x"]       = x;
        params["y"]       = y;
        params["theta"]   = theta;
        params["timeout"] = 7000;

        emit executeCommand("add_state_simu", QJsonDocument(params).toJson(QJsonDocument::Compact));
    }
    else {
        // ── Mode débutant : création d'un triplet set_angle / avancer / set_angle ──
        // Lecture de l'orientation courante du robot depuis le DataCenter.
        // teta_pos est stocké dans la même unité que setAndGetInRad (rad ou deg).
        // Conversion en degrés : les blocs BlockBot mode débutant travaillent toujours en degrés.
        // teta_rad est transmis séparément (en radians) pour le commentaire récapitulatif info_debutant.
        double teta_pos = m_application->m_data_center->read("teta_pos").toDouble();
        double teta     = teta_pos;      // en degrés pour les blocs set_angle_robot
        double teta_rad = teta_pos;      // en radians pour le commentaire récapitulatif

        if (m_application->m_SimuBot->setAndGetInRad) {
            teta = teta_pos * 180.0 / Pi;   // rad → deg pour les blocs débutant
            // teta_rad reste teta_pos (déjà en radians)
        } else {
            teta_rad = teta_pos * Pi / 180.0;   // deg → rad pour le commentaire
        }

        int x = m_application->m_data_center->read("x_pos").toInt();
        int y = m_application->m_data_center->read("y_pos").toInt();

        // Construction du JSON de paramètres transmis à BlockBot.
        // angle est déjà normalisé en degrés par catchDoubleClick() dans CSimuBot.
        QJsonObject params;
        params["angle"]    = angle;
        params["distance"] = distance;
        params["teta"]     = teta;
        params["x"]        = x;
        params["y"]        = y;
        params["teta_rad"] = teta_rad;

        emit executeCommand("add_pos_simu_debutant", QJsonDocument(params).toJson(QJsonDocument::Compact));
    }
}

void CBlockBotLab::onDownloadRequested(QWebEngineDownloadItem *download)
{
    QString defaultDir =
        QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);

    QString defaultFile =
        defaultDir + "/" + download->downloadFileName();

    QString selectedFile = QFileDialog::getSaveFileName(
        m_ihm.ui.centralwidget,
        tr("Enregistrer le workspace Blockly"),
        defaultFile,
        tr("Fichier Blockly (*.json)")
    );

    // Si l'utilisateur annule
    if (selectedFile.isEmpty()) {
        download->cancel();
        return;
    }

    download->setPath(selectedFile);
    download->accept();
}


// =====================================================================
//                     HIL (Hardware In the Loop)
// =====================================================================

// _____________________________________________________________________
/*!
 * Slot connecté à actionPlayHIL.
 * Demande à BlockBot l'état de départ (sélectionné ou premier de la chaîne).
 * Le flux continue dans processHILExport() quand JS répond.
 */
void CBlockBotLab::Slot_PlayHIL()
{
    m_application->m_print_view->print_info(this, "[HIL] Demande de l'etat de depart...");
    emit executeCommand("get_hil_start_state", "");
}

// _____________________________________________________________________
/*!
 * Slot connecté à actionStopHIL.
 * Arrête le HIL (arrêt de sécurité + nettoyage).
 */
void CBlockBotLab::Slot_StopHIL()
{
    if (m_hilEngine->isRunning()) {
        m_application->m_print_view->print_info(this, "[HIL] ARRET UTILISATEUR — arret de securite");
    }
    m_hilEngine->stop();
}

// _____________________________________________________________________
/*!
 * Slot connecté à actionPlayOnlyOneHIL.
 * Demande à BlockBot l'action sélectionnée dans le workspace.
 * Le flux continue dans processHILExport() quand JS répond.
 */
void CBlockBotLab::Slot_PlaySingleActionHIL()
{
    m_application->m_print_view->print_info(this, "[HIL] Demande de l'action selectionnee...");
    emit executeCommand("export_hil_single_action", "");
}

// _____________________________________________________________________
/*!
 * Slot connecté à actionMergeProject.
 * Ouvre un fichier JSON via boîte de dialogue et fusionne les blocs dans le workspace
 * courant sans l'effacer — équivalent de l'import de fragment de CActuatorSequencer.
 * La commande envoyée est "import_project" (distincte de "load_project" qui efface).
 */
void CBlockBotLab::Slot_MergeProject()
{
    QString fileName = QFileDialog::getOpenFileName(
        m_ihm.ui.centralwidget,
        tr("Importer un fragment de strategie"),
        QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation),
        tr("Fichier Blockly (*.json)")
    );

    if (fileName.isEmpty())
        return;

    QFile file(fileName);
    if (file.open(QIODevice::ReadOnly)) {
        QString json = file.readAll();
        file.close();
        emit executeCommand("import_project", json);
    }
}

// _____________________________________________________________________
/*!
 * Slot appelé par JS via QWebChannel en réponse aux commandes HIL.
 *
 * @param hilType  "start_state" : réponse à get_hil_start_state (nom de l'état de départ)
 *                 "state"       : réponse à get_hil_state (JSON d'un état complet)
 *                 "single_action" : réponse à export_hil_single_action (JSON d'une action)
 * @param hilJson  Données JSON ou nom d'état (vide si rien de valide)
 */
void CBlockBotLab::processHILExport(const QString &hilType, const QString &hilJson)
{
    if (hilType == "start_state") {
        // Réponse à get_hil_start_state : lancer le HIL depuis cet état
        if (hilJson.isEmpty()) {
            m_application->m_print_view->print_warning(this, "[HIL] Aucun etat de depart trouve dans le workspace");
            return;
        }
        m_hilEngine->startFromState(hilJson);
    }
    else if (hilType == "state") {
        // Réponse à get_hil_state : transmettre la description au moteur HIL
        m_hilEngine->feedState(hilJson);
    }
    else if (hilType == "single_action") {
        // Réponse à export_hil_single_action : exécuter l'action unique
        if (hilJson.isEmpty()) {
            m_application->m_print_view->print_warning(this, "[HIL] Aucune action selectionnee (selectionnez un bloc action dans le workspace)");
            return;
        }
        m_hilEngine->executeSingleAction(hilJson);
    }
    else if (hilType == "logic_keys") {
        // Réponse à hil_logic_init : clés DM renvoyées par JS pour si_vrai_expert
        m_hilEngine->feedLogicKeys(hilJson);
    }
    else if (hilType == "logic_result") {
        // Réponse à hil_logic_tick : résultat de l'évaluation de la condition
        m_hilEngine->feedLogicResult(hilJson == "true");
    }
}

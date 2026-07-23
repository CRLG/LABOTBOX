#!/bin/bash
# _____________________________________________________________________________
# build_robot_logic.sh -- POC hot-reload Simulia
#
# Compile la logique robot (RobotLogicPlugin.pro) en une librobotlogic_AAAAMMJJ_HHMMSS.so
# et depose le resultat dans le repertoire scanne par Simulia, pret a etre recharge a chaud.
#
# Ce script est le point d'entree unique du build "logique robot" :
#  - depuis BlockBotLab (bouton "Compiler pour Simulia" de la barre d'outils BlockBot),
#  - depuis la barre d'outils "Logique robot" de Simulia,
#  - ou a la main en terminal.
#
# Il absorbe les deux contraintes du POC qui obligeaient jusqu'ici a sortir de l'IHM :
#  1. qmake DOIT etre lance depuis un repertoire de build FRERE de Simulia/, sinon les chemins
#     relatifs du .pro (../Soft_STM32) ne resolvent pas.
#  2. le .so doit atterrir dans le repertoire scanne par Simulia (clef EEPROM
#     [Simulia] robot_logic_lib_path) -> plus aucune copie manuelle.
#
# qmake est relance a CHAQUE build, volontairement :
#  - le TARGET est horodate (evalue a l'instant du qmake) -> une lib distincte par build ;
#  - les SOURCES sont des globs (*.cpp) -> un NOUVEAU fichier sm_xxx.cpp genere par BlockBot
#    n'est vu qu'apres un nouveau qmake.
# Le cout est negligeable : qmake + compilation incrementale + link tiennent sous la seconde.
#
# IMPORTANT : ce script ne touche PAS Soft_STM32 (chaine de build/flash STM32 preservee).
#
# Variables d'environnement (toutes optionnelles, surchargeables par les options ci-dessous) :
#   ROBOT_LOGIC_DEST       repertoire de depot du .so       (defaut : le repertoire de build)
#   ROBOT_LOGIC_BUILD_DIR  repertoire de build              (defaut : <racine>/build_plugin)
#   ROBOT_LOGIC_KEEP       nombre de libs conservees        (defaut : 5)
#   ROBOT_LOGIC_JOBS       parallelisme de make             (defaut : nproc)
#   QMAKE                  binaire qmake a utiliser         (defaut : qmake, sinon qmake-qt5)
# _____________________________________________________________________________

set -u
set -o pipefail

PREFIX="[build_robot_logic]"

usage()
{
    cat <<EOF
Usage: $(basename "$0") [options]

  --dest DIR        repertoire de depot de la librobotlogic_*.so
                    (doit etre celui de la clef EEPROM [Simulia] robot_logic_lib_path)
  --build-dir DIR   repertoire de build (objets .o reutilises d'un build a l'autre)
  --keep N          nombre de librobotlogic_*.so conservees dans le repertoire de depot
  --jobs N          parallelisme de make
  -h | --help       cette aide
EOF
}

# _____________________________________________________________________________
# Localisation : ce script vit dans <racine>/Simulia/tools/ ; la racine est le repertoire
# PARENT de Simulia/, seul endroit d'ou les chemins relatifs du .pro (../Soft_STM32) resolvent.
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
SIMULIA_DIR="$(cd "$SCRIPT_DIR/.." && pwd)"
ROOT_DIR="$(cd "$SIMULIA_DIR/.." && pwd)"
PRO_FILE="$SIMULIA_DIR/RobotLogicPlugin.pro"

BUILD_DIR="${ROBOT_LOGIC_BUILD_DIR:-$ROOT_DIR/build_plugin}"
DEST_DIR="${ROBOT_LOGIC_DEST:-}"
KEEP="${ROBOT_LOGIC_KEEP:-5}"
JOBS="${ROBOT_LOGIC_JOBS:-$(nproc 2>/dev/null || echo 1)}"

while [ $# -gt 0 ]; do
    case "$1" in
        --dest)       DEST_DIR="${2:-}"  ; shift 2 ;;
        --build-dir)  BUILD_DIR="${2:-}" ; shift 2 ;;
        --keep)       KEEP="${2:-}"      ; shift 2 ;;
        --jobs)       JOBS="${2:-}"      ; shift 2 ;;
        -h|--help)    usage ; exit 0 ;;
        *)            echo "$PREFIX option inconnue : $1" >&2 ; usage >&2 ; exit 2 ;;
    esac
done

if [ ! -f "$PRO_FILE" ]; then
    echo "$PREFIX ERREUR : $PRO_FILE introuvable." >&2
    exit 1
fi

# Choix du binaire qmake (Robuntu fournit qmake ; certaines distros n'ont que qmake-qt5).
QMAKE_BIN="${QMAKE:-}"
if [ -z "$QMAKE_BIN" ]; then
    if   command -v qmake     > /dev/null 2>&1 ; then QMAKE_BIN="qmake"
    elif command -v qmake-qt5 > /dev/null 2>&1 ; then QMAKE_BIN="qmake-qt5"
    else
        echo "$PREFIX ERREUR : qmake introuvable (installer les outils de developpement Qt5)." >&2
        exit 1
    fi
fi

# _____________________________________________________________________________
# Repertoires. Le repertoire de build est conserve d'un build a l'autre : c'est lui qui rend
# la compilation incrementale (seuls les .cpp modifies par BlockBot sont recompiles).
mkdir -p "$BUILD_DIR" || { echo "$PREFIX ERREUR : impossible de creer $BUILD_DIR" >&2 ; exit 1 ; }
BUILD_DIR="$(cd "$BUILD_DIR" && pwd)"

# Sans destination explicite, le .so reste dans le repertoire de build (comportement historique).
if [ -z "$DEST_DIR" ]; then
    DEST_DIR="$BUILD_DIR"
else
    mkdir -p "$DEST_DIR" || { echo "$PREFIX ERREUR : impossible de creer $DEST_DIR" >&2 ; exit 1 ; }
    DEST_DIR="$(cd "$DEST_DIR" && pwd)"
fi

echo "$PREFIX projet    : $PRO_FILE"
echo "$PREFIX build      : $BUILD_DIR"
echo "$PREFIX depot      : $DEST_DIR"

cd "$BUILD_DIR" || exit 1

# DESTDIR est lu par le .pro depuis l'environnement, AU MOMENT DU QMAKE ($$(ROBOT_LOGIC_DEST)).
export ROBOT_LOGIC_DEST="$DEST_DIR"

START_TIME=$SECONDS

echo "$PREFIX qmake..."
if ! "$QMAKE_BIN" "$PRO_FILE"; then
    echo "$PREFIX ECHEC : qmake" >&2
    exit 1
fi

echo "$PREFIX make -j$JOBS..."
if ! make -j"$JOBS"; then
    echo "$PREFIX ECHEC : compilation de la logique robot" >&2
    exit 1
fi

# _____________________________________________________________________________
# Menage : le nom horodate cree une lib par build. On ne garde que les KEEP plus recentes,
# sinon le combo de selection de Simulia se remplit indefiniment.
# NB : supprimer une lib actuellement chargee par Simulia est sans danger sous Linux
# (l'inode reste vivant tant que le processus la mappe).
if [ "$KEEP" -gt 0 ] 2>/dev/null; then
    OLD_LIBS="$(ls -1t "$DEST_DIR"/librobotlogic_*.so 2>/dev/null | tail -n +$((KEEP + 1)))"
    if [ -n "$OLD_LIBS" ]; then
        echo "$OLD_LIBS" | while read -r old_lib; do
            rm -f "$old_lib"
            echo "$PREFIX purge de l'ancienne lib : $(basename "$old_lib")"
        done
    fi
fi

LATEST_LIB="$(ls -1t "$DEST_DIR"/librobotlogic_*.so 2>/dev/null | head -n 1)"
if [ -z "$LATEST_LIB" ]; then
    echo "$PREFIX ECHEC : aucune librobotlogic_*.so produite dans $DEST_DIR" >&2
    exit 1
fi

echo "$PREFIX OK en $((SECONDS - START_TIME))s : $LATEST_LIB"
exit 0

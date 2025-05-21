#!/bin/bash
function getgit() {
CDIR="$( cd -- "$( dirname -- "${BASH_SOURCE[0]:-$0}"; )" &> /dev/null && pwd 2> /dev/null; )";

for i in $(ls -R | grep :); do
    DIR=${i%:}                    # Strip ':'
    cd $DIR
    if [ -d "$DIR/.git" ] || [ -e ./.git ]; then   # Ne fait le pull que sur les répertoires contenant un élément git (projet ou sous-module)
	echo $DIR
	$1                        # Your command
    fi
    cd $CDIR
done
echo "Fin"
}



(getgit "git pull origin master") &



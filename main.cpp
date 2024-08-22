#include <QStyleFactory>
#include <QToolTip>
#include <QStyle>
#include <QApplication>
#include "CApplication.h"

void setApplicationApparence();

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    setApplicationApparence();
    CApplication LaBotBox;
    LaBotBox.run();

    return a.exec();
    //
}


// force l'apparence de l'application (nécessaire pour compatibilité PC/Raspberry)
void setApplicationApparence()
{
    // sur Rasbperry OS, le style par défaut ne permet pas d'afficher la couleur sur les boutons
    // => force le style à Fusion (par défaut sur PC mais pas sur Raspberry)
    QApplication::setStyle(QStyleFactory::create("Fusion"));  // pour récupérer la liste des styles possibles sur la plateforme courante : "qDebug() << QApplication::style();"

    // sur Rasberry OS, la couleur du tooltip est blanche sur fond jaune et ne se voit pas
    QPalette palette = QToolTip::palette();
    palette.setColor(QPalette::ToolTipText, Qt::black);
    QToolTip::setPalette(palette);
}

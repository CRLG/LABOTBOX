#include <QApplication>
#include "CApplication.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    CApplication LaBotBox;
    LaBotBox.run();

    return a.exec();
    //
}

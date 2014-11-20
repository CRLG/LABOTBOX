#include <QApplication>
#include "CLaBotBox.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    CLaBotBox LaBotBox;
    LaBotBox.run();

    return a.exec();
}

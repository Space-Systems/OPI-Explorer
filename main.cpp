#include "opiexplorermain.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    OpiExplorerMain w;
    w.show();

    return a.exec();
}

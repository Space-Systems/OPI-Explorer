#include "opiexplorermain.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    OpiExplorerMain w;
    QFont globalFont = w.font();
    globalFont.setPointSize(10);
    w.setFont(globalFont);
    w.show();

    return a.exec();
}

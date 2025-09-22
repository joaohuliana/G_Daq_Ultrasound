#include "gdaq.h"
#include <QApplication>
//#include <QMessageBox>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    gdaq w;
    w.show();

    return a.exec();
}

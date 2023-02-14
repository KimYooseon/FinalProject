#include "mainserver.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainServer w;

    //QTest::qExec(&w);

    w.show();
    return a.exec();
}

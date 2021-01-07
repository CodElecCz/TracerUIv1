#include <QApplication>
#include "MainWindow.h"

int main(int argc, char *argv[])
{
    Q_INIT_RESOURCE(TracerUI);

    QApplication a(argc, argv);
    QStringList args = a.arguments();
    QString filePath = "";

    foreach(QString arg, args)
    {
        if(arg.contains(".exe"))
            ;
        else if(arg.contains(":"))
            filePath = arg;
    }

    MainWindow w(filePath);
    w.show();
    return a.exec();
}

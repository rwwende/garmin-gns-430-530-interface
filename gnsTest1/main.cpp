#include <QtGui>
#include "log.h"
#include "mainWindow.h"


int main(int argc, char *argv[])
{
    char moduleName[MAX_PATH];
    char logFileName[MAX_PATH];

    Q_INIT_RESOURCE(gnsTest1);
    QApplication app(argc, argv);


    GetModuleFileNameA(GetModuleHandle(0), moduleName, sizeof(moduleName));
    sprintf(logFileName, "%s.log", moduleName);
    logInit(logFileName);


    MainWindow mainWindow;
    mainWindow.show();

    app.exec();
   
    
    return 0;
}

#include <QCoreApplication>
#include <QThread>
#include <QDebug>

#include <iostream>
#include <thread>

#include "runner.h"

int main(int argc, char *argv[])
{
    std::cout << "COM unit test" << std::endl;

    QCoreApplication app(argc, argv);
    QStringList argumentList = QCoreApplication::arguments();

    if(QCoreApplication::arguments().size() < 3) {
        std::cout << "using: COMport filename" << std::endl;
        return 0;
    }


    Runner runner;
    if(runner.Init(argv[1], argv[2]))
    {
        std::cout << "Runner init OK" << std::endl;

        runner.printInputData();
        runner.run();
    } else {
        std::cout << "Error open port " << argv[1] << std::endl;
        return 1;
    }

    return app.exec();
}

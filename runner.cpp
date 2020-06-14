#include "runner.h"
#include "serialportwriter.h"
#include "serialportworker.h"

#include <fstream>
#include <chrono>
#include <QCoreApplication>

#define MAX_SIZE_LINE 2048

static high_resolution_clock::time_point to_time_point(timeval tv)
{
    return high_resolution_clock::time_point{seconds{tv.tv_sec} + microseconds{tv.tv_usec}};
}


Runner::Runner(QObject* parent)
    : QObject(parent)
{
    mainThread = std::make_unique<QThread>();
}

Runner::~Runner()
{
}


bool Runner::Init(const char*port, const char* name)
{
    fileName = name;

    bool isReadFile = false;
    std::fstream file(fileName, std::fstream::in);

    if(file.is_open())
    {
        while(true)
        {
            char str1[MAX_SIZE_LINE];
            file.getline(str1, MAX_SIZE_LINE);

            if(!file.eof())
            {
                Record record(str1);
                if(record.getType() == TypeRecord::HOST) {
                    inputDataHost.push_back(record);
                } else if(record.getType() == TypeRecord::EGM) {
                    inputDataEGM.push_back(record);
                }
            }
            else
            {
                break;
            }
        }

        file.close();
        isReadFile = true;
    }

    if(isReadFile) {
        worker = std::make_unique<SerialPortWorker>(port, inputDataHost, inputDataEGM);//Создаем обьект по классу

        connect(worker.get(), SIGNAL(finishTest()),  this, SLOT(finishTest()));

        worker->moveToThread(mainThread.get());                 //помешаем класс  в поток
        worker->getSerialPort()->moveToThread(mainThread.get());
    }



    return isReadFile && worker->getPortState();
}

void Runner::printInputData()
{
    for(const auto &i: inputDataHost){
        std::cout << i << std::endl;
    }

    for(const auto &i: inputDataEGM){
        std::cout << i << std::endl;
    }
}

void Runner::run()
{
    mainThread->start();
}

void Runner::finishTest()
{
    std::cout << "void Runner::finishTest()" << std::endl;

    std::cout << "get data size=" << worker->getData().size() << std::endl;

    for(auto& iii: worker->getData()) {
        std::cout << iii.getData() << std::endl;
        std::cout << iii << std::endl;
    }

    exit(0);
}


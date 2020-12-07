#include "mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setupControlType(controlType);

    serial = new QSerialPort;
    setupSerial(serial);

    double rangeFlow[4] = {1, -1, 1, 1000};
    setupAxisRect(ui->graphFlow, rangeFlow);

    double rangePressure[4] = {1, -2, 1, 1000};
    setupAxisRect(ui->graphPressure,  rangePressure);

    double rangeVolume[4] = {1, -3, 1, 1000};
    setupAxisRect(ui->graphVolume, rangeVolume);

    setupDataToGrafs(dataToGrafs);

    serial->write(0);
    connect(serial, SIGNAL(readyRead()), this, SLOT(serialRecieveSlot()));
    connect(&dataTimer, SIGNAL(timeout()), this, SLOT(realtimeDataSlot()));
    dataTimer.start(25);

}

MainWindow::~MainWindow()
{
    serial->close();
    delete serial;
    delete ui;
}

void MainWindow::serialRecieveSlot()
{
    QByteArray message;
    message = serial->read(16);
    setDataFromMassage(&message, dataFromMassage);
}

void MainWindow::realtimeDataSlot()
{
    setDataToGrafs(pointPosition, dataToGrafs, dataFromMassage);
    pointPosition++;
    if (pointPosition > 999)
        pointPosition = 0;

    ui->graphFlow->graph(0)->setData(dataToGrafs[1], dataToGrafs[0]);
    ui->graphFlow->replot();

    ui->graphPressure->graph(0)->setData(dataToGrafs[3], dataToGrafs[2]);
    ui->graphPressure->replot();

    ui->graphVolume->graph(0)->setData(dataToGrafs[5], dataToGrafs[4]);
    ui->graphVolume->replot();


}

void MainWindow::setupControlType(QMap<QString, QByteArray>& controlType)
{
   controlType.insert("flow", "0x00");
   controlType.insert("pressure", "0x01");
   controlType.insert("volume", "0x02");
}

void MainWindow::setupSerial(QSerialPort* serial)
{
    serial->setPortName("/dev/ttyACM1");
    serial->setDataBits(QSerialPort::Data8);
    serial->setBaudRate(QSerialPort::Baud115200);
    serial->setStopBits(QSerialPort::OneStop);
    serial->setParity(QSerialPort::NoParity);
    serial->setFlowControl(QSerialPort::NoFlowControl);
    serial->open(QIODevice::ReadWrite);

}

void MainWindow::setupDataToGrafs(QVector<QVector<double>>& dataToGrafs)
{
    dataToGrafs.resize(8);
    for(int iter = 0; iter < dataToGrafs.size(); iter++){
        dataToGrafs[iter].resize(1020);
    }
}

void MainWindow::setupAxisRect(QCustomPlot* customPlot , double* range)
{
    customPlot->plotLayout()->clear();

    QCPAxisRect* axisRect = new QCPAxisRect(customPlot, false);

    setupAxis(axisRect, range);

    customPlot->plotLayout()->addElement(0, 0,  axisRect);
    customPlot->addGraph(axisRect->axis(QCPAxis::atBottom),  axisRect->axis(QCPAxis::atLeft));
}

void MainWindow::setupAxis(QCPAxisRect* axisRect, double* range)
{
    axisRect->addAxes(QCPAxis::atBottom | QCPAxis::atLeft);

    axisRect->axis(QCPAxis::atLeft)->setRange(range[0] , range[1]);
    axisRect->axis(QCPAxis::atLeft)->grid()->setVisible(true);

    axisRect->axis(QCPAxis::atBottom)->setRange(range[2] , range[3]);
    axisRect->axis(QCPAxis::atBottom)->grid()->setVisible(true);
}

void MainWindow::setDataFromMassage(QByteArray* message, double* dataFromMassage)
{
    uint8_t sizeOfOneNum = 4;
    uint32_t scaleToDouble = 1000000;
    for(uint8_t iter = 0; iter <= message->length(); iter += sizeOfOneNum)
    {
        QByteArray oneNum = message->mid(iter, sizeOfOneNum);
        std::reverse(oneNum.begin(), oneNum.end());
        dataFromMassage[iter/sizeOfOneNum] = (((int32_t)oneNum.toHex().toUInt(0, 16)));
        dataFromMassage[iter/sizeOfOneNum] /= scaleToDouble;
//        qDebug() << (int32_t)oneNum.toHex().toUInt(0, 16);
    }
//    qDebug() <<"---------";
}

void MainWindow::setDataToGrafs(int pointPosition, QVector<QVector<double>>& dataToGrafs, double* dataFromMassage)
{

    for(int axes = 0; axes < dataToGrafs.size(); axes+=2){
        dataToGrafs[axes][pointPosition] = dataFromMassage[axes/2];
        setGapsDataToGrafs(pointPosition, dataToGrafs[axes]);
        dataToGrafs[axes+1][pointPosition] = pointPosition;

    }
}

void MainWindow::setGapsDataToGrafs(int pointPosition, QVector<double>& dataToGrafs)
{
    int sizeOfGap = 10;
    for(int data = pointPosition+1; data < (pointPosition + sizeOfGap); data++)
        dataToGrafs[data] = qQNaN();
}

void MainWindow::sendSerial()
{
//    QByteArray senf
//    serial->write(senf);
    //    if (serial->waitForBytesWritten()) {
    //    } else {
    //        qDebug()<<"ti lox";
    //    }
}

void MainWindow::on_butFlow_clicked()
{
    sendBuffer[0] = 0x00;
    qDebug() << "Flow";

}

void MainWindow::on_butPressure_clicked()
{
    sendBuffer[0] = 0x01;
    qDebug()<<"Pressure";
}

void MainWindow::on_butVolume_clicked()
{
    sendBuffer[0] = 0x02;
    qDebug()<<"Volume";
}

void MainWindow::on_butApply_clicked()
{
    serial->write(sendBuffer);
    qDebug()<< QString(sendBuffer);
    qDebug()<<"apply";
}

#include "mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    QPalette p(palette());
    p.setColor(QPalette::Window, Qt::white);
    ui->tabSettings->setAutoFillBackground(true);
    ui->tabSettings->setPalette(p);

    QPixmap pix(":/img/img/logo.png");
    int w = ui->logo->width();
    int h = ui->logo->height();
    ui->logo->setPixmap(pix.scaled(w, h, Qt::KeepAspectRatio));

    sendBuffer.resize(7);
    sendBuffer[0] = 0x21;
    sendBuffer[1] = 0x00;
    sendBuffer[2] = 0x00;
    sendBuffer[3] = 0x00;
    sendBuffer[4] = 0x00;
    sendBuffer[5] = 0x12;
    sendBuffer[6] = 0x00;
    dataFromMassage[0] = 0;
    dataFromMassage[1] = 0;
    dataFromMassage[2] = 0;
    dataFromMassage[3] = 0;
    dataFromMassage[4] = 0;
    dataFromMassage[5] = 0;
    textBuffer.resize(5);



    serial = new QSerialPort;
    setupSerial(serial);

    double rangeFlow[4] = {-0.5, 1.5, 1, 500};
    setupAxisRect(ui->graphFlow, rangeFlow);

    double rangeVolume[4] = {-1, 5, 1, 500};
    setupAxisRect(ui->graphVolume, rangeVolume);

    setupDataToGrafs(dataToGrafs);

    connect(serial, SIGNAL(readyRead()), this, SLOT(serialRecieveSlot()));
    connect(&dataTimer, SIGNAL(timeout()), this, SLOT(realtimeDataSlot()));
    connect(serial, SIGNAL(errorOccurred(QSerialPort::SerialPortError)), this, SLOT(connectionCheckSlot(QSerialPort::SerialPortError)));
    connect(&connectionTimer, SIGNAL(timeout()), this, SLOT(reconnectSerial()));

    dataTimer.start(70);

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
    QString spo = "Содержание кислорода: %1 \%";

    ui->label->setText(spo.arg(dataFromMassage[2]));
    ui->label_2->setText(spo.arg(dataFromMassage[2]));

    pointPosition++;
    if (pointPosition > 500)
        pointPosition = 0;

    ui->graphFlow->graph(0)->setData(dataToGrafs[1], dataToGrafs[0]);//0
    ui->graphFlow->replot();


    ui->graphVolume->graph(0)->setData(dataToGrafs[3], dataToGrafs[2]);//1
    ui->graphVolume->replot();


}


void MainWindow::setupSerial(QSerialPort* serial)
{
    serial->close();
    const auto listPort = QSerialPortInfo::availablePorts();
    qDebug()<<listPort.size();
    if(listPort.size()!= 0)
    {
        qDebug()<< listPort[0].portName();
        serial->setPortName(listPort[0].portName());
        serial->setDataBits(QSerialPort::Data8);
        serial->setBaudRate(QSerialPort::Baud115200);
        serial->setStopBits(QSerialPort::OneStop);
        serial->setParity(QSerialPort::NoParity);
        serial->setFlowControl(QSerialPort::NoFlowControl);
        if (!serial->open(QIODevice::ReadWrite))
        {
            qDebug()<< "not op";

        }else{
            connectionTimer.stop();
        }


    }else{
        connectionTimer.start(1500);
    }
}

void MainWindow::setupDataToGrafs(QVector<QVector<double>>& dataToGrafs)
{
    dataToGrafs.resize(8);
    for(int iter = 0; iter < dataToGrafs.size(); iter++){
        dataToGrafs[iter].resize(511);
    }
}

void MainWindow::setupAxisRect(QCustomPlot* customPlot , double* range)
{
    customPlot->plotLayout()->clear();

    QCPAxisRect* axisRect = new QCPAxisRect(customPlot, false);

    setupAxis(axisRect, range);

    customPlot->plotLayout()->addElement(0, 0,  axisRect);
    customPlot->addGraph(axisRect->axis(QCPAxis::atBottom),  axisRect->axis(QCPAxis::atLeft));
    customPlot->graph(0)->setPen(QPen(QColor(66, 145, 255), 4));
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
        if(iter/sizeOfOneNum != 2){
            dataFromMassage[iter/sizeOfOneNum] /= scaleToDouble;
        }
    }
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
    const int sizeOfGap = 10;
    for(int data = pointPosition+1; data < (pointPosition + sizeOfGap); data++)
        dataToGrafs[data] = qQNaN();
}

void MainWindow::sendSerial()
{

}

void MainWindow::on_butFlow_clicked()
{
    qDebug() << "Flow";
}

void MainWindow::on_butPressure_clicked()
{
    qDebug()<<"Pressure";
}

void MainWindow::on_butVolume_clicked()
{
    qDebug()<<"Volume";
}

void MainWindow::on_butApply_clicked()
{
    serial->write(sendBuffer);
    QByteArray oneNum;
    oneNum[0] = sendBuffer[6];
    qDebug()<< QString::number((int32_t)oneNum.toHex().toUInt(0, 16));
    qDebug()<<"apply";
}


void MainWindow::connectionCheckSlot(QSerialPort::SerialPortError error)
{
    qDebug()<<"slot error";
    connectionTimer.start(1500);

}

void MainWindow::reconnectSerial()
{
    setupSerial(serial);
    qDebug()<<"reconectSerial";
    serial->error();
}

void MainWindow::on_dial_valueChanged(int value)
{
    sendBuffer[6] = value;
    auto v = static_cast<double>(value) /10;
    ui->spinBox->setValue(v);
}

void MainWindow::on_spinBox_valueChanged(int arg1)
{

}

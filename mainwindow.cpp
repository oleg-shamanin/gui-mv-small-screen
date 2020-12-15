#include "mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    sendBuffer.resize(5);
    sendBuffer[0] = 0x21;
    sendBuffer[1] = 0x00;
    sendBuffer[2] = 0x00;
    sendBuffer[3] = 0x00;
    sendBuffer[4] = 0x12;
    dataFromMassage[0] = 0;
    dataFromMassage[1] = 0;
    dataFromMassage[2] = 0;



    ui->setupUi(this);
    setupControlType(controlType);

    serial = new QSerialPort;
    setupSerial(serial);

    double rangeFlow[4] = {-1.5, 4, 1, 1000};
    setupAxisRect(ui->graphFlow, rangeFlow);

    double rangePressure[4] = {-1.5, 3.5, 1, 1000};
    setupAxisRect(ui->graphPressure,  rangePressure);

    double rangeVolume[4] = {-1.5, 2500, 1, 1000};
    setupAxisRect(ui->graphVolume, rangeVolume);

    setupDataToGrafs(dataToGrafs);

    connect(serial, SIGNAL(readyRead()), this, SLOT(serialRecieveSlot()));
    connect(&dataTimer, SIGNAL(timeout()), this, SLOT(realtimeDataSlot()));
    connect(serial, SIGNAL(errorOccurred(QSerialPort::SerialPortError)), this, SLOT(connectionCheckSlot(QSerialPort::SerialPortError)));
    connect(&connectionTimer, SIGNAL(timeout()), this, SLOT(reconnectSerial()));
//    qDebug()<< QString::number( serial->pinoutSignals());

//    serial->errorOccurred();

    dataTimer.start(50);


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
    QByteArray oneNum;

    oneNum[0] = textBuffer[1];
    int clapan1 = (int32_t)oneNum.toHex().toUInt(0, 16);

    oneNum[0] = textBuffer[2];
    int clapan2 = (int32_t)oneNum.toHex().toUInt(0, 16);

    oneNum[0] = textBuffer[3];
    int clapan3 = (int32_t)oneNum.toHex().toUInt(0, 16);

    QFile file("/home/oleg/project/pyqt5/small/gui-mv-small-screen/1.txt");
    file.open(QIODevice::ReadWrite);
    file.readAll();

    QString str = "\t%1\t%2\t%3\t%4\t%5\t%6\t%7\n";
    QTextStream stream(&file);
        foreach(QString s, str.arg(clapan1).arg(clapan2).arg(clapan3).arg(dataFromMassage[0]).arg(dataFromMassage[1]).arg(dataFromMassage[2]).arg(dataFromMassage[3]))
        {
            stream<<s;
        }


    pointPosition++;
    if (pointPosition > 990)
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
            ui->labelCon->setText("не подключён  setup error");

        }else{
            connectionTimer.stop();
            ui->labelCon->setText("подключён");
        }


    }else{
        connectionTimer.start(1500);
    }
}

void MainWindow::setupDataToGrafs(QVector<QVector<double>>& dataToGrafs)
{
    dataToGrafs.resize(8);
    for(int iter = 0; iter < dataToGrafs.size(); iter++){
        dataToGrafs[iter].resize(1050);
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
        if(iter/sizeOfOneNum != 3){
            dataFromMassage[iter/sizeOfOneNum] /= scaleToDouble;
        }

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
    int sizeOfGap = 30;
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
    QByteArray oneNum;
    oneNum[0] = sendBuffer[3];
    textBuffer = sendBuffer;
    qDebug()<< QString::number((int32_t)oneNum.toHex().toUInt(0, 16));
    qDebug()<<"apply";
}

void MainWindow::on_spinBox_valueChanged(const QString &arg1)
{
    sendBuffer[1] = ui->spinBox->value();
}

void MainWindow::on_spinBox_2_valueChanged(const QString &arg1)
{
    sendBuffer[2] = ui->spinBox_2->value();
}

void MainWindow::on_spinBox_3_valueChanged(const QString &arg1)
{
    sendBuffer[3] = ui->spinBox_3->value();
}

void MainWindow::on_spinBox_editingFinished()
{
//    sendBuffer[1] = ui->spinBox->value();
}

void MainWindow::on_spinBox_2_editingFinished()
{
//    sendBuffer[2] = ui->spinBox_2->value();
}

void MainWindow::on_spinBox_3_editingFinished()
{
//    sendBuffer[3] = ui->spinBox_3->value();
}

void MainWindow::on_dial_valueChanged(int value)
{
    ui->spinBox->setValue(value);
}

void MainWindow::on_dial_2_valueChanged(int value)
{
    ui->spinBox_2->setValue(value);
}

void MainWindow::on_dial_3_valueChanged(int value)
{
    ui->spinBox_3->setValue(value);
}

void MainWindow::connectionCheckSlot(QSerialPort::SerialPortError error)
{
    ui->labelCon->setText("не подключён slot error");
    qDebug()<<"slot error";
    connectionTimer.start(1500);

}

void MainWindow::reconnectSerial()
{
    setupSerial(serial);
    qDebug()<<"reconectSerial";
    serial->error();
}

void MainWindow::on_butConnect_clicked()
{
    setupSerial(serial);
}

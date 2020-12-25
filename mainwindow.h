#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include <QVector>
#include <QDebug>
#include <QString>
#include <QPixmap>
#include <QtSerialPort/QSerialPort>
#include <QtSerialPort/QSerialPortInfo>
#include "ui_mainwindow.h"

#include <QFile>


namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

signals:
    void valueChanged();

private slots:
    void serialRecieveSlot();
    void realtimeDataSlot();


    void on_butFlow_clicked();
    void on_butPressure_clicked();
    void on_butVolume_clicked();
    void on_butApply_clicked();

    void connectionCheckSlot(QSerialPort::SerialPortError error);
    void reconnectSerial();

    void on_dial_valueChanged(int value);

    void on_spinBox_valueChanged(int arg1);

private:
    Ui::MainWindow *ui;
    QSerialPort *serial;
    QTimer dataTimer;
    QTimer connectionTimer;

    QVector<QVector<double>> dataToGrafs;
    double dataFromMassage[6];
    int pointPosition = 0;
    QByteArray sendBuffer;
    QByteArray textBuffer;

    bool firstEnter = true;

//    QStringList toFile;

    void setupSerial(QSerialPort* serial);
    void setupDataToGrafs(QVector<QVector<double>>& dataToGrafs);
    void setupAxisRect(QCustomPlot* customPlot, double* range);
    void setupAxis(QCPAxisRect* axisRect, double* range);

    void setDataFromMassage(QByteArray* message, double* dataFromMassage);
    void setDataToGrafs(int pointPosition, QVector<QVector<double>>& dataToGrafs, double* dataFromMassage);
    void setGapsDataToGrafs(int pointPosition, QVector<double>& dataToGrafs);


    void sendSerial();


};

#endif // MAINWINDOW_H

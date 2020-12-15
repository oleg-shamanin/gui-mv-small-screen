#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include <QVector>
#include <QDebug>
#include <QString>
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

private slots:
    void serialRecieveSlot();
    void realtimeDataSlot();


    void on_butFlow_clicked();
    void on_butPressure_clicked();
    void on_butVolume_clicked();
    void on_butApply_clicked();

    void on_spinBox_valueChanged(const QString &arg1);
    void on_spinBox_2_valueChanged(const QString &arg1);
    void on_spinBox_3_valueChanged(const QString &arg1);

    void on_spinBox_editingFinished();
    void on_spinBox_2_editingFinished();
    void on_spinBox_3_editingFinished();

    void on_dial_valueChanged(int value);
    void on_dial_2_valueChanged(int value);
    void on_dial_3_valueChanged(int value);

    void connectionCheckSlot(QSerialPort::SerialPortError error);
    void reconnectSerial();



    void on_butConnect_clicked();

private:
    Ui::MainWindow *ui;
    QSerialPort *serial;
    QTimer dataTimer;
    QTimer connectionTimer;

    QVector<QVector<double>> dataToGrafs;
    double dataFromMassage[5];
    int pointPosition = 0;
    QMap<QString, QByteArray> controlType;
    QByteArray sendBuffer;
    QByteArray textBuffer;

    bool firstEnter = true;

    QStringList toFile;

    void setupControlType(QMap<QString, QByteArray>& controlType);
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

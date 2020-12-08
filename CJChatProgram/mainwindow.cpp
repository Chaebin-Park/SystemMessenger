#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QObject>
#include <QString>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),


    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setWindowTitle("**CJ Chatting Program**");
}


MainWindow::~MainWindow()
{
    delete ui;
    delete socket;
}

bool is_registered = false;



void MainWindow::connect_server(){

    socket = new QTcpSocket(this);


    connect(socket, SIGNAL(connected()), this, SLOT(connect_server()));
    connect(socket, SIGNAL(readyRead()), this, SLOT(ready_to_read()));


}


void MainWindow::ready_to_read(){
    while(socket->canReadLine()){
        QString line = QString::fromUtf8(socket->readLine());
        line = line.left(line.length()-1);
        ui->list_text_view->addItem(QString().arg(line));
    }

    ui->list_text_view->scrollToBottom();
}

void MainWindow::on_btn_register_clicked(){
    if(!is_registered){
        connect_server();
        ui->list_text_view->addItem(QString("%2").arg(QString("Connecting..")).toUtf8());

        socket->connectToHost(ui->edit_address->text(), 3333);

        if(!socket->waitForConnected(5000)){	
            ui->list_text_view->addItem(QString("%2").arg(QString("Connect Error!")).toUtf8());
        }
        socket->write(QString(ui->edit_address->text()).toUtf8());
    }else{
        ui->list_text_view->addItem(QString("%2").arg(QString("You Are Registered")).toUtf8());
    }
    is_registered = true;

}

void MainWindow::on_btn_send_clicked(){

    QString msg = ui->edit_message->text().trimmed();
    QString name = ui->edit_name->text().toUtf8();

    socket->write(QString(name + "test : ddd" + msg).toUtf8());

    ui->list_text_view->addItem(QString(socket->readLine(50)));

    ui->edit_message->clear();
    ui->edit_message->setFocus();


}

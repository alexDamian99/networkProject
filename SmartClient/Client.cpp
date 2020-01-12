#include "Client.h"
#include "ui_Client.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <QDebug>
#include <QMessageBox>
#include <QCloseEvent>
Client::Client(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    lThread = new ListeningThread(this);
    connect(lThread, SIGNAL(UpdateList(char*)),
            this, SLOT(onUpdateList(char*)));

    connect(lThread, SIGNAL(UpdateLocation(char*)),
            this, SLOT(onNewLocation(char*)));

    connect(lThread, SIGNAL(InitLabels(char*, char*, char*)),
            this, SLOT(InitLabels(char*, char*, char*)));

    connect(lThread, SIGNAL(UpdateLimit(char*)),
            this, SLOT(onUpdateLimit(char*)));

    connect(lThread, SIGNAL(UpdateSpeed(char*)),
            this, SLOT(onUpdateSpeed(char*)));

    connect(lThread, SIGNAL(DestinationReached()),
            this, SLOT(onDestinationReached()));
}

Client::~Client()
{
    delete lThread;
    delete ui;
}


bool Client::conn(){//the listening thread will start after the connection was established
    this->listen_socket = socket(AF_INET, SOCK_STREAM, 0);
    this->info_socket = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in server;

    server.sin_addr.s_addr = inet_addr("127.0.0.1");
    server.sin_port = htons(2038);
    server.sin_family = AF_INET;

    int conn_code = ::connect(this->info_socket, (struct sockaddr*)&server, sizeof(struct sockaddr));
    int conn_code2 = ::connect(this->listen_socket, (struct sockaddr*)&server, sizeof(struct sockaddr));

    if(conn_code == -1 || conn_code2 == -1){
        ui->listWidget->addItem("Eroare la stabilire conexiune!");
        return false;
    }

    form = new Dialog(this, info_socket);
    int connect = form->exec();
    if(connect == 1){
        ui->listWidget->addItem("Conectare reusita!");
        ui->listWidget->addItem("Bine ati venit!");
        ui->listWidget->addItem("Aici veti primi informatii relevante locatiei curente.");
        lThread->setDescriptor(listen_socket);
        lThread->start();
        return true;
    }
    return false;

}


void Client::on_pushButton_clicked()
{
    QString message = ui->lineEdit->text();
    ui->lineEdit->setText("");
    if(message == "accident"){
        message.append("&&");
        message.append(lThread->getLocation());

        ::write(info_socket, message.toStdString().c_str(), message.size());
    }


}

void Client::onUpdateList(char*msg){
    ui->listWidget->addItem(msg);
}

void Client::onNewLocation(char*s){
    ui->location_name->setText(s);
}

void Client::InitLabels(char*start, char*path, char*destination){
    ui->dest_label->setText(destination);
    ui->start_label->setText(start);
    ui->path_label->setText(path);
}

void Client::onUpdateLimit(char*s){
    char*x = strtok(s, ":");
    x = strtok(NULL, ":");
    ui->limit_label->setText(x);
}

void Client::onUpdateSpeed(char*s){
    ui->speed_label->setText(s);
}

void Client::onDestinationReached(){
    ui->lineEdit->setDisabled(true);
    ui->listWidget->addItem("Ati ajuns la destinatie");
    QMessageBox::information(this, "Info", "Ati ajuns la destinatie!");
}


void Client::closeEvent(QCloseEvent *event)
{
    write(info_socket, "quit", 5);
    event->accept();
}

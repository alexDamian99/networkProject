#include "login.h"
#include "ui_login.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <QDebug>


Login::Login(QWidget *parent, int sd) :
    QDialog(parent),
    ui(new Ui::Login)
{
    ui->setupUi(this);
    info_descriptor = sd;

}

Login::~Login()
{
    delete ui;
}

void Login::on_pushButton_login_clicked()
{
    write(info_descriptor, "login", 6);
    char*response = new char[15];
    read(info_descriptor, response, 10);

    if(strcmp(response, "ok") == 0){
        QString user = ui->lineEdit_user->text();
        QString pass = ui->lineEdit_pass->text();
        ui->lineEdit_pass->setText("");
        ui->lineEdit_user->setText("");
        char*credentials = new char[100];
        credentials[0] = '\0';
        strcat(credentials, user.toStdString().c_str());
        strcat(credentials, "&&");
        strcat(credentials, pass.toStdString().c_str());

        write(info_descriptor, credentials, strlen(credentials) + 1);

        char response[100];
        read(info_descriptor, response, 100);

        if(strcmp(response, "OK") == 0){
            hide();
            this->done(1);
        }else{

            ui->label_info->setText("Conectare esuata");

        }
    }else{
        ui->label_info->setText("Error.");
    }
}



void Login::on_pushButton_register_clicked()
{
    reg_form = new Register(this, info_descriptor);
    hide();
    setResult(reg_form->exec());
}

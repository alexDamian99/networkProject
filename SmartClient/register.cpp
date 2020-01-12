#include "register.h"
#include "ui_register.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <QDebug>
#include "login.h"
Register::Register(QWidget *parent, int sd) :
    QDialog(parent),
    ui(new Ui::Register)
{
    ui->setupUi(this);
    info_descriptor = sd;

}

Register::~Register()
{
    delete ui;
}

void Register::on_pushButton_register_clicked()
{
    QString user = ui->lineEdit_user->text();
    QString pass = ui->lineEdit_pass->text();
    char options[4] = {'0', '0', '0', '\0'};
    if(ui->checkBox_vreme->checkState() == Qt::Checked){
        options[0] = '1';
    }
    if(ui->checkBox_sport->checkState() == Qt::Checked){
        options[1] = '1';
    }
    if(ui->checkBox_peco->checkState() == Qt::Checked){
        options[2] = '1';
    }
    printf("sending %s\n", options);
    bool valid = true;
    for(int i = 0; i < user.size() && valid; i++){
        if(user[i] == ' '){
            valid = false;
        }
    }
    for(int i = 0; i < pass.size() && valid; i++){
        if(pass[i] == ' '){
            valid = false;
        }
    }
    if(!valid || user.size() == 0 || pass.size() == 0){
        ui->label_info2->setText("Introduceti un nume de utilizator si o parola valida.");
    }else{
        write(info_descriptor, "register", 9);
        char*response = new char[15];
        read(info_descriptor, response, 10);
        qDebug()<<response<<"\n";
        if(strcmp(response, "ok") == 0){//trimit catre server nume parola si optiuni
            ui->lineEdit_pass->setText("");
            ui->lineEdit_user->setText("");
            char*credentials = new char[100];
            credentials[0] = '\0';
            strcat(credentials, user.toStdString().c_str());
            strcat(credentials, "&&");
            strcat(credentials, pass.toStdString().c_str());
            strcat(credentials, "&&");
            strcat(credentials, options);

            write(info_descriptor, credentials, strlen(credentials) + 1);

            char response[100];
            read(info_descriptor, response, 100);

            if(strcmp(response, "OK") == 0){
                hide();
                this->done(1);
            }else{

                ui->label_info2->setText("Inregistrare esuata. Numele de utilizator exista deja.");
            }
        }else{
            ui->label_info2->setText("Eroare");
        }
    }
}

void Register::on_pushButton_login_clicked()
{
    login = new Login(this, info_descriptor);
    hide();
    setResult(login->exec());
}

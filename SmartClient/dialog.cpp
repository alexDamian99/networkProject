#include "dialog.h"
#include "ui_dialog.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <QDebug>

Dialog::Dialog(QWidget *parent, int sd) :
    QDialog(parent),
    ui(new Ui::Dialog)
{
    ui->setupUi(this);
    info_descriptor = sd;
}

Dialog::~Dialog()
{
    delete ui;
}

void Dialog::on_pushButton_login_clicked()
{
    //open up the login dialog
    login = new Login(this, info_descriptor);
    hide();
    setResult(login->exec());
}

void Dialog::on_pushButton_register_clicked()
{
    register_dialog = new Register(this, info_descriptor);
    hide();
    setResult(register_dialog->exec());
}

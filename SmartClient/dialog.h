#ifndef DIALOG_H
#define DIALOG_H

#include <QDialog>
#include "login.h"
#include "register.h"
namespace Ui {
class Dialog;
}

class Dialog : public QDialog
{
    Q_OBJECT

public:
    explicit Dialog(QWidget *parent = nullptr, int sd = 1);
    ~Dialog();

private slots:
    void on_pushButton_login_clicked();

    void on_pushButton_register_clicked();

private:
    Ui::Dialog *ui;
    Login *login;
    Register *register_dialog;
    int info_descriptor;
};

#endif // DIALOG_H

#ifndef LOGIN_H
#define LOGIN_H

#include <QDialog>
#include "register.h"
namespace Ui {
class Login;
}

class Login : public QDialog
{
    Q_OBJECT

public:
    explicit Login(QWidget *parent = nullptr, int info_descriptor = 1);
    ~Login();

private slots:
    void on_pushButton_login_clicked();

    void on_pushButton_register_clicked();

private:
    Ui::Login *ui;
    int info_descriptor;
    Register *reg_form;

};

#endif // LOGIN_H

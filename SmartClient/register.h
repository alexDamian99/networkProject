#ifndef REGISTER_H
#define REGISTER_H

#include <QDialog>

namespace Ui {
class Register;
}

class Register : public QDialog
{
    Q_OBJECT

public:
    explicit Register(QWidget *parent = nullptr, int sd = 1);
    ~Register();

private slots:
    void on_pushButton_register_clicked();

    void on_pushButton_login_clicked();

private:
    Ui::Register *ui;
    int info_descriptor;
    QDialog * login;

};

#endif // REGISTER_H

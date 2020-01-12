#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "listeningthread.h"
#include "dialog.h"
QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class Client : public QMainWindow
{
    Q_OBJECT

public:
    Client(QWidget *parent = nullptr);
    ~Client();
    bool conn();
    void run();

private slots:
    void on_pushButton_clicked();
    void onUpdateList(char*);
    void onNewLocation(char*);
    void InitLabels(char*, char*, char*);
    void onUpdateLimit(char*);
    void onUpdateSpeed(char*);
    void onDestinationReached();




private:
    Ui::MainWindow *ui;
    ListeningThread*lThread;
    int listen_socket, info_socket;
    Dialog *form;
    void closeEvent(QCloseEvent*);
};
#endif // MAINWINDOW_H

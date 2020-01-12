#include "Client.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    Client w;
    if(w.conn() == false){
        a.closeAllWindows();
        return 1;
    }
    w.show();
    return a.exec();
}

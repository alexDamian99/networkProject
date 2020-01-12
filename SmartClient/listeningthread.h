#ifndef LISTENINGTHREAD_H
#define LISTENINGTHREAD_H

#include <QObject>
#include <QThread>

class ListeningThread : public QThread
{
    Q_OBJECT
public:
    ListeningThread(QObject *parent);
    void setDescriptor(int);
    void run();
    char* getLocation();
private:
    bool close;
    struct muchie{
        int cap[2];
        char*nume;
        double lungime;
        int nr_locuinte[2];
    }strada[100];
    int sd, nr_strazi, nod_max;
    double speed;
    double distanta_parcursa_pe_strada;
    int viz[100], path[100], nr_strazi_din_traseu;
    void obtain_city_map();
    void obtain_path(int start, int end);
    int dfs(int, int);
    void add_strada(char*);
    char* itoa(int);
    int get_street(int);
    char*location;
signals:
    void UpdateLocation(char*);
    void UpdateLimit(char*);
    void UpdateSpeed(char*);
    void UpdateList(char*);
    void InitLabels(char*, char*, char*);
    void DestinationReached();
};

#endif // LISTENINGTHREAD_H

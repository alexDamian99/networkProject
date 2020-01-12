#include "listeningthread.h"
#include <QtCore>
#include <sys/poll.h>
#include <unistd.h>
#include <QtDebug>
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <time.h>
#include <sys/select.h>
#include <poll.h>
#include <cstring>

ListeningThread::ListeningThread(QObject* parent):QThread(parent)
{
    this->close = false;
    this->nr_strazi = 0;
    this->speed = 20 + rand() % 100;
    this->location = new char[100];
    nod_max = 0;
    distanta_parcursa_pe_strada = 0;
}

void ListeningThread::setDescriptor(int sd){
    this->sd = sd;
}

void ListeningThread::run(){
    ////////////////////////
    /////Stabilire ruta/////
    ////////////////////////
    this->obtain_city_map();

    int start_node, end_node;
    srand(time(NULL));
    start_node = 1 + rand() % nod_max;
    end_node = 1 + rand() % nod_max;
    while(end_node == start_node){
       end_node = 1 + rand() % nod_max;
    }

    this->obtain_path(start_node, end_node);

    if(nr_strazi_din_traseu == 1){
        emit InitLabels(strada[get_street(0)].nume, strada[get_street(0)].nume, strada[get_street(0)].nume);
    }else{
        char*traseu = new char[512];
        memset(traseu, 0, 512);
        traseu[0] = '\0';
        for(int k = 0; k < nr_strazi_din_traseu - 1; k++){

            strcat(traseu, strada[get_street(k)].nume);
            strcat(traseu, "-");
        }
        emit InitLabels(strada[get_street(0)].nume, traseu, strada[get_street(this->nr_strazi_din_traseu - 2)].nume);
    }


    //pregatim structura folosita la poll
    struct pollfd fds[1];
    memset(fds, 0, sizeof(fds));
    fds[0].fd = sd;
    fds[0].events = POLLIN;


    int time = 2000;//2 seconds;

    char*msg = new char[2048];
    //////////////////////////////
    /// Comunicare cu serverul ///
    //////////////////////////////

    int k = 0;
    int current_street = get_street(k);
    double lungime_bloc = strada[current_street].lungime /
            (strada[current_street].nr_locuinte[1] - strada[current_street].nr_locuinte[0] + 1);

    while(this->close == false){
        int poll_code = poll(fds, 1, time);
        if(poll_code == 0){//poll ul a expirat, voi actualiza distanta totala parcursa pe strada si o voi trimite serverului

            this->speed = 20 + rand() % 100;
            emit UpdateSpeed(itoa(speed));

            distanta_parcursa_pe_strada += (speed*2)/3600;

            if(distanta_parcursa_pe_strada >= strada[current_street].lungime){
                k++;
                if(k == nr_strazi_din_traseu - 1){//am ajuns la destinatia clientului
                    close = true;
                    write(sd, "Succes", 7);
                    emit DestinationReached();
                    break;
                }else{
                    distanta_parcursa_pe_strada = distanta_parcursa_pe_strada - strada[current_street].lungime;
                    current_street = get_street(k);
                    lungime_bloc = strada[current_street].lungime /
                            (strada[current_street].nr_locuinte[1] - strada[current_street].nr_locuinte[0] + 1);
                }
            }
            //scriem locatia si viteza cu care circula masina catre server
            char*locatie = new char[512];
            memset(locatie, 0, 512);
            strcat(locatie, itoa(speed));
            strcat(locatie, "&&locatie&&");
            strcat(locatie, strada[current_street].nume);
            int nr_bloc = (distanta_parcursa_pe_strada / lungime_bloc) + 1;
            strcat(locatie, "&&");
            strcat(locatie, itoa(nr_bloc));
            write(sd, locatie, strlen(locatie) + 1);


            //scriem locatia pentru client(GUI)
            memset(locatie, 0, 512);
            strcpy(locatie, strada[current_street].nume);
            strcat(locatie, " ");
            strcat(locatie, itoa(nr_bloc));
            strcpy(this->location, locatie);
            emit UpdateLocation(locatie);
        }
        else {//s-a intamplat ceva pe socket
            memset(msg, 0, 2048);
            msg[0] = '\0';
            int codr = read(fds[0].fd, msg, 2048);
            if(codr < 0){
                if(errno != EWOULDBLOCK){
                    perror("Eroare la citire din socket");
                    break;
                }
            }else if(codr == 0){
                printf("Conexiune inchisa!\n");
                break;
            }
            if(strlen(msg) > 0){

                char*x = strtok(msg, "&&");
                while(x != NULL){
                    char*info = new char[100];
                    memset(info, 0, 100);
                    info = strdup(x);
                    unsigned int i = 0;
                    char type[50];
                    while(info[i] != ':' && i < strlen(info)){
                        type[i] = info[i];
                        i++;
                    }
                    type[i] = '\0';
                    if(strcmp(type, "limita") == 0){
                        emit UpdateLimit(info);
                    }else{
                        emit UpdateList(info);
                    }
                    x = strtok(NULL, "&&");

                }
            }

        }
    }
}

void ListeningThread::obtain_city_map(){
    int fd = open("/home/alex/Documents/Retele/project/SmartClient/oras.txt", O_RDONLY);
    if(fd < 0){
        perror("Eroare la deschidere fisier");
        ::exit(errno);
    }


    int k = 0;
    char*line;
    line = (char*)malloc(100);


    while(1){
        unsigned char ch;
        int codr = read(fd, &ch, 1);
        if(codr == 0){
            break;
        }
        else if(codr == -1){
            perror("Eroare la citire din fisier!");
            ::exit(errno);
        }
        if(ch == '\n'){
            line[k] = '\0';
            if(k >= 1){
                add_strada(line);
            }
            k = 0;
        }else {
            line[k++] = ch;
        }
    }

    ::close(fd);
}

void ListeningThread::add_strada(char*s){
    int i = 0, t = 0;
    char nr[10];
    //adaugare capete
    for(int j = 0; j <= 1; j++){
        while(s[i] != ' '){
            nr[t] = s[i];
            t++; i++;
        }
        nr[t] = '\0';
        strada[nr_strazi].cap[j] = atoi(nr);
        if(atoi(nr) > nod_max){
            nod_max = atoi(nr);
        }
        t = 0;i++;
    }

    //adaugare numere de pe strada
    for(int j = 0; j <= 1; j++){
        while(s[i] != ' '){
            nr[t] = s[i];
            t++; i++;
        }
        nr[t] = '\0';
        strada[nr_strazi].nr_locuinte[j] = atoi(nr);
        t = 0;i++;
    }

    //lungime
    while(s[i] != ' '){
        nr[t] = s[i];
        t++; i++;
    }
    nr[t] = '\0';
    strada[nr_strazi].lungime = atoi(nr);
    t = 0;i++;

    //nume strada
    strada[nr_strazi].nume = (char*)malloc(100);
    while(i < (int)strlen(s)){
        strada[nr_strazi].nume[t] = s[i];
        t++; i++;
    }
    strada[nr_strazi].nume[t] = '\0';
    t = 0;

    nr_strazi++;

}

void ListeningThread::obtain_path(int start, int end){
    nr_strazi_din_traseu = 0;
    for(int i = 0; i < 100; i++){
        viz[i] = 0;
        path[i] = 0;
    }
    dfs(start, end);
}

int ListeningThread::dfs(int x, int end){
    viz[x] = 1;
    path[nr_strazi_din_traseu++] = x;
    if(end == x){
        return 1;
    }else {
        int found = 0;
        for(int i = 0; i < nr_strazi && found == 0; i++){
            if(strada[i].cap[0] == x && viz[strada[i].cap[1]] == 0){
                found = dfs(strada[i].cap[1], end);
            }else if(strada[i].cap[1] == x && viz[strada[i].cap[0]] == 0){
                found = dfs(strada[i].cap[0], end);
            }
        }
        if(found == 0){
            nr_strazi_din_traseu--;
        }
        return found;
    }
}
///returneaza numarul de ordine din vectorul strada[] pentru al k lea element din path
int ListeningThread::get_street(int k){
    for(int i = 0; i < nr_strazi; i++){
        if((strada[i].cap[0] == path[k] && strada[i].cap[1] == path[k+1]) ||
            (strada[i].cap[1] == path[k] && strada[i].cap[0] == path[k+1])){
            return i;
        }
    }
    return -1;
}

char* ListeningThread::itoa(int a){
    char*number = (char*)(malloc(11));
    memset(number, 0, 11);
    int k = 0;

    if(a == 0){
        k = 1;
    }
    int m = a;
    while(m != 0){
        m/=10;
        k++;
    }

    if(a < 0){
        number[0] = '-';
        a = a * -1;
        while(k >= 1){
            number[k] = a%10 + '0';
            a/=10;
            k--;
        }
    }
    else {
        k--;
        while(k >= 0){
            number[k] = a%10 + '0';
            a/=10;
            k--;
        }
    }

    return number;
}

char* ListeningThread::getLocation(){
    return this->location;
}


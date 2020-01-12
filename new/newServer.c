#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <stdlib.h>
#include <signal.h>
#include <pthread.h>
#include <stdint.h>  //pt intptr_t
#include <fcntl.h>
#include <sys/select.h>
#include <poll.h>
#define PORT 2038
#define INPUT_SIZE 50
//TODO GET VITEZA CLIENT
typedef struct threadData{
    int thread_id, broadcast_fd, client_fd;
}thData;
int total_threads, accident, trigger;

struct acc{
    char*locatie;
}accidente[100];
int nr_total_accidente;

struct muchie{
    int cap[2];
    char*nume;
    int lungime;
    int limita;
    int nr_locuinte[2];
}strada[100];
int nr_strazi;

struct ev{
    char*street;
    int number;
    int type[3];
    char*message;
}event[100];
int total_events;
///integer to ascii
char* itoa(int a){
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

///imparte linia citita in ordinea : [capete, numere de pe strada, limita viteza, lungime, nume strada]
void add_street(char*s){//capete, numere de pe strada(interval), limita, lungime, nume
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

    //limita
    while(s[i] != ' '){
        nr[t] = s[i];
        t++; i++;
    }
    nr[t] = '\0';
    strada[nr_strazi].limita = atoi(nr);
    t = 0;i++;
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
    while(i < strlen(s)){
        strada[nr_strazi].nume[t] = s[i];
        t++; i++;
    }
    strada[nr_strazi].nume[t] = '\0';
    t = 0;

    nr_strazi++;

}

void obtain_city_map(){
    int fd = open("oras.txt", O_RDONLY);
    if(fd == -1){
        perror("Eroare la deschidere fisier");
        exit(errno);
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
            exit(errno);
        }
        if(ch == '\n'){
            line[k] = '\0';
            if(k >= 1){
                add_street(line);
            }
            k = 0;
        }else {
            line[k++] = ch;
        }
    }
    close(fd);
}


void add_event(char*s){
    int i = 0;
    ////////////////////////////////////
    /////Obtinere locatie eveniment/////
    ////////////////////////////////////

    //obtinem numele strazii
    event[total_events].street = (char*)malloc(100);
    while((s[i] > '9' || s[i] < '0') && s[i] != '!'){
        event[total_events].street[i] = s[i];
        i++;
    }
    i--;
    event[total_events].street[i] = '\0';
    char*aux = (char*)malloc(100);
    s = s + i + 1;
    strcpy(aux, s);
    char*number = strtok(aux, " ");
    //obtinem numarul de pe strada(daca eventul nu are numar(ex:vremea) in fisier va fi prezent ! semnificand toata muchia)
    if(number[0] == '!'){
        event[total_events].number = -1;
    }else {
        event[total_events].number = atoi(number);
    }
    number = strtok(NULL, " ");
    //identificam tipul evenimentului (0 = vreme, 1 = sport, 2 = peco)
    for(int i = 0; i < 3; i++){
        event[total_events].type[i] = 0;
    }
    event[total_events].type[atoi(number)] = 1;

    //obtinem mesajul(evenimentul)
    event[total_events].message = (char*)malloc(100);
    char*msg = strtok(NULL, "\0");
    strcpy(event[total_events].message, msg);
    total_events++;
}

void load_events(){
    int fd = open("events.txt", O_RDONLY);
    if(fd == -1){
        perror("Eroare la deschidere fisier");
        exit(errno);
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
            exit(errno);
        }
        if(ch == '\n'){
            line[k] = '\0';
            if(k >= 1){
                add_event(line);
            }
            k = 0;
        }else {
            line[k++] = ch;
        }
    }
    close(fd);
}


int check_login_credentials(char*credentials, char options[4]){// -1 daca e eroare, 1 daca am gasit user + pass, 0 altfel
    char user[50], pass[50];
    int k = 0, m = 0;
    while(credentials[k] != '&'){
        user[k] = credentials[k];
        k++;
    }
    user[k] = '\0';

    while(credentials[k] == '&'){
        k++;
    }
    while(credentials[k] != '&' && k < strlen(credentials)){
        pass[m++] = credentials[k++];
    }
    pass[m] = '\0';

    int fd = open("useri.txt", O_RDWR);
    if(fd < 0){
        perror("Eroare la deschidere fisier");
        return -1;
    }
    int found = 0;
    k = 0;
    char word1[110];

    while(!found){//parcurg fisierul
        unsigned char ch;
        int codr = read(fd, &ch, 1);
        if(codr == 0){  //end of file
            break;
        }else if(codr == -1){
            perror("Eroare la citire");
            close(fd);
            return -1;
        }
        if(ch != ' ' && ch != '\n'){//obtinem user-ul de pe o linie
            word1[k++] = ch;
        }
        if(ch == ' '){
            word1[k] = '\0';
            if(strcmp(word1, user) != 0){//daca e un user diferit, sarim linia
                while(ch != '\n'){//parcurgem linia pana la sfarsit
                    read(fd, &ch, 1);
                }
                k = 0;
            }else{ // daca am gasit user-ul verificam parola
                int t = 0;
                char password[50];

                read(fd, &ch, 1);
                password[t++] = ch;
                while(ch != ' '){//obtinem parola
                    read(fd, &ch, 1);
                    password[t++] = ch;
                }
                password[t-1] = '\0';

                if(strcmp(pass, password) == 0){//verificam parola
                    //daca parola este buna, vom citi optiunile
                    
                    for(int o = 0; o < 3; o++){
                        read(fd, &ch, 1);
                        options[o] = ch;
                    }
                    options[3] = '\0';
                    close(fd);
                    return 1;
                }
                while(ch != '\n'){//parcurgem linia pana la sfarsit
                    read(fd, &ch, 1);
                }
                k = 0;
            }
        }
    }
    close(fd);
    return found;
}

int check_register_credentials(char*credentials, char options[4]){// -1 daca e eroare, 1 daca am gasit user-ul, 0 daca nu l-am gasit
    char user[50];
    int k = 0;
    while(credentials[k] != '&'){
        user[k] = credentials[k];
        k++;
    }
    user[k] = '\0';

    int fd = open("useri.txt", O_RDWR);
    if(fd < 0){
        perror("Eroare la deschidere fisier");
        return -1;
    }
    k = 0;
    char word1[110];
    while(1){//parcurg fisierul
        unsigned char ch;
        int codr = read(fd, &ch, 1);
        if(codr == 0){  //end of file
            close(fd);
            return 0;
        }else if(codr == -1){
            perror("Eroare la citire");
            close(fd);
            return -1;
        }
        if(ch != ' ' && ch != '\n'){//obtinem user-ul de pe o linie
            word1[k++] = ch;
        }
        if(ch == ' '){
            word1[k] = '\0';
            if(strcmp(word1, user) == 0){// daca am gasit doar usernameul => return 1
                close(fd);
                return 1;
            }else{ // daca nu am gasit userul, trecem la linia urmatoare.
                while(ch != '\n'){
                    read(fd, &ch, 1);
                }
                k = 0;
            }
        }
    }
    word1[k] = '\0';
    if(strcmp(word1, user) == 0){
        close(fd);
        return 1;
    }else{
        close(fd);
        return 0;
    }
    close(fd);
    return 0;
}

void add_user(char*credentials, char options[4]){
    
    int fd = open("useri.txt", O_RDWR);
    if(fd < 0){
        perror("Eroare la deschidere fisier");
        return ;
    }
    char user[50], pass[50];
    int k = 0, m = 0;
    while(credentials[k] != '&'){
        user[k] = credentials[k];
        k++;
    }
    user[k] = '\0';
    lseek(fd, 0, SEEK_END);
    write(fd, user, strlen(user));
    write(fd, " ", 1);
    
    while(credentials[k] == '&'){
        k++;
    }
    while(credentials[k] != '&' && k < strlen(credentials)){
        pass[m++] = credentials[k++];
    }
    pass[m] = '\0';
    write(fd, pass, strlen(pass));
    write(fd, " ", 1);
    while(credentials[k] == '&'){
        k++;
    }
    int b = 0;
    while(k < strlen(credentials)){
        options[b] = credentials[k];
        b++; k++;
    }
    options[b] = '\0';
    
    
    
   
    write(fd, options, 3);
    write(fd, "\n", 1);
    
    close(fd);
}

int register_user(int sd, int id, char options[4]){
    /// check ///
    char credentials[100]; 
    if(read(sd, credentials, 100) < 0){
        perror("Eroare la citire mesaj");
        return -1;
    }
    int check_code = check_register_credentials(credentials, options);
    
    if(check_code == 1){
        if(write(sd, "NOT OK", 7) < 0){
            perror("Eroare la scriere mesaj");
            return -1;
        }
        printf("[thread %d]Clientul nu s-a inregistrat cu succes!\n", id);
        return 0;
    }else if(check_code == 0){//add user
        add_user(credentials, options);
        printf("[thread %d]Clientul s-a inregistrat cu succes!\n", id);
        if(write(sd, "OK", 3) < 0){
            perror("Eroare la scriere mesaj");
            return -1;
        }
        return 1;            
    }
    return 0;
}
int login(int sd, int id, char options[4]){
    char credentials[100];
    if(read(sd, credentials, 100) < 0){
        perror("Eroare la citire mesaj");
        return -1;
    }
    

    /// check ///
    int check_code;
    if(strlen(credentials) <= 2){
        check_code = 0;
    }
    else check_code = check_login_credentials(credentials, options);
    if(check_code == 0){
        if(write(sd, "NOT OK", 7) < 0){
            perror("Eroare la scriere mesaj");
            return -1;
        }
        printf("[thread %d]Clientul nu s-a logat cu succes!\n", id);
        return 0;
    }else if(check_code == 1){
        if(write(sd, "OK", 3) < 0){
            perror("Eroare la scriere mesaj");
            return -1;
        }
        printf("[thread %d]Clientul s-a logat cu succes!\n", id);
        return 1;
    }
    return 0;
}

int identify(int sd, int id, char options[4]){
    int identified = 0;
    while(identified == 0){
        char msg[100];
        if(read(sd, msg, 100) < 0){
            perror("Eroare la citire mesaj");
            return -1;
        }
        if(write(sd, "ok", 3) < 0){
            perror("Eroare la scriere mesaj");
            return -1;
        }
        printf("[thread %d]Am primit >%s< de la client\n", id, msg);
        if(strcmp(msg, "register") == 0){
            identified = register_user(sd, id, options);
        }else if(strcmp(msg, "login") == 0){
            identified = login(sd, id, options);
        }else {//eroare
            identified = -1;
        }
    }
    return identified;
}
///obtine strazile vecine cu strada cu numarul de ordine din vectorul strada[] ref_number 
char**get_neighbours(int ref_number, int*k){
    char**neighbours = (char**)malloc(100);
    *k = 0;
    for(int i = 0; i < nr_strazi; i++){
        if((strada[i].cap[0] == strada[ref_number].cap[0] 
        || strada[i].cap[0] == strada[ref_number].cap[1] 
        || strada[i].cap[1] == strada[ref_number].cap[0] 
        || strada[i].cap[1] == strada[ref_number].cap[1])){
            neighbours[*k] = (char*)malloc(200);
            strcpy(neighbours[*k], strada[i].nume);
            (*k)++;
        }
    }

    return neighbours;
}
///gaseste evenimentele din aproape de strada street_name si le returneaza sub forma unui char*
char* get_events(char*street_name, int number_on_street, int event_sent[100], char options[4]){
    int found = 0, ref_number;
    //obtinere indicele strazii clientului din vectorul strada[]
    for(int i = 0; i < nr_strazi && found == 0; i++){
        if(strcmp(strada[i].nume, street_name) == 0 
            && number_on_street >= strada[i].nr_locuinte[0]                                    
            && number_on_street <= strada[i].nr_locuinte[1]){
                found = 1;
                ref_number = i;
            }
    }
    char*events = (char*)malloc(200);
    events[0] = '\0';
    strcat(events, "limita:");
    strcat(events, itoa(strada[ref_number].limita));
    strcat(events, "&&");
    int k;
    char**neighbours = get_neighbours(ref_number, &k);
    for(int i = 0; i < total_events; i++){
        for(int j = 0; j < k; j++){//verifica vecinii
            if(strcmp(event[i].street, neighbours[j]) == 0 && event_sent[i] == 0){//caut un eveniment netrimis situat in apropiere

                if(event[i].type[0] == 1 && options[0] == '1'){//vreme
                    strcat(events, "Vreme:");
                    strcat(events, event[i].message);
                    strcat(events, " pe strada ");
                    strcat(events, event[i].street);
                    if(event[i].number != -1){
                        strcat(events, " nr. ");
                        strcat(events, itoa(event[i].number));
                    }
                    strcat(events, "&&");
                    event_sent[i] = 1;
                }else if(event[i].type[1] == 1 && options[1] == '1'){//sport
                    strcat(events, "Sport:");
                    strcat(events, event[i].message);
                    strcat(events, " pe strada ");
                    
                    strcat(events, event[i].street);
                    if(event[i].number != -1){
                        strcat(events, " nr. ");
                        strcat(events, itoa(event[i].number));
                    }
                    strcat(events, "&&");
                    event_sent[i] = 1;
                }else if(event[i].type[2] == 1 &&options[2] == '1'){//pret
                    strcat(events, "Pret statie peco:");
                    strcat(events, event[i].message);
                    strcat(events, " pe strada ");
                    
                    strcat(events, event[i].street);
                    if(event[i].number != -1){
                        strcat(events, " nr. ");
                        strcat(events, itoa(event[i].number));
                    }
                    strcat(events, "&&");
                    event_sent[i] = 1;
                }
            }
        }
    }
    return events;
}

void* routine(void*arg){
    thData thread = *((thData*)arg);
    int sd = thread.client_fd;
    printf("[thread %d]Thread inceput.\n", thread.thread_id);
    char options[3];
    int idf = identify(sd, thread.thread_id, options);
    if(idf == -1){
        printf("[thread %d] Inchidere thread.\n", thread.thread_id);
        close((uintptr_t)arg);
        return NULL;
    }
    char msg[100];
    int broadcast_descriptor = thread.broadcast_fd, quit = 0;
    int nr_accidente_din_thread = 0;
    int client_speed = 0;
    struct pollfd fds[1];
    memset(fds, 0, sizeof(fds));
    fds[0].fd = sd;
    fds[0].events = POLLIN;

    struct pollfd location_fd[1];
    memset(location_fd, 0, sizeof(location_fd));
    location_fd[0].fd = broadcast_descriptor;
    location_fd[0].events = POLLIN;


    int time = 2000;//2 seconds;
    int event_sent[100];
    for(int i = 0 ; i < 100; i++){
        event_sent[i] = 0;
    }
    while(quit == 0){
        printf("[thread %d] Stau si ascult evenimente de la client\n", thread.thread_id);
        
        int poll_code = poll(fds, 1, time); //al doilea parametru e folosit pentru a spune cate elemente are multimea de descriptori
        if(poll_code < 0){
            perror("Eroare la poll");
            break;
        }
        if(poll_code == 0){//timeout
            printf("[thread %d]accident poll timedout\n", thread.thread_id);
        }
        else {//s-a intamplat ceva pe socket
            memset(msg, 0, 100);
            int codr = read(fds[0].fd, msg, 100);
            if(codr < 0){
                if(errno != EWOULDBLOCK){
                    perror("Eroare la citire din socket");
                    break;
                }
            }else if(codr == 0){
                printf("[thread %d] Conexiune inchisa de catre client\n", thread.thread_id);
                break;
            }


            printf("[thread %d] am primit de la client mesajul >%s<\n", thread.thread_id, msg);
            //mesajele primite de la client catre server tratate aici sunt de tipul fi de tipul
            // "accident&&nume_strada nr_de_pe_strada
            if(strcmp(msg, "quit") == 0){
                quit = 1;
                break;
            }else{
                char*x = strtok(msg, "&&");
                x = strtok(NULL, "&&");
                accidente[nr_total_accidente].locatie = (char*)malloc(512);
                strcpy(accidente[nr_total_accidente].locatie, x);
                nr_total_accidente++;
            }

            
        }
        char*total_accidente = (char*)malloc(2048);
        total_accidente[0] = '\0';
        while(nr_accidente_din_thread < nr_total_accidente){//la accidente trimit la toti clientii indiferent de locatia lor
            strcat(total_accidente, "Accident:");
            strcat(total_accidente,  accidente[nr_accidente_din_thread].locatie);
            strcat(total_accidente, "&&");
            nr_accidente_din_thread++;
        }
        printf("[thread %d] nr accidente din thread = %d, nr total = %d\n", thread.thread_id, nr_accidente_din_thread, nr_total_accidente);
        printf("[thread %d] scriu catre client >%s<\n", thread.thread_id, total_accidente);
        if(write(broadcast_descriptor, total_accidente, strlen(total_accidente)) < 0){
            perror("Eroare la write");
            quit = 1;
            break;
        }

        //astepta locatia trimisa de client
        poll_code = poll(location_fd, 1, 1000);
        if(poll_code == 1){//clientul a trimis locatia sau un cod de succes, daca a ajuns la destinatie
            memset(msg, 0, 100);
            int codr = read(location_fd[0].fd, msg, 100);
            
            if(codr <= 0){
                if(errno != EWOULDBLOCK){
                    perror("Eroare la citire din socket");
                    break;
                }
            }
            if(strcmp(msg, "Succes") == 0){//clientul a ajuns la destinatie
                quit = 1;
            }else{
                char* aux = (char*)malloc(100);
                memset(aux, 0, 100);
                strcpy(aux, msg);
                char * x = (char*)malloc(100);
                strcpy(x, strtok(aux, "&&"));
                client_speed = atoi(x);
                strtok(NULL, "&&");
                char*street = strtok(NULL, "&&");
                char*number = strtok(NULL, "&&");
                int i = 0;
                while(number[i] <= '9' && number[i] >= '0' && i <= strlen(number))i++;
                number[i] = '\0';

                int street_number = atoi(number);
                printf("[thread %d] viteza client = %d, locatie client = %s %d\n", thread.thread_id, client_speed, street, street_number);
                char* events = get_events(street, street_number, event_sent, options);//TODO adauga parametru si options(alea alese de utilizator la inregistrare)
                printf("[thread %d] scriu catre client %s\n", thread.thread_id, events);
                if(write(broadcast_descriptor, events, strlen(events) + 1) < 0){
                    perror("Eroare la write");
                    quit = 1;
                    break;
                }
            }
        }
    }
    
    

    printf("[thread %d] Inchidere thread\n", thread.thread_id);;
    total_threads--;
    close(broadcast_descriptor);
    close(sd);
    close((uintptr_t)arg);
    return NULL;
}

int main(int argc, char*argv[]){
    obtain_city_map();
    load_events();
    
    int sd = socket(AF_INET, SOCK_STREAM, 0);
    if(sd == -1){
        perror("Eroare la creare socket");
        exit(2);
    }
    int opt = 1;
    setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(int));
    struct sockaddr_in server, client;
    thData threads[100];
    server.sin_addr.s_addr = htonl(INADDR_ANY);
    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);

    if(bind(sd, (struct sockaddr*)&server, sizeof(struct sockaddr)) < 0){
        perror("Eroare la bind");
        return errno;
    }

    if(listen(sd, 50) < 0){
        perror("eroare la listen");
        return errno;
    }

    while(1){
        printf("[server] Serverul asteapta la portul %d \n", PORT);

        int client_fd1, client_fd2;
        unsigned int len = sizeof(client);

        client_fd1 = accept(sd, (struct sockaddr*)&client, &len);
        client_fd2 = accept(sd, (struct sockaddr*)&client, &len);
        if(client_fd1 == -1 || client_fd2 == -1){
            perror("Eroare la accept");
            return errno;
        }

        thData* thread = (struct threadData*)malloc(sizeof(struct threadData));
        thread->thread_id = total_threads++;
        thread->client_fd = client_fd1;
        thread->broadcast_fd = client_fd2;


        if(pthread_create((pthread_t*)&threads[total_threads], NULL, &routine, thread) < 0){
            perror("Eroare la creare thread");
            return errno;
        }
        printf("Am creat un thread cu id %d nou pentru un client\n", total_threads);
    }

    return 0;
}
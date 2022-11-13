#include <iostream>
#include <fstream>
#include <cstdlib>
#include <cstring>
#include <string.h>
#include <vector>
#include <pthread.h>


using namespace std;

void cargarips(string ruta, vector<string>& v);
int contalineas(string ruta);
void sacardatos(vector<string>& v);
void imprimirdatos(string e,string r,string p,vector<string>& v);
pthread_mutex_t mut;
void *hacerping(void *args){
    pthread_mutex_lock(&mut);
    char * ip = (char *) args;
    string comando="ping -q -c"+to_string(6)+" "+ip+"> .ping.txt";
    system(comando.c_str());
    pthread_mutex_unlock(&mut);
}

int main(int argc, char *argsv[]){    //en argsv[1] esta la ruta del archivo
    string ruta=argsv[1];
    int paquetes=stoi(argsv[2]);
    int cantip=contalineas(ruta);
    vector<string> ips;
    cargarips(ruta,ips);
    pthread_t hilo[cantip];
    
    pthread_mutex_init(&mut,NULL);
    for (int i = 0; i < cantip; i++)
    {
        char * url = (char *)ips.at(i).c_str();
        pthread_create(&hilo[i],NULL,hacerping,(void *) url);
        pthread_join(hilo[i],NULL);
    }
    pthread_mutex_destroy(&mut);    
    
    
    sacardatos(ips);
    
}

void cargarips(string ruta, vector<string>& v){
    ifstream archivo;
    string texto;
    archivo.open(ruta,ios::in);
    if (archivo.fail())
    {
        cout << "no se pudo abrir el archivo \n";
        exit(1);
    }
    while (!archivo.eof())
    {
       string s;
       getline(archivo,s);
       v.push_back(s);
    }
    archivo.close();
}
int contalineas(string ruta){
    ifstream archivo;
    int i=0;
    archivo.open(ruta,ios::in);
    if (archivo.fail())
    {
        cout << "no se pudo abrir el archivo \n";
        exit(1);
    }
    string linea;
    while (!archivo.eof())
    {
        getline(archivo,linea);
        i++;
    }
    archivo.close();
    return i-1;
}
void sacardatos(vector<string>& v){
    ifstream archivo;
    string t,enviados,recibidos,perdidos;
    archivo.open(".ping.txt",ios::in);
    if (archivo.fail())
    {
        cout << "no se pudo abrir el archivo \n";
        exit(1);
    }
    while (!archivo.eof())
    {
       getline(archivo,t);
       if(t.find("packets") != string::npos){
            //cout<<"linea buena \n "<<t;
            enviados=t.substr(0,1);
            recibidos=t.substr(23,1);
            perdidos=t.substr(35,1);
       }
    }
    archivo.close();
    system("rm .ping.txt");
    imprimirdatos(enviados,recibidos,perdidos,v);


}
void imprimirdatos(string e,string r,string p,vector<string>& v){
    string estado="UP";
    if (stoi(p)==1)
    {
        p=""+e;
    }
    
    if (stoi(r)<=0)
    {
        estado="DOWN";
    }
    
    cout<<"IP           Trans.      Rec.        Perd.       Estado \n";
    cout<<"─────────────────────────────────────────────────────── \n";
    cout<<v[1]<<"       "+e+"          "+r+"           "+p+"           "+estado+"\n";
}
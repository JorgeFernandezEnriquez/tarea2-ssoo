#include <iostream>
#include <fstream>
#include <cstdlib>
#include <cstring>
#include <string.h>
#include <vector>
#include <pthread.h>


using namespace std;
typedef struct Reporte
{
    string ip;
    string datos;
};


void cargarips(string ruta, vector<string>& v);
int contalineas(string ruta);
void sacardatos(vector<string>& v);
string sacarip(string linea);
string datos(string linea);
void imprimirdatos();
void *hacerping(void *args);
pthread_mutex_t mut;
vector<Reporte> reportes;



int main(int argc, char *argsv[]){    //en argsv[1] esta la ruta del archivo
    string ruta=argsv[1];
    string paquetes=argsv[2];
    int cantip=contalineas(ruta);
    vector<string> ips;
    cargarips(ruta,ips);
    pthread_t hilo[cantip];
    
    pthread_mutex_init(&mut,NULL);
    for (int i = 0; i < cantip; i++)
    {
        string a=ips.at(i)+"+"+paquetes;
        char * url = (char *)a.c_str();
        pthread_create(&hilo[i],NULL,hacerping,(void *) url);
        cout<<"haciendo ping numero: "<<i+1<<endl;
        pthread_join(hilo[i],NULL);
    }
    pthread_mutex_destroy(&mut);
    imprimirdatos();
    
}

void *hacerping(void *args){
    pthread_mutex_lock(&mut);
    vector<string> res;
    string dir,dat,ip,paq;
    char * datosping = (char *) args;
    string d=datosping;
    size_t pos;
    pos=d.find("+");
    ip=d.substr(0,pos);
    paq=d.substr(pos,d.length());

    string comando="ping -q -c"+paq+" "+ip+"> .ping.txt";
    system(comando.c_str());
    sacardatos(res);
    dir=sacarip(res[0]);
    dat=datos(res[1]);
    Reporte rep;
    rep.ip=dir;
    rep.datos=dat;
    reportes.push_back(rep);
    
    pthread_mutex_unlock(&mut);
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
       if(t.find("PING") != string::npos){
            //cout<<"linea buena \n "<<t;
            v.push_back(t);
       }
       if(t.find("packets") != string::npos){
            //cout<<"linea buena \n "<<t;
            v.push_back(t);
       }
    }
    archivo.close();
    system("rm .ping.txt");
    


}
string sacarip(string linea){
    size_t pos,posf;
    string linea2;
    string ip;
    char * url = (char *)linea.c_str();
    pos=strspn(url,"PING");
    linea2=linea.substr(pos+1,linea.length());
    posf=linea2.find(" (");
    ip=linea2.substr(0,posf); 
    return ip;
}
string datos(string linea){
    size_t pos;
    string linea2,linea3;
    string env,rec,per;
    char * url = (char *)linea.c_str();
    pos=linea.find(" packets transmitted");
    env=linea.substr(0,pos);
    
    linea2=linea.substr(pos+22,linea.length());
    pos=linea2.find(" received");
    rec=linea2.substr(0,pos);

    linea3=linea2.substr(pos+11,linea2.length());
    pos=linea3.find("% packet loss");
    per=linea3.substr(0,pos);
    return env+"+"+rec+"+"+per;
    
}
void imprimirdatos(){
    cout<<endl<<"IP                    Trans.      Rec.        Perd.       Estado \n";
    cout<<"──────────────────────────────────────────────────────────────── \n";
    for (int i = 0; i < reportes.size(); i++)
    {   
        string env,rec,per,aux;
        string datos=reportes[i].datos;
        size_t pos,pos2;
        pos=datos.find("+");
        env=datos.substr(0,pos);
        aux=datos.substr(pos+1,datos.length());
        pos=aux.find("+");
        rec=aux.substr(0,pos);
        per=aux.substr(pos+1,datos.length());
        string estado="UP";
        int perd=((stoi(env)*stoi(per))/100);
        
        if (stoi(rec)<=0)
        {
            estado="DOWN";
        }
        int dif=20-reportes[i].ip.length();
        for (int j = 0; j < dif; j++)
        {
            reportes[i].ip=reportes[i].ip+" ";
        }
        
        cout<<reportes[i].ip<<"    "+env+"          "+rec+"           "+to_string(perd)+"           "+estado+"\n";
    }
}
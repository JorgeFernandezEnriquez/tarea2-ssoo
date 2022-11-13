#include <iostream>
#include <fstream>
#include <cstdlib>
#include <cstring>
#include <string.h>
#include <vector>
#include <pthread.h>


using namespace std;
typedef struct Reporte//estructura en la cual las hebras guardaran sus resultados
{
    string ip;
    string datos;
};


void cargarips(string ruta, vector<string>& v);//coloca la ips del archivo en un vector de strings
void sacardatos(vector<string>& v);//carca el archivo oculto con los datos del ping y los almacena en un vector de strings
string sacarip(string linea);//recibe la primera linea del archivo generado la cual contiene la ip a la que se esta haciendo ping y retorna para imprimirla
string datos(string linea);//recibe la primera linea del archivo generado la cual contiene los datos del ping los separa y los retorna concatenados co un + de por medio para poder separarlos
void imprimirdatos();//imprime los datos que se encuentran en la estructura compartida
void *hacerping(void *args);//hace el ping con los parametros enviados y crea un archivo de texto oculto con los resultados
string colocarespacios(string original,int cant);//recibe un string y coloca los espacios faltantes para que al momento de imprimir no se vea corrido
pthread_mutex_t mut;
vector<Reporte> reportes;



int main(int argc, char *argsv[]){    //en argsv[1] esta la ruta del archivo
    //recibir parametros al ejecutar
    string ruta=argsv[1];
    string paquetes=argsv[2];
    //llenar el vector con las ips del archivo
    vector<string> ips;
    cargarips(ruta,ips);
    //validar tamaño de los parametros de entrada
    int cantip=ips.size();
    if (cantip==0)
    {
        cout<<"el archivo esta vacio o no contiene ips validas (las ips no deben contener espacios)"<<endl;
        exit(1);
    }
    int pa =stoi(paquetes);
    if (pa>125)
    {
        cout<<pa;
        cout<<"la cantidad de paquetes supera el limite establecido (limite 125)"<<endl;
        exit(1);
    }
    //creacion de los hilos
    pthread_t hilo[cantip];
    pthread_mutex_init(&mut,NULL);
    for (int i = 0; i < cantip; i++)
    {
        //concatenar parametros para enviar a la funcion ping
        string a=ips.at(i)+"+"+paquetes;
        char * url = (char *)a.c_str();
        //creacion del hilo correspondiente asignando la funcion ping y los parametros necesarios (ip,paquetes) 
        pthread_create(&hilo[i],NULL,hacerping,(void *) url);
        //mensaje para ver que avanza
        cout<<"haciendo ping numero: "<<i+1<<endl;
        //ejecucion del hilo correspondiente
        pthread_join(hilo[i],NULL);
    }
    pthread_mutex_destroy(&mut);
    //imprime los datos almacenados por los hilos
    imprimirdatos();
    
}

void *hacerping(void *args){
    pthread_mutex_lock(&mut);
    //creacion del vector con los resultados del ping
    vector<string> res;
    //cargar parametros de la funcion
    string dir,dat,ip,paq;
    char * datosping = (char *) args;
    string d=datosping;
    //separar parametros que venian concatenados por un +
    size_t pos;
    pos=d.find("+");
    ip=d.substr(0,pos);
    paq=d.substr(pos,d.length());
    //ejecucion del comando ping y creacion del archivo con resultados
    string comando="ping -q -c"+paq+" "+ip+"> .ping.txt";
    system(comando.c_str());
    //sacar resultados del archivo
    sacardatos(res);
    dir=sacarip(res[0]);
    dat=datos(res[1]);
    //creacion del reporte
    Reporte rep;
    rep.ip=dir;
    rep.datos=dat;
    //agregar reporte a la estrcutura
    reportes.push_back(rep);
    
    pthread_mutex_unlock(&mut);
}

void cargarips(string ruta, vector<string>& v){
    //cargar archivo con ips
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
        //si la linea del archivo no esta vacia y no contiene espacios es una linea valida para hacer ping y se guarda en el vector
        if (!s.empty() && s.find(" ")==string::npos)
        {    
            v.push_back(s);
        }
    }
    archivo.close();
}
void sacardatos(vector<string>& v){
    //cargar archivo oculto con los resultados
    ifstream archivo;
    string t;
    archivo.open(".ping.txt",ios::in);
    if (archivo.fail())
    {
        cout << "no se pudo abrir el archivo \n";
        exit(1);
    }
    while (!archivo.eof())
    {
       getline(archivo,t);
       //analizar linea que contiene ip
       if(t.find("PING") != string::npos){
            v.push_back(t);
       }
       //analizar linea que contiene los datos del ping
       if(t.find("packets") != string::npos){
            v.push_back(t);
       }
    }
    archivo.close();
    //eliminacion del archivo creado
    system("rm .ping.txt");
    


}
string sacarip(string linea){
    size_t pos,posf;
    string linea2;
    string ip;
    char * url = (char *)linea.c_str();
    //separar la ip que esta entre PING y ( en la linea del archivo
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
    //separar diferentes resultados del ping que vienen en la linea recibida
    pos=linea.find(" packets transmitted");
    env=linea.substr(0,pos);
    
    linea2=linea.substr(pos+22,linea.length());
    pos=linea2.find(" received");
    rec=linea2.substr(0,pos);

    linea3=linea2.substr(pos+11,linea2.length());
    pos=linea3.find("% packet loss");
    per=linea3.substr(0,pos);
    //concatenar resultados para poder utilizarlos
    return env+"+"+rec+"+"+per;
    
}
void imprimirdatos(){
    cout<<endl<<"IP                    Trans.           Rec.           Perd.          Estado \n";
    cout<<"──────────────────────────────────────────────────────────────────────────────── \n";
    for (int i = 0; i < reportes.size(); i++)
    {   
        //separar variables concatenadas por + en datos 
        string env,rec,per,aux,ip;
        string datos=reportes[i].datos;
        size_t pos,pos2;
        pos=datos.find("+");
        env=datos.substr(0,pos);
        aux=datos.substr(pos+1,datos.length());
        pos=aux.find("+");
        rec=aux.substr(0,pos);
        per=aux.substr(pos+1,datos.length());
        //calcular perdida de paquetes ya que el ping retorna un procentaje
        int perd=(stoi(env)-stoi(rec));
        per=to_string(perd);
        //creacion de estado si no recibe paquetes se dice que tiene el estado DOWN si recibe UP
        string estado="UP";
        if (stoi(rec)<=0)
        {
            estado="DOWN";
        }
        //colocar espacios para imprimir los datos
        ip=colocarespacios(reportes[i].ip,20);
        env=colocarespacios(env,5);
        rec=colocarespacios(rec,5);
        per=colocarespacios(per,5);
        //imprimir datos
        cout<<ip<<"    "+env+"          "+rec+"           "+per+"           "+estado+"\n";
    }
}

string colocarespacios(string original,int cant){
    //calcular cuantos espacios agregar segun el largo del original y cuantos debe tener
    int dif=cant-original.length();
        for (int j = 0; j < dif; j++)
        {
            original=original+" ";
        }
    return original;
}
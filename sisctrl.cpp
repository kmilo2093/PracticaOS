#include "yaml-cpp/yaml.h"
#include <iostream>
#include <fstream>
#include <string>
#include <string.h>
#include <vector>
#include <algorithm>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <ctype.h>
#include <sys/select.h>
#include <semaphore.h>
#include <fcntl.h>
#include <errno.h>
#include <signal.h>
#define MAX_BUFFER 512
#define SNAME "/mysem"
#define AUTO_NO_ENC "No ha sido encontrado ninguna automata con esa caracteristica";
using namespace std;
// our data 

struct  indv {
	std::string state_name;
	int id;
};
struct info{
	int id;
	std::string auto_name;
	std::vector <indv> list_states;
};
struct data {
    std::string recog;
    std::string rest;
    
};

struct acep {
    std::string name;
    std::string msg;
    int pos;

};
struct data_user {
   std::string cmd;
   std::string msg;
};


struct trans {

    std::string in;
    std::string next;
    int tuberiaSalida;
};
struct delta {

    std::string name;
    std::vector < trans > trans_list;
    int pipes[2];

};

struct automata {

    std::string name;
    std::string description;

    std::vector < std::string > alpha;

    std::vector < std::string > states;

    std::string start;
    std::vector < std::string > final;


    std::vector < delta > delta_list;

};

//metodo que crea el yaml de aceptacion para cada estado
void emision_acep(unsigned size, char *buffer,string limit,string str){
			    YAML::Emitter out_1;
			    out_1 << YAML::BeginMap;
			    out_1 << YAML::Key << "recog";
			    out_1 << YAML::Value << limit.c_str();
			    out_1 << YAML::Key << "rest";
			    string rest;
			    if (str.size() - size > 1) {
				for (unsigned p = size; p < str.size(); p++) {
				    rest.push_back(str[p]);
			    }} else {

				rest = "";
			    }
			    out_1 << YAML::Value << rest;
			    out_1 << YAML::EndMap;
			    string join = out_1.c_str();
			    join.insert(0, "{");
			    join.insert(join.size(), "}");
			    strcpy(buffer, join.c_str());

}
//metodo que crea el yaml de fallo para cada estado
void emision_fallo(string str, char *buffer,string automata){
 		    YAML::Emitter out;
		    out << YAML::BeginMap;
		    out << YAML::Key << "codterm";
		    out << YAML::Value << 1;
		    out << YAML::Key << "recog";
		    out << YAML::Value << "";
		    out << YAML::Key << "rest";
		    out << YAML::Value << str.substr(0, str.size() - 1);
		    out << YAML::Key << "auto";
		    out << YAML::Value <<automata;
		    out << YAML::EndMap;
		    strcpy(buffer, out.c_str());

}

// metodo que ejecutan todos los procesos estado
void state(int in, delta out, int final, int pipe_sis[], string automata)
{
    sem_t *sem= sem_open(SNAME,0);
    char buffer[MAX_BUFFER];
    int c;
    while ((c = read(in, buffer, MAX_BUFFER))) {
	if (c == 0)
	    break;
	else if (c == -1)
	    break;
	else if (c > 0) {
	    string str(buffer, c);
	    //std::cout<<str<<endl;
	    if (!str.substr(0, 1).compare("{")) {
		str = str.substr(1, str.size() - 2);

		std::stringstream ss(str);
		YAML::Parser parser(ss);
		YAML::Node doc;
		parser.GetNextDocument(doc);
		data datos;
		doc["recog"] >> datos.recog;
		doc["rest"] >> datos.rest;

		int entro = 0;
		for (unsigned i = 0; i < out.trans_list.size(); i++) {
		    string limit = out.trans_list[i].in;
		    unsigned size = limit.size();
		    if (size <= datos.rest.size()) {
			string part = datos.rest.substr(0, size);
			if (!part.compare(limit)) {
			    entro = 1;
			    YAML::Emitter out_1;
			    out_1 << YAML::BeginMap;
			    out_1 << YAML::Key << "recog";
			    datos.recog.insert(datos.recog.size(), limit.c_str());
			    out_1 << YAML::Value << datos.recog;
			    out_1 << YAML::Key << "rest";
			    string rest;
			    if (datos.rest.size() - size > 1) {
				for (unsigned p = size; p < datos.rest.size(); p++) {
				    rest.push_back(datos.rest[p]);

			    }} else {
				rest = "";
			    }
			    out_1 << YAML::Value << rest;
			    out_1 << YAML::EndMap;
			    string join = out_1.c_str();
			    join.insert(0, "{");
			    join.insert(join.size(), "}");
			    strcpy(buffer, join.c_str());   
			    write(out.trans_list[i].tuberiaSalida, buffer, strlen(buffer));
			    break;
			}

		    }

		}

		if (!entro && datos.rest.size() == 0 && final) {

		    
		    YAML::Emitter out;
		    out << YAML::BeginMap;
		    out << YAML::Key << "codterm";
		    out << YAML::Value << 0;
		    out << YAML::Key << "recog";
		    out << YAML::Value << datos.recog;
		    out << YAML::Key << "rest";
		    out << YAML::Value << datos.rest;
		    out << YAML::Key << "auto";
		    out << YAML::Value << automata;
		    out << YAML::EndMap;
		    
		    strcpy(buffer, out.c_str());
		    sem_wait(sem);
		    write(pipe_sis[1], buffer, strlen(buffer));
		    
		} else if (!entro) {


		    YAML::Emitter out;
		    out << YAML::BeginMap;
		    out << YAML::Key << "codterm";
		    out << YAML::Value << 1;
		    out << YAML::Key << "recog";
		    out << YAML::Value << datos.recog;
		    out << YAML::Key << "rest";
		    out << YAML::Value << datos.rest.substr(0, datos.rest.size() - 1);
		    out << YAML::Key << "auto";
		    out << YAML::Value << automata;
		    out << YAML::EndMap;
		    strcpy(buffer, out.c_str());
		    sem_wait(sem);
		    write(pipe_sis[1], buffer, strlen(buffer));
		  

		} 
	    } else {
		int entro = 0;
		for (unsigned i = 0; i < out.trans_list.size(); i++) {
		    string limit = out.trans_list[i].in;
		    unsigned size = limit.size();
		    if (size <= str.size()) {
			string part = str.substr(0, size);
			if (!part.compare(limit)) {
			    entro = 1;
			    emision_acep(size,buffer,limit,str); 
			    write(out.trans_list[i].tuberiaSalida, buffer, strlen(buffer));
			    break;
			}

		    }
		}
		if (!entro) {
		    emision_fallo(str,buffer,automata);
		    sem_wait(sem);
		    write(pipe_sis[1], buffer, strlen(buffer));
	            
		}
	    }
	}
    }
}

// obtención especifica de los datos obtenidos en el archivo yaml

void operator >>(const YAML::Node & node, data_user & data_user)
{
        node["cmd"]>>data_user.cmd;
	node["msg"]>>data_user.msg;
}

void operator >>(const YAML::Node & node, trans & trans)
{
    node["in"] >> trans.in;
    node["next"] >> trans.next;
    trans.tuberiaSalida = 0;
}

void operator >>(const YAML::Node & node, delta & delta)
{

    node["node"] >> delta.name;
    const YAML::Node & trans_1 = node["trans"];
    for (unsigned i = 0; i < trans_1.size(); i++) {
	trans trans_2;
	trans_1[i] >> trans_2;
	delta.trans_list.push_back(trans_2);
    }
    pipe(delta.pipes);
}


void operator >>(const YAML::Node & node, automata & automata)
{

    node["automata"] >> automata.name;
    node["description"] >> automata.description;

    const YAML::Node & alpha = node["alpha"];
    for (unsigned i = 0; i < alpha.size(); i++) {
	std::string help;
	alpha[i] >> help;
	automata.alpha.push_back(help);
    }

    const YAML::Node & states = node["states"];
    for (unsigned i = 0; i < states.size(); i++) {
	std::string help;
	states[i] >> help;
	automata.states.push_back(help);
    }

    node["start"] >> automata.start;

    const YAML::Node & final = node["final"];
    for (unsigned i = 0; i < final.size(); i++) {
	std::string help;
	final[i] >> help;
	automata.final.push_back(help);
    }

    const YAML::Node & deltas = node["delta"];
    for (unsigned i = 0; i < deltas.size(); i++) {
	delta delta_1;
	deltas[i] >> delta_1;
	automata.delta_list.push_back(delta_1);
    }
}
// asesina todo de manera mas sencilla
void killer(int signum){
		exit(signum);
}
// emision de datos para la caracteristica de info msg=null

void emision_info_full(vector <info>  list_auto_info, char *buffer){
		       YAML::Emitter out;
		       out << YAML::BeginMap;
		       out << YAML::Key << "msgtype";
		       out << YAML::Value << "info";
		       out << YAML::Key << "info";
		       out<<YAML::Value<<YAML::BeginMap;
			
		       for(unsigned k=0; k<list_auto_info.size();k++){
			       out << YAML::Key << "automata";
		               out << YAML::Value << list_auto_info[k].auto_name;
			       out << YAML::Key << "ppid";
		               out << YAML::Value << list_auto_info[k].id;
			       //out <<YAML::Value<<YAML::BeginMap;	
   			       for(unsigned m=0; m<list_auto_info[k].list_states.size();m++){
       				
				out << YAML::Key <<  "-node";
		                out << YAML::Value << list_auto_info[k].list_states[m].state_name;
			        out << YAML::Key << "pid";
		                out << YAML::Value << list_auto_info[k].list_states[m].id;	
			       }
			       //out << YAML::EndMap;
		         }
			out << YAML::EndMap;
		        out << YAML::EndMap;
			strcpy(buffer, out.c_str());
}

// emision especifia del yaml de un estado particular
void emision_info_espc(vector <info>  list_auto_info, char *buffer, unsigned k){
	 YAML::Emitter out;
	 out << YAML::BeginMap;
	 out << YAML::Key << "msgtype";
	 out << YAML::Value << "info";
	 out << YAML::Key << "info";
	 out<<YAML::Value<<YAML::BeginMap;
	 out << YAML::Key << "automata";
	 out << YAML::Value << list_auto_info[k].auto_name;
	 out << YAML::Key << "ppid";
	 out << YAML::Value << list_auto_info[k].id;     	
	 for(unsigned m=0; m<list_auto_info[k].list_states.size();m++){
			  				
		out << YAML::Key <<  "-node";
		out << YAML::Value << list_auto_info[k].list_states[m].state_name;
		out << YAML::Key << "pid";
		out << YAML::Value << list_auto_info[k].list_states[m].id;	
	 }
	 out << YAML::EndMap;
	 out << YAML::EndMap;
	 strcpy(buffer, out.c_str());
}

// metodo para asesinar a todos los procesos
void kill_everything (vector <info>  list_auto_info){
  for(unsigned k=0; k<list_auto_info.size();k++){			       
      for(unsigned m=0; m<list_auto_info[k].list_states.size();m++){
	 int status;
	 kill(list_auto_info[k].list_states[m].id,SIGKILL);
	 wait(&status);
	 if(WIFEXITED(status)){
	     WEXITSTATUS(status);
	 }	
      }  
   }
  kill(list_auto_info[0].id,SIGKILL);
}

// emision de mensajes de rechazo
void emision_rej(vector <acep> error, char *buffer){

		       YAML::Emitter out;
		       out << YAML::BeginMap;
		       out << YAML::Key << "msgtype";
		       out << YAML::Value << "reject";
		       out << YAML::Key << "reject";
		       out<<YAML::Value<<YAML::BeginMap;
		
		       for(unsigned k=0; k<error.size();k++){
			       out << YAML::Key << "automata";
		               out << YAML::Value << error[k].name;
			       out << YAML::Key << "msg";
		               out << YAML::Value << error[k].msg;
			       out << YAML::Key << "pos";
		               out << YAML::Value << error[k].pos;		
							  
		         }
			out << YAML::EndMap;
		        out << YAML::EndMap;
			strcpy(buffer, out.c_str());
}

// emision de mensajes de aceptacion
void emision_acep(vector <acep> success, char *buffer){
YAML::Emitter out;
		       out << YAML::BeginMap;
		       out << YAML::Key << "msgtype";
		       out << YAML::Value << "accept";
		       out << YAML::Key << "accept";
		       out<<YAML::Value<<YAML::BeginMap;
		
		       for(unsigned k=0; k<success.size();k++){
			       out << YAML::Key << "automata";
		               out << YAML::Value << success[k].name;
			       out << YAML::Key << "msg";
		               out << YAML::Value << success[k].msg;		
							  
		         }
			out << YAML::EndMap;
		        out << YAML::EndMap;
			strcpy(buffer, out.c_str());
}

// metodo de  procedimiento y lectura 
void lectura_larga(vector <info>  list_auto_info ,vector < int >out, unsigned cantAuto,sem_t *sem,int in){
 
	vector <acep> success;
	vector <acep> error;
	int c;
	fd_set fdin;
	int rin;
	unsigned cant;
	for (;;) {
           
	    char buffer[MAX_BUFFER];
	    FD_ZERO(&fdin);
	    FD_SET(0, &fdin);
	    FD_SET(in, &fdin);
	    rin = select((in + 1), &fdin, NULL, NULL, NULL);

	    if (rin == 1) {
		if (FD_ISSET(0, &fdin)) {
		    
		    c = read(0, buffer, MAX_BUFFER);
		    
		    // adquisición de datos ingresados desde consola
		    
			    string str(buffer,c);
			    std::stringstream ss(str);
			    YAML::Parser parser(ss);
			    YAML::Node doc;
			    parser.GetNextDocument(doc);
			    data_user  datos;
			    doc ["cmd"] >> datos.cmd;
			    doc ["msg"] >> datos.msg;
		    

		    if (!datos.cmd.compare("send")){
			cant=0;
			strcpy(buffer,datos.msg.c_str());
			c=strlen(buffer)+1;

		    }else if (!datos.cmd.compare("info")){
			
		      if(!datos.msg.compare("~")){

		          emision_info_full(list_auto_info,buffer);
			  int res;
		          res = write(1, buffer, strlen(buffer));
			   if (res < -1) {
			    fprintf(stderr, "Error en salida estandar padre");
		          } else {
			    fprintf(stdout, "\n");
		          }
			  continue;

			}else{
				
			       int entro=0;
			       for(unsigned k=0; k<list_auto_info.size();k++){
				if(!datos.msg.compare(list_auto_info[k].auto_name)){
				       entro=1;
				        emision_info_espc(list_auto_info,buffer,k);
					int res;
					res = write(1, buffer, strlen(buffer));
					printf("\n");
					 if (res < -1) {
					    fprintf(stderr, "Error en salida estandar padre");
					 } else {
					    fprintf(stdout, "\n");
					}
					
					break;
				 }
				}
				 if (!entro){
					string n_found= AUTO_NO_ENC;
					strcpy(buffer, n_found.c_str());
					int res;
					res = write(1, buffer, strlen(buffer));
					printf("\n");
					 if (res < -1) {
					    fprintf(stderr, "Error en salida estandar padre");
					 } else {
					    fprintf(stdout, "\n");
					}
				}
				continue;
			}

		    }else if (!datos.cmd.compare("stop")){
				kill_everything(list_auto_info);	
		    }else{
			std::cout<<"comando no reconocido; error en ejecucion."<<endl;
		    }

		} else if (FD_ISSET(in, &fdin)) {
		    c = read(in, buffer, MAX_BUFFER);
			sem_post(sem);
			cant++;
			string str (buffer,c);

		        std::stringstream ss(str);
		        YAML::Parser parser(ss);
			YAML::Node doc;
			parser.GetNextDocument(doc);
			data datos;
			string automata;
			int codeTerm;
			doc["codterm"]>> codeTerm;
			doc["recog"] >> datos.recog;
			doc["rest"] >>  datos.rest;
			doc["auto"] >>  automata;
			
		    
			acep ac;
		        ac.name=automata;
			ac.pos=datos.recog.size()+1;
			datos.recog.insert(datos.recog.size(),datos.rest);
			ac.msg=datos.recog;
			switch (codeTerm){
			
			   case 0:  
				    ac.pos=-1;
				    
				    success.push_back(ac);
				     break;
			   case 1:
				    
				     error.push_back(ac);
				     break;
			   default:	
				 std::cout<<"error en la ejecucion"<<endl;
				break;
			}

			strcpy(buffer,"llego algo...\n");
			c=strlen(buffer);
		}
	    }
	    if (c == 0)
		break;
	    else if (c == -1)
		break;
	    else if (c > 0) {
		if (FD_ISSET(0, &fdin)) {
		    int res;
		    for (unsigned k = 0; k < out.size(); k++) {
			res = write(out[k], buffer, c);
			 
			if (res < -1) {
			    fprintf(stderr, "Error en salida tuberia padre");
			}
		    }
		} else if (cant==cantAuto) {
		    int res;
	               if(error.size()>0){

		           emision_rej(error,buffer);
			   error.clear();
		           res = write(1, buffer, strlen(buffer));

                  	if (res < -1) {
			    fprintf(stderr, "Error en salida estandar padre");
		        }else{
			    printf("\n");
			}
		       }
			
		       if(success.size()>0){
		        emision_acep(success,buffer);
			success.clear();
		        res = write(1, buffer, strlen(buffer));
			}
		    if (res < -1) {
			fprintf(stderr, "Error en salida estandar padre");
		    }else{
				printf("\n");
			}
		}
	    }
	}
	fprintf(stderr, "Termino sisctrl con error");

}

int main()
{
    try {
//creación de un semaforo para poder que los procesos accedan adecuadamente a la tuberia del sisctrl
// 0644 permisos de lectura y escritura y el 1 significa que es un semaforo compartido entre procesos
	sem_t *sem = sem_open(SNAME, O_CREAT, 0644,1); 
        sem_init(sem, 1, 1);

	//lectura del archivo .yaml
	std::ifstream fin("automata1.yaml");

	// creación de datos que seran utilizados y identificado posteriormente
	int in;
	vector <info> list_auto_info;
	vector < int >out;

	

	// Parser yaml
	YAML::Parser parser(fin);
	YAML::Node doc;
	parser.GetNextDocument(doc);

	//Creación de la tuberia del sisctrl
	int pipe_sis[2];
	pipe(pipe_sis);
// in sera la variable donde los procesos tiene que copiar para cualquier resultado en su ejecución
	in = pipe_sis[0];

	
	string automataName;
	unsigned cantAuto=doc.size();

	// ciclo para recolectar los datos del yaml y lanzar los procesos
	for (unsigned l = 0; l < cantAuto; l++) {

	    automata automata_s;
	    doc[l] >> automata_s;

	    info auto_info;
	    //identificación del automata inicial
	    std::string start;
	    start = automata_s.start;

	    automataName=automata_s.name;
	    auto_info.auto_name=automataName;

  	    // obtencion de los de cada  automata y estados 
	    for (unsigned i = 0; i < automata_s.delta_list.size(); i++) {
		
		// definicion de la tuberia input de un estado
		int in_;
		in_ = automata_s.delta_list[i].pipes[0];

		// asiganción de tuberias de salida para cada transición en cada estado(Proceso)
		for (unsigned k = 0; k < automata_s.delta_list[i].trans_list.size(); k++) {
		    for (unsigned p = 0; p < automata_s.delta_list.size(); p++) {
			if (!automata_s.delta_list[p].name.compare(automata_s.delta_list[i].trans_list[k].next)) {
		 
			    automata_s.delta_list[i].trans_list[k].tuberiaSalida = automata_s.delta_list[p].pipes[1];
			    break;
			}
		    }
		}


		// identificar cuales estados son finales
		int final = 0;
		for (unsigned k = 0; k < automata_s.final.size(); k++) {
		    if (!automata_s.delta_list[i].name.compare(automata_s.final[k])) {                
			final = 1;
			break;
		    }
		}

		delta outs = automata_s.delta_list[i];

		// id de cada proceso creado 
		pid_t pid;

		if ((pid=fork()) == 0) {
		    // cierre de lectura y escritura de consola
		    close(1);
		    close(0); 
		    //rutina a seguir para esa proceso creado   
		    state(in_, outs, final, pipe_sis,automataName);
		}
		    // adquisición de caracteristicas del proceso creado 
		    indv info_s;
		    info_s.state_name=automata_s.delta_list[i].name;
		    info_s.id=pid;
		    auto_info.list_states.push_back(info_s);

		 // OUT guarda las tuberias de escritura de todos los estados iniciales
		if(!start.compare(automata_s.delta_list[i].name)){
		    out.push_back(automata_s.delta_list[i].pipes[1]);			
		}

	    }
		 // adquisición de caracteristicas del proceso creado
		 auto_info.id=getpid();
		 list_auto_info.push_back(auto_info);
	}

	// obtención de la señal ctrl-c por parte del usuario 
	signal(SIGINT,killer);

	// lectura de datos desde consola y respuesta por parte del sisctrl
	lectura_larga(list_auto_info,out,cantAuto,sem,in);
    }
    catch(YAML::ParserException & e) {
	  std::cout << "houston poseemos problemas..."<<endl;
	  std::cout << e.what() <<endl;
    }

    return 0;
}

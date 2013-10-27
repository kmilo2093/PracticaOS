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
#define MAX_BUFFER 512
#define SNAME "/mysem"
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
			    //std::cout<<out.trans_list[i].next<<endl;
			    write(out.trans_list[i].tuberiaSalida, buffer, strlen(buffer));
			    break;
			}

		    }

		}

		if (!entro && datos.rest.size() == 0 && final) {

		    //printf("it is final %d n",final);
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
		  

		} else {


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
			    write(out.trans_list[i].tuberiaSalida, buffer, strlen(buffer));
			    break;
			}

		    }
		}
		if (!entro) {


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
		    std::cout<<"\n"<<endl;
		    strcpy(buffer, out.c_str());
		    sem_wait(sem);
		    write(pipe_sis[1], buffer, strlen(buffer));
	            
		}
	    }
	}
    }
}

// now the extraction operators for these types

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


int main()
{
    try {
	sem_t *sem = sem_open(SNAME, O_CREAT, 0644,1);
        sem_init(sem, 1, 1);
	std::ifstream fin("automata1.yaml");
	int in;
	vector <info> list_auto_info;
	vector < int >out;
	vector <acep> success;
	vector <acep> error;
	YAML::Parser parser(fin);
	YAML::Node doc;
	parser.GetNextDocument(doc);
	int pipe_sis[2];
	pipe(pipe_sis);
	in = pipe_sis[0];
	string automataName;
	int cantAuto=doc.size();
	for (unsigned l = 0; l < doc.size(); l++) {
	    automata monster;
	    doc[l] >> monster;
	    info auto_info;
	   
	    std::string start;
	    start = monster.start;
	    automataName=monster.name;
	    auto_info.auto_name=automataName;  
	    for (unsigned i = 0; i < monster.delta_list.size(); i++) {
		std::vector < std::string > Next;
		for (unsigned j = 0; j < monster.delta_list[i].trans_list.size(); j++) {
		    std::string help;
		    help.assign(monster.delta_list[i].trans_list[j].next);
		    Next.push_back(help);
		}
		std::sort(Next.begin(), Next.end());
		Next.erase(std::unique(Next.begin(), Next.end()), Next.end());

		int in_;
		in_ = monster.delta_list[i].pipes[0];

		for (unsigned k = 0; k < monster.delta_list[i].trans_list.size(); k++) {
		    for (unsigned p = 0; p < monster.delta_list.size(); p++) {



			if (!monster.delta_list[p].name.compare(monster.delta_list[i].trans_list[k].next)) {
			    //std::cout<<"copiando tuberia de "<<monster.delta_list[p].name<<endl;
			    monster.delta_list[i].trans_list[k].tuberiaSalida = monster.delta_list[p].pipes[1];
			    break;
			}


		    }
		}

		int final = 0;
		for (unsigned k = 0; k < monster.final.size(); k++) {
		    if (!monster.delta_list[i].name.compare(monster.final[k])) {
			//std::cout<<"Im final "<<monster.delta_list[i].name<<endl;                             
			final = 1;
			break;
		    }
		}

		//printf("it's final %d n",final);
		delta outs = monster.delta_list[i];
		
		pid_t pid;
		if ((pid=fork()) == 0) {
		    close(1);
		    close(0);
		    
		    state(in_, outs, final, pipe_sis,automataName);
		}
		   
		    indv info_s;
		    info_s.state_name=monster.delta_list[i].name;
		    info_s.id=pid;
		    auto_info.list_states.push_back(info_s);

		    
		if(!start.compare(monster.delta_list[i].name)){
		    out.push_back(monster.delta_list[i].pipes[1]);			
		}

		
	    }
		 
		 auto_info.id=getpid();
		 list_auto_info.push_back(auto_info);
	}

	
	// lectura
	
	int c;
	fd_set fdin;
	int rin;
	int cant;
	for (;;) {
           
	    char buffer[MAX_BUFFER];
	    FD_ZERO(&fdin);
	    FD_SET(0, &fdin);
	    FD_SET(in, &fdin);
	    rin = select((in + 1), &fdin, NULL, NULL, NULL);

	    if (rin == 1) {
		if (FD_ISSET(0, &fdin)) {
		    
		    c = read(0, buffer, MAX_BUFFER);
		    
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
			int res;
		        res = write(1, buffer, strlen(buffer));
			printf("\n");
			continue;
			 if (res < -1) {
			    fprintf(stderr, "Error en salida estandar padre");
		         } else {
			    fprintf(stdout, "\n");
		        }

		    }else if (!datos.cmd.compare("stop")){
			std::cout<<"Entro"<<endl;	
		    }else{
			std::cout<<"porque"<<endl;
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

			strcpy(buffer," wait please ...\n");
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
			error.clear();
			strcpy(buffer, out.c_str());
		        res = write(1, buffer, strlen(buffer));
			if (res < -1) {
			fprintf(stderr, "Error en salida estandar padre");
		          }
			}
			
			if(success.size()>0){
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
			success.clear();
			strcpy(buffer, out.c_str());
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
	fprintf(stderr, "Termino padren");

    }
    catch(YAML::ParserException & e) {
	std::cout << e.what() <<endl;
    }

    return 0;
}


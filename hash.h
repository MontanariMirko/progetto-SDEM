#ifndef HASH_H
#define HASH_H
#include <string>
#include <vector>
#include <map>
#include <fstream>
#include "bit.h"
using namespace std;

class LMPhash{
	//dimensione del dizionario
	unsigned _dim_diz;
	//dimensione del look head buffer
	unsigned _dim_look;
	//dimensione del buffer
	unsigned _dim_buff;

public:
	//costruttore
	LMPhash(unsigned dim_diz, unsigned dim_look, unsigned dim_buff) : _dim_diz(dim_diz), _dim_look(dim_look), _dim_buff(dim_buff) {}

	//restituisce la dimensione del dizionario
	unsigned getDimDiz(){
		return _dim_diz;
	}

	//restituisce la dimensione del look head buffer
	unsigned getDimLook(){
		return _dim_look;
	}

	//restituisce la dimensione del buffer
	unsigned getDimBuffer(){
		return _dim_buff;
	}

	//funzione hash
	//il parametro string s è la stringa su cui effettuare la funzione hash
	unsigned func_hash(string s){
		unsigned key = 0;
		//primo passaggio hash, si moltiplica un intero crescente per ogni lettera della stringa
		for(unsigned i=0; i<s.length(); ++i)
			key += (i+1)*s[i];
	
		//secondo passaggio hash
		unsigned h1 = ((171*key+931)>>5);
		//terzo passaggio hash
		unsigned h2 = ((13*key+17)<<5);
		//quarto passaggio hash, è una funzione hash di secondo livello:
		//vengono effettuate due funzioni hash che poi vengono unite
		unsigned h = (3*h1 + h2)%_dim_diz;
	
		return h;
	}

	//creazione del dizionario
	//parametri richiesti:
	//string s: è la stringa su cui creare il dizionario
	//vector diz: è il vettore di mappe che conterrà il dizionario hash
	void crea_diz(string s, vector<map<string,string>> &diz){
		unsigned count = 0;
		//cancello il dizionario e lo ridimensiono secondo i parametri dati
		diz.clear();
		diz.resize(_dim_diz);
	
		//creazione di tutte le possibili combinazioni di parole da inserire nel dizionario
		for(unsigned i=0; i<s.length(); ++i){
			string prova;
			count = 0;
			for(unsigned j=0; j<s.length()-i; ++j){
				prova += s[i+j];
				++count;
				//se la lunghezza della parola supera la dimensione stabilita dal
				//look head esce dal ciclo
				if(count > _dim_look)
					break;
				if((i+j)==s.length()-1 && count<_dim_look)
					j=-1;
				//inserisco solo le parole maggiori o uguali di 3
				if(prova.length() >= 3){
					//calcolo hash della parola da aggiungere al dizionario
					unsigned key = func_hash(prova);
					//a è il numero di quanti caratteri andare indietro
					char a = s.length()-i;
					//b è quanti caratteri ripetere
					char b = count;
					string c;
					c += a;
					c +=b;
					//inserimento della parola nel dizionario, aggiungendo la coppia di valori a e b
					diz[key].insert(pair<string,string>(prova,c));		
				}
			}
		}
	}

	void creaFileHash(string nomein, string nomeout){
		//apro il file da leggere
		fstream f(nomein, fstream::in | fstream::binary);
		//creo il file codificato
		fstream fout(nomeout, fstream::out | fstream::binary);
		bitwriter bw(fout);
		//calcolo il numero massimo di bit necessari per scrivere il file
		unsigned nbit = bw.numBit(_dim_buff);

		//dizionario hash
		vector<map<string,string>> diz;
		string buff;
		string look;
		string tmp;
		unsigned char c;

		//scrivo i primi 3 caratteri
		for(unsigned i=0; i<3; ++i){
			f.read((char*)&c,1);
			if(f.eof())
				break;
			//scivo un bit a 1 per indicare che il prossimo byte è un carattere
			bw.write(1,1);
			bw.write(unsigned(c),8);
			//cout << "1." << c << endl;
			buff += c;
		}
	
		do{
			//aggiorno il look head buffer
			for(unsigned i=0; (i<_dim_look && look.size()<_dim_look); ++i){
				f.read((char*)&c,1);
				if(f.eof())
					break;
				look += c;
			}
			//creo il dizionario
			crea_diz(buff,diz);
			bool trovato = true;
			//controllo se la stringa è presente nel dizionario SOLO se è maggiore o uguale di 3 caratteri
			//altrimenti scrivo un carattere alla volta
			if(look.size() > 2)
				tmp = look.substr(0,3);
			else
				trovato = false;
			//key: valore hash
			//a: di quante posizioni bisogna spostarsi indietro nel buffer
			//b: numero di ripetizioni dei caratteri del buffer
			//num: indice nella posizione massima della stringa
			unsigned key,a,b,num;
			num = 3;
			a = b = 0;
			while(trovato){
				trovato = false;
				key = func_hash(tmp);
				//se ho trovato un valore nel dizionario aggiorno le variabili a e b
				if(diz[key][tmp] != ""){
					trovato = true;
					a = unsigned(diz[key][tmp][0]);
					b = unsigned(diz[key][tmp][1]);
					if(tmp.size() < look.size())
						tmp += look[num++];
					else
						break;
				}
			}
			//questa condizione indica che la stringa non è presente nel dizionario
			//aggiungo un solo valore al file
			if(a==0 && b==0){
				bw.write(1,1);
				bw.write(unsigned(look[0]),8);
				//cout << "1." << look[0] << endl;
				//aggiungo un solo carattere al buffer
				buff += look[0];
				//cancello il primo carattere al look head buffer
				look.erase(0,1);
			}
			else
			{
				//scrivo un bit a 0: indica che segue una coppia di valori
				bw.write(0,1);
				//scrivo a e b
				bw.write(unsigned(a),nbit);
				bw.write(unsigned(b),nbit);
				//cout << "0." << a << "." << b << endl;
				tmp = tmp.substr(0,b);
				buff += tmp;
				look.erase(0,b);
			}
			//se la dimensione del buffer supera la lunghezza massima cancello i primi n caratteri
			//per ottenere al massimo la lunghezza desiderata
			if(buff.size() > _dim_buff)
				buff = buff.substr(buff.size()-_dim_buff,_dim_buff);
		}while(!f.eof() || (f.eof() && look.size()>0));

		//chiudo i due file
		bw.~bitwriter();
		f.close();
		fout.close();
	}
};

#endif //HASH_H
#ifndef HUFFMAN_H
#define HUFFMAN_H

#include <list>
#include <algorithm>
#include <functional>
#include <vector>
#include <map>
#include <iostream>
#include <fstream>
#include "bit.h"

using namespace std;

//struttura dati del nodo di huffman
struct dati{
	unsigned char val;
	unsigned char codice;
	unsigned lunghezza;
	double probab;
	dati *sx;
	dati *dx;
};

bool ordina(dati *a, dati *b){
		return a->probab > b->probab;
	}

class huffman{
	list<dati*> _lista;
	string _nomefile;
	map<unsigned char,dati> _mappa;
	map<unsigned char,dati> _canonico;
	int i;
	unsigned int lunghezzaMassima;
	vector<pair<unsigned,int>> vettoreCanonico;

	int *firstcode;
	byte **symbol;

public:
	//costruttore
	huffman(string nome) : _nomefile(nome) {}

	void calcolaHuffman(){
	//apre il file da codificare
	fstream fin(_nomefile, fstream::in | fstream::binary);
	unsigned char c;
	//contatore che legge il numero totale di byte presenti nel file
	unsigned tot = 0;
	//vettore contenente le frequenze di tutti i byte
	vector<unsigned> freq(256);
	while(!fin.eof()){
		fin.read((char*)&c,1);
		if(fin.eof())
			break;
		freq[c]++;
		++tot;
	}
	//calcolo le probabilità dei valori
	vector<double> prob(256);
	transform(freq.begin(), freq.end(), prob.begin(), bind2nd(divides<double>(), tot));
	//richiamo il metodo creaAlbero che andrà a creare la struttura
	creaAlbero(prob);
	creaMappa();
	creaCanonico();

	//chiude il file di lettura
	fin.close();
}

void creaAlbero(const vector<double>& prob){
	//creazione della lista
	for(unsigned i=0; i<prob.size(); ++i){
		if(prob[i] > 0){
			dati *tmp = new dati();
			tmp->val = i;
			tmp->probab = prob[i];
			tmp->lunghezza = 0;
			tmp->dx = NULL;
			tmp->sx = NULL;
			_lista.push_back(tmp);
		}
	}
	//oridnamento della lista
	_lista.sort(ordina);
	//finchè ci sono due nodi continua a ciclare
	while(_lista.size() > 1){
		//coipa il primo nodo
		dati *tmp1 = _lista.back();
		_lista.pop_back();
		//copia il secondo nodo
		dati *tmp2 = _lista.back();
		_lista.pop_back();
		//crea un nuovo nodo
		dati *tmp = new dati();
		tmp->probab = tmp1->probab + tmp2->probab;
		tmp->sx=tmp1;
		tmp->dx = tmp2;
		tmp->lunghezza = 0;
		_lista.push_back(tmp);
		_lista.sort(ordina);
	}
}

//legge il nodo sinistro dell'albero
void leggiSx(int num, int l, dati *nodo){
	if(nodo->sx == NULL && nodo->dx == NULL){
		dati tmp;
		tmp.codice = num;
		tmp.lunghezza = l;
		_mappa.insert(pair<int,dati>(nodo->val,tmp));
	}
	else{
		leggiSx((num<<1),l+1,nodo->sx);
		leggiDx((num<<1)+1,l+1,nodo->dx);
	}
}

//legge il nodo destro dell'albero
void leggiDx(int num, int l, dati *nodo){
	if(nodo->sx == NULL && nodo->dx == NULL){
		dati tmp;
		tmp.codice = num;
		tmp.lunghezza = l;
		_mappa.insert(pair<unsigned char,dati>(nodo->val,tmp));
	}
	else{
		leggiSx((num<<1),l+1,nodo->sx);
		leggiDx((num<<1)+1,l+1,nodo->dx);
	}
}

//crea la mappa secondo l'albero di huffman
void creaMappa(){
	int n = 0;
	leggiSx((n<<1),1,_lista.front()->sx);
	leggiDx((n<<1)+1,1,_lista.front()->dx);
}

//crea il dizionario di huffman canonico
//dopo aver calcolato la lunghezza di ogni simbolo
void creaCanonico(){
	map<unsigned char,dati>::iterator it(_mappa.begin());
	vector<pair<unsigned,int>> tmp;

	//Setto la lunghezza massima iniziale uguale a 0
	lunghezzaMassima = 0;

	//ordino la mappa di huffman secondo la lunghezza dei simboli
	while(it != _mappa.end()){
		tmp.push_back(pair<unsigned,unsigned char>(it->second.lunghezza,it->first));

		if(it->second.lunghezza > lunghezzaMassima)
			lunghezzaMassima = it->second.lunghezza;
		it++;
	}
	sort(tmp.begin(),tmp.end());

	//Copio il vettore
	vettoreCanonico = tmp;
	
	/*
	//huffman canonico
	unsigned char code = 0;
	unsigned i;
	dati d;
	//applico il codice per calcolare il simbolo canonico
	for(i=0; i<tmp.size()-1; ++i){
		code = (code+1) << (tmp[i+1].first - tmp[i].first);
		d.codice = code;
		d.lunghezza = tmp[i].first;
		//inserisco il simbolo nel dizionario
		_canonico.insert(pair<unsigned char,dati>(tmp[i].second,d));
	}
	code += 1;
	d.codice = code;
	d.lunghezza = tmp[i].first;
	//inserisco l'ultimo simbolo nel dizionario
	_canonico.insert(pair<unsigned char,dati>(tmp[i].second,d));
	*/

	/*
	*	PROVA
	*/
	int *numl = new int[lunghezzaMassima+1];
	firstcode = new int[lunghezzaMassima+1];
	int *nextcode = new int[lunghezzaMassima+1];
	int *codeword = new int[tmp.size()];

	//memset(numl,0,sizeof(int));
	for (unsigned k = 0; k < lunghezzaMassima+1; k++)
	{
		numl[k] = 0;
	}
	for (unsigned i = 0; i < tmp.size(); i++)
	{
		numl[tmp[i].first]++;
	}
	firstcode[lunghezzaMassima] = 0;
	for (unsigned k = lunghezzaMassima-1; k > 0; k--)
	{
		firstcode[k] = (firstcode[k+1] + numl[k+1])/2;
	}
	for (unsigned k = 1; k < lunghezzaMassima+1; k++)
	{
		nextcode[k] = firstcode[k];
	}
	dati d;
	for (unsigned i = 0; i < tmp.size(); i++)
	{
		codeword[i] = nextcode[tmp[i].first];
		nextcode[tmp[i].first] = nextcode[tmp[i].first] + 1;

		d.codice = codeword[i];
		d.lunghezza = tmp[i].first;
		_canonico.insert(pair<byte, dati>(tmp[i].second,d));
	}
}

void scriviHuffman(string nome){
	//creo il file da scrivere
	fstream fout(nome, fstream::out | fstream::binary);
	bitwriter bw(fout);
	//apro il file da codificare
	fstream fin(_nomefile, fstream::in | fstream::binary);
	unsigned char c;
	while(!fin.eof()){
		fin.read((char*)&c,1);
		if(fin.eof())
			break;
		//scrivo il file codificato a lunghezza variabile
		bw.write(_canonico[c].codice,_canonico[c].lunghezza);
	}
	//richiamo il costruttore e chiudo il file
	bw.~bitwriter();
	fout.close();
}

unsigned long scriviHuffman(fstream& file){
	bitwriter bw(file);
	//apro il file da codificare
	fstream fin(_nomefile, fstream::in | fstream::binary);
	unsigned char c;

	//Per prima cosa scrivo i simboli e le relative lunghezze
	bw.write(vettoreCanonico.size()-1,8);

	unsigned numBit = bw.numBit(lunghezzaMassima);
	bw.write(numBit,8);

	for (unsigned i = 0; i < vettoreCanonico.size(); ++i)
	{
		bw.write(vettoreCanonico[i].second,8);
		bw.write(vettoreCanonico[i].first,numBit);
	}
	/*map<unsigned char,dati>::iterator it;
	for(it = _canonico.begin(); it != _canonico.end(); ++it){
		bw.write(it->first, 8);
		bw.write(it->second.lunghezza, numBit);
	}*/

	while(!fin.eof()){
		fin.read((char*)&c,1);
		if(fin.eof())
			break;
		//scrivo il file codificato a lunghezza variabile
		bw.write(_canonico[c].codice,_canonico[c].lunghezza);
	}
	//richiamo il costruttore e chiudo il file
	bw.~bitwriter();

	unsigned long t = bw.getByteScritti();

	//ritorna il numero di byte scritti
	return bw.getByteScritti();
}

string leggiHuffman(fstream& file) {
	bitreader br(file);

	byte carattere = 'a';
	string nomeFile ="";
	unsigned long lunghezzaFile = 0;
	unsigned lunghezzaVettore = 0;
	unsigned numBits = 0;
	vector<pair<unsigned,byte>> vettoreTmp;

	while (true)
	{
		carattere = file.get();
		if(file.good()) 
		{
			if (carattere != '#')
				nomeFile.append(1,carattere);
			else
				break;
		}
	}

	file.read((char*)&lunghezzaFile, sizeof(unsigned long));
	br.read(lunghezzaVettore, 8);
	++lunghezzaVettore;
	br.read(numBits,8);

	unsigned resto = 0;
	lunghezzaFile -= 2;

	for (unsigned i = 0; i < lunghezzaVettore; ++i)
	{
		unsigned a = 0;
		unsigned b = 0;

		br.read(a,8);
		br.read(b,numBits);

		vettoreTmp.push_back(pair<unsigned, byte>(b, a));
	}

	lunghezzaFile -= lunghezzaVettore;
	lunghezzaFile -= ((numBits*lunghezzaVettore)/8);
	resto = (numBits*lunghezzaVettore)%8;

	creaCanonicoDecodifica(vettoreTmp);
	
	fstream fout(_nomefile, fstream::out | fstream::binary);
	unsigned val = 0;
	unsigned bit = 0;
	unsigned count = 0;
	unsigned long lung = br.getByteLetti();
	unsigned bits = br.getBitLetti();

	//map<byte ,dati>::iterator it;

	bitwriter bw(fout);
	while (!file.eof() || lunghezzaFile==0)
	{
		int v;
		unsigned tmp = 0;
		unsigned k =1;
		bool fine = false;

		br.read(tmp);
		++resto;
		if(resto==8)
		{
			--lunghezzaFile;
			resto=0;
		}
		if(file.eof() || lunghezzaFile==0)
			break;
		v = tmp;
		while (v<firstcode[k])
		{
			br.read(tmp);
			++resto;
			if(resto==8)
				{
					--lunghezzaFile;
					resto=0;
				}
			if (file.eof() || lunghezzaFile==0)
			{
				fine = true;
				break;
			}
			v = 2*v + tmp;
			++k;
		}

		if(fine)
			break;

		//cout << symbol[k][v-firstcode[k]+1];
		fout.write((char*)&symbol[k][v-firstcode[k]+1],sizeof(byte));

	}
	
	//br.read(val);
	//++bits;
	//if(bits==8)
	//{
	//	lung++;
	//	bits=0;
	//}

	//if(lung == lunghezzaFile)
	//	return 0;
	////val |= bit << 1;

	//while (true)
	//{
	//	
	//	++count;
	//	for (it = _canonico.begin(); it != _canonico.end(); ++it)
	//	{
	//		if((it->second.codice == val) && (it->second.lunghezza == count))
	//		{
	//			//fout.write((char*)&it->first,sizeof(byte));
	//			bw.write(it->first,8);
	//			//cout << it->first << endl;
	//			val = 0;
	//			count = 0;
	//			break;
	//		}
	//	}

	//	br.read(bit);
	//	val = (val<<1)|bit;

	//	++bits;
	//	if(bits==8)
	//	{
	//		lung++;
	//		bits=0;
	//	}

	//	if(lung > lunghezzaFile)
	//		break;
	//}

	bw.~bitwriter();
	fout.close();

	return nomeFile;
}

void creaCanonicoDecodifica(vector<pair<unsigned, byte>> tmp){
	////huffman canonico
	//unsigned char code = 0;
	//unsigned i;
	//dati d;
	////applico il codice per calcolare il simbolo canonico
	//for(i=0; i<tmp.size()-1; ++i){
	//	code = (code+1) << (tmp[i+1].first - tmp[i].first);
	//	d.codice = code;
	//	d.lunghezza = tmp[i].first;
	//	//inserisco il simbolo nel dizionario
	//	_canonico.insert(pair<unsigned char,dati>(tmp[i].second,d));
	//}
	//code += 1;
	//d.codice = code;
	//d.lunghezza = tmp[i].first;
	////inserisco l'ultimo simbolo nel dizionario
	//_canonico.insert(pair<unsigned char,dati>(tmp[i].second,d));

	lunghezzaMassima = tmp[tmp.size()-1].first;
	int *numl = new int[lunghezzaMassima+1];
	firstcode = new int[lunghezzaMassima+1];
	int *nextcode = new int[lunghezzaMassima+1];
	int *codeword = new int[tmp.size()];
	symbol = new byte*[lunghezzaMassima+1];

	for (unsigned k = 0; k < lunghezzaMassima+1; k++)
	{
		numl[k] = 0;
	}
	for (unsigned i = 0; i < tmp.size(); i++)
	{
		numl[tmp[i].first]++;
	}
	firstcode[lunghezzaMassima] = 0;
	for (unsigned k = lunghezzaMassima-1; k > 0; k--)
	{
		firstcode[k] = (firstcode[k+1] + numl[k+1])/2;
	}
	for (unsigned k = 1; k < lunghezzaMassima+1; k++)
	{
		nextcode[k] = firstcode[k];
	}
	dati d;
		for (unsigned i = 0; i < lunghezzaMassima+1; i++)
	{
		symbol[i] = new byte[256];
	}
	for (unsigned i = 0; i < tmp.size(); i++)
	{
		codeword[i] = nextcode[tmp[i].first];
		nextcode[tmp[i].first] = nextcode[tmp[i].first] + 1;
		symbol[tmp[i].first][nextcode[tmp[i].first]-firstcode[tmp[i].first]] = tmp[i].second;

		d.codice = codeword[i];
		d.lunghezza = tmp[i].first;
		_canonico.insert(pair<byte, dati>(tmp[i].second,d));
	}
}

};

#endif //HUFFMAN_H
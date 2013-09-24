/**
	Progetto di Laboratorio di Ingegneria Informatica
	Sistemi di Elaborazione Multimediale

	Titolo: Progetto e implementazione di un software per la gestione 
			archivi di file compressi con LZ77 ottimizzato in termini di velocità

	Leader di progetto:	Paganelli Alberto
	Altri componenti:   Montanari Mirko
						Lancellotti Matteo
**/

#include <iostream>
#include <intrin.h>
#include <time.h>
#include "lzss.h"
#include "huffman.h"
#include "hash.h"

using namespace std;

void main(int argc, char *argv[]){

	clock_t start, end;
	double tempo;
	start = clock();

	//visualizza a schermo l'help
	if(argc==2 && strcmp(argv[1],"-help")==0){
		cout << endl << "Sintassi corretta: LMPzip [-c|-d file_sorgente file_destinazione] | [-help]" << endl << endl;
		cout << "-c \t compressione del file" << endl;
		cout << "-d \t decompressione del file" << endl << endl;
		return;
	}

	//se inserisco meno di 4 parametri visualizzo l'errore e la corretta sintassi
	if(argc < 4){
		cout << endl << "Errore d'esecuzione." << endl;
		cout << "Sintassi corretta: LMPzip [-c|-d file_sorgente file_destinazione] | [-help]" << endl << endl;
		cout << "-c \t compressione del file" << endl;
		cout << "-d \t decompressione del file" << endl << endl;
		return;
	}

	//se il parametro non è tra quelli indicati visualizzo un errore e la corretta sintassi
	if(strcmp(argv[1],"-c")!=0 && strcmp(argv[1],"-d")!=0){
		cout << endl << "Errore d'esecuzione." << endl;
		cout << "Sintassi corretta: LMPzip [-c|-d file_sorgente file_destinazione] | [-help]" << endl << endl;
		cout << "-c \t compressione del file" << endl;
		cout << "-d \t decompressione del file" << endl << endl;
		return;
	}

	cout << endl;
	cout << "Progetto di Laboratorio di Ingegneria Informatica" << endl;
	cout << "Sistemi di Elaborazione Multimediale" << endl;
	cout << "Titolo: Progetto e implementazione di un software per" << endl;
	cout << "\t la gestione di archivi di file compressi con LZ77 ottimizzato in termini di velocita'" << endl << endl;
	cout << "Leader di progetto: Paganelli Alberto" << endl;
	cout << "Altri componenti: Montanari Mirko e Lancellotti Matteo" << endl << endl;
	
	//funzione di compressione
	if(strcmp(argv[1],"-c")==0){
		cout << "Compressione in corso..." << endl;
		//compressione
		//LMPhash h(10,10,10);
		//h.creaFileHash(argv[2],"hash.tmp");
		lzss_codifica(argv[2],"hash.tmp", 4096, 32);

		fstream fout(argv[3], fstream::out | fstream::binary);
		fout << argv[2];
		fout << "#";
		unsigned long n = 100;
		fout.write((char*)&n,sizeof(unsigned long));
		//fout << "#";
		huffman huff("hash.tmp");
		huff.calcolaHuffman();
		n = huff.scriviHuffman(fout);
		long pos = fout.tellp();
		fout.seekp(pos - n - 4);
		fout.write((char*)&n,sizeof(long));
		fout.close();
		//cancella il file temporaneo
		//_unlink("hash.tmp");

		cout << "Compressione eseguita!" << endl << endl;
	}

	//funzione di decompressione
	if(strcmp(argv[1],"-d")==0){
		cout << "Decompressione in corso..." << endl;
		//compressione
		fstream fin(argv[2], fstream::in | fstream::binary);
		huffman huf("hash2.tmp");
		string nome = huf.leggiHuffman(fin);
		lzss_decodifica("hash2.tmp", nome);
		//lzss_decodifica("hash.tmp", "test.pdf");
		_unlink("hash2.tmp");
		cout << "Decompressione eseguita!" << endl << endl;
	}
	
	end = clock();
	tempo = ((double)(end-start)/CLOCKS_PER_SEC);

	cout << "Tempo impegato: " << tempo << endl;

}



#include <fstream>
#include <iostream>

#include "bit.h"
#include "lzss.h"

//byte dizionario[DIZIONARIO];
//byte lookahead[LOOKAHEAD];
//
//unsigned int prossimo[DIZIONARIO];
unsigned int tabella_hash[HASH];

byte *dizionario;
byte *lookahead;

unsigned int *prossimo;


Lzss_lmp::risultati_ricerca trovato;

using namespace std;

void Lzss_lmp::lzss_codifica(string inputFilename, string outputFilename){

	dizionario = new byte[dim_diz];
	lookahead = new byte[dim_look];
	prossimo = new unsigned int[dim_diz];

	unsigned int lun = 0;
	unsigned int puntatore_dizionario, puntatore_lookahead;
	unsigned int numero_caratteri;
	byte c;

	fstream in(inputFilename.c_str(), fstream::in | fstream::binary);
	fstream out(outputFilename.c_str(), fstream::out | fstream::binary);
	bitwriter bw(out);

	/*in.seekg(0,ios_base::end);
	unsigned long len = in.tellg();
	in.seekg(0,ios_base::beg);

	out.write((char*)&len,sizeof(unsigned long));*/

	bw.write(dim_diz-1,12);
	bw.write(dim_look-1,5);

	//All'inizio setto tutto il dizionario vuoto
	memset(dizionario, ' ',  dim_diz * sizeof(byte));

	//Riempio il lookahead buffer
	while(true)
	{
		if(lun == dim_look)
			break;

		c = in.get();

		if(in.eof())
			break;

		lookahead[lun] = c;
		++lun;
	}

	//Inizializzo la tabella hash
	memset(tabella_hash, HASH_NULL, HASH*sizeof(unsigned int));

	//Inizializzo la tabella utilizzata per gestire le collisioni
	for (unsigned int i = 0; i < dim_diz-1; i++)
	{
		prossimo[i]=i+1;
	}
	prossimo[dim_diz-1] = HASH_NULL;

	for (int i = 0; i < HASH; i++)
	{
		tabella_hash[i] = HASH_NULL;
	}

	//Calcolo il primo valore di hash e sicuramente non avrò niente
	tabella_hash[calcola_hash(0, NO_LOOKAHEAD)] = 0;

	puntatore_dizionario = 0;
	puntatore_lookahead = 0;
	numero_caratteri = 0;

	unsigned bit_diz = bw.numBit(dim_diz);
	unsigned bit_look = bw.numBit(dim_look);

	//Cerco la prima corrispondenza
	trovato = cerca(puntatore_dizionario, puntatore_lookahead, numero_caratteri);

	while (lun > 0)
	{
		if (trovato.lunghezza > lun)
		{
			trovato.lunghezza = lun;
		}

		if (trovato.lunghezza <= LUNGHEZZA_MINIMA_HASH)
		{
			bw.write(1);
			bw.write(lookahead[puntatore_lookahead], 8);

			trovato.lunghezza=1;

			//cout << "1," << lookahead[puntatore_lookahead] << endl;
			
		}
		else
		{
			bw.write(0);
			//if(numero_caratteri > DIZIONARIO)
			//{
			//	bw.write(DIZIONARIO - trovato.posizione, 12);
			//}
			//else
			//{
			//	bw.write(numero_caratteri - trovato.posizione,12);
			//}
			bw.write(trovato.posizione, bit_diz);
			//bw.write(puntatore_lookahead - trovato.posizione, 12);
			bw.write(trovato.lunghezza, bit_look);

			//cout << "0," << puntatore_lookahead - trovato.posizione << "," << trovato.lunghezza << endl;
		}

		unsigned i = 0;

		while ((i < trovato.lunghezza) && (!in.eof()))
		{
			c = in.get();
			sostituisciCarattere(puntatore_dizionario, lookahead[puntatore_lookahead]);
			lookahead[puntatore_lookahead] = c;
			puntatore_dizionario = aggiustaValore((puntatore_dizionario+1),dim_diz);
			puntatore_lookahead = aggiustaValore((puntatore_lookahead +1 ), dim_look);
			i++;
			numero_caratteri++;
		}

		while (i < trovato.lunghezza)
		{
			sostituisciCarattere(puntatore_dizionario, lookahead[puntatore_lookahead]);
			puntatore_dizionario = aggiustaValore((puntatore_dizionario+1),dim_diz);
			puntatore_lookahead = aggiustaValore((puntatore_lookahead +1 ), dim_look);
			i++;
			numero_caratteri++;
			lun--;
		}

		trovato = cerca(puntatore_dizionario, puntatore_lookahead, numero_caratteri);

	}
}

unsigned int Lzss_lmp::calcola_hash(unsigned int posizione, unsigned int is_lookahead){

	int risultato;

	risultato = 0;

	//Se devo calcolare l'hash nel lookahead buffer
	if(is_lookahead)
	{
		for (int i = 0; i < LUNGHEZZA_MINIMA_HASH; i++)
		{
			risultato = (risultato << 5) ^ (lookahead[posizione]);
			risultato %= HASH;
			posizione = aggiustaValore((posizione + 1),dim_look);
		}
	}
	//Se sono nel dizionario
	else
	{
		for (int i = 0; i < LUNGHEZZA_MINIMA_HASH; i++)
		{
			risultato = (risultato << 5) ^ (dizionario[posizione]);
			risultato %= HASH;
			posizione = aggiustaValore((posizione + 1), dim_diz);
		}
	}

	return risultato;

}

Lzss_lmp::risultati_ricerca Lzss_lmp::cerca(unsigned int puntatore_dizionario, unsigned int puntatore_lookahed, unsigned int numero_caratteri)
{
	risultati_ricerca risultato;
	//unsigned int i,j,k,d;

	byte a,b,c;

	risultato.lunghezza=0;
	risultato.posizione=0;

	unsigned int i = tabella_hash[calcola_hash(puntatore_lookahed, SI_LOOKAHEAD)];
	unsigned int j,k,d;

	j=0;
	k=0;
	d=0;

	unsigned int nc;

	while (i != HASH_NULL)
	{
		if(numero_caratteri > dim_diz)
			nc = dim_diz;
		else
			nc = numero_caratteri;

		if (dizionario[i] == lookahead[puntatore_lookahed])
		{
			j = 1;
			c=dizionario[aggiustaValore((i+j), dim_diz)];
			b=lookahead[aggiustaValore((puntatore_lookahed + j),dim_look)];

			while ((!k && (c == b)) || (k && (a == b)))
			{
				if(j >= dim_look)
					break;

				j++;
				d++;

				if((i+j)%nc == 0)
				{
					k=1;
					d=0;
				}

				a=dizionario[aggiustaValore((i + d), dim_diz)];
				b=lookahead[aggiustaValore((puntatore_lookahed + j), dim_look)];
				c=dizionario[aggiustaValore((i + j), dim_diz)];

			}

			if (j > risultato.lunghezza)
			{
				risultato.lunghezza = j;
				risultato.posizione = i;
			}

		}

		if (j >= dim_look)
		{
			risultato.lunghezza = dim_look;
			break;
		}

		i = prossimo[i];

	}

	return risultato;
}

void Lzss_lmp::sostituisciCarattere(unsigned int indice, byte sostituito)
{

	unsigned int primoIndice, i;

	if(indice < 2)
		primoIndice = (dim_diz + indice) - LUNGHEZZA_MINIMA_HASH;
	else
		primoIndice = indice - LUNGHEZZA_MINIMA_HASH;

	for (i = 0; i < 3; i++)
	{
		togliStringa(aggiustaValore((primoIndice +i), dim_diz));
	}

	dizionario[indice] = sostituito;

		for (i = 0; i < 3; i++)
	{
		aggiungiStringa(aggiustaValore((primoIndice +i), dim_diz));
	}

}

void Lzss_lmp::togliStringa(unsigned int indice)
{

	unsigned int i, tmpHash, next;

	next = prossimo[indice];
	prossimo[indice] = HASH_NULL;

	tmpHash = calcola_hash(indice, NO_LOOKAHEAD);

	if(tabella_hash[tmpHash] == indice)
	{
		tabella_hash[tmpHash] = next;
		return;
	}

	i = tabella_hash[tmpHash];

	while (prossimo[i] != indice)
	{
		i = prossimo[i];
	}

	prossimo[i] = next;
}

void Lzss_lmp::aggiungiStringa(unsigned int indice)
{

	unsigned int i, tmpHash;

	prossimo[indice] = HASH_NULL;

	tmpHash = calcola_hash(indice, NO_LOOKAHEAD);

	if(tabella_hash[tmpHash] == HASH_NULL)
	{
		tabella_hash[tmpHash] = indice;
		return;
	}

	i = tabella_hash[tmpHash];

	while (prossimo[i] != HASH_NULL)
	{
		i = prossimo[i];
	}

	prossimo[i] = indice;
}

unsigned Lzss_lmp::aggiustaValore(unsigned value, unsigned limite){
	//return (((value) < (limit)) ? (value) : ((value) - (limit)));
	return (((value) < (limite)) ? (value) : (value%limite));
}

void Lzss_lmp::lzss_decodifica(std::string inputFilename, std::string outputFilename){

	fstream fin(inputFilename.c_str(), fstream::in | fstream::binary);
	fstream fout(outputFilename.c_str(), fstream::out | fstream::binary);
	bitreader br(fin);

	unsigned int diz, look;

	br.read(diz, 12);
	br.read(look, 5);

	dim_diz=diz+1;
	dim_look=look+1;

	dizionario = new byte[dim_diz];
	lookahead = new byte[look];

	unsigned bit_diz = br.numBit(dim_diz);
	unsigned bit_look = br.numBit(dim_look);

	/*unsigned long len = 0;
	fin.read((char*)&len,sizeof(unsigned long));*/
	//All'inizio setto tutto il dizionario vuoto
	memset(dizionario, ' ',  dim_diz * sizeof(byte));

	unsigned posizione = 0;
	unsigned bit = 0;
	unsigned carattere = 0;
	unsigned offset = 0;
	unsigned lunghezza = 0;

	unsigned prova = 0;

	//fstream tmp("prova.pdf", fstream::in | fstream::binary);
	//byte tmpC = ' ';

	while (!fin.eof())
	{
		/*if(posizione == len)
			break;*/

		br.read(bit);
		prova++;
		if(fin.eof())
			break;
		
		if (bit)
		{
			br.read(carattere,8);
			prova+=8;
			if(fin.eof())
				break;
			fout.write((char*)&carattere, sizeof(byte));
			dizionario[posizione%dim_diz] = (byte)carattere;
			//tmpC = tmp.get();
			//if(tmpC != carattere)
			//	cout << "PORCA TROIA" << endl;
			++posizione;
			/*if(posizione==DIZIONARIO)
				posizione=0;*/
		}
		else
		{
			br.read(offset, bit_diz);
			br.read(lunghezza, bit_look);
			prova+=bit_diz+bit_look;
			if((fin.eof()) || (offset == 0 && lunghezza == 0))
				break;
			int j = 0;
			bool tornato = false;
			unsigned passi = 0;
			for (unsigned i = 0; i < lunghezza; i++)
			{
				if(offset==0)
					offset = 4096;

				if(posizione > dim_diz)
				{
					//tmpC = tmp.get();
					//if(tmpC != dizionario[(offset + j)%DIZIONARIO])
					//	cout << "PORCA TROIA 2" << endl;
					fout.write((char*)&dizionario[(offset + j)%dim_diz], sizeof(byte));
					lookahead[i] = dizionario[( offset + j)%dim_diz];
					passi++;
					//dizionario[(posizione+i)%DIZIONARIO] = dizionario[(DIZIONARIO - offset + j)%DIZIONARIO];
					if((dim_diz - offset) == 1)
					{
						j++;
					}
					else
					{
						if((((dim_diz-offset)==2) && (passi == 4)) )
						{
							tornato = true;
							j++;
						}else
						{
							j = ((j+1) == (dim_diz - offset)) ? 0 : ++j;
						}
						
					}
				}
				else 
				{
					//tmpC = tmp.get();
					//if(tmpC != dizionario[(offset + j)%DIZIONARIO])
					//	cout << "PORCA TROIA 3" << endl;
					fout.write((char*)&dizionario[(offset + j)%dim_diz], sizeof(byte));
					lookahead[i] = dizionario[(offset + j)%dim_diz];
					//dizionario[(posizione+i)%DIZIONARIO] = dizionario[(posizione - offset + j)%DIZIONARIO];
						j = ((j+1) == (posizione - offset)) ? 0 : ++j;
				}
			}
			j = 0;
			for (unsigned i = 0; i < lunghezza; i++)
			{
				dizionario[(posizione+i)%dim_diz] = lookahead[i];
			}
			posizione += lunghezza;
			offset = 0;
			lunghezza = 0;
		}

	}

}
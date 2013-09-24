#ifndef BIT_H
#define BIT_H

#include <fstream>

using namespace std;

typedef unsigned char byte;

class bitwriter{

	fstream& _file;
	int _numBit;
	byte _buffer;
	unsigned long _byteScritti;

public:
	bitwriter(fstream& file) : _file(file), _numBit(0), _byteScritti(0) {}

	~bitwriter(){
		while(_numBit>0){
			write(0);
		}
	}

	void write(unsigned bit){
		_buffer = _buffer << 1;
		_buffer = _buffer + (bit & 1);
		++_numBit;
		if(_numBit==8){
			_file.write((char *)&_buffer,1);
			_numBit=0;
			++_byteScritti;
		}
	}

	void write(unsigned bit, int n){
		while(n>0){
			write(bit>>--n);
		}
	}

		// ritorna il numero massimo di bit utilizzati per la rappresentazione dei valori
	unsigned numBit(unsigned val){
		int i = 0;
		while(val > 1){
			val = val / 2;
			++i;
		}
		++i;
		return i;
	}

	//restituisce il numero totale di byte scritti su file
	unsigned long getByteScritti(){
		return _byteScritti;
	}
};

class bitreader{

	fstream& _file;
	byte _buffer;
	int _numBit;
	unsigned long _byteLetti;

		void riempiBuffer(){
		_file.read((char *) &_buffer, 1);
		_numBit = 8;
		++_byteLetti;
	}

public:
	bitreader(fstream& file) : _file(file), _numBit(0), _byteLetti(0) {}

	void read(unsigned& bit){
		if(_numBit == 0)
			riempiBuffer();
		_numBit--;
		bit = (_buffer >> _numBit) & 1;
	}

	void read(unsigned& bit, int n){
		bit = 0;
		while(n>0){
			unsigned a;
			read(a);
			bit |= a<<--n;
		}
	}

	//restituisce il numero totale di byte letti su file
	unsigned long getByteLetti(){
		return _byteLetti;
	}

	unsigned getBitLetti(){
		unsigned a = 8- _numBit;
		return a;
	}

	unsigned numBit(unsigned val){
		int i = 0;
		while(val > 1){
			val = val / 2;
			++i;
		}
		++i;
		return i;
	}
};

#endif
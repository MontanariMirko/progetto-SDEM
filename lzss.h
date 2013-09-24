#include <string>

//#define DIZIONARIO 4096
//#define LOOKAHEAD 31
#define HASH 1024
#define HASH_NULL 4097
#define LUNGHEZZA_MINIMA_HASH 2
#define SI_LOOKAHEAD 1
#define NO_LOOKAHEAD 0

class Lzss_lmp{

public:
	typedef unsigned char byte;

	unsigned int dim_diz;
	unsigned int dim_look;

	typedef struct risultati_ricerca
	{
		unsigned int posizione;
		unsigned int lunghezza;
	} risultati_ricerca;

	Lzss_lmp(unsigned int diz, unsigned int look) : dim_diz(diz), dim_look(look) {};
	Lzss_lmp() : dim_diz(0), dim_look(0) {};

	void lzss_codifica(std::string inputFilename, std::string outputFilename);
	void lzss_decodifica(std::string inputFilename, std::string outputFilename);

private:
	unsigned int calcola_hash(unsigned int posizione, unsigned int is_lookahead);
	risultati_ricerca cerca(unsigned int puntatore_dizionario, unsigned int puntatore_lookahed, unsigned int numero_caratteri);
	void sostituisciCarattere(unsigned int indice, byte sostituito);
	void togliStringa(unsigned int indice);
	void aggiungiStringa(unsigned int indice);
	unsigned aggiustaValore(unsigned value, unsigned limite);

};
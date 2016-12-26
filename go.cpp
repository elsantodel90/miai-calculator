#define BOOST_DYNAMIC_BITSET_DONT_USE_FRIENDS

#include "combinatorios.h"
#include "boost/boost_1_62_0/boost/dynamic_bitset.hpp"
#include "boost/boost_1_62_0/boost/unordered_map.hpp"

typedef boost::dynamic_bitset<unsigned long long> Bitset;

struct Board
{
    Bitset bs;
    bool operator==(const Board &o) const { return bs == o.bs; }
};

std::size_t hash_value(const Board& b) {
    return boost::hash_value(b.bs.m_bits);
}


// Cuestiones de la "transposition table".

boost::unordered_map<Board, ThermoGraph > transpositionTable;

const Number MINUS_ONE(-1);

void makePending(ThermoGraph &t)
{
    t.left.v.clear();
    t.left.v.push_back(MINUS_ONE);
}

bool pending(const ThermoGraph &t)
{
    return !t.left.v.empty() && t.left.v.back().negative();
}

// KO NORMAL:
// IDEA: Usar la "regla trucha de resolucion de Ko", que da un valor miai super-optimista (seria el valor de un "Absolute Ko Monster").
//               Notar que se asume que "no hay tableros de 1xN" (mas precisamente, que toda interseccion tiene al menos 2 vecinas).

// CICLOS LARGOS:
//  Si el juego tiene un ciclo largo (back-edge: Salta como un "processing" en la lookup-table de termografos):
//       -> Opcion 1: avisa y se cancela todo.
//       -> Opcion 2: Lo analiza pero marca como "dirty" todos los ancestros hasta la cima de la back-edge, es decir que su resultado no se guarda.
//                     La opcion 2 tiene bastante sentido. Basicamente a los del ciclo se los recalcula cada vez para poder analizar los "vericuetos"
//                      particulares de SuperKo para cada uno. Ademas, de alguna manera hay que incorporar en las respuestas (TermoGrafos) la nocion
//                       de "No Result" que surge al elegir jugar el ciclo.
//           Plan: Implementar la 1, la 2 ya veremos XD

int main()
{
    Board b;
    transpositionTable[b] = ThermoGraph();
    int n; cin >> n;
    Bitset bs(n);
    bs.set(3);
    bs.set(4);
    bs.set(5);
    bs.reset(4);
    for (int i=0;i<n;i++) cout << bs.test(i) << endl;
    return 0;
}

#include "combinatorios.h"
#include <bitset>
#include <unordered_map>

const int MAX_AREA = 32;

typedef bitset<2*MAX_AREA> Bitset;

// Indices de jugadores: 0 BLACK, 1 WHITE
enum BoardIntersection {EMPTY = 0, KOBAN = 1, BLACK = 2, WHITE = 3}; 

int boardN, boardM;
int totalArea;

typedef unsigned char Index;

const Index OUTER_NULL = 255;
const Index OUTER_BLACK = 254;
const Index OUTER_WHITE = 253;

struct Board
{
    Bitset bs;
    bool operator==(const Board &o) const { return bs == o.bs; }
    bool emptyCell(Index pos) const { return bs[1|(pos<<1)] == 0; }
    bool emptyCellIsKobanned(Index pos) const { return bs[pos<<1] != 0; } // Asumiendo una celda vacia
    int stoneColor(Index pos) const { return bs[pos<<1];}                 // Asumiendo una celda no vacia (con piedra)
    BoardIntersection get(Index pos) const { return BoardIntersection((bs[1|(pos<<1)]<<1) | bs[pos<<1]); }
    void set(Index pos, BoardIntersection value) {
        bs[1|(pos<<1)] = (value >> 1);
        bs[pos<<1]     = (value & 1);
    }
};

// Board hash function
namespace std { template <> struct hash<Board> { std::size_t operator()(const Board& b) const { return hash<Bitset>()(b.bs); } }; }

// Cuestiones de la "transposition table".

unordered_map<Board, ThermoGraph > transpositionTable;

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

Index neighbors[MAX_AREA][4];

void fillNeighbors()
{
    
}

void readBoard()
{
    cin >> boardN >> boardM;
    assert(boardN >= 3);
    assert(boardM >= 3);
    boardN -= 2;
    boardM -= 2;
    totalArea = boardN * boardM;
    assert(totalArea <= MAX_AREA);
    fillNeighbors();
}

int main()
{
    assert(2*MAX_AREA <= OUTER_NULL);
    assert(2*MAX_AREA <= OUTER_BLACK);
    assert(2*MAX_AREA <= OUTER_WHITE);
    
    readBoard();
    
    Board b;
    
    transpositionTable[b] = ThermoGraph();
    Bitset bs;
    bs.set(3);
    bs.set(4);
    bs.set(5);
    bs.reset(4);
    for (int i=0;i<2*MAX_AREA;i++) cout << bs.test(i) << endl;
    return 0;
}

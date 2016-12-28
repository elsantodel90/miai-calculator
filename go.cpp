#include "combinatorios.h"
#include <cstring>
#include <bitset>
#include <unordered_map>

const int MAX_AREA = 32;

typedef bitset<2*MAX_AREA> Bitset;

// Indices de jugadores: 0 BLACK, 1 WHITE
enum BoardIntersection {EMPTY = 0, KOBAN = 1, BLACK = 2, WHITE = 3}; 

int boardN, boardM;
int totalArea;
int koMonster; // 0 o 1, jugador que es el Absolute Ko Monster

typedef unsigned char Index;

const Index OUTER_NULL = 255; // Se usa que sea "todos 1" en binario para memset
const Index OUTER_WHITE = 254;
const Index OUTER_BLACK = 253; // Se usa que OUTER_BLACK + 1 == OUTER_WHITE

struct Board
{
    Bitset bs;
    bool operator==(const Board &o) const { return bs == o.bs; }
    bool emptyCell(Index pos) const { return bs[1|(int(pos)<<1)] == 0; }
    bool emptyCellIsKobanned(Index pos) const { return bs[int(pos)<<1] != 0; } // Asumiendo una celda vacia
    int stoneColor(Index pos) const { return bs[int(pos)<<1];}                 // Asumiendo una celda no vacia (con piedra)
    BoardIntersection get(Index pos) const { return BoardIntersection((bs[1|(int(pos)<<1)]<<1) | bs[int(pos)<<1]); }
    void set(Index pos, BoardIntersection value) {
        bs[1|(int(pos)<<1)] = (value >> 1);
        bs[int(pos)<<1]     = (value & 1);
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

BoardIntersection charToCell(char c)
{
    switch (c)
    {
        case 'W':
            return WHITE;
        case 'B':
            return BLACK;
        case '.':
            return EMPTY;
        default:
            assert(false);
            break;
    }
}

Board fillNeighbors()
{
    const int di[4] = {0,0,1,-1};
    const int dj[4] = {1,-1,0,0};
    char board[MAX_AREA+2][MAX_AREA+2+1];
    for (int i=0;i<boardN+2; i++)
        cin >> board[i];
    Board startingBoard;
    for (int i=0;i<boardN;i++)
    for (int j=0;j<boardM;j++)
    {
        Index intersectionNumber = Index(boardM * i + j);
        startingBoard.set(intersectionNumber, charToCell(board[1+i][1+j]));
        for (int dir = 0; dir < 4; dir++)
        {
            int ni = i + di[dir];
            int nj = j + dj[dir];
            Index neighbor;
            if (ni == -1 || nj == -1 || ni == boardN || nj == boardM)
            {
                switch (board[1+ni][1+nj])
                {
                    case 'W':
                        neighbor = OUTER_WHITE;
                        break;
                    case 'B':
                        neighbor = OUTER_BLACK;
                        break;
                    case 'X':
                        neighbor = OUTER_NULL;
                        break;
                    default:
                        assert(false);
                        break;
                }
            }
            else
                neighbor = Index(boardM * ni + nj);
            neighbors[intersectionNumber][dir] = neighbor;
        }
    }
    return startingBoard;
}

Board readBoard()
{
    cin >> boardN >> boardM;
    assert(boardN >= 3);
    assert(boardM >= 3);
    boardN -= 2;
    boardM -= 2;
    totalArea = boardN * boardM;
    assert(totalArea <= MAX_AREA);
    return fillNeighbors();
}

void checkMoves()
{
    Board board;
    
    // Recolectamos informacion de los grupos libertades etc...
    
    Index queue[MAX_AREA]; // Tambien es naturalmente la lista piedras de cada grupo.
    char groupLiberties[MAX_AREA];
    Index groupEnd[MAX_AREA];
    Index groupId[MAX_AREA]; // o groupStart
    memset(groupId, OUTER_NULL, sizeof(groupId));
    Index qB = 0, qF = 0;
    Index kobanpos = OUTER_NULL;
    for (Index i = 0; i < totalArea; i++)
    {
        BoardIntersection val = board.get(i);
        if (val == KOBAN) kobanpos = i;
        if (groupId[i] == OUTER_NULL && (val == WHITE || val == BLACK))
        {
            Index currentGroup = groupId[i] = qB;
            groupLiberties[currentGroup] = 0;
            
            queue[qB++] = i;
            while (qF != qB)
            {
                Index x = queue[qF++];
                for (int dir = 0; dir < 4; dir++)
                {
                    Index y = neighbors[x][dir];
                    if (y == OUTER_NULL) continue;
                    if (y == OUTER_BLACK || y == OUTER_WHITE)
                    {
                        if ((val == BLACK && y == OUTER_BLACK) || (y == OUTER_WHITE && val == WHITE))
                            groupLiberties[currentGroup] = 2; // "+Inf" conceptualmente, pero 2 alcanza y queremos estar lejos de overflow.
                    }
                    else
                    {
                        BoardIntersection yVal = board.get(y);
                        if (groupId[y] == OUTER_NULL && yVal == val)
                        {
                            groupId[y] = currentGroup;
                            queue[qB++] = y;
                        }
                        else if (groupLiberties[currentGroup] < 2 && (yVal == EMPTY || yVal == KOBAN) && groupId[y] != currentGroup) // Pintamos ficticiamente las libertades con el grupo actual, para no contarlas varias veces.
                        {
                            groupLiberties[currentGroup]++;
                            groupId[y] = currentGroup;
                        }
                    }
                }
            }
            groupEnd[currentGroup] = qB;
        }
    }
    
    // Generamos todas las posiciones vecinas factibles para cada jugador
    for (int iter = 0; iter < 3; iter++)
    {
        int player;
        if (iter == 2)
        {
            if (kobanpos == OUTER_NULL) break;
            // kobanpos es una opcion de jugada para el KoMonster:
            // Hacer esa jugada (es facil de procesar a mano porque es solo la captura de la piedra de ko), y luego OTRA jugada en otra interseccion!
            player = koMonster;
            board.set(kobanpos, BoardIntersection(2+koMonster));
            for (int dir = 0; dir < 4; dir++)
            {
                Index y = neighbors[kobanpos][dir];
                if (y < totalArea && board.get(y) == BoardIntersection(2+(!koMonster)) && groupLiberties[groupId[y]] == 1)
                {
                    board.set(y, EMPTY);
                    break;
                }
            }
        }
        else
            player = iter;
        const int otherPlayer = !player;
        for (Index i = 0; i < totalArea; i++)
        {
            BoardIntersection val = board.get(i);
            if (val == EMPTY || (val == KOBAN &&  player != koMonster))
            {
                // Hacer jugada alli
                    //  -- Si tiene un vecino rival con una sola libertad, ese grupo entero fue capturado (hasta 4 y con repetidos). Vaciarlos y contarlos (prisioneros)
                    //  -- Si no capturo vecinos, verificar que no sea suicido: Una casilla vecina esta vacia, o bien tengo un vecino de mi propio color con 2 libertades.
                    //  -- Limpiar el KoBan previo que pudiera existir.
                    //  -- Si se captura exactamente una piedra rival con una piedra solitaria que ahora tiene exactamente una libertad (esa de la captura), ponerle el Ko-ban a esa celda si no somos el KoMonster.
                int groupCaptures = 0;
                Index captured[4];
                
                for (int dir = 0; dir < 4; dir++)
                {
                    Index y = neighbors[i][dir];
                    if (y < totalArea)
                    {
                        if (board.get(y) == BoardIntersection(2+otherPlayer) && groupLiberties[groupId[y]] == 1)
                        {
                            for (int j = 0; j < groupCaptures; j++) if (captured[j] == groupId[y]) break;
                            captured[groupCaptures++] = groupId[y];
                        }
                    }
                }
                if (groupCaptures > 0)
                {
                    // Ante capturas, copiamos y sabemos que la jugada es legal sin revisar si hubo suicidio.
                    Board newBoard = board;
                    // Realizar capturas
                    int stonesCaptured = 0;
                    for (int j = 0; j < groupCaptures; j++)
                    {
                        stonesCaptured += groupEnd[captured[j]] - captured[j];
                        for (Index pos = captured[j]; pos < groupEnd[captured[j]]; pos++)
                            newBoard.set(queue[pos], EMPTY);
                    }
                    if (stonesCaptured == 1 && player != koMonster)
                    {
                        // Verificar que sea una piedra solitaria con exactamente una libertad
                        int stoneLiberties = 0;
                        Index liberty;
                        for (int dir = 0; dir < 4; dir++)
                        {
                            Index y = neighbors[i][dir];
                            if (y == OUTER_BLACK + player) goto noKoban;
                            if (y < totalArea)
                            {
                                BoardIntersection yVal = newBoard.get(y);
                                if (yVal == EMPTY || yVal == KOBAN) {stoneLiberties++; liberty = y;}
                                else if (yVal == BoardIntersection(2 + player)) goto noKoban;
                            }
                        }
                        if (stoneLiberties == 1)
                            newBoard.set(liberty, KOBAN);
                    }
                    noKoban:;
                    // Agregar un koban si es necesario
                    newBoard.set(i,BoardIntersection(2+player));
                    if (kobanpos != i && kobanpos != OUTER_NULL) newBoard.set(kobanpos, EMPTY);
                    REPORT(newBoard);
                }
                else
                {
                    // Si no hubo captura, hay que verificar que no se viole la regla de no suicidio.
                    for (int dir = 0; dir < 4; dir++)
                    {
                        Index y = neighbors[i][dir];
                        if (y < totalArea)
                        {
                            if (board.emptyCell(y) || (board.stoneColor(y) == player && groupLiberties[groupId[y]] >= 2))
                                goto noSuicide;
                        }
                    }
                    continue; // Jugada suicida, no se procesa
                noSuicide:;
                    // Estamos ante la jugada tipica: no captura nada y no es suicidio.
                    board.set(i,BoardIntersection(2+player));
                    if (kobanpos != i && kobanpos != OUTER_NULL) board.set(kobanpos, EMPTY);
                    REPORT_MOVE(board);
                    if (kobanpos != i && kobanpos != OUTER_NULL) board.set(kobanpos, KOBAN);
                    board.set(i,val);
                }
            }
        }
    }
}

int main()
{
    assert(freopen("example.in","r",stdin));
    assert(MAX_AREA <= OUTER_NULL);
    assert(MAX_AREA <= OUTER_BLACK);
    assert(MAX_AREA <= OUTER_WHITE);
    
    Board startingBoard = readBoard();
    koMonster = 0;
    
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

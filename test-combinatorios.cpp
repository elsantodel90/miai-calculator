#include "combinatorios.h"

int main()
{
    GameTree zero;
    GameTree one{{zero}, {}};
    GameTree two{{zero, one, zero}, {}};
    GameTree three{{two, one, one, one, zero, zero, one, zero, zero}, {}};
    GameTree four{{two, one, three, one, zero, zero, one, three, zero}, {}};
        
    GameTree negone{{}, {zero}};
    GameTree negtwo{{}, {negone}};
    GameTree negthree{{}, {negtwo}};
    GameTree negfour{{}, {negthree}};

    GameTree twoZero{{two}, {zero}}, threeZero {{three}, {zero}};
    
    GameTree negtwoZero{{zero}, {negtwo}}, negthreeZero{{zero}, {negthree}};
    
    GameTree g1{{two}, {twoZero}}, g2{{two}, {threeZero}}, g3{{one}, {threeZero}}, g4{{four}, {threeZero}};
    
    GameTree h1{{negtwoZero}, {negtwo}}, h2{{negthreeZero}, {negtwo}}, h3{{negthreeZero}, {negone}}, h4{{negthreeZero}, {negfour}};
    
    ThermoGraph t;
    
    thermograph(t,four);
    assert(t.mast() == Number(4));
    assert(t.temperature() == ZERO);
    
    thermograph(t,one);
    assert(t.mast() == Number(1));
    assert(t.temperature() == ZERO);
    
    thermograph(t,zero);
    assert(t.mast() == ZERO);
    assert(t.temperature() == ZERO);
    
    thermograph(t,two);
    assert(t.mast() == Number(2));
    assert(t.temperature() == ZERO);
    
    thermograph(t,three);
    assert(t.mast() == Number(3));
    assert(t.temperature() == ZERO);
    
    thermograph(t,threeZero);
    assert(t.mast() == Number(3,1));
    assert(t.temperature() == Number(3,1));
    
    thermograph(t,twoZero);
    assert(t.mast() == Number(1));
    assert(t.temperature() == Number(1));
    
    thermograph(t,g1);
    assert(t.mast() == Number(2));
    assert(t.temperature() == Number(0));
    
    thermograph(t,g2);
    assert(t.mast() == Number(3));
    assert(t.temperature() == Number(0));
    
    thermograph(t,g3);
    assert(t.mast() == Number(2));
    assert(t.temperature() == Number(0));
    
    thermograph(t,g4);
    assert(t.mast() == Number(3));
    assert(t.temperature() == Number(1));
    
    thermograph(t,h1);
    assert(t.mast() == Number(-2));
    assert(t.temperature() == Number(0));
    
    thermograph(t,h2);
    assert(t.mast() == Number(-3));
    assert(t.temperature() == Number(0));
    
    thermograph(t,h3);
    assert(t.mast() == Number(-2));
    assert(t.temperature() == Number(0));
    
    thermograph(t,h4);
    assert(t.mast() == Number(-3));
    assert(t.temperature() == Number(1));
    
    
    GameTree star {{zero}, {zero}};
    GameTree game {{ {{star,negtwoZero},{negone,star}}  }, {negtwo}};
    thermograph(t,game);
    cout << t << endl;
    
    return 0;
}

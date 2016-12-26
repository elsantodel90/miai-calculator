#include <iostream>
#include <vector>
#include <cassert>

using namespace std;

typedef long long IntegerScore;

typedef long long NumberInt;
typedef unsigned char DenominatorExp;

struct Number
{
    NumberInt numerator;
    DenominatorExp denominatorExp;
    
    Number () : Number(0,0) {}
    explicit Number (IntegerScore score) : Number(score,0) {}
    Number (IntegerScore score, DenominatorExp den) : numerator(score), denominatorExp(den) {}
    bool operator ==(const Number &o) const {return numerator == o.numerator && denominatorExp == o.denominatorExp;}
    bool operator !=(const Number &o) const {return numerator != o.numerator || denominatorExp != o.denominatorExp;}
    bool operator <(const Number &o) const
    {
        NumberInt a = numerator, b = o.numerator;
        DenominatorExp ea = denominatorExp, eb = o.denominatorExp;
        DenominatorExp desired = max(ea, eb);
        while (ea < desired) { ea++; a *= 2;}
        while (eb < desired) { eb++; b *= 2;}
        return a < b;
    }
    bool operator <=(const Number &o) const 
    {
        NumberInt a = numerator, b = o.numerator;
        DenominatorExp ea = denominatorExp, eb = o.denominatorExp;
        DenominatorExp desired = max(ea, eb);
        while (ea < desired) { ea++; a *= 2;}
        while (eb < desired) { eb++; b *= 2;}
        return a <= b;
    }
    bool zero() const { return numerator == 0; }
    bool negative() const { return numerator < 0; }
    bool positive() const { return numerator > 0; }
    bool isInteger() const { return denominatorExp == 0; }
    void halve() { if (numerator != 0) denominatorExp++; }
    void negate() { numerator = -numerator; }
    Number& operator -=(const Number &o)
    {
        NumberInt b = o.numerator;
        DenominatorExp eb = o.denominatorExp;
        DenominatorExp desired = max(denominatorExp, eb);
        while (denominatorExp < desired) { denominatorExp++; numerator *= 2;}
        while (eb < desired) { eb++; b *= 2;}
        numerator -= b;
        while (denominatorExp > 0 && numerator % 2 == 0) numerator /= 2, denominatorExp--;
        return *this;
    }
    Number& operator +=(const Number &o)
    {
        NumberInt b = o.numerator;
        DenominatorExp eb = o.denominatorExp;
        DenominatorExp desired = max(denominatorExp, eb);
        while (denominatorExp < desired) { denominatorExp++; numerator *= 2;}
        while (eb < desired) { eb++; b *= 2;}
        numerator += b;
        while (denominatorExp > 0 && numerator % 2 == 0) numerator /= 2, denominatorExp--;
        return *this;
    }
    NumberInt denom() const {return 1LL << denominatorExp;}
};

ostream &operator<<(ostream & os, const Number &number)
{
    if (number.denominatorExp == 0) os << number.numerator;
    else if (number.numerator == 0) os << 0;
    else os << number.numerator << "/" << number.denom();
    return os;
}

const Number ZERO(0);
const Number INF(10000); // Temperatura absurdamente grande para nuestro contexto

enum SectionType {BELOW = 0, ABOVE = 1};

struct Section
{
    Number x;
    SectionType t;
    bool operator <(const Section &o) const { return x < o.x || (x == o.x && t < o.t); }
    bool negative() const {return x.negative() || (x.zero() && t == BELOW); }
    bool positive() const {return x.positive() || (x.zero() && t == ABOVE); }
    void negate() {t = SectionType(!t); x.negate();}
};

Number simplicityRule(Section a, Section b)
{
    assert(a < b);
    // El Number con menor denominador entre a y b. Si hay mas de un entero en rango, el de menor modulo.
    bool flip = false;
    if (a.negative())
    {
        if (b.positive()) return ZERO;
        // a < b < 0 [Como secciones]
        flip = true;
        swap(a,b);
        a.negate();
        b.negate();
    }
    // 0 < a < b [Como secciones]
    
    NumberInt nA = a.x.numerator;
    NumberInt nB = b.x.numerator;
    Number ret;
    DenominatorExp e = DenominatorExp(1 + max(a.x.denominatorExp, b.x.denominatorExp));
    nA <<= (e - a.x.denominatorExp);
    nB <<= (e - b.x.denominatorExp);
    if (a.t == BELOW) nA--;
    if (b.t == ABOVE) nB++;
    while (nB-nA > 1)
    {
        ret.numerator = nA+1;
        ret.denominatorExp = e;
        
        nA /= 2;
        nB = (nB+1) / 2;
        if (e == 0) break;
        e--;
    }
    assert(ret.denominatorExp == 0 || ret.numerator % 2 == 1);
    if (flip)
        ret.negate();
    return ret;
}

Number simplicityOnlyLeftOption(Section s)
{
    if (s.negative()) return ZERO;
    if (s.t == BELOW && s.x.isInteger()) return s.x;
    return Number(1 + (s.x.numerator >> s.x.denominatorExp));
}

Number simplicityOnlyRightOption(Section s)
{
    s.negate();
    Number x = simplicityOnlyLeftOption(s);
    x.negate();
    return x;
}

struct ThermoLine
{
    vector<Number> v;
    Section base;
    bool startsUp;
    void turnLine() {startsUp ^= 1;}
    bool terminal() {return startsUp && v.empty();}
};

struct ThermoGraph
{
    ThermoLine left, right;
    Number temperature() const
    {
        Number ret = ZERO;
        if (!left.v.empty()) ret = left.v.back();
        if (!right.v.empty()) ret = max(ret, right.v.back());
        return ret;
    }
    Number mast() const
    {
        Number mastVal = left.base.x, currentT;
        bool up = left.startsUp;
        for (Number nextT : left.v)
        {
            if (!up) {mastVal += currentT; mastVal -= nextT; }
            currentT = nextT;
            up ^= 1;
        }
        return mastVal;
    }
};

ostream & operator<<(ostream &os, const ThermoGraph &t)
{
    os << t.mast() << "(" << t.temperature() << ")";
    return os;
}

const ThermoGraph ZERO_THERMOGRAPH{{vector<Number>(), {ZERO, BELOW}, true}, {vector<Number>(), {ZERO, ABOVE}, true}};

void takeMax(ThermoLine &ret, const ThermoLine &line1, const ThermoLine &line2)
{
    // line1 y line2 son leftLines (suben y "restan" [A la derecha en termografo])
    ret.v.clear();
    ret.v.reserve(line1.v.size() + line2.v.size() + 1);
    
    Number A = line1.base.x,B = line2.base.x;
    bool aUp = line1.startsUp;
    bool bUp = line2.startsUp;
    Number C; bool cUp;
    if (A < B)
    {
        cUp = bUp;
        C = B;
    }
    else if (B < A)
    {
        cUp = aUp;
        C = A;
    }
    else
    {
        cUp = aUp || bUp;
        C = A;
    }
    ret.startsUp = cUp;
    ret.base = max(line1.base, line2.base);
    Number currentT; // Starts from temperature zero and up
    int i=0,j=0;
    while (i < (int)line1.v.size() || j < (int)line2.v.size())
    {
        assert(A <= C);
        assert(B <= C);
        assert((A == C && aUp == cUp) || (B == C && bUp == cUp));
        assert(A != B || cUp == (aUp || bUp));
        const Number nextT1 = (i < (int)line1.v.size()) ? line1.v[i] : INF;
        const Number nextT2 = (j < (int)line2.v.size()) ? line2.v[j] : INF;
        const Number nextT =  min(nextT1, nextT2);
        if ((aUp || bUp) && !cUp)
        {
            const Number upCoord = aUp ? A : B;
            assert(upCoord < C);
            Number colisionT = currentT;
            colisionT += C;
            colisionT -= upCoord;
            if (colisionT < nextT)
            {
                ret.v.push_back(colisionT);
                cUp = true;
                A = B = C = upCoord;
                continue;
            }
        }
        
        Number delta = nextT;
        delta -= currentT;
        if (!aUp) A -= delta;
        if (!bUp) B -= delta;
        
        if (nextT1 < nextT2)
        {
            i++;
            aUp ^= 1;
        }
        else if (nextT2 < nextT1)
        {
            j++;
            bUp ^= 1;
        }
        else
        {
            i++;
            j++;
            aUp ^= 1;
            bUp ^= 1;
        }
        currentT = nextT;
        if (A < B)
        {
            if (cUp != bUp)
                ret.v.push_back(currentT);
            cUp = bUp;
            C = B;
        }
        else if (B < A)
        {
            if (cUp != aUp)
                ret.v.push_back(currentT);
            cUp = aUp;
            C = A;
        }
        else
        {
            if (cUp != (aUp || bUp))
                ret.v.push_back(currentT);
            cUp = aUp || bUp;
            C = A;
        }
    }
}

void takeMin(ThermoLine &ret, const ThermoLine &line1, const ThermoLine &line2)
{
    // line1 y line2 son rightLines (suben y "suman" [A la izquierda en termografo])
    ret.v.clear();
    ret.v.reserve(line1.v.size() + line2.v.size() + 1);
    
    Number A = line1.base.x,B = line2.base.x;
    bool aUp = line1.startsUp;
    bool bUp = line2.startsUp;
    Number C; bool cUp;
    if (A < B)
    {
        cUp = aUp;
        C = A;
    }
    else if (B < A)
    {
        cUp = bUp;
        C = B;
    }
    else
    {
        cUp = aUp || bUp;
        C = A;
    }
    ret.startsUp = cUp;
    ret.base = min(line1.base, line2.base);
    Number currentT; // Starts from temperature zero and up
    int i=0,j=0;
    while (i < (int)line1.v.size() || j < (int)line2.v.size())
    {
        assert(C <= A);
        assert(C <= B);
        assert((A == C && aUp == cUp) || (B == C && bUp == cUp));
        assert(A != B || cUp == (aUp || bUp));
        const Number nextT1 = (i < (int)line1.v.size()) ? line1.v[i] : INF;
        const Number nextT2 = (j < (int)line2.v.size()) ? line2.v[j] : INF;
        const Number nextT =  min(nextT1, nextT2);
        if ((aUp || bUp) && !cUp)
        {
            const Number upCoord = aUp ? A : B;
            assert(C < upCoord);
            Number colisionT = currentT;
            colisionT += upCoord;
            colisionT -= C;
            if (colisionT < nextT)
            {
                ret.v.push_back(colisionT);
                cUp = true;
                A = B = C = upCoord;
                continue;
            }
        }
        
        Number delta = nextT;
        delta -= currentT;
        if (!aUp) A += delta;
        if (!bUp) B += delta;
        
        if (nextT1 < nextT2)
        {
            i++;
            aUp ^= 1;
        }
        else if (nextT2 < nextT1)
        {
            j++;
            bUp ^= 1;
        }
        else
        {
            i++;
            j++;
            aUp ^= 1;
            bUp ^= 1;
        }
        currentT = nextT;
        if (A < B)
        {
            if (cUp != aUp)
                ret.v.push_back(currentT);
            cUp = aUp;
            C = A;
        }
        else if (B < A)
        {
            if (cUp != bUp)
                ret.v.push_back(currentT);
            cUp = bUp;
            C = B;
        }
        else
        {
            if (cUp != (aUp || bUp))
                ret.v.push_back(currentT);
            cUp = aUp || bUp;
            C = A;
        }
    }
}

void mergeOnlyLeft(ThermoGraph &ret, const ThermoLine &leftLine)
{
    ThermoLine &leftRet = ret.left;
    ThermoLine &rightRet = ret.right;
    leftRet.base.x = rightRet.base.x = simplicityOnlyLeftOption(leftLine.base);
    leftRet.base.t = BELOW; rightRet.base.t = ABOVE;
    leftRet.startsUp = rightRet.startsUp = true;
    leftRet.v.clear(); rightRet.v.clear();
}

void mergeOnlyRight(ThermoGraph &ret, const ThermoLine &rightLine)
{
    ThermoLine &leftRet = ret.left;
    ThermoLine &rightRet = ret.right;
    leftRet.base.x = rightRet.base.x = simplicityOnlyRightOption(rightLine.base);
    leftRet.base.t = BELOW; rightRet.base.t = ABOVE;
    leftRet.startsUp = rightRet.startsUp = true;
    leftRet.v.clear(); rightRet.v.clear();
}

void merge(ThermoGraph &ret, const ThermoLine &leftLine, const ThermoLine &rightLine)
{
    // Se asume: leftLine termina hacia la derecha, rightLine termina hacia la izquierda (ninguna hacia arriba)
    ThermoLine &leftRet = ret.left;
    ThermoLine &rightRet = ret.right;
    Number A = leftLine.base.x,B = rightLine.base.x;
    if (leftLine.base < rightLine.base)
    {
        leftRet.base.x = rightRet.base.x = simplicityRule(leftLine.base,rightLine.base);
        leftRet.base.t = BELOW; rightRet.base.t = ABOVE;
        leftRet.startsUp = rightRet.startsUp = true;
        leftRet.v.clear(); rightRet.v.clear();
    }
    else if (A == B)
    {
        leftRet.base = leftLine.base;
        rightRet.base = rightLine.base;
        leftRet.startsUp = rightRet.startsUp = true;
        leftRet.v.clear(); rightRet.v.clear();
    }
    else
    {
        // Copiar pero con un espacio extra al final que tenga un +INF ("explicitamos" la representacion que lo lleva implicito)
        leftRet .v.resize(1 + leftLine .v.size());
        rightRet.v.resize(1 + rightLine.v.size());
        for (int i = 0; i < (int)leftLine.v.size(); i++)
            leftRet.v[i] = leftLine.v[i];
        for (int j = 0; j < (int)rightLine.v.size(); j++)
            rightRet.v[j] = rightLine.v[j];
        leftRet.v.back() = INF;
        rightRet.v.back() = INF;
        leftRet .base = leftLine .base;
        rightRet.base = rightLine.base;
        leftRet .startsUp = leftLine.startsUp;
        rightRet.startsUp = rightLine.startsUp;
        
        bool aUp = leftLine.startsUp;
        bool bUp = rightLine.startsUp;
        Number currentT; // Starts from temperature zero and up
        int i=0,j=0;
        while (true)
        {
            // Invariante: A > B
            const Number &nL = leftRet.v[i];
            const Number &nR = rightRet.v[j];
            const Number nextT = min(nL, nR);
            Number delta = nextT;
            delta -= currentT;
            if (!aUp) A -= delta;
            if (!bUp) B += delta;
            if (A <= B)
            {
                if (!aUp) A += delta;
                if (!bUp) B -= delta;
                break; // Encontramos el mastil
            }
            
            if (nL < nR)
            {
                i++;
                aUp ^= 1;
            }
            else if (nR < nL)
            {
                j++;
                bUp ^= 1;
            }
            else
            {
                i++;
                j++;
                aUp ^= 1;
                bUp ^= 1;
            }
            currentT = nextT;
        }
        // Calcular y agregar mastil + ultimos tramos (si existen porque no es "aUp" o "bUp"), y listo
        assert(B < A);
        assert(!(aUp && bUp));
        A -= B;
        if (aUp)
        {
            currentT += A;
            rightRet.v[j++] = currentT;
        }
        else if (bUp)
        {
            currentT += A;
            leftRet.v[i++] = currentT;
        }
        else 
        {
            A.halve();
            currentT += A;
            leftRet .v[i++] = currentT;
            rightRet.v[j++] = currentT;
        }
        leftRet.v.resize(i);
        rightRet.v.resize(j);
    }
}

struct GameTree
{
    vector<GameTree> left;
    vector<GameTree> right;
    bool operator ==(const GameTree &o) const { return left == o.left && right == o.right;}
};

void thermograph(ThermoGraph &ret, const GameTree &game)
{
    bool pri = true;
    ThermoLine bestLeft;
    for (const auto &option : game.left)
    {
        ThermoGraph otg;
        thermograph(otg, option);
        otg.right.startsUp ^= 1;
        if (pri)
        {
            bestLeft = otg.right;
            pri = false;
        }
        else
        {
            ThermoLine aux;
            takeMax(aux, bestLeft, otg.right);
            bestLeft = aux;
        }
    }
    pri = true;
    ThermoLine bestRight;
    for (const auto &option : game.right)
    {
        ThermoGraph otg;
        thermograph(otg, option);
        otg.left.startsUp ^= 1;
        if (pri)
        {
            bestRight = otg.left;
            pri = false;
        }
        else
        {
            ThermoLine aux;
            takeMin(aux, bestRight, otg.left);
            bestRight = aux;
        }
    }
    if (game.left.empty() && game.right.empty())
        ret = ZERO_THERMOGRAPH;
    else if (game.left.empty())
        mergeOnlyRight(ret, bestRight);
    else if (game.right.empty())
        mergeOnlyLeft(ret, bestLeft);
    else
        merge(ret, bestLeft, bestRight);
}

/*
╔══════════════════════════════════════════════════════════════════════╗
║  Made by Tim                                                 ║
║   ║
╚══════════════════════════════════════════════════════════════════════╝
*/

#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <ctime>
#include <iomanip>
#include <limits>
#include <sstream>
#include <algorithm>
#include <cmath>
#ifdef _WIN32
  #include <windows.h>
  #define MSLEEP(ms) Sleep(ms)
#else
  #include <unistd.h>
  #define MSLEEP(ms) usleep((ms)*1000)
#endif
using namespace std;

// ═══════════════════════════════════════════════════════════════════════
//  ANSI ESCAPE CODES
// ═══════════════════════════════════════════════════════════════════════

// Style
#define RST   "\033[0m"
#define BOLD  "\033[1m"
#define DIM   "\033[2m"
#define ITAL  "\033[3m"
#define UL    "\033[4m"

// Foreground
#define fBLK  "\033[30m"
#define fRED  "\033[31m"
#define fGRN  "\033[32m"
#define fYLW  "\033[33m"
#define fBLU  "\033[34m"
#define fMAG  "\033[35m"
#define fCYN  "\033[36m"
#define fWHT  "\033[37m"
#define fBRED "\033[91m"
#define fBGRN "\033[92m"
#define fBYLW "\033[93m"
#define fBBLU "\033[94m"
#define fBMAG "\033[95m"
#define fBCYN "\033[96m"
#define fBWHT "\033[97m"

// Background
#define bBLK  "\033[40m"
#define bRED  "\033[41m"
#define bGRN  "\033[42m"
#define bYLW  "\033[43m"
#define bBLU  "\033[44m"
#define bMAG  "\033[45m"
#define bCYN  "\033[46m"
#define bBBLU "\033[104m"

// Cursor / screen
#define CLS   "\033[2J\033[H"
#define HID   "\033[?25l"   // hide cursor
#define SHW   "\033[?25h"   // show cursor
#define SAVEC "\033[s"
#define LODC  "\033[u"

// Box-drawing — heavy
#define HH_TL "╔" // top-left
#define HH_TR "╗"
#define HH_BL "╚"
#define HH_BR "╝"
#define HH_H  "═"
#define HH_V  "║"
#define HH_ML "╠"
#define HH_MR "╣"
#define HH_TT "╦"
#define HH_BT "╩"
#define HH_CR "╬"

// Box-drawing — light
#define LT_TL "┌"
#define LT_TR "┐"
#define LT_BL "└"
#define LT_BR "┘"
#define LT_H  "─"
#define LT_V  "│"
#define LT_ML "├"
#define LT_MR "┤"
#define LT_TT "┬"
#define LT_BT "┴"
#define LT_CR "┼"

// Double-line thin mix
#define DT_TL "╒"
#define DT_TR "╕"
#define DT_BL "╘"
#define DT_BR "╛"
#define DT_H  "═"
#define DT_ML "╞"
#define DT_MR "╡"

// Symbols
#define SYM_BULLET   "●"
#define SYM_DIAMOND  "◆"
#define SYM_CHECK    "✔"
#define SYM_CROSS    "✖"
#define SYM_ARR_R    "▶"
#define SYM_ARR_L    "◀"
#define SYM_STAR     "★"
#define SYM_CARD     "▪"
#define SYM_LOCK     "◉"
#define SYM_CASH     "◈"
#define SYM_WAVE     "≋"
#define SYM_DOT      "·"
#define SYM_BLOCK    "█"
#define SYM_HALF     "▓"
#define SYM_LIGHT    "░"
#define SYM_SPARKS   "✦"

// ═══════════════════════════════════════════════════════════════════════
//  LAYOUT CONSTANTS
// ═══════════════════════════════════════════════════════════════════════

const int W = 70;          // total terminal width (cols)
const int INNER = W - 4;   // inner content width (inside ║ padding)

// ═══════════════════════════════════════════════════════════════════════
//  BANK DATA
// ═══════════════════════════════════════════════════════════════════════

const int NB = 4;
const string BNAMES[NB]  = { "BDO", "BPI", "Metrobank", "Security Bank" };
const double LFEE[NB]    = { 25.0,  20.0,  30.0,        15.0  };
const double IFEE[NB]    = { 150.0, 125.0, 200.0,       100.0 };
const double DLIMIT[NB]  = { 50000, 75000, 100000,      60000 };
// ANSI accent per bank
const string BCLR[NB]    = { fBBLU, fBRED, fBYLW, fBGRN };

// ATM machine cash
const int ND = 2;
int DENOM[ND]    = { 1000, 500 };
int DCNT[ND]     = { 500, 500 };

// ═══════════════════════════════════════════════════════════════════════
//  ENCODING  (Caesar +3, stored as "XX|YY|ZZ")
// ═══════════════════════════════════════════════════════════════════════

string encode(const string& s) {
    string r;
    for (int i = 0; i < (int)s.size(); i++) {
        r += to_string((int)s[i] + 3);
        if (i < (int)s.size()-1) r += "|";
    }
    return r;
}
string decode(const string& e) {
    string r, tok;
    for (int i = 0; i <= (int)e.size(); i++) {
        if (i == (int)e.size() || e[i]=='|') {
            if (!tok.empty()) { r += (char)(stoi(tok)-3); tok=""; }
        } else tok += e[i];
    }
    return r;
}

// ═══════════════════════════════════════════════════════════════════════
//  ACCOUNT STRUCTURE
// ═══════════════════════════════════════════════════════════════════════

struct TxRecord {
    string type;
    double amount;
    double fee;
    string timestamp;
    int    direction; // +1 credit, -1 debit
};

struct Account {
    string card;
    string encPin;
    string name;
    string bank;
    string accType;   // "Local" or "International"
    double balance;
    vector<TxRecord> history;
};

// Global account pool (parallel vectors via struct)
vector<Account> ACCOUNTS;
string ADMIN_PASS;

// ═══════════════════════════════════════════════════════════════════════
//  UTILITY — sleep, clear, strings
// ═══════════════════════════════════════════════════════════════════════

void sleep_ms(int ms) { MSLEEP(ms); }

string repeat(const string& s, int n) {
    string r; for (int i=0;i<n;i++) r+=s; return r;
}

// Pad a string to fixed visual width (no ANSI inside s)
string padR(const string& s, int w) {
    if ((int)s.size()>=w) return s.substr(0,w);
    return s + string(w-(int)s.size(),' ');
}
string padL(const string& s, int w) {
    if ((int)s.size()>=w) return s.substr(0,w);
    return string(w-(int)s.size(),' ') + s;
}
string padC(const string& s, int w) {
    int p=(w-(int)s.size())/2;
    int rp=w-(int)s.size()-p;
    if (p<0) p=0; if (rp<0) rp=0;
    return string(p,' ')+s+string(rp,' ');
}

// Format PHP currency  e.g. 50000 → "PHP 50,000.00"
string php(double v) {
    ostringstream o; o << fixed << setprecision(2) << v;
    string n = o.str();
    int dot = (int)n.find('.');
    string intP = n.substr(0,dot), decP = n.substr(dot);
    string fmt;
    int cnt=0;
    for (int i=(int)intP.size()-1;i>=0;i--) {
        if (cnt>0 && cnt%3==0) fmt = "," + fmt;
        fmt = intP[i] + fmt; cnt++;
    }
    return "PHP " + fmt + decP;
}

// Mask card number  "1234 •••• •••• 3456"
string maskCard(const string& c) {
    if ((int)c.size()!=16) return c;
    return c.substr(0,4)+" •••• •••• "+c.substr(12,4);
}

bool isDigits(const string& s) {
    for (char c:s) if (c<'0'||c>'9') return false;
    return !s.empty();
}

// Date/Time helpers
string nowDate() {
    time_t t=time(0); tm* tm=localtime(&t);
    ostringstream o;
    o<<setfill('0')<<setw(2)<<(tm->tm_mon+1)<<"/"
     <<setw(2)<<tm->tm_mday<<"/"<<(tm->tm_year+1900);
    return o.str();
}
string nowTime() {
    time_t t=time(0); tm* tm=localtime(&t);
    ostringstream o;
    o<<setfill('0')<<setw(2)<<tm->tm_hour<<":"
     <<setw(2)<<tm->tm_min<<":"<<setw(2)<<tm->tm_sec;
    return o.str();
}
string nowFull() { return nowDate()+"  "+nowTime(); }
bool inMaintWindow() {
    time_t t=time(0); tm* tm=localtime(&t);
    return tm->tm_hour==8 && tm->tm_min<=15;
}

// Bank index lookup
int bankIdx(const string& b) {
    for (int i=0;i<NB;i++) if (BNAMES[i]==b) return i;
    return -1;
}
int findAccount(const string& card) {
    for (int i=0;i<(int)ACCOUNTS.size();i++)
        if (ACCOUNTS[i].card==card) return i;
    return -1;
}
double totalCash() {
    double t=0; for (int i=0;i<ND;i++) t+=DENOM[i]*DCNT[i]; return t;
}

// ═══════════════════════════════════════════════════════════════════════
//  RECURSION — bill breakdown
// ═══════════════════════════════════════════════════════════════════════

void breakBills(double amt, int& b1000, int& b500) {
    if (amt<=0) return;
    if (amt>=1000) { b1000++; breakBills(amt-1000,b1000,b500); }
    else if (amt>=500) { b500++; breakBills(amt-500,b1000,b500); }
}
double calcFeeRecursive(double amt, int iter) {
    if (iter<=0) return 0;
    return (amt*0.001) + calcFeeRecursive(amt, iter-1);
}

// ═══════════════════════════════════════════════════════════════════════
//  fstream LOGGING
// ═══════════════════════════════════════════════════════════════════════

void logTx(const string& card, const string& type, double amt, double fee) {
    ofstream f("transactions.csv", ios::app);
    if (f.is_open()) {
        f<<nowDate()<<","<<nowTime()<<","<<card<<","<<type<<","
         <<fixed<<setprecision(2)<<amt<<","<<fee<<"\n";
        f.close();
    }
}

// ═══════════════════════════════════════════════════════════════════════
//  DRAWING PRIMITIVES
// ═══════════════════════════════════════════════════════════════════════

// Full-width heavy top bar
void boxTop(const string& c=fBCYN) {
    cout<<c<<HH_TL<<repeat(HH_H,W-2)<<HH_TR<<RST<<"\n";
}
void boxBot(const string& c=fBCYN) {
    cout<<c<<HH_BL<<repeat(HH_H,W-2)<<HH_BR<<RST<<"\n";
}
void boxMid(const string& c=fBCYN) {
    cout<<c<<HH_ML<<repeat(HH_H,W-2)<<HH_MR<<RST<<"\n";
}
// Light divider
void lightDiv(const string& c=fBCYN) {
    cout<<"  "<<c<<DIM<<LT_ML<<repeat(LT_H,W-6)<<LT_MR<<RST<<"\n";
}
// Empty heavy row
void emptyRow(const string& c=fBCYN) {
    cout<<c<<HH_V<<RST<<string(W-2,' ')<<c<<HH_V<<RST<<"\n";
}
// Heavy row — plain content string (no ANSI), pad to INNER
void hRow(const string& plain, const string& display,
          const string& bc=fBCYN, const string& tc=fBWHT) {
    int pad = INNER-2 - (int)plain.size();
    if (pad<0) pad=0;
    cout<<bc<<HH_V<<RST<<" "<<tc<<display<<string(pad,' ')<<RST<<" "<<bc<<HH_V<<RST<<"\n";
}
// Heavy row — centered
void hRowC(const string& plain, const string& display,
           const string& bc=fBCYN) {
    int inner = W-4;
    int lp=(inner-(int)plain.size())/2; if(lp<0)lp=0;
    int rp=inner-(int)plain.size()-lp;  if(rp<0)rp=0;
    cout<<bc<<HH_V<<RST<<string(lp+1,' ')<<display<<string(rp+1,' ')<<bc<<HH_V<<RST<<"\n";
}

// Light box row
void lRow(const string& plain, const string& display,
          const string& bc=fBCYN, const string& tc=fBWHT) {
    int pad=W-8-(int)plain.size(); if(pad<0)pad=0;
    cout<<"  "<<bc<<DIM<<LT_V<<RST<<" "<<tc<<display<<string(pad,' ')<<RST<<" "<<bc<<DIM<<LT_V<<RST<<"\n";
}

// Key-value row inside heavy box (label left, value right)
void kvRow(const string& label, const string& val,
           const string& valColor=fBWHT, const string& bc=fBCYN) {
    string left  = "  " + label;
    string right = val + "  ";
    int pad = INNER-2-(int)left.size()-(int)val.size()-2;
    if (pad<0) pad=0;
    cout<<bc<<HH_V<<RST<<" "
        <<DIM<<left<<RST
        <<string(pad,' ')
        <<valColor<<BOLD<<right<<RST
        <<" "<<bc<<HH_V<<RST<<"\n";
}

// ═══════════════════════════════════════════════════════════════════════
//  ANIMATED ELEMENTS
// ═══════════════════════════════════════════════════════════════════════

// Progress bar  ████░░░░░░  pct 0-100
void progressBar(int pct, int barW=30,
                 const string& fillC=fBCYN,
                 const string& emptyC=DIM) {
    int filled = (barW * pct) / 100;
    cout<<"  "<<fillC;
    for (int i=0;i<filled;i++) cout<<SYM_BLOCK;
    cout<<emptyC;
    for (int i=filled;i<barW;i++) cout<<SYM_LIGHT;
    cout<<RST;
}

// Animated loading bar — fills in real-time
void animLoading(const string& label, int ms=700) {
    const int BAR=32;
    cout<<"\n  "<<DIM<<label<<RST<<"\n  "<<fBCYN<<"["<<RST;
    cout.flush();
    for (int i=0;i<BAR;i++) {
        sleep_ms(ms/BAR);
        cout<<fBCYN<<SYM_BLOCK<<RST; cout.flush();
    }
    cout<<fBCYN<<"]"<<RST<<"  "<<fBGRN<<BOLD<<SYM_CHECK<<" Done"<<RST<<"\n";
}

// Typing effect
void typeOut(const string& s, int delayMs=14) {
    for (char c:s) { cout<<c; cout.flush(); sleep_ms(delayMs); }
    cout<<"\n";
}

// Spinner (3 ticks)
void spinnerLine(const string& label) {
    const char* frames[] = {"⠋","⠙","⠹","⠸","⠼","⠴","⠦","⠧","⠇","⠏"};
    for (int i=0;i<20;i++) {
        cout<<"\r  "<<fBCYN<<frames[i%10]<<RST<<"  "<<DIM<<label<<RST<<"   ";
        cout.flush(); sleep_ms(60);
    }
    cout<<"\r  "<<fBGRN<<SYM_CHECK<<RST<<"  "<<label<<"   \n";
}

// ═══════════════════════════════════════════════════════════════════════
//  SCREEN HEADER  (persistent top panel every screen)
// ═══════════════════════════════════════════════════════════════════════

void screenHeader(const string& acctName="", const string& bank="",
                  const string& accType="", double balance=-1) {
    cout<<CLS<<HID;

    // ── Brand bar
    boxTop(fBCYN);
    emptyRow(fBCYN);

    // Logo + name centered
    string logo = "  " + string(SYM_SPARKS) + "  PILIPINAS ATM NETWORK  " + string(SYM_SPARKS) + "  ";
    hRowC(logo, string(BOLD) + fBYLW + logo + RST, fBCYN);

    string tagline = "Secure  " + string(SYM_DIAMOND) + "  Reliable  " + string(SYM_DIAMOND) + "  Always Available";
    hRowC(tagline, string(DIM) + fBCYN + tagline + RST, fBCYN);

    emptyRow(fBCYN);

    // Date/Time row
    string dt = nowDate() + "   " + SYM_DIAMOND + "   " + nowTime();
    string dtDisp = string(DIM) + fBCYN + nowDate() + RST + "   " + fBCYN + SYM_DIAMOND + RST + "   "
                  + BOLD + fBCYN + nowTime() + RST;
    kvRow("", dtDisp + "   ", fBCYN, fBCYN);  // right-ish

    // ── Account info panel (only when logged in)
    if (!acctName.empty()) {
        boxMid(fBCYN);
        emptyRow(fBCYN);

        int bi = bankIdx(bank);
        string bColor = (bi>=0) ? BCLR[bi] : fBWHT;

        // Name + bank
        string nameLabel = "  " + string(SYM_CARD) + "  " + acctName;
        string bankLabel = bank + "  ·  " + accType;
        string nameFull  = string(BOLD) + fBWHT + nameLabel + RST;
        string bankFull  = DIM + bColor + "  " + bankLabel + RST;

        // Row: name left, bank right
        int nameW=(int)nameLabel.size(), bankW=(int)bankLabel.size()+2;
        int gap = INNER-2-nameW-bankW; if(gap<0)gap=0;
        cout<<fBCYN<<HH_V<<RST<<" "
            <<BOLD<<fBWHT<<nameLabel<<RST
            <<string(gap,' ')
            <<DIM<<bColor<<bankLabel<<"  "<<RST
            <<" "<<fBCYN<<HH_V<<RST<<"\n";

        // Balance row
        if (balance >= 0) {
            string balStr  = php(balance);
            string balLabel = "  Available Balance";
            int bg = INNER-2-(int)balLabel.size()-(int)balStr.size()-2;
            if(bg<0)bg=0;
            cout<<fBCYN<<HH_V<<RST<<" "
                <<DIM<<balLabel<<RST
                <<string(bg,' ')
                <<BOLD<<fBGRN<<balStr<<"  "<<RST
                <<" "<<fBCYN<<HH_V<<RST<<"\n";
        }
        emptyRow(fBCYN);
    }

    boxBot(fBCYN);
    cout<<"\n";
}

// ═══════════════════════════════════════════════════════════════════════
//  SECTION TITLE  (light box with title inside)
// ═══════════════════════════════════════════════════════════════════════

void sectionTitle(const string& title, const string& color=fBCYN) {
    cout<<"\n";
    cout<<"  "<<color<<LT_TL<<repeat(LT_H,W-6)<<LT_TR<<RST<<"\n";
    string t = "  " + string(SYM_ARR_R) + "  " + title;
    int pad = W-8-(int)t.size(); if(pad<0)pad=0;
    cout<<"  "<<color<<LT_V<<RST<<" "<<BOLD<<fBWHT<<t<<RST<<string(pad,' ')<<" "<<color<<LT_V<<RST<<"\n";
    cout<<"  "<<color<<LT_BL<<repeat(LT_H,W-6)<<LT_BR<<RST<<"\n";
}

// ═══════════════════════════════════════════════════════════════════════
//  MENU DISPLAY  (overloaded: with/without title)
// ═══════════════════════════════════════════════════════════════════════

void displayMenuC(const vector<string>& items, const string& color=fBCYN) {
    cout<<"  "<<color<<DIM<<LT_TL<<repeat(LT_H,W-6)<<RST<<"\n";
    for (int i=0;i<(int)items.size();i++) {
        string num="["+to_string(i+1)+"]";
        string row = "  " + num + "  " + items[i];
        int pad=W-8-(int)row.size(); if(pad<0)pad=0;
        cout<<"  "<<color<<DIM<<LT_V<<RST<<"  "
            <<BOLD<<color<<num<<RST
            <<"  "<<fBWHT<<items[i]<<string(pad,' ')<<RST
            <<"  "<<color<<DIM<<LT_V<<RST<<"\n";
    }
    cout<<"  "<<color<<DIM<<LT_BL<<repeat(LT_H,W-6)<<LT_BR<<RST<<"\n";
}

void displayMenu(const vector<string>& items, const string& title, const string& color=fBCYN) {
    sectionTitle(title, color);
    displayMenuC(items, color);
}

// ═══════════════════════════════════════════════════════════════════════
//  STATUS MESSAGES
// ═══════════════════════════════════════════════════════════════════════

void msgOK   (const string& m){ cout<<"\n  "<<BOLD<<fBGRN<<" "<<SYM_CHECK<<"  "<<m<<RST<<"\n"; }
void msgERR  (const string& m){ cout<<"\n  "<<BOLD<<fBRED<<" "<<SYM_CROSS<<"  "<<m<<RST<<"\n"; }
void msgINFO (const string& m){ cout<<"\n  "<<BOLD<<fBYLW<<" "<<SYM_DIAMOND<<"  "<<m<<RST<<"\n"; }
void msgWARN (const string& m){ cout<<"\n  "<<BOLD<<fYLW <<" ⚠  "<<m<<RST<<"\n"; }

void pressEnter(const string& label="Press ENTER to continue") {
    cout<<"\n  "<<DIM<<SYM_DOT<<SYM_DOT<<SYM_DOT<<" "
        <<fBCYN<<label<<RST<<" ";
    cin.ignore(numeric_limits<streamsize>::max(),'\n');
    cin.get();
}

// ═══════════════════════════════════════════════════════════════════════
//  INPUT PROMPTS
// ═══════════════════════════════════════════════════════════════════════

string inputStr(const string& label) {
    cout<<"\n  "<<fBCYN<<SYM_ARR_R<<RST<<"  "<<BOLD<<label<<RST<<"\n"
        <<"  "<<fBCYN<<"  └─▶  "<<RST;
    string v; cin>>v; return v;
}
double inputDbl(const string& label) {
    cout<<"\n  "<<fBCYN<<SYM_ARR_R<<RST<<"  "<<BOLD<<label<<RST<<"\n"
        <<"  "<<fBCYN<<"  └─▶  PHP "<<RST;
    double v; cin>>v; return v;
}
int inputInt(const string& label, int lo, int hi) {
    while (true) {
        cout<<"\n  "<<fBCYN<<SYM_ARR_R<<RST<<"  "<<BOLD<<label<<RST<<"\n"
            <<"  "<<fBCYN<<"  └─▶  "<<RST;
        int v; 
        if (cin>>v && v>=lo && v<=hi) return v;
        cin.clear(); cin.ignore(numeric_limits<streamsize>::max(),'\n');
        msgERR("Invalid choice. Enter "+to_string(lo)+"–"+to_string(hi)+".");
    }
}
string inputPIN(const string& label) {
    cout<<"\n  "<<fBCYN<<SYM_LOCK<<RST<<"  "<<BOLD<<label<<RST<<"\n"
        <<"  "<<fBCYN<<"  └─▶  "<<RST;
    string v; cin>>v; return v;
}

// ═══════════════════════════════════════════════════════════════════════
//  RECEIPT PRINTER
// ═══════════════════════════════════════════════════════════════════════

void receiptOpen(const string& title) {
    cout<<"\n";
    cout<<"  "<<fBYLW<<DT_TL<<repeat(DT_H,W-6)<<DT_TR<<RST<<"\n";
    string t="  RECEIPT  ·  "+title+"  ";
    int p=(W-6-(int)t.size())/2; if(p<0)p=0;
    int r=W-6-(int)t.size()-p;   if(r<0)r=0;
    cout<<"  "<<fBYLW<<HH_V<<RST<<string(p,' ')
        <<BOLD<<fBYLW<<t<<RST<<string(r,' ')
        <<"  "<<fBYLW<<HH_V<<RST<<"\n";
    cout<<"  "<<fBYLW<<DT_ML<<repeat(DT_H,W-6)<<DT_MR<<RST<<"\n";
}
void receiptRow(const string& label, const string& val,
                const string& valC=fBWHT) {
    int lw=22;
    string l = "  "+padR(label,lw);
    int pad=W-8-(int)l.size()-(int)val.size()-2; if(pad<0)pad=0;
    cout<<"  "<<fBYLW<<HH_V<<RST<<" "<<DIM<<l<<RST
        <<string(pad,' ')<<valC<<BOLD<<val<<"  "<<RST
        <<"  "<<fBYLW<<HH_V<<RST<<"\n";
}
void receiptDiv() {
    // dashed divider inside receipt
    string dsh;
    for (int i=0;i<W-8;i++) dsh+=(i%2?"─":" ");
    cout<<"  "<<fBYLW<<HH_V<<RST<<" "<<DIM<<dsh<<RST<<" "<<fBYLW<<HH_V<<RST<<"\n";
}
void receiptClose() {
    cout<<"  "<<fBYLW<<DT_BL<<repeat(DT_H,W-6)<<DT_BR<<RST<<"\n";
}

// ═══════════════════════════════════════════════════════════════════════
//  TRANSACTION HISTORY DISPLAY
// ═══════════════════════════════════════════════════════════════════════

void showHistory(const vector<TxRecord>& txs) {
    sectionTitle("TRANSACTION HISTORY  ·  Last 10");

    if (txs.empty()) {
        msgINFO("No transactions on record.");
        pressEnter(); return;
    }

    // Header
    cout<<"\n  "<<BOLD<<fBCYN
        <<padR("  #",5)
        <<padR("TYPE",18)
        <<padR("AMOUNT",18)
        <<padR("FEE",12)
        <<"DATE / TIME"<<RST<<"\n";
    cout<<"  "<<DIM<<repeat(LT_H,W-4)<<RST<<"\n";

    int start=(int)txs.size()>10?(int)txs.size()-10:0;
    for (int i=start;i<(int)txs.size();i++) {
        const TxRecord& t=txs[i];
        bool debit=(t.direction<0);
        string aColor = debit ? fBRED : fBGRN;
        string sign   = debit ? "-" : "+";

        string numS   = "  "+to_string(i-start+1);
        string amtS   = sign+php(t.amount);

        cout<<"  "<<DIM<<padR(numS,5)<<RST
            <<fBWHT<<padR(t.type,18)<<RST
            <<aColor<<BOLD<<padR(amtS,18)<<RST
            <<DIM<<padR(php(t.fee),12)<<RST
            <<DIM<<t.timestamp<<RST<<"\n";
    }
    cout<<"  "<<DIM<<repeat(LT_H,W-4)<<RST<<"\n";
    pressEnter();
}

// ═══════════════════════════════════════════════════════════════════════
//  WITHDRAW  (overload v1 — double amount)
// ═══════════════════════════════════════════════════════════════════════

bool doWithdraw(vector<Account>& accts, int idx, double amount) {
    Account& a=accts[idx];
    int bi=bankIdx(a.bank);
    double fee=(a.accType=="Local")?LFEE[bi]:IFEE[bi];
    double total=amount+fee;

    if (amount<=0 || fmod(amount,500.0)!=0.0) {
        msgERR("Amount must be a positive multiple of PHP 500."); return false;
    }
    if (amount>DLIMIT[bi])  { msgERR("Exceeds daily limit of "+php(DLIMIT[bi])); return false; }
    if (a.balance<total)    { msgERR("Insufficient balance. Need "+php(total)); return false; }
    if (totalCash()<amount) { msgERR("ATM has insufficient cash. Try a lower amount."); return false; }

    int b1000=0,b500=0;
    breakBills(amount,b1000,b500);
    if (b1000>DCNT[0]||b500>DCNT[1]) { msgERR("Cannot dispense exact bills. Try another amount."); return false; }

    // ── Summary
    cout<<"\n  "<<DIM<<"Transaction Preview"<<RST<<"\n";
    cout<<"  "<<DIM<<repeat(SYM_DOT,W-4)<<RST<<"\n";
    cout<<"  "<<fBYLW<<"  Withdraw   : "<<RST<<BOLD<<php(amount)<<RST<<"\n";
    cout<<"  "<<fBYLW<<"  Fee        : "<<RST<<php(fee)<<"\n";
    cout<<"  "<<fBYLW<<"  Total      : "<<RST<<BOLD<<fBRED<<php(total)<<RST<<"\n";
    cout<<"  "<<DIM<<repeat(SYM_DOT,W-4)<<RST<<"\n";

    string conf=inputStr("Confirm withdrawal? (y/n)");
    if (conf!="y"&&conf!="Y") { msgINFO("Withdrawal cancelled."); return false; }

    animLoading("Dispensing cash...",800);

    DCNT[0]-=b1000; DCNT[1]-=b500;
    a.balance-=total;
    TxRecord tx{"Withdrawal",amount,fee,nowFull(),-1};
    a.history.push_back(tx);
    logTx(a.card,"Withdrawal",amount,fee);

    // ── Receipt
    receiptOpen("CASH WITHDRAWAL");
    receiptRow("Date / Time:", nowFull());
    receiptRow("Cardholder:", a.name);
    receiptRow("Card:", maskCard(a.card));
    receiptRow("Bank:", a.bank+" ("+a.accType+")");
    receiptDiv();
    receiptRow("Amount Withdrawn:", php(amount), fBRED);
    receiptRow("Service Fee:", php(fee));
    receiptDiv();
    receiptRow("New Balance:", php(a.balance), fBGRN);
    receiptRow("Bills Dispensed:", to_string(b1000)+" × PHP1000   "+to_string(b500)+" × PHP500");
    receiptClose();
    return true;
}

// ═══════════════════════════════════════════════════════════════════════
//  WITHDRAW  (overload v2 — preset string)
// ═══════════════════════════════════════════════════════════════════════

bool doWithdraw(vector<Account>& accts, int idx, const string& preset) {
    return doWithdraw(accts, idx, stod(preset));
}

// ═══════════════════════════════════════════════════════════════════════
//  TRANSFER
// ═══════════════════════════════════════════════════════════════════════

void doTransfer(vector<Account>& accts, int idx) {
    string recipCard=inputStr("Recipient Card Number (16 digits)");
    int rIdx=findAccount(recipCard);
    if (rIdx==-1) { msgERR("Recipient not found."); pressEnter(); return; }
    if (rIdx==idx) { msgERR("Cannot transfer to yourself."); pressEnter(); return; }

    double amount=inputDbl("Amount to Transfer (multiples of PHP 500)");
    if (amount<=0) { msgERR("Invalid amount."); pressEnter(); return; }

    Account& a=accts[idx];
    Account& r=accts[rIdx];
    int bi=bankIdx(a.bank);
    bool same=(a.bank==r.bank);
    double fee=same?LFEE[bi]:IFEE[bi];
    string txType=same?"Local Transfer":"International Transfer";
    double total=amount+fee;

    if (amount>DLIMIT[bi]) { msgERR("Exceeds daily limit."); pressEnter(); return; }
    if (a.balance<total)   { msgERR("Insufficient balance. Need "+php(total)); pressEnter(); return; }

    // ── Summary
    cout<<"\n  "<<DIM<<"Transfer Preview"<<RST<<"\n";
    cout<<"  "<<DIM<<repeat(SYM_DOT,W-4)<<RST<<"\n";
    cout<<"  "<<fBYLW<<"  To         : "<<RST<<BOLD<<r.name<<RST<<"  "<<DIM<<"("+r.bank+")"<<RST<<"\n";
    cout<<"  "<<fBYLW<<"  Card       : "<<RST<<maskCard(recipCard)<<"\n";
    cout<<"  "<<fBYLW<<"  Amount     : "<<RST<<BOLD<<php(amount)<<RST<<"\n";
    cout<<"  "<<fBYLW<<"  Fee        : "<<RST<<php(fee)<<"  "<<DIM<<"["<<(same?"Local":"International")<<"]"<<RST<<"\n";
    cout<<"  "<<fBYLW<<"  Total      : "<<RST<<BOLD<<fBRED<<php(total)<<RST<<"\n";
    cout<<"  "<<DIM<<repeat(SYM_DOT,W-4)<<RST<<"\n";

    string conf=inputStr("Confirm transfer? (y/n)");
    if (conf!="y"&&conf!="Y") { msgINFO("Transfer cancelled."); pressEnter(); return; }

    animLoading("Processing transfer...",900);

    a.balance-=total;
    r.balance+=amount;

    TxRecord txOut{txType,amount,fee,nowFull(),-1};
    TxRecord txIn{"Transfer In",amount,0.0,nowFull(),+1};
    a.history.push_back(txOut);
    r.history.push_back(txIn);
    logTx(a.card,txType,amount,fee);

    receiptOpen("FUND TRANSFER");
    receiptRow("Date / Time:", nowFull());
    receiptRow("From:", a.name+" · "+maskCard(a.card));
    receiptRow("To:", r.name+" · "+maskCard(recipCard));
    receiptRow("Transfer Type:", txType, same?fBGRN:fBYLW);
    receiptDiv();
    receiptRow("Amount Sent:", php(amount), fBRED);
    receiptRow("Service Fee:", php(fee));
    receiptDiv();
    receiptRow("New Balance:", php(a.balance), fBGRN);
    receiptClose();
    pressEnter();
}

// ═══════════════════════════════════════════════════════════════════════
//  CLIENT MENU
// ═══════════════════════════════════════════════════════════════════════

void clientMenu(vector<Account>& accts,
                // per-spec: all vectors passed by ref (flat parallel for C-string compliance)
                vector<string>& cardNums, vector<string>& encPins,
                vector<double>& balances, vector<string>& banks,
                vector<string>& types) {

    // ── Card entry ──────────────────────────────────────────────
    screenHeader();
    sectionTitle("INSERT CARD");

    cout<<"\n  "<<DIM<<"Quick test cards:"<<RST<<"\n";
    for (int i=0;i<(int)accts.size();i++) {
        int bi=bankIdx(accts[i].bank);
        string bc=(bi>=0)?BCLR[bi]:fBWHT;
        cout<<"  "<<bc<<SYM_BULLET<<RST<<"  "
            <<DIM<<maskCard(accts[i].card)<<RST
            <<"   "<<bc<<padR(accts[i].bank,14)<<RST
            <<"  "<<fBWHT<<accts[i].name<<RST<<"\n";
    }

    string card=inputStr("Card Number (16 digits)");
    if (card.size()!=16||!isDigits(card)) {
        msgERR("Invalid card number format."); pressEnter(); return;
    }
    int accIdx=findAccount(card);
    if (accIdx==-1) { msgERR("Card not recognized. Please contact your bank."); pressEnter(); return; }

    // ── PIN entry ──────────────────────────────────────────────
    screenHeader();
    sectionTitle("ENTER PIN");

    int bi=bankIdx(accts[accIdx].bank);
    string bColor=(bi>=0)?BCLR[bi]:fBWHT;
    cout<<"\n";
    cout<<"  "<<DIM<<"Card       : "<<RST<<BOLD<<maskCard(card)<<RST<<"\n";
    cout<<"  "<<DIM<<"Bank       : "<<RST<<bColor<<BOLD<<accts[accIdx].bank<<RST<<"\n";
    cout<<"  "<<DIM<<"Cardholder : "<<RST<<accts[accIdx].name<<"\n";

    // Attempts dots display
    auto showAttemptDots=[](int used){
        cout<<"\n  "<<DIM<<"Attempts: "<<RST;
        for (int i=0;i<3;i++) {
            if (i<used) cout<<fBRED<<SYM_BULLET<<RST<<" ";
            else        cout<<fBGRN<<SYM_BULLET<<RST<<" ";
        }
        cout<<"\n";
    };

    bool authed=false;
    for (int attempt=1;attempt<=3;attempt++) {
        showAttemptDots(attempt-1);
        string pin=inputPIN("PIN  (Attempt "+to_string(attempt)+"/3)");
        if (encode(pin)==accts[accIdx].encPin) { authed=true; break; }
        if (attempt<3) msgERR("Incorrect PIN.  "+to_string(3-attempt)+" attempt(s) remaining.");
    }
    if (!authed) {
        msgERR("Card blocked after 3 failed PIN attempts. Contact your bank.");
        pressEnter(); return;
    }

    spinnerLine("Verifying credentials...");
    msgOK("Authentication successful!");
    sleep_ms(400);

    // ── Session loop ──────────────────────────────────────────
    bool active=true;
    while (active) {
        Account& acc=accts[accIdx];
        int bankI=bankIdx(acc.bank);
        string bc=(bankI>=0)?BCLR[bankI]:fBWHT;

        screenHeader(acc.name, acc.bank, acc.accType, acc.balance);

        // Quick-access amount strip
        cout<<"  "<<DIM<<"Quick Withdraw:"<<RST;
        string presets[]={"500","1000","2000","5000","10000","20000"};
        for (int i=0;i<6;i++) {
            cout<<"  "<<fBCYN<<"["<<(i+1)<<"]"<<RST<<" PHP"<<presets[i];
        }
        cout<<"\n\n";

        vector<string> menu={
            "Check Balance",
            "Withdraw Cash",
            "Transfer Funds",
            "Transaction History",
            "Change PIN",
            "Logout"
        };
        displayMenu(menu,"WHAT WOULD YOU LIKE TO DO?",fBCYN);
        int ch=inputInt("Select",1,6);

        switch(ch) {
        // ── Balance ──────────────────────────────────────────
        case 1: {
            screenHeader(acc.name,acc.bank,acc.accType,acc.balance);
            sectionTitle("BALANCE INQUIRY");
            cout<<"\n  "<<DIM<<"As of  "<<RST<<fBCYN<<nowFull()<<RST<<"\n\n";

            // Big balance display
            string bigBal=php(acc.balance);
            cout<<"  "<<DIM<<"Available Balance"<<RST<<"\n";
            cout<<"\n  "<<BOLD<<fBGRN;
            typeOut("  "+bigBal,20);
            cout<<RST;

            cout<<"  "<<DIM<<"Bank       : "<<RST<<bc<<acc.bank<<RST
                <<"   ("<<acc.accType<<")\n";
            cout<<"  "<<DIM<<"Card       : "<<RST<<maskCard(acc.card)<<"\n";
            cout<<"  "<<DIM<<"Daily Limit: "<<RST<<php(DLIMIT[bankI])<<"\n";
            pressEnter();
            break;
        }
        // ── Withdraw ──────────────────────────────────────────
        case 2: {
            screenHeader(acc.name,acc.bank,acc.accType,acc.balance);
            sectionTitle("WITHDRAW CASH");

            double fee=(acc.accType=="Local")?LFEE[bankI]:IFEE[bankI];
            cout<<"  "<<DIM<<"Service Fee : "<<RST<<BOLD<<fBYLW<<php(fee)<<RST
                <<"   "<<DIM<<"Daily Limit : "<<RST<<BOLD<<php(DLIMIT[bankI])<<RST<<"\n\n";

            // Preset grid  2×3
            string PS[]={"500","1000","2000","5000","10000","20000"};
            cout<<"  "<<fBCYN<<DIM<<LT_TL<<repeat(LT_H,W-6)<<RST<<"\n";
            for (int r=0;r<2;r++) {
                cout<<"  "<<fBCYN<<DIM<<LT_V<<RST<<"  ";
                for (int c=0;c<3;c++) {
                    int idx2=r*3+c;
                    string num="["+to_string(idx2+1)+"]";
                    string amt="PHP "+PS[idx2];
                    string cell=num+"  "+padR(amt,12);
                    cout<<BOLD<<fBCYN<<num<<RST<<"  "<<fBWHT<<padR(amt,14)<<RST;
                }
                cout<<"  "<<fBCYN<<DIM<<LT_V<<RST<<"\n";
            }
            cout<<"  "<<fBCYN<<DIM<<LT_ML<<repeat(LT_H,W-6)<<RST<<"\n";
            cout<<"  "<<fBCYN<<DIM<<LT_V<<RST<<"  "
                <<BOLD<<fBCYN<<"[7]"<<RST<<"  "<<fBWHT<<"Custom Amount"<<RST
                <<string(W-8-16,' ')
                <<"  "<<fBCYN<<DIM<<LT_V<<RST<<"\n";
            cout<<"  "<<fBCYN<<DIM<<LT_BL<<repeat(LT_H,W-6)<<LT_BR<<RST<<"\n";

            int wc=inputInt("Select amount",1,7);
            if (wc<=6) {
                doWithdraw(accts,accIdx,PS[wc-1]);
            } else {
                double custom=inputDbl("Custom amount (multiple of PHP 500)");
                doWithdraw(accts,accIdx,custom);
            }
            pressEnter();
            break;
        }
        // ── Transfer ──────────────────────────────────────────
        case 3: {
            screenHeader(acc.name,acc.bank,acc.accType,acc.balance);
            sectionTitle("FUND TRANSFER");
            doTransfer(accts,accIdx);
            break;
        }
        // ── History ──────────────────────────────────────────
        case 4: {
            screenHeader(acc.name,acc.bank,acc.accType,acc.balance);
            showHistory(acc.history);
            break;
        }
        // ── Change PIN ──────────────────────────────────────
        case 5: {
            screenHeader(acc.name,acc.bank,acc.accType,acc.balance);
            sectionTitle("CHANGE PIN");
            string oldPin=inputPIN("Current PIN");
            if (encode(oldPin)!=acc.encPin) {
                msgERR("Incorrect current PIN."); pressEnter(); break;
            }
            string newPin=inputPIN("New PIN");
            string confPin=inputPIN("Confirm New PIN");
            if (newPin!=confPin) {
                msgERR("PINs do not match."); pressEnter();
            } else if (encode(newPin)==acc.encPin) {
                msgERR("New PIN must differ from current."); pressEnter();
            } else {
                acc.encPin=encode(newPin);
                encPins[accIdx]=acc.encPin; // keep parallel vector in sync
                spinnerLine("Updating PIN...");
                msgOK("PIN changed successfully."); pressEnter();
            }
            break;
        }
        // ── Logout ──────────────────────────────────────────
        case 6: {
            screenHeader();
            cout<<"\n\n";
            string bye="  "+string(SYM_SPARKS)+" Thank you for using PILIPINAS ATM NETWORK "+string(SYM_SPARKS);
            cout<<"  "<<fBYLW<<LT_TL<<repeat(LT_H,W-6)<<LT_TR<<RST<<"\n";
            int p=(W-6-(int)bye.size())/2;if(p<0)p=0;
            int rp=W-6-(int)bye.size()-p;if(rp<0)rp=0;
            cout<<"  "<<fBYLW<<HH_V<<RST<<string(p,' ')<<BOLD<<fBYLW<<bye<<RST<<string(rp,' ')<<"  "<<fBYLW<<HH_V<<RST<<"\n";
            string bye2="  Please take your card and receipt. Have a great day!  ";
            int p2=(W-6-(int)bye2.size())/2;if(p2<0)p2=0;
            int r2=W-6-(int)bye2.size()-p2;if(r2<0)r2=0;
            cout<<"  "<<fBYLW<<HH_V<<RST<<string(p2,' ')<<DIM<<bye2<<RST<<string(r2,' ')<<"  "<<fBYLW<<HH_V<<RST<<"\n";
            cout<<"  "<<fBYLW<<LT_BL<<repeat(LT_H,W-6)<<LT_BR<<RST<<"\n";
            cout<<SHW;
            sleep_ms(1400);
            active=false;
            break;
        }
        }
    }
}

// ═══════════════════════════════════════════════════════════════════════
//  ADMIN MENU
// ═══════════════════════════════════════════════════════════════════════

void adminMenu(vector<Account>& accts,
               vector<string>& cardNums, vector<string>& encPins,
               vector<double>& balances, vector<string>& banks,
               vector<string>& types,
               string& adminPass) {

    // ── Auth
    screenHeader();
    sectionTitle("ADMIN AUTHENTICATION", fBRED);
    msgWARN("RESTRICTED ACCESS  ·  Authorized Personnel Only");

    bool authed=false;
    for (int attempt=1;attempt<=3;attempt++) {
        string pass=inputPIN("Admin Passcode  (Attempt "+to_string(attempt)+"/3)");
        if (encode(pass)==adminPass) { authed=true; break; }
        if (attempt<3) msgERR("Wrong passcode. "+to_string(3-attempt)+" attempt(s) remaining.");
    }
    if (!authed) {
        msgERR("System locked after 3 failed attempts."); pressEnter(); return;
    }

    animLoading("Loading admin panel...",600);
    msgOK("Admin access granted.");
    sleep_ms(300);

    bool running=true;
    while (running) {
        screenHeader();
        cout<<"  "<<DIM<<"Logged in as  "<<RST<<BOLD<<fBMAG<<"ADMINISTRATOR"<<RST
            <<"   "<<DIM<<nowFull()<<RST<<"\n\n";

        vector<string> menu={
            "ATM Cash Status",
            "Refill ATM Cash  (8:00–8:15 AM window)",
            "View All Accounts",
            "Create New Account",
            "Delete Account",
            "Reset Account PIN",
            "Change Admin Passcode",
            "Exit Admin Panel"
        };
        displayMenu(menu,"ADMIN OPERATIONS", fBMAG);
        int ch=inputInt("Select",1,8);

        switch(ch) {
        case 1: {
            screenHeader();
            sectionTitle("ATM CASH STATUS", fBMAG);
            cout<<"\n";
            double tot=0;
            cout<<"  "<<fBMAG<<DIM<<LT_TL<<repeat(LT_H,W-6)<<LT_TR<<RST<<"\n";
            for (int i=0;i<ND;i++) {
                double sub=(double)DENOM[i]*DCNT[i]; tot+=sub;
                string dLabel="  PHP "+to_string(DENOM[i])+" bills";
                string countS =to_string(DCNT[i])+" bills";
                string subS   =php(sub);
                string dFull  =padR(dLabel,22);
                string cFull  =padR(countS,16);
                cout<<"  "<<fBMAG<<DIM<<LT_V<<RST
                    <<"  "<<BOLD<<fBWHT<<padR("PHP "+to_string(DENOM[i]),10)<<RST
                    <<"  "<<DIM<<padR(to_string(DCNT[i])+" bills",18)<<RST
                    <<fBGRN<<BOLD<<subS<<RST
                    <<string(W-8-10-18-(int)subS.size(),' ')
                    <<"  "<<fBMAG<<DIM<<LT_V<<RST<<"\n";
            }
            cout<<"  "<<fBMAG<<DIM<<LT_ML<<repeat(LT_H,W-6)<<LT_MR<<RST<<"\n";
            string totS="PHP "+to_string((int)tot); // rough display
            cout<<"  "<<fBMAG<<DIM<<LT_V<<RST
                <<"  "<<BOLD<<padR("TOTAL CASH",28)<<RST
                <<fBYLW<<BOLD<<php(tot)<<RST
                <<string(W-8-28-(int)php(tot).size(),' ')
                <<"  "<<fBMAG<<DIM<<LT_V<<RST<<"\n";
            cout<<"  "<<fBMAG<<DIM<<LT_BL<<repeat(LT_H,W-6)<<LT_BR<<RST<<"\n";
            pressEnter();
            break;
        }
        case 2: {
            screenHeader();
            sectionTitle("CASH REFILL", fBMAG);
            if (!inMaintWindow()) {
                msgERR("Refill only allowed during 8:00–8:15 AM maintenance window.");
                msgINFO("Current time: "+nowTime());
            } else {
                msgOK("Maintenance window active. Proceed with refill.");
                for (int i=0;i<ND;i++) {
                    cout<<"\n  "<<DIM<<"Current PHP "<<DENOM[i]<<" bills: "<<RST<<BOLD<<DCNT[i]<<RST<<"\n";
                    int add=inputInt("Add how many PHP "+to_string(DENOM[i])+" bills? (0–10000)",0,10000);
                    DCNT[i]+=add;
                    if(add>0) msgOK("Added "+to_string(add)+" × PHP"+to_string(DENOM[i]));
                }
                msgOK("Refill complete. Total: "+php(totalCash()));
            }
            pressEnter();
            break;
        }
        case 3: {
            screenHeader();
            sectionTitle("ALL ACCOUNTS", fBMAG);
            if (accts.empty()) { msgINFO("No accounts."); pressEnter(); break; }

            cout<<"\n  "<<BOLD<<fBMAG
                <<padR("  #",5)<<padR("CARD (masked)",22)
                <<padR("NAME",22)<<padR("BANK",16)
                <<"BALANCE"<<RST<<"\n";
            cout<<"  "<<DIM<<repeat(LT_H,W-4)<<RST<<"\n";

            for (int i=0;i<(int)accts.size();i++) {
                int bi2=bankIdx(accts[i].bank);
                string bc=(bi2>=0)?BCLR[bi2]:fBWHT;
                cout<<"  "<<DIM<<padR("  "+to_string(i+1),5)<<RST
                    <<padR(maskCard(accts[i].card),22)
                    <<padR(accts[i].name,22)
                    <<bc<<BOLD<<padR(accts[i].bank,16)<<RST
                    <<fBGRN<<BOLD<<php(accts[i].balance)<<RST<<"\n";
            }
            cout<<"  "<<DIM<<repeat(LT_H,W-4)<<RST<<"\n";
            pressEnter();
            break;
        }
        case 4: {
            screenHeader();
            sectionTitle("CREATE NEW ACCOUNT", fBMAG);
            string nc=inputStr("Card Number (16 digits)");
            if (nc.size()!=16||!isDigits(nc)) { msgERR("Invalid card."); pressEnter(); break; }
            if (findAccount(nc)!=-1)           { msgERR("Card already exists."); pressEnter(); break; }

            string nn, np, nbank, ntype;
            double nbal;
            cout<<"\n  "<<DIM<<"Enter cardholder name: "<<RST; cin.ignore(); getline(cin,nn);
            np=inputPIN("Initial PIN");

            cout<<"\n  Bank:\n";
            for (int i=0;i<NB;i++)
                cout<<"  "<<BCLR[i]<<SYM_BULLET<<RST<<"  "<<fBCYN<<"["<<(i+1)<<"]"<<RST<<"  "<<BCLR[i]<<BOLD<<BNAMES[i]<<RST<<"\n";
            int bc2=inputInt("Bank",1,NB);
            nbank=BNAMES[bc2-1];

            cout<<"\n  "<<fBCYN<<"[1]"<<RST<<" Local    "<<fBCYN<<"[2]"<<RST<<" International\n";
            int at=inputInt("Account Type",1,2);
            ntype=(at==2)?"International":"Local";

            nbal=inputDbl("Initial Balance");

            Account na{nc,encode(np),nn,nbank,ntype,nbal,{}};
            accts.push_back(na);
            cardNums.push_back(nc);
            encPins.push_back(encode(np));
            balances.push_back(nbal);
            banks.push_back(nbank);
            types.push_back(ntype);

            spinnerLine("Creating account...");
            msgOK("Account created: "+maskCard(nc)+"  ("+nbank+")");
            pressEnter();
            break;
        }
        case 5: {
            screenHeader();
            sectionTitle("DELETE ACCOUNT", fBMAG);
            string dc=inputStr("Card Number to Delete");
            int di=findAccount(dc);
            if (di==-1) { msgERR("Account not found."); pressEnter(); break; }

            Account& da=accts[di];
            cout<<"\n  "<<DIM<<"Account   : "<<RST<<maskCard(dc)<<"  ("<<da.bank<<")\n";
            cout<<"  "<<DIM<<"Name      : "<<RST<<da.name<<"\n";
            cout<<"  "<<DIM<<"Balance   : "<<RST<<fBRED<<BOLD<<php(da.balance)<<RST<<"\n";
            msgWARN("This action is IRREVERSIBLE.");

            string conf=inputStr("Type  YES  to confirm");
            if (conf=="YES") {
                accts.erase(accts.begin()+di);
                cardNums.erase(cardNums.begin()+di);
                encPins.erase(encPins.begin()+di);
                balances.erase(balances.begin()+di);
                banks.erase(banks.begin()+di);
                types.erase(types.begin()+di);
                msgOK("Account permanently deleted.");
            } else {
                msgINFO("Deletion cancelled.");
            }
            pressEnter();
            break;
        }
        case 6: {
            screenHeader();
            sectionTitle("RESET ACCOUNT PIN", fBMAG);
            string rc=inputStr("Card Number");
            int ri=findAccount(rc);
            if (ri==-1) { msgERR("Account not found."); pressEnter(); break; }
            string np=inputPIN("New PIN for "+maskCard(rc));
            accts[ri].encPin=encode(np);
            encPins[ri]=encode(np);
            msgOK("PIN reset for "+accts[ri].name+" ("+accts[ri].bank+")");
            pressEnter();
            break;
        }
        case 7: {
            screenHeader();
            sectionTitle("CHANGE ADMIN PASSCODE", fBMAG);
            string cur=inputPIN("Current Passcode");
            if (encode(cur)!=adminPass) { msgERR("Incorrect passcode."); pressEnter(); break; }
            string nw=inputPIN("New Passcode");
            string cf=inputPIN("Confirm New Passcode");
            if (nw!=cf) { msgERR("Passcodes do not match."); pressEnter(); }
            else { adminPass=encode(nw); msgOK("Admin passcode updated."); pressEnter(); }
            break;
        }
        case 8:
            msgINFO("Exiting Admin Panel...");
            sleep_ms(400);
            running=false;
            break;
        }
    }
}

// ═══════════════════════════════════════════════════════════════════════
//  LOGIN SPLASH
// ═══════════════════════════════════════════════════════════════════════

int loginSplash() {
    cout<<CLS<<HID;

    // ── Giant welcome splash ────────────────────────────────────
    cout<<"\n\n";

    // Decorative top accent
    cout<<"  "<<fBCYN<<DIM;
    for (int i=0;i<W-4;i++) cout<<(i%3==0?SYM_SPARKS:SYM_DOT);
    cout<<RST<<"\n\n";

    // ASCII-art style title
    string t1="██████╗   ██████╗  ██████╗   ██████╗  ███████╗";
    string t2="██╔══██╗ ██╔═══██╗ ██╔══██╗ ██╔════╝  ██╔════╝";
    string t3="██████╔╝ ██║   ██║ ██║  ██║ ██║  ███╗ █████╗  ";
    string t4="██╔══██╗ ██║   ██║ ██║  ██║ ██║   ██║ ██╔══╝  ";
    string t5="██║  ██║ ╚██████╔╝ ██████╔╝ ╚██████╔╝ ███████╗";
    string t6="╚═╝  ╚═╝  ╚═════╝  ╚═════╝   ╚═════╝  ╚══════╝";
    cout<<BOLD<<fBBLU<<t1<<RST<<"\n";
    cout<<BOLD<<fBBLU<<t2<<RST<<"\n";
    cout<<BOLD<<fBCYN<<t3<<RST<<"\n";
    cout<<BOLD<<fBCYN<<t4<<RST<<"\n";
    cout<<BOLD<<fBWHT<<t5<<RST<<"\n";
    cout<<BOLD<<fBWHT<<t6<<RST<<"\n\n";

    // Sub-brand
    string sub="A T M   N E T W O R K   ·   P R E M I U M   B A N K I N G";
    int sp=(W-4-(int)sub.size())/2; if(sp<0)sp=0;
    cout<<"  "<<string(sp,' ')<<DIM<<fBCYN<<sub<<RST<<"\n\n";

    // Bank strip
    string strip;
    for (int i=0;i<NB;i++) {
        strip+="  ";
        strip+=string(BCLR[i])+string(SYM_BULLET)+string(RST)+" "+BNAMES[i];
    }
    cout<<"  ";
    for (int i=0;i<NB;i++)
        cout<<BCLR[i]<<BOLD<<SYM_BULLET<<RST<<"  "<<BCLR[i]<<padR(BNAMES[i],16)<<RST;
    cout<<"\n\n";

    // Date/time
    cout<<"  "<<DIM<<fBCYN<<nowDate()<<"   "<<SYM_DIAMOND<<"   "<<nowTime()<<RST<<"\n\n";

    // Decorative bottom accent
    cout<<"  "<<fBCYN<<DIM;
    for (int i=0;i<W-4;i++) cout<<(i%3==0?SYM_SPARKS:SYM_DOT);
    cout<<RST<<"\n\n";

    // Options
    cout<<"  "<<fBCYN<<BOLD<<"["<<"1"<<"]"<<RST<<"  "<<BOLD<<fBWHT<<"Client Login"<<RST<<"\n";
    cout<<"  "<<fBCYN<<BOLD<<"["<<"2"<<"]"<<RST<<"  "<<BOLD<<fBWHT<<"Admin Login"<<RST<<"\n";
    cout<<"  "<<fBCYN<<BOLD<<"["<<"0"<<"]"<<RST<<"  "<<DIM<<"Exit"<<RST<<"\n\n";

    return inputInt("Select",0,2);
}

// ═══════════════════════════════════════════════════════════════════════
//  SEED ACCOUNTS
// ═══════════════════════════════════════════════════════════════════════

void seedAccounts(vector<Account>& accts,
                  vector<string>& cardNums, vector<string>& encPins,
                  vector<double>& bals, vector<string>& bnks, vector<string>& types) {

    auto add=[&](const string& card, const string& pin, const string& name,
                 const string& bank, const string& type, double bal,
                 vector<TxRecord> seed={}) {
        Account a{card,encode(pin),name,bank,type,bal,seed};
        accts.push_back(a);
        cardNums.push_back(card); encPins.push_back(encode(pin));
        bals.push_back(bal);      bnks.push_back(bank); types.push_back(type);
    };

    add("1234567890123456","1234","Juan dela Cruz",    "BDO",          "Local",        50000.0,{
        {"Withdrawal",2000,25,  "2025-03-12 08:14",-1},
        {"Transfer Out",5000,25,"2025-03-12 09:00",-1},
        {"Transfer In",1500,0,  "2025-03-11 15:30",+1},
    });
    add("9876543210987654","5678","Maria Santos",      "BPI",          "Local",        75000.0,{
        {"Withdrawal",3000,20,  "2025-03-12 11:20",-1},
        {"Transfer In",10000,0, "2025-03-10 14:05",+1},
    });
    add("1111222233334444","9999","Pedro Reyes",       "Metrobank",    "International",100000.0);
    add("5555666677778888","0000","Ana Garcia",        "Security Bank","Local",        60000.0,{
        {"Withdrawal",5000,15,  "2025-03-11 16:40",-1},
    });
    add("4444333322221111","1111","Jose Rizal Jr.",    "BDO",          "Local",        35000.0);
    add("8888777766665555","2222","Lola Nena Flores",  "BPI",          "International",120000.0,{
        {"Transfer Out",20000,125,"2025-03-10 09:30",-1},
        {"Withdrawal",10000,125, "2025-03-09 14:15",-1},
        {"Transfer In",50000,0,  "2025-03-08 10:00",+1},
    });
    add("1234123412341234","3333","Miguel Torres",     "Metrobank",    "Local",        47500.0);
    add("9999888877776666","4444","Clara Bautista",    "Security Bank","Local",        92000.0,{
        {"Transfer In",30000,0, "2025-03-11 08:00",+1},
        {"Withdrawal",8000,15,  "2025-03-11 12:30",-1},
    });
}

// ═══════════════════════════════════════════════════════════════════════
//  MAIN
// ═══════════════════════════════════════════════════════════════════════

int main() {
    // CSV init
    { ifstream ck("transactions.csv");
      if (!ck.good()) {
          ofstream o("transactions.csv");
          o<<"Date,Time,Card,Type,Amount,Fee\n";
      }
    }

    // Admin passcode (default "6767")
    ADMIN_PASS = encode("6767");

    // Parallel vectors (for spec compliance — functions also receive struct vector)
    vector<string> cardNums, encPins, banks, types;
    vector<double> balances;

    // Seed data
    seedAccounts(ACCOUNTS, cardNums, encPins, balances, banks, types);

    // ── Main loop
    bool running=true;
    while (running) {
        cout<<SHW; // ensure cursor visible at prompt
        int role=loginSplash();

        if (role==1) {
            clientMenu(ACCOUNTS,cardNums,encPins,balances,banks,types);
        } else if (role==2) {
            adminMenu(ACCOUNTS,cardNums,encPins,balances,banks,types,ADMIN_PASS);
        } else {
            cout<<CLS;
            cout<<"\n\n  "<<BOLD<<fBYLW
                <<SYM_SPARKS<<"  Thank you for using Rodge ATM  "<<SYM_SPARKS
                <<RST<<"\n\n";
            cout<<SHW;
            running=false;
        }
    }
    return 0;
}

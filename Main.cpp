/*
╔══════════════════════════════════════════════════════════════════════════════╗
║        F E I N   B A N K I N G   T E R M I N A L   ·   v5.0  .                ║
║        Philippine Banking Simulation  ·  Professional Terminal UI            ║
╠══════════════════════════════════════════════════════════════════════════════╣
║  Authors:  Dimabayao, Karl Ancel T.                                          ║
║            Ramos, Timothy Christopher A.                                     ║
║            Odiame, Rodge Ravin C.                                            ║
╚══════════════════════════════════════════════════════════════════════════════╝
*/

// ════════════════════════════════════════════════════════════════════════
//  DESIGN NOTES
// ════════════════════════════════════════════════════════════════════════
//
//  Box Hierarchy (3 levels, consistent widths):
//
//    Level 1 — HEAVY BOX  (╔═══╗)  Width = W = 74
//              Used for: screen header / brand banner / account panel
//              Functions: hTop() hBot() hMid() hBlank() hRow() hRowC() hKV()
//
//    Level 2 — LIGHT BOX  (┌───┐)  Width = W - 6, indent = 2 spaces
//              Used for: section titles, menus, data panels, previews
//              Functions: lTop() lBot() lMid() lBlank() lRow() lRowC() lKV()
//
//    Level 3 — RECEIPT BOX  (╒═══╕ + ┌───┐ sides)
//              Used for: transaction receipts only
//              Functions: rTop() rRow() rDiv() rBot()
//
//  Color System — MONOTONE only:
//    fWHT  [97m  Bright white  — primary content, values, highlights
//    fSIL  [37m  Silver        — secondary text, labels, descriptions
//    fGRY  [90m  Dark grey     — borders, muted content, timestamps
//    BOLD              Emphasis modifier (no color change)
//    DIM               Deemphasis modifier (no color change)
//
//  Widths:
//    W    = 74   total terminal columns
//    IW   = 70   heavy box inner width  (W - 4)
//    LIW  = 66   light box inner width  (W - 8)
//
//  Key Functions:
//    screenHeader()   — clears screen, draws brand + optional account panel
//    sectionTitle()   — single-row light box with section name + arrow icon
//    showMenu()       — numbered list inside a light box
//    animBar()        — animated fill bar with label, inside a light box
//    spinnerLine()    — braille spinner with completion check
//    typeOut()        — character-by-character typing effect
//    breakBills()     — recursive bill denomination calculator
//    logTx()          — appends transaction to transactions.csv
//    doWithdraw()     — full withdrawal flow (validation→preview→commit→receipt)
//    doTransfer()     — full transfer flow  (validation→preview→commit→receipt)
//
// ════════════════════════════════════════════════════════════════════════
//  FEES REFERENCE
// ════════════════════════════════════════════════════════════════════════
//
//  Bank           Local Fee   Interbank Fee   Daily Limit
//  ──────────────────────────────────────────────────────
//  BDO            PHP 25      PHP 150         PHP 50,000
//  BPI            PHP 20      PHP 125         PHP 75,000
//  Metrobank      PHP 30      PHP 200         PHP 100,000
//  Security Bank  PHP 15      PHP 100         PHP 60,000
//
//  ATM Cassettes:
//    1000-bill cassette  :  500 bills  =  PHP 500,000
//    500-bill  cassette  :  500 bills  =  PHP 250,000
//    Total initial cash  :             =  PHP 750,000
//
// ════════════════════════════════════════════════════════════════════════
//  SECURITY MODEL
// ════════════════════════════════════════════════════════════════════════
//
//  PIN Storage: Caesar cipher +3, pipe-delimited ASCII ordinals
//    Example:  "1234" → "52|53|54|55"
//    Function: encode(pin) / decode(encodedPin)
//
//  PIN Entry: 3-attempt lockout, visual attempt indicator (█ vs ░)
//  Admin:     3-attempt lockout on passcode entry
//  Session:   No persistent state; each login is a fresh session
//
// ════════════════════════════════════════════════════════════════════════


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

// ════════════════════════════════════════════════════════════════════════
//  ANSI  ·  MONOTONE PALETTE  (white / silver / grey)
// ════════════════════════════════════════════════════════════════════════

#define RST   "\033[0m"
#define BOLD  "\033[1m"
#define DIM   "\033[2m"

#define fWHT  "\033[97m"   // bright white   — primary / active
#define fSIL  "\033[37m"   // silver          — secondary / labels
#define fGRY  "\033[90m"   // dark grey       — muted / borders

#define CLS   "\033[2J\033[H"
#define HID   "\033[?25l"
#define SHW   "\033[?25h"

// ── Heavy border characters
#define HV_TL "╔"
#define HV_TR "╗"
#define HV_BL "╚"
#define HV_BR "╝"
#define HV_H  "═"
#define HV_V  "║"
#define HV_ML "╠"
#define HV_MR "╣"

// ── Light border characters
#define LT_TL "┌"
#define LT_TR "┐"
#define LT_BL "└"
#define LT_BR "┘"
#define LT_H  "─"
#define LT_V  "│"
#define LT_ML "├"
#define LT_MR "┤"

// ── Symbols
#define S_DOT   "·"
#define S_BULL  "●"
#define S_DIAM  "◆"
#define S_CHK   "✔"
#define S_CRS   "✖"
#define S_ARR   "▶"
#define S_LOCK  "◉"
#define S_BLK   "█"
#define S_LIT   "░"
#define S_HALF  "▓"
#define S_DASH  "—"

// ════════════════════════════════════════════════════════════════════════
//  LAYOUT
// ════════════════════════════════════════════════════════════════════════

const int W = 74;          // total terminal width
const int IW = W - 4;      // inner width (inside "║ " and " ║")

// ════════════════════════════════════════════════════════════════════════
//  BANK CONSTANTS
// ════════════════════════════════════════════════════════════════════════

const int NB = 4;
const string BNAMES[NB]  = { "BDO", "BPI", "Metrobank", "Security Bank" };
const double LFEE[NB]    = { 25.0,  20.0,  30.0,        15.0  };
const double IFEE[NB]    = { 150.0, 125.0, 200.0,       100.0 };
const double DLIMIT[NB]  = { 50000, 75000, 100000,      60000 };

const int ND       = 2;
int DENOM[ND]      = { 1000, 500  };
int DCNT[ND]       = { 500,  500  };

// ════════════════════════════════════════════════════════════════════════
//  ENCODING  (Caesar +3, pipe-delimited)
// ════════════════════════════════════════════════════════════════════════

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
            if (!tok.empty()) { r += (char)(stoi(tok)-3); tok = ""; }
        } else tok += e[i];
    }
    return r;
}

// ════════════════════════════════════════════════════════════════════════
//  DATA STRUCTURES
// ════════════════════════════════════════════════════════════════════════

struct TxRecord {
    string type;
    double amount;
    double fee;
    string timestamp;
    int    direction;   // +1 credit  /  -1 debit
};

struct Account {
    string card;
    string encPin;
    string name;
    string bank;
    string accType;     // "Local" | "International"
    double balance;
    vector<TxRecord> history;
};

vector<Account> ACCOUNTS;
string ADMIN_PASS;

// ════════════════════════════════════════════════════════════════════════
//  GENERAL UTILITIES
// ════════════════════════════════════════════════════════════════════════

void sleep_ms(int ms) { MSLEEP(ms); }

string rep(const string& s, int n) {
    string r; for (int i = 0; i < n; i++) r += s; return r;
}

string padR(const string& s, int w) {
    if ((int)s.size() >= w) return s.substr(0, w);
    return s + string(w - (int)s.size(), ' ');
}
string padL(const string& s, int w) {
    if ((int)s.size() >= w) return s.substr(0, w);
    return string(w - (int)s.size(), ' ') + s;
}
string padC(const string& s, int w) {
    int l = (w - (int)s.size()) / 2; if (l < 0) l = 0;
    int r = w - (int)s.size() - l;   if (r < 0) r = 0;
    return string(l, ' ') + s + string(r, ' ');
}

// Format Philippine Peso:  50000  →  "PHP 50,000.00"
string php(double v) {
    ostringstream o; o << fixed << setprecision(2) << v;
    string n = o.str();
    int dot   = (int)n.find('.');
    string ip = n.substr(0, dot), dp = n.substr(dot);
    string fmt; int cnt = 0;
    for (int i = (int)ip.size()-1; i >= 0; i--) {
        if (cnt > 0 && cnt % 3 == 0) fmt = "," + fmt;
        fmt = ip[i] + fmt; cnt++;
    }
    return "PHP " + fmt + dp;
}

// Mask card:  "1234 **** **** 5678"
string maskCard(const string& c) {
    if ((int)c.size() != 16) return c;
    return c.substr(0,4) + " **** **** " + c.substr(12,4);
}

bool isDigits(const string& s) {
    for (char c : s) if (c < '0' || c > '9') return false;
    return !s.empty();
}

string nowDate() {
    time_t t = time(0); tm* tm = localtime(&t);
    ostringstream o;
    o << setfill('0') << setw(2) << (tm->tm_mon+1) << "/"
      << setw(2) << tm->tm_mday << "/" << (tm->tm_year+1900);
    return o.str();
}
string nowTime() {
    time_t t = time(0); tm* tm = localtime(&t);
    ostringstream o;
    o << setfill('0') << setw(2) << tm->tm_hour << ":"
      << setw(2) << tm->tm_min  << ":" << setw(2) << tm->tm_sec;
    return o.str();
}
string nowFull() { return nowDate() + "  " + nowTime(); }

bool inMaintWindow() {
    time_t t = time(0); tm* tm = localtime(&t);
    return tm->tm_hour == 8 && tm->tm_min <= 15;
}

int bankIdx(const string& b) {
    for (int i = 0; i < NB; i++) if (BNAMES[i] == b) return i;
    return -1;
}
int findAccount(const string& card) {
    for (int i = 0; i < (int)ACCOUNTS.size(); i++)
        if (ACCOUNTS[i].card == card) return i;
    return -1;
}
double totalCash() {
    double t = 0;
    for (int i = 0; i < ND; i++) t += (double)DENOM[i] * DCNT[i];
    return t;
}

// ════════════════════════════════════════════════════════════════════════
//  RECURSION  — bill breakdown
// ════════════════════════════════════════════════════════════════════════

void breakBills(double amt, int& b1000, int& b500) {
    if (amt <= 0) return;
    if (amt >= 1000) { b1000++; breakBills(amt - 1000, b1000, b500); }
    else if (amt >= 500) { b500++; breakBills(amt - 500, b1000, b500); }
}

// ════════════════════════════════════════════════════════════════════════
//  LOGGING
// ════════════════════════════════════════════════════════════════════════

void logTx(const string& card, const string& type, double amt, double fee) {
    ofstream f("transactions.csv", ios::app);
    if (f.is_open()) {
        f << nowDate() << "," << nowTime() << "," << card << ","
          << type << "," << fixed << setprecision(2) << amt << "," << fee << "\n";
        f.close();
    }
}

// ════════════════════════════════════════════════════════════════════════
//  DRAWING ENGINE  ·  all boxes use same W constant — no drift
// ════════════════════════════════════════════════════════════════════════

/*  HEAVY BOX  — outer frame  (╔═══╗  ║   ║  ╚═══╝)
    Every row is exactly W characters wide.
    Content area = W - 4  (║·<content>·║, one space each side)          */

void hTop() {
    cout << BOLD << fGRY << HV_TL << rep(HV_H, W-2) << HV_TR << RST << "\n";
}
void hBot() {
    cout << BOLD << fGRY << HV_BL << rep(HV_H, W-2) << HV_BR << RST << "\n";
}
void hMid() {
    cout << BOLD << fGRY << HV_ML << rep(HV_H, W-2) << HV_MR << RST << "\n";
}
void hBlank() {
    cout << BOLD << fGRY << HV_V << RST << string(W-2, ' ') << BOLD << fGRY << HV_V << RST << "\n";
}

// Left-aligned content row inside heavy box
// plain = raw string (no ANSI) used for length, display = styled version
void hRow(const string& plain, const string& display) {
    int pad = IW - (int)plain.size(); if (pad < 0) pad = 0;
    cout << BOLD << fGRY << HV_V << RST
         << " " << display << string(pad - 1, ' ')
         << " " << BOLD << fGRY << HV_V << RST << "\n";
}

// Centered content row inside heavy box
void hRowC(const string& plain, const string& display) {
    int l = (IW - (int)plain.size()) / 2; if (l < 0) l = 0;
    int r = IW - (int)plain.size() - l;   if (r < 0) r = 0;
    cout << BOLD << fGRY << HV_V << RST
         << string(l, ' ') << display << string(r, ' ')
         << BOLD << fGRY << HV_V << RST << "\n";
}

// Key / value row inside heavy box — label left, value right
void hKV(const string& label, const string& val, bool hiVal = false) {
    string L = "  " + label;
    string R = val + "  ";
    int pad = IW - (int)L.size() - (int)R.size(); if (pad < 0) pad = 0;
    cout << BOLD << fGRY << HV_V << RST
         << " " << DIM << fSIL << L << RST
         << string(pad, ' ')
         << (hiVal ? string(BOLD) + fWHT : string(fSIL)) << R << RST
         << BOLD << fGRY << HV_V << RST << "\n";
}

/*  LIGHT BOX  — inner panels  (┌───┐  │   │  └───┘)
    Indented 2 spaces, inner width = W - 8                                */

const int LIW = W - 8;   // light inner width

void lTop() {
    cout << "  " << fGRY << LT_TL << rep(LT_H, W-6) << LT_TR << RST << "\n";
}
void lBot() {
    cout << "  " << fGRY << LT_BL << rep(LT_H, W-6) << LT_BR << RST << "\n";
}
void lMid() {
    cout << "  " << fGRY << LT_ML << rep(LT_H, W-6) << LT_MR << RST << "\n";
}
void lBlank() {
    cout << "  " << fGRY << LT_V << RST << string(W-6, ' ') << fGRY << LT_V << RST << "\n";
}

// Left-aligned row in light box
void lRow(const string& plain, const string& display) {
    int pad = LIW - (int)plain.size(); if (pad < 0) pad = 0;
    cout << "  " << fGRY << LT_V << RST
         << " " << display << string(pad - 1, ' ')
         << " " << fGRY << LT_V << RST << "\n";
}

// Centered row in light box
void lRowC(const string& plain, const string& display) {
    int l = (LIW - (int)plain.size()) / 2; if (l < 0) l = 0;
    int r = LIW - (int)plain.size() - l;   if (r < 0) r = 0;
    cout << "  " << fGRY << LT_V << RST
         << string(l, ' ') << display << string(r, ' ')
         << fGRY << LT_V << RST << "\n";
}

// Key / value row in light box
void lKV(const string& label, const string& val, bool hiVal = false) {
    string L = "  " + padR(label, 22);
    string R = val + "  ";
    int pad = LIW - (int)L.size() - (int)R.size(); if (pad < 0) pad = 0;
    cout << "  " << fGRY << LT_V << RST
         << " " << DIM << fSIL << L << RST
         << string(pad, ' ')
         << (hiVal ? string(BOLD) + fWHT : string(fSIL)) << R << RST
         << fGRY << LT_V << RST << "\n";
}

// ════════════════════════════════════════════════════════════════════════
//  ANIMATED ELEMENTS
// ════════════════════════════════════════════════════════════════════════

void animBar(const string& label, int ms = 700) {
    const int BAR = 38;
    cout << "\n";
    lTop();
    lRow(label, string(fSIL) + label + RST);
    // progress bar row — plain = BAR+4 chars
    string plain(BAR + 4, ' ');
    cout << "  " << fGRY << LT_V << RST << "  " << fGRY << "[" << RST;
    cout.flush();
    for (int i = 0; i < BAR; i++) {
        sleep_ms(ms / BAR);
        cout << fWHT << S_BLK << RST;
        cout.flush();
    }
    cout << fGRY << "]" << RST;
    int rpad = LIW - BAR - 4 - 1; if (rpad < 0) rpad = 0;
    cout << string(rpad, ' ') << " " << fGRY << LT_V << RST << "\n";
    string done = "  " + string(S_CHK) + "  Complete";
    lRow(done, string(BOLD) + fWHT + done + RST);
    lBot();
}

void spinnerLine(const string& label) {
    const char* fr[] = {"⠋","⠙","⠹","⠸","⠼","⠴","⠦","⠧","⠇","⠏"};
    for (int i = 0; i < 22; i++) {
        cout << "\r  " << fSIL << fr[i%10] << RST << "  " << DIM << fSIL << label << RST << "   ";
        cout.flush(); sleep_ms(55);
    }
    cout << "\r  " << BOLD << fWHT << S_CHK << RST << "  " << fWHT << label << RST << "   \n";
}

void typeOut(const string& s, int d = 13) {
    for (char c : s) { cout << c; cout.flush(); sleep_ms(d); }
    cout << "\n";
}

// ════════════════════════════════════════════════════════════════════════
//  STATUS MESSAGES  (outside any box, indented)
// ════════════════════════════════════════════════════════════════════════

void msgOK  (const string& m) { cout << "\n  " << BOLD << fWHT << S_CHK << "  " << m << RST << "\n"; }
void msgERR (const string& m) { cout << "\n  " << BOLD << fWHT << S_CRS << "  " << m << RST << "\n"; }
void msgINFO(const string& m) { cout << "\n  " << fSIL  << S_DIAM << "  " << m << RST << "\n"; }
void msgWARN(const string& m) { cout << "\n  " << BOLD << fWHT << "!  " << m << RST << "\n"; }

void pressEnter(const string& lbl = "Press ENTER to continue") {
    cout << "\n  " << DIM << fGRY << S_DOT << S_DOT << S_DOT << "  " << fSIL << lbl << RST << " ";
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    cin.get();
}

// ════════════════════════════════════════════════════════════════════════
//  INPUT PROMPTS
// ════════════════════════════════════════════════════════════════════════

string inputStr(const string& lbl) {
    cout << "\n  " << BOLD << fWHT << lbl << RST
         << "\n  " << fGRY << "  └─> " << RST << fWHT;
    string v; cin >> v; cout << RST; return v;
}

double inputDbl(const string& lbl) {
    cout << "\n  " << BOLD << fWHT << lbl << RST
         << "\n  " << fGRY << "  └─> PHP " << RST << fWHT;
    double v; cin >> v; cout << RST; return v;
}

int inputInt(const string& lbl, int lo, int hi) {
    while (true) {
        cout << "\n  " << BOLD << fWHT << lbl << RST
             << "\n  " << fGRY << "  └─> " << RST << fWHT;
        int v;
        if (cin >> v && v >= lo && v <= hi) { cout << RST; return v; }
        cin.clear(); cin.ignore(numeric_limits<streamsize>::max(), '\n');
        cout << RST;
        msgERR("Please enter a number from " + to_string(lo) + " to " + to_string(hi) + ".");
    }
}

string inputPIN(const string& lbl) {
    cout << "\n  " << BOLD << fWHT << string(S_LOCK) + "  " + lbl << RST
         << "\n  " << fGRY << "  └─> " << RST << fWHT;
    string v; cin >> v; cout << RST; return v;
}

// ════════════════════════════════════════════════════════════════════════
//  SCREEN HEADER  — printed at top of every screen
// ════════════════════════════════════════════════════════════════════════

void screenHeader(const string& name = "", const string& bank = "",
                  const string& accType = "", double balance = -1.0) {
    cout << CLS << HID;

    // ── Brand box
    hTop();
    hBlank();
    hRowC("FEIN BANKING TERMINAL",
          string(BOLD) + fWHT + "FEIN BANKING TERMINAL" + RST);
    hRowC("Secure  " + string(S_DIAM) + "  Reliable  " + string(S_DIAM) + "  Always Available",
          string(DIM) + fSIL + "Secure  " + S_DIAM + "  Reliable  " + S_DIAM + "  Always Available" + RST);
    hBlank();

    // Date + time, flush right
    {
        string dtL = "  " + string(S_DIAM) + "  " + nowDate();
        string dtR = nowTime() + "  ";
        int pad = IW - (int)dtL.size() - (int)dtR.size(); if (pad < 0) pad = 0;
        cout << BOLD << fGRY << HV_V << RST
             << " " << DIM << fSIL << dtL << RST
             << string(pad, ' ')
             << DIM << fSIL << dtR << RST
             << " " << BOLD << fGRY << HV_V << RST << "\n";
    }

    // ── Account panel — only when logged in
    if (!name.empty()) {
        hMid();
        hBlank();

        // Name left  /  account type right
        {
            string L = "  " + string(S_BULL) + "  " + name;
            string R = accType + "  ";
            int pad = IW - (int)L.size() - (int)R.size(); if (pad < 0) pad = 0;
            cout << BOLD << fGRY << HV_V << RST
                 << " " << BOLD << fWHT << L << RST
                 << string(pad, ' ')
                 << DIM << fSIL << R << RST
                 << " " << BOLD << fGRY << HV_V << RST << "\n";
        }

        // Bank left  /  balance right
        {
            string L = "   " + bank;
            string R = (balance >= 0 ? php(balance) : "") + "  ";
            int pad = IW - (int)L.size() - (int)R.size(); if (pad < 0) pad = 0;
            cout << BOLD << fGRY << HV_V << RST
                 << " " << DIM << fSIL << L << RST
                 << string(pad, ' ')
                 << BOLD << fWHT << R << RST
                 << " " << BOLD << fGRY << HV_V << RST << "\n";
        }

        hBlank();
    }
    hBot();
    cout << "\n";
}

// ════════════════════════════════════════════════════════════════════════
//  SECTION TITLE  — light box, single row
// ════════════════════════════════════════════════════════════════════════

void sectionTitle(const string& title) {
    cout << "\n";
    lTop();
    string t = " " + string(S_ARR) + "  " + title;
    lRow(t, string(BOLD) + fWHT + t + RST);
    lBot();
    cout << "\n";
}

// ════════════════════════════════════════════════════════════════════════
//  MENU — numbered light box
// ════════════════════════════════════════════════════════════════════════

void showMenu(const vector<string>& items, const string& title) {
    sectionTitle(title);
    lTop();
    lBlank();
    for (int i = 0; i < (int)items.size(); i++) {
        string num = "[" + to_string(i+1) + "]";
        string txt = "  " + num + "  " + items[i];
        string disp = "  " + string(BOLD) + fWHT + num + RST
                    + "  " + fSIL + items[i] + RST;
        lRow(txt, disp);
    }
    lBlank();
    lBot();
}

// ════════════════════════════════════════════════════════════════════════
//  RECEIPT  — double-top / light-side box
// ════════════════════════════════════════════════════════════════════════

void rTop(const string& title) {
    string t = "  RECEIPT  " + string(S_DIAM) + "  " + title + "  ";
    cout << "\n";
    cout << "  " << fGRY << "╒" << rep("═", W-6) << "╕" << RST << "\n";
    int l = (W-6 - (int)t.size()) / 2; if (l < 0) l = 0;
    int r = W-6 - (int)t.size() - l;   if (r < 0) r = 0;
    cout << "  " << fGRY << HV_V << RST << string(l, ' ')
         << BOLD << fWHT << t << RST
         << string(r, ' ')
         << fGRY << HV_V << RST << "\n";
    cout << "  " << fGRY << "╞" << rep("═", W-6) << "╡" << RST << "\n";
}

void rRow(const string& label, const string& val, bool hi = false) {
    string L = "  " + padR(label, 24);
    string R = val + "  ";
    int pad = W-6-2 - (int)L.size() - (int)R.size(); if (pad < 0) pad = 0;
    cout << "  " << fGRY << LT_V << RST
         << " " << DIM << fSIL << L << RST
         << string(pad, ' ')
         << (hi ? string(BOLD) + fWHT : string(fSIL)) << R << RST
         << fGRY << LT_V << RST << "\n";
}

void rDiv() {
    cout << "  " << fGRY << LT_ML << rep(LT_H, W-6) << LT_MR << RST << "\n";
}

void rBot() {
    cout << "  " << fGRY << "╘" << rep("═", W-6) << "╛" << RST << "\n";
}

// ════════════════════════════════════════════════════════════════════════
//  TRANSACTION HISTORY DISPLAY
// ════════════════════════════════════════════════════════════════════════

void showHistory(const vector<TxRecord>& txs) {
    sectionTitle("TRANSACTION HISTORY  " + string(S_DIAM) + "  Last 10");
    if (txs.empty()) { msgINFO("No transactions on record."); pressEnter(); return; }

    // Column widths inside the light box content area
    const int cN  = 4;
    const int cT  = 20;
    const int cA  = 18;
    const int cF  = 14;
    // remainder = timestamp

    lTop();
    // Header
    {
        string h = " " + padR("#",cN) + padR("TYPE",cT) + padR("AMOUNT",cA) + padR("FEE",cF) + "TIMESTAMP";
        lRow(h, string(BOLD) + fSIL + h + RST);
    }
    lMid();

    int start = (int)txs.size() > 10 ? (int)txs.size()-10 : 0;
    for (int i = start; i < (int)txs.size(); i++) {
        const TxRecord& t = txs[i];
        bool dbt = (t.direction < 0);
        string sign = dbt ? "-" : "+";
        string row = " "
            + padR(to_string(i-start+1), cN)
            + padR(t.type, cT)
            + padR(sign+php(t.amount), cA)
            + padR(php(t.fee), cF)
            + t.timestamp;
        string disp = string(" ")
            + string(DIM) + fSIL + padR(to_string(i-start+1), cN) + RST
            + fSIL + padR(t.type, cT) + RST
            + (dbt ? string(DIM)+fSIL : string(BOLD)+fWHT)
            + padR(sign+php(t.amount), cA) + RST
            + DIM + fSIL + padR(php(t.fee), cF) + RST
            + DIM + fGRY + t.timestamp + RST;
        lRow(row, disp);
    }
    lBot();
    pressEnter();
}

// ════════════════════════════════════════════════════════════════════════
//  WITHDRAW  (overloaded)
// ════════════════════════════════════════════════════════════════════════

bool doWithdraw(vector<Account>& accts, int idx, double amount) {
    Account& a = accts[idx];
    int bi = bankIdx(a.bank);
    double fee   = (a.accType == "Local") ? LFEE[bi] : IFEE[bi];
    double total = amount + fee;

    // Validation
    if (amount <= 0 || fmod(amount, 500.0) != 0.0) {
        msgERR("Amount must be a positive multiple of PHP 500."); return false;
    }
    if (amount > DLIMIT[bi]) {
        msgERR("Exceeds your daily withdrawal limit of " + php(DLIMIT[bi])); return false;
    }
    if (a.balance < total) {
        msgERR("Insufficient balance. Total required: " + php(total)); return false;
    }
    if (totalCash() < amount) {
        msgERR("ATM has insufficient cash. Please try a lower amount."); return false;
    }

    int b1000 = 0, b500 = 0;
    breakBills(amount, b1000, b500);
    if (b1000 > DCNT[0] || b500 > DCNT[1]) {
        msgERR("Cannot dispense exact bills. Please try a different amount."); return false;
    }

    // Transaction preview
    cout << "\n";
    lTop();
    string ph = " Transaction Preview";
    lRow(ph, string(DIM) + fSIL + ph + RST);
    lMid();
    lKV("Withdraw :",        php(amount));
    lKV("Service Fee :",     php(fee));
    lKV("Total Deducted :",  php(total),   true);
    lKV("Bills :", to_string(b1000) + " x PHP 1000   " + to_string(b500) + " x PHP 500");
    lBot();

    string conf = inputStr("Confirm withdrawal? (y / n)");
    if (conf != "y" && conf != "Y") { msgINFO("Withdrawal cancelled."); return false; }

    animBar("Dispensing cash...", 800);

    // Commit
    DCNT[0] -= b1000; DCNT[1] -= b500;
    a.balance -= total;
    a.history.push_back({"Withdrawal", amount, fee, nowFull(), -1});
    logTx(a.card, "Withdrawal", amount, fee);

    rTop("CASH WITHDRAWAL");
    rRow("Date / Time :", nowFull());
    rRow("Cardholder :",  a.name,          true);
    rRow("Card :",        maskCard(a.card));
    rRow("Bank :",        a.bank + "  (" + a.accType + ")");
    rDiv();
    rRow("Amount Withdrawn :", php(amount), true);
    rRow("Service Fee :",      php(fee));
    rDiv();
    rRow("New Balance :", php(a.balance),   true);
    rRow("Bills Dispensed :", to_string(b1000) + " x PHP 1000   " + to_string(b500) + " x PHP 500");
    rBot();
    return true;
}

bool doWithdraw(vector<Account>& accts, int idx, const string& preset) {
    return doWithdraw(accts, idx, stod(preset));
}

// ════════════════════════════════════════════════════════════════════════
//  TRANSFER
// ════════════════════════════════════════════════════════════════════════

void doTransfer(vector<Account>& accts, int idx) {
    string recipCard = inputStr("Recipient Card Number (16 digits)");
    int rIdx = findAccount(recipCard);
    if (rIdx == -1)  { msgERR("Recipient card not found in system.");     pressEnter(); return; }
    if (rIdx == idx) { msgERR("Cannot transfer funds to your own card."); pressEnter(); return; }

    double amount = inputDbl("Amount to Transfer (multiples of PHP 500)");
    if (amount <= 0) { msgERR("Invalid amount entered."); pressEnter(); return; }

    Account& a = accts[idx];
    Account& r = accts[rIdx];
    int bi       = bankIdx(a.bank);
    bool same    = (a.bank == r.bank);
    double fee   = same ? LFEE[bi] : IFEE[bi];
    double total = amount + fee;
    string txType = same ? "Local Transfer" : "Interbank Transfer";

    if (amount > DLIMIT[bi]) { msgERR("Amount exceeds your daily transfer limit."); pressEnter(); return; }
    if (a.balance < total)   { msgERR("Insufficient balance. Required: " + php(total)); pressEnter(); return; }

    // Preview
    cout << "\n";
    lTop();
    string ph = " Transfer Preview";
    lRow(ph, string(DIM) + fSIL + ph + RST);
    lMid();
    lKV("Recipient :",      r.name,   true);
    lKV("Card :",           maskCard(recipCard));
    lKV("Bank :",           r.bank);
    lKV("Transfer Type :",  txType);
    lMid();
    lKV("Amount :",         php(amount), true);
    lKV("Service Fee :",    php(fee));
    lKV("Total Deducted :", php(total),  true);
    lBot();

    string conf = inputStr("Confirm transfer? (y / n)");
    if (conf != "y" && conf != "Y") { msgINFO("Transfer cancelled."); pressEnter(); return; }

    animBar("Processing transfer...", 900);

    a.balance -= total;
    r.balance += amount;

    a.history.push_back({txType,      amount, fee,  nowFull(), -1});
    r.history.push_back({"Transfer In",amount, 0.0, nowFull(), +1});
    logTx(a.card, txType, amount, fee);

    rTop("FUND TRANSFER");
    rRow("Date / Time :", nowFull());
    rRow("From :", a.name + "   " + maskCard(a.card));
    rRow("To :",   r.name + "   " + maskCard(recipCard), true);
    rRow("Type :", txType);
    rDiv();
    rRow("Amount Sent :",   php(amount), true);
    rRow("Service Fee :",   php(fee));
    rDiv();
    rRow("New Balance :",   php(a.balance), true);
    rBot();
    pressEnter();
}

// ════════════════════════════════════════════════════════════════════════
//  CLIENT SESSION
// ════════════════════════════════════════════════════════════════════════

void clientMenu(vector<Account>& accts,
                vector<string>& cardNums, vector<string>& encPins,
                vector<double>& balances, vector<string>& banks,
                vector<string>& types) {

    // ── Step 1 : Card number
    screenHeader();
    sectionTitle("INSERT CARD");

    cout << "  " << DIM << fSIL << "Available accounts on this terminal:" << RST << "\n";
    lTop();
    for (int i = 0; i < (int)accts.size(); i++) {
        string row = "  " + maskCard(accts[i].card)
                   + "   " + padR(accts[i].bank, 15)
                   + "  " + accts[i].name;
        lRow(row, string("  ") + string(DIM) + fSIL + maskCard(accts[i].card)
                + "   " + padR(accts[i].bank, 15)
                + "  " + fSIL + accts[i].name + RST);
    }
    lBot();

    string card = inputStr("Enter Card Number (16 digits)");
    if ((int)card.size() != 16 || !isDigits(card)) {
        msgERR("Invalid card number format."); pressEnter(); return;
    }
    int accIdx = findAccount(card);
    if (accIdx == -1) {
        msgERR("Card not recognized. Please contact your bank."); pressEnter(); return;
    }

    // ── Step 2 : PIN
    screenHeader();
    sectionTitle("ENTER PIN");

    cout << "\n";
    lTop();
    lKV("Card :",       maskCard(card), true);
    lKV("Bank :",       accts[accIdx].bank);
    lKV("Cardholder :", accts[accIdx].name);
    lBot();

    // Attempt indicator using block characters
    auto showDots = [](int used) {
        cout << "\n  " << DIM << fSIL << "Attempts:  " << RST;
        for (int i = 0; i < 3; i++) {
            if (i < used) cout << BOLD << fWHT << S_BLK << RST << "  ";
            else          cout << DIM  << fGRY << S_LIT << RST << "  ";
        }
        cout << "\n";
    };

    bool authed = false;
    for (int att = 1; att <= 3; att++) {
        showDots(att - 1);
        string pin = inputPIN("PIN  (Attempt " + to_string(att) + " of 3)");
        if (encode(pin) == accts[accIdx].encPin) { authed = true; break; }
        if (att < 3) msgERR("Incorrect PIN.  " + to_string(3-att) + " attempt(s) remaining.");
    }
    if (!authed) {
        msgERR("Card blocked after 3 failed attempts. Please contact your bank.");
        pressEnter(); return;
    }

    spinnerLine("Verifying credentials...");
    msgOK("Authentication successful.");
    sleep_ms(450);

    // ── Step 3 : Transaction loop
    bool active = true;
    while (active) {
        Account& acc = accts[accIdx];
        int bankI = bankIdx(acc.bank);

        screenHeader(acc.name, acc.bank, acc.accType, acc.balance);

        // Quick-withdraw hint strip
        {
            string presets[] = {"500","1000","2000","5000","10000","20000"};
            cout << "  " << DIM << fSIL << "Quick Withdraw:  " << RST;
            for (int i = 0; i < 6; i++)
                cout << BOLD << fWHT << "[" << (i+1) << "]" << RST
                     << DIM << fSIL << " PHP" << presets[i] << RST << "  ";
            cout << "\n\n";
        }

        vector<string> menu = {
            "Check Balance",
            "Withdraw Cash",
            "Transfer Funds",
            "Transaction History",
            "Change PIN",
            "Logout"
        };
        showMenu(menu, "SELECT TRANSACTION");
        int ch = inputInt("Enter option (1 - 6)", 1, 6);

        switch (ch) {

        // ── Balance inquiry
        case 1: {
            screenHeader(acc.name, acc.bank, acc.accType, acc.balance);
            sectionTitle("BALANCE INQUIRY");
            cout << "\n";
            lTop();
            lKV("Cardholder :", acc.name);
            lKV("Bank :",        acc.bank + "   (" + acc.accType + ")");
            lKV("Card :",        maskCard(acc.card));
            lKV("Daily Limit :", php(DLIMIT[bankI]));
            lKV("As of :",       nowFull());
            lMid();
            string bs = php(acc.balance);
            string brow = "  Available Balance  " + bs;
            lRow(brow, string("  ") + string(DIM) + fSIL + "Available Balance  " + RST
                     + BOLD + fWHT + bs + RST);
            lBot();
            pressEnter();
            break;
        }

        // ── Withdraw
        case 2: {
            screenHeader(acc.name, acc.bank, acc.accType, acc.balance);
            sectionTitle("WITHDRAW CASH");

            double fee = (acc.accType == "Local") ? LFEE[bankI] : IFEE[bankI];

            // Info row
            cout << "\n";
            lTop();
            lKV("Service Fee :", php(fee));
            lKV("Daily Limit :", php(DLIMIT[bankI]));
            lMid();

            // 2 × 3 preset grid
            string PS[] = {"500","1000","2000","5000","10000","20000"};
            for (int row = 0; row < 2; row++) {
                string pRow, pDisp;
                pRow  = "  ";
                pDisp = "  ";
                for (int col = 0; col < 3; col++) {
                    int id = row*3 + col;
                    string num = "[" + to_string(id+1) + "]";
                    string amt = padR("PHP " + PS[id], 14);
                    pRow  += num + "  " + amt + "   ";
                    pDisp += string(BOLD) + fWHT + num + RST + "  " + string(fSIL) + amt + RST + "   ";
                }
                lRow(pRow, pDisp);
            }
            lMid();
            lRow("  [7]  Custom Amount",
                 "  " + string(BOLD)+fWHT+"[7]"+RST+"  "+fSIL+"Custom Amount"+RST);
            lBot();

            int wc = inputInt("Select amount (1 - 7)", 1, 7);
            if (wc <= 6) doWithdraw(accts, accIdx, PS[wc-1]);
            else {
                double custom = inputDbl("Enter custom amount (multiple of PHP 500)");
                doWithdraw(accts, accIdx, custom);
            }
            pressEnter();
            break;
        }

        // ── Transfer
        case 3: {
            screenHeader(acc.name, acc.bank, acc.accType, acc.balance);
            sectionTitle("FUND TRANSFER");
            doTransfer(accts, accIdx);
            break;
        }

        // ── History
        case 4: {
            screenHeader(acc.name, acc.bank, acc.accType, acc.balance);
            showHistory(acc.history);
            break;
        }

        // ── Change PIN
        case 5: {
            screenHeader(acc.name, acc.bank, acc.accType, acc.balance);
            sectionTitle("CHANGE PIN");
            string oldPin = inputPIN("Current PIN");
            if (encode(oldPin) != acc.encPin) {
                msgERR("Incorrect current PIN."); pressEnter(); break;
            }
            string newPin  = inputPIN("New PIN");
            string confPin = inputPIN("Confirm New PIN");
            if (newPin != confPin) {
                msgERR("PINs do not match. Please try again."); pressEnter();
            } else if (encode(newPin) == acc.encPin) {
                msgERR("New PIN must be different from current PIN."); pressEnter();
            } else {
                acc.encPin     = encode(newPin);
                encPins[accIdx] = acc.encPin;
                spinnerLine("Updating PIN...");
                msgOK("PIN changed successfully."); pressEnter();
            }
            break;
        }

        // ── Logout
        case 6: {
            screenHeader();
            cout << "\n";
            lTop();
            lRowC("Thank you for using FEIN BANKING TERMINAL",
                  string(BOLD) + fWHT + "Thank you for using FEIN BANKING TERMINAL" + RST);
            lRowC("Please collect your card. Have a safe day.",
                  string(DIM) + fSIL + "Please collect your card. Have a safe day." + RST);
            lBot();
            cout << SHW;
            sleep_ms(1400);
            active = false;
            break;
        }

        } // end switch
    } // end session loop
}

// ════════════════════════════════════════════════════════════════════════
//  ADMINISTRATOR PANEL
// ════════════════════════════════════════════════════════════════════════

void adminMenu(vector<Account>& accts,
               vector<string>& cardNums, vector<string>& encPins,
               vector<double>& balances, vector<string>& banks,
               vector<string>& types, string& adminPass) {

    screenHeader();
    sectionTitle("ADMINISTRATOR ACCESS");
    msgWARN("RESTRICTED AREA  " + string(S_DIAM) + "  Authorized Personnel Only");

    // Auth
    bool authed = false;
    for (int att = 1; att <= 3; att++) {
        string pass = inputPIN("Admin Passcode  (Attempt " + to_string(att) + " of 3)");
        if (encode(pass) == adminPass) { authed = true; break; }
        if (att < 3) msgERR("Wrong passcode.  " + to_string(3-att) + " attempt(s) remaining.");
    }
    if (!authed) {
        msgERR("System locked after 3 failed attempts."); pressEnter(); return;
    }

    animBar("Loading admin panel...", 600);
    msgOK("Administrator access granted.");
    sleep_ms(300);

    bool running = true;
    while (running) {
        screenHeader();
        cout << "  " << DIM << fSIL << "Logged in as  " << RST
             << BOLD << fWHT << "ADMINISTRATOR" << RST
             << "     " << DIM << fGRY << nowFull() << RST << "\n\n";

        vector<string> menu = {
            "ATM Cash Status",
            "Refill ATM Cash  [ 8:00 - 8:15 AM only ]",
            "View All Accounts",
            "Create New Account",
            "Delete Account",
            "Reset Account PIN",
            "Change Admin Passcode",
            "Exit Admin Panel"
        };
        showMenu(menu, "ADMIN OPERATIONS");
        int ch = inputInt("Enter option (1 - 8)", 1, 8);

        switch (ch) {

        // ── Cash status
        case 1: {
            screenHeader();
            sectionTitle("ATM CASH STATUS");
            cout << "\n";
            lTop();
            double tot = 0;
            for (int i = 0; i < ND; i++) {
                double sub = (double)DENOM[i] * DCNT[i]; tot += sub;
                lKV("PHP " + to_string(DENOM[i]) + " bills :",
                    to_string(DCNT[i]) + " pcs   " + php(sub));
            }
            lMid();
            lKV("Total Cash in ATM :", php(tot), true);
            lBot();
            pressEnter();
            break;
        }

        // ── Refill
        case 2: {
            screenHeader();
            sectionTitle("CASH REFILL");
            if (!inMaintWindow()) {
                msgERR("Refill is only allowed during 8:00 to 8:15 AM maintenance window.");
                msgINFO("Current time: " + nowTime());
            } else {
                msgOK("Maintenance window active. Proceed with refill.");
                for (int i = 0; i < ND; i++) {
                    cout << "\n";
                    lTop();
                    lKV("Denomination :", "PHP " + to_string(DENOM[i]));
                    lKV("Current Count :", to_string(DCNT[i]) + " bills");
                    lBot();
                    int add = inputInt("Add how many PHP " + to_string(DENOM[i]) + " bills? (0 to 10000)", 0, 10000);
                    DCNT[i] += add;
                    if (add > 0) msgOK("Added " + to_string(add) + " x PHP " + to_string(DENOM[i]));
                }
                msgOK("Refill complete.  Total cash: " + php(totalCash()));
            }
            pressEnter();
            break;
        }

        // ── View accounts
        case 3: {
            screenHeader();
            sectionTitle("ALL REGISTERED ACCOUNTS");
            if (accts.empty()) { msgINFO("No accounts registered."); pressEnter(); break; }

            cout << "\n";
            lTop();
            {
                const int cN  = 4;
                const int cC  = 22;
                const int cNm = 22;
                const int cB  = 16;
                const int cTp = 16;
                string h = " " + padR("#",cN) + padR("CARD",cC) + padR("HOLDER",cNm)
                         + padR("BANK",cB) + padR("TYPE",cTp) + "BALANCE";
                lRow(h, string(BOLD) + fSIL + h + RST);
            }
            lMid();
            for (int i = 0; i < (int)accts.size(); i++) {
                const int cN=4,cC=22,cNm=22,cB=16,cTp=16;
                string row = " "
                    + padR(to_string(i+1), cN)
                    + padR(maskCard(accts[i].card), cC)
                    + padR(accts[i].name, cNm)
                    + padR(accts[i].bank, cB)
                    + padR(accts[i].accType, cTp)
                    + php(accts[i].balance);
                string disp = string(" ")
                    + string(DIM) + fGRY + padR(to_string(i+1), cN) + RST
                    + DIM + fSIL + padR(maskCard(accts[i].card), cC) + RST
                    + fSIL + padR(accts[i].name, cNm) + RST
                    + BOLD + fWHT + padR(accts[i].bank, cB) + RST
                    + DIM + fSIL + padR(accts[i].accType, cTp) + RST
                    + BOLD + fWHT + php(accts[i].balance) + RST;
                lRow(row, disp);
            }
            lBot();
            pressEnter();
            break;
        }

        // ── Create account
        case 4: {
            screenHeader();
            sectionTitle("CREATE NEW ACCOUNT");

            string nc = inputStr("New Card Number (16 digits)");
            if ((int)nc.size() != 16 || !isDigits(nc)) { msgERR("Invalid card number."); pressEnter(); break; }
            if (findAccount(nc) != -1) { msgERR("Card number already exists in system."); pressEnter(); break; }

            string nn;
            cout << "\n  " << BOLD << fWHT << "Cardholder Full Name:" << RST
                 << "\n  " << fGRY << "  └─> " << RST << fWHT;
            cin.ignore(); getline(cin, nn); cout << RST;

            string np = inputPIN("Initial PIN");

            cout << "\n";
            lTop();
            for (int i = 0; i < NB; i++) {
                string row = "  [" + to_string(i+1) + "]  " + BNAMES[i];
                lRow(row, "  " + string(BOLD)+fWHT+"["+to_string(i+1)+"]"+RST+"  "+fSIL+BNAMES[i]+RST);
            }
            lBot();
            int bc2 = inputInt("Select Bank (1 - 4)", 1, NB);
            string nbank = BNAMES[bc2-1];

            cout << "\n";
            lTop();
            lRow("  [1]  Local", "  "+string(BOLD)+fWHT+"[1]"+RST+"  "+fSIL+"Local"+RST);
            lRow("  [2]  International", "  "+string(BOLD)+fWHT+"[2]"+RST+"  "+fSIL+"International"+RST);
            lBot();
            int at = inputInt("Account Type (1 - 2)", 1, 2);
            string ntype = (at == 2) ? "International" : "Local";

            double nbal = inputDbl("Initial Balance");

            Account na{nc, encode(np), nn, nbank, ntype, nbal, {}};
            accts.push_back(na);
            cardNums.push_back(nc); encPins.push_back(encode(np));
            balances.push_back(nbal); banks.push_back(nbank); types.push_back(ntype);

            spinnerLine("Creating account...");
            msgOK("Account created:  " + maskCard(nc) + "   (" + nbank + ")");
            pressEnter();
            break;
        }

        // ── Delete account
        case 5: {
            screenHeader();
            sectionTitle("DELETE ACCOUNT");

            string dc = inputStr("Card Number to Delete");
            int di = findAccount(dc);
            if (di == -1) { msgERR("Account not found."); pressEnter(); break; }

            Account& da = accts[di];
            cout << "\n";
            lTop();
            lKV("Card :",    maskCard(dc));
            lKV("Holder :",  da.name, true);
            lKV("Bank :",    da.bank);
            lKV("Balance :", php(da.balance));
            lBot();
            msgWARN("This action is permanent and cannot be reversed.");

            string conf = inputStr("Type  YES  to confirm deletion");
            if (conf == "YES") {
                accts.erase(accts.begin() + di);
                cardNums.erase(cardNums.begin() + di);
                encPins.erase(encPins.begin() + di);
                balances.erase(balances.begin() + di);
                banks.erase(banks.begin() + di);
                types.erase(types.begin() + di);
                msgOK("Account permanently deleted.");
            } else {
                msgINFO("Deletion cancelled.");
            }
            pressEnter();
            break;
        }

        // ── Reset PIN
        case 6: {
            screenHeader();
            sectionTitle("RESET ACCOUNT PIN");
            string rc = inputStr("Card Number");
            int ri = findAccount(rc);
            if (ri == -1) { msgERR("Account not found."); pressEnter(); break; }
            string np = inputPIN("New PIN for  " + maskCard(rc));
            accts[ri].encPin = encode(np);
            encPins[ri]      = encode(np);
            msgOK("PIN reset for:  " + accts[ri].name + "   (" + accts[ri].bank + ")");
            pressEnter();
            break;
        }

        // ── Change admin passcode
        case 7: {
            screenHeader();
            sectionTitle("CHANGE ADMIN PASSCODE");
            string cur = inputPIN("Current Passcode");
            if (encode(cur) != adminPass) { msgERR("Incorrect passcode."); pressEnter(); break; }
            string nw = inputPIN("New Passcode");
            string cf = inputPIN("Confirm New Passcode");
            if (nw != cf) {
                msgERR("Passcodes do not match."); pressEnter();
            } else {
                adminPass = encode(nw);
                msgOK("Admin passcode updated successfully."); pressEnter();
            }
            break;
        }

        case 8:
            msgINFO("Exiting administrator panel...");
            sleep_ms(400);
            running = false;
            break;

        } // end admin switch
    }
}

// ════════════════════════════════════════════════════════════════════════
//  SPLASH SCREEN
// ════════════════════════════════════════════════════════════════════════

int loginSplash() {
    cout << CLS << HID << "\n\n";

    // Top border line
    cout << "  " << DIM << fGRY << rep(LT_H, W-4) << RST << "\n\n";

    // ASCII wordmark — FEIN
    string art[] = {
        "  ███████╗███████╗██╗███╗  ██╗",
        "  ██╔════╝██╔════╝██║████╗ ██║",
        "  █████╗  █████╗  ██║██╔██╗██║",
        "  ██╔══╝  ██╔══╝  ██║██║╚████║",
        "  ██║     ███████╗██║██║  ╚███║",
        "  ╚═╝     ╚══════╝╚═╝╚═╝   ╚══╝"
    };
    // Gradient brightness top→bottom
    const char* shades[] = {
        BOLD "\033[97m", BOLD "\033[97m",
        "\033[37m",      "\033[37m",
        DIM  "\033[37m", DIM  "\033[90m"
    };
    for (int i = 0; i < 6; i++)
        cout << shades[i] << art[i] << RST << "\n";

    cout << "\n";

    // Sub-brand line
    string sub = "B A N K I N G   T E R M I N A L   "
                 + string(S_DIAM) + "   P H I L I P P I N E   S Y S T E M";
    int sp = (W-4 - (int)sub.size()) / 2; if (sp < 0) sp = 0;
    cout << "  " << string(sp, ' ') << DIM << fSIL << sub << RST << "\n\n";

    // Supported banks
    cout << "  " << DIM << fGRY << rep(LT_H, W-4) << RST << "\n";
    cout << "  ";
    for (int i = 0; i < NB; i++)
        cout << DIM << fSIL << S_BULL << "  " << padR(BNAMES[i], 17) << RST;
    cout << "\n";
    cout << "  " << DIM << fGRY << rep(LT_H, W-4) << RST << "\n\n";

    // Date / time
    cout << "  " << DIM << fGRY << nowDate()
         << "   " << S_DIAM << "   " << nowTime() << RST << "\n\n";

    // Bottom border line
    cout << "  " << DIM << fGRY << rep(LT_H, W-4) << RST << "\n\n";

    // Login options
    lTop();
    lRow("  [1]  Client Login",
         "  " + string(BOLD)+fWHT+"[1]"+RST+"  "+fSIL+"Client Login"+RST);
    lRow("  [2]  Administrator Login",
         "  " + string(BOLD)+fWHT+"[2]"+RST+"  "+fSIL+"Administrator Login"+RST);
    lRow("  [0]  Exit",
         string("  ") + string(DIM)+fGRY+"[0]  Exit"+RST);
    lBot();

    return inputInt("Select (0 - 2)", 0, 2);
}

// ════════════════════════════════════════════════════════════════════════
//  SEED DATA
// ════════════════════════════════════════════════════════════════════════

void seedAccounts(vector<Account>& accts,
                  vector<string>& cardNums, vector<string>& encPins,
                  vector<double>& bals, vector<string>& bnks, vector<string>& types) {

    auto add = [&](const string& card, const string& pin, const string& name,
                   const string& bank, const string& type, double bal,
                   vector<TxRecord> seed = {}) {
        Account a{card, encode(pin), name, bank, type, bal, seed};
        accts.push_back(a);
        cardNums.push_back(card); encPins.push_back(encode(pin));
        bals.push_back(bal);      bnks.push_back(bank); types.push_back(type);
    };

    add("1234567890123456","1234","Juan dela Cruz",   "BDO",          "Local",         50000.0,{
        {"Withdrawal",   2000, 25,  "2025-03-12  08:14", -1},
        {"Transfer Out", 5000, 25,  "2025-03-12  09:00", -1},
        {"Transfer In",  1500,  0,  "2025-03-11  15:30", +1},
    });
    add("9876543210987654","5678","Maria Santos",     "BPI",          "Local",         75000.0,{
        {"Withdrawal",   3000, 20,  "2025-03-12  11:20", -1},
        {"Transfer In", 10000,  0,  "2025-03-10  14:05", +1},
    });
    add("1111222233334444","9999","Pedro Reyes",      "Metrobank",    "International",100000.0);
    add("5555666677778888","0000","Ana Garcia",       "Security Bank","Local",         60000.0,{
        {"Withdrawal",   5000, 15,  "2025-03-11  16:40", -1},
    });
    add("4444333322221111","1111","Jose Rizal Jr.",   "BDO",          "Local",         35000.0);
    add("8888777766665555","2222","Lola Nena Flores", "BPI",          "International",120000.0,{
        {"Transfer Out",20000,125,  "2025-03-10  09:30", -1},
        {"Withdrawal",  10000,125,  "2025-03-09  14:15", -1},
        {"Transfer In", 50000,  0,  "2025-03-08  10:00", +1},
    });
    add("1234123412341234","3333","Miguel Torres",    "Metrobank",    "Local",         47500.0);
    add("9999888877776666","4444","Clara Bautista",   "Security Bank","Local",         92000.0,{
        {"Transfer In", 30000,  0,  "2025-03-11  08:00", +1},
        {"Withdrawal",   8000, 15,  "2025-03-11  12:30", -1},
    });
}

// ════════════════════════════════════════════════════════════════════════
//  MAIN
// ════════════════════════════════════════════════════════════════════════

int main() {
    // Initialise transaction log
    {
        ifstream ck("transactions.csv");
        if (!ck.good()) {
            ofstream o("transactions.csv");
            o << "Date,Time,Card,Type,Amount,Fee\n";
        }
    }

    ADMIN_PASS = encode("6767");   // default admin passcode

    vector<string> cardNums, encPins, banks, types;
    vector<double> balances;

    seedAccounts(ACCOUNTS, cardNums, encPins, balances, banks, types);

    bool running = true;
    while (running) {
        cout << SHW;
        int role = loginSplash();

        if (role == 1) {
            clientMenu(ACCOUNTS, cardNums, encPins, balances, banks, types);
        } else if (role == 2) {
            adminMenu(ACCOUNTS, cardNums, encPins, balances, banks, types, ADMIN_PASS);
        } else {
            cout << CLS << "\n\n";
            lTop();
            lRowC("Thank you for using FEIN BANKING TERMINAL",
                  string(BOLD) + fWHT + "Thank you for using FEIN BANKING TERMINAL" + RST);
            lRowC("Dimabayao  |  Ramos  |  Odiame",
                  string(DIM) + fSIL + "Dimabayao  |  Ramos  |  Odiame" + RST);
            lBot();
            cout << "\n" << SHW;
            running = false;
        }
    }
    return 0;
}
// ════════════════════════════════════════════════════════════════════════
//  QUICK REFERENCE  ·  Test Accounts
// ════════════════════════════════════════════════════════════════════════
//
//  Card Number       PIN   Name                  Bank           Type
//  ──────────────────────────────────────────────────────────────────────
//  1234567890123456  1234  Juan dela Cruz         BDO            Local
//  9876543210987654  5678  Maria Santos           BPI            Local
//  1111222233334444  9999  Pedro Reyes            Metrobank      International
//  5555666677778888  0000  Ana Garcia             Security Bank  Local
//  4444333322221111  1111  Jose Rizal Jr.         BDO            Local
//  8888777766665555  2222  Lola Nena Flores       BPI            International
//  1234123412341234  3333  Miguel Torres          Metrobank      Local
//  9999888877776666  4444  Clara Bautista         Security Bank  Local
//
//  Admin Passcode : 6767
//
// ════════════════════════════════════════════════════════════════════════
//  Build:  g++ -std=c++17 -o fein fein_atm.cpp && ./fein
// ════════════════════════════════════════════════════════════════════════

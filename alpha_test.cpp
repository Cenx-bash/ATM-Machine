#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <ctime>
#include <algorithm>
#include <map>
#include <functional>
#include <thread>
#include <chrono>
#include <cmath>
#include <cstdio>
#include <termios.h>
#include <unistd.h>
#include <sys/ioctl.h>

// Enhanced ANSI color codes
#define RESET       "\033[0m"
#define BLACK       "\033[30m"
#define RED         "\033[31m"
#define GREEN       "\033[32m"
#define YELLOW      "\033[33m"
#define BLUE        "\033[34m"
#define MAGENTA     "\033[35m"
#define CYAN        "\033[36m"
#define WHITE       "\033[37m"
#define GRAY        "\033[90m"

// Bold colors
#define BOLD_RED    "\033[1;31m"
#define BOLD_GREEN  "\033[1;32m"
#define BOLD_YELLOW "\033[1;33m"
#define BOLD_BLUE   "\033[1;34m"
#define BOLD_CYAN   "\033[1;36m"

// Background colors
#define BG_RED      "\033[41m"
#define BG_GREEN    "\033[42m"
#define BG_YELLOW   "\033[43m"
#define BG_BLUE     "\033[44m"
#define BG_CYAN     "\033[46m"

// Text styles
#define BOLD        "\033[1m"
#define UNDERLINE   "\033[4m"
#define BLINK       "\033[5m"

// Forward declarations
struct Account;
struct Transaction;
struct Admin;
struct ATM_Cash;
struct Notification;

// Function declarations
std::string getCurrentDateTime();
std::string getCurrentDate();
std::string generateReference();
void setColor(const std::string& color);
void resetColor();
void clearScreen();
char getCh();
int getConsoleWidth();
int getConsoleHeight();
void setConsoleTitle(const std::string& title);
void sleepMs(int milliseconds);
void printCentered(const std::string& text);
void printLine(char ch, int length = 0);
void printHeader(const std::string& title);
void printSuccess(const std::string& message);
void printError(const std::string& message);
void printWarning(const std::string& message);
void printInfo(const std::string& message);
void printMoney(double amount);
void printBox(const std::vector<std::string>& content, const std::string& title = "");
void loadingAnimation(const std::string& message, int seconds = 1);
void typewriterEffect(const std::string& message, int delayMs = 30);
std::string encodeString(const std::string& input);
std::string decodeString(const std::string& input);
bool validateCardNumber(const std::string& cardNumber);
bool validatePIN(const std::string& pin);
bool validateEmail(const std::string& email);
bool validatePhone(const std::string& phone);
void logTransaction(const std::string& cardNumber, const std::string& type, 
                   double amount, double fee, const std::string& status);
double calculateFeeRecursive(double amount, int depth = 0);
bool calculateBillsRecursive(double amount, int& hundreds, int& fifties, 
                            int& twenties, int& tens, int& fives, int& ones, int depth = 0);
void addNotification(const std::string& type, const std::string& message, 
                    const std::string& recipientCard = "");
void loadData();
void saveData();
void loadTransactions();
bool findRecipient(const std::string& cardNumber, Account*& recipient);
bool checkMaintenanceWindow();
void displayProgressBar(int percentage, int width = 50);
void displayReceipt(const std::map<std::string, std::string>& details);
void displayMenu(const std::vector<std::pair<std::string, std::string>>& options, 
                const std::string& title, const std::string& footer = "");
void displayAccountCard(const Account& account);
void displayATMCashPretty();
void displayStatistics();
bool login();
void checkBalance();
void withdraw();
void transferFunds();
void viewTransactionHistory();
void changePIN();
void viewNotifications();
void updateProfile();
void logout();
void clientMenu();
void viewATMCash();
void refillCash();
void createAccount();
void viewAccounts();
void deleteAccount();
void resetUserPIN();
void changeAdminPasscode();
void viewAdminUsers();
void systemStatistics();
void adminMenu();

// Structure definitions
struct Account {
    std::string cardNumber;
    std::string pin;
    std::string name;
    double balance;
    bool isActive;
    std::string createdAt;
    std::string lastLogin;
    int failedAttempts;
    bool isLocked;
    std::string email;
    std::string phone;
    
    Account() : balance(0.0), isActive(true), failedAttempts(0), 
                isLocked(false) {}
};

struct Admin {
    std::string username;
    std::string password;
    std::string lastLogin;
    std::string role;
};

struct Transaction {
    std::string cardNumber;
    std::string type;
    double amount;
    double fee;
    std::string dateTime;
    std::string status;
    std::string reference;
    double balanceAfter;
};

struct ATM_Cash {
    int bills100;
    int bills50;
    int bills20;
    int bills10;
    int bills5;
    int bills1;
    std::string lastRefill;
    std::string lastAudit;
    
    ATM_Cash() : bills100(100), bills50(100), bills20(100), 
                 bills10(100), bills5(100), bills1(100) {
        lastRefill = getCurrentDateTime();
        lastAudit = getCurrentDateTime();
    }
    
    double getTotalCash() {
        return bills100 * 100 + bills50 * 50 + bills20 * 20 + 
               bills10 * 10 + bills5 * 5 + bills1 * 1;
    }
    
    int getTotalBills() {
        return bills100 + bills50 + bills20 + bills10 + bills5 + bills1;
    }
};

struct Notification {
    std::string type;
    std::string message;
    std::string dateTime;
    bool isRead;
    std::string recipientCard;
};

// Global variables
std::vector<Account> accounts;
std::vector<Admin> admins;
std::vector<Transaction> transactions;
std::vector<Notification> notifications;
ATM_Cash atmCash;
Account* currentUser = nullptr;
Admin* currentAdmin = nullptr;
const std::string TRANSACTION_FILE = "transactions.csv";
const std::string NOTIFICATION_FILE = "notifications.dat";

// Cross-platform console functions
void setColor(const std::string& color) {
    std::cout << color;
}

void resetColor() {
    std::cout << RESET;
}

void clearScreen() {
    std::cout << "\033[2J\033[1;1H";
    std::cout.flush();
}

char getCh() {
    struct termios oldt, newt;
    char ch;
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    ch = getchar();
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    return ch;
}

int getConsoleWidth() {
    struct winsize w;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) == 0) {
        return w.ws_col;
    }
    return 80;
}

int getConsoleHeight() {
    struct winsize w;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) == 0) {
        return w.ws_row;
    }
    return 24;
}

void setConsoleTitle(const std::string& title) {
    std::cout << "\033]0;" << title << "\007";
}

void sleepMs(int milliseconds) {
    std::this_thread::sleep_for(std::chrono::milliseconds(milliseconds));
}

// Utility functions
std::string getCurrentDateTime() {
    time_t now = time(0);
    tm* ltm = localtime(&now);
    char buffer[80];
    strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", ltm);
    return std::string(buffer);
}

std::string getCurrentDate() {
    time_t now = time(0);
    tm* ltm = localtime(&now);
    char buffer[80];
    strftime(buffer, sizeof(buffer), "%Y-%m-%d", ltm);
    return std::string(buffer);
}

std::string generateReference() {
    static int counter = 0;
    counter++;
    time_t now = time(0);
    std::string ref = "TXN" + std::to_string(now) + std::to_string(counter);
    return ref.substr(0, 15);
}

void printCentered(const std::string& text) {
    int width = getConsoleWidth();
    int padding = (width - text.length()) / 2;
    if (padding > 0) std::cout << std::string(padding, ' ');
    std::cout << text;
}

void printLine(char ch, int length) {
    if (length == 0) length = getConsoleWidth();
    std::cout << std::string(length, ch);
}

void printHeader(const std::string& title) {
    clearScreen();
    int width = getConsoleWidth();
    
    setColor(BOLD_CYAN);
    printLine('=', width);
    std::cout << "\n";
    
    std::string decorated = " 🏦 " + title + " 🏦 ";
    printCentered(decorated);
    std::cout << "\n";
    
    printLine('=', width);
    resetColor();
    std::cout << "\n\n";
}

void printSuccess(const std::string& message) {
    setColor(BOLD_GREEN);
    std::cout << "  ✅ " << message << "\n";
    resetColor();
}

void printError(const std::string& message) {
    setColor(BOLD_RED);
    std::cout << "  ❌ " << message << "\n";
    resetColor();
}

void printWarning(const std::string& message) {
    setColor(BOLD_YELLOW);
    std::cout << "  ⚠️  " << message << "\n";
    resetColor();
}

void printInfo(const std::string& message) {
    setColor(BOLD_BLUE);
    std::cout << "  ℹ️  " << message << "\n";
    resetColor();
}

void printMoney(double amount) {
    setColor(BOLD_GREEN);
    std::cout << "$" << std::fixed << std::setprecision(2) << amount;
    resetColor();
}

void printBox(const std::vector<std::string>& content, const std::string& title) {
    int width = getConsoleWidth() - 4;
    int contentWidth = 0;
    for (const auto& line : content) {
        contentWidth = std::max(contentWidth, (int)line.length());
    }
    contentWidth = std::min(contentWidth, width - 4);
    
    setColor(CYAN);
    std::cout << "  ╔" << std::string(contentWidth + 2, '═') << "╗\n";
    
    if (!title.empty()) {
        std::cout << "  ║ " << std::left << std::setw(contentWidth) 
                  << BOLD + title + RESET + CYAN << " ║\n";
        std::cout << "  ╠" << std::string(contentWidth + 2, '═') << "╣\n";
    }
    
    for (const auto& line : content) {
        std::cout << "  ║ " << std::left << std::setw(contentWidth) 
                  << line << " ║\n";
    }
    
    std::cout << "  ╚" << std::string(contentWidth + 2, '═') << "╝\n";
    resetColor();
}

void loadingAnimation(const std::string& message, int seconds) {
    std::cout << message;
    for (int i = 0; i < seconds * 4; i++) {
        std::cout << ".";
        std::cout.flush();
        sleepMs(250);
    }
    std::cout << "\n";
}

void typewriterEffect(const std::string& message, int delayMs) {
    for (char c : message) {
        std::cout << c;
        std::cout.flush();
        sleepMs(delayMs);
    }
    std::cout << "\n";
}

void displayMenu(const std::vector<std::pair<std::string, std::string>>& options, 
                const std::string& title, const std::string& footer) {
    int width = getConsoleWidth() - 4;
    int maxOptionLength = 0;
    for (const auto& opt : options) {
        maxOptionLength = std::max(maxOptionLength, (int)(opt.first.length() + opt.second.length() + 5));
    }
    
    setColor(BOLD_CYAN);
    std::cout << "  ╔" << std::string(width, '═') << "╗\n";
    std::cout << "  ║" << std::setw(width) << std::left << " " << "║\n";
    
    // Title
    std::string centeredTitle = " " + title + " ";
    int titlePadding = (width - centeredTitle.length()) / 2;
    std::cout << "  ║" << std::string(titlePadding, ' ') 
              << BOLD_YELLOW << centeredTitle << RESET << BOLD_CYAN
              << std::string(width - titlePadding - centeredTitle.length(), ' ') << "║\n";
    
    std::cout << "  ║" << std::setw(width) << std::left << " " << "║\n";
    std::cout << "  ╠" << std::string(width, '═') << "╣\n";
    
    // Options
    for (size_t i = 0; i < options.size(); i++) {
        std::string optionNum = "  " + std::to_string(i + 1) + ". ";
        std::string fullOption = optionNum + options[i].first + " " + options[i].second;
        
        std::cout << "  ║ " << std::left << std::setw(width - 2) << fullOption << " ║\n";
    }
    
    std::cout << "  ║" << std::setw(width) << std::left << " " << "║\n";
    
    if (!footer.empty()) {
        std::cout << "  ╠" << std::string(width, '═') << "╣\n";
        std::cout << "  ║ " << std::left << std::setw(width - 2) << footer << " ║\n";
    }
    
    std::cout << "  ╚" << std::string(width, '═') << "╝\n";
    resetColor();
    std::cout << "\n  " << BOLD << "Enter choice: " << RESET;
}

void displayAccountCard(const Account& account) {
    std::vector<std::string> card;
    card.push_back("┌────────────────────────────────────┐");
    card.push_back("│                                    │");
    card.push_back("│  " + std::string(BOLD) + "CARD NUMBER" + RESET + "                    │");
    card.push_back("│  " + account.cardNumber.substr(0, 4) + " " + 
                         account.cardNumber.substr(4, 4) + " " + 
                         account.cardNumber.substr(8, 4) + " " + 
                         account.cardNumber.substr(12, 4) + "          │");
    card.push_back("│                                    │");
    card.push_back("│  " + std::string(BOLD) + "ACCOUNT HOLDER" + RESET + "                 │");
    card.push_back("│  " + account.name + "                         │");
    card.push_back("│                                    │");
    card.push_back("│  " + std::string(BOLD) + "BALANCE" + RESET + "                         │");
    card.push_back("│  " + std::string(BOLD_GREEN) + "$" + std::to_string(account.balance) + RESET + "                       │");
    card.push_back("│                                    │");
    card.push_back("│  " + std::string(BOLD) + "STATUS" + RESET + "                          │");
    card.push_back("│  " + std::string(account.isActive ? BOLD_GREEN : BOLD_RED) + 
                         (account.isActive ? "ACTIVE" : "INACTIVE") + RESET + "                       │");
    card.push_back("│                                    │");
    card.push_back("└────────────────────────────────────┘");
    
    for (const auto& line : card) {
        std::cout << "  " << line << "\n";
    }
}

void displayATMCashPretty() {
    double total = atmCash.getTotalCash();
    int width = 50;
    
    setColor(BOLD_CYAN);
    std::cout << "  ┌────────────────────────────────────────────────┐\n";
    std::cout << "  │              ATM CASH INVENTORY                │\n";
    std::cout << "  ├────────────────────────────────────────────────┤\n";
    std::cout << "  │ Denomination  │ Count    │ Value               │\n";
    std::cout << "  ├────────────────────────────────────────────────┤\n";
    resetColor();
    
    printf("  │ $100          │ %-8d│ $%-20.2f│\n", atmCash.bills100, atmCash.bills100 * 100.0);
    printf("  │ $50           │ %-8d│ $%-20.2f│\n", atmCash.bills50, atmCash.bills50 * 50.0);
    printf("  │ $20           │ %-8d│ $%-20.2f│\n", atmCash.bills20, atmCash.bills20 * 20.0);
    printf("  │ $10           │ %-8d│ $%-20.2f│\n", atmCash.bills10, atmCash.bills10 * 10.0);
    printf("  │ $5            │ %-8d│ $%-20.2f│\n", atmCash.bills5, atmCash.bills5 * 5.0);
    printf("  │ $1            │ %-8d│ $%-20.2f│\n", atmCash.bills1, atmCash.bills1 * 1.0);
    
    setColor(BOLD_CYAN);
    std::cout << "  ├────────────────────────────────────────────────┤\n";
    resetColor();
    printf("  │ TOTAL CASH    │         │ $%-20.2f│\n", total);
    
    setColor(BOLD_CYAN);
    std::cout << "  └────────────────────────────────────────────────┘\n";
    resetColor();
    
    std::cout << "\n";
    std::cout << "  Last Refill: " << atmCash.lastRefill << "\n";
    std::cout << "  Last Audit:  " << atmCash.lastAudit << "\n";
    
    if (total < 10000) {
        printWarning("⚠️  ATM cash level is LOW!");
        printInfo("  Recommended refill amount: $" + std::to_string(20000 - total));
    }
}

void displayProgressBar(int percentage, int width) {
    int filled = (percentage * width) / 100;
    std::cout << "[";
    setColor(BOLD_GREEN);
    for (int i = 0; i < filled; i++) std::cout << "█";
    resetColor();
    for (int i = filled; i < width; i++) std::cout << "░";
    std::cout << "] " << percentage << "%";
}

void displayReceipt(const std::map<std::string, std::string>& details) {
    std::cout << "\n";
    setColor(BOLD_CYAN);
    std::cout << "  ┌────────────────────────────────────────┐\n";
    std::cout << "  │         TRANSACTION RECEIPT            │\n";
    std::cout << "  ├────────────────────────────────────────┤\n";
    resetColor();
    
    for (const auto& pair : details) {
        std::cout << "  │ " << std::left << std::setw(20) << pair.first 
                  << ": " << std::setw(20) << pair.second << " │\n";
    }
    
    setColor(BOLD_CYAN);
    std::cout << "  ├────────────────────────────────────────┤\n";
    std::cout << "  │     Thank you for banking with us!     │\n";
    std::cout << "  └────────────────────────────────────────┘\n";
    resetColor();
}

// Security functions
std::string encodeString(const std::string& input) {
    std::string encoded = input;
    for (char& c : encoded) {
        c = c + 3;
    }
    return encoded;
}

std::string decodeString(const std::string& input) {
    std::string decoded = input;
    for (char& c : decoded) {
        c = c - 3;
    }
    return decoded;
}

bool validateCardNumber(const std::string& cardNumber) {
    if (cardNumber.length() != 16) return false;
    for (char c : cardNumber) {
        if (!isdigit(c)) return false;
    }
    
    int sum = 0;
    bool alternate = false;
    for (int i = cardNumber.length() - 1; i >= 0; i--) {
        int n = cardNumber[i] - '0';
        if (alternate) {
            n *= 2;
            if (n > 9) n -= 9;
        }
        sum += n;
        alternate = !alternate;
    }
    return (sum % 10 == 0);
}

bool validatePIN(const std::string& pin) {
    if (pin.length() < 4 || pin.length() > 6) return false;
    for (char c : pin) {
        if (!isdigit(c)) return false;
    }
    return true;
}

bool validateEmail(const std::string& email) {
    size_t atPos = email.find('@');
    size_t dotPos = email.rfind('.');
    return (atPos != std::string::npos && dotPos != std::string::npos && 
            atPos < dotPos && dotPos < email.length() - 1);
}

bool validatePhone(const std::string& phone) {
    if (phone.length() != 10) return false;
    for (char c : phone) {
        if (!isdigit(c)) return false;
    }
    return true;
}

// Transaction functions
void logTransaction(const std::string& cardNumber, const std::string& type, 
                   double amount, double fee, const std::string& status) {
    Transaction trans;
    trans.cardNumber = cardNumber;
    trans.type = type;
    trans.amount = amount;
    trans.fee = fee;
    trans.dateTime = getCurrentDateTime();
    trans.status = status;
    trans.reference = generateReference();
    
    for (const auto& acc : accounts) {
        if (acc.cardNumber == cardNumber) {
            trans.balanceAfter = acc.balance;
            break;
        }
    }
    
    transactions.push_back(trans);
    
    if (transactions.size() > 5000) {
        transactions.erase(transactions.begin());
    }
    
    std::ofstream file(TRANSACTION_FILE, std::ios::app);
    if (file.is_open()) {
        file << trans.cardNumber << "," << trans.type << "," 
             << trans.amount << "," << trans.fee << ","
             << trans.dateTime << "," << trans.status << ","
             << trans.reference << "," << trans.balanceAfter << "\n";
        file.close();
    }
}

double calculateFeeRecursive(double amount, int depth) {
    if (amount <= 100 || depth >= 5) return 0.0;
    
    double feeRate = 0.0;
    if (amount > 5000) feeRate = 0.025;
    else if (amount > 2000) feeRate = 0.02;
    else if (amount > 1000) feeRate = 0.015;
    else if (amount > 500) feeRate = 0.01;
    else if (amount > 100) feeRate = 0.005;
    
    double fee = amount * feeRate;
    
    if (fee > 100) {
        fee += calculateFeeRecursive(fee, depth + 1);
    }
    
    return fee;
}

bool calculateBillsRecursive(double amount, int& hundreds, int& fifties, 
                            int& twenties, int& tens, int& fives, int& ones, int depth) {
    if (amount < 0.01) return true;
    if (depth > 100) return false;
    
    amount = std::round(amount * 100) / 100;
    
    if (amount >= 100) {
        int maxBills = std::min(static_cast<int>(amount / 100), 
                               atmCash.bills100 - hundreds);
        for (int i = maxBills; i >= 0; i--) {
            hundreds += i;
            if (calculateBillsRecursive(amount - i * 100, hundreds, fifties, 
                                       twenties, tens, fives, ones, depth + 1)) {
                return true;
            }
            hundreds -= i;
        }
    }
    else if (amount >= 50) {
        int maxBills = std::min(static_cast<int>(amount / 50), 
                               atmCash.bills50 - fifties);
        for (int i = maxBills; i >= 0; i--) {
            fifties += i;
            if (calculateBillsRecursive(amount - i * 50, hundreds, fifties, 
                                       twenties, tens, fives, ones, depth + 1)) {
                return true;
            }
            fifties -= i;
        }
    }
    else if (amount >= 20) {
        int maxBills = std::min(static_cast<int>(amount / 20), 
                               atmCash.bills20 - twenties);
        for (int i = maxBills; i >= 0; i--) {
            twenties += i;
            if (calculateBillsRecursive(amount - i * 20, hundreds, fifties, 
                                       twenties, tens, fives, ones, depth + 1)) {
                return true;
            }
            twenties -= i;
        }
    }
    else if (amount >= 10) {
        int maxBills = std::min(static_cast<int>(amount / 10), 
                               atmCash.bills10 - tens);
        for (int i = maxBills; i >= 0; i--) {
            tens += i;
            if (calculateBillsRecursive(amount - i * 10, hundreds, fifties, 
                                       twenties, tens, fives, ones, depth + 1)) {
                return true;
            }
            tens -= i;
        }
    }
    else if (amount >= 5) {
        int maxBills = std::min(static_cast<int>(amount / 5), 
                               atmCash.bills5 - fives);
        for (int i = maxBills; i >= 0; i--) {
            fives += i;
            if (calculateBillsRecursive(amount - i * 5, hundreds, fifties, 
                                       twenties, tens, fives, ones, depth + 1)) {
                return true;
            }
            fives -= i;
        }
    }
    else if (amount >= 1) {
        int needed = static_cast<int>(amount);
        if (needed <= atmCash.bills1 - ones) {
            ones += needed;
            return true;
        }
    }
    
    return false;
}

void addNotification(const std::string& type, const std::string& message, 
                    const std::string& recipientCard) {
    Notification notif;
    notif.type = type;
    notif.message = message;
    notif.dateTime = getCurrentDateTime();
    notif.isRead = false;
    notif.recipientCard = recipientCard;
    notifications.push_back(notif);
    
    std::ofstream file(NOTIFICATION_FILE, std::ios::binary | std::ios::app);
    if (file.is_open()) {
        size_t len = notif.type.length();
        file.write(reinterpret_cast<char*>(&len), sizeof(len));
        file.write(notif.type.c_str(), len);
        
        len = notif.message.length();
        file.write(reinterpret_cast<char*>(&len), sizeof(len));
        file.write(notif.message.c_str(), len);
        
        len = notif.dateTime.length();
        file.write(reinterpret_cast<char*>(&len), sizeof(len));
        file.write(notif.dateTime.c_str(), len);
        
        file.write(reinterpret_cast<char*>(&notif.isRead), sizeof(notif.isRead));
        
        len = notif.recipientCard.length();
        file.write(reinterpret_cast<char*>(&len), sizeof(len));
        file.write(notif.recipientCard.c_str(), len);
        
        file.close();
    }
}

// Data persistence functions
void loadData() {
    std::ifstream file("accounts.dat", std::ios::binary);
    if (file.is_open()) {
        size_t count;
        file.read(reinterpret_cast<char*>(&count), sizeof(count));
        accounts.resize(count);
        for (auto& acc : accounts) {
            size_t len;
            file.read(reinterpret_cast<char*>(&len), sizeof(len));
            acc.cardNumber.resize(len);
            file.read(&acc.cardNumber[0], len);
            
            file.read(reinterpret_cast<char*>(&len), sizeof(len));
            acc.pin.resize(len);
            file.read(&acc.pin[0], len);
            
            file.read(reinterpret_cast<char*>(&len), sizeof(len));
            acc.name.resize(len);
            file.read(&acc.name[0], len);
            
            file.read(reinterpret_cast<char*>(&acc.balance), sizeof(acc.balance));
            file.read(reinterpret_cast<char*>(&acc.isActive), sizeof(acc.isActive));
            
            file.read(reinterpret_cast<char*>(&len), sizeof(len));
            acc.createdAt.resize(len);
            file.read(&acc.createdAt[0], len);
            
            file.read(reinterpret_cast<char*>(&len), sizeof(len));
            acc.lastLogin.resize(len);
            file.read(&acc.lastLogin[0], len);
            
            file.read(reinterpret_cast<char*>(&acc.failedAttempts), sizeof(acc.failedAttempts));
            file.read(reinterpret_cast<char*>(&acc.isLocked), sizeof(acc.isLocked));
            
            file.read(reinterpret_cast<char*>(&len), sizeof(len));
            acc.email.resize(len);
            file.read(&acc.email[0], len);
            
            file.read(reinterpret_cast<char*>(&len), sizeof(len));
            acc.phone.resize(len);
            file.read(&acc.phone[0], len);
        }
        file.close();
    }
    
    std::ifstream afile("admins.dat", std::ios::binary);
    if (afile.is_open()) {
        size_t count;
        afile.read(reinterpret_cast<char*>(&count), sizeof(count));
        admins.resize(count);
        for (auto& admin : admins) {
            size_t len;
            afile.read(reinterpret_cast<char*>(&len), sizeof(len));
            admin.username.resize(len);
            afile.read(&admin.username[0], len);
            
            afile.read(reinterpret_cast<char*>(&len), sizeof(len));
            admin.password.resize(len);
            afile.read(&admin.password[0], len);
            
            afile.read(reinterpret_cast<char*>(&len), sizeof(len));
            admin.lastLogin.resize(len);
            afile.read(&admin.lastLogin[0], len);
            
            afile.read(reinterpret_cast<char*>(&len), sizeof(len));
            admin.role.resize(len);
            afile.read(&admin.role[0], len);
        }
        afile.close();
    }
    
    std::ifstream cfile("atm_cash.dat", std::ios::binary);
    if (cfile.is_open()) {
        cfile.read(reinterpret_cast<char*>(&atmCash), sizeof(atmCash));
        cfile.close();
    }
    
    std::ifstream nfile(NOTIFICATION_FILE, std::ios::binary);
    if (nfile.is_open()) {
        size_t count;
        nfile.read(reinterpret_cast<char*>(&count), sizeof(count));
        notifications.resize(count);
        for (auto& notif : notifications) {
            size_t len;
            nfile.read(reinterpret_cast<char*>(&len), sizeof(len));
            notif.type.resize(len);
            nfile.read(&notif.type[0], len);
            
            nfile.read(reinterpret_cast<char*>(&len), sizeof(len));
            notif.message.resize(len);
            nfile.read(&notif.message[0], len);
            
            nfile.read(reinterpret_cast<char*>(&len), sizeof(len));
            notif.dateTime.resize(len);
            nfile.read(&notif.dateTime[0], len);
            
            nfile.read(reinterpret_cast<char*>(&notif.isRead), sizeof(notif.isRead));
            
            nfile.read(reinterpret_cast<char*>(&len), sizeof(len));
            notif.recipientCard.resize(len);
            nfile.read(&notif.recipientCard[0], len);
        }
        nfile.close();
    }
}

void saveData() {
    std::ofstream file("accounts.dat", std::ios::binary);
    if (file.is_open()) {
        size_t count = accounts.size();
        file.write(reinterpret_cast<char*>(&count), sizeof(count));
        for (const auto& acc : accounts) {
            size_t len = acc.cardNumber.length();
            file.write(reinterpret_cast<char*>(&len), sizeof(len));
            file.write(acc.cardNumber.c_str(), len);
            
            len = acc.pin.length();
            file.write(reinterpret_cast<char*>(&len), sizeof(len));
            file.write(acc.pin.c_str(), len);
            
            len = acc.name.length();
            file.write(reinterpret_cast<char*>(&len), sizeof(len));
            file.write(acc.name.c_str(), len);
            
            file.write(reinterpret_cast<const char*>(&acc.balance), sizeof(acc.balance));
            file.write(reinterpret_cast<const char*>(&acc.isActive), sizeof(acc.isActive));
            
            len = acc.createdAt.length();
            file.write(reinterpret_cast<char*>(&len), sizeof(len));
            file.write(acc.createdAt.c_str(), len);
            
            len = acc.lastLogin.length();
            file.write(reinterpret_cast<char*>(&len), sizeof(len));
            file.write(acc.lastLogin.c_str(), len);
            
            file.write(reinterpret_cast<const char*>(&acc.failedAttempts), sizeof(acc.failedAttempts));
            file.write(reinterpret_cast<const char*>(&acc.isLocked), sizeof(acc.isLocked));
            
            len = acc.email.length();
            file.write(reinterpret_cast<char*>(&len), sizeof(len));
            file.write(acc.email.c_str(), len);
            
            len = acc.phone.length();
            file.write(reinterpret_cast<char*>(&len), sizeof(len));
            file.write(acc.phone.c_str(), len);
        }
        file.close();
    }
    
    std::ofstream afile("admins.dat", std::ios::binary);
    if (afile.is_open()) {
        size_t count = admins.size();
        afile.write(reinterpret_cast<char*>(&count), sizeof(count));
        for (const auto& admin : admins) {
            size_t len = admin.username.length();
            afile.write(reinterpret_cast<char*>(&len), sizeof(len));
            afile.write(admin.username.c_str(), len);
            
            len = admin.password.length();
            afile.write(reinterpret_cast<char*>(&len), sizeof(len));
            afile.write(admin.password.c_str(), len);
            
            len = admin.lastLogin.length();
            afile.write(reinterpret_cast<char*>(&len), sizeof(len));
            afile.write(admin.lastLogin.c_str(), len);
            
            len = admin.role.length();
            afile.write(reinterpret_cast<char*>(&len), sizeof(len));
            afile.write(admin.role.c_str(), len);
        }
        afile.close();
    }
    
    std::ofstream cfile("atm_cash.dat", std::ios::binary);
    if (cfile.is_open()) {
        cfile.write(reinterpret_cast<const char*>(&atmCash), sizeof(atmCash));
        cfile.close();
    }
    
    std::ofstream nfile(NOTIFICATION_FILE, std::ios::binary);
    if (nfile.is_open()) {
        size_t count = notifications.size();
        nfile.write(reinterpret_cast<char*>(&count), sizeof(count));
        for (const auto& notif : notifications) {
            size_t len = notif.type.length();
            nfile.write(reinterpret_cast<char*>(&len), sizeof(len));
            nfile.write(notif.type.c_str(), len);
            
            len = notif.message.length();
            nfile.write(reinterpret_cast<char*>(&len), sizeof(len));
            nfile.write(notif.message.c_str(), len);
            
            len = notif.dateTime.length();
            nfile.write(reinterpret_cast<char*>(&len), sizeof(len));
            nfile.write(notif.dateTime.c_str(), len);
            
            nfile.write(reinterpret_cast<const char*>(&notif.isRead), sizeof(notif.isRead));
            
            len = notif.recipientCard.length();
            nfile.write(reinterpret_cast<char*>(&len), sizeof(len));
            nfile.write(notif.recipientCard.c_str(), len);
        }
        nfile.close();
    }
}

void loadTransactions() {
    std::ifstream file(TRANSACTION_FILE);
    if (!file.is_open()) return;
    
    std::string line;
    std::getline(file, line); // Skip header
    
    while (std::getline(file, line)) {
        std::stringstream ss(line);
        Transaction trans;
        std::string field;
        
        std::getline(ss, trans.cardNumber, ',');
        std::getline(ss, trans.type, ',');
        
        std::getline(ss, field, ',');
        trans.amount = std::stod(field);
        
        std::getline(ss, field, ',');
        trans.fee = std::stod(field);
        
        std::getline(ss, trans.dateTime, ',');
        std::getline(ss, trans.status, ',');
        std::getline(ss, trans.reference, ',');
        
        std::getline(ss, field, ',');
        trans.balanceAfter = std::stod(field);
        
        transactions.push_back(trans);
    }
    file.close();
}

bool findRecipient(const std::string& cardNumber, Account*& recipient) {
    for (auto& account : accounts) {
        if (account.cardNumber == cardNumber && account.isActive && !account.isLocked) {
            recipient = &account;
            return true;
        }
    }
    return false;
}

bool checkMaintenanceWindow() {
    time_t now = time(0);
    tm* ltm = localtime(&now);
    int hour = ltm->tm_hour;
    return (hour >= 2 && hour < 4);
}

bool login() {
    printHeader("ATM SECURE LOGIN");
    
    std::cout << "\n";
    
    // Display welcome message with typewriter effect
    setColor(BOLD_CYAN);
    typewriterEffect("  Welcome to the ATM Banking System", 20);
    resetColor();
    std::cout << "\n";
    
    std::vector<std::pair<std::string, std::string>> loginOptions = {
        {"👤", "Client Login"},
        {"🛠️", "Admin Login"},
        {"🚪", "Exit System"}
    };
    
    displayMenu(loginOptions, "SELECT LOGIN TYPE");
    
    int choice;
    std::cin >> choice;
    
    if (choice == 3) {
        printHeader("GOODBYE");
        std::vector<std::string> farewell = {
            "Thank you for using our ATM system!",
            "Have a great day! 👋"
        };
        printBox(farewell, "FAREWELL");
        std::cout << "\n\n";
        exit(0);
    }
    
    std::cout << "\n";
    std::string username, password;
    
    setColor(BOLD_BLUE);
    std::cout << "  ┌─ " << (choice == 1 ? "Card Number" : "Username") << ": ";
    resetColor();
    std::cin >> username;
    
    setColor(BOLD_BLUE);
    std::cout << "  ├─ PIN/Password: ";
    resetColor();
    
    // Hide password input
    char ch;
    password = "";
    while ((ch = getCh()) != '\n' && ch != '\r') {
        if (ch == 127 || ch == '\b') {
            if (password.length() > 0) {
                password.pop_back();
                std::cout << "\b \b";
            }
        } else {
            password += ch;
            std::cout << '*';
        }
    }
    std::cout << "\n";
    
    if (choice == 1) {
        // Check if account is locked
        for (auto& account : accounts) {
            if (account.cardNumber == username) {
                if (account.isLocked) {
                    printError("Account is locked due to too many failed attempts!");
                    printInfo("Please contact customer support.");
                    return false;
                }
                break;
            }
        }
        
        // Client login
        for (auto& account : accounts) {
            if (account.cardNumber == username && 
                decodeString(account.pin) == password && 
                account.isActive) {
                
                account.failedAttempts = 0;
                account.lastLogin = getCurrentDateTime();
                currentUser = &account;
                
                printSuccess("Login successful!");
                loadingAnimation("Welcome " + account.name, 1);
                return true;
            }
        }
        
        // Increment failed attempts
        for (auto& account : accounts) {
            if (account.cardNumber == username) {
                account.failedAttempts++;
                if (account.failedAttempts >= 3) {
                    account.isLocked = true;
                    printWarning("Account has been locked due to multiple failed attempts!");
                }
                break;
            }
        }
        
        printError("Invalid credentials!");
        return false;
        
    } else if (choice == 2) {
        // Admin login
        for (auto& admin : admins) {
            if (admin.username == username && 
                decodeString(admin.password) == password) {
                currentAdmin = &admin;
                admin.lastLogin = getCurrentDateTime();
                saveData();
                
                printSuccess("Admin login successful!");
                loadingAnimation("Welcome " + admin.username, 1);
                return true;
            }
        }
        
        printError("Invalid admin credentials!");
        return false;
    }
    
    return false;
}

void checkBalance() {
    printHeader("BALANCE INQUIRY");
    displayAccountCard(*currentUser);
    
    logTransaction(currentUser->cardNumber, "BALANCE_INQUIRY", 
                   currentUser->balance, 0.0, "SUCCESS");
    
    std::cout << "\n  Account Health:\n  ";
    int healthPercent = std::min(100, static_cast<int>((currentUser->balance / 10000) * 100));
    displayProgressBar(healthPercent, 50);
    std::cout << "\n";
}

void withdraw() {
    printHeader("CASH WITHDRAWAL");
    
    std::cout << "\n  Current Balance: ";
    printMoney(currentUser->balance);
    std::cout << "\n\n";
    
    std::cout << "  Available Denominations:\n";
    setColor(GRAY);
    std::cout << "    $100  $50  $20  $10  $5  $1\n\n";
    resetColor();
    
    std::vector<double> quickAmounts = {20, 40, 60, 80, 100, 200, 500};
    std::cout << "  Quick Amounts: ";
    for (size_t i = 0; i < quickAmounts.size(); i++) {
        setColor(BOLD_CYAN);
        std::cout << "[" << (i + 1) << "]";
        resetColor();
        std::cout << "$" << quickAmounts[i] << " ";
    }
    std::cout << "\n\n";
    
    double amount;
    std::cout << "  Enter amount to withdraw: $";
    std::cin >> amount;
    
    if (amount >= 1 && amount <= 7 && amount == static_cast<int>(amount)) {
        amount = quickAmounts[static_cast<int>(amount) - 1];
        std::cout << "  Selected quick amount: $" << amount << "\n";
    }
    
    if (amount <= 0) {
        printError("Invalid amount!");
        return;
    }
    
    if (amount > 10000) {
        printError("Maximum withdrawal amount is $10,000 per transaction!");
        return;
    }
    
    if (amount > currentUser->balance) {
        printError("Insufficient balance!");
        return;
    }
    
    double fee = calculateFeeRecursive(amount, 0);
    double totalDeduction = amount + fee;
    
    if (totalDeduction > currentUser->balance) {
        printError("Insufficient balance to cover fee of $" + std::to_string(fee));
        return;
    }
    
    int hundreds = 0, fifties = 0, twenties = 0, tens = 0, fives = 0, ones = 0;
    
    loadingAnimation("  Calculating optimal denomination", 1);
    
    if (!calculateBillsRecursive(amount, hundreds, fifties, twenties, tens, fives, ones, 0)) {
        printError("ATM cannot dispense this amount!");
        return;
    }
    
    if (hundreds > atmCash.bills100 || fifties > atmCash.bills50 || 
        twenties > atmCash.bills20 || tens > atmCash.bills10 || 
        fives > atmCash.bills5 || ones > atmCash.bills1) {
        printError("ATM has insufficient bills!");
        return;
    }
    
    std::cout << "\n  " << std::string(50, '─') << "\n";
    std::cout << "  WITHDRAWAL SUMMARY\n";
    std::cout << "  " << std::string(50, '─') << "\n";
    std::cout << "  Amount:        ";
    printMoney(amount);
    std::cout << "\n  Fee:           ";
    printMoney(fee);
    std::cout << "\n  Total:         ";
    printMoney(totalDeduction);
    std::cout << "\n  New Balance:   ";
    printMoney(currentUser->balance - totalDeduction);
    std::cout << "\n\n  Bills Dispensed:\n";
    
    if (hundreds > 0) std::cout << "    $100 x " << hundreds << "\n";
    if (fifties > 0) std::cout << "    $50  x " << fifties << "\n";
    if (twenties > 0) std::cout << "    $20  x " << twenties << "\n";
    if (tens > 0) std::cout << "    $10  x " << tens << "\n";
    if (fives > 0) std::cout << "    $5   x " << fives << "\n";
    if (ones > 0) std::cout << "    $1   x " << ones << "\n";
    
    std::cout << "\n  " << std::string(50, '─') << "\n";
    std::cout << "  Confirm withdrawal? (y/n): ";
    
    char confirm;
    std::cin >> confirm;
    
    if (confirm == 'y' || confirm == 'Y') {
        currentUser->balance -= totalDeduction;
        
        atmCash.bills100 -= hundreds;
        atmCash.bills50 -= fifties;
        atmCash.bills20 -= twenties;
        atmCash.bills10 -= tens;
        atmCash.bills5 -= fives;
        atmCash.bills1 -= ones;
        
        logTransaction(currentUser->cardNumber, "WITHDRAWAL", amount, fee, "SUCCESS");
        
        std::map<std::string, std::string> receipt;
        receipt["Transaction Type"] = "Withdrawal";
        receipt["Date/Time"] = getCurrentDateTime();
        receipt["Reference"] = generateReference();
        receipt["Amount"] = "$" + std::to_string(amount);
        receipt["Fee"] = "$" + std::to_string(fee);
        receipt["Total"] = "$" + std::to_string(totalDeduction);
        receipt["New Balance"] = "$" + std::to_string(currentUser->balance);
        
        displayReceipt(receipt);
        
        printSuccess("Withdrawal successful!");
        
        if (atmCash.getTotalCash() < 5000) {
            addNotification("LOW_CASH", "ATM cash level is below $5000", "");
        }
    } else {
        printWarning("Withdrawal cancelled.");
    }
}

void transferFunds() {
    printHeader("FUND TRANSFER");
    
    std::cout << "\n  Your Balance: ";
    printMoney(currentUser->balance);
    std::cout << "\n\n";
    
    std::string recipientCard;
    std::cout << "  Enter recipient's card number: ";
    std::cin >> recipientCard;
    
    if (!validateCardNumber(recipientCard)) {
        printError("Invalid card number format!");
        return;
    }
    
    Account* recipient = nullptr;
    if (!findRecipient(recipientCard, recipient)) {
        printError("Recipient account not found or inactive!");
        return;
    }
    
    if (recipient->cardNumber == currentUser->cardNumber) {
        printError("Cannot transfer to your own account!");
        return;
    }
    
    std::string maskedName = recipient->name;
    if (maskedName.length() > 2) {
        maskedName = maskedName.substr(0, 2) + std::string(maskedName.length() - 2, '*');
    }
    std::string maskedCard = "**** **** **** " + recipient->cardNumber.substr(12, 4);
    
    std::cout << "\n  Recipient: " << maskedName << "\n";
    std::cout << "  Card: " << maskedCard << "\n\n";
    
    double amount;
    std::cout << "  Enter amount to transfer: $";
    std::cin >> amount;
    
    if (amount <= 0) {
        printError("Invalid amount!");
        return;
    }
    
    if (amount > 50000) {
        printError("Maximum transfer amount is $50,000!");
        return;
    }
    
    double fee = calculateFeeRecursive(amount, 0);
    double totalDeduction = amount + fee;
    
    if (totalDeduction > currentUser->balance) {
        printError("Insufficient balance to cover transfer and fee!");
        return;
    }
    
    std::cout << "\n  " << std::string(50, '─') << "\n";
    std::cout << "  TRANSFER SUMMARY\n";
    std::cout << "  " << std::string(50, '─') << "\n";
    std::cout << "  Amount:        ";
    printMoney(amount);
    std::cout << "\n  Fee:           ";
    printMoney(fee);
    std::cout << "\n  Total:         ";
    printMoney(totalDeduction);
    std::cout << "\n  New Balance:   ";
    printMoney(currentUser->balance - totalDeduction);
    std::cout << "\n\n  " << std::string(50, '─') << "\n";
    
    std::cout << "  Confirm transfer? (y/n): ";
    char confirm;
    std::cin >> confirm;
    
    if (confirm == 'y' || confirm == 'Y') {
        currentUser->balance -= totalDeduction;
        recipient->balance += amount;
        
        logTransaction(currentUser->cardNumber, "TRANSFER_OUT", amount, fee, "SUCCESS");
        logTransaction(recipient->cardNumber, "TRANSFER_IN", amount, 0.0, "SUCCESS");
        
        std::map<std::string, std::string> receipt;
        receipt["Transaction Type"] = "Fund Transfer";
        receipt["Date/Time"] = getCurrentDateTime();
        receipt["Reference"] = generateReference();
        receipt["To Account"] = maskedCard;
        receipt["Amount"] = "$" + std::to_string(amount);
        receipt["Fee"] = "$" + std::to_string(fee);
        receipt["Total"] = "$" + std::to_string(totalDeduction);
        receipt["New Balance"] = "$" + std::to_string(currentUser->balance);
        
        displayReceipt(receipt);
        
        printSuccess("Transfer successful!");
        
        addNotification("TRANSFER_IN", 
                       "You received $" + std::to_string(amount) + " from " + 
                       currentUser->cardNumber, recipient->cardNumber);
    } else {
        printWarning("Transfer cancelled.");
    }
}

void viewTransactionHistory() {
    printHeader("TRANSACTION HISTORY");
    
    std::vector<Transaction> userTransactions;
    for (const auto& trans : transactions) {
        if (trans.cardNumber == currentUser->cardNumber) {
            userTransactions.push_back(trans);
        }
    }
    
    if (userTransactions.empty()) {
        printInfo("No transactions found.");
        return;
    }
    
    double totalWithdrawn = 0, totalDeposited = 0, totalFees = 0;
    for (const auto& trans : userTransactions) {
        if (trans.type == "WITHDRAWAL" || trans.type == "TRANSFER_OUT") {
            totalWithdrawn += trans.amount;
            totalFees += trans.fee;
        } else if (trans.type == "TRANSFER_IN") {
            totalDeposited += trans.amount;
        }
    }
    
    setColor(BOLD_CYAN);
    std::cout << "  ┌────────────────────────────────────────────────────────────────┐\n";
    std::cout << "  │ " << std::left << std::setw(20) << "Date/Time" 
              << std::setw(15) << "Type" 
              << std::setw(12) << "Amount" 
              << std::setw(10) << "Fee" 
              << std::setw(15) << "Reference" << " │\n";
    std::cout << "  ├────────────────────────────────────────────────────────────────┤\n";
    resetColor();
    
    int count = 0;
    for (const auto& trans : userTransactions) {
        if (count++ >= 10) {
            setColor(GRAY);
            std::cout << "  │ " << std::left << std::setw(74) 
                      << "... and " + std::to_string(userTransactions.size() - 10) + " more transactions" << " │\n";
            resetColor();
            break;
        }
        
        std::cout << "  │ " << std::left << std::setw(20) << trans.dateTime.substr(0, 16)
                  << std::setw(15) << trans.type;
        
        if (trans.type.find("OUT") != std::string::npos || trans.type == "WITHDRAWAL") {
            setColor(RED);
        } else if (trans.type.find("IN") != std::string::npos) {
            setColor(GREEN);
        }
        
        std::cout << "$" << std::setw(11) << std::fixed << std::setprecision(2) << trans.amount;
        resetColor();
        
        std::cout << "$" << std::setw(9) << trans.fee
                  << std::setw(15) << trans.reference << " │\n";
    }
    
    setColor(BOLD_CYAN);
    std::cout << "  └────────────────────────────────────────────────────────────────┘\n";
    resetColor();
    
    std::cout << "\n  📊 Statistics:\n";
    std::cout << "  " << std::string(40, '─') << "\n";
    std::cout << "  Total Deposited/Received: ";
    printMoney(totalDeposited);
    std::cout << "\n  Total Withdrawn/Sent: ";
    printMoney(totalWithdrawn);
    std::cout << "\n  Total Fees Paid: ";
    printMoney(totalFees);
    std::cout << "\n  Net Flow: ";
    printMoney(totalDeposited - totalWithdrawn - totalFees);
    std::cout << "\n";
}

void changePIN() {
    printHeader("CHANGE PIN");
    
    std::string oldPIN, newPIN, confirmPIN;
    
    std::cout << "  Enter current PIN: ";
    std::cin >> oldPIN;
    
    if (decodeString(currentUser->pin) != oldPIN) {
        printError("Incorrect current PIN!");
        return;
    }
    
    std::cout << "  Enter new PIN (4-6 digits): ";
    std::cin >> newPIN;
    
    if (!validatePIN(newPIN)) {
        printError("Invalid PIN format! PIN must be 4-6 digits.");
        return;
    }
    
    if (newPIN == oldPIN) {
        printError("New PIN cannot be same as old PIN!");
        return;
    }
    
    std::cout << "  Confirm new PIN: ";
    std::cin >> confirmPIN;
    
    if (newPIN != confirmPIN) {
        printError("PINs do not match!");
        return;
    }
    
    currentUser->pin = encodeString(newPIN);
    saveData();
    
    logTransaction(currentUser->cardNumber, "PIN_CHANGE", 0.0, 0.0, "SUCCESS");
    
    printSuccess("PIN changed successfully!");
    printWarning("Please remember your new PIN for future transactions.");
}

void viewNotifications() {
    printHeader("NOTIFICATIONS");
    
    bool hasNotifications = false;
    for (auto& notif : notifications) {
        if (notif.recipientCard.empty() || notif.recipientCard == currentUser->cardNumber) {
            hasNotifications = true;
            std::cout << (notif.isRead ? "  📧 " : "  📫 ");
            setColor(notif.isRead ? GRAY : BOLD_YELLOW);
            std::cout << "[" << notif.dateTime << "] ";
            std::cout << notif.type << ": ";
            std::cout << notif.message;
            resetColor();
            std::cout << "\n";
            
            if (!notif.isRead && notif.recipientCard == currentUser->cardNumber) {
                notif.isRead = true;
            }
        }
    }
    
    if (!hasNotifications) {
        printInfo("No notifications.");
    }
    
    saveData();
}

void updateProfile() {
    printHeader("UPDATE PROFILE");
    
    std::cout << "  Current Information:\n";
    std::cout << "  Name: " << currentUser->name << "\n";
    std::cout << "  Email: " << currentUser->email << "\n";
    std::cout << "  Phone: " << currentUser->phone << "\n\n";
    
    std::cout << "  Enter new email (or press Enter to skip): ";
    std::cin.ignore();
    std::string newEmail;
    std::getline(std::cin, newEmail);
    
    if (!newEmail.empty()) {
        if (validateEmail(newEmail)) {
            currentUser->email = newEmail;
            printSuccess("Email updated!");
        } else {
            printError("Invalid email format!");
        }
    }
    
    std::cout << "  Enter new phone (10 digits, or press Enter to skip): ";
    std::string newPhone;
    std::getline(std::cin, newPhone);
    
    if (!newPhone.empty()) {
        if (validatePhone(newPhone)) {
            currentUser->phone = newPhone;
            printSuccess("Phone updated!");
        } else {
            printError("Invalid phone number!");
        }
    }
    
    saveData();
}

void logout() {
    if (currentUser != nullptr) {
        printSuccess("Goodbye, " + currentUser->name + "!");
        currentUser = nullptr;
    } else if (currentAdmin != nullptr) {
        printSuccess("Admin logged out. Goodbye, " + currentAdmin->username + "!");
        currentAdmin = nullptr;
    }
    saveData();
    sleepMs(1000);
}

void clientMenu() {
    while (currentUser != nullptr) {
        printHeader("CLIENT DASHBOARD");
        
        displayAccountCard(*currentUser);
        std::cout << "\n";
        
        std::vector<std::pair<std::string, std::string>> options = {
            {"💰", "Check Balance"},
            {"💵", "Withdraw Cash"},
            {"↔️", "Transfer Funds"},
            {"📋", "Transaction History"},
            {"🔐", "Change PIN"},
            {"📨", "View Notifications"},
            {"👤", "Update Profile"},
            {"🚪", "Logout"}
        };
        
        std::string footer = "Last Login: " + currentUser->lastLogin;
        
        // Check for unread notifications
        int unreadCount = 0;
        for (const auto& notif : notifications) {
            if (!notif.isRead && (notif.recipientCard.empty() || 
                notif.recipientCard == currentUser->cardNumber)) {
                unreadCount++;
            }
        }
        if (unreadCount > 0) {
            footer += " | 📫 " + std::to_string(unreadCount) + " unread";
        }
        
        displayMenu(options, "MAIN MENU", footer);
        
        int choice;
        std::cin >> choice;
        
        switch (choice) {
            case 1: checkBalance(); break;
            case 2: withdraw(); break;
            case 3: transferFunds(); break;
            case 4: viewTransactionHistory(); break;
            case 5: changePIN(); break;
            case 6: viewNotifications(); break;
            case 7: updateProfile(); break;
            case 8: 
                logout();
                return;
            default:
                printError("Invalid choice!");
        }
        
        if (currentUser != nullptr && choice != 8) {
            std::cout << "\n  Press any key to continue...";
            getCh();
        }
    }
}

void viewATMCash() {
    printHeader("ATM CASH STATUS");
    displayATMCashPretty();
}

void refillCash() {
    printHeader("REFILL CASH");
    
    if (!checkMaintenanceWindow() && currentAdmin->role != "super") {
        printWarning("Cash refill can only be done during maintenance hours (2 AM - 4 AM)!");
        printInfo("Super admins can bypass this restriction.");
        
        if (currentAdmin->role != "super") {
            return;
        }
    }
    
    std::cout << "  Current ATM Cash Status:\n\n";
    displayATMCashPretty();
    
    std::cout << "\n  Enter new counts for each denomination:\n";
    std::cout << "  $100 bills: ";
    std::cin >> atmCash.bills100;
    std::cout << "  $50 bills: ";
    std::cin >> atmCash.bills50;
    std::cout << "  $20 bills: ";
    std::cin >> atmCash.bills20;
    std::cout << "  $10 bills: ";
    std::cin >> atmCash.bills10;
    std::cout << "  $5 bills: ";
    std::cin >> atmCash.bills5;
    std::cout << "  $1 bills: ";
    std::cin >> atmCash.bills1;
    
    atmCash.lastRefill = getCurrentDateTime();
    saveData();
    
    printSuccess("ATM cash refilled successfully!");
    
    double newTotal = atmCash.getTotalCash();
    printInfo("New total cash: $" + std::to_string(newTotal));
    
    logTransaction("ADMIN", "CASH_REFILL", newTotal, 0.0, "SUCCESS");
}

void createAccount() {
    printHeader("CREATE NEW ACCOUNT");
    
    Account newAccount;
    
    std::cout << "  Enter Card Number (16 digits): ";
    std::cin >> newAccount.cardNumber;
    
    if (!validateCardNumber(newAccount.cardNumber)) {
        printError("Invalid card number format!");
        return;
    }
    
    for (const auto& acc : accounts) {
        if (acc.cardNumber == newAccount.cardNumber) {
            printError("Card number already exists!");
            return;
        }
    }
    
    std::string pin;
    std::cout << "  Enter PIN (4-6 digits): ";
    std::cin >> pin;
    
    if (!validatePIN(pin)) {
        printError("Invalid PIN format!");
        return;
    }
    newAccount.pin = encodeString(pin);
    
    std::cout << "  Enter Account Holder Name: ";
    std::cin.ignore();
    std::getline(std::cin, newAccount.name);
    
    std::cout << "  Enter Email Address: ";
    std::getline(std::cin, newAccount.email);
    
    if (!validateEmail(newAccount.email) && !newAccount.email.empty()) {
        printWarning("Email format may be invalid, but will be saved.");
    }
    
    std::cout << "  Enter Phone Number (10 digits): ";
    std::getline(std::cin, newAccount.phone);
    
    if (!validatePhone(newAccount.phone) && !newAccount.phone.empty()) {
        printWarning("Phone format may be invalid, but will be saved.");
    }
    
    std::cout << "  Enter Initial Deposit: $";
    std::cin >> newAccount.balance;
    
    if (newAccount.balance < 0) {
        printError("Initial deposit cannot be negative!");
        return;
    }
    
    newAccount.isActive = true;
    newAccount.isLocked = false;
    newAccount.failedAttempts = 0;
    newAccount.createdAt = getCurrentDateTime();
    newAccount.lastLogin = "Never";
    
    accounts.push_back(newAccount);
    saveData();
    
    printSuccess("Account created successfully!");
    
    std::cout << "\n  Account Summary:\n";
    displayAccountCard(newAccount);
    
    logTransaction(newAccount.cardNumber, "ACCOUNT_CREATED", newAccount.balance, 0.0, "SUCCESS");
}

void viewAccounts() {
    printHeader("ALL ACCOUNTS");
    
    if (accounts.empty()) {
        printInfo("No accounts found.");
        return;
    }
    
    double totalBalance = 0;
    int activeCount = 0;
    int lockedCount = 0;
    
    for (const auto& acc : accounts) {
        totalBalance += acc.balance;
        if (acc.isActive) activeCount++;
        if (acc.isLocked) lockedCount++;
    }
    
    setColor(BOLD_CYAN);
    std::cout << "  ┌─────────────────────────────────────────────────────────────────────┐\n";
    std::cout << "  │ " << std::left << std::setw(19) << "Card Number" 
              << std::setw(22) << "Name" 
              << std::setw(12) << "Balance" 
              << std::setw(10) << "Status" 
              << "Last Login     │\n";
    std::cout << "  ├─────────────────────────────────────────────────────────────────────┤\n";
    resetColor();
    
    for (const auto& acc : accounts) {
        std::string shortName = acc.name.length() > 20 ? acc.name.substr(0, 18) + ".." : acc.name;
        
        std::cout << "  │ " << std::left << std::setw(19) << acc.cardNumber
                  << std::setw(22) << shortName;
        
        setColor(GREEN);
        std::cout << "$" << std::setw(11) << std::fixed << std::setprecision(2) << acc.balance;
        resetColor();
        
        std::string status;
        if (!acc.isActive) {
            status = "INACTIVE";
            setColor(RED);
        } else if (acc.isLocked) {
            status = "LOCKED";
            setColor(RED);
        } else {
            status = "ACTIVE";
            setColor(GREEN);
        }
        std::cout << std::setw(10) << status;
        resetColor();
        
        std::cout << std::setw(15) << (acc.lastLogin.length() > 10 ? acc.lastLogin.substr(0, 10) : acc.lastLogin)
                  << " │\n";
    }
    
    setColor(BOLD_CYAN);
    std::cout << "  └─────────────────────────────────────────────────────────────────────┘\n";
    resetColor();
    
    std::cout << "\n  📊 Account Statistics:\n";
    std::cout << "  " << std::string(40, '─') << "\n";
    std::cout << "  Total Accounts: " << accounts.size() << "\n";
    std::cout << "  Active Accounts: " << activeCount << "\n";
    std::cout << "  Locked Accounts: " << lockedCount << "\n";
    std::cout << "  Total Balance: ";
    printMoney(totalBalance);
    std::cout << "\n  Average Balance: ";
    printMoney(totalBalance / accounts.size());
    std::cout << "\n";
}

void deleteAccount() {
    printHeader("DELETE ACCOUNT");
    
    std::string cardNumber;
    std::cout << "  Enter card number to delete: ";
    std::cin >> cardNumber;
    
    for (auto it = accounts.begin(); it != accounts.end(); ++it) {
        if (it->cardNumber == cardNumber) {
            std::cout << "\n  Account found:\n";
            displayAccountCard(*it);
            
            std::cout << "\n  " << std::string(40, '─') << "\n";
            std::cout << "  Confirm deletion? (y/n): ";
            char confirm;
            std::cin >> confirm;
            
            if (confirm == 'y' || confirm == 'Y') {
                logTransaction(cardNumber, "ACCOUNT_DELETED", it->balance, 0.0, "SUCCESS");
                accounts.erase(it);
                saveData();
                printSuccess("Account deleted successfully!");
            } else {
                printWarning("Deletion cancelled.");
            }
            return;
        }
    }
    
    printError("Account not found!");
}

void resetUserPIN() {
    printHeader("RESET USER PIN");
    
    std::string cardNumber;
    std::cout << "  Enter card number: ";
    std::cin >> cardNumber;
    
    for (auto& account : accounts) {
        if (account.cardNumber == cardNumber) {
            std::cout << "  Account: " << account.name << "\n";
            std::cout << "  Current Status: " << (account.isLocked ? "LOCKED" : "Active") << "\n\n";
            
            std::string newPIN;
            std::cout << "  Enter new PIN (4-6 digits): ";
            std::cin >> newPIN;
            
            if (!validatePIN(newPIN)) {
                printError("Invalid PIN format!");
                return;
            }
            
            std::cout << "  Confirm new PIN: ";
            std::string confirmPIN;
            std::cin >> confirmPIN;
            
            if (newPIN != confirmPIN) {
                printError("PINs do not match!");
                return;
            }
            
            account.pin = encodeString(newPIN);
            account.failedAttempts = 0;
            account.isLocked = false;
            saveData();
            
            logTransaction(cardNumber, "PIN_RESET", 0.0, 0.0, "SUCCESS");
            
            printSuccess("PIN reset successful!");
            printInfo("Account has been unlocked.");
            return;
        }
    }
    
    printError("Account not found!");
}

void changeAdminPasscode() {
    printHeader("CHANGE ADMIN PASSCODE");
    
    std::string oldPass, newPass, confirmPass;
    
    std::cout << "  Enter current passcode: ";
    std::cin >> oldPass;
    
    if (decodeString(currentAdmin->password) != oldPass) {
        printError("Incorrect current passcode!");
        return;
    }
    
    std::cout << "  Enter new passcode: ";
    std::cin >> newPass;
    
    if (newPass.length() < 6) {
        printError("Passcode must be at least 6 characters!");
        return;
    }
    
    std::cout << "  Confirm new passcode: ";
    std::cin >> confirmPass;
    
    if (newPass != confirmPass) {
        printError("Passcodes do not match!");
        return;
    }
    
    currentAdmin->password = encodeString(newPass);
    saveData();
    
    printSuccess("Admin passcode changed successfully!");
}

void viewAdminUsers() {
    printHeader("ADMIN USERS");
    
    setColor(BOLD_CYAN);
    std::cout << "  ┌────────────────────────────────────────────────┐\n";
    std::cout << "  │ " << std::left << std::setw(15) << "Username" 
              << std::setw(15) << "Role" 
              << "Last Login              │\n";
    std::cout << "  ├────────────────────────────────────────────────┤\n";
    resetColor();
    
    for (const auto& admin : admins) {
        std::cout << "  │ " << std::left << std::setw(15) << admin.username 
                  << std::setw(15) << admin.role
                  << admin.lastLogin << " │\n";
    }
    
    setColor(BOLD_CYAN);
    std::cout << "  └────────────────────────────────────────────────┘\n";
    resetColor();
}

void systemStatistics() {
    printHeader("SYSTEM STATISTICS");
    
    time_t now = time(0);
    tm* ltm = localtime(&now);
    
    double totalATMCash = atmCash.getTotalCash();
    double totalUserBalance = 0;
    int activeUsers = 0;
    int lockedUsers = 0;
    int todayTransactions = 0;
    double todayVolume = 0;
    std::string today = getCurrentDate();
    
    for (const auto& acc : accounts) {
        totalUserBalance += acc.balance;
        if (acc.isActive) activeUsers++;
        if (acc.isLocked) lockedUsers++;
    }
    
    for (const auto& trans : transactions) {
        if (trans.dateTime.substr(0, 10) == today) {
            todayTransactions++;
            todayVolume += trans.amount;
        }
    }
    
    setColor(BOLD_CYAN);
    std::cout << "  ┌──────────────────────────────────────────────────┐\n";
    std::cout << "  │             SYSTEM STATISTICS                    │\n";
    std::cout << "  ├──────────────────────────────────────────────────┤\n";
    std::cout << "  │ Date/Time: " << std::left << std::setw(33) << getCurrentDateTime() << " │\n";
    std::cout << "  ├──────────────────────────────────────────────────┤\n";
    std::cout << "  │ ATM Statistics:                                   │\n";
    resetColor();
    printf("  │   Total ATM Cash: $%-33.2f│\n", totalATMCash);
    setColor(CYAN);
    std::cout << "  │   Last Refill: " << std::setw(35) << atmCash.lastRefill << " │\n";
    std::cout << "  │   Last Audit: " << std::setw(36) << atmCash.lastAudit << " │\n";
    std::cout << "  ├──────────────────────────────────────────────────┤\n";
    std::cout << "  │ Account Statistics:                               │\n";
    std::cout << "  │   Total Accounts: " << std::setw(31) << accounts.size() << " │\n";
    std::cout << "  │   Active Users: " << std::setw(33) << activeUsers << " │\n";
    std::cout << "  │   Locked Users: " << std::setw(33) << lockedUsers << " │\n";
    resetColor();
    printf("  │   Total User Balance: $%-27.2f│\n", totalUserBalance);
    setColor(CYAN);
    std::cout << "  ├──────────────────────────────────────────────────┤\n";
    std::cout << "  │ Transaction Statistics:                           │\n";
    std::cout << "  │   Today's Transactions: " << std::setw(25) << todayTransactions << " │\n";
    resetColor();
    printf("  │   Today's Volume: $%-31.2f│\n", todayVolume);
    setColor(CYAN);
    std::cout << "  │   Total Transactions: " << std::setw(27) << transactions.size() << " │\n";
    std::cout << "  └──────────────────────────────────────────────────┘\n";
    resetColor();
    
    atmCash.lastAudit = getCurrentDateTime();
    saveData();
}

void adminMenu() {
    while (currentAdmin != nullptr) {
        printHeader("ADMIN CONTROL PANEL");
        
        std::cout << "  Logged in as: " << BOLD << currentAdmin->username << RESET;
        setColor(GRAY);
        std::cout << " (Role: " << currentAdmin->role << ")\n\n";
        resetColor();
        
        std::vector<std::pair<std::string, std::string>> options = {
            {"💰", "View ATM Cash Status"},
            {"💳", "Refill Cash"},
            {"➕", "Create New Account"},
            {"📋", "View All Accounts"},
            {"❌", "Delete Account"},
            {"🔄", "Reset User PIN"},
            {"🔑", "Change Admin Passcode"},
            {"👥", "View Admin Users"},
            {"📊", "System Statistics"},
            {"🚪", "Logout"}
        };
        
        std::string footer = "Total Accounts: " + std::to_string(accounts.size()) + 
                            " | ATM Cash: $" + std::to_string(atmCash.getTotalCash());
        
        displayMenu(options, "ADMIN MENU", footer);
        
        int choice;
        std::cin >> choice;
        
        switch (choice) {
            case 1: viewATMCash(); break;
            case 2: refillCash(); break;
            case 3: createAccount(); break;
            case 4: viewAccounts(); break;
            case 5: deleteAccount(); break;
            case 6: resetUserPIN(); break;
            case 7: changeAdminPasscode(); break;
            case 8: viewAdminUsers(); break;
            case 9: systemStatistics(); break;
            case 10: 
                logout();
                return;
            default:
                printError("Invalid choice!");
        }
        
        if (currentAdmin != nullptr && choice != 10) {
            std::cout << "\n  Press any key to continue...";
            getCh();
        }
    }
}

int main() {
    setConsoleTitle("ATM Banking System");
    
    loadData();
    loadTransactions();
    
    // Create default admins if none exist
    if (admins.empty()) {
        // Default admin
        Admin defaultAdmin;
        defaultAdmin.username = "admin";
        defaultAdmin.password = encodeString("admin123");
        defaultAdmin.lastLogin = "Never";
        defaultAdmin.role = "super";
        admins.push_back(defaultAdmin);
        
        // Custom admin as requested
        Admin customAdmin;
        customAdmin.username = "admin6767";
        customAdmin.password = encodeString("67676767");
        customAdmin.lastLogin = "Never";
        customAdmin.role = "super";
        admins.push_back(customAdmin);
        
        saveData();
    }
    
    // Create sample account if no accounts exist
    if (accounts.empty()) {
        Account sampleAccount;
        sampleAccount.cardNumber = "1234567890123456";
        sampleAccount.pin = encodeString("1234");
        sampleAccount.name = "John Doe";
        sampleAccount.balance = 5000.00;
        sampleAccount.isActive = true;
        sampleAccount.isLocked = false;
        sampleAccount.failedAttempts = 0;
        sampleAccount.createdAt = getCurrentDateTime();
        sampleAccount.lastLogin = "Never";
        sampleAccount.email = "john.doe@email.com";
        sampleAccount.phone = "9876543210";
        
        accounts.push_back(sampleAccount);
        saveData();
    }
    
    // Welcome screen with ASCII art
    clearScreen();
    int width = getConsoleWidth();
    
    setColor(BOLD_CYAN);
    std::vector<std::string> welcomeArt = {
        "  ╔══════════════════════════════════════════════════════════════╗",
        "  ║                                                              ║",
        "  ║     ████████╗██╗  ██╗███████╗     █████╗ ████████╗███╗   ███╗  ║",
        "  ║     ╚══██╔══╝██║  ██║██╔════╝    ██╔══██╗╚══██╔══╝████╗ ████║  ║",
        "  ║        ██║   ███████║█████╗      ███████║   ██║   ██╔████╔██║  ║",
        "  ║        ██║   ██╔══██║██╔══╝      ██╔══██║   ██║   ██║╚██╔╝██║  ║",
        "  ║        ██║   ██║  ██║███████╗    ██║  ██║   ██║   ██║ ╚═╝ ██║  ║",
        "  ║        ╚═╝   ╚═╝  ╚═╝╚══════╝    ╚═╝  ╚═╝   ╚═╝   ╚═╝     ╚═╝  ║",
        "  ║                                                              ║",
        "  ╚══════════════════════════════════════════════════════════════╝"
    };
    
    for (const auto& line : welcomeArt) {
        printCentered(line);
        std::cout << "\n";
    }
    resetColor();
    
    std::cout << "\n";
    setColor(BOLD_YELLOW);
    printCentered("Version 3.0 | Ultimate Secure Banking System");
    std::cout << "\n";
    setColor(GRAY);
    printCentered("© 2025 ATM Corporation. All rights reserved.");
    std::cout << "\n\n";
    resetColor();
    
    // Display login information box
    std::vector<std::string> loginInfo;
    loginInfo.push_back("Admin Login: admin6767 / 67676767");
    loginInfo.push_back("Sample Account: 1234567890123456 / 1234");
    loginInfo.push_back("Balance: $5000.00");
    printBox(loginInfo, "LOGIN INFORMATION");
    
    std::cout << "\n";
    loadingAnimation("Initializing system", 2);
    
    // Main program loop
    while (true) {
        if (login()) {
            if (currentUser != nullptr) {
                clientMenu();
            } else if (currentAdmin != nullptr) {
                adminMenu();
            }
        }
        
        std::cout << "\n  Press any key to continue...";
        getCh();
    }
    
    return 0;
}

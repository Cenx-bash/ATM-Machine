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

// ANSI color codes for Linux/Mac
#define RESET   "\033[0m"
#define RED     "\033[31m"
#define GREEN   "\033[32m"
#define YELLOW  "\033[33m"
#define BLUE    "\033[34m"
#define MAGENTA "\033[35m"
#define CYAN    "\033[36m"
#define WHITE   "\033[37m"
#define GRAY    "\033[90m"
#define BOLD    "\033[1m"

// Forward declarations
struct Account;
struct Transaction;
struct Admin;
struct ATM_Cash;
struct Notification;

// Function declarations (move these up)
std::string getCurrentDateTime();
std::string getCurrentDate();
std::string generateReference();
void setColor(const char* color);
void resetColor();
void clearScreen();
char getCh();
int getConsoleWidth();
void setConsoleTitle(const std::string& title);
void sleepMs(int milliseconds);
void printCentered(const std::string& text);
void printLine(char ch = '=', int length = 0);
void printHeader(const std::string& title);
void printSuccess(const std::string& message);
void printError(const std::string& message);
void printWarning(const std::string& message);
void printInfo(const std::string& message);
void printMoney(double amount);
void loadingAnimation(const std::string& message, int seconds = 1);
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
void setColor(const char* color) {
    std::cout << color;
}

void resetColor() {
    std::cout << RESET;
}

void clearScreen() {
    system("clear");
}

// Linux getch() implementation
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

// Get console width for centering text
int getConsoleWidth() {
    struct winsize w;
    int width = 80;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) == 0) {
        width = w.ws_col;
    }
    return width;
}

// Set console title
void setConsoleTitle(const std::string& title) {
    std::cout << "\033]0;" << title << "\007";
}

// Cross-platform sleep
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
    for (int i = 0; i < length; i++) {
        std::cout << ch;
    }
}

void printHeader(const std::string& title) {
    clearScreen();
    setColor(CYAN);
    printLine('=', 80);
    std::cout << "\n";
    printCentered("🏦 " + title + " 🏦");
    std::cout << "\n";
    printLine('=', 80);
    resetColor();
    std::cout << "\n\n";
}

void printSuccess(const std::string& message) {
    setColor(GREEN);
    std::cout << "✅ " << message << "\n";
    resetColor();
}

void printError(const std::string& message) {
    setColor(RED);
    std::cout << "❌ " << message << "\n";
    resetColor();
}

void printWarning(const std::string& message) {
    setColor(YELLOW);
    std::cout << "⚠️  " << message << "\n";
    resetColor();
}

void printInfo(const std::string& message) {
    setColor(BLUE);
    std::cout << "ℹ️  " << message << "\n";
    resetColor();
}

void printMoney(double amount) {
    setColor(GREEN);
    std::cout << "$" << std::fixed << std::setprecision(2) << amount;
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
    std::getline(file, line);
    
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

void displayProgressBar(int percentage, int width) {
    int filled = (percentage * width) / 100;
    std::cout << "[";
    setColor(GREEN);
    for (int i = 0; i < filled; i++) std::cout << "█";
    resetColor();
    for (int i = filled; i < width; i++) std::cout << "░";
    std::cout << "] " << percentage << "%";
}

void displayReceipt(const std::map<std::string, std::string>& details) {
    printHeader("TRANSACTION RECEIPT");
    printLine('-', 50);
    std::cout << "\n";
    for (const auto& pair : details) {
        std::cout << std::left << std::setw(20) << pair.first << ": " << pair.second << "\n";
    }
    printLine('-', 50);
    std::cout << "\nThank you for banking with us!\n";
}

bool login() {
    printHeader("ATM SECURE LOGIN");
    
    std::cout << "\n";
    printCentered("🔐 Please select login type");
    std::cout << "\n\n";
    
    std::vector<std::string> options = {
        "👤 Client Login",
        "🛠️  Admin Login",
        "🚪 Exit System"
    };
    
    for (size_t i = 0; i < options.size(); i++) {
        setColor(CYAN);
        std::cout << "  " << (i + 1) << ". ";
        resetColor();
        std::cout << options[i] << "\n";
    }
    
    std::cout << "\n" << std::string(50, '-') << "\n";
    std::cout << "Enter choice: ";
    
    int choice;
    std::cin >> choice;
    
    if (choice == 3) {
        printHeader("GOODBYE");
        printCentered("Thank you for using our ATM system!");
        std::cout << "\n\n";
        printCentered("👋 Have a great day!");
        std::cout << "\n\n";
        exit(0);
    }
    
    std::cout << "\n";
    std::string username, password;
    
    setColor(BLUE);
    std::cout << "┌─ Enter Username/Card Number: ";
    resetColor();
    std::cin >> username;
    
    setColor(BLUE);
    std::cout << "├─ Enter PIN/Password: ";
    resetColor();
    
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
    
    std::cout << "\n";
    std::cout << "┌─ " << std::string(46, '-') << "┐\n";
    std::cout << "│  " << std::left << std::setw(20) << "Account Holder:" 
              << std::setw(25) << currentUser->name << " │\n";
    std::cout << "│  " << std::left << std::setw(20) << "Card Number:" 
              << std::setw(25) << currentUser->cardNumber << " │\n";
    std::cout << "│  " << std::left << std::setw(20) << "Account Status:" 
              << std::setw(25) << (currentUser->isActive ? "Active ✅" : "Inactive ❌") << " │\n";
    std::cout << "│  " << std::left << std::setw(20) << "Current Balance:" 
              << std::setw(25);
    printMoney(currentUser->balance);
    std::cout << " │\n";
    std::cout << "│  " << std::left << std::setw(20) << "Date/Time:" 
              << std::setw(25) << getCurrentDateTime() << " │\n";
    std::cout << "└─ " << std::string(46, '-') << "┘\n";
    
    logTransaction(currentUser->cardNumber, "BALANCE_INQUIRY", 
                   currentUser->balance, 0.0, "SUCCESS");
    
    std::cout << "\nAccount Health:\n";
    int healthPercent = std::min(100, static_cast<int>((currentUser->balance / 10000) * 100));
    displayProgressBar(healthPercent, 50);
    std::cout << "\n";
}

void withdraw() {
    printHeader("CASH WITHDRAWAL");
    
    std::cout << "\nCurrent Balance: ";
    printMoney(currentUser->balance);
    std::cout << "\n\n";
    
    std::cout << "Available Denominations:\n";
    setColor(GRAY);
    std::cout << "  $100  $50  $20  $10  $5  $1\n\n";
    resetColor();
    
    std::vector<double> quickAmounts = {20, 40, 60, 80, 100, 200, 500};
    std::cout << "Quick Amounts: ";
    for (size_t i = 0; i < quickAmounts.size(); i++) {
        setColor(CYAN);
        std::cout << "[" << (i + 1) << "]";
        resetColor();
        std::cout << "$" << quickAmounts[i] << " ";
    }
    std::cout << "\n\n";
    
    double amount;
    std::cout << "Enter amount to withdraw: $";
    std::cin >> amount;
    
    if (amount >= 1 && amount <= 7 && amount == static_cast<int>(amount)) {
        amount = quickAmounts[static_cast<int>(amount) - 1];
        std::cout << "Selected quick amount: $" << amount << "\n";
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
        printError("Insufficient balance to cover fee of " + 
                   std::to_string(fee));
        return;
    }
    
    int hundreds = 0, fifties = 0, twenties = 0, tens = 0, fives = 0, ones = 0;
    
    loadingAnimation("Calculating optimal denomination", 1);
    
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
    
    std::cout << "\n" << std::string(50, '-') << "\n";
    std::cout << "WITHDRAWAL SUMMARY\n";
    std::cout << std::string(50, '-') << "\n";
    std::cout << "Amount:        ";
    printMoney(amount);
    std::cout << "\nFee:           ";
    printMoney(fee);
    std::cout << "\nTotal:         ";
    printMoney(totalDeduction);
    std::cout << "\nNew Balance:   ";
    printMoney(currentUser->balance - totalDeduction);
    std::cout << "\n\nBills Dispensed:\n";
    
    if (hundreds > 0) std::cout << "  $100 x " << hundreds << "\n";
    if (fifties > 0) std::cout << "  $50  x " << fifties << "\n";
    if (twenties > 0) std::cout << "  $20  x " << twenties << "\n";
    if (tens > 0) std::cout << "  $10  x " << tens << "\n";
    if (fives > 0) std::cout << "  $5   x " << fives << "\n";
    if (ones > 0) std::cout << "  $1   x " << ones << "\n";
    
    std::cout << "\n" << std::string(50, '-') << "\n";
    std::cout << "Confirm withdrawal? (y/n): ";
    
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
    
    std::cout << "\nYour Balance: ";
    printMoney(currentUser->balance);
    std::cout << "\n\n";
    
    std::string recipientCard;
    std::cout << "Enter recipient's card number: ";
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
    
    std::cout << "\nRecipient: " << maskedName << "\n";
    std::cout << "Card: " << maskedCard << "\n\n";
    
    double amount;
    std::cout << "Enter amount to transfer: $";
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
    
    std::cout << "\n" << std::string(50, '-') << "\n";
    std::cout << "TRANSFER SUMMARY\n";
    std::cout << std::string(50, '-') << "\n";
    std::cout << "Amount:        ";
    printMoney(amount);
    std::cout << "\nFee:           ";
    printMoney(fee);
    std::cout << "\nTotal:         ";
    printMoney(totalDeduction);
    std::cout << "\nNew Balance:   ";
    printMoney(currentUser->balance - totalDeduction);
    std::cout << "\n\n" << std::string(50, '-') << "\n";
    
    std::cout << "Confirm transfer? (y/n): ";
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
    
    std::cout << "Account: " << currentUser->name << "\n";
    std::cout << "Card: " << currentUser->cardNumber << "\n\n";
    
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
    
    std::cout << "+" << std::string(78, '=') << "+\n";
    std::cout << "| " << std::left << std::setw(20) << "Date/Time" 
              << std::setw(20) << "Type" 
              << std::setw(12) << "Amount" 
              << std::setw(10) << "Fee" 
              << std::setw(14) << "Reference" << " |\n";
    std::cout << "+" << std::string(78, '=') << "+\n";
    
    int count = 0;
    for (const auto& trans : userTransactions) {
        if (count++ >= 20) {
            std::cout << "| " << std::string(76, ' ') << " |\n";
            std::cout << "| " << std::left << std::setw(76) 
                      << "... and " + std::to_string(userTransactions.size() - 20) + " more transactions" << " |\n";
            break;
        }
        
        std::cout << "| " << std::left << std::setw(20) << trans.dateTime.substr(0, 19)
                  << std::setw(20) << trans.type;
        
        if (trans.type.find("OUT") != std::string::npos || trans.type == "WITHDRAWAL") {
            setColor(RED);
        } else if (trans.type.find("IN") != std::string::npos) {
            setColor(GREEN);
        }
        
        std::cout << "$" << std::setw(11) << std::fixed << std::setprecision(2) << trans.amount;
        resetColor();
        
        std::cout << "$" << std::setw(9) << trans.fee
                  << std::setw(14) << trans.reference.substr(0, 10) << " |\n";
    }
    
    std::cout << "+" << std::string(78, '=') << "+\n";
    
    std::cout << "\nStatistics:\n";
    std::cout << std::string(40, '-') << "\n";
    std::cout << "Total Deposited/Received: ";
    printMoney(totalDeposited);
    std::cout << "\nTotal Withdrawn/Sent: ";
    printMoney(totalWithdrawn);
    std::cout << "\nTotal Fees Paid: ";
    printMoney(totalFees);
    std::cout << "\nNet Flow: ";
    printMoney(totalDeposited - totalWithdrawn - totalFees);
    std::cout << "\n";
}

void changePIN() {
    printHeader("CHANGE PIN");
    
    std::string oldPIN, newPIN, confirmPIN;
    
    std::cout << "Enter current PIN: ";
    std::cin >> oldPIN;
    
    if (decodeString(currentUser->pin) != oldPIN) {
        printError("Incorrect current PIN!");
        return;
    }
    
    std::cout << "Enter new PIN (4-6 digits): ";
    std::cin >> newPIN;
    
    if (!validatePIN(newPIN)) {
        printError("Invalid PIN format! PIN must be 4-6 digits.");
        return;
    }
    
    if (newPIN == oldPIN) {
        printError("New PIN cannot be same as old PIN!");
        return;
    }
    
    std::cout << "Confirm new PIN: ";
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
            std::cout << (notif.isRead ? "📧 " : "📫 ");
            setColor(notif.isRead ? GRAY : YELLOW);
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
    
    std::cout << "Current Information:\n";
    std::cout << "Name: " << currentUser->name << "\n";
    std::cout << "Email: " << currentUser->email << "\n";
    std::cout << "Phone: " << currentUser->phone << "\n\n";
    
    std::cout << "Enter new email (or press Enter to skip): ";
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
    
    std::cout << "Enter new phone (10 digits, or press Enter to skip): ";
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
        
        setColor(CYAN);
        std::cout << "+----------------------------------------------------+\n";
        std::cout << "| ";
        resetColor();
        std::cout << std::left << std::setw(25) << "Welcome, " + currentUser->name;
        setColor(GREEN);
        std::cout << std::right << std::setw(20) << "Balance: $" + std::to_string(currentUser->balance);
        resetColor();
        std::cout << " |\n";
        setColor(CYAN);
        std::cout << "+----------------------------------------------------+\n\n";
        resetColor();
        
        std::vector<std::pair<std::string, std::string>> options = {
            {"💰", "Check Balance"},
            {"💵", "Withdraw Cash"},
            {"↔️", "Transfer Funds"},
            {"📋", "View Transaction History"},
            {"🔐", "Change PIN"},
            {"📨", "View Notifications"},
            {"👤", "Update Profile"},
            {"🚪", "Logout"}
        };
        
        for (size_t i = 0; i < options.size(); i++) {
            setColor(CYAN);
            std::cout << "  " << (i + 1) << ". ";
            resetColor();
            std::cout << options[i].first << "  " << options[i].second;
            
            if (i == 5) {
                int unreadCount = 0;
                for (const auto& notif : notifications) {
                    if (!notif.isRead && (notif.recipientCard.empty() || 
                        notif.recipientCard == currentUser->cardNumber)) {
                        unreadCount++;
                    }
                }
                if (unreadCount > 0) {
                    setColor(YELLOW);
                    std::cout << " 🔔 " << unreadCount;
                    resetColor();
                }
            }
            std::cout << "\n";
        }
        
        std::cout << "\n" << std::string(50, '-') << "\n";
        std::cout << "Enter choice: ";
        
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
            std::cout << "\nPress any key to continue...";
            getCh();
        }
    }
}

void viewATMCash() {
    printHeader("ATM CASH STATUS");
    
    double total = atmCash.getTotalCash();
    
    std::cout << "+--------------------------------------------------+\n";
    std::cout << "|              ATM CASH INVENTORY                   |\n";
    std::cout << "+--------------------------------------------------+\n";
    std::cout << "| Denomination  | Count    | Value                 |\n";
    std::cout << "+--------------------------------------------------+\n";
    std::cout << "| $100          | " << std::setw(7) << atmCash.bills100 
              << " | $" << std::setw(20) << std::left << std::to_string(atmCash.bills100 * 100) << " |\n";
    std::cout << "| $50           | " << std::setw(7) << atmCash.bills50 
              << " | $" << std::setw(20) << (atmCash.bills50 * 50) << " |\n";
    std::cout << "| $20           | " << std::setw(7) << atmCash.bills20 
              << " | $" << std::setw(20) << (atmCash.bills20 * 20) << " |\n";
    std::cout << "| $10           | " << std::setw(7) << atmCash.bills10 
              << " | $" << std::setw(20) << (atmCash.bills10 * 10) << " |\n";
    std::cout << "| $5            | " << std::setw(7) << atmCash.bills5 
              << " | $" << std::setw(20) << (atmCash.bills5 * 5) << " |\n";
    std::cout << "| $1            | " << std::setw(7) << atmCash.bills1 
              << " | $" << std::setw(20) << (atmCash.bills1 * 1) << " |\n";
    std::cout << "+--------------------------------------------------+\n";
    std::cout << "| TOTAL CASH:   |         | $" << std::setw(20) << std::left << total << " |\n";
    std::cout << "+--------------------------------------------------+\n";
    
    std::cout << "\nLast Refill: " << atmCash.lastRefill << "\n";
    std::cout << "Last Audit: " << atmCash.lastAudit << "\n";
    
    if (total < 10000) {
        printWarning("⚠️  ATM cash level is LOW!");
        printInfo("Recommended refill amount: $" + std::to_string(20000 - total));
    }
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
    
    std::cout << "Current ATM Cash Status:\n\n";
    viewATMCash();
    
    std::cout << "\nEnter new counts for each denomination:\n";
    std::cout << "$100 bills: ";
    std::cin >> atmCash.bills100;
    std::cout << "$50 bills: ";
    std::cin >> atmCash.bills50;
    std::cout << "$20 bills: ";
    std::cin >> atmCash.bills20;
    std::cout << "$10 bills: ";
    std::cin >> atmCash.bills10;
    std::cout << "$5 bills: ";
    std::cin >> atmCash.bills5;
    std::cout << "$1 bills: ";
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
    
    std::cout << "Enter Card Number (16 digits): ";
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
    std::cout << "Enter PIN (4-6 digits): ";
    std::cin >> pin;
    
    if (!validatePIN(pin)) {
        printError("Invalid PIN format!");
        return;
    }
    newAccount.pin = encodeString(pin);
    
    std::cout << "Enter Account Holder Name: ";
    std::cin.ignore();
    std::getline(std::cin, newAccount.name);
    
    std::cout << "Enter Email Address: ";
    std::getline(std::cin, newAccount.email);
    
    if (!validateEmail(newAccount.email)) {
        printWarning("Email format may be invalid, but will be saved.");
    }
    
    std::cout << "Enter Phone Number (10 digits): ";
    std::getline(std::cin, newAccount.phone);
    
    std::cout << "Enter Initial Deposit: $";
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
    
    std::cout << "\nAccount Summary:\n";
    std::cout << "Card Number: " << newAccount.cardNumber << "\n";
    std::cout << "Name: " << newAccount.name << "\n";
    std::cout << "Initial Balance: ";
    printMoney(newAccount.balance);
    std::cout << "\nCreated: " << newAccount.createdAt << "\n";
    
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
    
    std::cout << "+--------------------------------------------------+\n";
    std::cout << "| " << std::left << std::setw(19) << "Card Number" 
              << std::setw(25) << "Name" 
              << std::setw(12) << "Balance" 
              << std::setw(10) << "Status" 
              << "Last Login |\n";
    std::cout << "+--------------------------------------------------+\n";
    
    for (const auto& acc : accounts) {
        std::cout << "| " << std::left << std::setw(19) << acc.cardNumber
                  << std::setw(25) << (acc.name.length() > 24 ? acc.name.substr(0, 22) + ".." : acc.name);
        
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
        
        std::cout << std::setw(14) << (acc.lastLogin.length() > 10 ? acc.lastLogin.substr(0, 10) : acc.lastLogin)
                  << " |\n";
    }
    
    std::cout << "+--------------------------------------------------+\n";
    
    std::cout << "\n📊 Account Statistics:\n";
    std::cout << std::string(40, '-') << "\n";
    std::cout << "Total Accounts: " << accounts.size() << "\n";
    std::cout << "Active Accounts: " << activeCount << "\n";
    std::cout << "Locked Accounts: " << lockedCount << "\n";
    std::cout << "Total Balance: ";
    printMoney(totalBalance);
    std::cout << "\nAverage Balance: ";
    printMoney(totalBalance / accounts.size());
    std::cout << "\n";
}

void deleteAccount() {
    printHeader("DELETE ACCOUNT");
    
    std::string cardNumber;
    std::cout << "Enter card number to delete: ";
    std::cin >> cardNumber;
    
    for (auto it = accounts.begin(); it != accounts.end(); ++it) {
        if (it->cardNumber == cardNumber) {
            std::cout << "\nAccount found:\n";
            std::cout << "Name: " << it->name << "\n";
            std::cout << "Balance: ";
            printMoney(it->balance);
            std::cout << "\nStatus: " << (it->isActive ? "Active" : "Inactive") << "\n";
            std::cout << "Created: " << it->createdAt << "\n";
            
            std::cout << "\n" << std::string(40, '-') << "\n";
            std::cout << "Confirm deletion? (y/n): ";
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
    std::cout << "Enter card number: ";
    std::cin >> cardNumber;
    
    for (auto& account : accounts) {
        if (account.cardNumber == cardNumber) {
            std::cout << "Account: " << account.name << "\n";
            std::cout << "Current Status: " << (account.isLocked ? "LOCKED" : "Active") << "\n\n";
            
            std::string newPIN;
            std::cout << "Enter new PIN (4-6 digits): ";
            std::cin >> newPIN;
            
            if (!validatePIN(newPIN)) {
                printError("Invalid PIN format!");
                return;
            }
            
            std::cout << "Confirm new PIN: ";
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
    
    std::cout << "Enter current passcode: ";
    std::cin >> oldPass;
    
    if (decodeString(currentAdmin->password) != oldPass) {
        printError("Incorrect current passcode!");
        return;
    }
    
    std::cout << "Enter new passcode: ";
    std::cin >> newPass;
    
    if (newPass.length() < 6) {
        printError("Passcode must be at least 6 characters!");
        return;
    }
    
    std::cout << "Confirm new passcode: ";
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
    
    std::cout << "+--------------------------------------------------+\n";
    std::cout << "| " << std::left << std::setw(15) << "Username" 
              << std::setw(15) << "Role" 
              << "Last Login" << " |\n";
    std::cout << "+--------------------------------------------------+\n";
    
    for (const auto& admin : admins) {
        std::cout << "| " << std::left << std::setw(15) << admin.username 
                  << std::setw(15) << admin.role
                  << admin.lastLogin << " |\n";
    }
    
    std::cout << "+--------------------------------------------------+\n";
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
    
    std::cout << "+--------------------------------------------------+\n";
    std::cout << "|              SYSTEM STATISTICS                   |\n";
    std::cout << "+--------------------------------------------------+\n";
    std::cout << "| Date/Time: " << std::left << std::setw(38) << getCurrentDateTime() << " |\n";
    std::cout << "+--------------------------------------------------+\n";
    std::cout << "| ATM Statistics:                                   |\n";
    std::cout << "|   Total ATM Cash: $" << std::setw(33) << std::left << totalATMCash << " |\n";
    std::cout << "|   Last Refill: " << std::setw(37) << atmCash.lastRefill << " |\n";
    std::cout << "|   Last Audit: " << std::setw(38) << atmCash.lastAudit << " |\n";
    std::cout << "+--------------------------------------------------+\n";
    std::cout << "| Account Statistics:                               |\n";
    std::cout << "|   Total Accounts: " << std::setw(33) << accounts.size() << " |\n";
    std::cout << "|   Active Users: " << std::setw(35) << activeUsers << " |\n";
    std::cout << "|   Locked Users: " << std::setw(35) << lockedUsers << " |\n";
    std::cout << "|   Total User Balance: $" << std::setw(28) << totalUserBalance << " |\n";
    std::cout << "+--------------------------------------------------+\n";
    std::cout << "| Transaction Statistics:                           |\n";
    std::cout << "|   Today's Transactions: " << std::setw(27) << todayTransactions << " |\n";
    std::cout << "|   Today's Volume: $" << std::setw(32) << todayVolume << " |\n";
    std::cout << "|   Total Transactions: " << std::setw(29) << transactions.size() << " |\n";
    std::cout << "+--------------------------------------------------+\n";
    
    atmCash.lastAudit = getCurrentDateTime();
    saveData();
}

void adminMenu() {
    while (currentAdmin != nullptr) {
        printHeader("ADMIN CONTROL PANEL");
        
        std::cout << "Logged in as: " << currentAdmin->username;
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
        
        for (size_t i = 0; i < options.size(); i++) {
            setColor(CYAN);
            std::cout << "  " << (i + 1) << ". ";
            resetColor();
            std::cout << options[i].first << "  " << options[i].second << "\n";
        }
        
        std::cout << "\n" << std::string(50, '-') << "\n";
        std::cout << "Enter choice: ";
        
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
            std::cout << "\nPress any key to continue...";
            getCh();
        }
    }
}

int main() {
    setConsoleTitle("ATM Banking System");
    
    loadData();
    loadTransactions();
    
    if (admins.empty()) {
        Admin defaultAdmin;
        defaultAdmin.username = "admin";
        defaultAdmin.password = encodeString("admin123");
        defaultAdmin.lastLogin = "Never";
        defaultAdmin.role = "super";
        admins.push_back(defaultAdmin);
        saveData();
    }
    
    printHeader("ATM BANKING SYSTEM");
    
    std::vector<std::string> welcomeArt = {
        "████████╗██╗  ██╗███████╗     █████╗ ████████╗███╗   ███╗",
        "╚══██╔══╝██║  ██║██╔════╝    ██╔══██╗╚══██╔══╝████╗ ████║",
        "   ██║   ███████║█████╗      ███████║   ██║   ██╔████╔██║",
        "   ██║   ██╔══██║██╔══╝      ██╔══██║   ██║   ██║╚██╔╝██║",
        "   ██║   ██║  ██║███████╗    ██║  ██║   ██║   ██║ ╚═╝ ██║",
        "   ╚═╝   ╚═╝  ╚═╝╚══════╝    ╚═╝  ╚═╝   ╚═╝   ╚═╝     ╚═╝"
    };
    
    setColor(CYAN);
    for (const auto& line : welcomeArt) {
        printCentered(line);
        std::cout << "\n";
    }
    resetColor();
    
    std::cout << "\n";
    printCentered("Version 2.0 | Secure Banking System");
    std::cout << "\n";
    printCentered("© 2024 ATM Corporation. All rights reserved.");
    std::cout << "\n\n";
    
    loadingAnimation("Initializing system", 2);
    
    while (true) {
        if (login()) {
            if (currentUser != nullptr) {
                clientMenu();
            } else if (currentAdmin != nullptr) {
                adminMenu();
            }
        }
        
        std::cout << "\nPress any key to continue...";
        getCh();
    }
    
    return 0;
}

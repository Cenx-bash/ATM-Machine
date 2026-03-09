#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <ctime>
#include <termios.h>
#include <unistd.h>
#include <thread>
#include <chrono>

// Simple color codes
#define RESET   "\033[0m"
#define RED     "\033[31m"
#define GREEN   "\033[32m"
#define YELLOW  "\033[33m"
#define BLUE    "\033[34m"
#define CYAN    "\033[36m"

// Structures
struct Account {
    std::string cardNumber;
    std::string pin;
    std::string name;
    double balance;
    bool isActive;
    std::string createdAt;
    int failedAttempts;
    bool isLocked;
};

struct Admin {
    std::string username;
    std::string password;
};

struct Transaction {
    std::string cardNumber;
    std::string type;
    double amount;
    std::string dateTime;
};

struct ATM_Cash {
    int hundreds;
    int fifties;
    int twenties;
    int tens;
    int fives;
    int ones;
    
    ATM_Cash() : hundreds(50), fifties(50), twenties(50), 
                 tens(50), fives(50), ones(50) {}
    
    double getTotal() {
        return hundreds*100 + fifties*50 + twenties*20 + 
               tens*10 + fives*5 + ones*1;
    }
};

// Global variables
std::vector<Account> accounts;
std::vector<Admin> admins;
std::vector<Transaction> transactions;
ATM_Cash atmCash;
Account* currentUser = nullptr;
Admin* currentAdmin = nullptr;

// Helper functions
char getch() {
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

void clearScreen() {
    std::cout << "\033[2J\033[1;1H";
}

std::string getCurrentDateTime() {
    time_t now = time(0);
    char buffer[80];
    strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", localtime(&now));
    return std::string(buffer);
}

void printHeader(const std::string& title) {
    clearScreen();
    std::cout << CYAN << "\n========================================\n";
    std::cout << "       " << title << "\n";
    std::cout << "========================================\n" << RESET;
}

void printSuccess(const std::string& msg) {
    std::cout << GREEN << "✓ " << msg << RESET << "\n";
}

void printError(const std::string& msg) {
    std::cout << RED << "✗ " << msg << RESET << "\n";
}

void printWarning(const std::string& msg) {
    std::cout << YELLOW << "⚠ " << msg << RESET << "\n";
}

void printInfo(const std::string& msg) {
    std::cout << BLUE << "ℹ " << msg << RESET << "\n";
}

void printMoney(double amount) {
    std::cout << GREEN << "$" << std::fixed << std::setprecision(2) << amount << RESET;
}

// Simple encryption (Caesar cipher)
std::string encode(const std::string& input) {
    std::string result = input;
    for (char& c : result) c = c + 3;
    return result;
}

std::string decode(const std::string& input) {
    std::string result = input;
    for (char& c : result) c = c - 3;
    return result;
}

// Validation
bool isValidCard(const std::string& card) {
    if (card.length() != 16) return false;
    for (char c : card) if (!isdigit(c)) return false;
    return true;
}

bool isValidPIN(const std::string& pin) {
    if (pin.length() < 4 || pin.length() > 6) return false;
    for (char c : pin) if (!isdigit(c)) return false;
    return true;
}

// Data persistence
void loadData() {
    // Load accounts
    std::ifstream file("accounts.txt");
    if (file.is_open()) {
        std::string line;
        while (std::getline(file, line)) {
            std::stringstream ss(line);
            Account acc;
            std::getline(ss, acc.cardNumber, '|');
            std::getline(ss, acc.pin, '|');
            std::getline(ss, acc.name, '|');
            ss >> acc.balance;
            ss.ignore();
            std::getline(ss, acc.createdAt, '|');
            ss >> acc.isActive >> acc.failedAttempts >> acc.isLocked;
            accounts.push_back(acc);
        }
        file.close();
    }
    
    // Load admins
    std::ifstream afile("admins.txt");
    if (afile.is_open()) {
        std::string line;
        while (std::getline(afile, line)) {
            std::stringstream ss(line);
            Admin admin;
            std::getline(ss, admin.username, '|');
            std::getline(ss, admin.password, '|');
            admins.push_back(admin);
        }
        afile.close();
    }
    
    // Load transactions
    std::ifstream tfile("transactions.txt");
    if (tfile.is_open()) {
        std::string line;
        while (std::getline(tfile, line)) {
            std::stringstream ss(line);
            Transaction trans;
            std::getline(ss, trans.cardNumber, '|');
            std::getline(ss, trans.type, '|');
            ss >> trans.amount;
            ss.ignore();
            std::getline(ss, trans.dateTime, '|');
            transactions.push_back(trans);
        }
        tfile.close();
    }
    
    // Load ATM cash
    std::ifstream cfile("atm.txt");
    if (cfile.is_open()) {
        cfile >> atmCash.hundreds >> atmCash.fifties >> atmCash.twenties
              >> atmCash.tens >> atmCash.fives >> atmCash.ones;
        cfile.close();
    }
}

void saveData() {
    // Save accounts
    std::ofstream file("accounts.txt");
    for (const auto& acc : accounts) {
        file << acc.cardNumber << "|" << acc.pin << "|" << acc.name << "|"
             << acc.balance << "|" << acc.createdAt << "|" << acc.isActive << "|"
             << acc.failedAttempts << "|" << acc.isLocked << "\n";
    }
    file.close();
    
    // Save admins
    std::ofstream afile("admins.txt");
    for (const auto& admin : admins) {
        afile << admin.username << "|" << admin.password << "\n";
    }
    afile.close();
    
    // Save transactions
    std::ofstream tfile("transactions.txt", std::ios::app);
    for (const auto& trans : transactions) {
        tfile << trans.cardNumber << "|" << trans.type << "|"
              << trans.amount << "|" << trans.dateTime << "\n";
    }
    tfile.close();
    
    // Save ATM cash
    std::ofstream cfile("atm.txt");
    cfile << atmCash.hundreds << " " << atmCash.fifties << " " << atmCash.twenties << " "
          << atmCash.tens << " " << atmCash.fives << " " << atmCash.ones;
    cfile.close();
}

void addTransaction(const std::string& card, const std::string& type, double amount) {
    Transaction t;
    t.cardNumber = card;
    t.type = type;
    t.amount = amount;
    t.dateTime = getCurrentDateTime();
    transactions.push_back(t);
}

// Fee calculation (simple)
double calculateFee(double amount) {
    if (amount <= 100) return 0;
    if (amount <= 500) return amount * 0.01;
    if (amount <= 1000) return amount * 0.015;
    return amount * 0.02;
}

// Bill calculation (simple)
bool canDispense(double amount, int& h, int& f, int& tw, int& te, int& fi, int& o) {
    h = f = tw = te = fi = o = 0;
    double remaining = amount;
    
    h = std::min(static_cast<int>(remaining / 100), atmCash.hundreds);
    remaining -= h * 100;
    
    f = std::min(static_cast<int>(remaining / 50), atmCash.fifties);
    remaining -= f * 50;
    
    tw = std::min(static_cast<int>(remaining / 20), atmCash.twenties);
    remaining -= tw * 20;
    
    te = std::min(static_cast<int>(remaining / 10), atmCash.tens);
    remaining -= te * 10;
    
    fi = std::min(static_cast<int>(remaining / 5), atmCash.fives);
    remaining -= fi * 5;
    
    o = std::min(static_cast<int>(remaining), atmCash.ones);
    remaining -= o;
    
    return (remaining < 0.01);
}

// Login
bool login() {
    printHeader("ATM LOGIN");
    
    std::cout << "\n1. Client Login\n";
    std::cout << "2. Admin Login\n";
    std::cout << "3. Exit\n";
    std::cout << "Choice: ";
    
    int choice;
    std::cin >> choice;
    
    if (choice == 3) {
        std::cout << "\nThank you for using ATM. Goodbye!\n";
        exit(0);
    }
    
    std::string username, password;
    std::cout << "\nUsername/Card: ";
    std::cin >> username;
    
    std::cout << "PIN/Password: ";
    char ch;
    password = "";
    while ((ch = getch()) != '\n' && ch != '\r') {
        if (ch == 127 || ch == '\b') {
            if (!password.empty()) {
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
        // Check if locked
        for (auto& acc : accounts) {
            if (acc.cardNumber == username && acc.isLocked) {
                printError("Account is locked!");
                return false;
            }
        }
        
        // Try login
        for (auto& acc : accounts) {
            if (acc.cardNumber == username && decode(acc.pin) == password && acc.isActive) {
                acc.failedAttempts = 0;
                currentUser = &acc;
                printSuccess("Welcome " + acc.name + "!");
                return true;
            }
        }
        
        // Increment failed attempts
        for (auto& acc : accounts) {
            if (acc.cardNumber == username) {
                acc.failedAttempts++;
                if (acc.failedAttempts >= 3) {
                    acc.isLocked = true;
                    printWarning("Account locked after 3 failed attempts!");
                }
                break;
            }
        }
        printError("Invalid credentials!");
        return false;
    }
    else if (choice == 2) {
        for (auto& admin : admins) {
            if (admin.username == username && decode(admin.password) == password) {
                currentAdmin = &admin;
                printSuccess("Admin login successful!");
                return true;
            }
        }
        printError("Invalid admin credentials!");
        return false;
    }
    
    return false;
}

// Client functions
void checkBalance() {
    printHeader("BALANCE INQUIRY");
    std::cout << "\nAccount: " << currentUser->name << "\n";
    std::cout << "Card: " << currentUser->cardNumber << "\n";
    std::cout << "Balance: ";
    printMoney(currentUser->balance);
    std::cout << "\nDate: " << getCurrentDateTime() << "\n";
    addTransaction(currentUser->cardNumber, "BALANCE", currentUser->balance);
}

void withdraw() {
    printHeader("WITHDRAWAL");
    
    std::cout << "\nBalance: ";
    printMoney(currentUser->balance);
    std::cout << "\n\n";
    
    std::cout << "Quick amounts: [1]$20 [2]$40 [3]$60 [4]$80 [5]$100\n";
    std::cout << "Enter amount: $";
    
    double amount;
    std::cin >> amount;
    
    if (amount <= 0) {
        printError("Invalid amount!");
        return;
    }
    
    if (amount > 1000) {
        printError("Maximum withdrawal is $1000!");
        return;
    }
    
    if (amount > currentUser->balance) {
        printError("Insufficient balance!");
        return;
    }
    
    double fee = calculateFee(amount);
    double total = amount + fee;
    
    if (total > currentUser->balance) {
        printError("Insufficient for fee of $" + std::to_string(fee));
        return;
    }
    
    int h, f, tw, te, fi, o;
    if (!canDispense(amount, h, f, tw, te, fi, o)) {
        printError("Cannot dispense amount!");
        return;
    }
    
    std::cout << "\nFee: $" << fee << "\n";
    std::cout << "Total: $" << total << "\n";
    std::cout << "\nBills:\n";
    if (h) std::cout << "  $100 x " << h << "\n";
    if (f) std::cout << "  $50 x " << f << "\n";
    if (tw) std::cout << "  $20 x " << tw << "\n";
    if (te) std::cout << "  $10 x " << te << "\n";
    if (fi) std::cout << "  $5 x " << fi << "\n";
    if (o) std::cout << "  $1 x " << o << "\n";
    
    std::cout << "\nConfirm? (y/n): ";
    char confirm;
    std::cin >> confirm;
    
    if (confirm == 'y' || confirm == 'Y') {
        currentUser->balance -= total;
        atmCash.hundreds -= h;
        atmCash.fifties -= f;
        atmCash.twenties -= tw;
        atmCash.tens -= te;
        atmCash.fives -= fi;
        atmCash.ones -= o;
        
        addTransaction(currentUser->cardNumber, "WITHDRAW", amount);
        printSuccess("Withdrawal successful!");
        std::cout << "New balance: ";
        printMoney(currentUser->balance);
        std::cout << "\n";
    }
}

void transfer() {
    printHeader("TRANSFER FUNDS");
    
    std::cout << "\nYour balance: ";
    printMoney(currentUser->balance);
    std::cout << "\n\n";
    
    std::string recipientCard;
    std::cout << "Recipient card number: ";
    std::cin >> recipientCard;
    
    Account* recipient = nullptr;
    for (auto& acc : accounts) {
        if (acc.cardNumber == recipientCard && acc.isActive && !acc.isLocked) {
            recipient = &acc;
            break;
        }
    }
    
    if (!recipient) {
        printError("Recipient not found!");
        return;
    }
    
    if (recipient->cardNumber == currentUser->cardNumber) {
        printError("Cannot transfer to yourself!");
        return;
    }
    
    double amount;
    std::cout << "Amount to transfer: $";
    std::cin >> amount;
    
    if (amount <= 0) {
        printError("Invalid amount!");
        return;
    }
    
    double fee = calculateFee(amount);
    double total = amount + fee;
    
    if (total > currentUser->balance) {
        printError("Insufficient balance!");
        return;
    }
    
    std::cout << "\nFee: $" << fee << "\n";
    std::cout << "Total: $" << total << "\n";
    std::cout << "Recipient: " << recipient->name << "\n";
    
    std::cout << "\nConfirm? (y/n): ";
    char confirm;
    std::cin >> confirm;
    
    if (confirm == 'y' || confirm == 'Y') {
        currentUser->balance -= total;
        recipient->balance += amount;
        
        addTransaction(currentUser->cardNumber, "TRANSFER_OUT", amount);
        addTransaction(recipient->cardNumber, "TRANSFER_IN", amount);
        
        printSuccess("Transfer successful!");
        std::cout << "New balance: ";
        printMoney(currentUser->balance);
        std::cout << "\n";
    }
}

void history() {
    printHeader("TRANSACTION HISTORY");
    
    int count = 0;
    for (const auto& t : transactions) {
        if (t.cardNumber == currentUser->cardNumber) {
            count++;
            std::cout << t.dateTime << " | " << t.type << " | ";
            printMoney(t.amount);
            std::cout << "\n";
        }
    }
    
    if (count == 0) {
        printInfo("No transactions found.");
    }
}

void changePIN() {
    printHeader("CHANGE PIN");
    
    std::string oldPIN, newPIN;
    
    std::cout << "Current PIN: ";
    std::cin >> oldPIN;
    
    if (decode(currentUser->pin) != oldPIN) {
        printError("Incorrect PIN!");
        return;
    }
    
    std::cout << "New PIN (4-6 digits): ";
    std::cin >> newPIN;
    
    if (!isValidPIN(newPIN)) {
        printError("Invalid PIN format!");
        return;
    }
    
    std::cout << "Confirm: ";
    std::string confirm;
    std::cin >> confirm;
    
    if (newPIN != confirm) {
        printError("PINs don't match!");
        return;
    }
    
    currentUser->pin = encode(newPIN);
    addTransaction(currentUser->cardNumber, "PIN_CHANGE", 0);
    printSuccess("PIN changed successfully!");
}

void clientMenu() {
    while (currentUser) {
        printHeader("CLIENT MENU - " + currentUser->name);
        std::cout << "Balance: ";
        printMoney(currentUser->balance);
        std::cout << "\n\n";
        
        std::cout << "1. Check Balance\n";
        std::cout << "2. Withdraw Cash\n";
        std::cout << "3. Transfer Funds\n";
        std::cout << "4. Transaction History\n";
        std::cout << "5. Change PIN\n";
        std::cout << "6. Logout\n";
        std::cout << "Choice: ";
        
        int choice;
        std::cin >> choice;
        
        switch (choice) {
            case 1: checkBalance(); break;
            case 2: withdraw(); break;
            case 3: transfer(); break;
            case 4: history(); break;
            case 5: changePIN(); break;
            case 6: 
                currentUser = nullptr;
                printSuccess("Logged out!");
                return;
            default:
                printError("Invalid choice!");
        }
        
        std::cout << "\nPress any key...";
        getch();
    }
}

// Admin functions
void viewATM() {
    printHeader("ATM CASH STATUS");
    std::cout << "\n";
    std::cout << "$100: " << atmCash.hundreds << " = $" << atmCash.hundreds*100 << "\n";
    std::cout << "$50:  " << atmCash.fifties << " = $" << atmCash.fifties*50 << "\n";
    std::cout << "$20:  " << atmCash.twenties << " = $" << atmCash.twenties*20 << "\n";
    std::cout << "$10:  " << atmCash.tens << " = $" << atmCash.tens*10 << "\n";
    std::cout << "$5:   " << atmCash.fives << " = $" << atmCash.fives*5 << "\n";
    std::cout << "$1:   " << atmCash.ones << " = $" << atmCash.ones*1 << "\n";
    std::cout << "------------------------\n";
    std::cout << "TOTAL: ";
    printMoney(atmCash.getTotal());
    std::cout << "\n";
}

void refillATM() {
    printHeader("REFILL ATM");
    viewATM();
    
    std::cout << "\nEnter new counts:\n";
    std::cout << "$100: "; std::cin >> atmCash.hundreds;
    std::cout << "$50:  "; std::cin >> atmCash.fifties;
    std::cout << "$20:  "; std::cin >> atmCash.twenties;
    std::cout << "$10:  "; std::cin >> atmCash.tens;
    std::cout << "$5:   "; std::cin >> atmCash.fives;
    std::cout << "$1:   "; std::cin >> atmCash.ones;
    
    printSuccess("ATM refilled!");
}

void createAccount() {
    printHeader("CREATE ACCOUNT");
    
    Account acc;
    
    std::cout << "Card Number (16 digits): ";
    std::cin >> acc.cardNumber;
    
    if (!isValidCard(acc.cardNumber)) {
        printError("Invalid card number!");
        return;
    }
    
    for (const auto& a : accounts) {
        if (a.cardNumber == acc.cardNumber) {
            printError("Card number exists!");
            return;
        }
    }
    
    std::string pin;
    std::cout << "PIN (4-6 digits): ";
    std::cin >> pin;
    
    if (!isValidPIN(pin)) {
        printError("Invalid PIN!");
        return;
    }
    acc.pin = encode(pin);
    
    std::cout << "Account Holder Name: ";
    std::cin.ignore();
    std::getline(std::cin, acc.name);
    
    std::cout << "Initial Deposit: $";
    std::cin >> acc.balance;
    
    if (acc.balance < 0) {
        printError("Invalid amount!");
        return;
    }
    
    acc.isActive = true;
    acc.isLocked = false;
    acc.failedAttempts = 0;
    acc.createdAt = getCurrentDateTime();
    
    accounts.push_back(acc);
    addTransaction(acc.cardNumber, "ACCOUNT_OPENED", acc.balance);
    
    printSuccess("Account created!");
}

void viewAccounts() {
    printHeader("ALL ACCOUNTS");
    
    if (accounts.empty()) {
        printInfo("No accounts.");
        return;
    }
    
    double total = 0;
    for (const auto& acc : accounts) {
        total += acc.balance;
        std::cout << acc.cardNumber << " | " << acc.name << " | ";
        printMoney(acc.balance);
        std::cout << " | " << (acc.isActive ? "Active" : "Inactive");
        if (acc.isLocked) std::cout << " [LOCKED]";
        std::cout << "\n";
    }
    std::cout << "------------------------\n";
    std::cout << "Total: ";
    printMoney(total);
    std::cout << " (" << accounts.size() << " accounts)\n";
}

void resetPIN() {
    printHeader("RESET USER PIN");
    
    std::string card;
    std::cout << "Card number: ";
    std::cin >> card;
    
    for (auto& acc : accounts) {
        if (acc.cardNumber == card) {
            std::cout << "Account: " << acc.name << "\n";
            std::cout << "Status: " << (acc.isLocked ? "LOCKED" : "Active") << "\n\n";
            
            std::string newPIN;
            std::cout << "New PIN: ";
            std::cin >> newPIN;
            
            if (!isValidPIN(newPIN)) {
                printError("Invalid PIN!");
                return;
            }
            
            acc.pin = encode(newPIN);
            acc.failedAttempts = 0;
            acc.isLocked = false;
            
            addTransaction(card, "PIN_RESET", 0);
            printSuccess("PIN reset successful!");
            return;
        }
    }
    
    printError("Account not found!");
}

void adminMenu() {
    while (currentAdmin) {
        printHeader("ADMIN MENU - " + currentAdmin->username);
        std::cout << "\n";
        std::cout << "1. View ATM Cash\n";
        std::cout << "2. Refill ATM\n";
        std::cout << "3. Create Account\n";
        std::cout << "4. View All Accounts\n";
        std::cout << "5. Reset User PIN\n";
        std::cout << "6. Change Admin Password\n";
        std::cout << "7. Logout\n";
        std::cout << "Choice: ";
        
        int choice;
        std::cin >> choice;
        
        switch (choice) {
            case 1: viewATM(); break;
            case 2: refillATM(); break;
            case 3: createAccount(); break;
            case 4: viewAccounts(); break;
            case 5: resetPIN(); break;
            case 6:
                std::cout << "New password: ";
                std::string newPass;
                std::cin >> newPass;
                currentAdmin->password = encode(newPass);
                printSuccess("Password changed!");
                break;
            case 7:
                currentAdmin = nullptr;
                printSuccess("Admin logged out!");
                return;
            default:
                printError("Invalid choice!");
        }
        
        std::cout << "\nPress any key...";
        getch();
    }
}

int main() {
    // Load data
    loadData();
    
    // Create default admin if none
    if (admins.empty()) {
        Admin admin;
        admin.username = "admin6767";
        admin.password = encode("67676767");
        admins.push_back(admin);
        saveData();
    }
    
    // Create sample account if none
    if (accounts.empty()) {
        Account acc;
        acc.cardNumber = "1234567890123456";
        acc.pin = encode("1234");
        acc.name = "John Doe";
        acc.balance = 5000.00;
        acc.isActive = true;
        acc.isLocked = false;
        acc.failedAttempts = 0;
        acc.createdAt = getCurrentDateTime();
        accounts.push_back(acc);
        saveData();
    }
    
    // Welcome screen
    clearScreen();
    std::cout << CYAN << "\n";
    std::cout << "╔════════════════════════════════╗\n";
    std::cout << "║     ATM BANKING SYSTEM         ║\n";
    std::cout << "╚════════════════════════════════╝\n" << RESET;
    std::cout << "\n";
    std::cout << "Admin: admin6767 / 67676767\n";
    std::cout << "Sample: 1234567890123456 / 1234 ($5000)\n";
    std::cout << "\nPress any key to start...";
    getch();
    
    // Main loop
    while (true) {
        if (login()) {
            if (currentUser) clientMenu();
            else if (currentAdmin) adminMenu();
        }
        saveData();
    }
    
    return 0;
}

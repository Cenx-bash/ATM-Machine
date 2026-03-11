#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <ctime>
#include <termios.h>
#include <unistd.h>

// Simple color codes (optional - can be removed if you don't want colors)
#define RESET   "\033[0m"
#define RED     "\033[31m"
#define GREEN   "\033[32m"
#define YELLOW  "\033[33m"
#define BLUE    "\033[34m"
#define CYAN    "\033[36m"

// Structure to store account information
struct Account {
    std::string cardNumber;     // 16-digit card number
    std::string pin;            // Encrypted PIN
    std::string name;           // Account holder name
    double balance;              // Current balance
    bool isActive;               // Account active status
    std::string createdAt;       // Account creation date
    int failedAttempts;          // Number of failed login attempts
    bool isLocked;               // Account lock status
};

// Structure to store admin information
struct Admin {
    std::string username;        // Admin username
    std::string password;        // Admin password (encrypted)
};

// Structure to store transaction records
struct Transaction {
    std::string cardNumber;      // Card number associated with transaction
    std::string type;            // Transaction type (WITHDRAW, DEPOSIT, etc.)
    double amount;                // Transaction amount
    std::string dateTime;         // Date and time of transaction
};

// Structure to track ATM cash inventory
struct ATM_Cash {
    int hundreds;                 // Number of $100 bills
    int fifties;                  // Number of $50 bills
    int twenties;                 // Number of $20 bills
    int tens;                     // Number of $10 bills
    int fives;                    // Number of $5 bills
    int ones;                     // Number of $1 bills
    
    // Constructor with default values
    ATM_Cash() : hundreds(50), fifties(50), twenties(50), 
                 tens(50), fives(50), ones(50) {}
    
    // Calculate total cash in ATM
    double getTotal() {
        return hundreds * 100 + fifties * 50 + twenties * 20 + 
               tens * 10 + fives * 5 + ones * 1;
    }
};

// Global variables
std::vector<Account> accounts;        // All bank accounts
std::vector<Admin> admins;            // All admin users
std::vector<Transaction> transactions; // All transactions
ATM_Cash atmCash;                      // ATM cash inventory
Account* currentUser = nullptr;        // Currently logged in user
Admin* currentAdmin = nullptr;         // Currently logged in admin

// Function to get a single character without echoing (for password input)
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

// Function to clear the console screen
void clearScreen() {
    std::cout << "\033[2J\033[1;1H";
}

// Function to get current date and time as string
std::string getCurrentDateTime() {
    time_t now = time(0);
    char buffer[80];
    strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", localtime(&now));
    return std::string(buffer);
}

// Function to display a formatted header
void printHeader(const std::string& title) {
    clearScreen();
    std::cout << "\n";
    std::cout << "========================================\n";
    std::cout << "       " << title << "\n";
    std::cout << "========================================\n";
    std::cout << "\n";
}

// Function to display success messages
void printSuccess(const std::string& msg) {
    std::cout << "[SUCCESS] " << msg << "\n";
}

// Function to display error messages
void printError(const std::string& msg) {
    std::cout << "[ERROR] " << msg << "\n";
}

// Function to display warning messages
void printWarning(const std::string& msg) {
    std::cout << "[WARNING] " << msg << "\n";
}

// Function to display information messages
void printInfo(const std::string& msg) {
    std::cout << "[INFO] " << msg << "\n";
}

// Function to display money with proper formatting
void printMoney(double amount) {
    std::cout << "$" << std::fixed << std::setprecision(2) << amount;
}

// Simple encryption function (Caesar cipher with shift 3)
std::string encode(const std::string& input) {
    std::string result = input;
    for (char& c : result) {
        c = c + 3;  // Shift each character by 3
    }
    return result;
}

// Simple decryption function
std::string decode(const std::string& input) {
    std::string result = input;
    for (char& c : result) {
        c = c - 3;  // Reverse the shift
    }
    return result;
}

// Function to validate card number (16 digits)
bool isValidCard(const std::string& card) {
    if (card.length() != 16) return false;
    for (char c : card) {
        if (!isdigit(c)) return false;
    }
    return true;
}

// Function to validate PIN (4-6 digits)
bool isValidPIN(const std::string& pin) {
    if (pin.length() < 4 || pin.length() > 6) return false;
    for (char c : pin) {
        if (!isdigit(c)) return false;
    }
    return true;
}

// Function to load all data from files
void loadData() {
    // Load accounts from accounts.txt
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
    
    // Load admins from admins.txt
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
    
    // Load transactions from transactions.txt
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
    
    // Load ATM cash from atm.txt
    std::ifstream cfile("atm.txt");
    if (cfile.is_open()) {
        cfile >> atmCash.hundreds >> atmCash.fifties >> atmCash.twenties
              >> atmCash.tens >> atmCash.fives >> atmCash.ones;
        cfile.close();
    }
}

// Function to save all data to files
void saveData() {
    // Save accounts to accounts.txt
    std::ofstream file("accounts.txt");
    for (const auto& acc : accounts) {
        file << acc.cardNumber << "|" << acc.pin << "|" << acc.name << "|"
             << acc.balance << "|" << acc.createdAt << "|" << acc.isActive << "|"
             << acc.failedAttempts << "|" << acc.isLocked << "\n";
    }
    file.close();
    
    // Save admins to admins.txt
    std::ofstream afile("admins.txt");
    for (const auto& admin : admins) {
        afile << admin.username << "|" << admin.password << "\n";
    }
    afile.close();
    
    // Append new transactions to transactions.txt
    std::ofstream tfile("transactions.txt", std::ios::app);
    for (const auto& trans : transactions) {
        tfile << trans.cardNumber << "|" << trans.type << "|"
              << trans.amount << "|" << trans.dateTime << "\n";
    }
    tfile.close();
    
    // Clear transactions vector after saving
    transactions.clear();
    
    // Save ATM cash to atm.txt
    std::ofstream cfile("atm.txt");
    cfile << atmCash.hundreds << " " << atmCash.fifties << " " << atmCash.twenties << " "
          << atmCash.tens << " " << atmCash.fives << " " << atmCash.ones;
    cfile.close();
}

// Function to add a new transaction record
void addTransaction(const std::string& card, const std::string& type, double amount) {
    Transaction t;
    t.cardNumber = card;
    t.type = type;
    t.amount = amount;
    t.dateTime = getCurrentDateTime();
    transactions.push_back(t);
}

// Function to calculate transaction fee (simple percentage based)
double calculateFee(double amount) {
    if (amount <= 100) return 0;
    if (amount <= 500) return amount * 0.01;      // 1% fee
    if (amount <= 1000) return amount * 0.015;    // 1.5% fee
    return amount * 0.02;                           // 2% fee
}

// Function to check if ATM can dispense the requested amount
bool canDispense(double amount, int& h, int& f, int& tw, int& te, int& fi, int& o) {
    h = f = tw = te = fi = o = 0;
    double remaining = amount;
    
    // Calculate number of each bill needed
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
    
    // Return true if we can dispense the exact amount
    return (remaining < 0.01);
}

// Login function
bool login() {
    printHeader("ATM LOGIN");
    
    std::cout << "1. Client Login\n";
    std::cout << "2. Admin Login\n";
    std::cout << "3. Exit\n";
    std::cout << "Enter choice: ";
    
    int choice;
    std::cin >> choice;
    
    if (choice == 3) {
        std::cout << "\nThank you for using ATM. Goodbye!\n";
        return false;
    }
    
    std::string username, password;
    std::cout << "\nEnter Username/Card Number: ";
    std::cin >> username;
    
    std::cout << "Enter PIN/Password: ";
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
        // Check if account is locked
        for (auto& acc : accounts) {
            if (acc.cardNumber == username && acc.isLocked) {
                printError("Account is locked. Please contact customer support.");
                return false;
            }
        }
        
        // Attempt client login
        for (auto& acc : accounts) {
            if (acc.cardNumber == username && decode(acc.pin) == password && acc.isActive) {
                acc.failedAttempts = 0;
                currentUser = &acc;
                printSuccess("Welcome " + acc.name + "!");
                return true;
            }
        }
        
        // Increment failed attempts for existing account
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
        printError("Invalid card number or PIN!");
        return false;
    }
    else if (choice == 2) {
        // Attempt admin login
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

// Client function: Check balance
void checkBalance() {
    printHeader("BALANCE INQUIRY");
    std::cout << "Account Holder: " << currentUser->name << "\n";
    std::cout << "Card Number: " << currentUser->cardNumber << "\n";
    std::cout << "Current Balance: ";
    printMoney(currentUser->balance);
    std::cout << "\n";
    std::cout << "Date/Time: " << getCurrentDateTime() << "\n";
    addTransaction(currentUser->cardNumber, "BALANCE_INQUIRY", currentUser->balance);
}

// Client function: Withdraw cash
void withdraw() {
    printHeader("CASH WITHDRAWAL");
    
    std::cout << "Your Balance: ";
    printMoney(currentUser->balance);
    std::cout << "\n\n";
    
    std::cout << "Available denominations: $100, $50, $20, $10, $5, $1\n";
    std::cout << "Enter amount to withdraw: $";
    
    double amount;
    std::cin >> amount;
    
    if (amount <= 0) {
        printError("Invalid amount!");
        return;
    }
    
    if (amount > 1000) {
        printError("Maximum withdrawal amount is $1000!");
        return;
    }
    
    if (amount > currentUser->balance) {
        printError("Insufficient balance!");
        return;
    }
    
    double fee = calculateFee(amount);
    double total = amount + fee;
    
    if (total > currentUser->balance) {
        printError("Insufficient balance to cover fee of $" + std::to_string(fee));
        return;
    }
    
    int h, f, tw, te, fi, o;
    if (!canDispense(amount, h, f, tw, te, fi, o)) {
        printError("ATM cannot dispense this amount with available bills!");
        return;
    }
    
    std::cout << "\nTransaction Summary:\n";
    std::cout << "  Withdrawal Amount: ";
    printMoney(amount);
    std::cout << "\n  Fee: ";
    printMoney(fee);
    std::cout << "\n  Total Deduction: ";
    printMoney(total);
    std::cout << "\n\nBills to be dispensed:\n";
    if (h > 0) std::cout << "  $100 x " << h << "\n";
    if (f > 0) std::cout << "  $50 x " << f << "\n";
    if (tw > 0) std::cout << "  $20 x " << tw << "\n";
    if (te > 0) std::cout << "  $10 x " << te << "\n";
    if (fi > 0) std::cout << "  $5 x " << fi << "\n";
    if (o > 0) std::cout << "  $1 x " << o << "\n";
    
    std::cout << "\nConfirm withdrawal? (y/n): ";
    char confirm;
    std::cin >> confirm;
    
    if (confirm == 'y' || confirm == 'Y') {
        // Update balance
        currentUser->balance -= total;
        
        // Update ATM cash
        atmCash.hundreds -= h;
        atmCash.fifties -= f;
        atmCash.twenties -= tw;
        atmCash.tens -= te;
        atmCash.fives -= fi;
        atmCash.ones -= o;
        
        // Record transaction
        addTransaction(currentUser->cardNumber, "WITHDRAWAL", amount);
        
        printSuccess("Withdrawal successful!");
        std::cout << "New balance: ";
        printMoney(currentUser->balance);
        std::cout << "\n";
    } else {
        printWarning("Withdrawal cancelled.");
    }
}

// Client function: Transfer funds
void transfer() {
    printHeader("FUND TRANSFER");
    
    std::cout << "Your Balance: ";
    printMoney(currentUser->balance);
    std::cout << "\n\n";
    
    std::string recipientCard;
    std::cout << "Enter recipient's card number: ";
    std::cin >> recipientCard;
    
    // Find recipient account
    Account* recipient = nullptr;
    for (auto& acc : accounts) {
        if (acc.cardNumber == recipientCard && acc.isActive && !acc.isLocked) {
            recipient = &acc;
            break;
        }
    }
    
    if (!recipient) {
        printError("Recipient account not found or inactive!");
        return;
    }
    
    if (recipient->cardNumber == currentUser->cardNumber) {
        printError("Cannot transfer to your own account!");
        return;
    }
    
    double amount;
    std::cout << "Enter amount to transfer: $";
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
    
    std::cout << "\nTransfer Summary:\n";
    std::cout << "  Recipient: " << recipient->name << "\n";
    std::cout << "  Amount: ";
    printMoney(amount);
    std::cout << "\n  Fee: ";
    printMoney(fee);
    std::cout << "\n  Total Deduction: ";
    printMoney(total);
    std::cout << "\n";
    
    std::cout << "\nConfirm transfer? (y/n): ";
    char confirm;
    std::cin >> confirm;
    
    if (confirm == 'y' || confirm == 'Y') {
        // Update balances
        currentUser->balance -= total;
        recipient->balance += amount;
        
        // Record transactions
        addTransaction(currentUser->cardNumber, "TRANSFER_OUT", amount);
        addTransaction(recipient->cardNumber, "TRANSFER_IN", amount);
        
        printSuccess("Transfer successful!");
        std::cout << "Your new balance: ";
        printMoney(currentUser->balance);
        std::cout << "\n";
    } else {
        printWarning("Transfer cancelled.");
    }
}

// Client function: View transaction history
void viewHistory() {
    printHeader("TRANSACTION HISTORY");
    
    int count = 0;
    std::cout << "Date/Time                | Type          | Amount\n";
    std::cout << "------------------------------------------------\n";
    
    for (const auto& t : transactions) {
        if (t.cardNumber == currentUser->cardNumber) {
            count++;
            std::cout << t.dateTime << " | ";
            std::cout << std::left << std::setw(12) << t.type << " | ";
            printMoney(t.amount);
            std::cout << "\n";
        }
    }
    
    if (count == 0) {
        printInfo("No transactions found.");
    } else {
        std::cout << "------------------------------------------------\n";
        std::cout << "Total transactions: " << count << "\n";
    }
}

// Client function: Change PIN
void changePIN() {
    printHeader("CHANGE PIN");
    
    std::string oldPIN, newPIN, confirmPIN;
    
    std::cout << "Enter current PIN: ";
    std::cin >> oldPIN;
    
    if (decode(currentUser->pin) != oldPIN) {
        printError("Incorrect PIN!");
        return;
    }
    
    std::cout << "Enter new PIN (4-6 digits): ";
    std::cin >> newPIN;
    
    if (!isValidPIN(newPIN)) {
        printError("Invalid PIN format! PIN must be 4-6 digits.");
        return;
    }
    
    std::cout << "Confirm new PIN: ";
    std::cin >> confirmPIN;
    
    if (newPIN != confirmPIN) {
        printError("PINs do not match!");
        return;
    }
    
    // Update PIN
    currentUser->pin = encode(newPIN);
    addTransaction(currentUser->cardNumber, "PIN_CHANGE", 0);
    
    printSuccess("PIN changed successfully!");
}

// Client menu
void clientMenu() {
    while (currentUser != nullptr) {
        printHeader("CLIENT MENU - " + currentUser->name);
        std::cout << "Current Balance: ";
        printMoney(currentUser->balance);
        std::cout << "\n\n";
        
        std::cout << "1. Check Balance\n";
        std::cout << "2. Withdraw Cash\n";
        std::cout << "3. Transfer Funds\n";
        std::cout << "4. View Transaction History\n";
        std::cout << "5. Change PIN\n";
        std::cout << "6. Logout\n";
        std::cout << "Enter choice: ";
        
        int choice;
        std::cin >> choice;
        
        switch (choice) {
            case 1:
                checkBalance();
                break;
            case 2:
                withdraw();
                break;
            case 3:
                transfer();
                break;
            case 4:
                viewHistory();
                break;
            case 5:
                changePIN();
                break;
            case 6:
                currentUser = nullptr;
                printSuccess("Logged out successfully!");
                return;
            default:
                printError("Invalid choice! Please try again.");
        }
        
        std::cout << "\nPress any key to continue...";
        getch();
    }
}

// Admin function: View ATM cash status
void viewATM() {
    printHeader("ATM CASH STATUS");
    std::cout << "\n";
    std::cout << "Denomination | Count | Value\n";
    std::cout << "-----------------------------\n";
    std::cout << "$100         | " << std::setw(5) << atmCash.hundreds 
              << " | $" << atmCash.hundreds * 100 << "\n";
    std::cout << "$50          | " << std::setw(5) << atmCash.fifties 
              << " | $" << atmCash.fifties * 50 << "\n";
    std::cout << "$20          | " << std::setw(5) << atmCash.twenties 
              << " | $" << atmCash.twenties * 20 << "\n";
    std::cout << "$10          | " << std::setw(5) << atmCash.tens 
              << " | $" << atmCash.tens * 10 << "\n";
    std::cout << "$5           | " << std::setw(5) << atmCash.fives 
              << " | $" << atmCash.fives * 5 << "\n";
    std::cout << "$1           | " << std::setw(5) << atmCash.ones 
              << " | $" << atmCash.ones * 1 << "\n";
    std::cout << "-----------------------------\n";
    std::cout << "TOTAL CASH   |       | ";
    printMoney(atmCash.getTotal());
    std::cout << "\n";
}

// Admin function: Refill ATM cash
void refillATM() {
    printHeader("REFILL ATM");
    viewATM();
    
    std::cout << "\nEnter new bill counts:\n";
    std::cout << "Number of $100 bills: ";
    std::cin >> atmCash.hundreds;
    std::cout << "Number of $50 bills: ";
    std::cin >> atmCash.fifties;
    std::cout << "Number of $20 bills: ";
    std::cin >> atmCash.twenties;
    std::cout << "Number of $10 bills: ";
    std::cin >> atmCash.tens;
    std::cout << "Number of $5 bills: ";
    std::cin >> atmCash.fives;
    std::cout << "Number of $1 bills: ";
    std::cin >> atmCash.ones;
    
    printSuccess("ATM cash refilled successfully!");
    std::cout << "New total cash: ";
    printMoney(atmCash.getTotal());
    std::cout << "\n";
}

// Admin function: Create new account
void createAccount() {
    printHeader("CREATE NEW ACCOUNT");
    
    Account newAcc;
    
    std::cout << "Enter Card Number (16 digits): ";
    std::cin >> newAcc.cardNumber;
    
    if (!isValidCard(newAcc.cardNumber)) {
        printError("Invalid card number! Must be 16 digits.");
        return;
    }
    
    // Check if card number already exists
    for (const auto& acc : accounts) {
        if (acc.cardNumber == newAcc.cardNumber) {
            printError("Card number already exists!");
            return;
        }
    }
    
    std::string pin;
    std::cout << "Enter PIN (4-6 digits): ";
    std::cin >> pin;
    
    if (!isValidPIN(pin)) {
        printError("Invalid PIN! Must be 4-6 digits.");
        return;
    }
    newAcc.pin = encode(pin);
    
    std::cout << "Enter Account Holder Name: ";
    std::cin.ignore();
    std::getline(std::cin, newAcc.name);
    
    std::cout << "Enter Initial Deposit: $";
    std::cin >> newAcc.balance;
    
    if (newAcc.balance < 0) {
        printError("Initial deposit cannot be negative!");
        return;
    }
    
    // Set default values
    newAcc.isActive = true;
    newAcc.isLocked = false;
    newAcc.failedAttempts = 0;
    newAcc.createdAt = getCurrentDateTime();
    
    accounts.push_back(newAcc);
    addTransaction(newAcc.cardNumber, "ACCOUNT_CREATED", newAcc.balance);
    
    printSuccess("Account created successfully!");
    std::cout << "Account Summary:\n";
    std::cout << "  Card: " << newAcc.cardNumber << "\n";
    std::cout << "  Name: " << newAcc.name << "\n";
    std::cout << "  Initial Balance: ";
    printMoney(newAcc.balance);
    std::cout << "\n";
}

// Admin function: View all accounts
void viewAllAccounts() {
    printHeader("ALL ACCOUNTS");
    
    if (accounts.empty()) {
        printInfo("No accounts found.");
        return;
    }
    
    double totalBalance = 0;
    std::cout << "Card Number      | Name                | Balance   | Status\n";
    std::cout << "------------------------------------------------------------\n";
    
    for (const auto& acc : accounts) {
        totalBalance += acc.balance;
        std::cout << acc.cardNumber << " | ";
        std::cout << std::left << std::setw(18) << acc.name.substr(0, 17) << " | ";
        std::cout << std::right << std::setw(8);
        printMoney(acc.balance);
        std::cout << " | ";
        if (!acc.isActive) {
            std::cout << "Inactive";
        } else if (acc.isLocked) {
            std::cout << "LOCKED";
        } else {
            std::cout << "Active";
        }
        std::cout << "\n";
    }
    
    std::cout << "------------------------------------------------------------\n";
    std::cout << "Total Accounts: " << accounts.size() << "\n";
    std::cout << "Total Balance: ";
    printMoney(totalBalance);
    std::cout << "\n";
}

// Admin function: Reset user PIN
void resetUserPIN() {
    printHeader("RESET USER PIN");
    
    std::string cardNumber;
    std::cout << "Enter card number: ";
    std::cin >> cardNumber;
    
    for (auto& acc : accounts) {
        if (acc.cardNumber == cardNumber) {
            std::cout << "Account found:\n";
            std::cout << "  Name: " << acc.name << "\n";
            std::cout << "  Status: " << (acc.isLocked ? "LOCKED" : "Active") << "\n\n";
            
            std::string newPIN;
            std::cout << "Enter new PIN (4-6 digits): ";
            std::cin >> newPIN;
            
            if (!isValidPIN(newPIN)) {
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
            
            // Reset PIN and unlock account
            acc.pin = encode(newPIN);
            acc.failedAttempts = 0;
            acc.isLocked = false;
            
            addTransaction(cardNumber, "PIN_RESET", 0);
            
            printSuccess("PIN reset successful!");
            printInfo("Account has been unlocked.");
            return;
        }
    }
    
    printError("Account not found!");
}

// Admin menu
void adminMenu() {
    while (currentAdmin != nullptr) {
        printHeader("ADMIN MENU - " + currentAdmin->username);
        
        std::cout << "1. View ATM Cash Status\n";
        std::cout << "2. Refill ATM Cash\n";
        std::cout << "3. Create New Account\n";
        std::cout << "4. View All Accounts\n";
        std::cout << "5. Reset User PIN\n";
        std::cout << "6. Change Admin Password\n";
        std::cout << "7. Logout\n";
        std::cout << "Enter choice: ";
        
        int choice;
        std::cin >> choice;
        
        switch (choice) {
            case 1:
                viewATM();
                break;
            case 2:
                refillATM();
                break;
            case 3:
                createAccount();
                break;
            case 4:
                viewAllAccounts();
                break;
            case 5:
                resetUserPIN();
                break;
            case 6: {
                std::string newPass;
                std::cout << "Enter new password: ";
                std::cin >> newPass;
                currentAdmin->password = encode(newPass);
                printSuccess("Password changed successfully!");
                break;
            }
            case 7:
                currentAdmin = nullptr;
                printSuccess("Admin logged out!");
                return;
            default:
                printError("Invalid choice!");
        }
        
        std::cout << "\nPress any key to continue...";
        getch();
    }
}

// Main function
int main() {
    // Load existing data
    loadData();
    
    // Create default admin if none exists
    if (admins.empty()) {
        Admin defaultAdmin;
        defaultAdmin.username = "admin6767";
        defaultAdmin.password = encode("67676767");
        admins.push_back(defaultAdmin);
        saveData();
    }
    
    // Create sample account if none exists
    if (accounts.empty()) {
        Account sampleAcc;
        sampleAcc.cardNumber = "1234567890123456";
        sampleAcc.pin = encode("1234");
        sampleAcc.name = "John Doe";
        sampleAcc.balance = 5000.00;
        sampleAcc.isActive = true;
        sampleAcc.isLocked = false;
        sampleAcc.failedAttempts = 0;
        sampleAcc.createdAt = getCurrentDateTime();
        accounts.push_back(sampleAcc);
        saveData();
    }
    
    // Welcome screen
    clearScreen();
    std::cout << "\n";
    std::cout << "========================================\n";
    std::cout << "     ATM BANKING SYSTEM v1.0\n";
    std::cout << "========================================\n";
    std::cout << "\n";
    std::cout << "Default Login Credentials:\n";
    std::cout << "-------------------------\n";
    std::cout << "Admin: admin6767 / 67676767\n";
    std::cout << "Sample Account: 1234567890123456 / 1234\n";
    std::cout << "Sample Balance: $5000.00\n";
    std::cout << "\n";
    std::cout << "Press any key to continue...";
    getch();
    
    // Main program loop
    while (true) {
        if (login()) {
            if (currentUser != nullptr) {
                clientMenu();
            } else if (currentAdmin != nullptr) {
                adminMenu();
            }
        }
        
        // Save data after each session
        saveData();
        
        std::cout << "\nPress any key to continue...";
        getch();
    }
    
    return 0;
}

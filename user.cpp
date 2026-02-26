#include <iostream>
#include <vector>
#include <map>
#include <string>
#include <iomanip>
#include <sstream>
#include <fstream>
#include <ctime>
#include <algorithm>
#include <limits>
#include <random>
#include <memory>
#include <cmath>

// Transaction structure
struct Transaction {
    std::time_t timestamp;
    std::string type;
    std::string details;
    double balanceAfter;
    
    Transaction(std::string t, std::string d, double ba) 
        : type(t), details(d), balanceAfter(ba) {
        timestamp = std::time(nullptr);
    }
    
    std::string getFormattedTime() const {
        char buffer[80];
        std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", std::localtime(&timestamp));
        return std::string(buffer);
    }
};

// Account Class
class Account {
private:
    std::string accountNumber;
    std::string accountHolderName;
    std::string pin;
    double balance;
    std::vector<Transaction> transactionHistory;
    bool locked;
    int failedAttempts;
    std::time_t lastTransaction;

public:
    Account() : balance(0.0), locked(false), failedAttempts(0) {
        lastTransaction = std::time(nullptr);
    }
    
    Account(std::string accNo, std::string name, std::string p, double initialDeposit) 
        : accountNumber(accNo), accountHolderName(name), pin(p), 
          balance(initialDeposit), locked(false), failedAttempts(0) {
        lastTransaction = std::time(nullptr);
        addTransaction("ACCOUNT_OPENED", "Initial deposit: $" + std::to_string(initialDeposit));
    }
    
    bool validatePin(const std::string& inputPin) {
        if (locked) {
            std::cout << "\n⚠ Account is locked. Please contact customer service." << std::endl;
            return false;
        }
        
        if (pin == inputPin) {
            failedAttempts = 0;
            return true;
        } else {
            failedAttempts++;
            if (failedAttempts >= 3) {
                locked = true;
                addTransaction("ACCOUNT_LOCKED", "Account locked due to 3 failed PIN attempts");
            }
            return false;
        }
    }
    
    bool withdraw(double amount) {
        if (amount <= 0 || amount > balance) {
            addTransaction("WITHDRAWAL_FAILED", "Failed withdrawal attempt: $" + std::to_string(amount));
            return false;
        }
        
        balance -= amount;
        lastTransaction = std::time(nullptr);
        addTransaction("WITHDRAWAL", "Withdrawn: $" + std::to_string(amount));
        return true;
    }
    
    void deposit(double amount) {
        if (amount > 0) {
            balance += amount;
            lastTransaction = std::time(nullptr);
            addTransaction("DEPOSIT", "Deposited: $" + std::to_string(amount));
        }
    }
    
    void addTransaction(const std::string& type, const std::string& details) {
        Transaction trans(type, details, balance);
        transactionHistory.push_back(trans);
        
        // Keep only last 50 transactions
        if (transactionHistory.size() > 50) {
            transactionHistory.erase(transactionHistory.begin());
        }
    }
    
    void displayTransactionHistory() const {
        if (transactionHistory.empty()) {
            std::cout << "   No transactions found." << std::endl;
            return;
        }
        
        std::cout << "\n┌─────────────────────── TRANSACTION HISTORY ───────────────────────┐" << std::endl;
        std::cout << "│ Date/Time            │ Type           │ Details          │ Balance  │" << std::endl;
        std::cout << "├───────────────────────┼────────────────┼──────────────────┼──────────┤" << std::endl;
        
        for (const auto& trans : transactionHistory) {
            std::cout << "│ " << std::left << std::setw(20) << trans.getFormattedTime() << " │ ";
            std::cout << std::left << std::setw(14) << trans.type << "│ ";
            std::cout << std::left << std::setw(16) << trans.details << "│ $";
            std::cout << std::right << std::setw(8) << std::fixed << std::setprecision(2) << trans.balanceAfter << " │" << std::endl;
        }
        std::cout << "└─────────────────────────────────────────────────────────────────────┘" << std::endl;
    }
    
    // Getters and Setters
    std::string getAccountNumber() const { return accountNumber; }
    std::string getAccountHolderName() const { return accountHolderName; }
    double getBalance() const { return balance; }
    bool isLocked() const { return locked; }
    
    void setPin(const std::string& newPin) { 
        pin = newPin; 
        addTransaction("PIN_CHANGED", "PIN was changed");
    }
    
    void unlockAccount() {
        locked = false;
        failedAttempts = 0;
        addTransaction("ACCOUNT_UNLOCKED", "Account was unlocked");
    }
    
    // Serialization for file storage
    std::string serialize() const {
        std::stringstream ss;
        ss << accountNumber << "|" << accountHolderName << "|" << pin << "|" 
           << balance << "|" << locked << "|" << failedAttempts << "\n";
        return ss.str();
    }
    
    void deserialize(const std::string& data) {
        std::stringstream ss(data);
        std::getline(ss, accountNumber, '|');
        std::getline(ss, accountHolderName, '|');
        std::getline(ss, pin, '|');
        ss >> balance;
        ss.ignore();
        ss >> locked;
        ss.ignore();
        ss >> failedAttempts;
    }
};

// ATM Class
class ATM {
private:
    static const std::string DATA_FILE;
    static const std::string TRANSACTION_LOG;
    std::map<std::string, Account> accounts;
    Account* currentAccount;
    bool sessionActive;
    
    // Utility methods
    void clearScreen() {
        #ifdef _WIN32
            system("cls");
        #else
            system("clear");
        #endif
    }
    
    void pause() {
        std::cout << "\nPress Enter to continue...";
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::cin.get();
    }
    
    int getIntInput(int min, int max) {
        int input;
        while (true) {
            if (std::cin >> input) {
                if (input >= min && input <= max) {
                    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                    return input;
                }
            }
            std::cout << "⚠ Please enter a number between " << min << " and " << max << ": ";
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        }
    }
    
    double getDoubleInput(double min, double max) {
        double input;
        while (true) {
            if (std::cin >> input) {
                if (input >= min && input <= max) {
                    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                    // Round to 2 decimal places
                    return std::round(input * 100.0) / 100.0;
                }
            }
            std::cout << "⚠ Please enter an amount between $" << std::fixed << std::setprecision(2) 
                 << min << " and $" << max << ": ";
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        }
    }
    
    std::string getMaskedAccountNumber(const std::string& accNo) const {
        if (accNo.length() <= 4) return accNo;
        return "****" + accNo.substr(accNo.length() - 4);
    }
    
    std::string generateAccountNumber() {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(10000000, 99999999);
        
        std::string accNo;
        do {
            accNo = std::to_string(dis(gen));
        } while (accounts.find(accNo) != accounts.end());
        
        return accNo;
    }
    
    void logTransaction(const std::string& type, const std::string& details) {
        std::ofstream logFile(TRANSACTION_LOG, std::ios::app);
        if (logFile.is_open()) {
            std::time_t now = std::time(nullptr);
            char timestamp[80];
            std::strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", std::localtime(&now));
            
            logFile << "[" << timestamp << "] " << type 
                    << " | Account: " << (currentAccount ? currentAccount->getAccountNumber() : "SYSTEM")
                    << " | " << details << std::endl;
            logFile.close();
        }
    }
    
    void saveAccounts() {
        std::ofstream file(DATA_FILE);
        if (file.is_open()) {
            for (const auto& pair : accounts) {
                file << pair.second.serialize();
            }
            file.close();
        }
    }
    
    void loadAccounts() {
        std::ifstream file(DATA_FILE);
        if (file.is_open()) {
            std::string line;
            while (std::getline(file, line)) {
                if (!line.empty()) {
                    Account acc;
                    acc.deserialize(line);
                    accounts[acc.getAccountNumber()] = acc;
                }
            }
            file.close();
        }
    }
    
    void initializeDefaultAccounts() {
        if (accounts.empty()) {
            Account defaultAccount("10000001", "John Doe", "1234", 5000.00);
            accounts[defaultAccount.getAccountNumber()] = defaultAccount;
            saveAccounts();
        }
    }
    
    // Menu methods
    void showMainMenu() {
        std::cout << "\n┌───────────────────── MAIN MENU ─────────────────────┐" << std::endl;
        std::cout << "│  1. Insert Card (Login)                              │" << std::endl;
        std::cout << "│  2. Create New Account                               │" << std::endl;
        std::cout << "│  3. Admin Login                                      │" << std::endl;
        std::cout << "│  4. Exit                                             │" << std::endl;
        std::cout << "└──────────────────────────────────────────────────────┘" << std::endl;
        std::cout << "Choose option: ";
    }
    
    void showUserMenu() {
        std::cout << "\n┌──────────────────── ACCOUNT MENU ────────────────────┐" << std::endl;
        std::cout << "│  Welcome, " << std::left << std::setw(34) << currentAccount->getAccountHolderName() << " │" << std::endl;
        std::cout << "│  Account: " << std::left << std::setw(35) 
             << getMaskedAccountNumber(currentAccount->getAccountNumber()) << " │" << std::endl;
        std::cout << "├───────────────────────────────────────────────────────┤" << std::endl;
        std::cout << "│  1. Check Balance                                    │" << std::endl;
        std::cout << "│  2. Deposit Cash                                     │" << std::endl;
        std::cout << "│  3. Withdraw Cash                                    │" << std::endl;
        std::cout << "│  4. Transfer Funds                                   │" << std::endl;
        std::cout << "│  5. Change PIN                                       │" << std::endl;
        std::cout << "│  6. View Transaction History                         │" << std::endl;
        std::cout << "│  7. Fast Cash ($100)                                 │" << std::endl;
        std::cout << "│  8. Logout                                           │" << std::endl;
        std::cout << "└──────────────────────────────────────────────────────┘" << std::endl;
        std::cout << "Choose option: ";
    }
    
    void showAdminMenu() {
        std::cout << "\n┌───────────────────── ADMIN MENU ─────────────────────┐" << std::endl;
        std::cout << "│  1. View All Accounts                                │" << std::endl;
        std::cout << "│  2. View Total Balance                               │" << std::endl;
        std::cout << "│  3. View Transaction Log                             │" << std::endl;
        std::cout << "│  4. System Status                                    │" << std::endl;
        std::cout << "│  5. Unlock Account                                   │" << std::endl;
        std::cout << "│  6. Exit Admin Mode                                  │" << std::endl;
        std::cout << "└──────────────────────────────────────────────────────┘" << std::endl;
        std::cout << "Choose option: ";
    }
    
    // User operations
    void login() {
        std::string accNo, pin;
        
        std::cout << "\n┌───────────────────── LOGIN ─────────────────────┐" << std::endl;
        std::cout << "Enter account number: ";
        std::getline(std::cin, accNo);
        std::cout << "Enter PIN: ";
        std::getline(std::cin, pin);
        
        auto it = accounts.find(accNo);
        if (it != accounts.end() && it->second.validatePin(pin)) {
            currentAccount = &it->second;
            sessionActive = true;
            logTransaction("LOGIN", "Successful login");
            std::cout << "\n✅ Login successful!" << std::endl;
        } else {
            std::cout << "\n❌ Invalid account number or PIN!" << std::endl;
        }
        pause();
    }
    
    void createAccount() {
        std::string name, pin, confirmPin;
        double initialDeposit;
        
        std::cout << "\n┌───────────────── CREATE NEW ACCOUNT ─────────────────┐" << std::endl;
        std::cout << "Enter your full name: ";
        std::getline(std::cin, name);
        
        std::cout << "Create a 4-digit PIN: ";
        std::getline(std::cin, pin);
        
        if (pin.length() != 4 || !std::all_of(pin.begin(), pin.end(), ::isdigit)) {
            std::cout << "\n❌ PIN must be exactly 4 digits!" << std::endl;
            pause();
            return;
        }
        
        std::cout << "Confirm PIN: ";
        std::getline(std::cin, confirmPin);
        
        if (pin != confirmPin) {
            std::cout << "\n❌ PINs don't match!" << std::endl;
            pause();
            return;
        }
        
        std::cout << "Initial deposit amount (minimum $100): $";
        initialDeposit = getDoubleInput(100, 1000000);
        
        std::string accNo = generateAccountNumber();
        Account newAccount(accNo, name, pin, initialDeposit);
        accounts[accNo] = newAccount;
        saveAccounts();
        
        std::cout << "\n✅ Account created successfully!" << std::endl;
        std::cout << "   Your account number: " << accNo << std::endl;
        std::cout << "   Please remember your account number and PIN." << std::endl;
        logTransaction("ACCOUNT_CREATED", "New account created: " + accNo);
        pause();
    }
    
    void checkBalance() {
        std::cout << "\n┌────────────────── CURRENT BALANCE ──────────────────┐" << std::endl;
        std::cout << "│ Account: " << std::left << std::setw(42) 
             << getMaskedAccountNumber(currentAccount->getAccountNumber()) << " │" << std::endl;
        std::cout << "│ Holder:  " << std::left << std::setw(42) 
             << currentAccount->getAccountHolderName() << " │" << std::endl;
        std::cout << "│ Balance: $" << std::right << std::setw(39) << std::fixed << std::setprecision(2) 
             << currentAccount->getBalance() << " │" << std::endl;
        std::cout << "└───────────────────────────────────────────────────────┘" << std::endl;
        logTransaction("BALANCE_INQUIRY", "Balance checked");
        pause();
    }
    
    void deposit() {
        std::cout << "\n┌─────────────────── DEPOSIT CASH ───────────────────┐" << std::endl;
        std::cout << "│ Current balance: $" << std::left << std::setw(35) << std::fixed << std::setprecision(2) 
             << currentAccount->getBalance() << " │" << std::endl;
        std::cout << "├──────────────────────────────────────────────────────┤" << std::endl;
        std::cout << "Enter deposit amount: $";
        
        double amount = getDoubleInput(1, 10000);
        
        currentAccount->deposit(amount);
        saveAccounts();
        
        std::cout << "\n✅ $" << std::fixed << std::setprecision(2) << amount << " deposited successfully!" << std::endl;
        std::cout << "   New balance: $" << currentAccount->getBalance() << std::endl;
        logTransaction("DEPOSIT", "Deposited $" + std::to_string(amount));
        pause();
    }
    
    void withdraw() {
        std::cout << "\n┌─────────────────── WITHDRAW CASH ──────────────────┐" << std::endl;
        std::cout << "│ Current balance: $" << std::left << std::setw(35) << std::fixed << std::setprecision(2) 
             << currentAccount->getBalance() << " │" << std::endl;
        std::cout << "├──────────────────────────────────────────────────────┤" << std::endl;
        std::cout << "Enter withdrawal amount: $";
        
        double amount = getDoubleInput(1, 1000);
        
        if (amount > currentAccount->getBalance()) {
            std::cout << "\n❌ Insufficient funds!" << std::endl;
            pause();
            return;
        }
        
        // Check if amount is multiple of 20 (ATM dispenses $20 bills)
        if (std::fmod(amount, 20) != 0) {
            std::cout << "\n❌ ATM only dispenses $20 bills!" << std::endl;
            pause();
            return;
        }
        
        if (currentAccount->withdraw(amount)) {
            saveAccounts();
            std::cout << "\n✅ Please take your cash: $" << std::fixed << std::setprecision(2) << amount << std::endl;
            std::cout << "   Number of bills: " << static_cast<int>(amount / 20) << std::endl;
            std::cout << "   Remaining balance: $" << currentAccount->getBalance() << std::endl;
            logTransaction("WITHDRAWAL", "Withdrew $" + std::to_string(amount));
        } else {
            std::cout << "\n❌ Withdrawal failed!" << std::endl;
        }
        pause();
    }
    
    void transferFunds() {
        std::string recipientAcc, pin;
        
        std::cout << "\n┌─────────────────── TRANSFER FUNDS ──────────────────┐" << std::endl;
        std::cout << "│ Current balance: $" << std::left << std::setw(36) << std::fixed << std::setprecision(2) 
             << currentAccount->getBalance() << " │" << std::endl;
        std::cout << "├──────────────────────────────────────────────────────┤" << std::endl;
        
        std::cout << "Enter recipient account number: ";
        std::getline(std::cin, recipientAcc);
        
        auto it = accounts.find(recipientAcc);
        if (it == accounts.end()) {
            std::cout << "\n❌ Recipient account not found!" << std::endl;
            pause();
            return;
        }
        
        std::cout << "Enter transfer amount: $";
        double amount = getDoubleInput(1, currentAccount->getBalance());
        
        std::cout << "Enter your PIN to confirm: ";
        std::getline(std::cin, pin);
        
        if (!currentAccount->validatePin(pin)) {
            std::cout << "\n❌ Invalid PIN! Transfer cancelled." << std::endl;
            pause();
            return;
        }
        
        if (currentAccount->withdraw(amount)) {
            it->second.deposit(amount);
            saveAccounts();
            std::cout << "\n✅ $" << std::fixed << std::setprecision(2) << amount 
                 << " transferred successfully to " 
                 << getMaskedAccountNumber(recipientAcc) << "!" << std::endl;
            std::cout << "   New balance: $" << currentAccount->getBalance() << std::endl;
            logTransaction("TRANSFER", "Transferred $" + std::to_string(amount) + " to " + recipientAcc);
        }
        pause();
    }
    
    void changePin() {
        std::string currentPin, newPin, confirmPin;
        
        std::cout << "\n┌───────────────────── CHANGE PIN ─────────────────────┐" << std::endl;
        std::cout << "Enter current PIN: ";
        std::getline(std::cin, currentPin);
        
        if (!currentAccount->validatePin(currentPin)) {
            std::cout << "\n❌ Invalid current PIN!" << std::endl;
            pause();
            return;
        }
        
        std::cout << "Enter new 4-digit PIN: ";
        std::getline(std::cin, newPin);
        
        if (newPin.length() != 4 || !std::all_of(newPin.begin(), newPin.end(), ::isdigit)) {
            std::cout << "\n❌ PIN must be exactly 4 digits!" << std::endl;
            pause();
            return;
        }
        
        std::cout << "Confirm new PIN: ";
        std::getline(std::cin, confirmPin);
        
        if (newPin != confirmPin) {
            std::cout << "\n❌ PINs don't match!" << std::endl;
            pause();
            return;
        }
        
        currentAccount->setPin(newPin);
        saveAccounts();
        std::cout << "\n✅ PIN changed successfully!" << std::endl;
        logTransaction("PIN_CHANGE", "PIN changed");
        pause();
    }
    
    void viewTransactionHistory() {
        currentAccount->displayTransactionHistory();
        logTransaction("VIEW_HISTORY", "Viewed transaction history");
        pause();
    }
    
    void fastCash() {
        double amount = 100.00;
        
        if (amount > currentAccount->getBalance()) {
            std::cout << "\n❌ Insufficient funds!" << std::endl;
            pause();
            return;
        }
        
        if (currentAccount->withdraw(amount)) {
            saveAccounts();
            std::cout << "\n✅ Fast cash: $100.00 dispensed!" << std::endl;
            std::cout << "   Remaining balance: $" << std::fixed << std::setprecision(2) 
                 << currentAccount->getBalance() << std::endl;
            logTransaction("FAST_CASH", "Withdrew $100.00");
        }
        pause();
    }
    
    void logout() {
        logTransaction("LOGOUT", "Session ended");
        currentAccount = nullptr;
        sessionActive = false;
        std::cout << "\n✅ Logged out successfully!" << std::endl;
        pause();
    }
    
    // Admin operations
    void viewAllAccounts() {
        std::cout << "\n┌───────────────────── ALL ACCOUNTS ─────────────────────┐" << std::endl;
        if (accounts.empty()) {
            std::cout << "│           No accounts found.                           │" << std::endl;
        } else {
            for (const auto& pair : accounts) {
                const Account& acc = pair.second;
                std::cout << "│ " << std::left << std::setw(15) << acc.getAccountNumber() 
                     << " │ " << std::left << std::setw(20) << acc.getAccountHolderName()
                     << " │ $" << std::right << std::setw(10) << std::fixed << std::setprecision(2) 
                     << acc.getBalance() 
                     << (acc.isLocked() ? " 🔒" : "   ") << " │" << std::endl;
            }
        }
        std::cout << "└──────────────────────────────────────────────────────────┘" << std::endl;
        pause();
    }
    
    void viewTotalBalance() {
        double total = 0;
        for (const auto& pair : accounts) {
            total += pair.second.getBalance();
        }
        
        std::cout << "\n┌────────────────── TOTAL BANK BALANCE ──────────────────┐" << std::endl;
        std::cout << "│ Total balance in bank: $" << std::left << std::setw(32) << std::fixed 
             << std::setprecision(2) << total << " │" << std::endl;
        std::cout << "└──────────────────────────────────────────────────────────┘" << std::endl;
        pause();
    }
    
    void viewTransactionLog() {
        std::cout << "\n┌─────────────────── TRANSACTION LOG ───────────────────┐" << std::endl;
        
        std::ifstream logFile(TRANSACTION_LOG);
        if (logFile.is_open()) {
            std::string line;
            int count = 0;
            while (std::getline(logFile, line) && count < 50) { // Show last 50 entries
                std::cout << line << std::endl;
                count++;
            }
            logFile.close();
        } else {
            std::cout << "│           No transaction log found.                    │" << std::endl;
        }
        std::cout << "└──────────────────────────────────────────────────────────┘" << std::endl;
        pause();
    }
    
    void showSystemStatus() {
        std::cout << "\n┌───────────────────── SYSTEM STATUS ────────────────────┐" << std::endl;
        std::cout << "│ ATM Status:        ONLINE                                 │" << std::endl;
        std::cout << "│ Total Accounts:    " << std::left << std::setw(35) << accounts.size() << " │" << std::endl;
        std::cout << "│ Active Session:    " << std::left << std::setw(35) 
             << (sessionActive ? "YES" : "NO") << " │" << std::endl;
        std::cout << "│ Cash Dispenser:    OPERATIONAL                           │" << std::endl;
        std::cout << "│ Receipt Printer:   READY                                 │" << std::endl;
        std::cout << "│ Network:           CONNECTED                             │" << std::endl;
        std::cout << "└──────────────────────────────────────────────────────────┘" << std::endl;
        pause();
    }
    
    void unlockAccount() {
        std::string accNo;
        std::cout << "\nEnter account number to unlock: ";
        std::getline(std::cin, accNo);
        
        auto it = accounts.find(accNo);
        if (it != accounts.end()) {
            it->second.unlockAccount();
            saveAccounts();
            std::cout << "\n✅ Account " << getMaskedAccountNumber(accNo) << " unlocked successfully!" << std::endl;
            logTransaction("ADMIN_UNLOCK", "Account " + accNo + " unlocked by admin");
        } else {
            std::cout << "\n❌ Account not found!" << std::endl;
        }
        pause();
    }
    
    void adminLogin() {
        std::string password;
        std::cout << "\nEnter admin password: ";
        std::getline(std::cin, password);
        
        // In production, use proper authentication
        if (password == "admin123") {
            bool adminMode = true;
            while (adminMode) {
                clearScreen();
                showAdminMenu();
                int choice = getIntInput(1, 6);
                
                switch (choice) {
                    case 1: viewAllAccounts(); break;
                    case 2: viewTotalBalance(); break;
                    case 3: viewTransactionLog(); break;
                    case 4: showSystemStatus(); break;
                    case 5: unlockAccount(); break;
                    case 6: adminMode = false; break;
                }
            }
        } else {
            std::cout << "\n❌ Invalid admin password!" << std::endl;
            pause();
        }
    }
    
    void exitSystem() {
        std::cout << "\n┌───────────────────────────────────────────────────────┐" << std::endl;
        std::cout << "│  Thank you for using Secure Bank ATM!                 │" << std::endl;
        std::cout << "│  Have a great day!                                     │" << std::endl;
        std::cout << "└───────────────────────────────────────────────────────┘" << std::endl;
        saveAccounts();
        std::exit(0);
    }

public:
    ATM() : currentAccount(nullptr), sessionActive(false) {
        loadAccounts();
        initializeDefaultAccounts();
    }
    
    void start() {
        while (true) {
            clearScreen();
            
            std::cout << "\n╔═══════════════════════════════════════════════════════╗" << std::endl;
            std::cout << "║         WELCOME TO SECURE BANK ATM                    ║" << std::endl;
            std::cout << "╚═══════════════════════════════════════════════════════╝" << std::endl;
            
            if (!sessionActive) {
                showMainMenu();
                int choice = getIntInput(1, 4);
                
                switch (choice) {
                    case 1: login(); break;
                    case 2: createAccount(); break;
                    case 3: adminLogin(); break;
                    case 4: exitSystem(); break;
                }
            } else {
                showUserMenu();
                int choice = getIntInput(1, 8);
                
                switch (choice) {
                    case 1: checkBalance(); break;
                    case 2: deposit(); break;
                    case 3: withdraw(); break;
                    case 4: transferFunds(); break;
                    case 5: changePin(); break;
                    case 6: viewTransactionHistory(); break;
                    case 7: fastCash(); break;
                    case 8: logout(); break;
                }
            }
        }
    }
};

// Static member definitions
const std::string ATM::DATA_FILE = "accounts.dat";
const std::string ATM::TRANSACTION_LOG = "transactions.log";

// Main function
int main() {
    try {
        ATM atm;
        atm.start();
    } catch (const std::exception& e) {
        std::cerr << "\n❌ Fatal error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}

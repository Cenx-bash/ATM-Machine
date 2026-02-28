#include "User.h"
#include <iostream>
#include <iomanip>

using namespace std;

string User::getCurrentDateTime() {
    time_t now = time(0);
    char* dt = ctime(&now);
    return string(dt);
}

User::User() : balance(0.0), isActive(true) {}

User::User(string uname, string pwd, string name, string accNum) 
    : username(uname), password(pwd), fullName(name), 
      balance(0.0), isActive(true), accountNumber(accNum) {}

string User::getUsername() const { return username; }
string User::getFullName() const { return fullName; }
double User::getBalance() const { return balance; }
string User::getAccountNumber() const { return accountNumber; }
bool User::getStatus() const { return isActive; }
vector<Transaction> User::getTransactionHistory() const { return transactionHistory; }

void User::setFullName(string name) { fullName = name; }
void User::setStatus(bool status) { isActive = status; }

bool User::verifyPassword(string pwd) const {
    return password == pwd;
}

bool User::deposit(double amount) {
    if (amount > 0) {
        balance += amount;
        addTransaction("DEPOSIT", amount);
        return true;
    }
    return false;
}

bool User::withdraw(double amount) {
    if (amount > 0 && amount <= balance) {
        balance -= amount;
        addTransaction("WITHDRAWAL", amount);
        return true;
    }
    return false;
}

void User::addTransaction(string type, double amount) {
    Transaction trans;
    trans.type = type;
    trans.amount = amount;
    trans.date = getCurrentDateTime();
    trans.balanceAfter = balance;
    transactionHistory.push_back(trans);
}

void User::displayTransactionHistory() const {
    if (transactionHistory.empty()) {
        cout << "\nNo transactions found.\n";
        return;
    }

    cout << "\n=== TRANSACTION HISTORY ===\n";
    cout << left << setw(15) << "Type" 
         << setw(15) << "Amount" 
         << setw(25) << "Date" 
         << setw(15) << "Balance After" << endl;
    cout << string(70, '-') << endl;

    for (const auto& trans : transactionHistory) {
        cout << left << setw(15) << trans.type 
             << "$" << setw(14) << fixed << setprecision(2) << trans.amount
             << setw(25) << trans.date.substr(0, trans.date.length() - 1)
             << "$" << setw(14) << trans.balanceAfter << endl;
    }
}

void User::displayAccountInfo() const {
    cout << "\n=== ACCOUNT INFORMATION ===\n";
    cout << "Account Number: " << accountNumber << endl;
    cout << "Full Name: " << fullName << endl;
    cout << "Username: " << username << endl;
    cout << "Balance: $" << fixed << setprecision(2) << balance << endl;
    cout << "Status: " << (isActive ? "Active" : "Inactive") << endl;
}

bool User::changePassword(string oldPwd, string newPwd) {
    if (password == oldPwd) {
        password = newPwd;
        return true;
    }
    return false;
}

string User::serialize() const {
    stringstream ss;
    ss << username << "|" << password << "|" << fullName << "|" 
       << balance << "|" << isActive << "|" << accountNumber << "|";
    
    // Serialize transaction history size
    ss << transactionHistory.size() << "|";
    
    // Serialize each transaction
    for (const auto& trans : transactionHistory) {
        ss << trans.type << "|" << trans.amount << "|" 
           << trans.date << "|" << trans.balanceAfter << "|";
    }
    
    return ss.str();
}

User User::deserialize(const string& data) {
    User user;
    stringstream ss(data);
    string token;
    
    getline(ss, token, '|'); user.username = token;
    getline(ss, token, '|'); user.password = token;
    getline(ss, token, '|'); user.fullName = token;
    getline(ss, token, '|'); user.balance = stod(token);
    getline(ss, token, '|'); user.isActive = (token == "1");
    getline(ss, token, '|'); user.accountNumber = token;
    
    int transCount;
    getline(ss, token, '|'); transCount = stoi(token);
    
    for (int i = 0; i < transCount; i++) {
        Transaction trans;
        getline(ss, trans.type, '|');
        getline(ss, token, '|'); trans.amount = stod(token);
        getline(ss, trans.date, '|');
        getline(ss, token, '|'); trans.balanceAfter = stod(token);
        user.transactionHistory.push_back(trans);
    }
    
    return user;
}

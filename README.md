# 🏦 ATM Simulation System (C++)

## Project Overview

This project implements a **simulation of an Automated Teller Machine (ATM)** designed for a local bank in the Philippines. The system mimics real-world banking operations and supports both **Client transactions** and an **Admin management panel**.

The program is developed using **C++** and demonstrates core programming concepts such as loops, arrays, vectors, recursion, modular functions, and file handling.

The ATM supports **multiple banks**, **transaction fees**, **cash denomination tracking**, and **secure PIN handling using encoded strings**.

---

# 💻 System Features

## Client Features

Clients can perform common banking transactions using their ATM card.

* Insert card (enter card number)
* Select bank
* Enter PIN authentication
* Check account balance
* Withdraw money
* Transfer funds
* View transaction history
* Change PIN
* Logout safely

Each client screen displays:

* Current **date and time**
* **Bank name**
* **Account type**
* **Current balance**

---

## Admin Panel

The ATM system includes an **administrator interface** for bank personnel.

Admin features include:

* View ATM cash inventory
* Refill ATM cash
* Create new user accounts
* View all accounts and balances
* Delete or deactivate accounts
* Reset user PIN
* Change admin passcode
* View admin users

The ATM machine can only be **refilled between 8:00 AM and 8:15 AM** using the `ctime` library.

---

# 🏦 Supported Banks

| Bank          | Local Fee | International Fee | Daily Limit |
| ------------- | --------- | ----------------- | ----------- |
| BDO           | PHP 25    | PHP 150           | PHP 50,000  |
| BPI           | PHP 20    | PHP 125           | PHP 75,000  |
| Metrobank     | PHP 30    | PHP 200           | PHP 100,000 |
| Security Bank | PHP 15    | PHP 100           | PHP 60,000  |

International transfer fees apply when sending money to a **different bank**.

---

# 💰 ATM Cash Denominations

The ATM uses fixed denominations stored in arrays.

* PHP 500 bills
* PHP 1000 bills

The system tracks:

* Bill quantities
* Total cash available in the machine

---

# 🔐 Security System

Sensitive information such as:

* PIN codes
* Admin passcodes
* Card numbers

are stored as **encoded strings** rather than plaintext.

The encoding process:

1. Convert each character to its ASCII value
2. Convert ASCII values to strings
3. Store encoded strings in vectors

This prevents simple memory inspection attacks and improves security.

---

# 🧠 Programming Concepts Used

The project demonstrates several important **C++ programming topics**.

### Loops

Used for:

* Menu navigation
* Transaction history display
* Input validation

### Arrays

Used for fixed data such as:

* Bank configurations
* ATM bill denominations

### Vectors

Used for dynamic data including:

* User accounts
* Transaction logs
* Admin users

### Functions

The program uses modular functions to separate responsibilities.

Main functions include:

* `login()`
* `displayMenu()`
* `clientMenu()`
* `adminMenu()`

### Recursion

Recursion is used for:

* Calculating transaction fees
* Computing bill distribution during withdrawals

### Function Overloading

Multiple versions of functions such as:

* `withdraw()` for preset amounts
* `withdraw()` for custom amounts

### File Handling (fstream)

Transactions are logged to a CSV file.

Example format:

```
Date,Time,CardNumber,Type,Amount,Fee
03/02/2025,14:35,1234567890,Withdrawal,5000,25
```

---

# 📁 Project Structure

```
ATM-System/
│
├── main.cpp
├── security.cpp
├── transactions.cpp
├── display.cpp
├── transactions.csv
└── README.md
```

### File Description

**main.cpp**

* Entry point of the program
* Initializes vectors and arrays
* Controls system navigation

**security.cpp**

* Encoding and decoding functions
* Card validation

**transactions.cpp**

* Withdrawal logic
* Transfer logic
* Transaction logging

**display.cpp**

* Menu display
* Date and time output

**transactions.csv**

* Stores all transaction logs

---

# 🔄 System Workflow

1. Program starts
2. User selects role (Client or Admin)
3. Authentication is performed
4. System routes to the correct menu
5. User performs transactions or management operations
6. System logs transactions
7. User logs out and returns to login screen

---

# 📦 Technologies Used

* C++
* STL Vectors
* Arrays
* Recursion
* File Handling (`fstream`)
* Time Library (`ctime`)
* String Manipulation

---

# 📊 Evaluation Criteria

| Category      | Description                            |
| ------------- | -------------------------------------- |
| Functionality | All ATM features work correctly        |
| Code Quality  | Clean, modular, documented code        |
| C++ Topics    | All required programming concepts used |
| Security      | Encoded PIN and validation             |
| UI            | Clear menus and system flow            |
| ctime Usage   | Maintenance window and timestamps      |
| Bonus         | File logging using fstream             |

---

# 👨‍💻 Authors

Developed as a **C++ programming Mid-Term project** simulating real-world ATM banking operations.

---

# 📜 License

This project is for **educational purposes only**.



#ifndef ADMIN_H
#define ADMIN_H

#include <string>
#include <vector>
#include "User.h"

using namespace std;

class Admin {
private:
    string username;
    string password;
    string fullName;

public:
    Admin();
    Admin(string uname, string pwd, string name);
    
    string getUsername() const;
    string getFullName() const;
    bool verifyPassword(string pwd) const;
    void displayAdminInfo() const;
    
    // Admin operations on users
    User createUser(string username, string password, string fullName, string accNum);
    void viewAllUsers(const vector<User>& users) const;
    User* findUser(vector<User>& users, string username) const;
    void toggleUserStatus(User& user) const;
    void displayUserDetails(const User& user) const;
    void viewSystemStats(const vector<User>& users) const;
    bool deleteUser(vector<User>& users, string username);
    
    static Admin getDefaultAdmin();
};

#endif

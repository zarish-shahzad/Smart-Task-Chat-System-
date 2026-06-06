
//Compile:
 //g++ smarttaskchatsystem.cpp -o smarttask -pthread

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <map>
#include <algorithm>
#include <iomanip>
#include <ctime>

using namespace std;


//  UTILITY

void printLine(const string &c = "-", int len = 52) {
    for (int i = 0; i < len; i++) cout << c;
    cout << "\n";
}

string currentDate() {
    time_t t = time(nullptr);
    tm *lt   = localtime(&t);
    char buf[11];
    strftime(buf, sizeof(buf), "%Y-%m-%d", lt);
    return string(buf);
}

string currentTime() {
    time_t t = time(nullptr);
    tm *lt   = localtime(&t);
    char buf[6];
    strftime(buf, sizeof(buf), "%H:%M", lt);
    return string(buf);
}

void header(const string &title) {
    cout << "\n";
    printLine("=", 52);
    cout << "  " << title << "\n";
    printLine("=", 52);
}

//  TASK CLASS
//  Encapsulation: all fields private, getters/setters only

class Task {
private:
    int    id;
    string desc;
    string owner;
    string assignee;
    string deadline;   // YYYY-MM-DD or "none"
    string priority;   // high / medium / low
    string status;     // pending / done
    string createdAt;

public:
    // Constructor — new task
    Task(int id, const string &desc, const string &owner)
        : id(id), desc(desc), owner(owner), assignee(owner),
          deadline("none"), priority("medium"),
          status("pending"), createdAt(currentDate()) {}

    // Constructor — load from file
    Task(int id, const string &desc, const string &owner,
         const string &assignee, const string &deadline,
         const string &priority, const string &status,
         const string &createdAt)
        : id(id), desc(desc), owner(owner), assignee(assignee),
          deadline(deadline), priority(priority),
          status(status), createdAt(createdAt) {}

    // Getters
    int    getId()        const { return id; }
    string getDesc()      const { return desc; }
    string getOwner()     const { return owner; }
    string getAssignee()  const { return assignee; }
    string getDeadline()  const { return deadline; }
    string getPriority()  const { return priority; }
    string getStatus()    const { return status; }
    string getCreatedAt() const { return createdAt; }

    // Setters
    void setAssignee(const string &a) { assignee = a; }
    void setDeadline(const string &d) { deadline = d; }
    void setPriority(const string &p) { priority = p; }
    void setStatus  (const string &s) { status   = s; }

    // Overdue check
    bool isOverdue() const {
        if (deadline == "none" || status == "done") return false;
        return deadline < currentDate();
    }

    // Display one row
    void print() const {
        string flag = isOverdue() ? " [OVERDUE]" : "";
        cout << "  [" << setw(2) << id << "] "
             << left  << setw(24) << desc
             << " | @"  << setw(8)  << assignee
             << " | "   << setw(6)  << priority
             << " | "   << setw(10) << deadline
             << " | "   << setw(7)  << status
             << flag    << "\n";
    }

    // Serialize for file
    string serialize() const {
        return to_string(id) + "|" + desc     + "|" + owner    + "|" +
               assignee      + "|" + deadline + "|" + priority + "|" +
               status        + "|" + createdAt;
    }
};


//  USER  —  BASE CLASS
//  Inheritance + Polymorphism: virtual methods

class User {
protected:
    string username;
    string password;

public:
    User(const string &u, const string &p)
        : username(u), password(p) {}

    virtual ~User() {}

    // Pure virtual — every subclass must implement
    virtual bool authenticate(const string &pass) = 0;

    virtual void display() const {
        cout << "  " << username << "\n";
    }

    string getUsername() const { return username; }
    string getPassword() const { return password; }
};


//  REGISTEREDUSER  —  DERIVED CLASS
//  Overrides authenticate() and display() — Polymorphism

class RegisteredUser : public User {
private:
    string role;   // "admin" or "member"

public:
    RegisteredUser(const string &u, const string &p,
                   const string &r = "member")
        : User(u, p), role(r) {}

    // Override — runtime polymorphism via virtual dispatch
    bool authenticate(const string &pass) override {
        return password == pass;
    }

    void display() const override {
        cout << "  @" << left << setw(14) << username
             << " [" << role << "]\n";
    }

    string getRole() const { return role; }
};


//  CHAT MANAGER  (Composition: SmartTaskSystem has-a ChatManager)

class ChatManager {
private:
    const string chatFile = "SmartTaskChat.txt";

public:
    ChatManager() {}

    // Send: append one line to file
    void sendMessage(const string &sender,
                     const string &receiver,
                     const string &msg) {
        ofstream f(chatFile, ios::app);
        if (f)
            f << "[" << currentTime() << "] "
              << sender << " -> " << receiver
              << ": " << msg << "\n";
    }

    // Show full chat history
    void showHistory() const {
        header("CHAT HISTORY");
        ifstream f(chatFile);
        if (!f) { cout << "  (No messages yet)\n"; printLine("-",52); return; }
        string line; bool any = false;
        while (getline(f, line))
            if (!line.empty()) { cout << "  " << line << "\n"; any = true; }
        if (!any) cout << "  (No messages yet)\n";
        printLine("-", 52);
    }

    // Show only messages involving a specific user
    void showMyMessages(const string &user) const {
        header("MY MESSAGES  (@" + user + ")");
        ifstream f(chatFile);
        if (!f) { cout << "  (No messages yet)\n"; printLine("-",52); return; }
        string line; bool any = false;
        while (getline(f, line))
            if (line.find(user) != string::npos)
                { cout << "  " << line << "\n"; any = true; }
        if (!any) cout << "  (No messages for you)\n";
        printLine("-", 52);
    }

    // Clear chat file
    void clearHistory() {
        ofstream f(chatFile, ios::trunc);
        cout << "  [+] Chat history cleared.\n";
    }
};


//  FILE MANAGER  (Composition: SmartTaskSystem has-a FileManager)
//  All task and user disk I/O in one place

class FileManager {
private:
    const string taskFile = "SmartTaskData.txt";
    const string userFile = "SmartTaskUsers.txt";

public:
    FileManager() {}

    void saveTasks(const vector<Task> &tasks) {
        ofstream f(taskFile);
        for (auto &t : tasks) f << t.serialize() << "\n";
    }

    vector<Task> loadTasks() {
        vector<Task> tasks;
        ifstream f(taskFile);
        string line;
        while (getline(f, line)) {
            if (line.empty()) continue;
            stringstream ss(line);
            string id, desc, owner, assignee,
                   deadline, priority, status, createdAt;
            getline(ss, id,        '|');
            getline(ss, desc,      '|');
            getline(ss, owner,     '|');
            getline(ss, assignee,  '|');
            getline(ss, deadline,  '|');
            getline(ss, priority,  '|');
            getline(ss, status,    '|');
            getline(ss, createdAt, '|');
            tasks.emplace_back(stoi(id), desc, owner, assignee,
                               deadline, priority, status, createdAt);
        }
        return tasks;
    }

    void saveUsers(const map<string, RegisteredUser*> &users) {
        ofstream f(userFile);
        for (auto &kv : users)
            f << kv.second->getUsername() << "|"
              << kv.second->getPassword() << "|"
              << kv.second->getRole()     << "\n";
    }

    map<string, RegisteredUser*> loadUsers() {
        map<string, RegisteredUser*> users;
        ifstream f(userFile);
        string line;
        while (getline(f, line)) {
            if (line.empty()) continue;
            stringstream ss(line);
            string u, p, r;
            getline(ss, u, '|');
            getline(ss, p, '|');
            getline(ss, r, '|');
            users[u] = new RegisteredUser(u, p, r);
        }
        return users;
    }
};


//  SMARTTASK SYSTEM  —  MAIN CONTROLLER
//  Composition: has-a FileManager, ChatManager, vector<Task>
//  Encapsulation: all data private

class SmartTaskSystem {
private:
    FileManager                  fileManager;   // Composition
    ChatManager                  chatManager;   // Composition
    vector<Task>                 tasks;         // Composition
    map<string, RegisteredUser*> users;

    string currentUser;
    int    idCounter;

    Task* findById(int id) {
        for (auto &t : tasks)
            if (t.getId() == id) return &t;
        return nullptr;
    }

    int pickTaskId(const string &prompt) {
        showTasks();
        cout << "\n  " << prompt;
        int id; cin >> id; cin.ignore();
        if (!findById(id)) {
            cout << "  [!] No task with ID " << id << "\n";
            return -1;
        }
        return id;
    }

    void taskTableHeader() const {
        cout << "  " << left
             << setw(5)  << "ID"
             << setw(24) << "Description"
             << " | " << setw(9)  << "Assignee"
             << " | " << setw(7)  << "Priority"
             << " | " << setw(11) << "Deadline"
             << " | Status\n";
        printLine("-", 52);
    }

public:
    // Constructor
    SmartTaskSystem() : idCounter(1) {
        tasks = fileManager.loadTasks();
        users = fileManager.loadUsers();
        for (auto &t : tasks)
            idCounter = max(idCounter, t.getId() + 1);
    }

    // Destructor
    ~SmartTaskSystem() {
        for (auto &kv : users) delete kv.second;
    }

   
    //  AUTH
   
    bool login() {
        header("SMARTTASK  LOGIN");
        cout << "  Username: ";
        string u; getline(cin, u);
        cout << "  Password: ";
        string p; getline(cin, p);

        if (users.count(u)) {
            User *ptr = users[u];        // base class pointer
            if (ptr->authenticate(p)) { // virtual dispatch — Polymorphism
                currentUser = u;
                cout << "\n  [+] Welcome back, @" << u << "!\n";
                return true;
            }
            cout << "  [!] Wrong password.\n";
            return false;
        }
        // Auto-register
        cout << "  New user — creating account...\n";
        users[u] = new RegisteredUser(u, p, "member");
        fileManager.saveUsers(users);
        currentUser = u;
        cout << "  [+] Account created! Welcome, @" << u << "!\n";
        return true;
    }

   
    //  TASK OPERATIONS
   
    void addTask() {
        cout << "\n  Description: ";
        string desc; getline(cin, desc);
        if (desc.empty()) { cout << "  [!] Cannot be empty.\n"; return; }

        cout << "  Assign to (@username, Enter = yourself): ";
        string assignee; getline(cin, assignee);
        if (assignee.empty()) assignee = currentUser;

        cout << "  Priority  1=High  2=Medium(default)  3=Low : ";
        string pc; getline(cin, pc);
        string priority = (pc=="1") ? "high" : (pc=="3") ? "low" : "medium";

        cout << "  Deadline  (YYYY-MM-DD, Enter to skip): ";
        string dl; getline(cin, dl);
        if (dl.empty()) dl = "none";

        Task t(idCounter++, desc, currentUser);
        t.setAssignee(assignee);
        t.setPriority(priority);
        t.setDeadline(dl);
        tasks.push_back(t);
        fileManager.saveTasks(tasks);
        cout << "  [+] Task #" << t.getId() << " added!\n";
    }

    void showTasks() const {
        header("ALL TASKS");
        if (tasks.empty()) {
            cout << "  (No tasks yet)\n";
            printLine("-", 52);
            return;
        }
        taskTableHeader();
        for (auto &t : tasks) t.print();
        printLine("-", 52);
    }

    void myTasks() const {
        header("MY TASKS  (@" + currentUser + ")");
        taskTableHeader();
        bool found = false;
        for (auto &t : tasks)
            if (t.getAssignee() == currentUser) { t.print(); found = true; }
        if (!found) cout << "  (No tasks assigned to you)\n";
        printLine("-", 52);
    }

    void markDone() {
        int id = pickTaskId("Task ID to mark done: ");
        if (id == -1) return;
        findById(id)->setStatus("done");
        fileManager.saveTasks(tasks);
        cout << "  [+] Task #" << id << " marked as done.\n";
    }

    void deleteTask() {
        int id = pickTaskId("Task ID to delete: ");
        if (id == -1) return;
        cout << "  Confirm delete task #" << id << "? (y/n): ";
        string c; getline(cin, c);
        if (c != "y" && c != "Y") { cout << "  Cancelled.\n"; return; }
        tasks.erase(remove_if(tasks.begin(), tasks.end(),
            [&](Task &t){ return t.getId() == id; }), tasks.end());
        fileManager.saveTasks(tasks);
        cout << "  [+] Task deleted.\n";
    }

    void setPriority() {
        int id = pickTaskId("Task ID to change priority: ");
        if (id == -1) return;
        cout << "  1=High  2=Medium  3=Low : ";
        int c; cin >> c; cin.ignore();
        string p = (c==1) ? "high" : (c==3) ? "low" : "medium";
        findById(id)->setPriority(p);
        fileManager.saveTasks(tasks);
        cout << "  [+] Priority set to '" << p << "'.\n";
    }

    void setDeadline() {
        int id = pickTaskId("Task ID to set deadline: ");
        if (id == -1) return;
        cout << "  Deadline (YYYY-MM-DD): ";
        string d; getline(cin, d);
        if (d.empty()) { cout << "  [!] No deadline entered.\n"; return; }
        findById(id)->setDeadline(d);
        fileManager.saveTasks(tasks);
        cout << "  [+] Deadline set to '" << d << "'.\n";
    }

    void assignTask() {
        int id = pickTaskId("Task ID to reassign: ");
        if (id == -1) return;
        cout << "  Assign to @username: ";
        string a; getline(cin, a);
        if (a.empty()) { cout << "  [!] No username entered.\n"; return; }
        findById(id)->setAssignee(a);
        fileManager.saveTasks(tasks);
        cout << "  [+] Task #" << id << " assigned to @" << a << "\n";
    }

    void searchTasks() const {
        cout << "  Keyword: ";
        string kw; getline(cin, kw);
        if (kw.empty()) { cout << "  [!] Empty keyword.\n"; return; }
        header("SEARCH: \"" + kw + "\"");
        string kwL = kw;
        transform(kwL.begin(), kwL.end(), kwL.begin(), ::tolower);
        bool found = false;
        for (auto &t : tasks) {
            string dl = t.getDesc();
            transform(dl.begin(), dl.end(), dl.begin(), ::tolower);
            if (dl.find(kwL) != string::npos) { t.print(); found = true; }
        }
        if (!found) cout << "  (No matching tasks)\n";
        printLine("-", 52);
    }

    void showOverdue() const {
        header("OVERDUE TASKS");
        bool found = false;
        for (auto &t : tasks)
            if (t.isOverdue()) { t.print(); found = true; }
        if (!found) cout << "  (No overdue tasks)\n";
        printLine("-", 52);
    }

    void showStats() const {
        int total = tasks.size(), done = 0, overdue = 0;
        for (auto &t : tasks) {
            if (t.getStatus() == "done") done++;
            if (t.isOverdue())           overdue++;
        }
        header("LIVE STATISTICS");
        cout << "  Logged in as : @" << currentUser    << "\n"
             << "  Total tasks  : "  << total           << "\n"
             << "  Done         : "  << done            << "\n"
             << "  Pending      : "  << (total - done)  << "\n"
             << "  Overdue      : "  << overdue         << "\n"
             << "  Users        : "  << users.size()    << "\n";
        printLine("-", 52);
    }

    void showUsers() const {
        header("REGISTERED USERS");
        for (auto &kv : users)
            kv.second->display();   // Polymorphism: RegisteredUser::display()
        printLine("-", 52);
    }

   
    //  CHAT  
    //    Step 1 — User A logs in, sends a message to User B
    //             -> written to smarttask_chat.txt
    //    Step 2 — User B logs in (same machine / shared folder)
    //             -> reads smarttask_chat.txt, sees the message
    //    It's like a shared notice board / message log.
   
    void sendChat() {
        if (users.size() <= 1) {
            cout << "  [!] No other users yet.\n"
                 << "      Ask your teammate to run the program\n"
                 << "      once so their account is registered.\n";
            return;
        }
        cout << "\n  Registered users:\n";
        for (auto &kv : users)
            if (kv.first != currentUser)
                cout << "    @" << kv.first << "\n";

        cout << "  Send to @: ";
        string receiver; getline(cin, receiver);
        if (receiver.empty()) { cout << "  Cancelled.\n"; return; }

        cout << "  Message  : ";
        string msg; getline(cin, msg);
        if (msg.empty()) { cout << "  [!] Empty message.\n"; return; }

        chatManager.sendMessage(currentUser, receiver, msg);
        cout << "  [+] Message sent to @" << receiver << "!\n";
    }

    void viewChat()   { chatManager.showHistory(); }
    void viewMyChat() { chatManager.showMyMessages(currentUser); }

    void clearChat() {
        cout << "  Clear ALL chat history? (y/n): ";
        string c; getline(cin, c);
        if (c == "y" || c == "Y") chatManager.clearHistory();
        else cout << "  Cancelled.\n";
    }

   
    //  HTML REPORT
   
    void generateReport() const {
        int total = tasks.size(), done = 0, overdue = 0;
        for (auto &t : tasks) {
            if (t.getStatus() == "done") done++;
            if (t.isOverdue())           overdue++;
        }
        int pending = total - done;

        ofstream f("report.html");
        f << R"(<!DOCTYPE html><html><head><meta charset="UTF-8">
<title>SmartTask Report</title>
<style>
  body{font-family:Arial,sans-serif;background:#1a1a2e;color:#eee;padding:30px;}
  h1{color:#e94560;text-align:center;}
  .cards{display:flex;gap:20px;justify-content:center;margin:20px 0;}
  .card{background:#16213e;border-radius:10px;padding:20px 30px;text-align:center;min-width:110px;}
  .card h2{font-size:2em;margin:0;}
  .t h2{color:#4fc3f7;} .d h2{color:#81c784;}
  .p h2{color:#ffb74d;} .o h2{color:#e57373;}
  table{width:100%;border-collapse:collapse;margin-top:24px;}
  th{background:#0f3460;padding:10px;text-align:left;}
  td{background:#16213e;padding:8px;border-bottom:1px solid #333;}
  .high{color:#e57373;font-weight:bold;}
  .medium{color:#ffb74d;} .low{color:#81c784;}
  .done-s{color:#81c784;} .pending-s{color:#ffb74d;}
  .overdue-s{color:#e57373;font-weight:bold;}
</style></head><body>
<h1>SmartTask Dashboard Report</h1>
<p style="text-align:center;color:#aaa;">Generated: )";
        f << currentDate() << R"(</p>
<div class="cards">
  <div class="card t"><h2>)" << total   << R"(</h2><p>Total</p></div>
  <div class="card d"><h2>)" << done    << R"(</h2><p>Done</p></div>
  <div class="card p"><h2>)" << pending << R"(</h2><p>Pending</p></div>
  <div class="card o"><h2>)" << overdue << R"(</h2><p>Overdue</p></div>
</div>
<table>
<tr><th>ID</th><th>Description</th><th>Owner</th><th>Assignee</th>
    <th>Priority</th><th>Deadline</th><th>Status</th></tr>
)";
        for (auto &t : tasks) {
            string sc = (t.getStatus()=="done") ? "done-s"
                      : t.isOverdue()           ? "overdue-s"
                      :                           "pending-s";
            string st = (t.getStatus()=="done") ? "Done"
                      : t.isOverdue()           ? "OVERDUE"
                      :                           "Pending";
            f << "<tr><td>"  << t.getId()
              << "</td><td>" << t.getDesc()
              << "</td><td>" << t.getOwner()
              << "</td><td>" << t.getAssignee()
              << "</td><td class='" << t.getPriority()
                             << "'>" << t.getPriority()
              << "</td><td>" << t.getDeadline()
              << "</td><td class='" << sc << "'>" << st
              << "</td></tr>\n";
        }
        f << "</table></body></html>\n";
        cout << "  [+] report.html generated — open in any browser.\n";
    }

   
    //  MENU
   
    void run() {
        if (!login()) return;

        while (true) {
            cout << "\n";
            printLine("=", 52);
            cout << "  SMARTTASK MENU   [@" << currentUser << "]\n";
            printLine("=", 52);
            cout << "  -- TASKS ---------------------------\n"
                 << "   1.  Add Task\n"
                 << "   2.  View All Tasks\n"
                 << "   3.  My Tasks\n"
                 << "   4.  Mark Task as Done\n"
                 << "   5.  Delete Task\n"
                 << "   6.  Set Priority\n"
                 << "   7.  Set Deadline\n"
                 << "   8.  Reassign Task\n"
                 << "   9.  Search Tasks\n"
                 << "  10.  Overdue Tasks\n"
                 << "  -- CHAT ----------------------------\n"
                 << "  11.  Send Message\n"
                 << "  12.  View All Chat\n"
                 << "  13.  View My Messages\n"
                 << "  14.  Clear Chat History\n"
                 << "  -- INFO ----------------------------\n"
                 << "  15.  Live Statistics\n"
                 << "  16.  Show Users\n"
                 << "  17.  Generate HTML Report\n"
                 << "   0.  Exit\n";
            printLine("-", 52);
            cout << "  Choice: ";

            int ch; cin >> ch; cin.ignore();
            cout << "\n";

            switch (ch) {
                case  1: addTask();        break;
                case  2: showTasks();      break;
                case  3: myTasks();        break;
                case  4: markDone();       break;
                case  5: deleteTask();     break;
                case  6: setPriority();    break;
                case  7: setDeadline();    break;
                case  8: assignTask();     break;
                case  9: searchTasks();    break;
                case 10: showOverdue();    break;
                case 11: sendChat();       break;
                case 12: viewChat();       break;
                case 13: viewMyChat();     break;
                case 14: clearChat();      break;
                case 15: showStats();      break;
                case 16: showUsers();      break;
                case 17: generateReport(); break;
                case  0:
                    cout << "  Goodbye, @" << currentUser << "!\n";
                    return;
                default:
                    cout << "  [!] Invalid choice. Try again.\n";
            }
        }
    }
};


//  MAIN

int main() {
    SmartTaskSystem system;   
    system.run();
    return 0;
}
/*
    STUDENT MANAGEMENT SYSTEM (C++) — OOP Showcase
    -------------------------------------------------
    OOP concepts demonstrated (marked in comments throughout):

      - ABSTRACTION      : Person is an abstract base class (pure virtual
                            functions) — you can never create a bare Person.
      - ENCAPSULATION     : all data is private/protected, accessed only
                            through validated getters/setters.
      - INHERITANCE       : Student and Teacher both derive from Person.
      - POLYMORPHISM      : displayInfo()/getRole() are called through
                            Person* pointers and resolve to the derived
                            class at runtime (virtual dispatch).
      - OPERATOR OVERLOAD : operator==, operator<< for Student.
      - STATIC MEMBERS    : Student::totalStudents, Teacher::totalTeachers
                            track counts across all instances.
      - COMPOSITION       : StudentManagementSystem "has-a" collection of
                            Students and Teachers it manages.

    Compile:  g++ -std=c++17 -o sms student_management_system.cpp
    Run:      ./sms
*/

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <map>
#include <memory>
#include <iomanip>
#include <algorithm>
#include <limits>

using namespace std;

const string STUDENTS_FILE = "students.txt";
const string TEACHERS_FILE = "teachers.txt";

// =================================================================
// ABSTRACTION + ENCAPSULATION: Person (abstract base class)
// =================================================================
class Person {
protected:
    string id;
    string name;
    int age;
    string contact;

public:
    Person(string id_, string name_, int age_, string contact_)
        : id(move(id_)), name(move(name_)), age(age_), contact(move(contact_)) {}

    virtual ~Person() = default;

    // ---- Encapsulation: controlled access with validation ----
    string getId() const { return id; }
    string getName() const { return name; }
    int getAge() const { return age; }
    string getContact() const { return contact; }

    void setName(const string& n) { if (!n.empty()) name = n; }
    void setAge(int a) { if (a > 0 && a < 120) age = a; }
    void setContact(const string& c) { contact = c; }

    // ---- Abstraction: derived classes MUST implement these ----
    virtual void displayInfo() const = 0;
    virtual string getRole() const = 0;
    virtual string toFileLine() const = 0;
};

// =================================================================
// Simple data holder (not part of the inheritance hierarchy)
// =================================================================
struct Course {
    string code;
    string title;
    int creditHours;
};

// =================================================================
// INHERITANCE + POLYMORPHISM: Student derives from Person
// =================================================================
class Student : public Person {
private:
    string rollNo;
    string department;
    int semester;
    map<string, double> courseGrades;   // courseCode -> marks (0-100)

    static int totalStudents;            // STATIC MEMBER: shared by all Students

public:
    Student(string id_, string name_, int age_, string contact_,
            string rollNo_, string department_, int semester_)
        : Person(move(id_), move(name_), age_, move(contact_)),
          rollNo(move(rollNo_)), department(move(department_)), semester(semester_) {
        totalStudents++;
    }

    // Rule of five: a vector<Student> silently copies/moves elements on
    // reallocation. Every one of those must count as a "new" live object
    // (and increment totalStudents) so the static counter stays accurate
    // no matter how the vector reshuffles memory internally.
    Student(const Student& other)
        : Person(other), rollNo(other.rollNo), department(other.department),
          semester(other.semester), courseGrades(other.courseGrades) {
        totalStudents++;
    }
    Student(Student&& other) noexcept
        : Person(other), rollNo(move(other.rollNo)), department(move(other.department)),
          semester(other.semester), courseGrades(move(other.courseGrades)) {
        totalStudents++;
    }
    Student& operator=(const Student&) = default;
    Student& operator=(Student&&) noexcept = default;

    ~Student() override { totalStudents--; }

    // getters / validated setters
    string getRollNo() const { return rollNo; }
    string getDepartment() const { return department; }
    int getSemester() const { return semester; }
    const map<string, double>& getCourseGrades() const { return courseGrades; }

    void setSemester(int s) { if (s > 0 && s <= 12) semester = s; }
    void setDepartment(const string& d) { if (!d.empty()) department = d; }

    void addOrUpdateGrade(const string& courseCode, double marks) {
        marks = max(0.0, min(100.0, marks));
        courseGrades[courseCode] = marks;
    }

    static double marksToGradePoint(double marks) {
        if (marks >= 90) return 4.0;
        if (marks >= 80) return 3.7;
        if (marks >= 70) return 3.3;
        if (marks >= 60) return 3.0;
        if (marks >= 50) return 2.0;
        return 0.0;
    }

    double calculateGPA() const {
        if (courseGrades.empty()) return 0.0;
        double total = 0;
        for (auto& kv : courseGrades) total += marksToGradePoint(kv.second);
        return total / courseGrades.size();
    }

    // ---- POLYMORPHISM: overrides of the base class's pure virtuals ----
    void displayInfo() const override {
        cout << left << setw(8) << id << setw(18) << name << setw(5) << age
             << setw(10) << rollNo << setw(20) << department << setw(5) << semester
             << fixed << setprecision(2) << setw(6) << calculateGPA() << "\n";
    }

    string getRole() const override { return "Student"; }

    string toFileLine() const override {
        ostringstream oss;
        oss << id << "|" << name << "|" << age << "|" << contact << "|"
            << rollNo << "|" << department << "|" << semester << "|";
        bool first = true;
        for (auto& kv : courseGrades) {
            if (!first) oss << ",";
            oss << kv.first << ":" << kv.second;
            first = false;
        }
        return oss.str();
    }

    static int getTotalStudents() { return totalStudents; }

    // ---- OPERATOR OVERLOADING ----
    bool operator==(const Student& other) const { return id == other.id; }

    friend ostream& operator<<(ostream& os, const Student& s) {
        os << s.rollNo << " - " << s.name << " (" << s.department << ")";
        return os;
    }
};
int Student::totalStudents = 0;   // definition of the static member

// =================================================================
// INHERITANCE + POLYMORPHISM: Teacher also derives from Person
// (shows that two very different classes can share one interface)
// =================================================================
class Teacher : public Person {
private:
    string department;
    string subject;
    double salary;

    static int totalTeachers;

public:
    Teacher(string id_, string name_, int age_, string contact_,
            string department_, string subject_, double salary_)
        : Person(move(id_), move(name_), age_, move(contact_)),
          department(move(department_)), subject(move(subject_)), salary(salary_) {
        totalTeachers++;
    }

    Teacher(const Teacher& other)
        : Person(other), department(other.department), subject(other.subject),
          salary(other.salary) {
        totalTeachers++;
    }
    Teacher(Teacher&& other) noexcept
        : Person(other), department(move(other.department)), subject(move(other.subject)),
          salary(other.salary) {
        totalTeachers++;
    }
    Teacher& operator=(const Teacher&) = default;
    Teacher& operator=(Teacher&&) noexcept = default;

    ~Teacher() override { totalTeachers--; }

    string getDepartment() const { return department; }
    string getSubject() const { return subject; }
    double getSalary() const { return salary; }
    void setSalary(double s) { if (s >= 0) salary = s; }

    void displayInfo() const override {
        cout << left << setw(8) << id << setw(18) << name << setw(5) << age
             << setw(20) << department << setw(16) << subject
             << fixed << setprecision(0) << salary << "\n";
    }

    string getRole() const override { return "Teacher"; }

    string toFileLine() const override {
        ostringstream oss;
        oss << id << "|" << name << "|" << age << "|" << contact << "|"
            << department << "|" << subject << "|" << salary;
        return oss.str();
    }

    static int getTotalTeachers() { return totalTeachers; }
};
int Teacher::totalTeachers = 0;

// =================================================================
// HELPERS
// =================================================================

int readIntChoice() {
    int choice;
    while (!(cin >> choice)) {
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        cout << "Invalid input. Enter a number: ";
    }
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    return choice;
}

double readDoubleChoice() {
    double val;
    while (!(cin >> val)) {
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        cout << "Invalid input. Enter a number: ";
    }
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    return val;
}

string readLine(const string& prompt) {
    cout << prompt;
    string line;
    getline(cin, line);
    return line;
}

vector<string> splitStr(const string& s, char delim) {
    vector<string> out;
    stringstream ss(s);
    string item;
    while (getline(ss, item, delim)) out.push_back(item);
    if (!s.empty() && s.back() == delim) out.push_back("");
    return out;
}

// =================================================================
// COMPOSITION: StudentManagementSystem "has-a" set of Students/Teachers
// =================================================================
class StudentManagementSystem {
private:
    vector<Student> students;
    vector<Teacher> teachers;

public:
    StudentManagementSystem() { loadStudents(); loadTeachers(); }
    ~StudentManagementSystem() { saveStudents(); saveTeachers(); }

    // ---------------- FILE I/O ----------------

    void loadStudents() {
        ifstream in(STUDENTS_FILE);
        if (!in) return;
        string line;
        while (getline(in, line)) {
            if (line.empty()) continue;
            auto f = splitStr(line, '|');
            if (f.size() < 7) continue;
            Student s(f[0], f[1], stoi(f[2]), f[3], f[4], f[5], stoi(f[6]));
            if (f.size() > 7 && !f[7].empty()) {
                for (auto& pair_ : splitStr(f[7], ',')) {
                    auto kv = splitStr(pair_, ':');
                    if (kv.size() == 2) s.addOrUpdateGrade(kv[0], stod(kv[1]));
                }
            }
            students.push_back(s);
        }
    }

    void saveStudents() {
        ofstream out(STUDENTS_FILE, ios::trunc);
        for (auto& s : students) out << s.toFileLine() << "\n";
    }

    void loadTeachers() {
        ifstream in(TEACHERS_FILE);
        if (!in) return;
        string line;
        while (getline(in, line)) {
            if (line.empty()) continue;
            auto f = splitStr(line, '|');
            if (f.size() < 7) continue;
            teachers.emplace_back(f[0], f[1], stoi(f[2]), f[3], f[4], f[5], stod(f[6]));
        }
    }

    void saveTeachers() {
        ofstream out(TEACHERS_FILE, ios::trunc);
        for (auto& t : teachers) out << t.toFileLine() << "\n";
    }

    // ---------------- STUDENT OPERATIONS ----------------

    Student* findStudentById(const string& id) {
        for (auto& s : students) if (s.getId() == id) return &s;
        return nullptr;
    }

    void addStudent() {
        cout << "\n--- Add Student ---\n";
        string id = readLine("Student ID: ");
        if (findStudentById(id)) { cout << "A student with this ID already exists.\n"; return; }
        string name = readLine("Name: ");
        cout << "Age: "; int age = readIntChoice();
        string contact = readLine("Contact: ");
        string rollNo = readLine("Roll No: ");
        string dept = readLine("Department: ");
        cout << "Semester: "; int sem = readIntChoice();

        students.emplace_back(id, name, age, contact, rollNo, dept, sem);
        cout << "Student added successfully.\n";
    }

    void viewAllStudents() const {
        cout << "\n--- All Students ---\n";
        if (students.empty()) { cout << "No students registered yet.\n"; return; }
        cout << left << setw(8) << "ID" << setw(18) << "Name" << setw(5) << "Age"
             << setw(10) << "Roll No" << setw(20) << "Dept" << setw(5) << "Sem"
             << setw(6) << "GPA" << "\n";
        cout << string(66, '-') << "\n";
        // POLYMORPHISM: each call resolves to Student::displayInfo() at runtime
        for (auto& s : students) s.displayInfo();
        cout << "\n(Total students: " << Student::getTotalStudents() << ")\n";
    }

    void searchStudent() {
        cout << "\n--- Search Student ---\n";
        string query = readLine("Enter ID or Roll No: ");
        for (auto& s : students) {
            if (s.getId() == query || s.getRollNo() == query) {
                cout << "\nFound: " << s << "\n";  // uses overloaded operator<<
                cout << "Age: " << s.getAge() << " | Contact: " << s.getContact()
                     << " | Semester: " << s.getSemester()
                     << " | GPA: " << fixed << setprecision(2) << s.calculateGPA() << "\n";
                if (!s.getCourseGrades().empty()) {
                    cout << "Courses:\n";
                    for (auto& kv : s.getCourseGrades())
                        cout << "  " << kv.first << " : " << kv.second << "\n";
                }
                return;
            }
        }
        cout << "No matching student found.\n";
    }

    void updateStudent() {
        cout << "\n--- Update Student ---\n";
        string id = readLine("Student ID to update: ");
        Student* s = findStudentById(id);
        if (!s) { cout << "Student not found.\n"; return; }

        cout << "Leave blank to keep the current value.\n";
        string name = readLine("New name [" + s->getName() + "]: ");
        if (!name.empty()) s->setName(name);

        string contact = readLine("New contact [" + s->getContact() + "]: ");
        if (!contact.empty()) s->setContact(contact);

        string semStr = readLine("New semester [" + to_string(s->getSemester()) + "]: ");
        if (!semStr.empty()) s->setSemester(stoi(semStr));

        cout << "Student updated successfully.\n";
    }

    void deleteStudent() {
        cout << "\n--- Delete Student ---\n";
        string id = readLine("Student ID to delete: ");
        auto it = remove_if(students.begin(), students.end(),
                             [&](const Student& s) { return s.getId() == id; });
        if (it == students.end()) { cout << "Student not found.\n"; return; }
        students.erase(it, students.end());
        cout << "Student deleted successfully.\n";
    }

    void assignGrade() {
        cout << "\n--- Assign / Update Grade ---\n";
        string id = readLine("Student ID: ");
        Student* s = findStudentById(id);
        if (!s) { cout << "Student not found.\n"; return; }
        string course = readLine("Course code: ");
        cout << "Marks (0-100): "; double marks = readDoubleChoice();
        s->addOrUpdateGrade(course, marks);
        cout << "Grade recorded. Updated GPA: " << fixed << setprecision(2)
             << s->calculateGPA() << "\n";
    }

    void gpaReport() const {
        cout << "\n--- GPA Ranking Report ---\n";
        if (students.empty()) { cout << "No students to rank.\n"; return; }
        vector<const Student*> ranked;
        for (auto& s : students) ranked.push_back(&s);
        sort(ranked.begin(), ranked.end(),
             [](const Student* a, const Student* b) { return a->calculateGPA() > b->calculateGPA(); });

        cout << left << setw(6) << "Rank" << setw(10) << "Roll No" << setw(18) << "Name"
             << "GPA\n" << string(45, '-') << "\n";
        int rank = 1;
        for (auto* s : ranked) {
            cout << left << setw(6) << rank++ << setw(10) << s->getRollNo()
                 << setw(18) << s->getName() << fixed << setprecision(2)
                 << s->calculateGPA() << "\n";
        }
    }

    // ---------------- TEACHER OPERATIONS ----------------

    void addTeacher() {
        cout << "\n--- Add Teacher ---\n";
        string id = readLine("Teacher ID: ");
        for (auto& t : teachers)
            if (t.getId() == id) { cout << "A teacher with this ID already exists.\n"; return; }
        string name = readLine("Name: ");
        cout << "Age: "; int age = readIntChoice();
        string contact = readLine("Contact: ");
        string dept = readLine("Department: ");
        string subject = readLine("Subject: ");
        cout << "Salary: "; double salary = readDoubleChoice();

        teachers.emplace_back(id, name, age, contact, dept, subject, salary);
        cout << "Teacher added successfully.\n";
    }

    void viewAllTeachers() const {
        cout << "\n--- All Teachers ---\n";
        if (teachers.empty()) { cout << "No teachers registered yet.\n"; return; }
        cout << left << setw(8) << "ID" << setw(18) << "Name" << setw(5) << "Age"
             << setw(20) << "Dept" << setw(16) << "Subject" << "Salary\n";
        cout << string(70, '-') << "\n";
        for (auto& t : teachers) t.displayInfo();
        cout << "\n(Total teachers: " << Teacher::getTotalTeachers() << ")\n";
    }

    // ---------------- POLYMORPHISM DEMO ----------------
    // Treats Students and Teachers uniformly through Person* — the actual
    // displayInfo()/getRole() that runs depends on the real object type,
    // decided at runtime (this is the core idea of runtime polymorphism).
    void displayEveryonePolymorphically() const {
        cout << "\n--- Everyone (via base Person* pointers) ---\n";
        vector<const Person*> people;
        for (auto& s : students) people.push_back(&s);
        for (auto& t : teachers) people.push_back(&t);

        if (people.empty()) { cout << "No records yet.\n"; return; }
        for (auto* p : people) {
            cout << "[" << p->getRole() << "] ";
            p->displayInfo();   // virtual dispatch: Student:: or Teacher::
        }
    }
};

// =================================================================
// MENU
// =================================================================

void printMainMenu() {
    cout << "\n==================================================\n";
    cout << "         STUDENT MANAGEMENT SYSTEM (OOP)            \n";
    cout << "==================================================\n";
    cout << " 1. Add Student\n";
    cout << " 2. View All Students\n";
    cout << " 3. Search Student\n";
    cout << " 4. Update Student\n";
    cout << " 5. Delete Student\n";
    cout << " 6. Assign / Update Grade\n";
    cout << " 7. GPA Ranking Report\n";
    cout << " 8. Add Teacher\n";
    cout << " 9. View All Teachers\n";
    cout << "10. View Everyone (Polymorphism Demo)\n";
    cout << " 0. Exit\n";
    cout << "--------------------------------------------------\n";
    cout << "Enter your choice: ";
}

int main() {
    StudentManagementSystem sms;
    bool running = true;

    cout << "Welcome to the Student Management System!\n";

    while (running) {
        printMainMenu();
        int choice = readIntChoice();

        switch (choice) {
            case 1: sms.addStudent(); break;
            case 2: sms.viewAllStudents(); break;
            case 3: sms.searchStudent(); break;
            case 4: sms.updateStudent(); break;
            case 5: sms.deleteStudent(); break;
            case 6: sms.assignGrade(); break;
            case 7: sms.gpaReport(); break;
            case 8: sms.addTeacher(); break;
            case 9: sms.viewAllTeachers(); break;
            case 10: sms.displayEveryonePolymorphically(); break;
            case 0: running = false; cout << "\nData saved. Goodbye!\n"; break;
            default: cout << "Invalid choice. Please try again.\n";
        }
    }
    return 0;
}

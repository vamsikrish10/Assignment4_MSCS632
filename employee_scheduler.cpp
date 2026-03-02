/*
 * Assignment 4: Employee Schedule Manager (C++)
 * Demonstrates: conditionals, loops, branching, structs, vectors, maps, randomization.
 *
 * Compile:  g++ -std=c++17 -o scheduler employee_scheduler.cpp
 * Run:      ./scheduler
 */

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <algorithm>
#include <random>
#include <iomanip>
#include <sstream>

// ─────────────────────────────────────────────
// CONSTANTS
// ─────────────────────────────────────────────
const std::vector<std::string> DAYS   = {"Monday","Tuesday","Wednesday","Thursday",
                                          "Friday","Saturday","Sunday"};
const std::vector<std::string> SHIFTS = {"morning","afternoon","evening"};
const int MAX_DAYS_PER_WEEK       = 5;
const int MIN_EMPLOYEES_PER_SHIFT = 2;

// ─────────────────────────────────────────────
// DATA STRUCTURES
// ─────────────────────────────────────────────
struct Employee {
    std::string name;
    // preferences[day] = ordered list of preferred shifts (index 0 = highest priority)
    std::map<std::string, std::vector<std::string>> preferences;
    int daysWorked = 0;
};

// schedule[day][shift] = list of employee names
std::map<std::string, std::map<std::string, std::vector<std::string>>> schedule;

// ─────────────────────────────────────────────
// HELPER UTILITIES
// ─────────────────────────────────────────────
// Trim whitespace from both ends of a string
std::string trim(const std::string& s) {
    size_t start = s.find_first_not_of(" \t\r\n");
    size_t end   = s.find_last_not_of(" \t\r\n");
    return (start == std::string::npos) ? "" : s.substr(start, end - start + 1);
}

// Convert string to lowercase
std::string toLower(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(), ::tolower);
    return s;
}

// Split a string by a delimiter
std::vector<std::string> split(const std::string& s, char delim) {
    std::vector<std::string> tokens;
    std::stringstream ss(s);
    std::string token;
    while (std::getline(ss, token, delim)) {
        std::string t = trim(token);
        if (!t.empty()) tokens.push_back(t);
    }
    return tokens;
}

// Check whether a shift string is valid
bool isValidShift(const std::string& s) {
    return std::find(SHIFTS.begin(), SHIFTS.end(), s) != SHIFTS.end();
}

// Check if employee is already assigned on a given day
bool isWorkingOnDay(const Employee& emp, const std::string& day) {
    for (const auto& shift : SHIFTS) {
        const auto& workers = schedule[day][shift];
        if (std::find(workers.begin(), workers.end(), emp.name) != workers.end())
            return true;
    }
    return false;
}

// ─────────────────────────────────────────────
// SCHEDULE INITIALIZATION
// ─────────────────────────────────────────────
void initSchedule() {
    for (const auto& day : DAYS)
        for (const auto& shift : SHIFTS)
            schedule[day][shift] = {};
}

// ─────────────────────────────────────────────
// ASSIGN SHIFT
// Returns true if assignment was successful.
// ─────────────────────────────────────────────
bool assignShift(Employee& emp, const std::string& day, const std::string& shift) {
    if (emp.daysWorked >= MAX_DAYS_PER_WEEK) return false;
    if (isWorkingOnDay(emp, day))            return false;

    schedule[day][shift].push_back(emp.name);
    emp.daysWorked++;
    return true;
}

// ─────────────────────────────────────────────
// NEXT-DAY CONFLICT RESOLUTION
// ─────────────────────────────────────────────
void tryNextDayAssignment(Employee& emp, const std::string& originalDay) {
    auto it = std::find(DAYS.begin(), DAYS.end(), originalDay);
    if (it == DAYS.end() || std::next(it) == DAYS.end()) return;

    const std::string& nextDay = *std::next(it);
    const auto& prefs = emp.preferences.count(nextDay)
                        ? emp.preferences.at(nextDay)
                        : SHIFTS;

    for (const auto& shift : prefs) {
        if (assignShift(emp, nextDay, shift)) {
            std::cout << "  Conflict resolved: " << emp.name
                      << " moved from " << originalDay
                      << " -> " << nextDay << " (" << shift << ")\n";
            return;
        }
    }
}

// ─────────────────────────────────────────────
// BUILD SCHEDULE — honor employee preferences
// ─────────────────────────────────────────────
void buildSchedule(std::vector<Employee>& employees) {
    std::cout << "\nBuilding schedule...\n";

    for (const auto& day : DAYS) {
        for (auto& emp : employees) {
            // Get preference list for this day; skip if unavailable
            if (!emp.preferences.count(day)) continue;
            const auto& prefs = emp.preferences.at(day);
            if (prefs.empty()) continue;

            bool assigned = false;
            for (const auto& shift : prefs) {
                if (assignShift(emp, day, shift)) {
                    assigned = true;
                    break;
                }
            }

            if (!assigned) {
                // Conflict: attempt to place on next available day
                tryNextDayAssignment(emp, day);
            }
        }
    }
}

// ─────────────────────────────────────────────
// FILL UNDERSTAFFED SHIFTS
// ─────────────────────────────────────────────
void fillUnderstaffedShifts(std::vector<Employee>& employees) {
    std::cout << "Checking for understaffed shifts...\n";

    std::mt19937 rng(std::random_device{}());
    bool anyFilled = false;

    for (const auto& day : DAYS) {
        for (const auto& shift : SHIFTS) {
            while ((int)schedule[day][shift].size() < MIN_EMPLOYEES_PER_SHIFT) {
                // Collect eligible employees
                std::vector<int> eligible;
                for (int i = 0; i < (int)employees.size(); ++i) {
                    Employee& e = employees[i];
                    if (e.daysWorked < MAX_DAYS_PER_WEEK && !isWorkingOnDay(e, day))
                        eligible.push_back(i);
                }

                if (eligible.empty()) {
                    std::cout << "  WARNING: Cannot fill " << day << " " << shift
                              << " — no eligible employees remaining.\n";
                    break;
                }

                // Pick a random eligible employee
                std::uniform_int_distribution<int> dist(0, (int)eligible.size() - 1);
                int idx = eligible[dist(rng)];
                assignShift(employees[idx], day, shift);
                std::cout << "  Auto-assigned " << employees[idx].name
                          << " -> " << day << " " << shift << " (understaffed fill)\n";
                anyFilled = true;
            }
        }
    }

    if (!anyFilled)
        std::cout << "  All shifts sufficiently staffed.\n";
}

// ─────────────────────────────────────────────
// OUTPUT — final schedule
// ─────────────────────────────────────────────
void printSchedule(const std::vector<Employee>& employees) {
    std::cout << "\n" << std::string(60, '=') << "\n";
    std::cout << "        FINAL WEEKLY SCHEDULE\n";
    std::cout << std::string(60, '=') << "\n";

    for (const auto& day : DAYS) {
        std::cout << "\n  " << day << "\n";
        std::cout << "  " << std::string(40, '-') << "\n";
        for (const auto& shift : SHIFTS) {
            const auto& workers = schedule.at(day).at(shift);
            std::cout << "    " << std::left << std::setw(12)
                      << (shift[0] >= 'a' && shift[0] <= 'z'
                          ? std::string(1, (char)(shift[0]-32)) + shift.substr(1)
                          : shift)
                      << ": ";
            if (workers.empty()) {
                std::cout << "(no one assigned)";
            } else {
                for (size_t i = 0; i < workers.size(); ++i) {
                    if (i) std::cout << ", ";
                    std::cout << workers[i];
                }
            }
            std::cout << "\n";
        }
    }

    std::cout << "\n" << std::string(60, '=') << "\n";
    std::cout << "  EMPLOYEE SUMMARY (days worked this week)\n";
    std::cout << std::string(60, '=') << "\n";
    for (const auto& emp : employees) {
        std::cout << "    " << std::left << std::setw(20)
                  << emp.name << ": " << emp.daysWorked << " day(s)\n";
    }
    std::cout << "\n";
}

// ─────────────────────────────────────────────
// INPUT COLLECTION
// ─────────────────────────────────────────────
std::vector<Employee> collectEmployeeData() {
    std::vector<Employee> employees;

    std::cout << "\n" << std::string(60, '=') << "\n";
    std::cout << "  EMPLOYEE SCHEDULE MANAGER — Input Phase\n";
    std::cout << std::string(60, '=') << "\n";

    int numEmployees = 0;
    while (numEmployees < 1) {
        std::cout << "\nHow many employees to schedule? ";
        std::string line;
        std::getline(std::cin, line);
        try {
            numEmployees = std::stoi(line);
            if (numEmployees < 1) {
                std::cout << "  Please enter at least 1.\n";
                numEmployees = 0;
            }
        } catch (...) {
            std::cout << "  Invalid input. Please enter a whole number.\n";
        }
    }

    for (int i = 0; i < numEmployees; ++i) {
        Employee emp;
        std::cout << "\n--- Employee " << (i + 1) << " ---\n";

        do {
            std::cout << "  Enter employee name: ";
            std::getline(std::cin, emp.name);
            emp.name = trim(emp.name);
            if (emp.name.empty()) std::cout << "  Name cannot be empty.\n";
        } while (emp.name.empty());

        std::cout << "  Enter shift preferences for " << emp.name << ".\n";
        std::cout << "  Shifts: morning, afternoon, evening\n";
        std::cout << "  Comma-separated priority list (e.g. 'morning,evening').\n";
        std::cout << "  Leave blank to mark a day as unavailable.\n\n";

        for (const auto& day : DAYS) {
            std::cout << "    " << day << ": ";
            std::string line;
            std::getline(std::cin, line);
            line = toLower(trim(line));

            if (line.empty()) {
                emp.preferences[day] = {};   // unavailable
            } else {
                auto tokens = split(line, ',');
                std::vector<std::string> ordered;
                std::vector<bool> seen(SHIFTS.size(), false);

                for (auto& t : tokens) {
                    t = toLower(trim(t));
                    auto it = std::find(SHIFTS.begin(), SHIFTS.end(), t);
                    if (it != SHIFTS.end()) {
                        size_t idx = it - SHIFTS.begin();
                        if (!seen[idx]) {
                            seen[idx] = true;
                            ordered.push_back(t);
                        }
                    }
                }
                // Append remaining shifts at lowest priority
                for (size_t k = 0; k < SHIFTS.size(); ++k)
                    if (!seen[k]) ordered.push_back(SHIFTS[k]);

                emp.preferences[day] = ordered;
            }
        }
        employees.push_back(emp);
    }

    std::cout << "\nEmployee data collected successfully.\n";
    return employees;
}

// ─────────────────────────────────────────────
// DEMO MODE — pre-loaded sample data
// ─────────────────────────────────────────────
std::vector<Employee> loadDemoData() {
    std::vector<std::string> names = {"Alice","Bob","Carol","Dave","Eve","Frank","Grace"};
    std::vector<Employee> employees;

    std::mt19937 rng(42);   // fixed seed for reproducibility

    for (const auto& name : names) {
        Employee emp;
        emp.name = name;

        for (const auto& day : DAYS) {
            // ~85% chance the employee is available
            std::uniform_real_distribution<double> prob(0.0, 1.0);
            if (prob(rng) < 0.85) {
                std::vector<std::string> prefs = SHIFTS;
                std::shuffle(prefs.begin(), prefs.end(), rng);
                emp.preferences[day] = prefs;
            } else {
                emp.preferences[day] = {};  // unavailable
            }
        }
        employees.push_back(emp);
    }

    std::cout << "Demo data loaded for " << employees.size() << " employees.\n";
    return employees;
}

// ─────────────────────────────────────────────
// MAIN
// ─────────────────────────────────────────────
int main() {
    std::cout << "\nEmployee Schedule Manager\n";
    std::cout << "Select mode:\n";
    std::cout << "  1 — Interactive (enter employee data manually)\n";
    std::cout << "  2 — Demo       (use pre-loaded sample data)\n";
    std::cout << "\nChoice [1/2]: ";

    std::string choice;
    std::getline(std::cin, choice);
    choice = trim(choice);

    initSchedule();

    std::vector<Employee> employees;
    if (choice == "1") {
        employees = collectEmployeeData();
    } else {
        std::cout << "\nLoading demo data...\n";
        employees = loadDemoData();
    }

    buildSchedule(employees);
    fillUnderstaffedShifts(employees);
    printSchedule(employees);

    return 0;
}

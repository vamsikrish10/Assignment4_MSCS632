"""
Assignment 4: Employee Schedule Manager (PyQt5 GUI Version)
Enhanced Version: Professional GUI, multi-day selection, proper schedule display
"""

import sys
import random
from collections import defaultdict
from PyQt5.QtWidgets import (
    QApplication, QWidget, QVBoxLayout, QHBoxLayout, QFormLayout,
    QPushButton, QLabel, QLineEdit, QMessageBox,
    QTableWidget, QTableWidgetItem, QListWidget, QListWidgetItem,
    QGroupBox, QComboBox
)
from PyQt5.QtGui import QFont, QBrush, QColor
from PyQt5.QtCore import Qt

# ─────────────────────────────────────────────
# CONSTANTS
# ─────────────────────────────────────────────
DAYS = ["Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday", "Sunday"]
SHIFTS = ["morning", "afternoon", "evening", "night"]
MAX_DAYS_PER_WEEK = 5
MIN_EMPLOYEES_PER_SHIFT = 2

# ─────────────────────────────────────────────
# DATA STRUCTURES
# ─────────────────────────────────────────────
schedule = {day: {shift: [] for shift in SHIFTS} for day in DAYS}
days_worked = defaultdict(int)
employee_preferences = {}


# ─────────────────────────────────────────────
# SCHEDULING LOGIC
# ─────────────────────────────────────────────
def reset_data():
    global schedule, days_worked
    schedule = {day: {shift: [] for shift in SHIFTS} for day in DAYS}
    days_worked = defaultdict(int)


def assign_shift(employee, day, shift):
    if days_worked[employee] >= MAX_DAYS_PER_WEEK:
        return False
    for s in SHIFTS:
        if employee in schedule[day][s]:
            return False
    schedule[day][shift].append(employee)
    days_worked[employee] += 1
    return True


def build_schedule(employees):
    for day in DAYS:
        for employee in employees:
            prefs = employee_preferences.get(employee, {}).get(day, [])
            if not prefs:
                continue
            for preferred_shift in prefs:
                if assign_shift(employee, day, preferred_shift):
                    break


def fill_understaffed_shifts(employees):
    for day in DAYS:
        for shift in SHIFTS:
            while len(schedule[day][shift]) < MIN_EMPLOYEES_PER_SHIFT:
                eligible = [
                    e for e in employees
                    if days_worked[e] < MAX_DAYS_PER_WEEK
                    and all(e not in schedule[day][s] for s in SHIFTS)
                ]
                if not eligible:
                    break
                chosen = random.choice(eligible)
                assign_shift(chosen, day, shift)


def load_demo_data():
    """Preload demo employees and preferences"""
    reset_data()
    employee_preferences.clear()
    demo_employees = ["Alice", "Bob", "Carol", "Dave", "Eve", "Frank", "Grace", "Henry", "Ivy", "Jack"]
    random.seed(42)
    for emp in demo_employees:
        employee_preferences[emp] = {}
        for day in DAYS:
            if random.random() < 0.85:
                prefs = SHIFTS[:]
                random.shuffle(prefs)
                employee_preferences[emp][day] = prefs
            else:
                employee_preferences[emp][day] = []
    return demo_employees


# ─────────────────────────────────────────────
# GUI APPLICATION
# ─────────────────────────────────────────────
class ScheduleApp(QWidget):
    def __init__(self):
        super().__init__()
        self.setWindowTitle("Employee Schedule Manager")
        self.setGeometry(100, 100, 1200, 700)
        self.employees = []

        main_layout = QVBoxLayout()

        # ── Employee Input Section ──
        emp_group = QGroupBox("Add Employee")
        emp_group.setFont(QFont("Arial", 12, QFont.Bold))
        emp_layout = QFormLayout()

        # Name Input
        self.name_input = QLineEdit()
        emp_layout.addRow(QLabel("Employee Name:"), self.name_input)

        # Multi-select days
        self.days_list = QListWidget()
        self.days_list.setSelectionMode(QListWidget.MultiSelection)
        for day in DAYS:
            QListWidgetItem(day, self.days_list)
        emp_layout.addRow(QLabel(f"Select Day(s) (max {MAX_DAYS_PER_WEEK}):"), self.days_list)

        # Shift preference dropdown
        self.shift_combo = QComboBox()
        self.shift_combo.addItems(SHIFTS)
        emp_layout.addRow(QLabel("Shift Preference:"), self.shift_combo)

        emp_group.setLayout(emp_layout)
        main_layout.addWidget(emp_group)

        # ── Buttons Section ──
        btn_layout = QHBoxLayout()
        self.add_btn = QPushButton("Add Employee")
        self.demo_btn = QPushButton("Load Demo Data")
        self.generate_btn = QPushButton("Generate Schedule")
        btn_layout.addWidget(self.add_btn)
        btn_layout.addWidget(self.demo_btn)
        btn_layout.addWidget(self.generate_btn)
        main_layout.addLayout(btn_layout)

        # ── Schedule Table ──
        self.table = QTableWidget()
        self.table.setRowCount(len(DAYS))
        self.table.setColumnCount(len(SHIFTS))
        self.table.setHorizontalHeaderLabels([s.capitalize() for s in SHIFTS])
        self.table.setVerticalHeaderLabels(DAYS)
        self.table.setWordWrap(True)
        self.table.setVerticalScrollBarPolicy(Qt.ScrollBarAsNeeded)
        self.table.setHorizontalScrollBarPolicy(Qt.ScrollBarAsNeeded)
        self.table.setMinimumHeight(400)
        main_layout.addWidget(self.table)

        # ── Employee Summary ──
        self.summary = QListWidget()
        main_layout.addWidget(self.summary)

        self.setLayout(main_layout)

        # ── Connect buttons ──
        self.add_btn.clicked.connect(self.add_employee)
        self.demo_btn.clicked.connect(self.load_demo)
        self.generate_btn.clicked.connect(self.generate_schedule)

    # ── Button Handlers ──
    def add_employee(self):
        name = self.name_input.text().strip()
        if not name:
            QMessageBox.warning(self, "Error", "Employee name cannot be empty.")
            return

        selected_items = self.days_list.selectedItems()
        if not selected_items:
            QMessageBox.warning(self, "Error", "Select at least one day.")
            return

        if len(selected_items) > MAX_DAYS_PER_WEEK:
            QMessageBox.warning(self, "Error", f"Cannot select more than {MAX_DAYS_PER_WEEK} days.")
            return

        shift = self.shift_combo.currentText()

        if name not in employee_preferences:
            employee_preferences[name] = {d: [] for d in DAYS}
            self.employees.append(name)

        for item in selected_items:
            day = item.text()
            if shift not in employee_preferences[name][day]:
                employee_preferences[name][day].append(shift)

        QMessageBox.information(self, "Success", f"{name} added for {[i.text() for i in selected_items]} → {shift}")
        self.name_input.clear()
        self.days_list.clearSelection()

    def load_demo(self):
        """Load demo employees AND automatically generate their schedule"""
        self.employees = load_demo_data()
        build_schedule(self.employees)
        fill_understaffed_shifts(self.employees)
        self.display_schedule()
        QMessageBox.information(self, "Demo Loaded", "Demo employees loaded and schedule generated.")

    def generate_schedule(self):
        if not self.employees:
            QMessageBox.warning(self, "Error", "No employees added.")
            return
        reset_data()
        build_schedule(self.employees)
        fill_understaffed_shifts(self.employees)
        self.display_schedule()

    def display_schedule(self):
        self.table.clearContents()
        for row, day in enumerate(DAYS):
            for col, shift in enumerate(SHIFTS):
                workers = schedule[day][shift]
                text = ", ".join(workers) if workers else "(no one assigned)"
                item = QTableWidgetItem(text)
                item.setTextAlignment(Qt.AlignLeft | Qt.AlignTop)
                item.setFlags(item.flags() ^ Qt.ItemIsEditable)
                item.setToolTip(text)
                if row % 2 == 0:
                    item.setBackground(QBrush(QColor(240, 240, 240)))
                else:
                    item.setBackground(QBrush(QColor(255, 255, 255)))
                self.table.setItem(row, col, item)

        self.table.resizeColumnsToContents()
        self.table.resizeRowsToContents()

        self.summary.clear()
        for emp, count in sorted(days_worked.items()):
            self.summary.addItem(f"{emp}: {count} day(s)")


# ─────────────────────────────────────────────
# MAIN
# ─────────────────────────────────────────────
if __name__ == "__main__":
    app = QApplication(sys.argv)
    window = ScheduleApp()
    window.show()
    sys.exit(app.exec_())
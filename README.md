# Smart-Task-Chat-System-
OOP Project

## Overview
SmartTask Chat System is a C++ console-based task management application developed using Object-Oriented Programming (OOP) concepts. The system allows users to manage tasks, communicate through a simple chat system, and generate project reports.

## Features

- User Registration and Login
- Task Creation and Management
- Task Assignment to Users
- Priority Management (High, Medium, Low)
- Deadline Tracking
- Overdue Task Detection
- Chat System
- Live Statistics
- HTML Report Generation
- File Handling for Persistent Storage

## OOP Concepts Used

### Encapsulation
Data members are kept private and accessed through public methods.

### Inheritance
`RegisteredUser` inherits from the `User` base class.

### Polymorphism
Virtual functions such as `authenticate()` and `display()` are overridden in derived classes.

### Composition
`SmartTaskSystem` contains:
- FileManager
- ChatManager
- Task Collection

## Files

- smarttaskchatsystem.cpp
- SmartTaskData.txt
- SmartTaskUsers.txt
- SmartTaskChat.txt
- report.html

## Compilation

```bash
g++ smarttaskchatsystem.cpp -o smarttask

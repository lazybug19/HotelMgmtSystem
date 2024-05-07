# C-based Hotel Management System

## About

A POSIX-compliant C-based hotel management system supporting multiple concurrent requests for placing orders, calculating income, assigning waiters, and facilitating efficient management among admin, hotel manager, customers, and staff.

## Getting started

### Compiling into executable files
```sh
gcc -o table.out table.c
gcc -o waiter.out waiter.c
gcc -o admin.out admin.c
gcc -o hotelmanager.out hotelmanager.c
```

### Running executable files
```sh
table.out
waiter.out
admin.out
hotelmanager.out
```
## Key Features

### Table
<ul>
  <li> POSIX compliant IPC via pipes </li>
  <li> Concurrent execution and assignment of tables to new customers </li>
</ul>

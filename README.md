# C-based Hotel Management System

## About

A POSIX-compliant C-based hotel management system supporting multiple concurrent requests for placing orders, calculating income, assigning waiters, and facilitating efficient management among admin, hotel manager, customers, and staff.

## Getting started

### Navigating to root directory
```sh
cd /path/to/the/root
```

### Compiling (GCC compiler) into executable files
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
<ul>
  <li> Can support 10+ concurrent table processes with 5+ customers in the system </li>
  <li> POSIX compliant IPC via pipes - named and ordinary </li>
  <li> Concurrent execution and assignment of tables to new customers </li>
  <li> Distinct table-waiter pairs and admin-hotelmanager communication via shared memory </li>
  <li> Hotel manager is responsible for overseeing the total earnings, calculating the total earnings of all the waiters and handling termination </li>
</ul>

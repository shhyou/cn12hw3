#!/bin/bash
g++ testsnd.cpp -o testsnd icmp.o log.o -Wall -Wshadow -Wextra -std=c++11
g++ testrcv.cpp -o testrcv icmp.o log.o -Wall -Wshadow -Wextra -std=c++11


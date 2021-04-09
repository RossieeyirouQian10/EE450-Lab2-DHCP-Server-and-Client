# EE450-Lab2-DHCP-Server-and-Client

Create a DHCP server and a client that will communicate over TCP socket

Compilation Steps on nunki:
1. g++ -o server Server.cpp -lnsl –lresolv
2. g++ -o client Client.cpp -lnsl –lresolv

Executing Steps on terminal:
1. ./server
2. ./client viterbi-scf2.usc.edu

The most part of code in Server.cpp and Client.cpp are based on the code in Beej's guide, which are in the 6th chapter, especially from "6.1. A Simple Stream Server" and "6.2. A simple Stream Client".

In Server.cpp: from line 34 to line 47 and from line 73 to line 199, it based on "6.1. A Simple Stream Server". I also do some modification on the original code.

In Client.cpp: from line 31 to line 38 and from line 52 to line 150, it based on "6.2. A simple Stream Client". I also do some modification on the original code.

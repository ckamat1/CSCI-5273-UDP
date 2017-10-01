There are two folders containing server and client code and their respective makefiles.

Compile client.cpp and server.cpp by running make.



Client.cpp
---------------------------------------

1. Client program runs by connecting to the server IP and the port on which the server is running
2. It runs continuously expecting a request from the user to be sent to the server
3. The request from the user is appropriately parsed to handle all possible commands such as get, put, delete, ls and exit.
4. Any other wrong commands are validated at the server end and appropriate response is sent back.
5. For get and put functions, client writes and reads files in binary format respectively.
6. Client implements reliable transfer of files for get and put by using the stop-and-wait protocol.
7. For get function, when it receives a packet, it sends back an acknowledgement. If it receives any frame with sequence number less than the frame for which it has already sent an ack, it discards it and waits for sometime, and resends the ack informing the server that it has already received a higher seq. num frame.

Server.cpp
-----------------------------------------------------------------------------

1. Server runs on a specific fort and communicates with the client through a UDP socket.
2. It accepts and parses commands from the clients and caters to all the requests through a handleRequest() function.
3. Any wrong command is not understood and sends a message back to the client saying it didn't understand such a command.
4. FOr get and put functions, server reads and writes files in binary format respectively.
5. Server implements reliable transfer of files for get and put by using the stop-and-wait protocol same as the client.
6. ls commands lists all the files from "/home/user" directory.

Structure of the frame
--------------------------------

Frame structure consists of -
a) Sequence number
b) ack number
c) Frame ID
d) Packet structure

Packet itself contains a character buffer of 1024 bytes.





# Networks File Transfer
The goal of this project is to use UNIX sockets to create a server and client program connection that can transfer any type of file (binary file) across a local network.

## Usage
1. Compile the respective programs. For e.g., using gcc:   
```gcc deliver.c -lpthread -o deliver```  
2. Get the current local IP . For e.g., using curl:  
```curl ifconfig.me```
3. Run the compiled ```deliver``` and ```server``` programs on the same network  
Use the following syntax to run the programs:
```deliver  <local IP/server address> <port number>``` and ```server <port number>```
4. Use the following command in the running instance of ```deliver``` : ```ftp <filename>```
5. The server will receive the file from the sent packets and create a new exact copy of the sent file by combining the sent packets. The new file will be named ```new<filename>```

## Features
* Uses a packet format and acknowledgement implementation
* Packets are sent as ```total_fragments:current_fragment:total_size:filename:filedata```
* The file is fragmented if above the max fragment size and sent and split and reassembled at the server accordingly
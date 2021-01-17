#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>

#define PACKETSIZE 1000
#define BACKLOG 10
int main(int argc, char const *argv[]) {
   char* port = argv[1];

   struct addrinfo hints;
   struct addrinfo *res;
   // first, load up address structs with getaddrinfo():

   memset(&hints, 0, sizeof hints);
   hints.ai_family = AF_UNSPEC; // use IPv4 or IPv6, whichever
   hints.ai_socktype = SOCK_DGRAM;
   hints.ai_flags = AI_PASSIVE; // fill in my IP for me
   
   getaddrinfo(NULL, port, &hints, &res);

   // make a socket:
   int sockResp = socket(res->ai_family, res->ai_socktype, res->ai_protocol);

   if (sockResp == -1) {
      printf("Socket not created\n");
      return 0;
   }

   // bind it to the port we passed in to getaddrinfo():
   int bindResp = bind(sockResp, res->ai_addr, res->ai_addrlen);
   
   if (bindResp == -1) {
      printf("Bind failed\n");
      return 0;
   }

   printf("Server started at: 128.100.13.233\n");
   //receive data from client
   char buffer[PACKETSIZE+100] = {0};
   struct sockaddr_in clientAddr;
   int clientAddrSize = sizeof(clientAddr);

   char* commandResp;

   //get first package and see how many total
   int n = recvfrom(sockResp, buffer, sizeof(buffer), 0, (struct sockaddr*)&clientAddr,&clientAddrSize);
   buffer[n] = '\0';
   int ack = 1;
   printf("Recieved: %s\n",buffer);
   
   char fileName[PACKETSIZE] = {0};
   char currentData[PACKETSIZE+20] = {0};
   int colonCount = 0;
   char currentDataSize[5] = {0};

   int fileNameCount = 0;
   int fileDataCount = 0;
   int fileDataSizeCount = 0;
   int charCount = 0;
   for (int i = 0; i < sizeof(buffer); i++) {
      if(buffer[i] == ':' && colonCount< 4) {
         colonCount++;
         charCount++;
         continue;
      }
      if(colonCount == 2) {
         currentDataSize[fileDataSizeCount] = buffer[i];
         fileDataSizeCount++;
      }
      if(colonCount == 3) {
         fileName[fileNameCount] = buffer[i];
         fileNameCount++;
      }
      else if(colonCount == 4) {
         currentData[fileDataCount] = buffer[i];
         fileDataCount++;
      }
      if(colonCount !=4) {
         charCount++;
      }
   }

   FILE *fptr;
   char newFile[PACKETSIZE] = {0};
   sprintf(newFile, "%s%s", "new", fileName);
   fptr = fopen(newFile,"wb");

   fwrite(currentData, 1, atoi(currentDataSize),fptr);
   //send ack
   char stringAck[PACKETSIZE];
   sprintf(stringAck,"%d",ack);
   printf("Sending ACK: %s\n",stringAck);
   sendto(sockResp, stringAck, 1024, 0, (struct sockaddr*)&clientAddr, clientAddrSize); 

   char frags[10] = {};
   for (int i = 0; i< sizeof(buffer); i++) {
      if(buffer[i] == ':') {
         break;
      }
      frags[i] = buffer[i];
   }
   
   int totalFrags = atoi(frags);
   
   while(ack != totalFrags) {
      int n = recvfrom(sockResp, buffer, sizeof(buffer), 0, (struct sockaddr*)&clientAddr,&clientAddrSize);
      buffer[n] = '\0';
      ack++;
      printf("Recieved: %s\n",buffer);
      
      char fileName[PACKETSIZE] = {0};
      char currentData[PACKETSIZE+100] = {0};
      int colonCount = 0;
      char currentDataSize[5] = {0};

      int fileNameCount = 0;
      int fileDataCount = 0;
      int fileDataSizeCount = 0;
      int charCount = 0;
      for (int i = 0; i < sizeof(buffer); i++) {
         if(buffer[i] == ':' && colonCount< 4) {
            colonCount++;
            charCount++;
            continue;
         }
         if(colonCount == 2) {
            currentDataSize[fileDataSizeCount] = buffer[i];
            fileDataSizeCount++;
         }
         if(colonCount == 3) {
            fileName[fileNameCount] = buffer[i];
            fileNameCount++;
         }
         else if(colonCount == 4) {
            currentData[fileDataCount] = buffer[i];
            fileDataCount++;
         }
         if(colonCount !=4) {
            charCount++;
         }
      }
      fwrite(currentData, 1, atoi(currentDataSize),fptr);
      //send ack
      char stringAck[PACKETSIZE];
      sprintf(stringAck,"%d",ack);
      printf("Sending ACK: %s\n",stringAck);
      sendto(sockResp, stringAck, 1024, 0, (struct sockaddr*)&clientAddr, clientAddrSize); 
   }
   fclose(fptr);
   
}
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <stdbool.h>
#include <math.h>
#include <time.h>

#define BACKLOG 10
#define PACKETSIZE 1000
bool fileExists (char* fileName);

int main(int argc, char *argv[]) {
   struct packet {
      unsigned int total_frag;
      unsigned int frag_no;
      unsigned int size;
      char* filename;
      char filedata[PACKETSIZE];
   };
   
   

   char* addr = argv[1] + '\0';
   char* port = argv[2];

   struct addrinfo hints;
   struct addrinfo *res;
   // first, load up address structs with getaddrinfo():

   memset(&hints, 0, sizeof hints);
   hints.ai_family = AF_UNSPEC; // use IPv4 or IPv6, whichever
   hints.ai_socktype = SOCK_DGRAM;
   hints.ai_flags = AI_PASSIVE; // fill in my IP for me
   
   getaddrinfo(addr, port, &hints, &res);

   // make a socket:
   int sockResp = socket(res->ai_family, res->ai_socktype, res->ai_protocol);

   if (sockResp == -1) {
      printf("Socket not created\n");
      return 0;
   }

   //connect to server
   connect(sockResp, res->ai_addr, res->ai_addrlen);

   // send command
   char command[300];
   char fileName[300];
   printf("Enter command and file name: ");
   scanf("%s %s",command, fileName);
   printf("%s %s\n", command, fileName);

   clock_t start;
   struct packet packetSend;
   if(strcmp(&command, "ftp") == 0) {
      FILE *fp;
      fp = fopen(fileName,"rb");
      if(fp!=NULL) {
         char buf[PACKETSIZE] = {};
         int count = PACKETSIZE;
         int fileSize = 0;
         int fragNumber = 0;
         // while (count >= PACKETSIZE) {
         //    count = fread(&buf,sizeof(char), PACKETSIZE, fp);
         //    fileSize += count;
         // }
         
         fseek(fp, 0, SEEK_END);
         fileSize = ftell(fp);

         close(fp);

         FILE *fp;
         fp = fopen(fileName,"rb");

         if(fileSize%PACKETSIZE==0) {
            packetSend.total_frag = fileSize / PACKETSIZE;
         } else {
            packetSend.total_frag = fileSize / PACKETSIZE + 1;
         }

         
         count = PACKETSIZE;
         
         while (count >= PACKETSIZE) {
            fragNumber++;
            memset(packetSend.filedata, 0, sizeof(packetSend.filedata));
            count = fread(packetSend.filedata,sizeof(char), PACKETSIZE, fp);

            printf("\nPackage %d\n",fragNumber);
            
            packetSend.frag_no = fragNumber;
            packetSend.size = count;
            packetSend.filename = fileName;

            char stringTotalFrag[PACKETSIZE];
            char stringFragNo[PACKETSIZE];
            char stringSize[PACKETSIZE];

            sprintf(stringTotalFrag, "%d", packetSend.total_frag);
            sprintf(stringFragNo, "%d", packetSend.frag_no);
            sprintf(stringSize, "%d", packetSend.size);

            char packet[1200];
            memset(packet, 0, sizeof(packet));
            //sprintf(packet, "%s:%s:%s:%s:%s", stringTotalFrag, stringFragNo, stringSize, packetSend.filename, packetSend.filedata);
            int hlen = snprintf(packet, sizeof(packet), "%s:%s:%s:%s:", stringTotalFrag, stringFragNo, stringSize, packetSend.filename);
            memcpy(packet + hlen, packetSend.filedata, sizeof(packetSend.filedata));

           
            printf("Sending: %s\n",&packet);
            start = clock();
            send(sockResp, &packet, hlen + sizeof(packetSend.filedata), 0);

         }

      fclose(fp);

         
      } else {
         printf("File not found\n");
         close(sockResp);
         return 0;
      }
   } else {
      printf("Invalid command, expecting: ftp <fileName>\n");
      return 0;
   }

   //recieve from client
   int numRecv = 0;

   while (numRecv != packetSend.total_frag) {
      char buffer[100]; 
      recv(sockResp, buffer, sizeof(buffer), 0);
      printf("Recieved: %s\n",buffer);
      numRecv++;
   }
   clock_t end = clock();
   float seconds = (float)(end - start) / CLOCKS_PER_SEC;
   printf("The RTT is: %f ms\n", seconds*1000);

   //close the socket
   close(sockResp);
   
   return 0;
}  

bool fileExists (char* fileName) {
   struct stat buffer;
   return (stat(fileName, &buffer) == 0);
}
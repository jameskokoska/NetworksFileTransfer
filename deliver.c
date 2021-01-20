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
#include <poll.h>
#include <sys/time.h>

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
   printf("\033[1;36mEnter command and file name (ftp file.jpg):\033[0m ");
   scanf("%s %s",command, fileName);
   printf("%s %s\n", command, fileName);

   struct packet packetSend;
   if(strcmp(&command, "ftp") == 0) {
      FILE *fp;
      fp = fopen(fileName,"rb");
      if(fp!=NULL) {
         char buf[PACKETSIZE] = {0};
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

            packetSend.frag_no = fragNumber;
            packetSend.size = count;
            packetSend.filename = fileName;

            char stringTotalFrag[PACKETSIZE];
            char stringFragNo[PACKETSIZE];
            char stringSize[PACKETSIZE];

            sprintf(stringTotalFrag, "%d", packetSend.total_frag);
            sprintf(stringFragNo, "%d", packetSend.frag_no);
            sprintf(stringSize, "%d", packetSend.size);

            char packet[1000+30];
            memset(packet, 0, sizeof(packet));
            //sprintf(packet, "%s:%s:%s:%s:%s", stringTotalFrag, stringFragNo, stringSize, packetSend.filename, packetSend.filedata);
            int hlen = snprintf(packet, sizeof(packet), "%s:%s:%s:%s:", stringTotalFrag, stringFragNo, stringSize, packetSend.filename);
            printf("Sending: %s[data]\n",&packet);
            memcpy(packet + hlen, packetSend.filedata, sizeof(packetSend.filedata));
            
           

            struct pollfd fd;
            int ret = 0;
            char buffer[100];
            struct timeval  tvbefore, tvafter, tvresult;
            
            while (ret== 0 || ret== -1) {
               
               gettimeofday(&tvbefore, NULL);
               send(sockResp, &packet, hlen + sizeof(packetSend.filedata), 0);
               fd.fd = sockResp; // socket handler 
               fd.events = POLLIN;
               ret = poll(&fd, 1, 1); // 1 ms for timeout TRY SELECT()
               switch (ret) {
                  case -1:
                     // Error
                     break;
                  case 0:
                     // Timeout 
                     printf("\033[1;31m");
                     printf("TIMEOUT, RESENDING: %s\n", stringFragNo);
                     printf("\033[0m");
                     recv(sockResp,buffer,sizeof(buffer), 0); // get your data
                     
                     break;
                  default:
                     recv(sockResp,buffer,sizeof(buffer), 0); // get your data
                     gettimeofday(&tvafter, NULL);
                     timersub(&tvafter, &tvbefore, &tvresult);
                     printf("RTT is: %ld.%.6ld seconds\n", (long int) tvresult.tv_sec, (long int) tvresult.tv_usec);
                     printf("Recieved: %s\n",buffer);
                     break;
               }
               if(atoi(buffer)==atoi(stringTotalFrag)) {
                  printf("\033[1;35m");
                  printf("FILE TRANSFER COMPLETE\n");
                  printf("\033[0m");
                  break;
               }
            }
         }
      fclose(fp);
      } 
      
      else {
         printf("File not found\n");
         close(sockResp);
         return 0;
      }
   } 
   
   else {
      printf("Invalid command, expecting: ftp <fileName>\n");
      return 0;
   }
   //close the socket
   close(sockResp);
   return 0;
}  

bool fileExists (char* fileName) {
   struct stat buffer;
   return (stat(fileName, &buffer) == 0);
}

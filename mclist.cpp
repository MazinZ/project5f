#include <iostream>
extern "C" {

  #include "csapp.h"
  }
  
using namespace std;

int mycloud_listfiles(char *MachineName, int TCPport, int SecretKey, char **list, int *listbuflen) {
  // LIST corresponds to a request type of 3
  unsigned int requestType = 3;
  unsigned int networkOrder;
  
  // secret key + request type
  char *data = new char [(4+4)];
  char *bufPosition = data;

  // add the secret key to the buffer
  networkOrder = htonl(SecretKey);
  memcpy(bufPosition, &networkOrder, 4);
  bufPosition += 4;

  networkOrder = htonl(requestType);
  memcpy(bufPosition, &networkOrder, 4);
  bufPosition += 4;

  int clientfd = Open_clientfd(MachineName, TCPport);
  Rio_writen(clientfd, data, (4+4));
  // Get the status, list size and the
  char statusbuffer[4];
  char listSize[4];
  char filelist[100000];
  unsigned int status;
  rio_t rio;
   
  Rio_readinitb(&rio, clientfd);
  // Get the operation status
  Rio_readnb(&rio, statusbuffer, 4);
  memcpy(&networkOrder, &statusbuffer, 4);
  status = ntohl(networkOrder);
  // Get the number of bytes in the file listing
  Rio_readnb(&rio, listSize, 4);
  memcpy(&networkOrder, &listSize, 4);
  *listbuflen = ntohl(networkOrder);
  
  // Get the list data
  Rio_readnb(&rio, filelist, *listbuflen);
  *list = (char*) malloc (*listbuflen);
  memcpy(*list, &filelist, *listbuflen);
  
  /*for (int i = 0; i < 240; i++){
  printf("%c | ", filelist[i]);
  }*/

 

  Close(clientfd);
  return status;
}



int main(int argc, char *argv[]){

if (argc != 4) { 
    cout << "Usage: ./mclist <machine name> <port> <secret key>" << endl; 
    return -1; 
    }  
  char *MachineName = argv[1];
  unsigned int TCPport = atoi(argv[2]);
  unsigned int secretKey= atoi(argv[3]);  
  int listbuflen;
  char *listbuf;
  
  int status = mycloud_listfiles(MachineName, TCPport, secretKey, &listbuf, &listbuflen);
  if(status == 0) {
    char fileName[80];
    int i = 0;
    for(i = 0; i < listbuflen; i += 80) {
      memcpy(&fileName, listbuf + i, 80);
      cout << fileName << endl;
   
  }
  }

  return status;
}

#include <iostream>
extern "C" {

  #include "csapp.h"
  }
  
#include "mycloud.h"

using namespace std;

int mycloud_listfiles(char *MachineName, int TCPport, int SecretKey, char **list, int *listbuflen) {
  unsigned int requestType = 3;
  unsigned int networkOrder;

  char *message;
  
  // secret key + request type

  message = new char [(4+4)];
  char *bufPosition = message;

  // add the secret key to the buffer
  networkOrder = htonl(SecretKey);
  memcpy(bufPosition, &networkOrder, 4);
  bufPosition += 4;

  networkOrder = htonl(requestType);
  memcpy(bufPosition, &networkOrder, 4);
  bufPosition += 4;

  int clientfd = Open_clientfd(MachineName, TCPport);
  Rio_writen(clientfd, message, (4+4));
  free(message);

  char statusBuf[4];
  char listSizeBuf[4];
  char listBuf[100000];
  unsigned int status;
  rio_t rio;
  Rio_readinitb(&rio, clientfd);

    Rio_readnb(&rio, statusBuf, 4);
    memcpy(&networkOrder, &statusBuf, 4);
    status = ntohl(networkOrder);

    Rio_readnb(&rio, listSizeBuf, 4);
    memcpy(&networkOrder, &listSizeBuf, 4);
    *listbuflen = ntohl(networkOrder);


    Rio_readnb(&rio, listBuf, *listbuflen);
    *list = (char*) malloc (*listbuflen);
    memcpy(*list, &listBuf, *listbuflen);
    status = 0;

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

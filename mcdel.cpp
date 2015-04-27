#include <iostream>

using namespace std;


extern "C" {

  #include "csapp.h"
  }
#include "mycloud.h"

int mycloud_delfile(char *MachineName, int TCPport, int SecretKey, char *Filename) { 
  unsigned int requestType = 2;
  char *message;
  unsigned int messageSize, networkOrder;

  messageSize = 4 + REQUEST_TYPE_SIZE + FILE_NAME_SIZE;

  message = (char*) malloc (sizeof(char)*messageSize);
  char *bufPosition = message;

  networkOrder = htonl(SecretKey);
  memcpy(bufPosition, &networkOrder, 4);
  bufPosition += 4;

  networkOrder = htonl(requestType);
  memcpy(bufPosition, &networkOrder, REQUEST_TYPE_SIZE);
  bufPosition += REQUEST_TYPE_SIZE;

  memcpy(bufPosition, Filename, FILE_NAME_SIZE);
  bufPosition += 80;

  int clientfd = Open_clientfd(MachineName, TCPport);
  Rio_writen(clientfd, message, messageSize);

  char buf[4];
  unsigned int status;
  rio_t rio;
  Rio_readinitb(&rio, clientfd);
  
  status = -1;
  if(Rio_readnb(&rio, buf, 4) == 4) {
    memcpy(&networkOrder, &buf, 4);
    status = ntohl(networkOrder);
  }
  Close(clientfd);
  return status;

}


int main(int argc, char *argv[]) {
  if (argc != 5) { 
    cout << "Usage: ./mcdel <machine name> <port> <secret key> <filename>" << endl; 
    return -1; 
    }
    
  char* MachineName = argv[1];
  unsigned int TCPport = atoi(argv[2]);
  unsigned int secretKey = atoi(argv[3]);   
  char* Filename = argv[4];
  
  int status = mycloud_delfile(MachineName, TCPport, secretKey, Filename);
  return status;
}

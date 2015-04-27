#include <iostream>

using namespace std;


extern "C" {

  #include "csapp.h"
  }
  
int mycloud_delfile(char *MachineName, int TCPport, int SecretKey, char *Filename) { 
  // DEL corresponds to a request type of 2
  unsigned int requestType = 2;
  unsigned int networkOrder;

  char *data = new char[(4+4+80)];
  char *bufPosition = data;

  // Add the secret key to the buffer
  networkOrder = htonl(SecretKey);
  memcpy(bufPosition, &networkOrder, 4);
  bufPosition += 4;
  // Add the request type to the buffer
  networkOrder = htonl(requestType);
  memcpy(bufPosition, &networkOrder, 4);
  bufPosition += 4;
  // Add the filename to the buffer
  memcpy(bufPosition, Filename, 80);
  bufPosition += 80;
  
  int clientfd = Open_clientfd(MachineName, TCPport);
  Rio_writen(clientfd, data, (4+4+80));
  
  // Get the operation status
  char statusbuffer[4];
  rio_t rio;
  Rio_readinitb(&rio, clientfd);
  
  int status = -1;
  if(Rio_readnb(&rio, statusbuffer, 4) == 4) {
    memcpy(&networkOrder, &statusbuffer, 4);
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

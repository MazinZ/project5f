#include <iostream>
extern "C" {

  #include "csapp.h"
  }
 #include "mycloud.h"


using namespace std;

int mycloud_getfile(char *MachineName, int TCPport, int SecretKey, char *Filename, char **data, int *datalen) {
  // GET corresponds to a 0
  unsigned int requestType = 0;
  unsigned int networkOrder;
  
  // Secret key + request type + filename
  char * dataRequest = (char*) malloc (4+80+4);
  char *bufPosition = dataRequest;
  
  // Adds the secret key to the buffer
  networkOrder = htonl(SecretKey);
  memcpy(bufPosition, &networkOrder, 4);
  bufPosition += 4;
  
  // Adds the request type to the buffer
  networkOrder = htonl(requestType);
  memcpy(bufPosition, &networkOrder, 4);
  bufPosition += 4;
  
  // Adds the filename to the buffer
  memcpy(bufPosition, Filename, 80);
  bufPosition += 80;

  int clientfd = Open_clientfd(MachineName, TCPport);
  Rio_writen(clientfd, dataRequest, (4+80+4));

  // Here is where we get the response
  unsigned int status; 
  char buffer[(4+4+100000)];
  char fileData[100000];
  char fileSizeBuf[4];
  
  rio_t rio;
  Rio_readinitb(&rio, clientfd);

  // Read all of the data into a buffer
  Rio_readnb(&rio, buffer, 4);
  memcpy(&networkOrder, &buffer, 4);
  status = ntohl(networkOrder);

  // Get the file size
  Rio_readnb(&rio, fileSizeBuf, 4);
  memcpy(&networkOrder, &fileSizeBuf, 4);
  *datalen = ntohl(networkOrder);
  
  unsigned int filesize = *datalen;
  Rio_readnb(&rio, fileData, filesize);
  *data = new char [filesize];
  memcpy(*data, &fileData, filesize);

  Close(clientfd);
  return status;
}


int main(int argc, char *argv[]) {
  
  if (argc != 5) { 
    cout << "Usage: ./mcget <machine> <port> <secret key> <filename>i" << endl;
    return -1;
    }
   
  char * MachineName = argv[1];
  unsigned int TCPport = atoi(argv[2]);
  unsigned int secretKey = atoi(argv[3]);   
  char *FileName = argv[4];
  char *data;
  int datalen;


  int status = mycloud_getfile(MachineName, TCPport, secretKey, FileName, &data, &datalen);
  write(1, data, datalen);

  return status;
}

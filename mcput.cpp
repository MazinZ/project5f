#include <iostream>
extern "C" {

  #include "csapp.h"
  }
using namespace std;

int mycloud_putfile(char *MachineName, int TCPport, int SecretKey, char *FileName, char *data, int datalen) {
  // PUT corresponds to a request type of 1
  unsigned int requestType = 1;
  unsigned int networkOrder;

  // secretKey size + request type size + max filename length + 
  char * filedata = new char  [(4 + 4 + 80 + 4 + datalen)];
  char *bufposition = filedata;
  
  // Add the secret key to the data to be sent
  networkOrder = htonl(SecretKey);
  memcpy(bufposition, &networkOrder, 4);
  bufposition += 4;
  // Add the request type (1) to the data
  networkOrder = htonl(requestType);
  memcpy(bufposition, &networkOrder, 4);
  bufposition += 4;
  // Add the filename to the data
  memcpy(bufposition, FileName, 80);
  bufposition += 80;
  
  // Add the length of the data to the data buf
  networkOrder = htonl(datalen);
  memcpy(bufposition, &networkOrder, 4);
  bufposition += 4;

  // Add the actual data to the data buf
  memcpy(bufposition, data, datalen);
  bufposition += datalen;

  int clientfd = Open_clientfd(MachineName, TCPport);
  Rio_writen(clientfd, filedata, (4 + 4 + 80 + 4 + datalen));

  // Get the status of the operation 
  char buf[4];
  unsigned int status;
  rio_t rio;
  Rio_readinitb(&rio, clientfd);
  
  status = -1;
  Rio_readnb(&rio, buf, 4);
  memcpy(&networkOrder, &buf, 4);
  status = ntohl(networkOrder);
  
  Close(clientfd);
  return status;
}


int main(int argc, char *argv[]) {

  char *MachineName      = argv[1];
  unsigned int TCPport   = atoi(argv[2]);
  unsigned int secretKey = atoi(argv[3]);   
  char *Filename         = argv[4];
  char fileData[100000];
  int datalen;

  if (argc != 5) { 
    cout << "Usage: ./mcput <machine> <port> <secret key> <filename>" << endl; 
    return -1; 
    }

  // reading from standard input
  datalen = 0;
  char ch;
  unsigned int i =0;
  while (cin.get(ch)){
    fileData[i] = ch;
    i+=1;
  }
  datalen = i;
  int status = mycloud_putfile(MachineName, TCPport, secretKey, Filename, fileData, datalen);
  return status;
}


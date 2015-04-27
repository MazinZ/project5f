extern "C" {

#include "csapp.h"
}
#include "mycloud.h"
#include <iostream>
using namespace std;

int mycloud_putfile(char *MachineName, int TCPport, int SecretKey, char *FileName, char *data, int datalen) {
  unsigned int requestType = 1;
  int clientfd;
  char *filedata;
  unsigned int networkOrder;

  // secretKey size + request type size + max filename length + 
  filedata = (char*) malloc ((4 + 4 + 80 + 4 + datalen));
  char *bufposition = filedata;

  networkOrder = htonl(SecretKey);
  memcpy(bufposition, &networkOrder, 4);
  bufposition += 4;

  networkOrder = htonl(requestType);
  memcpy(bufposition, &networkOrder, 4);
  bufposition += 4;

  memcpy(bufposition, FileName, 80);
  bufposition += 80;

  networkOrder = htonl(datalen);
  memcpy(bufposition, &networkOrder, 4);
  bufposition += 4;

  memcpy(bufposition, data, datalen);
  bufposition += datalen;

  clientfd = Open_clientfd(MachineName, TCPport);
  Rio_writen(clientfd, filedata, (4 + 4 + 80 + 4 + datalen));

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

int mycloud_listfiles(char *MachineName, int TCPport, int SecretKey, char **list, int *listbuflen) {
  unsigned int requestType = 3;
  int clientfd;
  char *message;
  unsigned int messageSize, networkOrder;

  messageSize = 4 + REQUEST_TYPE_SIZE;

  message = (char*) malloc (sizeof(char)*messageSize);
  char *bufPosition = message;

  networkOrder = htonl(SecretKey);
  memcpy(bufPosition, &networkOrder, 4);
  bufPosition += 4;

  networkOrder = htonl(requestType);
  memcpy(bufPosition, &networkOrder, REQUEST_TYPE_SIZE);
  bufPosition += REQUEST_TYPE_SIZE;

  clientfd = Open_clientfd(MachineName, TCPport);
  Rio_writen(clientfd, message, messageSize);
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


int mycloud_delfile(char *MachineName, int TCPport, int SecretKey, char *Filename) { 
  unsigned int requestType = 2;
  int clientfd;
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

  clientfd = Open_clientfd(MachineName, TCPport);
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



int mycloud_getfile(char *MachineName, int TCPport, int SecretKey, char *Filename, char **data, int *datalen) {
  unsigned int requestType = 0;
  int clientfd;
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
  bufPosition += FILE_NAME_SIZE;

  clientfd = Open_clientfd(MachineName, TCPport);
  Rio_writen(clientfd, message, messageSize);
  free(message);

  int GET_STATUS_SIZE = 4 + 4 + 100000;
  char buf[GET_STATUS_SIZE];
  char fileSizeBuf[4];
  char fileData[100000];
  unsigned int status; 
  
  rio_t rio;
  Rio_readinitb(&rio, clientfd);


  if(Rio_readnb(&rio, buf, 4) == 4) {
    memcpy(&networkOrder, &buf, 4);
    status = ntohl(networkOrder);
  } 
  else
    status = -1;
  

  if(Rio_readnb(&rio, fileSizeBuf, 4) == 4) {
    memcpy(&networkOrder, &fileSizeBuf, 4);
    *datalen = htonl(networkOrder);
  } 
  else 
    status = -1;
    
  unsigned int filesize = *datalen;
  
  if(Rio_readnb(&rio, fileData, filesize) == filesize) {
    *data = (char*) malloc (filesize);
    memcpy(*data, &fileData, filesize);
  
  } 
  else 
     status = -1;
  

  Close(clientfd);
  return status;
}




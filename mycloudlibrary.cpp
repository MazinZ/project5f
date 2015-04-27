extern "C" {

#include "csapp.h"
}
#include "mycloud.h"
#include <iostream>
using namespace std;

int mycloud_putfile(char *MachineName, int TCPport, int SecretKey, char *FileName, char *data, int datalen) {
  int clientfd;
  char *filedata;
  unsigned int networkOrder;


  filedata = (char*) malloc ((4 + 4 + 80 + 100000 + datalen));
  char *bufposition = filedata;

  networkOrder = htonl(SecretKey);
  memcpy(bufposition, &networkOrder, 4);
  bufposition += 4;

  unsigned int request = 1;
  networkOrder = htonl(request);
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
  Rio_writen(clientfd, filedata, (4 + 4 + 80 + 100000 + datalen));

  size_t n;
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

int mycloud_listfiles(char *MachineName, int TCPport, int SecretKey, char **list, int *list_len) {

  int clientfd;
  char *message;
  unsigned int messageSize, networkOrder;

  messageSize = 4 + REQUEST_TYPE_SIZE;

  message = (char*) malloc (sizeof(char)*messageSize);
  char *messagePtr = message;

  networkOrder = htonl(SecretKey);
  memcpy(messagePtr, &networkOrder, 4);
  messagePtr += 4;

  unsigned int request = 3;
  networkOrder = htonl(request);
  memcpy(messagePtr, &networkOrder, REQUEST_TYPE_SIZE);
  messagePtr += REQUEST_TYPE_SIZE;

  clientfd = Open_clientfd(MachineName, TCPport);
  Rio_writen(clientfd, message, messageSize);
  free(message);

  size_t n;
  char statusBuf[4];
  char listSizeBuf[4];
  char listBuf[MAX_FILE_SIZE];
  unsigned int status;
  rio_t rio;
  Rio_readinitb(&rio, clientfd);

  if((n = Rio_readnb(&rio, statusBuf, 4)) == 4) {
    memcpy(&networkOrder, &statusBuf, 4);
    status = ntohl(networkOrder);
  }
  else 
    status = -1;

  if((n = Rio_readnb(&rio, listSizeBuf, 4)) == 4) {
    memcpy(&networkOrder, &listSizeBuf, 4);
    *list_len = ntohl(networkOrder);
  } 
  else 
    status = -1;

  if((n = Rio_readnb(&rio, listBuf, *list_len)) == *list_len) {
    *list = (char*) malloc (*list_len);
    if(*list == NULL) { fprintf(stderr, "Memory Error\n"); return -1; }
    memcpy(*list, &listBuf, *list_len);
    status = 0;
  } 
  else
    status = -1;

  Close(clientfd);
  return status;
}


int mycloud_delfile(char *MachineName, int TCPport, int SecretKey, char *Filename) { 

  int clientfd;
  char *message;
  unsigned int messageSize, networkOrder;

  messageSize = 4 + REQUEST_TYPE_SIZE + FILE_NAME_SIZE;

  message = (char*) malloc (sizeof(char)*messageSize);
  char *messagePtr = message;

  networkOrder = htonl(SecretKey);
  memcpy(messagePtr, &networkOrder, 4);
  messagePtr += 4;

  unsigned int request = 2;
  networkOrder = htonl(request);
  memcpy(messagePtr, &networkOrder, REQUEST_TYPE_SIZE);
  messagePtr += REQUEST_TYPE_SIZE;

  memcpy(messagePtr, Filename, FILE_NAME_SIZE);
  messagePtr += FILE_NAME_SIZE;

  clientfd = Open_clientfd(MachineName, TCPport);
  Rio_writen(clientfd, message, messageSize);


  size_t n;
  char buf[4];
  unsigned int status;
  rio_t rio;
  Rio_readinitb(&rio, clientfd);
  
  status = -1;
  if((n = Rio_readnb(&rio, buf, 4)) == 4) {
    memcpy(&networkOrder, &buf, 4);
    status = ntohl(networkOrder);
  }
  Close(clientfd);
  return status;

}



int mycloud_getfile(char *MachineName, int TCPport, int SecretKey, char *Filename, char **data, int *datalen) {

  int clientfd;
  char *message;
  unsigned int messageSize, networkOrder;

  messageSize = 4 + REQUEST_TYPE_SIZE + FILE_NAME_SIZE;

  message = (char*) malloc (sizeof(char)*messageSize);
  char *messagePtr = message;

  networkOrder = htonl(SecretKey);
  memcpy(messagePtr, &networkOrder, 4);
  messagePtr += 4;

  unsigned int request = 0;
  networkOrder = htonl(request);
  memcpy(messagePtr, &networkOrder, REQUEST_TYPE_SIZE);
  messagePtr += REQUEST_TYPE_SIZE;

  memcpy(messagePtr, Filename, FILE_NAME_SIZE);
  messagePtr += FILE_NAME_SIZE;

  clientfd = Open_clientfd(MachineName, TCPport);
  Rio_writen(clientfd, message, messageSize);
  free(message);

  size_t n;
  int GET_STATUS_SIZE = 4 + 4 + MAX_FILE_SIZE;
  char buf[GET_STATUS_SIZE];
  char fileSizeBuf[4];
  char dataBuf[MAX_FILE_SIZE];
  unsigned int status, bytesInFile;
  
  rio_t rio;
  Rio_readinitb(&rio, clientfd);


  if((n = Rio_readnb(&rio, buf, 4)) == 4) {
    memcpy(&networkOrder, &buf, 4);
    status = ntohl(networkOrder);
  } 
  else
    status = -1;
  


  if((n = Rio_readnb(&rio, fileSizeBuf, 4)) == 4) {
    memcpy(&networkOrder, &fileSizeBuf, 4);
    *datalen = htonl(networkOrder);
  } 
  else 
    status = -1;
  
  bytesInFile = *datalen;
  
  if((n = Rio_readnb(&rio, dataBuf, bytesInFile)) == bytesInFile) {

    *data = (char*) malloc (sizeof(char)*bytesInFile);
    memcpy(*data, &dataBuf, bytesInFile);
  
  } 
  else 
     status = -1;
  

  Close(clientfd);

  return status;
}




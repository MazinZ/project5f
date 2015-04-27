#include "mycloudlibrary.cpp"
#include "stdlib.h"
#include <iostream>

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
    int i;
    char fileName[80];
    for(i = 0; i < listbuflen; i += 80) {
      memcpy(&fileName, listbuf + i, 80);
      cout << fileName << endl;
    }
  }

  return status;
}

#include "mycloudlibrary.cpp"
#include "stdlib.h"
#include <iostream>

int main(int argc, char *argv[]){

  int list_len;
  char *list_buf;

if (argc != 4) { 
    cout << "Usage: ./mcput <machine name> <port> <secret key>" << endl; 
    return -1; 
    }  
  char *MachineName = argv[1];
  unsigned int TCPport = atoi(argv[2]);
  unsigned int secretKey= atoi(argv[3]);  

  int status = mycloud_listfiles(MachineName, TCPport, secretKey, &list_buf, &list_len);
  if(status == 0) {
    int i;
    char fileName[80];
    for(i = 0; i < list_len; i += 80) {
      memcpy(&fileName, list_buf + i, 80);
      cout << fileName << endl;
    }
  }

  return status;
}

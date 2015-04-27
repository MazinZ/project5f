#include <iostream>
#include <stdlib.h>
#include "mycloudlibrary.cpp"

using namespace std;

int main(int argc, char *argv[]) {
  
  if (argc != 5) { 
    cout << "Usage: ./mcget <machine> <port> <secret key> <filename>i" << endl; r
    return -1;
    }
   
  char * MachineName = argv[1];
  unsigned int TCPport = atoi(argv[2]);
  unsigned int secretKey = atoi(argv[3]);   
  char *FileName = argv[4];
  char *data;
  int datalen;


  int status = mycloud_getfile(MachineName, TCPport, secretKey, Filename, &data, &datalen);
  write(1, data, datalen);
  return status;
}

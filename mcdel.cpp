#include <iostream>
#include <stdlib.h>
#include "mycloudlibrary.cpp"

using namespace std;

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

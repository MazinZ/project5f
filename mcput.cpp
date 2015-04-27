#include <stdlib.h>
#include "mycloudlibrary.cpp"
#include <iostream>
using namespace std;

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

  datalen = 0;
  int offset = 0;
  int currentBufSize = 1;
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


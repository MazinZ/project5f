
#include <iostream>
#include <stdlib.h>
#include "mycloud.h"
#include <vector>
#include <algorithm>
#include <iterator>

using namespace std;
extern "C" {

#include "csapp.h"
}
#include <string.h>


bool validKey(rio_t *rio, unsigned int secretKey);
int getRequest(rio_t *rio);
int storeRequest(rio_t *rio, int connfd);
int retrieveRequest(rio_t *rio, int connfd);
int deleteRequest(rio_t *rio, int connfd);
int listFilesRequest(rio_t *rio, int connfd);
int addFileToList(char *fileName);
int removeFileFromList(char *fileName);
bool fileExists(char * filename);

vector<char *> fileList;


int main(int argc, char **argv) 
{
    int listenfd, connfd, port;
    socklen_t clientlen;
    struct sockaddr_in clientaddr;
    struct hostent *hp;
    char *haddrp;
    unsigned int secretKey;

    if (argc != 3) {
	fprintf(stderr, "usage: %s <port> <secret key>\n", argv[0]);
	exit(0);
    }
    port = atoi(argv[1]);
    secretKey = atoi(argv[2]);

    listenfd = Open_listenfd(port);
    while (1) {
	clientlen = sizeof(clientaddr);
	connfd = Accept(listenfd, (SA *)&clientaddr, &clientlen);

	hp = Gethostbyaddr((const char *)&clientaddr.sin_addr.s_addr, 
			   sizeof(clientaddr.sin_addr.s_addr), AF_INET);
	haddrp = inet_ntoa(clientaddr.sin_addr);
	printf("server connected to %s (%s)\n", hp->h_name, haddrp);

        int requestType = -1;
        int status = -1;
        rio_t rio;
        Rio_readinitb(&rio, connfd);
        
        requestType = getRequest(&rio);
        if(validKey(&rio, secretKey) == 0) {
            if(requestType == 0) {
                cout << "Request Type     = get" << endl;
                status = retrieveRequest(&rio, connfd);
            } else if(requestType == 1) {
                cout << "Request Type     = put" << endl;
                status = storeRequest(&rio, connfd);
            } else if(requestType == 2) {
                cout << "Request Type     = del" << endl;
                status = deleteRequest(&rio, connfd);
            } else if(requestType == 3) {
                cout << "Request Type     = list" << endl;
                status = listFilesRequest(&rio, connfd);
            } else {
                cout << "Request Type     = invalid" << endl;
            }
        }

        if(status == 0) { 
            cout << "Operation Status = success" << endl;
        }
        else { 
            cout << "Operation Status = error" << endl; 
            }
        cout << "------------------------------------" << endl;
	Close(connfd);
    }
    exit(0);
}

bool validKey(rio_t *rio, unsigned int secretKey) {
    char buf[4];
    unsigned int givenKey;
    unsigned int networkOrder;

    if(Rio_readnb(rio, buf, 4) == 4) {
        memcpy(&networkOrder, &buf, 4);
        givenKey = ntohl(networkOrder);        

        cout << "Secret Key       = " << givenKey << endl;
        if(givenKey == secretKey) { 
            return true; 
            } 
        else { 
            return false; 
        }
    }    
    return false;
}

int getRequest(rio_t *rio) {
    size_t n;
    char buf[REQUEST_TYPE_SIZE];
    unsigned int requestType, networkOrder;
    if((n = Rio_readnb(rio, buf, REQUEST_TYPE_SIZE)) == REQUEST_TYPE_SIZE) {
        memcpy(&networkOrder, &buf, REQUEST_TYPE_SIZE);
        requestType = ntohl(networkOrder);

        return requestType;
    }
    return -1;
}

int storeRequest(rio_t *rio, int connfd) {
    size_t n;
    char fileNameBuf[80];
    char fileName[80];
    char fileSizeBuf[4];
    unsigned int fileSize, networkOrder, status, messageSize;
    char dataBuf[MAX_FILE_SIZE];
    char *data, *message;
    FILE *file;

    if((n = Rio_readnb(rio, fileNameBuf, 80)) == 80) {
        memcpy(&fileName, &fileNameBuf, 80);
        printf("Filename         = %s\n", fileName);
    } else {
        printf("Filename         = NONE\n");
        status = -1;
    }

    if((n = Rio_readnb(rio, fileSizeBuf, 4)) == 4) {
        memcpy(&networkOrder, &fileSizeBuf, 4);
        fileSize = ntohl(networkOrder);
    } else {
        status = -1;
    }

    if((n = Rio_readnb(rio, dataBuf, fileSize)) == fileSize) {
        data = (char*) malloc (sizeof(char)*fileSize);
        if(data == NULL) { fprintf(stderr, "Memory Error\n"); return -1; }
        memcpy(data, &dataBuf, fileSize);
    } 
    
    else {
        status = -1;
    }
  
    file = Fopen(fileName, "w");
        Fwrite(data, sizeof(char), fileSize, file);
        Fclose(file);
        addFileToList(fileName);
        status = 0;
    
    //////////////////////////
    message = (char*) malloc (4);
    char *bufPosition = message;

    networkOrder = htonl(status);
    memcpy(bufPosition, &networkOrder, STATUS_SIZE);
    bufPosition += STATUS_SIZE;

    Rio_writen(connfd, message, 4);
    free(message);

    return status;
}

int retrieveRequest(rio_t *rio, int connfd) {
    size_t n;
    char fileNameBuf[80];
    char fileName[80];
    unsigned int fileSize, networkOrder, status, messageSize;
    char *data, *message;
    FILE *file;


    if((n = Rio_readnb(rio, fileNameBuf, 80)) == 80) {
        memcpy(&fileName, &fileNameBuf, 80);
        printf("Filename         = %s\n", fileName);
    } else {
        printf("Filename         = NONE\n");
        status = -1;
    }

     if(!fileExists(fileName)) { 
        fileSize = 0;
        status = -1; }
    else {
        
        file = fopen(fileName, "r");
            fseek(file, 0, SEEK_END);
            fileSize = ftell(file);
            rewind(file);

            data = (char*) malloc (fileSize);
            if((n = fread(data, 1, fileSize, file)) == fileSize) { fclose(file); status = 0; }
            else { fileSize = 0; status = -1; }
        
    }

    messageSize = STATUS_SIZE + MAX_NUM_BYTES_IN_FILE + fileSize;

    message = (char*) malloc (sizeof(char*)*messageSize);
    if(message == NULL) { fprintf(stderr, "Memory Error\n"); return -1; }
    char *bufPosition = message;

    networkOrder = htonl(status);
    memcpy(bufPosition, &networkOrder, STATUS_SIZE);
    bufPosition += STATUS_SIZE;

    networkOrder = htonl(fileSize);
    memcpy(bufPosition, &networkOrder, 4);
    bufPosition+= 4;

    memcpy(bufPosition, data, fileSize);
    bufPosition += fileSize;
    free(data);

    Rio_writen(connfd, message, messageSize);
    free(message);

    return status;
}

int deleteRequest(rio_t *rio, int connfd) {
    size_t n;
    char fileNameBuf[80];
    char fileName[80];
    unsigned int networkOrder, status, messageSize;
    char *message;   
    
    if((n = Rio_readnb(rio, fileNameBuf, 80)) == 80) {
        memcpy(&fileName, &fileNameBuf, 80);
        printf("Filename         = %s\n", fileName);
    } else {
        printf("Filename         = NONE\n");
        status = -1;
    }

  
    if(removeFileFromList(fileName) == -1) { 
        status = -1; 
        }
    else {
        if(remove(fileName) != 0) { status = -1; }
        else { 
           status = 0; 
            }
    }

    messageSize = STATUS_SIZE;
    message = (char*) malloc (sizeof(char*)*messageSize);
    char *bufPosition = message;

    networkOrder = htonl(status);
    memcpy(bufPosition, &networkOrder, STATUS_SIZE);
    bufPosition += STATUS_SIZE;

    Rio_writen(connfd, message, messageSize);
    free(message);

    return status;
}

int listFilesRequest(rio_t *rio, int connfd) {
    unsigned int datalen, networkOrder, status, messageSize;
    char *message;
    //vector<char *> test;
   // test.push_back("hellooooooooooooooooooooooooooooooo\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0");
   // test.push_back("test2\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0");
    for (int i = 0; i < fileList.size(); i++){
        cout << "FILES: "<<fileList[i] << endl;
           }
    datalen = fileList.size() * 80;

    messageSize = STATUS_SIZE + MAX_NUM_BYTES_IN_FILE + datalen;

    message = (char*) malloc (sizeof(char*)*messageSize);
    char *bufPosition = message;
    status = 0;

    networkOrder = htonl(status);
    memcpy(bufPosition, &networkOrder, STATUS_SIZE);
    bufPosition += STATUS_SIZE;

    networkOrder = htonl(datalen);
    memcpy(bufPosition, &networkOrder, 4);
    bufPosition += 4;

   
    memcpy(bufPosition, fileList[0], datalen);
    bufPosition += datalen;

    for (int i = 0; i < (messageSize); i++){
        printf("%c | ", message[i]);
        }
    Rio_writen(connfd, message, messageSize);
    free(message);

    return status;
}


int addFileToList(char *fileName) {
    string finalstr = string(fileName);
    char* finalbuf = new char[80];
    int strlength = finalstr.length();
    if(!fileExists(fileName)) {
        while (strlength <= 80)
        {
            finalstr += "\0";
            strlength+=1;

        }
        strcpy(finalbuf, finalstr.c_str());
        fileList.push_back(finalbuf);
        return 0;
    }
    return -1;
}

int removeFileFromList(char *fileName) {
    if(fileExists(fileName)) {
     fileList.erase(std::remove(fileList.begin(), fileList.end(), fileName), fileList.end());
    }
    else
        return -1;
}


bool fileExists(char * fileName){
    for (int i = 0; i < fileList.size(); i++){
        if (fileList[i] == fileName)
        return true;
    }
    return false;
}


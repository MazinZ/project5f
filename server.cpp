#include <iostream>
#include <vector>
#include <algorithm>
#include <iterator>

using namespace std;
extern "C" {

#include "csapp.h"
}
#include <string.h>


int mcput(rio_t *rio, int connfd);
int mcget(rio_t *rio, int connfd);
int mcdel(rio_t *rio, int connfd);
int mclist(rio_t *rio, int connfd);


int deleteFile(char *fileName);
bool fileExists(char * filename);
bool isValidKey(rio_t *rio, unsigned int secretKey);
int addFile(char *fileName);

// This vector is used to store the file names
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
	cout << "usage: " << argv[0] << " <port> <secret key>" << endl;
	exit(-1);
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
        
        if(isValidKey(&rio, secretKey)) {     
            // Read what request is given (next 4 bytes after the secret key)    
            char requestbuf[4];
            unsigned int networkOrder;
            if(Rio_readnb(&rio, requestbuf, 4) == 4) {
                memcpy(&networkOrder, &requestbuf, 4);
                requestType = ntohl(networkOrder);
            }
            ////
            // Call the appropriate function 
            if(requestType == 0) {
                cout << "Request Type     = get" << endl;
                status = mcget(&rio, connfd);
            } 
            else if(requestType == 1) {
                cout << "Request Type     = put" << endl;
                status = mcput(&rio, connfd);
            } 
            else if(requestType == 2) {
                cout << "Request Type     = del" << endl;
                status = mcdel(&rio, connfd);
            } 
            else if(requestType == 3) {
                cout << "Request Type     = list" << endl;
                status = mclist(&rio, connfd);
            } else {
                cout << "Request Type     = invalid" << endl;
            }
        }

        if(status == 0) 
        { 
            cout << "Operation Status = success" << endl;
        }
        else 
        { 
            cout << "Operation Status = error" << endl; 
        }
        cout << "------------------------------------" << endl;
	
    Close(connfd);
    }
    exit(0);
}

// Checks the given key against the actual secret key
bool isValidKey(rio_t *rio, unsigned int secretKey) {
    char buf[4];
    unsigned int givenKey;
    unsigned int networkOrder;
    // Read the first 4 bytes which should be the secret key
    // according to the specification.
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


int mcput(rio_t *rio, int connfd) {
    unsigned int fileSize; 
    unsigned int networkOrder;
    char fileData[100000];
    char *data;

    // File used for for writing purposes
    FILE *myfile;
    
    
    unsigned int status = 0;
    char fileSizeBuf[4];
    char fileNameBuf[80];
    char fileName[80];
    if(Rio_readnb(rio, fileNameBuf, 80) == 80) {
        memcpy(&fileName, &fileNameBuf, 80);
        cout << "Filename         = " << fileName << endl;
    } else {
        cout << "Filename         = NONE" << endl;
        status = -1;
    }
    // Get the length of the data
    Rio_readnb(rio, fileSizeBuf, 4);
    memcpy(&networkOrder, &fileSizeBuf, 4);
    fileSize = ntohl(networkOrder);
    // Read the data based on the length of the data
    Rio_readnb(rio, fileData, fileSize);
    data = (char *) malloc(fileSize);
    memcpy(data, &fileData, fileSize);
    // Write to the given data to the file in binary mode 
    myfile = Fopen(fileName, "wb");
    Fwrite(data, 1, fileSize, myfile);
    Fclose(myfile);
    addFile(fileName);
    status = 0;
    free(data);
    
    // Send the status of the operation (success or failure)
    char *response = (char *) malloc(4);
    char *bufPosition = response;

    networkOrder = htonl(status);
    memcpy(bufPosition, &networkOrder, 4);
    bufPosition += 4;
    
    Rio_writen(connfd, response, 4);
    free(response);
    return status;
}

int mcget(rio_t *rio, int connfd) {
    char fileNameBuf[80];
    char fileName[80];
    unsigned int fileSize, networkOrder, status;
    char *data = NULL; 
    
    
    
    char *fileData;
    FILE *file;
    

    if(Rio_readnb(rio, fileNameBuf, 80) == 80) {
        memcpy(&fileName, &fileNameBuf, 80);
        cout << "Filename         = " << fileName << endl;
    } else {
        cout << "Filename         = NONE" << endl;;
        status = -1;
    }
    // The file doesn't exist, set the status to -1 for error
     if(!fileExists(fileName)) { 
        fileSize = 0;
        status = -1; 
        
        }
    else {
        // Open file in binary mode
        file = fopen(fileName, "rb");
            // Get the length of the file
            fseek(file, 0, SEEK_END);
            fileSize = ftell(file);
            // Put the file pointer back at the beginning
            rewind(file);

            data = (char*) malloc (fileSize);
        if(fread(data, 1, fileSize, file) == fileSize){
           fclose(file); 
           status = 0; 
           }
            else { 
             fileSize = 0;
             status = -1; 
            }
        
    }

    fileData =  (char*) malloc(4+4+fileSize);
    char *bufPosition = fileData;
    
    // Copy the status of the operation to the buffer
    networkOrder = htonl(status);
    memcpy(bufPosition, &networkOrder, 4);
    bufPosition += 4;
    // Copy the filesize to the buffer
    networkOrder = htonl(fileSize);
    memcpy(bufPosition, &networkOrder, 4);
    bufPosition+= 4;
    // Copy the file's data to the buffer
    memcpy(bufPosition, data, fileSize);
    bufPosition += fileSize;
    free(data);
    Rio_writen(connfd, fileData, (4+4+fileSize));
    free (fileData);
    return status;
}

int mcdel(rio_t *rio, int connfd) {
    char fileNameBuf[80];
    char fileName[80];
    unsigned int networkOrder;
    unsigned int status;
    // Read the filename
    if(Rio_readnb(rio, fileNameBuf, 80) == 80) {
        memcpy(&fileName, &fileNameBuf, 80);
        cout << "Filename         = " <<  fileName << endl;
    } 
    else {
        cout << "Filename         = NONE" << endl;
        status = -1;
    }
    
    // If the file doesn't exist we can't delete it, set operation status to failure
    if(!fileExists(fileName)) { 
        status = -1; 
        }
    else {
        remove(fileName);
        status = 0;
        }
    
    char * response = (char*) malloc(4);
    char *bufPosition = response;
    
    // Copy the status into the response buffer
    networkOrder = htonl(status);
    memcpy(bufPosition, &networkOrder, 4);
    bufPosition += 4;

    Rio_writen(connfd, response, 4);
    free(response);
    return status;
}

int mclist(rio_t *rio, int connfd) {
    unsigned int networkOrder;
    unsigned int status;
    char * data;
    //vector<char *> test;
    // DEBUGGING COMMENT
   // test.push_back("hellooooooooooooooooooooooooooooooo\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0");
   // test.push_back("test2\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0");
    // Status size + Request size + length of data
    unsigned int datalen = fileList.size() * 80;

    data = (char*) malloc(4+4+datalen);
    char *bufPosition = data;
    status = 0;
    // Copy the status into the data buffer
    networkOrder = htonl(status);
    memcpy(bufPosition, &networkOrder, 4);
    bufPosition += 4;
    // Copy the data length to the buffer
    networkOrder = htonl(datalen);
    memcpy(bufPosition, &networkOrder, 4);
    bufPosition += 4;
    // Copy the filelist to the buffer
    string temp;
    for (int i = 0; i < fileList.size(); i++){
        temp = string(fileList[i]);
        for (int j = temp.length(); j < 80; j++){
            fileList[i][j] = '\0';
        }
        }
    for (int i = 0; i < fileList.size(); i++){
        for (int j = 0; j < 80; j++){
            printf("%c | ", fileList[i][j]);
            }
         }
    cout << "DATALEN: " << datalen << endl;
    for (int i = 0; i < fileList.size(); i++){
    memcpy(bufPosition, fileList[i], 80);
    bufPosition += 80;

    }

    Rio_writen(connfd, data, (4+4+datalen));
    free (data);
    return status;
}


int addFile(char * fileName) {
    string finalstr = string(fileName);
    char* finalbuf = (char *)malloc (80);
    int strlength = finalstr.length();
    if(!fileExists(fileName)) {
        while (strlength < 80)
        {
            finalstr += "\0";
            strlength+=1;

        }
      
        strcpy(finalbuf, finalstr.c_str());
       // memcpy(finalbuf, finalstr, 80);
        fileList.push_back(finalbuf);
        return 0;
    }
    return -1;
}

int deleteFile(char *fileName) {
    if(fileExists(fileName)) {
     fileList.erase(std::remove(fileList.begin(), fileList.end(), fileName), fileList.end());
    return 0;
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


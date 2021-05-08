#pragma once
#include "headers.h"


class peerInfo
{
private:
    unsigned short int portNumber;
public:
    std::string newIp;
    //constructors
    peerInfo()
    {
        portNumber=0;//port number consists of 2 bytes
    }
    peerInfo(unsigned  char* ipBuffer, unsigned  short int portNumberBuffer)
    {
        this->portNumber=portNumberBuffer;
    }
    //getters and setters
    unsigned short int getPortNumber()
    {
        return this->portNumber;
    }

    void setPortNumber(unsigned short int portNumberBuffer) {
        this->portNumber=portNumberBuffer;
    }
};

class fileInfo
{
private:
    std::string filename;
    unsigned long int length;//length of file in bytes
public:
    fileInfo();

    void setFilename(const std::string &filename);

    void setLength(unsigned long length);

    const std::string &getFilename() const;

    unsigned long getLength() const;
};

class TorrentFile
{
public:
    std::string torrentName;
    unsigned char info_hash[20];
    std::vector<fileInfo> files;
    std::string address;

   void addFile(fileInfo);

   void extractFilesInfo();

    void setTorrentName(const std::string &torrentName);

    TorrentFile(const std::string &torrentName);

    void calculateInfoHashAndAddress();
    std::string getAddress(char*, int);
    void getHash(char*, unsigned char*, int,std::ifstream*);

    int findStartPos(char*,int);

    int findEndPos(char*, int);

    void displayFiles();
};
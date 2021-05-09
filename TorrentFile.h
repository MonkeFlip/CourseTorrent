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
    std::string torrentName;//name of torrent-file
    unsigned char info_hash[20];//info hash of torrent-file
    std::vector<fileInfo> files;//information about every file in torrent
    std::string address;//address of torrent tracker
    unsigned int pieceLength;//length of one piece
    unsigned int pieceQuantity;//total amount of pieces in all files

   void addFile(fileInfo);

   void extractFilesInfo();

    void setTorrentName(const std::string &torrentName);

    TorrentFile(const std::string &torrentName);

    void calculateInfoHashAndAddress();
    std::string getAddress(char*, int);
    void getHash(char*, unsigned char*, int,std::ifstream*);

    int findStartPos(char*,int);

    int findEndPos(char*, int);

    void displayFiles();//print information about all files of torrent

    void extractPieceLengthAndQuantity(char* fileContentBuffer);

    void setPieceLength(unsigned int pieceLength);

    void setPieceQuantity(unsigned int pieceQuantity);

    unsigned int getPieceLength() const;

    unsigned int getPieceQuantity() const;
};
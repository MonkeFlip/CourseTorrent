#pragma once
#include "headers.h"


class PeerInfo
{
private:
    unsigned short int portNumber;
    bool choked;
    bool interested;
public:
    std::string ip;
    //constructors
    PeerInfo()
    {
        portNumber=0;//port number consists of 2 bytes
    }
    PeerInfo(unsigned  char* ipBuffer, unsigned  short int portNumberBuffer)
    {
        this->portNumber=portNumberBuffer;
    }
    //getters and setters
    unsigned short int getPortNumber()
    {
        return this->portNumber;
    }

    const std::string &getIp() const;

    void setIp(const std::string &ip);

    bool isChoked() const;

    bool isInterested() const;

    void setChoked(bool choked);

    void setInterested(bool interested);

    void setPortNumber(unsigned short int portNumberBuffer) {
        this->portNumber=portNumberBuffer;
    }
};

class FileInfo
{
private:
    std::string filename;
    unsigned long int length;//length of file in bytes
public:
    FileInfo();

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
    std::vector<FileInfo> files;//information about every file in torrent
    std::string address;//address of torrent tracker
    //unsigned int pieceLength;//length of one piece
    //unsigned int pieceQuantity;//total amount of pieces in all files

   void addFile(FileInfo);

   void extractFilesInfo(std::string downloadDirectory);

    void setTorrentName(const std::string &torrentName);

    TorrentFile(const std::string &torrentName);

    void calculateInfoHashAndAddress();
    std::string getAddress(char*, int);
    void getHash(char*, unsigned char*, int,std::ifstream*);

    int findStartPos(char*,int);

    int findEndPos(char*, int);

    void displayFiles();//print information about all files of torrent


};
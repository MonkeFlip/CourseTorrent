#pragma once

#include "TorrentFile.h"
class PeerManager//controls peer connection, state and messages
{
private:
public:
    class PeerInfo* allPeers;
    PeerManager(){
        this->allPeers= nullptr;
    }
    void sendChoke(int sockfd);
    void sendUnchoke(int sockfd);
    void sendInterested(int sockfd);
    void sendNotInterested(int sockfd);
    void sendRequest(int sockfd,int index,int begin,int length);

    int readMessage(int sockfd,std::string &response);
    int makeHandshake(class TorrentFile torrentFile);
};

class Downloader//create files of the torrent and fill them with pieces received from peer,
{               //uses PeerManager to request and receive pieces
public:
    unsigned long int downloaded_bytes;
    unsigned long int total_size;
    bool work_flag;
    void download(int sockfd,class TorrentFile torrentFile,PeerManager peerManager);

    Downloader();
};

void* progressBarThread(void* param);

#pragma once

#include "TorrentFile.h"
class PeerManager//controls peer connection, state and messages
{
private:
public:
    class peerInfo* allPeers;
    PeerManager(){
        this->allPeers= nullptr;
    }
    void sendChoke(int sockfd);
    void sendUnchoke(int sockfd);
    void sendInterested(int sockfd);
    void sendNotInterested(int sockfd);
    void sendRequest(int sockfd,int index,int begin,int length);

    int readMessage(int sockfd,std::string &response);
    int makeHandshake(char info_hash[],std::vector<class fileInfo> files, class TorrentFile torrentFile);
};

class Downloader//create files of the torrent and fill them with pieces received from peer,
{               //uses PeerManager to request and receive pieces
public:
    void download(int sockfd,std::vector<class fileInfo> files,class TorrentFile torrentFile,PeerManager peerManager);
};

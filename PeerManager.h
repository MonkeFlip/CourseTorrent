#pragma once

class PeerManager//controls peer connection, state and messages
{
private:
    bool choked;
    bool interested;
public:
    PeerManager(){
        this->choked=false;
        this->interested=false;
    }
    void sendChoke(int sockfd);
    void sendUnchoke(int sockfd);
    void sendInterested(int sockfd);
    void sendNotInterested(int sockfd);
    void sendRequest(int sockfd,int index,int begin,int length);

    int readMessage(int sockfd,std::string &response);
    //getters and setters
    bool isChoked() const;

    bool isInterested() const;

    void setChoked(bool choked);

    void setInterested(bool interested);
};

class Downloader//create files of the torrent and fill them with pieces received from peer,
{               //uses PeerManager to request and receive pieces
    void download(int sockfd,std::vector<class fileInfo> files,class TorrentFile torrentFile,PeerManager peerManager);
};

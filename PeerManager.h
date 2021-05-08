#pragma once

class PeerManager
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

    bool isChoked() const;

    bool isInterested() const;

    void setChoked(bool choked);

    void setInterested(bool interested);

};
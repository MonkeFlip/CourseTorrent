#include "headers.h"
#define BUF_SIZE 2000
bool PeerManager::isChoked() const {
    return choked;
}

bool PeerManager::isInterested() const {
    return interested;
}

void PeerManager::setChoked(bool choked) {
    PeerManager::choked = choked;
}

void PeerManager::setInterested(bool interested) {
    PeerManager::interested = interested;
}

void PeerManager::sendChoke(int sockfd)
{
    char buf[5]={0,0,0,1,0};
    write(sockfd,buf,5);
}

void PeerManager::sendUnchoke(int sockfd)
{
    char buf[5]={0,0,0,1,1};
    write(sockfd,buf,5);
}

void PeerManager::sendInterested(int sockfd)
{
    char buf[5]={0,0,0,1,2};
    write(sockfd,buf,5);
}

void PeerManager::sendNotInterested(int sockfd)
{
    char buf[5]={0,0,0,1,3};
    write(sockfd,buf,5);
}

void PeerManager::sendRequest(int sockfd, int index, int begin, int length)
{
    char buf[17]={0,0,1,3,6};
    //integers in big endian order
    //index
    buf[8]=index&0x000000ff;
    buf[7]=index&0x0000ff00>>8;
    buf[6]=index&0x00ff0000>>16;
    buf[5]=index&0xff000000>>24;
    //begin
    buf[12]=begin&0x000000ff;
    buf[11]=begin&0x0000ff00>>8;
    buf[10]=begin&0x00ff0000>>16;
    buf[9]=begin&0xff000000>>24;
    //length
    buf[16]=length&0x000000ff;
    buf[15]=length&0x0000ff00>>8;
    buf[14]=length&0x00ff0000>>16;
    buf[13]=length&0xff000000>>24;
    write(sockfd,buf,17);
}

int PeerManager::readMessage(int sockfd, std::string &response)
{
    int readBytes=0;
    char buffer[BUF_SIZE]={0};
    readBytes= read(sockfd,buffer,BUF_SIZE);
    response=buffer;
    return readBytes;
}

void Downloader::download(int sockfd,std::vector<fileInfo> files,TorrentFile torrentFile,PeerManager peerManager)
{
    std::ofstream file;
    int index=0;//zero-based piece index
    int begin=0;//zero-based byte offset within the piece
    int length=BUF_SIZE;//length requested from the peer
    std::vector<fileInfo>::const_iterator iterator=files.begin();
    std::string pieceBuffer;
    int readBytes=0;
    while(iterator!=files.end())
    {
        file.open(iterator->getFilename(),std::ios::app);
        while(index<torrentFile.getPieceQuantity())
        {
            while(begin<(iterator->getLength()))//this loop will work until we receive all data from file
            {
                peerManager.sendRequest(sockfd, index, begin, length);
                readBytes = peerManager.readMessage(sockfd, pieceBuffer);
                if (readBytes != -1)
                {
                    file << pieceBuffer;
                    pieceBuffer.clear();
                    begin+=readBytes;
                }
                else
                {
                    file.close();
                    exit(0);
                }
            }
            index++;
        }
        file.close();
        iterator++;
    }
}

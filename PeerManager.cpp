#include "headers.h"
#define BUF_SIZE 2000

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
    unsigned char buf[17]={0,0,1,3,6};
    //integers in big endian order
    //index
    buf[8]=index&0x000000ff;
    buf[7]=index&0x0000ff00>>8;
    buf[6]=index&0x00ff0000>>16;
    buf[5]=index&0xff000000>>24;
    //begin
    buf[12]=begin&0x000000ff;
    buf[11]=(begin&0x0000ff00)>>8;
    buf[10]=(begin&0x00ff0000)>>16;
    buf[9]=(begin&0xff000000)>>24;
    //length
    buf[16]=length&0x000000ff;
    buf[15]=(length&0x0000ff00)>>8;
    buf[14]=(length&0x00ff0000)>>16;
    buf[13]=(length&0xff000000)>>24;
    write(sockfd,buf,17);
}

int PeerManager::readMessage(int sockfd, std::string &response)
{
    int readBytes=0;
    response.clear();
    char buffer[BUF_SIZE+13]={0};
    readBytes= read(sockfd,buffer,BUF_SIZE+13);
    for(int i=0;i<readBytes;i++)
    {
        response+=buffer[i];
    }
    return readBytes;
}

void Downloader::download(int sockfd,TorrentFile torrentFile,PeerManager peerManager)
{
    std::ofstream file;
    float progress=0;
    long int total_size=0;//size of torrent in bytes
    long int current_size=0;
    int index=0;//zero-based file index
    int begin=0;//zero-based byte offset within the file
    int length=BUF_SIZE;//length requested from the peer
    std::vector<FileInfo>::const_iterator iterator=torrentFile.files.begin();
    while(iterator!=torrentFile.files.end())
    {
        total_size+=iterator->getLength();
        iterator++;
    }
    iterator=torrentFile.files.begin();
    std::string pieceBuffer;
    std::string buffer;
    int readBytes=0;
    int counter=0;
    std::cout<<"Progress:"<<std::endl;
    while(iterator!=torrentFile.files.end())
    {
        while(begin<(iterator->getLength()))//this loop will work until we receive all data from file
        {
            peerManager.sendRequest(sockfd, index, begin, length);
            readBytes = peerManager.readMessage(sockfd, buffer);
            counter++;
            //std::cout<<counter<<std::endl;
            //pieceBuffer=buffer.substr(13);
            pieceBuffer.clear();
            for(int i=13;i<readBytes;i++)
            {
                pieceBuffer+=buffer[i];
            }
            file.open(iterator->getFilename(),std::ios::app|std::ios::binary);
            if (readBytes != -1)
            {
                file << pieceBuffer;
                file.close();
                //std::cout<<pieceBuffer.size()<<std::endl;
                buffer.clear();
                pieceBuffer.clear();
                pieceBuffer.clear();
                begin+=readBytes-13;
            }
            else
            {
                file.close();
                exit(0);
            }
        }
        current_size+=iterator->getLength();
        progress=(float)(((float)current_size/(float)total_size)*100);

        //std::cout<<"\x1b[42m"<<(char)219;
        std::cout<<"Download progress is: "<<std::setprecision(5)<<progress<<"%"<<std::endl;
        index++;
        iterator++;
        begin=0;
    }
}

int PeerManager::makeHandshake(TorrentFile torrentFile)
{
    int sockfd=0;
    const int handshakeSize=1+19+8+20+20;//size of message for handshake
    char responseBuffer[handshakeSize]={0};
    char flags[8]={0,0,0,0,0,0,0,0};
    std::string handshakeBuffer;
    handshakeBuffer+=19;
    handshakeBuffer+="BitTorrent protocol";
    handshakeBuffer+=flags;
    handshakeBuffer+=(char*)torrentFile.info_hash;
    handshakeBuffer+="HAHA-0142421214125A-";
    sockaddr_in addr;
    addr.sin_family=AF_INET;
//    int n=0;
//    addr.sin_addr.s_addr= inet_addr(allPeers[n].newIp.c_str());
//    addr.sin_port= htons(allPeers[n].getPortNumber());
    addr.sin_addr.s_addr= inet_addr("0.0.0.0");
    addr.sin_port= htons(6881);
    sockfd=socket(AF_INET,SOCK_DGRAM,0);
    connect(sockfd,(struct sockaddr*)&addr,sizeof(addr));
    return sockfd;
}

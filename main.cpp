#include "headers.h"



void printHash(const unsigned char*);

peerInfo* parseResponseInfo(std::string,peerInfo*,int&);

std::string urlEncode(const unsigned char*);
size_t writeFunction(void*, size_t,size_t, std::string*);
peerInfo* makeGetRequest(std::string,peerInfo*,int&);
void makeHandshake(peerInfo*,int,char[],std::vector<fileInfo> files,PeerManager peerManager,TorrentFile torrentFile);


int main() {
    using namespace std;
    peerInfo* allPeers= nullptr;
    int peersQuantity=0;
    string completeUrl;
    TorrentFile torrentFile=TorrentFile("Blasphemous_P_RUS_+_ENG_+_7_ENG_2019_2_0_27_+_6_DLC_Scene_rutracker.torrent");
    torrentFile.calculateInfoHashAndAddress();
    //file.open("blasphemous.torrent", ifstream::binary);
    //file.open("civ.torrent", ifstream::binary);
    torrentFile.extractFilesInfo();
    torrentFile.displayFiles();
    PeerManager peerManager;
    string urlEncodedHash;
    urlEncodedHash=urlEncode(torrentFile.info_hash);
    //cout<<"Quantity of pieces: "<<torrentFile.getPieceQuantity()<<" Length of one piece: "<<torrentFile.getPieceLength()<<std::endl;

    cout<<urlEncodedHash<<endl;
    printHash(torrentFile.info_hash);

    completeUrl+=torrentFile.address+"?info_hash="+urlEncodedHash+"&uploaded=0"+"&downloaded=0"+"&port=6881"+"&left=1239";

    cout<<"Request url: "<<completeUrl<<endl;

    allPeers=makeGetRequest(completeUrl,allPeers,peersQuantity);
    //cout<<"Quantity of peers: "<<peersQuantity<<endl;
    makeHandshake(allPeers,peersQuantity,(char*)torrentFile.info_hash,torrentFile.files,peerManager,torrentFile);
    return 0;
}





size_t writeFunction(void *ptr, size_t size, size_t nmemb, std::string* data) {
    data->append((char*) ptr, size * nmemb);
    return size * nmemb;
}

peerInfo* parseResponseInfo(std::string responseString,peerInfo* allPeers,int& peersQuantity)
{
    int pos=-1;
    unsigned char ip[4]={0};
    unsigned short int portNumber=0;
    for(int i=0;i<responseString.size();i++)
    {
        if(responseString[i]=='p')
        {
            if(responseString[i+1]=='e' && responseString[i+2]=='e' && responseString[i+3]=='r' && responseString[i+4]=='s')
            {
                i+=5;
                while(responseString[i]!=':')
                    i++;
                pos=i+1;
                break;
            }
        }
    }
    std::cout<<"Position is: "<<pos<<std::endl;
    char mem;
    peersQuantity=(responseString.size()-pos)/6;
    allPeers=new peerInfo[peersQuantity];
    for(int iteration=0;iteration<peersQuantity;iteration++) //process all 6-byte strings from current position till the end to extract ip and port number
    {
        int check=0;
        for(int i=pos,j=0;j<4;j++,i++)
        {
            check=0;
            mem=responseString[i];
            for(int temp=128;temp>=1;temp/=2)
            {
                if((mem&temp)==temp)
                    check+=temp;
            }
            ip[j]=check;
            allPeers[iteration].newIp+=std::to_string(check)+'.';
        }
        allPeers[iteration].newIp.erase(allPeers[iteration].newIp.begin()+allPeers[iteration].newIp.size()-1);
//        std::cout<<"Ip is: ";
//        for(int i=0;i<4;i++)
//            std::cout<<(unsigned int)ip[i]<<'.';
//        std::cout<<std::endl;
        check=0;
        mem=responseString[pos+4];
        for(int temp=128;temp>=1;temp/=2)
        {
            if((mem&temp)==temp)
                check+=temp;
        }
        portNumber+=check;
        check=0;
        portNumber<<=8;
        mem=responseString[pos+5];
        for(int temp=128;temp>=1;temp/=2)
        {
            if((mem&temp)==temp)
                check+=temp;
        }
        portNumber+=check;
        //std::cout<<"Port number is: "<<portNumber<<std::endl;
        allPeers[iteration].setPortNumber(portNumber);
        pos+=6;
    }
    std::cout<<"Peers quantity: "<<peersQuantity<<std::endl;
    std::cout<<"Peers info:"<<std::endl;
    for(int i=0;i<peersQuantity;i++)
    {
        std::cout<<"Ip: "<<allPeers[i].newIp;
        std::cout<<"    port: "<<allPeers[i].getPortNumber()<<std::endl;
    }
    return allPeers;
}

peerInfo* makeGetRequest(std::string url,peerInfo* allPeers,int& peersQuantity)
{
    curl_global_init(CURL_GLOBAL_DEFAULT);
    auto curl=curl_easy_init();
    if(curl)
    {
        curl_easy_setopt(curl,CURLOPT_URL,url.c_str());
        curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 1L);
        curl_easy_setopt(curl, CURLOPT_USERPWD, "user:pass");
        curl_easy_setopt(curl, CURLOPT_USERAGENT, "curl/7.42.0");
        curl_easy_setopt(curl, CURLOPT_MAXREDIRS, 50L);
        curl_easy_setopt(curl, CURLOPT_TCP_KEEPALIVE, 1L);

        std::string response_string;
        std::string header_string;
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeFunction);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_string);
        curl_easy_setopt(curl, CURLOPT_HEADERDATA, &header_string);

        char* responseUrl;
        long response_code;
        double elapsed;
        curl_easy_perform(curl);
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
        curl_easy_getinfo(curl, CURLINFO_TOTAL_TIME, &elapsed);
        curl_easy_getinfo(curl, CURLINFO_EFFECTIVE_URL, &responseUrl);
        std::cout<<"ResponseUrl: "<<responseUrl<<std::endl;
        std::cout<<"Response code: "<<response_code<<std::endl;
        std::cout<<"Elapsed: "<<elapsed<<std::endl;
        curl_easy_cleanup(curl);
        curl = NULL;
        std::cout<<"Response string: "<<std::endl;
        std::cout<<response_string<<std::endl;
        //std::cout<<"Header string: "<<header_string<<std::endl;
        allPeers=parseResponseInfo(response_string,allPeers,peersQuantity);
    }
    return allPeers;
}

std::string urlEncode(const unsigned char* hash)
{
    std::ostringstream os;
    os.fill('0');
    os<<std::hex;
    for(int i=0;i<20;i++)
    {
        if(isalnum(hash[i]) || hash[i]=='-' || hash[i]=='_' || hash[i]=='.' || hash[i]=='~')
        {
            os<<hash[i];
        }
        else {
            os << '%' << std::setw(2) << int((unsigned int) hash[i]);
            os << std::nouppercase;
        }
    }
    //std::cout<<os.str()<<std::endl<<std::endl;
    return os.str();
}

void printHash(const unsigned char* test_sha) {

    //std::cout << "CALLED HEX REP...PREPPING TO PRINT!\n";
    std::ostringstream os;
    os.fill('0');
    os << std::hex;
    for (const unsigned char* ptr = test_sha; ptr < test_sha + 20; ptr++) {

        os << std::setw(2) << (unsigned int)*ptr;
    }
    std::string str=os.str();
    //if(str[0]=='2' && str[1]=='4' && str[2]=='2' && str[3]=='c')
        std::cout<<str<<std::endl<<std::endl;
        std::cout<<std::dec;
    //std::cout << os.str() << std::endl << std::endl;
}

void makeHandshake(peerInfo* allPeers,int peersQuantity,char info_hash[],std::vector<fileInfo> files,PeerManager peerManager,TorrentFile torrentFile)
{
    int sockfd;//file descriptor for socket
    const int handshakeSize=1+19+8+20+20;//size of message for handshake
    char responseBuffer[handshakeSize]={0};
    //char handshakeBuffer[handshakeSize]={NULL};

    char flags[8]={0,0,0,0,0,0,0,0};
    std::string handshakeBuffer;
    handshakeBuffer+=19;
    handshakeBuffer+="BitTorrent protocol";
    handshakeBuffer+=flags;
    handshakeBuffer+=info_hash;
    handshakeBuffer+="HAHA-0142421214125A-";


    sockaddr_in addr;
    addr.sin_family=AF_INET;
//    int n=0;
//    addr.sin_addr.s_addr= inet_addr(allPeers[n].newIp.c_str());
//    addr.sin_port= htons(allPeers[n].getPortNumber());
    addr.sin_addr.s_addr= inet_addr("0.0.0.0");
    addr.sin_port= htons(6881);
    sockfd=socket(AF_INET,SOCK_DGRAM,0);
    int wf=0, rf=0;
    if(connect(sockfd,(struct sockaddr*)&addr,sizeof(addr))==0)
    {
        wf=write(sockfd, handshakeBuffer.c_str(), handshakeSize);
        std::cout<<"Connected. Waiting for response."<<std::endl;
        rf=read(sockfd, responseBuffer, handshakeSize);
        std::cout<<"Got response: "<<responseBuffer<<"Response size: "<<rf<<std::endl;
        close(sockfd);
        std::cout<<"Got the data."<<std::endl;
    }
    else
    {
        std::cout<<"Connection failed"<<std::endl;
    }
    Downloader downloader;
    downloader.download(sockfd,files,torrentFile,peerManager);

}
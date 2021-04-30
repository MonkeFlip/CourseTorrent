#include <string>
#include <sstream>
#include <iomanip>
#include <fstream>
#include <iostream>
#include <sys/socket.h>
#include <openssl/sha.h>
#include <curl/curl.h>
#include <vector>
#include <netinet/in.h>
#include <boost/asio.hpp>

class peerInfo
{
private:
    unsigned char *ip;
    unsigned short int portNumber;
public:
    //constructors
    peerInfo()
    {
        ip=(unsigned char*)malloc(7);//4 bytes for ip and 3 for dots
        portNumber=0;//port number consists of 2 bytes
    }
    peerInfo(unsigned  char* ipBuffer, unsigned  short int portNumberBuffer)
    {
        for(int i=1;i<=4;i++)
        {
            this->ip[i*2-2]=ipBuffer[i-1];
            if(i<4)
                this->ip[i*2-1]='.';
        }
        this->portNumber=portNumberBuffer;
    }
    //destructor
    ~peerInfo()
    {
        delete this->ip;
    }
    //getters and setters
    unsigned char* getIp()
    {
        return this->ip;
    }
    unsigned short int getPortNumber()
    {
        return this->portNumber;
    }

    void setIp(unsigned  char* ipBuffer)
    {
        for(int i=1;i<=4;i++)
        {
            this->ip[i*2-2]=ipBuffer[i-1];
            if(i<4)
                this->ip[i*2-1]='.';
        }
    }

    void setPortNumber(unsigned short int portNumberBuffer) {
        this->portNumber=portNumberBuffer;
    }
};
int findStartPos(char*,int);

int findEndPos(char*, int);

void printHash(const unsigned char*);

peerInfo* parseResponseInfo(std::string,peerInfo*,int&);
std::string getAddress(char*, int);
void getHash(char*, unsigned char*, int,std::ifstream*);
std::string urlEncode(const unsigned char*);
size_t writeFunction(void*, size_t,size_t, std::string*);
peerInfo* makeGetRequest(std::string,peerInfo*,int&);
void makeHandshake(peerInfo*,int,const unsigned char[]);


int main() {
    using namespace std;
    peerInfo* allPeers=NULL;
    int peersQuantity=0;
    string completeUrl;
    ifstream file;
    //file.open("bm.torrent", ifstream::binary);
    file.open("blasphemous.torrent", ifstream::binary);
    //file.open("civ.torrent", ifstream::binary);
    if(!file.is_open())
        cout<<"Didn't open"<<endl;
    else
    {

    //Get file length
    file.seekg(0, file.end);
    int fileLength = file.tellg();
    //cout << "File length == " << fileLength << endl;
    char* fileContent=(char*)malloc(fileLength);
    file.seekg(0, file.beg);
    file.read((char*)fileContent,fileLength);//get content of file in buffer

    string address;
    address=getAddress(fileContent,fileLength);
    cout<<address<<endl;
    unsigned char hash[20];
    getHash(fileContent,hash,fileLength,&file);


    string urlEncodedHash;

    urlEncodedHash=urlEncode(hash);


    cout<<urlEncodedHash<<endl;
    printHash(hash);

    completeUrl+=address+"?info_hash="+urlEncodedHash+"&uploaded=0"+"&downloaded=0"+"&port=6881"+"&left=1239";

    cout<<"Request url: "<<completeUrl<<endl;

    allPeers=makeGetRequest(completeUrl,allPeers,peersQuantity);
    //cout<<"Quantity of peers: "<<peersQuantity<<endl;
    makeHandshake(allPeers,peersQuantity,hash);
    file.close();
    }

    return 0;
}

std::string getAddress(char* fileContent,int fileLength)
{
    std::ostringstream addressBuffer;
    int counter1=0;
    int counter2=0;
    int j=0;
    for(int i=0;i<fileLength;i++)
    {
        if(counter1!=2)
        {
            if(fileContent[i]==':')
            {
                counter1++;
            }
            continue;
        }
        else
        {
            if(counter2==3 && (fileContent[i]>='0' && fileContent[i]<='9'))
            {
                break;
            }
            addressBuffer<<fileContent[i];
            if(fileContent[i]=='/')
            {
                counter2++;
            }
            j++;
        }
    }
    return addressBuffer.str();
}

void getHash(char* fileContent,unsigned char* hash,int fileLength,std::ifstream* file)
{
    int start_offset=-2;
    int end_offset=-2;
    start_offset=findStartPos(fileContent,fileLength);
    end_offset=findEndPos(fileContent,fileLength);
    //std::cout<<"Start offset "<<start_offset<<std::endl;
    //std::cout<<"End offset "<<end_offset<<std::endl;

    fileLength-=start_offset;
    fileLength-=end_offset;
    fileContent=(char*)realloc(fileContent,fileLength);
    file->seekg(start_offset, file->beg);
    file->read((char*)fileContent,fileLength);//get content of file in buffer

    SHA1((unsigned char*)fileContent, fileLength, hash);
    //std::cout << "File length == " << fileLength << std::endl;
}

size_t writeFunction(void *ptr, size_t size, size_t nmemb, std::string* data) {
    data->append((char*) ptr, size * nmemb);
    return size * nmemb;
}

peerInfo* parseResponseInfo(std::string responseString,peerInfo* allPeers,int& peersQuantity)
{
    int pos=-1;
    unsigned char ip[4]={NULL};
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
    //allPeers=(peerInfo*)malloc(sizeof(peerInfo)*peersQuantity);
    for(int iteration=0;iteration<peersQuantity;iteration++)
    //while(pos<(responseString.size()-1))//process all 6-byte strings till from current position till the end
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
        }
//        std::cout<<"Ip is: ";
//        for(int i=0;i<4;i++)
//            std::cout<<(unsigned int)ip[i]<<'.';
//        std::cout<<std::endl;
        check=0;
        //int port=0;
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
        allPeers[iteration].setIp(ip);
        allPeers[iteration].setPortNumber(portNumber);
        pos+=6;
    }
    std::cout<<"Peers info:"<<std::endl;
    for(int i=0;i<peersQuantity;i++)
    {
        std::cout<<"Ip: ";
        for(int j=0;j<7;j++)
        {
            if(j%2!=0)
                std::cout<<*(allPeers[i].getIp()+j);
            else
                std::cout<<(unsigned int)*(allPeers[i].getIp()+j);
        }
        std::cout<<" port: "<<allPeers[i].getPortNumber()<<std::endl;
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

int findStartPos(char* content,int fileLength)
{
    int result=-1;
    for(int i=0;i<fileLength;i++)
        if(content[i]=='i' && content[i+1]=='n' && content[i+2]=='f' && content[i+3]=='o')
            result=i+4;
    return result;
}

int findEndPos(char* content,int fileLength)
{
    int result=-1;
    if(isalnum(content[fileLength-2]))
    {
        if(content[fileLength-2]=='e')
        {
            result=1;
        }
        else
        {
            for(int i=3;i<fileLength;i++)
            {
                if(content[fileLength-i]=='e' && (content[fileLength-i+1]>='0' && content[fileLength-i+1]<='9'))
                {
                    result=i-1;
                    break;
                }
            }
        }
    }
    return result;
}

void makeHandshake(peerInfo* allPeers,int peersQuantity,const unsigned char info_hash[])
{
//    hostent* localHost;
//    char* localIP;

    //localHost = gethostbyname("93.125.107.18");
    //localIP = inet_ntoa (*(struct in_addr *)*localHost->h_addr_list);
    //std::cout<<localIP<<std::endl;
    //////////////////////////////////////////////////////////
    int sockfd;//file descriptor for socket
    const int handshakeSize=1+19+8+20+20;//size of message for handshake
    char responseBuffer[handshakeSize]={NULL};
    char handshakeBuffer[handshakeSize]={NULL};
    char peer_id[20];
    for(int i=0;i<20;i++)
    {
        peer_id[i]='A'+i;
    }
    //handshake format: 1 byte for protocol Length+19 bytes of protocol Name+8 reserved bytes
    // +20 bytes of info_hash
    // +20 bytes peer identifier
    int protocolLength=19;
    const std::string protocolName="BitTorrent protocol";
    //info_hash
    //generate peer identifier(read at wiki)
    //unite all these variables into one buffer and write it at socket

    handshakeBuffer[0]=1;
    for(int i=0;i<19;i++)
    {
        handshakeBuffer[1+i]=protocolName[i];
    }
    for(int i=0;i<8;i++)
    {
        handshakeBuffer[20+i]=0;
    }
    for(int i=0;i<20;i++)
    {
        handshakeBuffer[28+i]=info_hash[i];
        handshakeBuffer[48+i]=peer_id[i];
    }
    //////////////////////////////
    addrinfo hints;
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_UNSPEC;    /* Allow IPv4 or IPv6 */
    hints.ai_socktype = SOCK_DGRAM; /* Datagram socket */
    hints.ai_flags = AI_NUMERICSERV;    /* For wildcard IP address */
    hints.ai_protocol = 0;          /* Any protocol */
    hints.ai_canonname = NULL;
    hints.ai_addr = NULL;
    hints.ai_next = NULL;
    ////////////////////////////////
    hostent *host;
    sockaddr_in addr;
    addr.sin_family=AF_INET;
    //addr.sin_port= htons(allPeers[0].getPortNumber());
    addr.sin_port= htons(6881);
    addr.sin_addr.s_addr= inet_addr("93.125.107.17");
    //addr.sin_addr.s_addr= inet_addr((const char*)(allPeers[0].getIp()));
    sockfd=socket(AF_INET,SOCK_DGRAM,0);
    if(connect(sockfd,(struct sockaddr*)&addr,sizeof(addr))==0)
    {
        write(sockfd, handshakeBuffer, handshakeSize);
        read(sockfd, responseBuffer, handshakeSize);
        close(sockfd);
    }
    else
    {
        std::cout<<"Connection failed"<<std::endl;
    }
}
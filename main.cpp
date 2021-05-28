#include "headers.h"



void printHash(const unsigned char*);

PeerInfo* parseResponseInfo(std::string, PeerInfo*, int&);

std::string urlEncode(const unsigned char*);
size_t writeFunction(void*, size_t,size_t, std::string*);
PeerInfo* makeGetRequest(std::string, PeerInfo*, int&);

int main() {
    using namespace std;
    //PeerInfo* allPeers= nullptr;
    int peersQuantity = 0;
    string completeUrl;
    int option = 0;
    std::string downloadDirectory;
    std::string torrentName;
    while(1)
    {
        std::cout << "1) Download files using torrent-file." << std::endl;
        std::cout << "2) Settings." << std::endl;
        std::cout << "3) Exit." << std::endl;
        std::cin >> option;
        switch (option) {
        case 1: {
            std::cout << "Enter torrent-file name." << std::endl;
            cin.ignore();
            std::getline(std::cin, torrentName);
            cout<<"1) Download to default directory."<<endl;
            cout<<"2) Select directory for download."<<endl;
            cout<<"3) Back."<<endl;
            std::cin>>option;
            switch (option)
            {
                case 1:
                {
                    ifstream settings_file;
                    settings_file.open("settings.txt");
                    getline(settings_file,downloadDirectory);
                    if(downloadDirectory[downloadDirectory.size()-1]!='/')
                    {
                        downloadDirectory+='/';
                    }
                    cout<<"Default download directory: "<<downloadDirectory<<endl;
                    settings_file.close();
                    break;
                }
                case 2:
                {
                    cout<<"Enter path to download directory."<<endl;
                    cin.ignore();
                    getline(cin,downloadDirectory);
                    break;
                }
                case 3:
                {
                    break;
                }
                default:
                    break;
            }
            TorrentFile torrentFile = TorrentFile(torrentName);
            torrentFile.calculateInfoHashAndAddress();
            torrentFile.extractFilesInfo(downloadDirectory);
            torrentFile.displayFiles();

            PeerManager peerManager;
            string urlEncodedHash;
            urlEncodedHash = urlEncode(torrentFile.info_hash);
            cout << urlEncodedHash << endl;
            printHash(torrentFile.info_hash);
            completeUrl += torrentFile.address + "?info_hash=" + urlEncodedHash + "&uploaded=0" + "&downloaded=0" +
                    "&port=6881" + "&left=1239";

            cout << "Request url: " << completeUrl << endl;

            peerManager.allPeers = makeGetRequest(completeUrl, peerManager.allPeers, peersQuantity);
            int sockfd = 0;
            sockfd = peerManager.makeHandshake(torrentFile);
            Downloader downloader;
            downloader.download(sockfd, torrentFile, peerManager);
            //cout<<"Quantity of peers: "<<peersQuantity<<endl;
        }
        case 2: {
            std::cout<<"1) Set default download directory."<<std::endl;
            std::cout<<"2) Back."<<std::endl;
            cin.ignore();
            cin>>option;
            switch (option)
            {
                case 1:
                {
                    std::cout<<"Enter path to default download directory."<<std::endl;
                    cin.ignore();
                    getline(cin,downloadDirectory);
                    std::ofstream settings_file;
                    settings_file.open("settings.txt",ios_base::out);
                    settings_file<<downloadDirectory;
                    settings_file.close();
                    break;
                }
                case 2:
                {
                    break;
                }
                default:
                    break;
            }
            break;
        }
        case 3: {
            return 0;
        }
        default:
            break;
        }
    }
    return 0;
}





size_t writeFunction(void *ptr, size_t size, size_t nmemb, std::string* data) {
    data->append((char*) ptr, size * nmemb);
    return size * nmemb;
}

PeerInfo* parseResponseInfo(std::string responseString, PeerInfo* allPeers, int& peersQuantity)
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
    allPeers=new PeerInfo[peersQuantity];
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
            allPeers[iteration].ip+=std::to_string(check)+'.';
        }
        allPeers[iteration].ip.erase(allPeers[iteration].ip.begin()+allPeers[iteration].ip.size()-1);
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
        std::cout<<"Ip: "<<allPeers[i].ip;
        std::cout<<"    port: "<<allPeers[i].getPortNumber()<<std::endl;
    }
    return allPeers;
}

PeerInfo* makeGetRequest(std::string url, PeerInfo* allPeers, int& peersQuantity)
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

#include "TorrentFile.h"
void TorrentFile::addFile(fileInfo file)
{
    this->files.push_back(file);
}

void TorrentFile::extractFilesInfo()
{
    int nameLength=0;
    fileInfo fi;
    std::ifstream file;
    file.open(this->torrentName, std::ifstream::binary);
    file.seekg(0, file.end);
    int fileLength = file.tellg();
    char *fileBuffer = (char *) malloc(fileLength);
    file.seekg(0, file.beg);
    file.read((char *) fileBuffer, fileLength);//get content of file in buffer
    file.close();
    std::string fileContent=fileBuffer;
    std::string buffer;
    free(fileBuffer);
    std::string::size_type positionInFile=0;
    positionInFile=fileContent.find("files");
    if(positionInFile!=std::string::npos)
    {
        while (1)
        {
            positionInFile = fileContent.find("lengthi", positionInFile);
            positionInFile += 7;
            //std::cout<<"Position: "<<positionInFile<<std::endl;
            while (fileContent[positionInFile] != 'e') {
                buffer += fileContent[positionInFile];
                positionInFile++;
            }
            fi.setLength(stoi(buffer));
            buffer.clear();
            positionInFile = fileContent.find("pathl", positionInFile);
            //std::cout<<"Position2: "<<positionInFile<<std::endl;
            positionInFile += 5;
            while (fileContent[positionInFile] != ':') {
                buffer += fileContent[positionInFile];
                positionInFile++;
            }
            nameLength = stoi(buffer);
            buffer.clear();
            positionInFile++;
            bool nameEnd = false;
            while (!nameEnd) {
                while (nameLength > 0) {
                    buffer += fileContent[positionInFile];
                    positionInFile++;
                    nameLength--;
                }
                if (isalpha(fileContent[positionInFile])) {
                    nameEnd = true;
                }
                if (!nameEnd) {
                    buffer.clear();
                    while (fileContent[positionInFile] != ':') {
                        buffer += fileContent[positionInFile];
                        positionInFile++;
                    }
                    nameLength = stoi(buffer);
                    buffer.clear();
                    positionInFile++;
                }
            }
            fi.setFilename(buffer);
            buffer.clear();
            this->addFile(fi);
            if (fileContent.find("pathl", positionInFile) == std::string::npos) {
                break;
            }
        }
    }
    else
    {
        positionInFile=fileContent.find("infod6:lengthi");
        positionInFile+=14;
        while(fileContent[positionInFile]!='e')
        {
            buffer+=fileContent[positionInFile];
            positionInFile++;
        }
        fi.setLength(stoi(buffer));
        buffer.clear();
        positionInFile=fileContent.find("name",positionInFile);
        positionInFile+=4;
        while(fileContent[positionInFile]!=':')
        {
            buffer+=fileContent[positionInFile];
            positionInFile++;
        }
        positionInFile++;//skip ";"
        nameLength=stoi(buffer);
        buffer.clear();
        while(nameLength>0)
        {
            buffer+=fileContent[positionInFile];
            positionInFile++;
            nameLength--;
        }
        fi.setFilename(buffer);
        buffer.clear();
        this->addFile(fi);
    }
}

void TorrentFile::setTorrentName(const std::string &torrentName) {
    TorrentFile::torrentName = torrentName;
}

TorrentFile::TorrentFile(const std::string &torrentName) : torrentName(torrentName) {}

fileInfo::fileInfo() {}

void TorrentFile::calculateInfoHashAndAddress() {
    std::ifstream file;
    file.open(this->torrentName,std::ifstream::binary);
    if(!file.is_open())
        std::cout<<"Failed to open file."<<std::endl;
    else {

        //Get file length
        file.seekg(0, file.end);
        int fileLength = file.tellg();
        //cout << "File length == " << fileLength << endl;
        char *fileContentBuffer = (char *) malloc(fileLength);
        file.seekg(0, file.beg);
        file.read((char *) fileContentBuffer, fileLength);//get content of file in buffer
        //this->extractPieceLengthAndQuantity(fileContentBuffer);
        //std::cout<<"File content: "<<std::endl;
        std::cout<<fileContentBuffer<<std::endl;
        this->address = getAddress(fileContentBuffer, fileLength);
        std::cout << this->address << std::endl;
        getHash(fileContentBuffer, this->info_hash, fileLength, &file);
        file.close();
        free(fileContentBuffer);
    }
}

void fileInfo::setFilename(const std::string &filename) {
    fileInfo::filename = filename;
}

void fileInfo::setLength(unsigned long length) {
    fileInfo::length = length;
}

const std::string &fileInfo::getFilename() const {
    return filename;
}

unsigned long fileInfo::getLength() const {
    return length;
}

void TorrentFile:: getHash(char* fileContent,unsigned char* hash,int fileLength,std::ifstream* file)
{
    int start_offset=-2;
    int end_offset=-2;
    start_offset=findStartPos(fileContent,fileLength);
    end_offset=findEndPos(fileContent,fileLength);

    fileLength-=start_offset;
    fileLength-=end_offset;
    fileContent=(char*)realloc(fileContent,fileLength);
    file->seekg(start_offset, file->beg);
    file->read((char*)fileContent,fileLength);//get content of file in buffer

    SHA1((unsigned char*)fileContent, fileLength, hash);
    //std::cout << "File length == " << fileLength << std::endl;
}

std::string TorrentFile::getAddress(char* fileContent,int fileLength)
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

int TorrentFile:: findStartPos(char* content,int fileLength)
{
    int result=-1;
    for(int i=0;i<fileLength;i++)
        if(content[i]=='i' && content[i+1]=='n' && content[i+2]=='f' && content[i+3]=='o')
            result=i+4;
    return result;
}

int TorrentFile:: findEndPos(char* content,int fileLength)
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

void TorrentFile::displayFiles()
{
    std::vector<fileInfo>::const_iterator iterator;
    iterator=this->files.begin();
    std::cout<<"List of files:"<<std::endl;
    while(iterator!=this->files.end())
    {
        std::cout<<iterator->getFilename()<<" "<<iterator->getLength()<<std::endl;
        iterator++;
    }
}

//void TorrentFile::extractPieceLengthAndQuantity(char *fileContentBuffer) {
//    int position=0;//position in string "fileContent"
//    std::string buffer;
//    std::string fileContent=fileContentBuffer;
//    position=fileContent.find("piece lengthi");
//    position+=13;
//    while(fileContent[position]!='e')
//    {
//        buffer+=fileContent[position];
//        position++;
//    }
//    //std::cout<<"Content of buffer with length: "<<buffer<<std::endl;
//    this->setPieceLength(stoi(buffer));
//    buffer.clear();
//    position=fileContent.find("pieces",position);
//    position+=6;
//    while(fileContent[position]!=':')
//    {
//        buffer+=fileContent[position];
//        position++;
//    }
//    //std::cout<<"Content of buffer with quantity: "<<buffer<<std::endl;
//    this->setPieceQuantity(stoi(buffer));
//}
//
//void TorrentFile::setPieceLength(unsigned int pieceLength) {
//    TorrentFile::pieceLength = pieceLength;
//}
//
//void TorrentFile::setPieceQuantity(unsigned int pieceQuantity) {
//    TorrentFile::pieceQuantity = pieceQuantity;
//}
//
//unsigned int TorrentFile::getPieceLength() const {
//    return pieceLength;
//}
//
//unsigned int TorrentFile::getPieceQuantity() const {
//    return pieceQuantity;
//}


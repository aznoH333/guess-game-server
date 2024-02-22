#ifndef COMMUNICATION_STRUCT
#define COMMUNICATION_STRUCT

#include <string>
#include "../utils/utils.h"

namespace Communication {


    const int MAX_CONTENT_SIZE = 128;

    enum CommunicationCode{
        ERROR = 0,
        TEXT,
        CLOSE_CONNECTION,
        LIST_PLAYERS,
        PLAY,
        GIVE_UP,
    };
    
    struct CommStruct{
        char communicationCode;
        int contentSize;
    };


    union CommHeader {
        CommStruct content;
        char bytes[sizeof(CommStruct)];
    };

    struct CommunicationContent{
        char bytes[MAX_CONTENT_SIZE];
    };

    struct CommunicationPacket{
        CommHeader header;
        CommunicationContent content;
    };

    union PacketUnion{
        CommunicationPacket packet;
        char bytes[sizeof(CommunicationPacket)];
    };

    

    

    std::string getTextFromContent(CommunicationPacket& packet);

    // response utility functions
    PacketUnion error();
    PacketUnion text(std::string text);
    PacketUnion closeConnection();
    void readPlayPacket(CommunicationPacket& packet, int& outId, std::string& outWord);
    PacketUnion play();

}

#endif
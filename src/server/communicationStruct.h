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
    };

    
    
    
    
    struct CommStruct{
        char communicationCode;
        int contentLength;
    };


    union CommUnion {
        CommStruct comm;
        char bytes[sizeof(CommStruct)];
    };

    struct CommunicationContent{
        char bytes[MAX_CONTENT_SIZE];
    };

    struct CommunicationPacket{
        CommUnion header;
        CommunicationContent content;
    };

    std::string getTextFromContent(CommunicationPacket& packet);

    // response utility functions
    CommunicationPacket error();
    CommunicationPacket text(std::string text);
    CommunicationPacket closeConnection();
    void readPlayPacket(CommunicationPacket& packet, int& outId, std::string& outWord);
    CommunicationPacket play();

}

#endif
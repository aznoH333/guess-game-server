#ifndef COMMUNICATION_STRUCT
#define COMMUNICATION_STRUCT

#include <string>

namespace Communication {


    const int MAX_CONTENT_SIZE = 128;

    enum CommunicationCode{
        ERROR = 0,
        TEXT
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
        char content[MAX_CONTENT_SIZE];
    };

    struct CommunicationPacket{
        CommUnion header;
        CommunicationContent content;
    };

    // response utility functions
    CommunicationPacket error();
    CommunicationPacket text(std::string text);

}

#endif
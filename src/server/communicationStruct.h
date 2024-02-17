#ifndef COMMUNICATION_STRUCT
#define COMMUNICATION_STRUCT

#include <string>

namespace Communication {



    const int MAX_CONTENT_SIZE = 64;


    enum CommunicationCode{
        ERROR = 0,
        CLOSE_CONNECTION,
        LIST,
        CHALLANGE,
        TEXT,
    };
    
    
    
    struct CommStruct{
        char communicationCode;
        char content[MAX_CONTENT_SIZE];
    };

    union CommUnion {
        CommStruct res;
        char bytes[sizeof(CommStruct)];
    };

    // response utility functions
    CommUnion error();
    CommUnion text(std::string text);
    CommUnion closeConnection();

}

#endif
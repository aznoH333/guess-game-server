#include "communicationStruct.h"


namespace Communication {

    CommUnion error(){
        return {
            CommunicationCode::ERROR, 
            {}
        };
    }

    CommUnion text(std::string text){
        
        CommUnion output;
        output.res.communicationCode = CommunicationCode::TEXT;

        for (int i = 0; i < MAX_CONTENT_SIZE; i++){
            output.res.content[i] = text[i] * (i < text.size());
        }
        
        return output;
    }

    CommUnion closeConnection(){
        return {
            CommunicationCode::CLOSE_CONNECTION,
            {},
        };
    }
}

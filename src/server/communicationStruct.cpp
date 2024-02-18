#include "communicationStruct.h"
#include <iostream>


namespace Communication {

    CommunicationPacket error(){
        return {
            CommunicationCode::ERROR, 
            0
        };
    }

    CommunicationPacket text(std::string text){
        
        CommunicationPacket output;
        output.header.comm.communicationCode = CommunicationCode::TEXT;
        output.header.comm.contentLength = text.length();


        for (int i = 0; i < text.length(); i++){
            output.content.content[i] = text[i];
            std::cout << i << " : " << text[i] << "\n";

        }
        
        return output;
    }

}

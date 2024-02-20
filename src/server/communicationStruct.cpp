#include "communicationStruct.h"
#include <iostream>


namespace Communication {

    CommunicationPacket error(){
        CommunicationPacket out;

        out.header.comm.communicationCode = CommunicationCode::ERROR;
        out.header.comm.contentLength = 0;
        
        return out;
    }

    CommunicationPacket text(std::string text){
        
        CommunicationPacket output;
        output.header.comm.communicationCode = CommunicationCode::TEXT;
        output.header.comm.contentLength = text.length();


        for (int i = 0; i < (int)text.length(); i++){
            output.content.bytes[i] = text[i];
        }
        
        return output;
    }

    CommunicationPacket closeConnection(){
        CommunicationPacket out;

        out.header.comm.communicationCode = CommunicationCode::CLOSE_CONNECTION;
        out.header.comm.contentLength = 0;
        
        return out;
    }

    std::string getTextFromContent(CommunicationPacket& packet){
        std::string out = "";

        for (int i = 0; i < packet.header.comm.contentLength; i++){
            out += packet.content.bytes[i];
        }

        return out;
    }

    void readPlayPacket(CommunicationPacket& packet, int& outId, std::string& outWord){
        // read player id
        outId = Utils::readIntFromBytes(packet.content.bytes, 0);
        outWord = "";
        // read word
        for (int i = 4; i < packet.header.comm.contentLength; i++){
            outWord += packet.content.bytes[i];
        }
    }


    CommunicationPacket play(){
        CommunicationPacket output;
        output.header.comm.communicationCode = CommunicationCode::PLAY;
        output.header.comm.contentLength = 0;
        return output;


    }

}

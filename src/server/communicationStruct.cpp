#include "communicationStruct.h"

namespace Communication {

    PacketUnion error(){
        CommunicationPacket out;

        out.header.content.communicationCode = CommunicationCode::ERROR;
        out.header.content.contentSize = 0;
        
        return {out};
    }

    PacketUnion text(std::string text){
        
        CommunicationPacket output;
        output.header.content.communicationCode = CommunicationCode::TEXT;
        output.header.content.contentSize = text.length();


        for (int i = 0; i < (int)text.length(); i++){
            output.content.bytes[i] = text[i];
        }

        return {output};
    }

    PacketUnion closeConnection(){
        CommunicationPacket out;

        out.header.content.communicationCode = CommunicationCode::CLOSE_CONNECTION;
        out.header.content.contentSize = 0;
        
        return {out};
    }

    std::string getTextFromContent(CommunicationPacket& packet){
        std::string out = "";

        for (int i = 0; i < packet.header.content.contentSize; i++){
            out += packet.content.bytes[i];
        }

        return out;
    }

    void readPlayPacket(CommunicationPacket& packet, int& outId, std::string& outWord){
        // read player id
        outId = Utils::readIntFromBytes(packet.content.bytes, 0);
        outWord = "";
        // read word
        for (int i = 4; i < packet.header.content.contentSize; i++){
            outWord += packet.content.bytes[i];
        }
    }


    PacketUnion play(){
        CommunicationPacket output;
        output.header.content.communicationCode = CommunicationCode::PLAY;
        output.header.content.contentSize = 0;
        return {output};
    }

}

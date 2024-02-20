#include "utils.h"

namespace Utils {
    std::vector<std::string> splitString(std::string input, std::string delimiter){
        
        std::vector<std::string> output;
        int index = 0;

        while(index < (int)input.length()){
            std::string s = input.substr(index);
            int nextPos = s.find(delimiter);
            if (nextPos == -1) nextPos = s.length();
            output.push_back(s.substr(0, nextPos));
            index += nextPos + delimiter.length();
        }

        return output;
    }


    void storeIntAsBytes(int value, char* bytesArray, int index){
        
        for (int i = 0; i < 4; i++){
            int maskedValue = value & (BIT_MASK << (i * 8));
            maskedValue = maskedValue >> ( i * 8);
            bytesArray[index + i] = (char) maskedValue;
        }
    }

    int readIntFromBytes(char* bytesArray, int index){
        int result = 0b0;


        for (int i = 0; i < 4; i++){
            int temp = (BIT_MASK & bytesArray[i + index]) << (i * 8);
            result = result | temp;
        }

        return result;
    }

}
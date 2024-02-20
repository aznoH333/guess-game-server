#ifndef UTILS
#define UTILS

#include <vector>
#include <string>

namespace Utils {
    const int BIT_MASK = 0b00000000000000000000000011111111;
    
    std::vector<std::string> splitString(std::string input, std::string delimiter);
    void storeIntAsBytes(int value, char* bytesArray, int index);
    int readIntFromBytes(char* bytesArray, int index);


}

#endif
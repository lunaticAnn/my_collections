#ifndef MDFIVE_H
#define MDFIVE_H

#define A 0x67452301
#define B 0xefcdab89
#define C 0x98badcfe
#define D 0x10325476

#include <cstdio>
#include <fstream>
#include <iostream>
#include <cmath>
#include <string>
#include <vector>
#include <exception>
#include <sstream>

using namespace std;
class MD5{
    public:
        MD5();
        /*
          short id: give the shorter uid with a0^b0^c0^d0
          else will provide the full MD5        
        */
        string GetMD5(const string& str, bool short_id = false);
    private:
        const int s[64] = {7, 12, 17, 22,  7, 12, 17, 22,  7, 12, 17, 22,  7, 12, 17, 22,
                           5,  9, 14, 20,  5,  9, 14, 20,  5,  9, 14, 20,  5,  9, 14, 20,     
                           4, 11, 16, 23,  4, 11, 16, 23,  4, 11, 16, 23,  4, 11, 16, 23,
                            6, 10, 15, 21,  6, 10, 15, 21,  6, 10, 15, 21,  6, 10, 15, 21
        };
        unsigned int k[64];
};

inline void print_stream(vector<bool>& string_stream){
    for(size_t i = 0; i < string_stream.size(); i++){
        cout<<string_stream[i];
    }
    cout<<endl;
}

inline unsigned int stream2int(int start, int size, vector<bool>& string_stream){
    try{
        unsigned int res = 0;
        for(size_t i = start; i < start + size; i++){
              res = (res<<1)+string_stream[i];                      
        }
        return res;
    }
    catch (exception& e){
        cout << "Standard exception: " << e.what() << endl;
    }
}

inline unsigned int leftrotate(unsigned int x, int c){
       return (x<<c)|(x>>(32-c));
}

#endif

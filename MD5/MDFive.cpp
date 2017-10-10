#include "MDFive.h"

using namespace std;


MD5::MD5(){
    for(size_t i = 0; i < 64; i++){
        double tmp = pow(2,32) * abs(sin(i+1));
        k[i] = (unsigned int)floor(tmp);
    }
}

string MD5::GetMD5(const string& str, bool short_id){
    long long size = str.size() * sizeof(char) * 8;
    
    //one leading essential bit    
    vector<bool> stream(1, true);
    
    //convert to bit stream
    for(size_t i = 0; i < str.size(); i++){
        for (int roller = 7; roller >= 0; roller--){
            stream.push_back((str[i]>>roller) & 1);            
        }        
    }
    
    //padding stream
    int rd = stream.size()%512;
    int padding_size = rd > 448 ? 960 - rd: 448 - rd;
    for(size_t i = 0; i < padding_size; i++){
        stream.push_back(0);
    }
    
    //padding size
    for(size_t i = 1; i < 65; i++){
        stream.push_back((size>>(64-i))&1);
    }

    unsigned int a0 = A;
    unsigned int b0 = B;
    unsigned int c0 = C;
    unsigned int d0 = D;
    
    int chunck = stream.size() / 512;
    for(int t = 0; t < chunck ; t++){
        unsigned int a = a0;
        unsigned int b = b0;
        unsigned int c = c0;
        unsigned int d = d0;
        for(size_t i = 0; i < 64; i++){
            unsigned int f,g;
            if(i >= 0 && i <= 15){
                f = (b&c)|((!b)&d);
                g = i;
            }
            if(i >= 16 && i <= 31){
                f = (b&d)|((!d)&c);
                g = (i *5 + 1)%16;
            }
            if(i >= 32 && i <= 47){
                f = b^c^d;
                g = (i * 3 + 5)%16;
            }
            if(i >= 48 && i <= 63){
                f = c^(b|(!d));
                g = (i * 7) %16;
            }
            f = f + a + k[i] + stream2int(32*g, 32, stream);
            a = d;
            d = c;
            c = b;
            b = b + leftrotate(f,s[i]);
        } 
        a0 = a0 + a;
        b0 = b0 + b;
        c0 = c0 + c;
        d0 = d0 + d;
    }

    if(short_id){
        return to_string(a0^b0^c0^d0);
    }
    stringstream s;
    s << hex << a0 << b0 << c0 << d0;
    return s.str();
}

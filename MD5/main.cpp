#include "MDFive.h"

using namespace std;
int main(){
    MD5* md5 = new MD5();
    string s0 = md5 -> GetMD5("The quick brown fox jumps over the lazy dog");
    string s1 = md5 -> GetMD5("The quick brown fox jumps over the lazy dog", true);
    cout<<"full id:"<<s0<<endl<<"short id:"<<s1<<endl;
    return 0;
}

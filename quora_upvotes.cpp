#include <cmath>
#include <cstdio>
#include <vector>
#include <iostream>
#include <algorithm>
using namespace std;

void windowed_result(vector<long long>& input_array, int array_size, int window_size){
    if(window_size==1){
        for(int i = 0; i < array_size;i++)
              cout<<0<<endl;
        return;
    }
    vector<int> inc,dec;
    //0 dec, 1 inc
    int increase = -1;
    inc.push_back(1);
    dec.push_back(1);
    vector<int> states;
 
    //first window
    for(int i = 0; i < window_size-1; i++){
        if(input_array[i]<input_array[i+1]){
            if(increase==0||increase==-1){
                dec.push_back(1);
                increase=1;
            }
            inc[inc.size()-1]++;
            states.push_back(1);
        }
        else if(input_array[i]>input_array[i+1]){
            if(increase==1||increase==-1){
                inc.push_back(1);
                increase=0;
            }
            dec[dec.size()-1]++;
            states.push_back(0);
        }
        else{
            inc[inc.size()-1]++;
            dec[dec.size()-1]++;
            states.push_back(-1);
			increase=-1;
        }
    }
   long long sum_inc = 0, sum_dec = 0;
    
   for(int i = 0;i < inc.size();i++){
        sum_inc += (inc[i]*inc[i]-inc[i])/2;
    }
	
    for(int i = 0;i < dec.size();i++){
        sum_dec += (dec[i]*dec[i]-dec[i])/2;
    }
	
    long long res = sum_inc - sum_dec;
    cout<< res<<endl;
    for(int i = window_size; i< array_size;i++){
        if(inc[0]==1) inc.erase(inc.begin());
        if(dec[0]==1) dec.erase(dec.begin());
        long long change = 0;
        if(states[i-window_size]==1){
            inc[0]--;
            change-=inc[0];
        }
        if(states[i-window_size]==0){
            dec[0]--;
            change+=dec[0];
        }
        if(states[i-window_size]==-1){
            dec[0]--;
            inc[0]--;
            change+=dec[0] - inc[0];
        }
        if(input_array[i-1]<input_array[i]){
            if(increase==0){
                dec.push_back(1);
                increase=1;
            }
            change += inc[inc.size()-1];
            inc[inc.size()-1]++;            
            states.push_back(1);
        }
        else if(input_array[i-1]>input_array[i]){
            if(increase==1){
                inc.push_back(1);
                increase=0;
            }
            change -= dec[dec.size()-1];
            dec[dec.size()-1]++;            
            states.push_back(0);
        }
        else{
            change += inc[inc.size()-1]-dec[dec.size()-1];
            inc[inc.size()-1]++;
            dec[dec.size()-1]++;          
            states.push_back(-1);
        }
        res+=change;
        cout<<res<<endl;
    }
    
}

int main() {
    /* Enter your code here. Read input from STDIN. Print output to STDOUT */
    freopen("input.in","r", stdin);
	freopen("output.out","w",stdout);
    int array_size, window_size;
    cin>>array_size>>window_size;
    vector<long long> input(array_size);
    for(int i=0; i < array_size; i++){
        cin>>input[i];
    }
    windowed_result(input,array_size,window_size);
    return 0;
}



#include "MDFive.h"
#include <fstream>
#include <unordered_map>
#include <unordered_set>

/*
dialoge extraction from DB
*/
using namespace std;
struct chat_item{
    string chat_id;
    string reply_to;
    string chat_content;
    string usr_id;
    string usr_name;
    chat_item(){
        chat_id = "";
        reply_to = "";
        chat_content = "";
        usr_id = "";
        usr_name = "";
    }
    void clear(){
        chat_id = "";
        reply_to = "";
        chat_content = "";
        usr_id = "";
        usr_name = "";
    }
};

int main(int argc, char* argv[]){
    unordered_map<string, chat_item*> records;
    unordered_set<string> has_reply;
    string infile_name(argv[1]);
    infile_name += ".txt";
    string ofile_name(argv[1]);
    ofile_name += "_processed.txt";
    freopen(infile_name.c_str(), "r", stdin);
    freopen( ofile_name.c_str(), "w", stdout);
    string input_string;
    MD5* md5 = new MD5();
    cin>>input_string;
    bool next_id = false, next_name = false, next_content = false, rt = false;
    chat_item* ci = new chat_item();
    while(!cin.eof()){       
        cin>>input_string;
        if(input_string == "RT" || input_string == "R" ||input_string == "M"){
             if(rt){
                // get MD5 of reply-to content
                string md5_rt = md5 -> GetMD5(ci -> usr_id + ci -> chat_content, true);
                ci -> clear();
                ci -> reply_to = md5_rt;
                rt = false;
            }
            else{
                string md5_c = md5 -> GetMD5(ci -> usr_id + ci -> chat_content, true);
                ci -> chat_id = md5_c;
                records[md5_c] = ci;
                
                if(ci->reply_to != ""){
                    has_reply.insert(ci -> reply_to);
                }
                ci = new chat_item();
            }            
            if(input_string == "RT"){
                rt = true;
            }
            next_content = false;
            next_id = true;                  
        }
        else if(next_id){
            ci -> usr_id = input_string;
            next_id = false;
            next_name = true;
        }
        else if(next_name){
            ci -> usr_name = input_string;
            next_name = false;
            next_content = true;
        }
        else if(next_content){
            ci -> chat_content += input_string;
        }
    }

    int tested = 0;
    for(auto it = records.begin(); it != records.end(); it++){
        //no reply to it, probably start node
        
        if(has_reply.find(it->first)==has_reply.end()){
            chat_item* test_it = it -> second;
            int chat_cnt = 0;
            string chat = "";
            unordered_set<string> visited;
            while(test_it -> reply_to != ""){
                visited.insert(test_it -> chat_id);
                chat_cnt ++;
                chat = "user:" + test_it -> chat_content + "\n" + chat;
                if(records.find(test_it->reply_to)==records.end()){
                    cout<<"[error]error in hashing"<<endl;
                    break;                
                }
                test_it = records[test_it->reply_to];    
                if(visited.find(test_it -> chat_id)!= visited.end()){
                    cout<<"[error]circle detected"<<endl;
                    break;
                }        
            }
            chat = "CHAT \nuser:" + test_it -> chat_content + "\n" + chat;
            if(chat_cnt > 0){
                cout<<chat<<endl;
            }        
        }
        
    }
        
    return 0;
}

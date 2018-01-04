/*==================================================
process identified by starting time and pid
warning details in:
detail_logs/HOUR_MINUTE_SECOND_PID
=====================================================*/
//#define __DEBUG_DETAILS__ 
#define _CRT_SECURE_NO_WARNINGS
#include <sys/types.h>
#include <sys/stat.h>
#include <fstream>
#include <stdio.h>     
#include <stdlib.h>  
#include <string>
#include <unordered_map>
#include <stdexcept>

long long THRESHOLD = 100000;
float THRESHOLD_TIME = 60.;
const char* helper = 
"usage: process [LogFile] [-t time_threshold(s)] [-m memory_threshold(kb)]\n";
struct proc{
    int progress;
    int hour;
    int min;
    float sec;
    int pid;
    long long vm_size_s;
    long long rss_size_s;
    long long vm_use;
    long long rss_use;
    std::string query;
    std::string whole_process;
    
    proc(int prog, int h, 
         int m, float s,
         long long v_s,
         long long r_s, 
         int pi, std::string& q){
        progress = prog;
        hour = h;
        min = m;
        sec = s;
        pid = pi;
        vm_size_s = v_s;
        rss_size_s = r_s;
        query = q;
    }

    ~proc(){}

    int log_proc(float time_lapse){
        if(vm_use > THRESHOLD || rss_use > THRESHOLD || 
           time_lapse > THRESHOLD_TIME||time_lapse < 0){
            std::string file_name = std::to_string(hour) + "_"
                                 + std::to_string(min) + "_"
                                 + std::to_string(sec) + "_"
                                 + std::to_string(pid) + ".txt";
            std::ofstream ofs_warn;
            ofs_warn.open ("warning.log", std::ofstream::out | std::ofstream::app);
            if(vm_use > THRESHOLD || rss_use > THRESHOLD){ 
                std::ofstream ofs("detail_logs/[Memory]" + file_name);
                ofs_warn << "[Warning] High Memory Usage."
                <<"["<<pid<<"]"<<query<<"\n";
                ofs << "[Warning] High Memory Usage. \n";
                ofs << "vm usage:" << vm_use << "\n";
                ofs << "vm usage:" << vm_use << "\n";
                ofs << whole_process;
                ofs.close();             
            }
            if(time_lapse > THRESHOLD_TIME){
                std::ofstream ofs("detail_logs/[Time]" + file_name);
                ofs_warn << "[Warning] Long Query Time."
                <<"["<<pid<<"]"<<query<<"\n";
                ofs << "[Warning] Long Query Time. \n";
                ofs << "Timelapse:" << time_lapse << "\n";
                ofs << whole_process;
                ofs.close();
            }
            if(time_lapse < 0){
                std::ofstream ofs("detail_logs/[NotFin]" + file_name);
                ofs_warn << "[Warning] Process Not Finished."
                <<"["<<pid<<"]"<<query<<"\n";
                ofs << "[Warning] Process was not finished. \n";
                ofs << whole_process;
                ofs.close();
            }
            ofs_warn.close();
                       
        }
        return 0;
    } 
    
};

float calc_time(int h, int m, float s){
    return h * 3600.0 + m * 60.0 + s;  
}

int process_log(std::string& str, std::unordered_map<int, proc*>* process_list){
    
    int idx_progress = str.find("run processor");
    //it is a log line    
    if(idx_progress != -1){        
        int p_start = str.substr(idx_progress + 4).find(" ") + idx_progress + 5;
        int p_end = str.substr(idx_progress).find("/") + idx_progress;
        int progress = std::stoi(str.substr(p_start, p_end - p_start));
        
        int idx_time = str.find(" ");
        std::string time_str = str.substr(idx_time + 1, 15); 
                       
        int h = std::stoi(time_str.substr(0,2));    
        int m = std::stoi(time_str.substr(3,2));
        float s = std::stof(time_str.substr(6));
        

        int pid_len = str.substr(idx_time + 17).find(" ");
        int pid = std::stoi(str.substr(idx_time + 17, pid_len));
        
        #ifdef __DEBUG_DETAILS__ 
            printf("%s\n",str.c_str());
            printf("--->PID: [%d]\n", pid);
            printf("--->Progress: [%d]\n", progress);
            printf("--->Time: [%s]\n", time_str.c_str());
        #endif       

        int processor_len = str.substr(p_end).find(",") - 4;
 
        int vm_size = str.find("vm size kb:") + 12;
        int vm_size_len = str.substr(vm_size).find(",");
        long long vm_s = std::stoll(str.substr(vm_size, vm_size_len));
        int vm_alloc_s = str.substr(vm_size + vm_size_len + 2).find(",") 
                         + vm_size + vm_size_len + 4;
        
        int rss_size = str.find(" rss size :") + 12;
        long long vm_alloc = std::stoll(str.substr(vm_alloc_s, 
                            rss_size - 12 - vm_alloc_s));
        
        int rss_size_len = str.substr(rss_size).find(",");
        long long rss_s = std::stoll(str.substr(rss_size, rss_size_len));
        int rss_alloc_s = str.substr(rss_size + rss_size_len + 2).find(",") 
                         + rss_size + rss_size_len + 4;
        int rss_alloc_len  = str.substr(rss_alloc_s).find(" ");
        long long rss_alloc = std::stoll(str.substr(rss_alloc_s, rss_alloc_len));
        
         //check if pid in the process list
        auto it = process_list -> find(pid);
        if(it != process_list->end()){ 
            it -> second -> whole_process += str + "\n";           
            it -> second -> vm_use += vm_alloc;
            it -> second -> rss_use += rss_alloc; 
        }
       
        int query_start = str.substr(rss_size).find("query");
        std::string query = "";
        if(query_start!=-1){
            query_start += rss_size;
            query = str.substr(query_start);            
        }
        #ifdef __DEBUG_DETAILS__ 
            printf("--->PROCESSOR: [%s]\n", str.substr(p_end + 4, processor_len).c_str());
            printf("--->vm size: [%lld]\n", vm_s); 
            printf("--->query: [%s]\n", query.c_str());
            printf("--->rss size: [%lld]\n", rss_s); 
        #endif 

        
        if(it != process_list->end()){
            if(progress == 34){
                long long mem_change_vm = vm_s - it -> second -> vm_size_s;
                long long mem_change_rss = rss_s - it -> second -> rss_size_s;
                printf("Process [%d]: %s \nStarted from [%02d:%02d:%f]\n", 
                        pid, query.c_str(),
                        it -> second -> hour, 
                        it -> second -> min, 
                        it ->second ->sec);
                printf("Finished at [%s]\nMemory changed vm: %lld, rss: %lld \n",
                        time_str.c_str(), 
                        mem_change_vm, 
                        mem_change_rss);                
                float time_lapse = calc_time(h, m, s) 
                                 - calc_time(it -> second -> hour, 
                                             it -> second -> min,
                                             it -> second -> sec);
                time_lapse = time_lapse < 0 ? 86400. + time_lapse : time_lapse; 
                printf("Time used %f \n\n" , time_lapse);
                it -> second -> log_proc(time_lapse); 
                delete it->second;
                process_list -> erase(it);                    
            }
            if(progress == 1){
                it -> second -> log_proc(-1.); 
                delete it->second;
                process_list -> erase(it);   
            }        
        }
        
        if(it == process_list -> end()||progress == 1){
            proc* p = new proc(progress, h, 
                                m, s,
                                vm_s,
                                rss_s,
                                pid,query);
            p -> whole_process = str + "\n";
            p -> vm_use = vm_alloc;
            p -> rss_use = rss_alloc;
            process_list -> insert(std::make_pair(pid, p));  
        } 
                       
    }
    else{
        int idx_file = str.find(".cpp");
        
        if(idx_file!= -1){
            
            int idx_time = str.find(" ");
            int pid_len = str.substr(idx_time + 17).find(" ");
            int pid = -1;
            try{
                pid = std::stoi(str.substr(idx_time + 17, pid_len));
            }
            catch(std::invalid_argument&){
                return -1;
            }
            auto it = process_list->find(pid);
            if(it != process_list->end()){
                it->second->whole_process += str + "\n";
            }
        }
    }
    //not a log line
    return -1;
}

int main(int argc, char* argv[]){
    if(argc < 2){
        printf("%s", helper);
        return 0;
    }    
    if(argv[1][0] == '-' && argv[1][1]=='h'){
        printf("%s", helper);
        return 0;
    }

    
    std::ifstream file(argv[1]);
    std::string str;
    int id = 2;
    while(id < argc){
        if (argv[id][0] =='-'){
            if(argv[id][1] == 't'){
                if(id + 1 < argc){
                    THRESHOLD_TIME = std::stof(argv[id+1]);
                    id += 2;
                }
                else{
                    printf("%s", helper);
                    return 0;
                }
            }
            else if(argv[id][1] == 'm'){
                if(id + 1 < argc){
                    THRESHOLD = std::stof(argv[id+1]);
                    id += 2;
                }
                else{
                    printf("%s", helper);
                    return 0;
                }
            }
        }
        else{
            printf("%s", helper);
                return 0;
        }
    }
    
    printf ("Start evaluating [%s]\n", argv[1]);
    printf ("Memory threshold %lld...\n", THRESHOLD);
    printf ("Time threshold %f...\n", THRESHOLD_TIME);
    printf ("Saving to warning.log\n");

    if (system(NULL)) puts ("Ok");
        else exit (EXIT_FAILURE);

    struct stat info;
    if(stat( "warning.log", &info) == 0){
		#ifdef _WIN32
			system("del warning.log");

		#elif __linux__
			system("rm warning.log");
		#endif
        
    }
    if(stat( "detail_logs", &info) != 0){
        system("mkdir detail_logs");
    }
    else if(!(info.st_mode & S_IFDIR)){
        system("mkdir detail_logs");
    }
    else{
	#ifdef _WIN32
        system("del detail_logs");
	
	#elif __linux__
		system("rm detail_logs/*");
	#endif
    }
    
    freopen("process_list.log", "w" ,stdout);
       
    std::unordered_map<int, proc*> process_list;
    while(std::getline(file, str)){
        process_log(str, &process_list);        
    }
    
}


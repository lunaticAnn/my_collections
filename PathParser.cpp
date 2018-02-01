/*==========================PathParser===========================*
 * To build:
 * $ g++ --std=c++11 PathParser.cpp -o PathParser
 * To use:                              
 * $ ./PathParser [Root] [-t file_suffix] [-v verbose]
 * i.e.
 * $ ./PathParser target -t cpp h -v
 * this will try to parse all the .cpp and .h file under target 
 * folder in the verbose mode.   
=================================================================*/

#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <vector>
#include <fstream>
#include <iostream>
#include <string>
#include <cstring>
#include <limits.h>
#include <stdlib.h>
#include <unordered_set>


const std::string helper =
"usage: PathParser [Root] [-t file_suffix] [-v verbose]";

std::string KEYWORD = "#include";
std::string ROOTPATH = "target";
bool VERBOSE = false;


int isDir(std::string path) {
   struct stat statbuf;
   if (stat(path.c_str(), &statbuf) != 0)
       return 0;
   return S_ISDIR(statbuf.st_mode);
}

/*=====================readDirectory=================
 * Read directory stream with entry root.           * 
====================================================*/
int readDirectory(std::string root, 
                   std::vector<std::string>* files,
                   std::vector<std::string>* dirs){
    if(!isDir(root)){
        if(VERBOSE)
            std::cout<<"root is not a directory."<<std::endl;
        return -1;    
    }
    DIR* dirp = opendir(root.c_str());    
    struct dirent * dp;
    //add slash
    std::string path = *(root.rbegin())=='/'? root : root + '/';
    while ((dp = readdir(dirp)) != NULL) {
        //ignore '.' and '..'
        if(strcmp(dp->d_name, ".") == 0||
           strcmp(dp->d_name, "..") == 0)
           continue; 
        std::string filepath = path + dp->d_name;
        if(isDir(filepath)){
            dirs->push_back(filepath);
        }
        else{
            files->push_back(filepath);
        }   
    }
    closedir(dirp);
    return 0;
}

/*=====================listFiles=====================
 * Recursively list all files in the directory.     * 
 * root - root directory to start with              * 
 * files- vector of filepaths                       * 
====================================================*/
int listFiles(std::string root, std::vector<std::string> * files){
    std::vector<std::string> dirs;
    dirs.push_back(root);
    while(dirs.size() > 0){
        readDirectory(*(dirs.begin()), files, &dirs);
        dirs.erase(dirs.begin());
    }
    return 0;
}

/*=======================================abs2Rel========================================*
 * modify the header files to convert the absolute path to                              *
 * relative path.                                                                       *
 * - input_string: header file path                                                     *
 *    i.e "OtherStuff1/A/B/C/D.h"                                                       *
 * - current_path: current file path,                                                   *
 *   i.e "OtherStuff2/A/B/E/F.cpp"                                                      * 
 * - root_path: shared parent folder, i.e "A"                                           *
 *                                                                                      *
 * header_path and file_path will get relative path of root:                            *
 *   OtherStuff1/A/B/C/D.h                                                              *
 * -> B/C/D.h                                                                           *
 *                                                                                      *
 *   OtherStuff2/A/B/E/F.cpp                                                            *
 * -> B/E/F.cpp                                                                         *
 *                                                                                      *
 * In case they have different precision.                                               *
========================================================================================*/
std::string abs2Rel(std::string input_string, 
                    std::string current_path,
                    std::string root_path){
    int header_path_idx = input_string.find(root_path) + root_path.size();
    int header_path_len = input_string.substr(header_path_idx).find("\"");
    std::string header_path = input_string.substr(header_path_idx, header_path_len);   
    
    int file_path_idx = current_path.find(root_path) + root_path.size();
    std::string file_path = current_path.substr(file_path_idx);
    
    int p = 0, q = 0;
    while(file_path[p] == header_path[q]){
        ++p;
        ++q;
    }
    
    std::string res = "";
    while(p < file_path.size()){
        if(file_path[p] == '/'){
            res += "../";
        }
        ++p;
    }
    while(q > 1){
        if(header_path[q-1] == '/'){
            break;
        }
        --q;
    }    
    return res + header_path.substr(q); 
    
}

int modifyFile(std::string current_path, std::string root_path){
    std::ifstream in_file(current_path);
    std::vector<std::string> file_content;
    int ret = 0;
    for (std::string line; std::getline(in_file, line); ){
        if(line.find(KEYWORD) != std::string::npos 
        && line.find(root_path) != std::string::npos){
            int header_idx = line.find("\"") + 1;
            int header_len = line.substr(header_idx).find("\"");
            ++ret;
            if(VERBOSE)
                std::cout<<"Change[" << ret <<"]" 
                         << std::endl <<"  "
                         << line << std::endl;
            line = KEYWORD + " \"" 
                 + abs2Rel(line.substr(header_idx, header_len), 
                           current_path, root_path)
                 + "\"";
            if(VERBOSE)
                std::cout<<"->"<<line<<std::endl;
        }
        file_content.push_back(line);        
    }
    
    in_file.close();
    
    //write modified file 
    std::ofstream out_file(current_path);
    for(size_t i = 0; i < file_content.size(); ++i){
        out_file << file_content[i] << std::endl;    
    }
    out_file.close();
    return ret;
}


int main(int argc, char* argv[]){
    if(argc < 2 || strcmp(argv[1],"-h")==0){
        std::cout<<helper<<std::endl;
        return 0;
    }
    //default suffix
    std::unordered_set<std::string> suffix( {"cpp","h","hpp"} );
    
    //have other configurations
    if(argc > 2){
        int argv_i = 2;
        int config_type = 0;
        while(argv_i < argc){
            if(argv[argv_i][0]=='-'){
                if(argv[argv_i][1]=='v'){
                    VERBOSE = true;
                }
                else if(argv[argv_i][1]=='t'){
                    config_type = 1;
                    suffix.clear();
                }
                
            }
            else if(config_type == 1){
                suffix.insert(argv[argv_i]);   
            }
            ++argv_i;
        }       
    }
    
     
    std::vector<std::string> fs;
    listFiles(argv[1], &fs);
    for(auto p: fs){
        std::string su = p.substr(p.find_last_of('.') + 1);
        if(suffix.find(su)!=suffix.end()){                    
            int line_change = modifyFile(p, ROOTPATH);
            if(VERBOSE && line_change > 0)
                std::cout<<"FILE >>>" << p << std::endl
                         << line_change << " line changed"
                         << std::endl << std::endl;
        }
    }
    
    return 0; 
   
}

#include <iostream>
#include <string>
#include <stdio.h>
#include <sstream>
#include <algorithm>
#include <iterator>
#include <vector>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>
#include <sys/wait.h>
using namespace std;

#define WHITESPACE " \n\r\t\f\v"
#define pb push_back

string remove_space(std::string& s)  //removes whitespace from the ends of a string
{
    int start_index=-1,end_index=-1;
    start_index = s.find_first_not_of(WHITESPACE);
    string temp;
    if(start_index!=-1) s= s.substr(start_index,s.size());

    end_index = s.find_last_not_of(WHITESPACE);
    if(end_index!=-1) s=s.substr(0, end_index + 1);
    return s;
    
}



// split string by the delimiter
vector<string> split_by_delim(string s,char delim){
    vector<string> res;
    string word;
    int i,j;
    for(i=0;i<s.length();i++){
        if(s[i]==delim){
            if(!word.empty()){
                res.push_back(word);
                word="";
            }
        }
        else{
            word.push_back(s[i]);
        }
    }
    if(!word.empty()){
        res.push_back(word);
    }
    return res;
}

// separates the input from output, and returns a vector such that the last element is the output file
vector<string> sep_input_output(string command){
    vector<string> v;
    string str1,str2;
    int input_flg=0,output_flg=0;
    for(auto x:command){
        if(x=='>') output_flg=1;
        if(x=='<') input_flg=1;
    }
    if(!input_flg && !output_flg) {v.pb(remove_space(command)); v.pb(""); v.pb(""); return v;}  
    if(input_flg && !output_flg ){ // a<b
        for(int i=0;i<command.size();i++){
            if(command[i]=='<'){
                str1=command.substr(0,i);
                str2=command.substr(i+1);
                v.pb(remove_space(str1));  v.pb(remove_space(str2)); // push a then b
                v.pb("");
                return v;
            }
        }
    }
    if(!input_flg && output_flg ){ //a>b
        for(int i=0;i<command.size();i++){
            if(command[i]=='>'){
                str1=command.substr(0,i);
                str2=command.substr(i+1);
                v.pb(remove_space(str1));  v.pb(""); v.pb(remove_space(str2));  // push a then b
                return v;
            }
        }
    }
    if(input_flg && output_flg){
        int in,out;
        in=out=0;
        string temp;
        for(int i=0;i<command.size();i++){
            if(command[i]=='<'){
                if(!out){ //a<b>c
                    str1=command.substr(0,i);
                    v.pb(remove_space(str1)); // pushes a
                    temp=command.substr(i+1);
                    in=1;
                }
                else{ //a>b<c
                    str1=command.substr(i+1);
                    v.pb(remove_space(str1)); //pushes c
                    str2=temp.substr(0,temp.size()-v[1].size()-1);
                    v.pb(remove_space(str2)); //pushes b
                }
            }
            if(command[i]=='>'){
                if(!in){ //a>b<c
                    str1=command.substr(0,i);
                    v.pb(remove_space(str1));  //pushes a
                    temp=command.substr(i+1);
                    out=1;
                }
                else{ //a<b>c
                    str1=temp.substr(0,temp.size()-(command.size()-i));
                    str2=command.substr(i+1);
                    v.pb(remove_space(str1)); // pushes b 
                    v.pb(remove_space(str2)); // pushes c
                }
            }
        }
       
    }
     return v;
} 


void ioredirect(string input_filename, string output_filename)
{
    int input_file_descriptor, output_file_descriptor;
    //input_filename and output_filename will be empty strings if no input/output redirector is present in the input command
    if(input_filename.size()>0)
    {
        input_file_descriptor = open(input_filename.c_str(),O_RDONLY, O_CREAT,O_EXCL);  // Opens input file in read only mode and returns file descriptor poinitng to the file table entry of input_filename
        //file will be created if it does not exist and a new creation will be prevented if it already exists
        if(input_file_descriptor<0 ) {cout<<"Error encountered while opening file: "<<input_filename<<"\n"; exit(EXIT_FAILURE);}
        close(0);//this is not required with dup2. dup2 performs close and assignment of fd atomically. UNLIKE dup which performs only assignemnt, so close is needed with dup
        int dp=dup2(input_file_descriptor,0); //File descriptor of STD_IN now points to input_filename
        if (dp<0) { cout<<"Input Redirection Error"<<"\n";  exit(EXIT_FAILURE); }

    }

    if(output_filename.size())
    {
        output_file_descriptor = open(output_filename.c_str(), O_CREAT|O_WRONLY|O_TRUNC, 0644);  // Opens output file in create and truncate mode
        if(output_file_descriptor<0 ) {cout<<"Error encountered while opening file: "<<output_filename<<"\n"; exit(EXIT_FAILURE); }
        close(1);
        int dp=dup2(output_file_descriptor,1); //File descriptor of STD_OUT now points to output_filename
        if (dp<0) { cout<<"Output Redirection Error"<<"\n";  exit(EXIT_FAILURE); }   
    }
}


void execute_command(string command)  // Execute the given command
{

    vector<string> args;
    for(string str : split_by_delim(command,' '))
        if(str.size()) // Ignore whitespaces
            args.pb(str);

    // Create a char* array for the arguments
    int n=args.size()+1;
    char** argv=(char**)(malloc(n*sizeof(char*)));
    for(int i=0 ; i<args.size() ; i++)
        argv[i] = const_cast<char*>(args[i].c_str()); // const_cast changes type of const(as returned by c_str) to non-const
    argv[args.size()] = NULL; // Terminate argv with NULL pointer

    char* const* exec_argv = argv; // Store argv in a constant array (syntax of execvp function demands this)
    execvp(args[0].c_str(),exec_argv); //replaces the current process with args[0]
}

int main()
{
    char s[100];
    string command;

    while(1)
    {
        cout<<"SHELL> ";
        getline(cin, command,'\n'); // fetches entire line

        int npipes=0; // stores no. of pipes in command string
        int bg_flag = 0; // background flag is 1 if command has '&' in it
        
        vector<string> temp=split_by_delim(command,' ');
        if(temp[0]=="cd") {    
            if(chdir(temp[1].c_str())){
                cout<<"Error encountered while changing directory";
            } 
            continue;
        }
        command = remove_space(command);
        if(command.size()==0) continue;
        for(string::iterator itr=command.begin(); itr!=command.end(); itr++){
            if(*itr=='|') npipes++;
            else if(*itr=='&') bg_flag=1;
        }
        
        if(bg_flag) command.pop_back();

        vector<string> command_vec=split_by_delim(command,'|');
        // If no pipes are required
        if(!npipes)   //no pipes
        {
            vector< string > separated = sep_input_output(command_vec[0]);
            
            pid_t pid = fork();
            if(pid == 0) // for the child process
            {
                ioredirect(separated[1],separated[2]); // Input and output redirection, does nothing if no input redirectors present
                execute_command(separated[0]); // Execute the command
                exit(0); 
            }
        }

        else
        {
//no. of commands=npipes+1

            int curr_fd[2]; //will store read and write file descriptors of pipe of current iteration
            int prev_fd[2]; //will store read and write file descriptors of pipe of previous iteration
            int i=0;
            while(i<=npipes)
            {
                vector<string> separated = sep_input_output(command_vec[i]);
                 // New pipe is called for all commands except the last one. 
                //This pipe is written onto by the current command's child process, 
                //while read by the previous command's child process(which is the parent of the current command's child process)
                if(i!=npipes)  pipe(curr_fd);                
                
                
                pid_t pid = fork();          // Call Fork for every command in the command_vec

                // child process executes the command
                if(pid == 0)
                {
                    if( i==0 || i==npipes)
                        ioredirect(separated[1], separated[2]);  // For the first and last command redirect the input output files
                    //separated[1] and separated[2] are strings containing input and output file names respectvely.

                    // Read from pipe of previous command (except the first command)
                    if(i)
                        { 
                            close(0); dup(prev_fd[0]); 
                            //close the read and write ends of previous pipe file descriptors
                            close(prev_fd[0]); close(prev_fd[1]);
                        }

                    // Write into pipe for everything except last command
                    if(i!=npipes){
                        //close file descriptor of STD_OUT and set it to point to the files pointed to by the curr_fd[1] (i.e., write end of current pipe)
                        close(1); dup(curr_fd[1]);
                        //close the read and write file descriptors of current pipe
                        close(curr_fd[1]);
                        close(curr_fd[0]); 
                    }

                    execute_command(separated[0]);
                    exit(0); //Added this exit cuz I think it's required. Check once.
                }

             
                //close the read and write ends of previous pipe file descriptors for parent process
                if(i>0)  {close(prev_fd[0]); close(prev_fd[1]);}
                
                // Copy curr_fd into prev_fd for everything except the last command
                if(i!=npipes)  {prev_fd[0] = curr_fd[0]; prev_fd[1] = curr_fd[1];}
                i++;
            }
        }
        // Waits till completion of all child processes in case of no background process
            if(!bg_flag) while( wait(NULL) > 0);
    }
}
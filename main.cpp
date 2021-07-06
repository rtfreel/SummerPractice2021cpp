#include <iostream>
#include <string>
#include <vector>
#include <thread>
#include <fstream>
#include <algorithm>
#include <chrono>
#include <pthread.h>

#include <sys/mman.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sstream>

const size_t NUMBER_OF_THREADS = 4;

using namespace std;

// don't think ternary operator helps, but it looks better for me
string process_line(string in_line){
    for(size_t i=0; i<in_line.size(); i++){
        if (isdigit(in_line[i]))
            in_line[i] = in_line[i] < 9 ? in_line[i] + 1 : '0';
        else 
            in_line[i] = isupper(in_line[i]) ? tolower(in_line[i]) : toupper(in_line[i]);
    }
    return in_line;
}

// data for one thread is now in a single chunk
void process_thread(vector<string>& lines, size_t offset){
    size_t end = lines.size(),
        chunk = end / NUMBER_OF_THREADS, 
        cur = offset * chunk;
    if(offset < NUMBER_OF_THREADS) 
        end = cur + chunk;

    for(; cur < end; cur++){
        lines[cur] = process_line(lines[cur]);
    }
}

int main() {
    chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
    chrono::steady_clock::time_point data_load;
    try{
        cout<<"Load input data"<<endl;
        ifstream in_file("../input.data");

        if(!in_file.is_open()){
            cout<<"Fail to open file!"<<endl;
            return 0;
        }

        vector<string> lines;
        string buffer;
        while(in_file >> buffer){
            lines.push_back(buffer+"\n");
        }
        in_file.close();

        // int fd = open("../input.data", O_RDONLY, 0);
        // struct stat st;
        // fstat(fd, &st);
        // unsigned char* data = (unsigned char*)mmap(nullptr, st.st_size,
        //     PROT_READ,
        //     MAP_PRIVATE,
        //     fd, 0);
        // vector<string> lines;
        // string buffer;
        // while(stream >> buffer){
        //     lines.push_back(buffer+"\n");
        // }
        // munmap(carr, st.st_size);
        // close(fd);

        data_load = std::chrono::steady_clock::now();

        cout << "Start " << NUMBER_OF_THREADS << " threads" << endl;
        vector<thread> thread_pool;
        for(size_t i=0; i<NUMBER_OF_THREADS; i++){
            // thread *th = new thread(process_thread, i);
            // thread_pool.push_back(th);
            thread_pool.emplace_back(process_thread, ref(lines), i);
        }

        cout<<"Wait for finish"<<endl;
        for(auto& th : thread_pool){
            th.join();
        }

        cout<<"Save output data"<<endl;
        ofstream out_file("../output_cpp.data");
        for(auto &line : lines){
            out_file.write(line.c_str(), line.size());
        }
        out_file.close();

        cout<<"Done"<<endl;
    }catch (const exception& e){
        cout<<e.what();
    }

    chrono::steady_clock::time_point end = std::chrono::steady_clock::now();

    std::cout << "data load time: " << std::chrono::duration_cast<std::chrono::milliseconds>(data_load - begin).count() << "ms" << std::endl;
    std::cout << "total execution time: " << std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count() << "ms" << std::endl;
    return 0;
}

/* First try:
    Load input data
    Start 4 threads
    Wait for finish
    Save output data
    Done
    data load time: 821ms
    total execution time: 6631ms
 * Result:
    Load input data
    Start 4 threads
    Wait for finish
    Save output data
    Done
    data load time: 848ms
    total execution time: 6161ms
    ):
 * CMake update:
    https://stackoverflow.com/questions/1620918/cmake-and-libpthread 
 * mmap():
    https://stackoverflow.com/questions/45972/mmap-vs-reading-blocks
 * push_back() vs. emplace_back():
    https://stackoverflow.com/questions/4303513/push-back-vs-emplace-back
    https://www.geeksforgeeks.org/push_back-vs-emplace_back-in-cpp-stl-vectors/
 * 
 * 
 */
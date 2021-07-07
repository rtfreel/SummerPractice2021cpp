#include <iostream>
#include <thread>
#include <fstream>
#include <chrono>

#include <sys/mman.h>
#include <fcntl.h>
#include <sys/stat.h>

const size_t NUMBER_OF_THREADS = 4;

using namespace std;

// process_thread now modifies every character
// one by one in memory
// data for one thread is now in a single chunk
void process_thread(char* begin, size_t total, size_t offset){
    size_t chunk = total / NUMBER_OF_THREADS;
    char *end = begin + total,
        *cur = begin + (offset * chunk);
    if(offset < NUMBER_OF_THREADS) 
        end = cur + chunk;

    //process every character one by one
    for(; cur < end; cur++){
        //got rid of if statements (not sure if it helps)
        *cur = isdigit(*cur) ?
            (*cur < '0' + 9 ? *cur + 1      : '0') :
            (isupper(*cur)  ? tolower(*cur) : toupper(*cur));
    }
}

int main() {
    chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
    chrono::steady_clock::time_point data_load;
    try{
        cout<<"Load input data"<<endl;

        //got rid of vector of lines and mapped file into memory
        int fd = open("../input.data", O_RDONLY, 0);
        struct stat st;
        fstat(fd, &st);
        size_t f_size = st.st_size;
        char* data = (char*)mmap(nullptr, f_size,
            PROT_READ|PROT_WRITE,
            MAP_PRIVATE,
            fd, 0);

        data_load = std::chrono::steady_clock::now();

        cout << "Start " << NUMBER_OF_THREADS << " threads" << endl;
        //used dynamic array instead of vector
        thread *thread_pool = new thread[NUMBER_OF_THREADS];
        for(size_t i=0; i<NUMBER_OF_THREADS; i++){
            thread_pool[i] = thread(process_thread, data, f_size, i);
        }

        cout<<"Wait for finish"<<endl;
        for(size_t i=0; i<NUMBER_OF_THREADS; i++){
            thread_pool[i].join();
        }

        //write to file directly from memory
        cout<<"Save output data"<<endl;
        ofstream out_file("../output_cpp.data");
        out_file.write(data, f_size);
        out_file.close();

        //unmapping file
        munmap(data, f_size);
        delete [] thread_pool;

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
    data load time: 0ms
    total execution time: 2143ms
    (:
 * Result (8 threads):
    Load input data
    Start 8 threads
    Wait for finish
    Save output data
    Done
    data load time: 0ms
    total execution time: 1717ms
 */
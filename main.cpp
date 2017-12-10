#include <vector>
#include <iostream>
#include <map>
#include <time.h>
#include <signal.h>
#include <wait.h>
#include <exception>
#include <unistd.h>

#include <thread>
#include <pthread.h>
#include <stdio.h>
#include <semaphore.h>
#include <mutex>
#include <cstdlib>
#include <sys/sem.h>
#include <sys/ipc.h>


#define N  250

bool run = true;
int writing = 0;
int reading = 0;
int wantWrite = 0;

void sigint_handler(int signum) {
    std::cerr << "Pressed C" << signum << std::endl;
    run = false;
}

void sigstop_handler(int signum) {
    std::cerr << "Pressed Z: " << signum << std::endl;
    run = false;
}

int size, n_readers, n_writers;
std::vector<int> v_buffer;
std::vector<std::thread> v_threads;
std::map<std::thread::id, unsigned int > m_threads; //<0 = writer , >0reader


pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
sem_t readSem = sem_t();
sem_t writeSem = sem_t();
pthread_cond_t cond;
pthread_t thre[6];


void reader_func();
void writer_func();

int main(int argc, char** argv) {

    signal(SIGINT, sigint_handler);
    signal(SIGSTOP, sigstop_handler);

    srand((unsigned) time(NULL));

     
    if (argc == 4) {
        size = std::stoi(argv[1]);
        n_readers = std::stoi(argv[2]);
        n_writers = std::stoi(argv[3]);
    } else {
        std::cout << "Größen Festlegen:" << std::endl;
        std::cout << "\n1.BufferSize:";
        std::cin>>size;
        std::cout << "2.ReaderAmount:";
        std::cin >>n_readers;
        std::cout << "3.WriterAmount:";
        std::cin>>n_writers;
        std::cout << std::endl;
    }
    try {
        sem_post(&writeSem);

        //Fill up vector
        for (int i = 0; i < size; i++) {
            v_buffer.push_back(rand() % 1000);
            sem_post(&readSem);
        }

        //Create  Threads
        std::thread thisr;
        std::thread thisw;

        for (int i = 1000; i < n_writers + 1000; i++) {
            v_threads.push_back(std::thread(writer_func));
        }

        for (int i = n_writers; i < (n_writers + n_readers); i++) {
            v_threads.push_back(std::thread(reader_func));
        }

        //Bring threads together
        for (int i = 0; i < v_threads.size() + 1; i++) {
            v_threads[i].join();
        }

        sem_destroy(&readSem);


    } catch (std::exception ex) {
        std::cerr << "CATCHED >>" << ex.what() << std::endl;
    } catch (...) {
        std::cerr << "CATCHED SOMETHING" << std::endl;
    }

    return 0;
}

void reader_func() {

    while (run) {

        sleep(1);

        int random = rand() % size;

        sem_wait(&writeSem);
        sem_wait(&readSem);
        sem_post(&writeSem);

        std::cout << "_READER:" << std::this_thread::get_id() << " - Value: "
                << v_buffer[random] << " Index: " << random << std::endl;

        sem_post(&readSem);
    }
}

void writer_func() {

    while (run) {

        sleep(1);
        sem_wait(&writeSem);

        for (int i = 0; i < size; i++)
            sem_wait(&readSem);

        for (int i = 0; i < size; i++)
            v_buffer[i] = rand() % 100;
        std::cout << "WRITER WROTE" << std::endl;

        for (int i = 0; i < size; i++)
            sem_post(&readSem);

        sem_post(&writeSem);


    }
}
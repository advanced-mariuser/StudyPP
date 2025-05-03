#include <semaphore.h>
#include <thread>
#include <iostream>
#include <unistd.h>

using namespace std;

int main()
{
    sem_t semaphore1;
    sem_init(&semaphore1, 0, 0);

    auto thread1 = thread([&]
                          {
                              sem_wait(&semaphore1);
                              printf("thread1 work!");
                          });
    auto thread2 = thread([&]
                          {
                              sem_wait(&semaphore1);
                              printf("thread2 work!");
                          });

    sleep(5);
    cout << "Main thread" << endl;

    sem_post(&semaphore1);
    sem_post(&semaphore1);

    thread1.join();
    thread2.join();

    return EXIT_SUCCESS;
}
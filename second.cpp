#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <semaphore.h>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>
#include <stdexcept>
#include <iostream>

class SharedArray
{
public:
    SharedArray(const char *array_name, int size) : size_(size)
    {
        fd_ = shm_open(array_name, O_CREAT | O_RDWR, 0644);

        if (fd_ == -1)
        {
            throw std::runtime_error("Could not create shared memory region.");
        }

        if (ftruncate(fd_, size_ * sizeof(int)) == -1)
        {
            throw std::runtime_error("Could not set the size of the shared memory region.");
        }

        shared_array_ = (int *)mmap(NULL, size_ * sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED, fd_, 0);

        if (shared_array_ == MAP_FAILED)
        {
            throw std::runtime_error("Could not map the shared memory region to the process's address space.");
        }

        memset(shared_array_, 0, size_ * sizeof(int));

        semaphore_ = sem_open("semaphore", O_CREAT | O_RDWR, 0666, 1);

        if (semaphore_ == SEM_FAILED)
        {
            throw std::runtime_error("Could not create semaphore.");
        }
    }

    ~SharedArray()
    {
        sem_close(semaphore_);
        munmap(shared_array_, size_ * sizeof(int));
        close(fd_);
    }

    int size() const
    {
        return size_;
    }

    const int operator[](const std::size_t i) const
    {
        if (i >= size())
        {
            throw std::out_of_range("out of range\n");
        }
        sem_wait(semaphore_);
        int value = shared_array_[i];
        sem_post(semaphore_);
        return value;
    }

    int &operator[](const std::size_t i)
    {
        if (i >= size())
        {
            throw std::out_of_range("out of range\n");
        }
        sem_wait(semaphore_);
        int &value = shared_array_[i];
        sem_post(semaphore_);
        return value;
    }

    sem_t *semaphore_;

private:
    int *shared_array_;
    int size_;
    int fd_;
};

int main()
{
    SharedArray array("array", 64);

    while (true)
    {
        for (int i = 0; i < array.size(); ++i)
        {
            std::cout << array[i] << " ";
        }
        std::cout << std::endl;
        sleep(1);
    }

    return 0;
}

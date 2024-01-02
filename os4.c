
#include <stdio.h>
#include <dirent.h>
#include <sys/stat.h>
#include <string.h>
#include <stdint.h>
#include <limits.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <semaphore.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>

#define TXT_COUNT_SHM_NAME "/txt_count_shm"
#define PNG_COUNT_SHM_NAME "/png_count_shm"
#define JPG_COUNT_SHM_NAME "/jpg_count_shm"
#define OTHER_COUNT_SHM_NAME "/other_count_shm"
#define FILE_COUNT_SHM_NAME "/file_count_shm"
typedef struct {
    char directoryPath[100];
    int *txtCount;
    int *pngCount;
    int *jpgCount;
    int *otherCount;
    int *fileCount;
} ThreadArgs;

void calculateRootFolderSize(const char *directoryPath, uintmax_t *totalSize) {
    DIR *dir = opendir(directoryPath);
    if (dir == NULL) {
        printf("Failed to open directory.\n");
        return;
    }
    struct dirent *entry;
    struct stat fileStat;
    while ((entry = readdir(dir)) != NULL) {
        // Get the file path
        char filePath[150];
        strcpy(filePath, directoryPath);
        strcat(filePath, "/");
        strcat(filePath, entry->d_name);
        if (stat(filePath, &fileStat) < 0) {
            printf("Failed to get file information for %s\n", entry->d_name);
            continue;
        }
        if (S_ISREG(fileStat.st_mode)) {
            *totalSize += fileStat.st_size;
        }
        if (S_ISDIR(fileStat.st_mode) && strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
            calculateRootFolderSize(filePath, totalSize);
        }
    }
    closedir(dir);
}



void findLargestAndSmallestFileSize(const char *directoryPath) {
    DIR *dir = opendir(directoryPath);
    if (dir == NULL) {
        printf("Failed to open directory.\n");
        return;
    }
    struct dirent *entry;
    struct stat fileStat;
    long long int largestSize = 0;
    long long int smallestSize = LLONG_MAX;
    char largestFilePath[150];
    char smallestFilePath[150];
    while ((entry = readdir(dir)) != NULL) {
        char filePath[150];
        strcpy(filePath, directoryPath);
        strcat(filePath, "/");
        strcat(filePath, entry->d_name);
        if (stat(filePath, &fileStat) < 0) {
            printf("Failed to get file information for %s\n", entry->d_name);
            continue;
        }
        if (S_ISREG(fileStat.st_mode)) {
            if (fileStat.st_size > largestSize) {
                largestSize = fileStat.st_size;
                strcpy(largestFilePath, filePath);
            }
            if (fileStat.st_size < smallestSize) {
                smallestSize = fileStat.st_size;
                strcpy(smallestFilePath, filePath);
            }
        }
    }
    printf("Largest file size: %lld bytes\n", largestSize);
    printf("Largest file path: %s\n", largestFilePath);
    printf("Smallest file size: %lld bytes\n", smallestSize);
    printf("Smallest file path: %s\n", smallestFilePath);
    closedir(dir);
}
void sharedLogic( const char directoryPath[100], int* txtCount, int* pngCount, int* jpgCount, int* otherCount, int* fileCount) {
    struct dirent* entry;
    struct stat fileStat;
    DIR *dir = opendir(directoryPath);
    printf("Directory path: %s\n", directoryPath);
    if (dir == NULL) {
        printf("Failed to open directory.\n");
        return;
    }
    int* txtCountShm;
int* pngCountShm;
int* jpgCountShm;
int* otherCountShm;
int* fileCountShm;

// Create or open the shared memory objects
int txtCountShmFd = shm_open(TXT_COUNT_SHM_NAME, O_CREAT | O_RDWR, 0666);
int pngCountShmFd = shm_open(PNG_COUNT_SHM_NAME, O_CREAT | O_RDWR, 0666);
int jpgCountShmFd = shm_open(JPG_COUNT_SHM_NAME, O_CREAT | O_RDWR, 0666);
int otherCountShmFd = shm_open(OTHER_COUNT_SHM_NAME, O_CREAT | O_RDWR, 0666);
int fileCountShmFd = shm_open(FILE_COUNT_SHM_NAME, O_CREAT | O_RDWR, 0666);

// Set the size of the shared memory objects
size_t countSize = sizeof(int);
ftruncate(txtCountShmFd, countSize);
ftruncate(pngCountShmFd, countSize);
ftruncate(jpgCountShmFd, countSize);
ftruncate(otherCountShmFd, countSize);
ftruncate(fileCountShmFd, countSize);

// Map the shared memory objects into the process address space
txtCountShm = (int*)mmap(0, countSize, PROT_READ | PROT_WRITE, MAP_SHARED, txtCountShmFd, 0);
pngCountShm = (int*)mmap(0, countSize, PROT_READ | PROT_WRITE, MAP_SHARED, pngCountShmFd, 0);
jpgCountShm = (int*)mmap(0, countSize, PROT_READ | PROT_WRITE, MAP_SHARED, jpgCountShmFd, 0);
otherCountShm = (int*)mmap(0, countSize, PROT_READ | PROT_WRITE, MAP_SHARED, otherCountShmFd, 0);
fileCountShm = (int*)mmap(0, countSize, PROT_READ | PROT_WRITE, MAP_SHARED, fileCountShmFd, 0);

// Update the counts using shared memory
(*txtCountShm) = 0;
(*pngCountShm) = 0;
(*jpgCountShm) = 0;
(*otherCountShm) = 0;
(*fileCountShm) = 0;
     while ((entry = readdir(dir)) != NULL) {
        // Get the file path
        char filePath[150];
        strcpy(filePath, directoryPath);
        strcat(filePath, "/");
        strcat(filePath, entry->d_name);
        
        // Get the file type
        if (stat(filePath, &fileStat) < 0) {
            printf("Failed to get file information for %s\n", entry->d_name);
            continue;
        }
        if (S_ISREG(fileStat.st_mode)) {
            char* extension = strrchr(entry->d_name, '.');
            if (strcmp(extension, ".txt") == 0) {
    (*txtCountShm)++;
} else if (strcmp(extension, ".png") == 0) {
    (*pngCountShm)++;
} else if (strcmp(extension, ".jpg") == 0 || strcmp(extension, ".jpeg") == 0) {
    (*jpgCountShm)++;
} else {
    (*otherCountShm)++;
}

(*fileCountShm)++;
        }
    }
   
    printf("Directory path: %s\n", directoryPath);
    
    
}

void* threadFunction(void* arg) {
    char* directoryPath = (char*)arg;
    int txtCount, pngCount, jpgCount, otherCount, fileCount;

    sharedLogic(directoryPath, &txtCount, &pngCount, &jpgCount, &otherCount, &fileCount);
    ThreadArgs args;
    strcpy(args.directoryPath, directoryPath);
    (*args.txtCount) += txtCount;
    (*args.pngCount) += pngCount;
    (*args.jpgCount) += jpgCount;
    (*args.otherCount) += otherCount;
    (*args.fileCount) += fileCount;
    return NULL;
}
int main() {
    char directoryPath[100];
    printf("Please enter the directory path: ");
    scanf("%s", directoryPath);

    DIR *dir = opendir(directoryPath);
    DIR *dir2 = opendir(directoryPath);
    if (dir == NULL) {
        printf("Failed to open directory.\n");
        return 1;
    }
    struct dirent* entry;
    struct stat fileStat;
    ThreadArgs threadArgs;
    strcpy(threadArgs.directoryPath, directoryPath);

    int txtCount, pngCount, jpgCount, otherCount, fileCount;
    sharedLogic(directoryPath, &txtCount, &pngCount, &jpgCount, &otherCount, &fileCount);
    threadArgs.txtCount = &txtCount;
    threadArgs.pngCount = &pngCount;
    threadArgs.jpgCount = &jpgCount;
    threadArgs.otherCount = &otherCount;
    threadArgs.fileCount = &fileCount;

    

    
    uintmax_t totalSize = 0;
    calculateRootFolderSize(directoryPath, &totalSize);
    findLargestAndSmallestFileSize(directoryPath);

    // Create a lock for synchronization
    pthread_mutex_t lock;
    pthread_mutex_init(&lock, NULL);

    // Create a semaphore for synchronization
    sem_t semaphore;
    sem_init(&semaphore, 0, 1); // Initialize semaphore with value 1

    //Create a process for each subdirectory
    pid_t pid;
    
    while ((entry = readdir(dir2)) != NULL) {
        if (entry->d_type == DT_DIR && strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
            char subdirectoryPath[150];
            strcpy(subdirectoryPath, directoryPath);
            strcat(subdirectoryPath, "/");
            strcat(subdirectoryPath, entry->d_name);

            pid = fork();
            if (pid < 0) {
                printf("Failed to create child process for %s\n", entry->d_name);
                continue;
            } else if (pid == 0) {
                // Child process
                DIR *subdir = opendir(subdirectoryPath);
                if (subdir == NULL) {
                    printf("Failed to open subdirectory %s\n", entry->d_name);
                    return 1;
                }
                struct dirent *subentry;
                while ((subentry = readdir(subdir)) != NULL) {
                    if (subentry->d_type == DT_DIR) {
                        // Create a thread for each unthreaded subdirectory
                        char unthreadedSubdirectoryPath[150];
                        strcpy(unthreadedSubdirectoryPath, subdirectoryPath);
                        strcat(unthreadedSubdirectoryPath, "/");
                        strcat(unthreadedSubdirectoryPath, subentry->d_name);

                        pthread_t thread;
                        pthread_create(&thread, NULL, threadFunction, (void*)&unthreadedSubdirectoryPath);
                        pthread_join(thread, NULL); // Wait for the thread to finish
                    }
                }
                closedir(subdir);
                return 0;
            }
        }
    }

    // Wait for all child processes to finish
    while (wait(NULL) > 0);
    printf(".txt count: %d\n", txtCount);
    printf(".png count: %d\n", pngCount);
    printf(".jpg count: %d\n", jpgCount);
    printf("Other file types count: %d\n", otherCount);
    printf("Total number of files: %d\n", fileCount);
    calculateRootFolderSize(directoryPath, &totalSize);
    printf("Total size of the root folder: %ju bytes\n", totalSize);
    // Clean up resources
    pthread_mutex_destroy(&lock);
    sem_destroy(&semaphore);
    closedir(dir);

    return 0;
}

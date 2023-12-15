#include <stdio.h>
#include <dirent.h>
#include <sys/stat.h>
#include <string.h>
#include <stdint.h>
#include <limits.h>

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

        // Get the file information
        if (stat(filePath, &fileStat) < 0) {
            printf("Failed to get file information for %s\n", entry->d_name);
            continue;
        }

        // Check if the file is a regular file and add its size to the total
        if (S_ISREG(fileStat.st_mode)) {
            *totalSize += fileStat.st_size;
        }

        // Check if the file is a directory and recursively calculate its size
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
    char largestFilePath[150];  // Variable to store the path of the largest file
    char smallestFilePath[150];  // Variable to store the path of the smallest file

    while ((entry = readdir(dir)) != NULL) {
        // Get the file path
        char filePath[150];
        strcpy(filePath, directoryPath);
        strcat(filePath, "/");
        strcat(filePath, entry->d_name);

        // Get the file information
        if (stat(filePath, &fileStat) < 0) {
            printf("Failed to get file information for %s\n", entry->d_name);
            continue;
        }

        // Check if the file is a regular file and update the largestSize and smallestSize if applicable
        if (S_ISREG(fileStat.st_mode)) {
            if (fileStat.st_size > largestSize) {
                largestSize = fileStat.st_size;
                strcpy(largestFilePath, filePath); // Update the path of the largest file
            }
            if (fileStat.st_size < smallestSize) {
                smallestSize = fileStat.st_size;
                strcpy(smallestFilePath, filePath); // Update the path of the smallest file
            }
        }
    }

    printf("Largest file size: %lld bytes\n", largestSize);
    printf("Largest file path: %s\n", largestFilePath);
    printf("Smallest file size: %lld bytes\n", smallestSize);
    printf("Smallest file path: %s\n", smallestFilePath);

    closedir(dir);
}

int main() {
    char directoryPath[100]; // Assuming a maximum path length of 100 characters

    printf("Please enter the directory path: ");
    scanf("%s", directoryPath);

    DIR *dir = opendir(directoryPath);
    if (dir == NULL) {
        printf("Failed to open directory.\n");
        return 1;
    }

    struct dirent *entry;
    struct stat fileStat;
    int txtCount = 0, pngCount = 0, jpgCount = 0, otherCount = 0  , fileCount = 0;

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

        // Check the file type and increment the corresponding count
        if (S_ISREG(fileStat.st_mode)) {
            // Extract the file extension
            char *extension = strrchr(entry->d_name, '.');
            if (extension != NULL && strlen(extension) > 1) {
                if (strcmp(extension, ".txt") == 0) {
                    txtCount++;
                } else if (strcmp(extension, ".png") == 0) {
                    pngCount++;
                } else if (strcmp(extension, ".jpg") == 0 || strcmp(extension, ".jpeg") == 0) {
                    jpgCount++;
                } else {
                    otherCount++;
                }
            } else {
                otherCount++;
            }
            fileCount++;
        }
    }

    printf(".txt count: %d\n", txtCount);
    printf(".png count: %d\n", pngCount);
    printf(".jpg count: %d\n", jpgCount);
    printf("Other file types count: %d\n", otherCount);
    printf("Total number of files: %d\n", fileCount);
    uintmax_t totalSize = 0;
    calculateRootFolderSize(directoryPath, &totalSize);
    findLargestAndSmallestFileSize(directoryPath);
    printf("Total size of the root folder: %ju bytes\n", totalSize);
    closedir(dir);
    return 0;
}
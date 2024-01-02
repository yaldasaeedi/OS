printf("Failed to open directory.\n");
            //     char subdirectoryPath[150];
            // strcpy(subdirectoryPath, directoryPath);
            // strcat(subdirectoryPath, "/");
            // strcat(subdirectoryPath, entry->d_name);

            // pid = fork();
            // if (pid < 0) {
            //     printf("Failed to create child process for %s\n", entry->d_name);
            //     continue;
            // } else if (pid == 0) {
            //     // Child process
            //     DIR *subdir = opendir(subdirectoryPath);
            //     if (subdir == NULL) {
            //         printf("Failed to open subdirectory %s\n", entry->d_name);
            //         return 1;
            //     }
            //     struct dirent *subentry;
            //     while ((subentry = readdir(subdir)) != NULL) {
            //         if (subentry->d_type == DT_DIR) {
            //             // Create a thread for each unthreaded subdirectory
            //             char unthreadedSubdirectoryPath[150];
            //             strcpy(unthreadedSubdirectoryPath, subdirectoryPath);
            //             strcat(unthreadedSubdirectoryPath, "/");
            //             strcat(unthreadedSubdirectoryPath, subentry->d_name);

            //             pthread_t thread;
            //             pthread_create(&thread, NULL, threadFunction, (void*)unthreadedSubdirectoryPath);
            //             pthread_join(thread, NULL); // Wait for the thread to finish
            //         }
            //     }
            //     closedir(subdir);
            //     return 0;
            // }
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>

#define MAX_FILES 100
#define BUF_SIZE 1024
#define PIPE_NAME "/tmp/file_manager_named_pipe"

char file_list[MAX_FILES][BUF_SIZE];
pthread_mutex_t file_list_mutex;

void *client_thread(void *arg)
{
    int fd = *((int *)arg);
    char buf[BUF_SIZE];
    int num_bytes;
    while (1)
    {
        num_bytes = read(fd, buf, BUF_SIZE);
        if (num_bytes == 0)
        {
            break;
        }
        buf[num_bytes] = '\0';

        // Parse command
        char command[BUF_SIZE];
        char filename[BUF_SIZE];
        sscanf(buf, "%s %s", command, filename);
        // Manipulate commands
        if (strcmp(command, "create") == 0)
        {
            pthread_mutex_lock(&file_list_mutex);
            int i;
            for (i = 0; i < MAX_FILES; i++)
            {
                if (strlen(file_list[i]) == 0)
                {
                    strcpy(file_list[i], filename);
                    break;
                }
            }
            pthread_mutex_unlock(&file_list_mutex);
            if (i < MAX_FILES)
            {
                int fd = open(filename, O_CREAT | O_WRONLY,0644);
                if (fd < 0)
                {
                    int c = open(PIPE_NAME, O_WRONLY);
                    write(c, "Error creating file\n", strlen("Error creating file\n"));
                    close(c);
                }
                else
                {
                    int c = open(PIPE_NAME, O_WRONLY);
                    write(c, "File created\n", strlen("File created\n"));
                    close(c);
                }
            }
            else
            {
                int c = open(PIPE_NAME, O_WRONLY);
                write(c, "Error: maximum number of files reached\n", strlen("Error: maximum number of files reached\n"));
                close(c);
            }
        } 
         else if (strcmp(command, "delete") == 0)
         {
             pthread_mutex_lock(&file_list_mutex);
             int i;
             for (i = 0; i < MAX_FILES; i++)
             {
                 if (strcmp(file_list[i], filename) == 0)
                 {
                     strcpy(file_list[i], "");
                     break;
                 }
             }
             pthread_mutex_unlock(&file_list_mutex);
             if (i < MAX_FILES)
             {
                 if (unlink(filename) == 0)
                 {
                    int fd = open(PIPE_NAME, O_WRONLY);
                     write(fd, "File deleted\n", strlen("File deleted\n"));
                     close(fd);
                 }
                 else
                 {
                    int fd = open(PIPE_NAME, O_WRONLY);
                     write(fd, "Error deleting file\n", strlen("Error deleting file\n"));
                     close(fd);
                 }
             }
             else
             {
                int fd = open(PIPE_NAME, O_WRONLY);
                 write(fd, "Error: file not found\n",strlen("Error: file not found\n"));
                 close(fd);
             }
         }
         else if (strcmp(command, "read") == 0)
         {
             pthread_mutex_lock(&file_list_mutex);
             int i;
             for (i = 0; i < MAX_FILES; i++)
             {
                 if (strcmp(file_list[i], filename) == 0)
                 {
                     break;
                 }
             }
             pthread_mutex_unlock(&file_list_mutex);
             if (i < MAX_FILES)
             {
                 if (fd < 0)
                 {   fd = open(PIPE_NAME, O_WRONLY);
                     write(fd, "Error reading file\n", strlen("Error reading file\n"));
                 }
                 else
                 {fd = open(PIPE_NAME, O_WRONLY);
                     while ((num_bytes = read(fd, buf, BUF_SIZE)) > 0)
                     {
                         write(fd, buf, num_bytes);
                     }
                     //write(fd, "\n", strlen("\n"));
                     close(fd);
                 }
             }
             else
             {  fd = open(PIPE_NAME, O_WRONLY);
                 write(fd, "Error: file not found\n", strlen("Error: file not found\n"));
                 close(fd);
             }
         }
         else if (strcmp(command, "write") == 0)
         {
             pthread_mutex_lock(&file_list_mutex);
             int i;
             for (i = 0; i < MAX_FILES; i++)
             {
                 if (strcmp(file_list[i], filename) == 0)
                 {
                     break;
                 }
             }
             pthread_mutex_unlock(&file_list_mutex);
             if (i < MAX_FILES)
             {
                 int fd = open(filename, O_WRONLY | O_APPEND);
                 if (fd < 0)
                 {   int fd = open(PIPE_NAME, O_WRONLY);
                     write(fd, "Error writing to file\n", strlen("Error writing to file\n"));
                     close(fd);
                 }
                 else
                 {
                     write(fd, buf + strlen(command) + strlen(filename) + 2, num_bytes - strlen(command) - strlen(filename) - 2);
                     write(fd, "\n", strlen("\n"));
                     close(fd);
                     int fd = open(PIPE_NAME, O_WRONLY);
                     write(fd, "Write successful\n", strlen("Write successful\n"));
                     close(fd);
                 }
             }
             else
             {   int fd = open(PIPE_NAME, O_WRONLY);
                 write(fd, "Error: file not found\n", strlen("Error: file not found\n"));
                 close(fd);
             }
         }
         else if (strcmp(command, "exit") == 0)
         {
            fd = open(PIPE_NAME, O_WRONLY);
            return 1;
            close(fd);
             break;
         }
         else
         {   fd = open(PIPE_NAME, O_WRONLY);
             //write(fd, "Error: invalid command\n", strlen("Error: invalid command\n"));
             close(fd);
             //sleep(1);
            break;
         }
    }
    close(fd);
    pthread_exit(NULL);
}

int main()
{
    // Create lock
    pthread_mutex_init(&file_list_mutex, NULL);

    // Create named pipe
    mkfifo(PIPE_NAME, 0666);

    while (1)
    {
        // Open named pipe for reading
        int fd = open(PIPE_NAME, O_RDONLY);
        if (fd < 0)
        {
            perror("Error opening pipe for reading");
            continue;
        }

        // Create client thread
        pthread_t client_thread_id;
        if (pthread_create(&client_thread_id, NULL, client_thread, (void *)&fd) != 0)
        {
            perror("Error creating client thread");
            close(fd);
            continue;
        }
    }
    // Destroy lock
    pthread_mutex_destroy(&file_list_mutex);

    return 0;
}

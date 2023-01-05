#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>

#define BUF_SIZE 1024
#define PIPE_NAME "/tmp/file_manager_named_pipe"

int main()
{
  while (1)
  {
    char buf[BUF_SIZE];
    printf("Enter command: ");
    fgets(buf, BUF_SIZE, stdin);
    buf[strlen(buf) - 1] = '\0';
    char command[BUF_SIZE];
    char filename[BUF_SIZE];
    sscanf(buf, "%s %s", command, filename);
    if (strcmp(command, "exit") == 0)
    {
      printf("Exiting...\n");
      return 1;
    }

    // Open named pipe for writing
    int fd = open(PIPE_NAME, O_WRONLY);
    if (fd < 0)
    {
      perror("Error opening pipe for writing");
      continue;
    }

    write(fd, buf, strlen(buf));
    close(fd);

    // Open named pipe for reading
    fd = open(PIPE_NAME, O_RDONLY);
    if (fd < 0)
    {
      perror("Error opening pipe for reading");
      continue;
    }

    // Doesnt work properly
    int num_bytes = read(fd, buf, BUF_SIZE);
    buf[num_bytes] = '\0';
    printf("%s\n", buf);
    close(fd);
  }

  return 0;
}
/* Put includes up here. */
#include <sys/types.h>         
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>

#define K	4096

void parent(pid_t pid, int sv[2])
{
  close(sv[1]);
  char buf[K];
  ssize_t r;
  
  while((r = read(STDIN_FILENO, buf, K)) > 0) {
    if (writeall(sv[0], buf, r) < 0) break;
    r = read(sv[0], buf, K);
    if (writeall(STDOUT_FILENO, buf, r) < 0) break;
  }
  close(sv[0]);
  waitpid(pid, NULL, 0);
}

void child(int sv[2])
{
  close(sv[0]);
  char buf[K];
  ssize_t r;
  int i;

  while((r = read(sv[1], buf, K)) > 0) {
    /* Do the conversion */
    for(i = 0; i < r; i++) {
      if (isalpha(buf[i])) {
	if (islower(buf[i])) {
	  buf[i] = 'a' + (((buf[i]-'a')+13)%26);
	} else {
	  buf[i] = 'A' + (((buf[i]-'A')+13)%26);
	}
      }
    }
    if (writeall(sv[1], buf, r) < 0) break;
  }
  close(sv[1]);
  exit(1);
}

int main(void)
{
  int sv[2];
  
  if (socketpair(AF_UNIX, SOCK_STREAM, 0, sv) < 0) {
    perror("socketpair");
    exit(0);
  }
  pid_t pid = fork();
  if (pid < 0) {
    perror("fork");
    exit(0);
  }
  if (pid == 0) child(sv);
  else parent(pid, sv);
  
  return 0;
}

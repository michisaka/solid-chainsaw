/*
main module

copyright (C) 2017 Koshi.Michisaka
*/

#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <errno.h>

#include "config.h"

typedef void Sigfunc(int);

Sigfunc *setup_signal(int signo, Sigfunc *func);
void sigchld_handler(int signo);


int main(int argc, char **argv)
{
  config config;
  int listenfd, connectfd;
  struct sockaddr_in listenaddr, clientaddr;
  socklen_t len;
  pid_t child_pid;

  build_config(argc, argv, &config);

  if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
    perror("socket error");
    exit(EXIT_FAILURE);
  }

  memset(&listenaddr, 0, sizeof(listenaddr));
  listenaddr.sin_family = AF_INET;
  listenaddr.sin_addr.s_addr = config.bind_address;
  listenaddr.sin_port = config.bind_port;

  if ((bind(listenfd, (struct sockaddr*)&listenaddr, sizeof(listenaddr))) == -1) {
    perror("bind error");
    exit(EXIT_FAILURE);
  }

  if ((listen(listenfd, 5)) == -1) {
    perror("listen error");
    exit(EXIT_FAILURE);
  }

  setup_signal(SIGCHLD, sigchld_handler);
  setup_signal(SIGPIPE, SIG_IGN);

  for (;;) {
    len = sizeof(clientaddr);
    if ((connectfd = accept(listenfd, (struct sockaddr*)&clientaddr, &len)) == -1) {
      if (errno == EINTR) {
	continue;
      }
      perror("accept error");
      exit(EXIT_FAILURE);
    }

    child_pid = fork();
    switch (child_pid) {
    case -1:
      perror("fork error");
      exit(EXIT_FAILURE);
    case 0:
      close(listenfd);
      close(connectfd);
      exit(EXIT_SUCCESS);
    default:
      close(connectfd);
    }
  }

  exit(EXIT_SUCCESS);
}

Sigfunc *setup_signal(int signo, Sigfunc *func)
{
  struct sigaction	act, oact;

  act.sa_handler = func;
  sigemptyset(&act.sa_mask);
  act.sa_flags = 0;
  if (signo == SIGALRM) {
#ifdef	SA_INTERRUPT
    act.sa_flags |= SA_INTERRUPT;	/* SunOS 4.x */
#endif
  } else {
#ifdef	SA_RESTART
    act.sa_flags |= SA_RESTART;		/* SVR4, 44BSD */
#endif
  }
  if (sigaction(signo, &act, &oact) < 0)
    return(SIG_ERR);
  return(oact.sa_handler);
}

void sigchld_handler(int signo)
{
  pid_t pid;
  int stat;

  while ((pid = waitpid(-1, &stat, WNOHANG)) > 0) {
    ;
  }

  return;
}

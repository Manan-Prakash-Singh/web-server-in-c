#define _GNU_SOURCE
#include "networking.h"
#include <asm-generic/errno.h>
#include <asm-generic/socket.h>
#include <errno.h>
#include <netinet/tcp.h>

int bz_create_socket(const char *sock_type) {
  int fd = 0;
  if (strcasecmp(sock_type, "tcp") == 0) {
    fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) {
      exit(1);
    }
  } else if (strcasecmp(sock_type, "udp") == 0) {
    fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd < 0) {
      exit(1);
    }
  } else {
    fprintf(stderr, "protocol not recognized.\n");
    exit(1);
  }

  return fd;
}

void bz_bind_socket(int fd, struct sockaddr_in *sadr) {
  if (bind(fd, (const struct sockaddr *)&sadr, sizeof(*sadr)) < 0) {
    perror("bind()");
    exit(1);
  }
}

void bz_start_listening(int fd, int backlog) {
  if (listen(fd, backlog) < 0) {
    perror("listen()");
    fprintf(stderr, "Server could not start. Aborting...\n");
    exit(1);
  }
}

int bz_set_socket_nonblocking(int fd) {
  int flags = fcntl(fd, F_GETFL, 0);
  
  if (flags < 0) {
    perror("fcntl()");
    return -1;
  }

  flags |= O_NONBLOCK;

  if (fcntl(fd, F_SETFL, flags) < 0) {
    perror("fcntl()");
    return -1;
  }
  return 0;
}

int bz_set_so_reuse_addr(int fd) {
  int yes = 1;
  if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &fd, sizeof(fd)) < 0) {
    perror("setsockopt()");
    exit(1);
  }
  return 0;
}

int bz_set_so_reuse_port(int fd) {
  int yes = 1;
  if (setsockopt(fd, SOL_SOCKET, SO_REUSEPORT, &fd, sizeof(fd)) < 0) {
    perror("setsockopt()");
    exit(1);
  }
  return 0;
}

int bz_set_tcp_keepalive(int fd) {
  int yes = 1;
  if (setsockopt(fd, SOL_SOCKET, SO_KEEPALIVE, &fd, sizeof(fd)) < 0) {
    perror("setsockopt()");
    exit(1);
  }
  return 0;
}

int bz_set_tcp_nodelay(int fd) {
  int yes = 1;
  if (setsockopt(fd, SOL_TCP, TCP_NODELAY, &fd, sizeof(fd)) < 0) {
    perror("setsockopt()");
    exit(1);
  }
  return 0;
}

/*  Use accept4() if the system has it. accept4() can accept
    an incoming connection and put it in non-blocking mode simultaneously.
    This saves system call to fcntl.
    In case accept4() is not defined, fallback to accept() and do
    fcntl()   */
int bz_accept(int fd, struct sockaddr *adr, socklen_t *len) {

  int connfd = -1;

#ifdef __linux__
  connfd = accept4(fd, adr, len, SOCK_NONBLOCK);
#else
  connfd = accept(fd, addr, len);
  if (bz_set_socket_nonblocking(fd) < 0) {
    close(fd);
    return -1;
  }
#endif

  return connfd;
}
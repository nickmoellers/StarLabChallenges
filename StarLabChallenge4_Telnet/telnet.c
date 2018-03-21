/* 
 * telnet.c - A simple TCP server, login and echo
 * author: nicholas moellers
 * usage: make all && make test
 * then add a username
 * then add a password
 * then if you log in, it will echo text back to you
 * Starter code from: https://www.cs.cmu.edu/afs/cs/academic/class/15213-f99/www/class26/tcpclient.c&usg=AOvVaw2guohV-5yGoaKmxciKegiD
 */

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define BUFSIZE 1024
#define NUM_USERS 3

#define MAX(x, y) (((x) > (y)) ? (x) : (y))
#define MIN(x, y) (((x) < (y)) ? (x) : (y))

#if 0
/* 
 * Structs exported from in.h
 */

/* Internet address */
struct in_addr {
  unsigned int s_addr; 
};

/* Internet style socket address */
struct sockaddr_in  {
  unsigned short int sin_family; /* Address family */
  unsigned short int sin_port;   /* Port number */
  struct in_addr sin_addr;	 /* IP address */
  unsigned char sin_zero[...];   /* Pad to size of 'struct sockaddr' */
};

/*
 * Struct exported from netdb.h
 */

/* Domain name service (DNS) host entry */
struct hostent {
  char    *h_name;        /* official name of host */
  char    **h_aliases;    /* alias list */
  int     h_addrtype;     /* host address type */
  int     h_length;       /* length of address */
  char    **h_addr_list;  /* list of addresses */
}
#endif

/*
 * error - wrapper for perror
 */
void error(char *msg) {
  perror(msg);
  exit(1);
}

int main(int argc, char **argv) {
  int parentfd; /* parent socket */
  int childfd; /* child socket */
  int portno; /* port to listen on */
  int clientlen; /* byte size of client's address */
  struct sockaddr_in serveraddr; /* server's addr */
  struct sockaddr_in clientaddr; /* client addr */
  struct hostent *hostp; /* client host info */
  char unamebuf[BUFSIZE]; /* message buffer */
  char passbuf[BUFSIZE]; /* message buffer */
  char echobuf[BUFSIZE]; /* message buffer */
  char * hostaddrp; /* dotted decimal host addr string */
  int optval; /* flag value for setsockopt */
  int n; /* message byte size */
  char *prompt;

  const char * unames[NUM_USERS];
  unames[0] = "nicholas";
  unames[1] = "michael";
  unames[2] = "moellers";

  const char * passes[NUM_USERS];
  passes[0] = "12345";
  passes[1] = "qwerty";
  passes[2] = "password";

  /* 
   * check command line arguments 
   */
  /*if (argc != 2) {
    fprintf(stderr, "usage: %s <port>\n", argv[0]);
    exit(1);
  }*/
  portno = 4567; //atoi(argv[1]);

  /* 
   * socket: create the parent socket 
   */
  parentfd = socket(AF_INET, SOCK_STREAM, 0);
  if (parentfd < 0) 
    error("ERROR opening socket");

  /* setsockopt: Handy debugging trick that lets 
   * us rerun the server immediately after we kill it; 
   * otherwise we have to wait about 20 secs. 
   * Eliminates "ERROR on binding: Address already in use" error. 
   */
  optval = 1;
  setsockopt(parentfd, SOL_SOCKET, SO_REUSEADDR, 
	     (const void *)&optval , sizeof(int));

  /*
   * build the server's Internet address
   */
  bzero((char *) &serveraddr, sizeof(serveraddr));

  /* this is an Internet address */
  serveraddr.sin_family = AF_INET;

  /* let the system figure out our IP address */
  serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);

  /* this is the port we will listen on */
  serveraddr.sin_port = htons((unsigned short)portno);

  /* 
   * bind: associate the parent socket with a port 
   */
  if (bind(parentfd, (struct sockaddr *) &serveraddr, 
	   sizeof(serveraddr)) < 0) 
    error("ERROR on binding");

  /* 
   * listen: make this socket ready to accept connection requests 
   */
  if (listen(parentfd, 5) < 0) /* allow 5 requests to queue up */ 
    error("ERROR on listen");

  /* 
   * main loop: wait for a connection request, echo input line, 
   * then close connection.
   */
  clientlen = sizeof(clientaddr);
  //while (1) {

    /* 
     * accept: wait for a connection request 
     */
    childfd = accept(parentfd, (struct sockaddr *) &clientaddr, &clientlen);
    if (childfd < 0) 
      error("ERROR on accept");
    
    /* 
     * gethostbyaddr: determine who sent the message 
     */
    //I could just do this by "localhost, but this is fancy!
    hostp = gethostbyaddr((const char *)&clientaddr.sin_addr.s_addr, 
			  sizeof(clientaddr.sin_addr.s_addr), AF_INET);
    if (hostp == NULL)
      error("ERROR on gethostbyaddr");
    hostaddrp = inet_ntoa(clientaddr.sin_addr);

    //hostaddrp = "localhost";
    if (hostaddrp == NULL)
      error("ERROR on inet_ntoa\n");
    printf("server established connection with %s (%s)\n", 
	   hostp->h_name, hostaddrp);

    // Ask for username
    prompt = "Username:\n";
    n = write(childfd, prompt, strlen(prompt));
    if (n < 0) 
      error("ERROR writing to socket");

    // Wait for username
    bzero(unamebuf, BUFSIZE);
    n = read(childfd, unamebuf, BUFSIZE);
    if (n < 0) 
      error("ERROR reading from socket");
    unamebuf[strlen(unamebuf)-2]='\0';
    printf("%s%s\n", prompt, unamebuf );
    //strcpy(usernameInput, unamebuf);

    // Ask for password
    prompt = "Password:\n";
    n = write(childfd, prompt, strlen(prompt));
    if (n < 0) 
      error("ERROR writing to socket");

    // Wait for password
    bzero(passbuf, BUFSIZE);
    n = read(childfd, passbuf, BUFSIZE);
    if (n < 0) 
      error("ERROR reading from socket");
    passbuf[strlen(passbuf)-2]='\0';
    printf("%s%s\n", prompt, passbuf );

    // Check username
    int id = -1;
    for( int i = 0 ; i < NUM_USERS ; i++ ) {
      if( strcmp( unames[i], unamebuf)==0 )  {
          id = i;
      }
    }

    //Check password if username found
    int login = -1;
    if( id >=0  && id < NUM_USERS ) {
      if( strcmp( passes[id], passbuf )== 0 ) {
        login=1;
      }
    }

    //check results and publish to client
    if( login == 1) {
      // Ask for password
      prompt = "Login successful!\nEchoing:\n";
    } else {
      if( id == -1 ) {
        prompt = "Login unsuccessful: incorrect username,\n";
      } else {
        prompt = "Login unsuccessful: incorrect password.\n";
      }
    }
    n = write(childfd, prompt, strlen(prompt));
    if (n < 0) 
      error("ERROR writing to socket");

    if( login == 1 ) {
      while( strcmp( echobuf , "close\n" ) ) {

        // Wait for message
        bzero(echobuf, BUFSIZE);
        n = read(childfd, echobuf, BUFSIZE);
        if (n < 0) 
          error("ERROR reading from socket");

        echobuf[n-2]='\n';
        echobuf[n-1]='\0';
        
        //Echo message
        printf("User typed:\t%s\n", echobuf );
        n = write(childfd, echobuf, strlen(echobuf));
        if (n < 0) 
          error("ERROR writing to socket");
      }
    }

    //close connection
    close(childfd);
  
  return 0;
}

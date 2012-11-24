#include <stdio.h>    /* Standard input/output definitions */
#include <stdlib.h> 
#include <stdint.h>   /* Standard types */
#include <string.h>   /* String function definitions */
#include <unistd.h>   /* UNIX standard function definitions */
#include <fcntl.h>    /* File control definitions */
#include <errno.h>    /* Error number definitions */
#include <termios.h>  /* POSIX terminal control definitions */
#include <sys/ioctl.h>
#include <signal.h>

#include <Ecore.h>


int serialport_init(const char* serialport, int baud);
int serialport_read_until(int fd, char* buf, char until);
int serialport_write(int fd, const char* str);


int _log_dom = -1;


static void
do_lengthy_task(Ecore_Pipe *pipe)
{
    int fd = 0;
	char buf[256];
	int baudrate = B9600;  // default
	char *buffer;
   /*
   for (i = 0; i < 20; i++)
     {
        sleep(1);
        buffer = malloc(sizeof(char) * i);
        for (j = 0; j < i; j++)
          buffer[j] = 'a' + j;
        ecore_pipe_write(pipe, buffer, i);
        free(buffer);
     }
     */

   	fd= serialport_init("/dev/ttyUSB0", baudrate);

	while(1)
	{
		serialport_write(fd, "DEVICE;0;DHT11;INT;17.296\n");
		serialport_read_until(fd, buf, '\n');
		buffer = strdup(buf);
		ecore_pipe_write(pipe, buffer, strlen(buffer));
		free(buffer);
		buffer = NULL;
		sleep(2);
   }
   
   	ecore_pipe_write(pipe, "close", 5);   
}

static void
handler(void *data, void *buf, unsigned int len)
{
   //printf("content: %s\n", (const char *)buf);
   //printf("received %d bytes\n", len);

   char *str = malloc(sizeof(char) * len + 1);
   memcpy(str, buf, len);
   str[len] = '\0';
   printf("received %d bytes\n", len);
   printf("content: %s\n", (const char *)str);
   free(str);
   if (len && !strncmp(buf, "close", len < 5 ? len : 5))
     {
        printf("close requested\n");
        ecore_main_loop_quit();
     }
}


int
main(int argc, char *argv[])
{
   Ecore_Pipe *pipe;
   pid_t child_pid;

   ecore_init();
   
   pipe = ecore_pipe_add(handler, NULL);
   
	child_pid = fork();
      
   if (!child_pid)
     {
        ecore_pipe_read_close(pipe);
        do_lengthy_task(pipe);
     }
   else
     {
        ecore_pipe_write_close(pipe);
        ecore_main_loop_begin();
     }      
     
	kill(child_pid, SIGKILL);
	
   ecore_pipe_del(pipe);
   ecore_shutdown();

   return 0;
}




int serialport_write(int fd, const char* str)
{
    int len = strlen(str);
    int n = write(fd, str, len);
    if( n!=len ) 
        return -1;
    return 0;
}

int serialport_read_until(int fd, char* buf, char until)
{
    char b[1];
    int i=0;
    do { 
        int n = read(fd, b, 1);  // read a char at a time
        if( n==-1) return -1;    // couldn't read
        if( n==0 ) {
            usleep( 10 * 1000 ); // wait 10 msec try again
            continue;
        }
        buf[i] = b[0]; i++;
    } while( b[0] != until );

    buf[i] = 0;  // null terminate the string
    return 0;
}

// takes the string name of the serial port (e.g. "/dev/tty.usbserial","COM1")
// and a baud rate (bps) and connects to that port at that speed and 8N1.
// opens the port in fully raw mode so you can send binary data.
// returns valid fd, or -1 on error
int serialport_init(const char* serialport, int baud)
{
    struct termios toptions;
    int fd;
    
    //fprintf(stderr,"init_serialport: opening port %s @ %d bps\n",
    //        serialport,baud);

    fd = open(serialport, O_RDWR | O_NOCTTY | O_NDELAY);
    if (fd == -1)  {
        perror("init_serialport: Unable to open port ");
        return -1;
    }
    
    if (tcgetattr(fd, &toptions) < 0) {
        perror("init_serialport: Couldn't get term attributes");
        return -1;
    }
    speed_t brate = baud; // let you override switch below if needed
    switch(baud) {
    case 4800:   brate=B4800;   break;
    case 9600:   brate=B9600;   break;
    case 19200:  brate=B19200;  break;
    case 38400:  brate=B38400;  break;
    case 57600:  brate=B57600;  break;
    case 115200: brate=B115200; break;
    }
    cfsetispeed(&toptions, brate);
    cfsetospeed(&toptions, brate);

    // 8N1
    toptions.c_cflag &= ~PARENB;
    toptions.c_cflag &= ~CSTOPB;
    toptions.c_cflag &= ~CSIZE;
    toptions.c_cflag |= CS8;
    // no flow control
    toptions.c_cflag &= ~CRTSCTS;

    toptions.c_cflag |= CREAD | CLOCAL;  // turn on READ & ignore ctrl lines
    toptions.c_iflag &= ~(IXON | IXOFF | IXANY); // turn off s/w flow ctrl

    toptions.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG); // make raw
    toptions.c_oflag &= ~OPOST; // make raw

    // see: http://unixwiz.net/techtips/termios-vmin-vtime.html
    toptions.c_cc[VMIN]  = 0;
    toptions.c_cc[VTIME] = 20;
    
    if( tcsetattr(fd, TCSANOW, &toptions) < 0) {
        perror("init_serialport: Couldn't set term attributes");
        return -1;
    }

    return fd;
}

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>

int main(void)
{
	int fd1, fd2;
	char text[1023];
	char read_buf[1020];
	int write_bytes;
	int read_bytes;
	
	fd1 = open("/dev/my_char_driver", O_RDWR);

	fd2 = open("/var/log/dpkg.log", O_RDONLY);

	if (fd1 == -1 && fd2 == -1) {
		printf("Error in reading file\n");
		return -1;
	}

	read_bytes = read(fd2, text, sizeof(text));

	write_bytes = write(fd1, text, sizeof(text));

	read_bytes = read(fd1, read_buf, sizeof(read_buf));

	if (read_bytes > write_bytes || read_bytes == 0) {
		printf("its not writing correctly dude\n");
		return -1;
	}

	printf("%s \n", read_buf);
	return 0;
}
		
	      

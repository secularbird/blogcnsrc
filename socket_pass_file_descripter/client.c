#include <stdio.h>
#include <sys/socket.h>
#include <errno.h>
#include <string.h>
#include <sys/un.h>
#include <unistd.h>
#include <limits.h>

#include "commondef.h"

int recv_fd(int socket)
{
	int sent_fd, available_ancillary_element_buffer_space;
	struct msghdr socket_message;
	struct iovec io_vector[1];
	struct cmsghdr *control_message = NULL;
	char message_buffer[1];
	char ancillary_element_buffer[CMSG_SPACE(sizeof(int))];

	/* start clean */
	memset(&socket_message, 0, sizeof(struct msghdr));
	memset(ancillary_element_buffer, 0, CMSG_SPACE(sizeof(int)));

	/* setup a place to fill in message contents */
	io_vector[0].iov_base = message_buffer;
	io_vector[0].iov_len = 1;
	socket_message.msg_iov = io_vector;
	socket_message.msg_iovlen = 1;

	/* provide space for the ancillary data */
	socket_message.msg_control = ancillary_element_buffer;
	socket_message.msg_controllen = CMSG_SPACE(sizeof(int));

	if(recvmsg(socket, &socket_message, 0) < 0)
		return -1;

	if(message_buffer[0] != 'F')
	{
		/* this did not originate from the above function */
		return -1;
	}

	if((socket_message.msg_flags & MSG_CTRUNC) == MSG_CTRUNC)
	{
		/* we did not provide enough space for the ancillary element array */
		return -1;
	}

	/* iterate ancillary elements */
	for(control_message = CMSG_FIRSTHDR(&socket_message);
	control_message != NULL;
	control_message = CMSG_NXTHDR(&socket_message, control_message))
	{
		if( (control_message->cmsg_level == SOL_SOCKET) &&
			(control_message->cmsg_type == SCM_RIGHTS) )
		{
			sent_fd = *((int *) CMSG_DATA(control_message));
			return sent_fd;
		}
	}

	return -1;
}

int main (int argc, char const *argv[])
{
	struct sockaddr_un server_address;
	int socket_fd;
	int rev_fd = -1;
	char *buffer = "This is from client";
	
	socket_fd = socket(AF_UNIX, SOCK_STREAM, 0);
	if(socket_fd < 0)
	{
		perror("socket() failed\n");
		return 1;
	}

	/* start with a clean address structure */
	memset(&server_address, 0, sizeof(struct sockaddr_un));
 
	server_address.sun_family = AF_UNIX;
	snprintf(server_address.sun_path, sizeof(server_address.sun_path), SOCKET_NAME);

	if(connect(socket_fd, (struct sockaddr *) &server_address, sizeof(struct sockaddr_un)) != 0)
	{
		perror("connect() failed\n");
		return 1;
	} 
	
	
	rev_fd = recv_fd(socket_fd);
	printf("recevied fd is %d\n",rev_fd);
	sleep(10);
	write(rev_fd, buffer, strlen(buffer)+1);
	
	close(socket_fd);
	
	return 0;
}
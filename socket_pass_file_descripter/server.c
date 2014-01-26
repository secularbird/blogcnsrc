#include <stdio.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>

int send_fd(int socket, int fd_to_send)
{
	struct msghdr socket_message;
	struct iovec io_vector[1];
	struct cmsghdr *control_message = NULL;
	char message_buffer[1];
	/* storage space needed for an ancillary element with a paylod of length is CMSG_SPACE(sizeof(length)) */
	char ancillary_element_buffer[CMSG_SPACE(sizeof(int))];
	int available_ancillary_element_buffer_space;

	/* at least one vector of one byte must be sent */
	message_buffer[0] = 'F';
	io_vector[0].iov_base = message_buffer;
	io_vector[0].iov_len = 1;

	/* initialize socket message */
	memset(&socket_message, 0, sizeof(struct msghdr));
	socket_message.msg_iov = io_vector;
	socket_message.msg_iovlen = 1;

	/* provide space for the ancillary data */
	available_ancillary_element_buffer_space = CMSG_SPACE(sizeof(int));
	memset(ancillary_element_buffer, 0, available_ancillary_element_buffer_space);
	socket_message.msg_control = ancillary_element_buffer;
	socket_message.msg_controllen = available_ancillary_element_buffer_space;

	/* initialize a single ancillary data element for fd passing */
	control_message = CMSG_FIRSTHDR(&socket_message);
	control_message->cmsg_level = SOL_SOCKET;
	control_message->cmsg_type = SCM_RIGHTS;
	control_message->cmsg_len = CMSG_LEN(sizeof(int));
	*((int *) CMSG_DATA(control_message)) = fd_to_send;

	return sendmsg(socket, &socket_message, 0);
}


int main (int argc, char const *argv[])
{
	struct sockaddr_un client_address;
	struct sockaddr_un server_address;
	int socket_fd = 0;
	int client_fd = 0;
	int address_length = 0;
	
	socket_fd = socket(AF_UNIX, SOCK_STREAM, 0);
	if(socket_fd < 0) {
		perror("socket() failed\n");
	}
	
	unlink("./demo_socket");
	
   	/* start with a clean address structure */
    memset(&server_address, 0, sizeof(struct sockaddr_un));
	
    server_address.sun_family = AF_UNIX;
    snprintf(server_address.sun_path, sizeof(server_address.sun_path), "./demo_socket");
	
    if(bind(socket_fd, 
            (struct sockaddr *) &server_address, 
            sizeof(struct sockaddr_un)) != 0)
    {
     perror("bind() failed\n");
     return 1;
    }
	
    if(listen(socket_fd, 5) != 0)
    {
     perror("listen() failed\n");
     return 1;
    }
	
	if((client_fd = accept(socket_fd, (struct sockaddr *) &client_address, &address_length)) > -1 )
	{
		printf("client connected\n");
		if(send_fd(client_fd, 1) < 0) {
			perror("send_fd()");
		} else {
			printf("fd sended\n");
		}
	}
	
	close(socket_fd);  
	
	return 0;
}
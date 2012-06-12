#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <errno.h>
#include <netdb.h>

#define SERV_PORT 80
int get(int sockfd, const char * host, const char * url, const char * version);
void response(int sockfd);
char ** resolve_url(char * url);

int main(int argc, char * argv[])
{
	int	sockfd;
	int 	servport;
	int 	n;
	char recvbuf[32678];
	char **	res;
	struct addrinfo	hints;
	struct addrinfo *	result;
	int 	dnserrno;
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = 0;
	if (argc < 2)
		fprintf(stderr, "usage: %s <URL>", argv[0]);
	res = resolve_url(argv[1]);
	//printf("%s\n%s\n%s\n", res[0], res[1], res[2]);
	dnserrno = getaddrinfo(res[1], res[0], &hints, &result);
	if (dnserrno)
	{
		fprintf(stderr, "%s\n", gai_strerror(dnserrno));
		exit(1);
	}
	if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
		perror("socket create error:");
	if (connect(sockfd, (struct sockaddr *) result->ai_addr, sizeof(struct sockaddr)) < 0)
		perror("connect error:");
	get(sockfd, res[1], res[2], "HTTP/1.1");
	response(sockfd);
	close(sockfd);
	freeaddrinfo(result);
	return 0;
}

int get(int sockfd, const char * host, 
		const char * path, 
		const char * version)
{
	char buf[4096];
	sprintf(buf, "GET ");
	strcat(buf, path);
	strcat(buf, " ");
	strcat(buf, version);
	strcat(buf, "\r\nHOST: ");
	strcat(buf, host);
	strcat(buf, "\r\n\r\n");
	write(sockfd, buf, strlen(buf));
	return 0;
}

void response(int sockfd)
{
	char buf[32768];
	char code_str[64];
	int code;
	int n;
	char * html;
	while(1)
	{
		n = read(sockfd, buf, 32678);
		if (n == EINTR)
			continue;
		else if (n < 0)
		{
			fprintf(stderr, "read error\n");
			exit(1);
		}
		else
		{

			strncpy(code_str, strchr(buf, ' ') + 1, 3);
			code_str[3] = '\0';
			code = atoi(code_str);
			switch (code) {
				case 200: 
					if (html = strstr(buf, "\r\n\r\n"))
						printf("%s\n", html + 4);
					else
						printf("%s\n", buf);
					break;
				default:
					bzero(&code_str, 64);
					strncpy(code_str, strchr(buf, ' ') + 1, 
							strchr(buf, '\r') - strchr(buf, ' '));
					printf("%s\n", code_str);
			}
			break;
		}
	}
	return ;
}

char ** resolve_url(char * url)
{
	char ** res;
	char * protocol = NULL;
	char * domainname = NULL;
	char * path = NULL;
	res = malloc(3 * sizeof (char *));
	int 	len;
	char *	start;
	if (start = strstr(url, "://"))
	{
		len = start - url;
		protocol = malloc((len + 1) * sizeof (char));
		strncpy(protocol, url, len);
		protocol[len] = '\0';
		start += 3;
	}
	else 
	{
		protocol = malloc(5 * sizeof (char));
		sprintf(protocol, "http");
		start = url;
	}
	if ((len = strchr(start, '/') - start) == ((char *)NULL - start))
	{
		domainname = url;	
		path = malloc(2 * sizeof (char));
		sprintf(path, "/");
	}
	else
	{
		domainname = malloc((len + 1) * sizeof (char));
		strncpy(domainname, start, len);
		domainname[len] = '\0';
		path = start + len;
	}
	res[0] = protocol;
	res[1] = domainname;
	res[2] = path;
	return res;
}

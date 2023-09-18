#include <errno.h>
#include <unistd.h>
#include <malloc.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <stdio.h>
#include <netinet/in.h>
#include <resolv.h>
#include <pthread.h>
#include "openssl/ssl.h"
#include "openssl/err.h"
#define min(x, y) (((x)<(y))?(x):(y))

typedef struct listen_hd {
	int port_num;
	SSL_CTX *ctx;
} listen_hd;


void BlockSigno(int signo)
{
    sigset_t signal_mask;
    sigemptyset(&signal_mask);
    sigaddset(&signal_mask, signo);
    pthread_sigmask(SIG_BLOCK, &signal_mask, NULL);
}

void sigpipe_hander(){
	perror("SIGPIPE CAUGHT!");
}

void* handle_https_request(void *arg) {
    SSL *ssl = (SSL*)arg;
	if (SSL_accept(ssl) == -1){
		perror("SSL_accept failed");
		exit(1);
	}

	char listen_buffer[1024] = {0};
	uint8_t response_buff[50000] = {0};
	int response_len = 0;

    int bytes = SSL_read(ssl, listen_buffer, sizeof(listen_buffer));
	if (bytes < 0) {
		perror("SSL_read failed");
		exit(1);
	}

	char method[10], url[50], version[15];
	sscanf(listen_buffer, "%s %s %s\r\n", method, url, version);

	if (strcmp(method, "GET")) {
		perror("Not supported method!");
		exit(1);
	}

	FILE *fp = fopen(url + 1, "r"); // 根据要访问的资源的URL打开文件流
	int bytes_in_file = 0;
	char message[40] = {0};
	
	// 如果访问文件流不存在，返回404,关闭
	if (fp == NULL) {
		strcat(message, "404 Not Found");
		response_len = sprintf((char*)response_buff, "%s %s\r\n", version, message);
		SSL_write(ssl, response_buff, strlen((char *)response_buff));
		int sock = SSL_get_fd(ssl);
    	SSL_free(ssl);
    	close(sock);
		perror("404 Not Found");
		return 0;
	} else {
		fseek(fp, 0, SEEK_END);
		bytes_in_file = ftell(fp) + 1; // 实际文件内总字节数
		fseek(fp, 0, SEEK_SET);
	}

	// 检查是否请求Range，VLC会请求Range
	char *range_ptr = strstr(listen_buffer, "Range:");
	int range_begin, range_end, range_flag, end_flag = 0;

	if (range_ptr != NULL) {
		int n_sscanf = sscanf(range_ptr, "Range: bytes=%d-%d\r\n", &range_begin, &range_end);
		// Deal with 100- like this.
		if (n_sscanf != 2) {
			range_end = -1;
		}
		strcat(message, "206 Partial Content");
		range_flag = 1;
	} else {
		strcat(message, "200 OK");
		range_flag = 0;
	}

	if (feof(fp)) {
		end_flag = 1;
		printf("PROCESS1 EOF!");
		fflush(stdout);
	}

	if (!range_flag) {
		// 情况1：没有请求Range信息，此时用HTTP 200 + HTTP-CHUNKED编码发送
		printf("HANDLE HTTPS GET WITHOUT RANGE: START!\n");
		fflush(stdout);
		response_len = sprintf((char*)response_buff, "%s %s\r\nConnection: close\r\nTransfer-Encoding: chunked\r\n\r\n", version, message);
		SSL_write(ssl, response_buff, response_len);
		while(!end_flag) {
			uint8_t content_buffer[4000] = {0};
			memset(response_buff, 0, sizeof(response_buff));
			response_len = fread(content_buffer, 1, 4000, fp);
			sprintf((char *)response_buff, "%x\r\n", response_len);
			int hex_length = 0;
			while(response_buff[hex_length] != '\r') hex_length++;
			memcpy(response_buff + hex_length + 2, content_buffer, response_len);
			sprintf((char *)response_buff + hex_length + 2 + response_len, "\r\n");
			SSL_write(ssl, response_buff, hex_length + response_len + 4);
			if (feof(fp)) {
				end_flag = 1;
				printf("HANDLE HTTPS GET WITHOUT RANGE: EOF!\n");
				fflush(stdout);
			}
		}
		memset(response_buff, 0, sizeof(response_buff));
		response_buff[0] = '0';
		response_buff[1] = '\r';
		response_buff[2] = '\n';
		response_buff[3] = '\r';
		response_buff[4] = '\n';
		SSL_write(ssl, response_buff, 5);
		// 只有不请求Range时才使用HTTP-CHUNKED编码，此时要发送0末尾。而RANGE-CONTENT不用发送。
		printf("HANDLE HTTPS GET WITHOUT RANGE: OVER!\n");
		fflush(stdout);
	} else if (range_flag) {
		fseek(fp, range_begin, SEEK_SET);
		if (range_end == -1) {
			// 情况2：请求了Range信息，但请求范围是“n-”类型。此时要返回总内容长度，用HTTP 206 Partial Content发送
			printf("HANDLE HTTPS GET WITH RANGE N-: START!\n");
			fflush(stdout);
			response_len = sprintf((char*)response_buff, "%s %s\r\nConnection: keep-alive\r\nContent-Length: %d\r\nContent-Range: bytes %d-%d/%d\r\n\r\n", 
				version, message, bytes_in_file, range_begin, bytes_in_file-1, bytes_in_file);
			SSL_write(ssl, response_buff, response_len);
			uint8_t content_buffer[4000];
			while(!end_flag) {
				response_len = fread(content_buffer, 1, 4000, fp);
				SSL_write(ssl, content_buffer, response_len);
				if (feof(fp)) {
					end_flag = 1;
					printf("HANDLE HTTPS GET WITH RANGE N-: EOF!\n");
					fflush(stdout);
				}
			}
			printf("HANDLE HTTPS GET WITH RANGE N-: END!\n");
			fflush(stdout);
		} else {
			// 情况3：请求了Range信息，且请求范围是“n-m”类型。此时要返回总内容长度，用HTTP 206 Partial Content发送
			printf("HANDLE HTTPS GET WITH RANGE N-M: START!\n");
			fflush(stdout);
			if(range_end >= bytes_in_file) range_end = bytes_in_file - 1; // 如果请求超范围，就按照发送最大范围来。
			// 注意start-end/total中，end最大为total-1
			int total_to_send = range_end - range_begin + 1; // 实际要发送的字节数
			int total_sent = 0; // 实际已经发送的字节数
			response_len = sprintf((char*)response_buff, "%s %s\r\nConnection: keep-alive\r\nContent-Length: %d\r\nContent-Range: bytes %d-%d/%d\r\n\r\n", 
					version, message, total_to_send, range_begin, range_end, total_to_send);
			SSL_write(ssl, response_buff, response_len);

			uint8_t content_buffer[4000];
			while(!end_flag) {
				response_len = total_sent + 4000 > total_to_send? total_to_send - total_sent: 4000;
				fread(content_buffer, 1, response_len, fp);
				SSL_write(ssl, content_buffer, response_len);
				total_sent += response_len;
				if (feof(fp) || total_sent == total_to_send) {
					end_flag = 1;
					printf("HANDLE HTTPS GET WITH RANGE N-M: EOF!\n");
					fflush(stdout);
				}
				printf("HANDLE HTTPS GET WITH RANGE N-M: END!\n");
				fflush(stdout);
			}
		}
	}
	
	int sock = SSL_get_fd(ssl);
    SSL_free(ssl);
    close(sock);
	printf("HTTPS THREAD EXIT!\n");
	fflush(stdout);
    return 0;	
}


/* This function aims to deal with the http
 * request from port 80.
 */
void* handle_http_request(void *sockfd) {
	char listen_buffer[1024] = {0};
	int csock = *(int*)sockfd;
	int bytes = recv(csock, listen_buffer, sizeof(listen_buffer), 0);
	if (bytes < 0) {
		perror("HTTP connection failed");
		exit(1);
	}
	char method[10], url[50], version[15], host_name[30];
	sscanf(listen_buffer, "%s %s %s\r\n", method, url, version);

	if (strcmp(method, "GET")) {
		perror("Not supported method!");
		exit(1);
	}

	char *host_pos = strstr(listen_buffer, "Host:");
	sscanf(host_pos, "Host: %s\r\n", host_name);

	const char *msg = "301 Moved Permanently";
	char location[100];
	strcat(location, "https://");
	strcat(location, host_name);
	strcat(location, url);

	char send_buf[1024] = {0};
	sprintf(send_buf, "%s %s\r\nLocation: %s\r\n", version, msg, location);

	send(csock, send_buf, sizeof(send_buf), 0);
	close(csock);
	printf("HTTP THREAD EXIT!\n");
	fflush(stdout);
	return 0;
}


/* This func aims to create a listening port for port
 * 443 and port 80. Port 443 uses https protocol and
 * port 80 uses http protocol.
 */
void* listen_port(void *arg) {
	// Get the arg's domain.
	listen_hd *fd = (listen_hd*)arg;
	int port_num = fd -> port_num;
	SSL_CTX *ctx = fd -> ctx;

	// Initialize the socket, get the handle sock.
	int sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock < 0) {
		perror("Opening socket failed");
		exit(1);
	}
	int enable = 1;
	if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0) {
		perror("setsockopt(SO_REUSEADDR) failed");
		exit(1);
	}

	struct sockaddr_in addr;
	bzero(&addr, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = INADDR_ANY;
	addr.sin_port = htons(port_num);

	if (bind(sock, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
		perror("Bind failed.");
		exit(1);
	}

	listen(sock, 10);

	while(1) {
		struct sockaddr_in caddr;
		socklen_t len;
		int csock = accept(sock, (struct sockaddr*)&caddr, &len);
		
		if (csock < 0) {
			perror("Accept failed");
			exit(1);
		}

		pthread_t client;
		if (port_num == 443) {
			SSL *ssl = SSL_new(ctx); 
			SSL_set_fd(ssl, csock);
			pthread_create(&client, NULL, handle_https_request, (void*)ssl);
		} else {
			pthread_create(&client, NULL, handle_http_request, (void*)&csock);
		}
	}
	close(sock);
}


int main() {
	// init SSL Library
	SSL_library_init();
	OpenSSL_add_all_algorithms();
	SSL_load_error_strings();
	// enable TLS method
	signal(SIGPIPE, sigpipe_hander);
	const SSL_METHOD *method = TLS_server_method();
	SSL_CTX *ctx = SSL_CTX_new(method);
    
	// load certificate and private key
	if (SSL_CTX_use_certificate_file(ctx, "./keys/cnlab.cert", SSL_FILETYPE_PEM) <= 0) {
		perror("load cert failed");
		exit(1);
	}

	if (SSL_CTX_use_PrivateKey_file(ctx, "./keys/cnlab.prikey", SSL_FILETYPE_PEM) <= 0) {
		perror("load prikey failed");
		exit(1);
	}

	// Create 2 threads to listen to 2 ports.
	pthread_t listen1, listen2;
	listen_hd p443, p80;

	p443.port_num = 443;
	p443.ctx = ctx;
	p80.port_num = 80;
	p80.ctx = NULL;

	int ret1 = pthread_create(&listen1, NULL, listen_port, (void*)&p443);
	int ret2 = pthread_create(&listen2, NULL, listen_port, (void*)&p80);

	if (ret1 || ret2) {
		perror("Creating new threads failed.");
		exit(1);
	}

	pthread_join(listen1, NULL);
	pthread_join(listen2, NULL);
	SSL_CTX_free(ctx);
}

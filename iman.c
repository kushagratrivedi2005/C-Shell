#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <stdbool.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include "color.h"

#define BUFFER_SIZE 4096

// Function to initialize and connect a socket to the specified server
int setup_connection(const char *hostname, int port_number)
{
    struct sockaddr_in server_address;
    struct hostent *host_info;

    // Create a TCP socket
    int socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd < 0)
    {
        perror(RED "Error creating socket" RESET);
        return -1;
    }

    // Retrieve the server's address information
    host_info = gethostbyname(hostname);
    if (host_info == NULL)
    {
        fprintf(stderr,RED "Host not found\n" RESET);
        close(socket_fd);
        return -1;
    }

    // Configure the server address structure
    memset(&server_address, 0, sizeof(server_address));
    server_address.sin_family = AF_INET;
    memcpy(&server_address.sin_addr.s_addr, host_info->h_addr, host_info->h_length);
    server_address.sin_port = htons(port_number);

    // Connect to the server
    if (connect(socket_fd, (struct sockaddr *)&server_address, sizeof(server_address)) < 0)
    {
        perror(RED "Connection error" RESET);
        close(socket_fd);
        return -1;
    }

    return socket_fd;
}

// Function to create and send an HTTP GET request, and handle the response
void fetch_man_page(const char *command_name)
{
    const char *hostname = "man.he.net";
    int port_number = 80;
    char request_buffer[BUFFER_SIZE];
    char response_buffer[BUFFER_SIZE];

    // Establish a connection to the server
    int socket_fd = setup_connection(hostname, port_number);
    if (socket_fd < 0)
    {
        return;
    }

    // Construct the HTTP GET request string
    snprintf(request_buffer, sizeof(request_buffer),
             "GET /?topic=%s&section=all HTTP/1.1\r\n"
             "Host: %s\r\n"
             "Connection: close\r\n\r\n",
             command_name, hostname);

    // Send the GET request to the server
    if (send(socket_fd, request_buffer, strlen(request_buffer), 0) < 0)
    {
        perror(RED "Request sending failed" RESET);
        close(socket_fd);
        return;
    }

    // Receive and process the server's response
    ssize_t bytes_received;
    bool in_html_tag = true;

    // Read the response in chunks
    while ((bytes_received = recv(socket_fd, response_buffer, BUFFER_SIZE - 1, 0)) > 0)
    {
        response_buffer[bytes_received] = '\0'; // Null-terminate the received data

        // Process the response buffer character by character
        for (int index = 0; index < bytes_received; ++index)
        {
            if (response_buffer[index] == '<')
            {
                in_html_tag = true; // Entering an HTML tag
            }

            if (!in_html_tag)
            {
                putchar(response_buffer[index]); // Output characters outside of HTML tags
            }

            if (response_buffer[index] == '>')
            {
                in_html_tag = false; // Exiting an HTML tag
            }
        }
    }

    if (bytes_received < 0)
    {
        perror(RED "Error receiving response" RESET);
    }

    // Close the socket
    close(socket_fd);
}

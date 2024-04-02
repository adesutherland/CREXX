//
// Created by Adrian Sutherland on 06/03/2024.
//
#include <stdio.h>
#include <stdlib.h>
#include <event2/event.h>
#include <event2/http.h>
#include <event2/buffer.h>
#include <event2/util.h>
#include <event2/http_struct.h>
#include <event2/keyvalq_struct.h>
#include <sys/queue.h>

// Common http headers for all requests
#define HTTP_COMMON "Content-Type: application/json\r\nServer: CREXX/0.1\r\nKeep-Alive: timeout=5, max=1000\r\nConnection: Keep-Alive\r\nCache-Control: no-cache\r\n"

// Default http header string to be used for normal connections
#define HTTP_HEADER HTTP_COMMON

// Default http header string to be used for long-polling connections
#define HTTP_HEADER_LONG_POLL HTTP_COMMON

void request_handler(struct evhttp_request *req, void *arg) {
    struct evbuffer *evb = evbuffer_new();

    printf("Request received %s\n", req->uri);

    // Print all headers to the console
    struct evkeyvalq *headers = req->input_headers;
    struct evkeyval *header;
    TAILQ_FOREACH(header, headers, next) {
        printf("Header: %s: %s\n", header->key, header->value);
    }


    // Add headers to the response
    evhttp_add_header(evhttp_request_get_output_headers(req), "Content-Type", "application/json");
    evhttp_add_header(evhttp_request_get_output_headers(req), "Server", "CREXX/0.1");
    evhttp_add_header(evhttp_request_get_output_headers(req), "Cache-Control", "no-cache");

    // TODO Keep-Alive: timeout=5, max=1000
    // TODO Connection: Keep-Alive

    // Add response to the body of the response
    // {
    //    "variable_name": "<variable_value>", ...
    // }
    evbuffer_add_printf(evb, "{\"%s\":\"<variable_value>\"}", req->uri);

    evhttp_send_reply(req, HTTP_OK, "OK", evb);
    evbuffer_free(evb);
}

int main() {
    struct event_base *base = event_base_new();
    struct evhttp *http = evhttp_new(base);

    if (evhttp_bind_socket(http, "0.0.0.0", 8080) != 0) {
        fprintf(stderr, "Could not bind to port 8080\n");
        return 1;
    }

    evhttp_set_gencb(http, request_handler, NULL);

    printf("Server started on port 8080\n");
    event_base_dispatch(base);

    evhttp_free(http);
    event_base_free(base);

    return 0;
}

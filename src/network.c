#include "pico/printf.h"

#include "lwip/altcp.h"
#include "lwip/apps/http_client.h"
#include "lwip/netif.h"

#include "network.h"
#include <string.h>

// Internal state handed to lwIP callbacks.
typedef struct {
  char *buf;
  size_t buf_size;
  size_t received;
  volatile bool done;
  volatile bool ok;
} http_req_state_t;

// Called by lwIP when the transfer finishes.
static void result_callback(void *arg, httpc_result_t httpc_result,
                            u32_t rx_content_len, u32_t srv_res, err_t err) {
  http_req_state_t *req = (http_req_state_t *)arg;
  if (httpc_result == HTTPC_RESULT_OK) {
    req->ok = true;
  } else {
    printf("HTTP result: %d (srv_res=%u, err=%d, rx=%lu bytes)\n", httpc_result,
           srv_res, err, (unsigned long)rx_content_len);
    req->ok = false;
  }
  req->done = true;
}

// Called by lwIP for each body chunk received.
static err_t recv_callback(void *arg, struct altcp_pcb *conn, struct pbuf *p,
                           err_t err) {
  http_req_state_t *req = (http_req_state_t *)arg;

  if (p == NULL) {
    // Connection closed by server
    return ERR_OK;
  }

  // Copy as much as fits, drop the rest
  if (req->received < req->buf_size) {
    size_t space = req->buf_size - req->received;
    size_t copy = (p->len < space) ? p->len : space;
    memcpy(req->buf + req->received, p->payload, copy);
    req->received += copy;
  }

  altcp_recved(conn, p->tot_len);
  pbuf_free(p);
  return ERR_OK;
}

bool http_get(const char *hostname, const char *path, char *response_buffer,
              size_t response_size) {
  if (!netif_is_up(netif_list)) {
    printf("Network not up.\n");
    return false;
  }

  httpc_connection_t settings; // zero-initialized
  httpc_state_t *conn = NULL;

  http_req_state_t req = {
      .buf = response_buffer,
      .buf_size = response_size,
      .received = 0,
      .done = false,
      .ok = false,
  };

  memset(response_buffer, 0, response_size);
  memset(&settings, 0, sizeof(settings));
  settings.result_fn = result_callback;
  settings.headers_done_fn = NULL;

  printf("Resolving and connecting to %s:80...\n", hostname);

  // Resolve DNS + Connect + GET in one call
  err_t err = httpc_get_file_dns(hostname, 80, path, &settings, recv_callback,
                                 &req, &conn);
  if (err != ERR_OK) {
    printf("httpc_get_file_dns failed %d\n", err);
    return false;
  }

  // Wait for DNS
  uint32_t waited = 0;
  while (!req.done && waited < 30000) {
    sleep_ms(10);
    waited += 10;
  }

  if (!req.done) {
    printf("HTTP GET timed out after %u ms\n", waited);
    return false;
  }
  if (!req.ok) {
    return false;
  }

  response_buffer[req.received] = '\0';
  return true;
}

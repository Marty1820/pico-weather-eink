#include "lwip/dns.h"
#include "lwip/ip_addr.h"
#include "lwip/tcp.h"
#include "pico/printf.h"

#include "network.h"
#include <string.h>

// Internal state machine
typedef enum {
  STATE_IDLE,
  STATE_DNS_RESOLVING,
  STATE_CONNECTING,
  STATE_SENDING,
  STATE_RECEIVING,
  STATE_DONE,
  STATE_ERROR
} http_state_t;

// Context struct for callbacks
typedef struct {
  char hostname[64];
  char path[256];
} http_context_t;

static http_state_t state = STATE_IDLE;
static struct tcp_pcb *pcb = NULL;
static uint32_t rx_len = 0;
static char *response_buf = NULL;
static size_t response_size = 0;
static bool dns_resolved = false;
static bool dns_failed = false;
static ip_addr_t resolved_addr;
static http_context_t http_ctx;

// DNS callback
static void dns_callback(const char *name, const ip_addr_t *ipaddr,
                         void *callback_arg) {
  if (ipaddr == NULL) {
    printf("DNS resolution failed for %s\n", name);
    dns_failed = true;
    state = STATE_ERROR;
  } else {
    printf("Resolved %s to %s\n", name, ipaddr_ntoa(ipaddr));
    resolved_addr = *ipaddr;
    dns_resolved = true;
    state = STATE_CONNECTING;
  }
}

// TCP recv callback
static err_t recv_callback(void *arg, struct tcp_pcb *tpcb, struct pbuf *p,
                           err_t err) {
  if (p == NULL) {
    tcp_close(tpcb);
    state = STATE_DONE;
    return ERR_OK;
  }
  if (rx_len + p->len < response_size) {
    memcpy(response_buf + rx_len, p->payload, p->len);
    rx_len += p->len;
  }
  tcp_recved(tpcb, p->len);
  pbuf_free(p);
  return ERR_OK;
}

// TCP connected callback
static err_t connected_callback(void *arg, struct tcp_pcb *tpcb, err_t err) {
  if (err != ERR_OK) {
    printf("Connect failed: %d\n", err);
    state = STATE_ERROR;
    return err;
  }
  printf("Connected!\n");
  state = STATE_SENDING;

  // Get context from callback arg
  http_context_t *ctx = (http_context_t *)arg;

  char request[512];
  snprintf(request, sizeof(request),
           "GET %s HTTP/1.1\r\n"
           "Host: %s\r\n"
           "Connection: close\r\n"
           "User-Agent: curl/8.21.0\r\n"
           "\r\n",
           ctx->path, ctx->hostname); // path passed via callback_arg

  printf("Sending GET%s HTTP/1.1 with Host: %s\n", ctx->path, ctx->hostname);

  err_t send_err =
      tcp_write(tpcb, request, strlen(request), TCP_WRITE_FLAG_COPY);
  if (send_err != ERR_OK) {
    state = STATE_ERROR;
    return send_err;
  }
  return ERR_OK;
}

bool http_get(const char *hostname, const char *path, char *response_buffer,
              size_t response_size_arg) {
  if (!netif_is_up(netif_list)) {
    printf("Network not up.\n");
    return false;
  }

  state = STATE_IDLE;
  rx_len = 0;
  dns_resolved = false;
  dns_failed = false;
  response_buf = response_buffer;
  response_size = response_size_arg;
  memset(response_buffer, 0, response_size_arg);

  // Store hostname and path in context
  strncpy(http_ctx.hostname, hostname, sizeof(http_ctx.hostname) - 1);
  strncpy(http_ctx.path, path, sizeof(http_ctx.path) - 1);

  // DNS Resolution
  printf("Resolving %s via DNS...\n", hostname);
  state = STATE_DNS_RESOLVING;

  err_t dns_err =
      dns_gethostbyname(hostname, &resolved_addr, dns_callback, NULL);

  if (dns_err == ERR_OK) {
    printf("DNS cache hit: %s -> %s\n", hostname, ipaddr_ntoa(&resolved_addr));
    dns_resolved = true;
    state = STATE_CONNECTING;
  } else if (dns_err != ERR_INPROGRESS) {
    printf("dns_gethostbyname failed: %d\n", dns_err);
    return false;
  }

  // Wait for DNS
  uint32_t dns_timeout = 0;
  while (state == STATE_DNS_RESOLVING && dns_timeout < 5000) {
    sleep_ms(10);
    dns_timeout += 10;
  }

  if (dns_failed || !dns_resolved) {
    printf("DNS resolution timeout\n");
    return false;
  }

  // TCP Connection
  printf("Connecting to %s (%s)...\n", hostname, ipaddr_ntoa(&resolved_addr));

  pcb = tcp_new();
  if (!pcb) {
    printf("Failed to create PCB\n");
    return false;
  }

  tcp_arg(pcb, (void *)&http_ctx); // Pass context struct
  tcp_recv(pcb, recv_callback);

  err_t err = tcp_connect(pcb, &resolved_addr, 80, connected_callback);
  if (err != ERR_OK) {
    printf("tcp_connect failed: %d\n", err);
    return false;
  }

  // Wait for response
  uint32_t timeout = 0;
  while (state != STATE_DONE && state != STATE_ERROR && timeout < 3000) {
    sleep_ms(10);
    timeout += 10;
  }

  if (state != STATE_DONE) {
    printf("HTTP GET failed (State: %d)\n", state);
    return false;
  }

  response_buffer[rx_len] = '\0';
  return true;
}

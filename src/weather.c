#include "lwip/dns.h"
#include "lwip/ip_addr.h"
#include "lwip/tcp.h"
#include "pico/printf.h"

#include "config/secrets.h"
#include "weather.h"

#include <string.h>

bool weather_fetched = false;
char weather_ascii_data[2048]; // Buffer for cleaned text

typedef enum {
  STATE_IDLE,
  STATE_DNS_RESOLVING,
  STATE_CONNECTING,
  STATE_SENDING,
  STATE_RECEIVING,
  STATE_DONE,
  STATE_ERROR
} http_state_t;

static http_state_t state = STATE_IDLE;
static struct tcp_pcb *pcb = NULL;
static uint32_t rx_len = 0;
static const char *host = "wttr.in";
static char current_path[256];
static ip_addr_t resolved_addr; // Holds resolved IP from DNS
static bool dns_resolved = false;
static bool dns_failed = false;

// DNS Resolution Callback
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

// TCP Receive Callback
static err_t recv_callback(void *arg, struct tcp_pcb *tpcb, struct pbuf *p,
                           err_t err) {
  if (p == NULL) {
    tcp_close(tpcb);
    state = STATE_DONE;
    return ERR_OK;
  }
  if (rx_len + p->len < sizeof(weather_ascii_data)) {
    memcpy(weather_ascii_data + rx_len, p->payload, p->len);
    rx_len += p->len;
  }
  tcp_recved(tpcb, p->len);
  pbuf_free(p);
  return ERR_OK;
}

// TCP Connected Callback
static err_t connected_callback(void *arg, struct tcp_pcb *tpcb, err_t err) {
  if (err != ERR_OK) {
    printf("Connect failed: %d\n", err);
    state = STATE_ERROR;
    return err;
  }
  printf("Connected to server!\n");
  state = STATE_SENDING;

  char request[300];
  snprintf(request, sizeof(request),
           "GET %s HTTP/1.1\r\n"
           "Host: %s\r\n"
           "Connection: close\r\n"
           "User-Agent: curl/8.21.0\r\n" // Pretend to be curl
           "\r\n",
           current_path, host);

  printf("Sending request: %s\n", request); // Debug print

  err_t send_err =
      tcp_write(tpcb, request, strlen(request), TCP_WRITE_FLAG_COPY);
  if (send_err != ERR_OK) {
    state = STATE_ERROR;
    return send_err;
  }
  return ERR_OK;
}

// // Helper to convert string IP to ip_addr_t
// static bool str_to_ip(const char *str, ip_addr_t *ip) {
//   unsigned int a, b, c, d;
//   if (sscanf(str, "%u.%u.%u.%u", &a, &b, &c, &d) != 4)
//     return false;
//   if (a > 255 || b > 255 || c > 255 || d > 255)
//     return false;
//   IP4_ADDR(ip, a, b, c, d);
//   return true;
// }

bool fetch_weather_data(void) {
  if (!netif_is_up(netif_list)) {
    printf("Network not up.\n");
    return false;
  }

  state = STATE_IDLE;
  rx_len = 0;
  dns_resolved = false;
  dns_failed = false;
  memset(weather_ascii_data, 0, sizeof(weather_ascii_data));

  // Build path dynamically
  snprintf(current_path, sizeof(current_path), "/%.4f,%.4f?%s",
           (double)LATITUDE, (double)LONGITUDE, OPTS);

  printf("Constructed path: %s\n", current_path);

  // --- DIRECT IP CONNECTION (Bypass DNS) ---
  // ip_addr_t remote_addr;
  // const char *target_ip_str = "5.9.243.187"; // Hardcoded IP
  // --- DNS RESOLUTION ---
  printf("Resolving %s via DNS...\n", host);
  state = STATE_DNS_RESOLVING;

  err_t dns_err = dns_gethostbyname(host, &resolved_addr, dns_callback, NULL);

  if (dns_err == ERR_OK) {
    // DNS result was cached
    printf("DNS cache hit: %s -> %s\n", host, ipaddr_ntoa(&resolved_addr));
    dns_resolved = true;
    state = STATE_CONNECTING;
  } else if (dns_err != ERR_INPROGRESS) {
    // DNS failed immediately
    printf("dns_gethostbyname failed: %d\n", dns_err);
    state = STATE_ERROR;
    return false;
  }

  // Wait for DNS resolution (with timeout)
  uint32_t dns_timeout = 0;
  while (state == STATE_DNS_RESOLVING && dns_timeout < 5000) {
    sleep_ms(10);
    dns_timeout += 10;
  }

  if (dns_failed || !dns_resolved) {
    printf("DNS resolution timeout or failed\n");
    return false;
  }

  // --- TCP CONNECTION ---
  printf("Connecting to %s (%s)...\n", host, ipaddr_ntoa(&resolved_addr));

  // Create PCB
  pcb = tcp_new();
  if (!pcb) {
    printf("Failed to create PCB\n");
    return false;
  }

  tcp_arg(pcb, NULL);
  tcp_recv(pcb, recv_callback);

  // Connect to Port 80
  err_t err = tcp_connect(pcb, &resolved_addr, 80, connected_callback);

  if (err != ERR_OK) {
    printf("tcp_connect failed: %d\n", err);
    return false;
  }

  // Wait for response
  uint32_t timeout = 0;
  while (state != STATE_DONE && state != STATE_ERROR && timeout < 3000) {
    sleep_ms(10);
    timeout++;
  }

  if (state != STATE_DONE) {
    printf("Fetch failed (State: %d)\n", state);
    return false;
  }

  weather_ascii_data[rx_len] = '\0';

  // Strip HTTP Headers
  char *header_end = strstr(weather_ascii_data, "\r\n\r\n");

  if (header_end) {
    // Move the pointer past the headers
    char *clean_data = header_end + 4;

    // Calc new length
    size_t clean_len = strlen(clean_data);

    // Shift data to beginning of buffer
    memmove(weather_ascii_data, clean_data, clean_len + 1); // +1 for null term
    rx_len = clean_len;

    printf("\n--- WEATHER DATA ---\n%s\n--- END ---\n", weather_ascii_data);
    weather_fetched = true;
    return true;
  } else {
    printf("\n--- RAW RESPONSE ---\n%s\n--- END ---\n", weather_ascii_data);
    return false;
  }
}

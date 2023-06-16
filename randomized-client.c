/*
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the Institute nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * This file is part of the Contiki operating system.
 *
 */


/*
  Notice by Ahmet Cetinkaya - 2023
  Modifications in RandomizedTransmissions project: This code has been
  modified in order to enable transmission of sensor data at random
  time instants.
*/

#include "contiki.h"
#include "net/routing/routing.h"
#include "random.h"
#include "net/netstack.h"
#include "net/ipv6/simple-udp.h"
#include <stdint.h>
#include <inttypes.h>
#include "dev/dht22.h"
#include "dev/button-hal.h"
#include "dev/leds.h"

#include "sys/log.h"
#define LOG_MODULE "App"
#define LOG_LEVEL LOG_LEVEL_INFO

#define WITH_SERVER_REPLY  1
#define UDP_CLIENT_PORT	7888
#define UDP_SERVER_PORT	8777

#define SEND_INTERVAL		  (5 * CLOCK_SECOND)

static struct simple_udp_connection udp_conn;
static uint32_t rx_count = 0;

PROCESS(udp_client_process, "UDP client");
PROCESS(button_press_process, "Buttons");
AUTOSTART_PROCESSES(&udp_client_process, &button_press_process);

static void
udp_rx_callback(struct simple_udp_connection *c,
                const uip_ipaddr_t *sender_addr,
                uint16_t sender_port,
                const uip_ipaddr_t *receiver_addr,
                uint16_t receiver_port,
                const uint8_t *data,
                uint16_t datalen)
{
  rx_count++;
}

static int temp=0;
static int button_count=0;
static int normal=1;
static int sensor_fail_count=0;

PROCESS_THREAD(button_press_process, ev, data) {
  PROCESS_BEGIN();
        leds_toggle(LEDS_BLUE);
        while(1) {
          PROCESS_YIELD();

          if (ev == button_hal_press_event) {
            if (normal == 1) {
              normal = 0;
            } else if (normal == 0) {
              normal = 1;
            }
            leds_toggle(LEDS_BLUE);
            leds_toggle(LEDS_RED);
          }
        }
  PROCESS_END();
}

/*---------------------------------------------------------------------------*/
PROCESS_THREAD(udp_client_process, ev, data)
{
  static struct etimer periodic_timer;
  static char str[32];
  uip_ipaddr_t dest_ipaddr;
  static uint32_t tx_count;
  static uint32_t missed_tx_count;
  int16_t temperature, humidity;

  PROCESS_BEGIN();
  SENSORS_ACTIVATE(dht22);

  simple_udp_register(&udp_conn, UDP_CLIENT_PORT, NULL,
                      UDP_SERVER_PORT, udp_rx_callback);

  etimer_set(&periodic_timer, random_rand() % SEND_INTERVAL);
  while(1) {
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&periodic_timer));

    if(dht22_read_all(&temperature, &humidity) != DHT22_ERROR) {
      temp = temperature;
      printf("TEMPERATURE: %d\n", temp);
      sensor_fail_count = 0;
    } else {
      sensor_fail_count++;
      if (sensor_fail_count >= 5) {
        if (normal == 1) {
           leds_toggle(LEDS_BLUE);
           leds_toggle(LEDS_RED);
        }
        normal = 0;
      }
      printf("Failed to read the sensor\n");
    }
    if (normal == 1) {
    } else {
       temp = 500 + random_rand() % 200;
    }

    if(NETSTACK_ROUTING.node_is_reachable() &&
       NETSTACK_ROUTING.get_root_ipaddr(&dest_ipaddr)) {

      snprintf(str, sizeof(str), "%d", temp);
      simple_udp_sendto(&udp_conn, str, strlen(str), &dest_ipaddr);
      tx_count++;
    } else {
      LOG_INFO("Not reachable yet\n");
      if(tx_count > 0) {
        missed_tx_count++;
      }
    }

    etimer_set(&periodic_timer, 10 + random_rand() % 2 * CLOCK_SECOND);
  }

  PROCESS_END();
}

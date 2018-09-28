#include <stdbool.h>
#include <stdio.h>

#include <ambient_light.h>
#include <humidity.h>
#include <temperature.h>
#include <timer.h>

#include <ieee802154.h>
#include <udp.h>

void print_ipv6(ipv6_addr_t *);

void print_ipv6(ipv6_addr_t *ipv6_addr) {
  for (int j = 0; j < 14; j += 2) {
    printf("%02x%02x:", ipv6_addr->addr[j], ipv6_addr->addr[j + 1]);
  }
  printf("%02x%02x", ipv6_addr->addr[14], ipv6_addr->addr[15]);
}

int main(void) {
  //delay_ms(2000);
  printf("[Sensors] Starting Sensors App.\n");
  printf("[Sensors] All available sensors on the platform will be sampled.\n");

  unsigned int humi = 4;
  int temp = 5;
  int lux  = 6;
  char packet[64];

  ieee802154_set_pan(0xABCD);
  ieee802154_config_commit();
  ieee802154_up();

  ipv6_addr_t ifaces[10];
  udp_list_ifaces(ifaces, 10);

  sock_handle_t handle;
  sock_addr_t addr = {
    ifaces[0],
    12345
  };

  printf("Opening socket on ");
  print_ipv6(&ifaces[0]);
  printf(" : %d\n", addr.port);
  udp_socket(&handle, &addr);

  sock_addr_t destination = {
    ifaces[1],
    23456
  };

  while (1) {
    // Some imixes are unable to read sensors due to hardware issues,
    // so the below code is commented out. Feel free to try it on
    // your imix to see if the sensors work.
    /*
       temperature_read_sync(&temp);
       humidity_read_sync(&humi);
       ambient_light_read_intensity_sync(&lux);
     */
    int len = snprintf(packet, sizeof(packet), "%d deg C; %d%%; %d luxxxxxxx;\n",
                       temp, humi, lux);
    int max_tx_len = udp_get_max_tx_len();
    if (len > max_tx_len) {
      printf("Cannot send packets longer than %d bytes without changing"
             " constants in kernel\n", max_tx_len);
      return 0;
    }
    printf("Sending packet (length %d) --> ", len);
    print_ipv6(&(destination.addr));
    printf(" : %d\n", destination.port);
    ssize_t result = udp_send_to(&handle, packet, len, &destination);

    switch (result) {
      case TOCK_SUCCESS:
        printf("Packet sent.\n\n");
        break;
      default:
        printf("Error sending packet %d\n\n", result);
    }

    delay_ms(4000);
  }
}

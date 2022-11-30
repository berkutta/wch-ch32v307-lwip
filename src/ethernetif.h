#ifndef ETHERNETIF__H
#define ETHERNETIF__H

#include "lwip/err.h"

extern struct netif lwip_netif;

err_t ethernetif_init(struct netif *netif);
void  ethernetif_input(struct netif *netif);

#endif
/*
 * Copyright (c) 2024, sakumisu
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include "usbd_core.h"
#include "usbd_cdc_ecm.h"

#ifndef CONFIG_USBDEV_CDC_ECM_USING_LWIP
#error "Please enable CONFIG_USBDEV_CDC_ECM_USING_LWIP for this demo"
#endif

/*!< endpoint address */
#define CDC_IN_EP  0x81
#define CDC_OUT_EP 0x02
#define CDC_INT_EP 0x83

#define USBD_VID           0xFFFF
#define USBD_PID           0xFFFF
#define USBD_MAX_POWER     100
#define USBD_LANGID_STRING 1033

/*!< config descriptor size */
#define USB_CONFIG_SIZE (9 + CDC_ECM_DESCRIPTOR_LEN)

#ifdef CONFIG_USB_HS
#define CDC_MAX_MPS 512
#else
#define CDC_MAX_MPS 64
#endif

#define CDC_ECM_ETH_STATISTICS_BITMAP 0x00000000

/* str idx = 4 is for mac address: aa:bb:cc:dd:ee:ff*/
#define CDC_ECM_MAC_STRING_INDEX 4

/* Ethernet Maximum Segment size, typically 1514 bytes */
#define CONFIG_CDC_ECM_ETH_MAX_SEGSZE 1514U

#ifdef CONFIG_USBDEV_ADVANCE_DESC
static const uint8_t device_descriptor[] = {
    USB_DEVICE_DESCRIPTOR_INIT(USB_2_0, 0xEF, 0x02, 0x01, USBD_VID, USBD_PID, 0x0100, 0x01)
};

static const uint8_t config_descriptor[] = {
    USB_CONFIG_DESCRIPTOR_INIT(USB_CONFIG_SIZE, 0x02, 0x01, USB_CONFIG_BUS_POWERED, USBD_MAX_POWER),
    CDC_ECM_DESCRIPTOR_INIT(0x00, CDC_INT_EP, CDC_OUT_EP, CDC_IN_EP, CDC_MAX_MPS, CDC_ECM_ETH_STATISTICS_BITMAP, CONFIG_CDC_ECM_ETH_MAX_SEGSZE, 0, 0, CDC_ECM_MAC_STRING_INDEX)
};

static const uint8_t device_quality_descriptor[] = {
    ///////////////////////////////////////
    /// device qualifier descriptor
    ///////////////////////////////////////
    0x0a,
    USB_DESCRIPTOR_TYPE_DEVICE_QUALIFIER,
    0x00,
    0x02,
    0x00,
    0x00,
    0x00,
    0x40,
    0x00,
    0x00,
};

static const char *string_descriptors[] = {
    (const char[]){ 0x09, 0x04 }, /* Langid */
    "CherryUSB",                  /* Manufacturer */
    "CherryUSB CDC ECM DEMO",     /* Product */
    "2022123456",                 /* Serial Number */
};

static const uint8_t *device_descriptor_callback(uint8_t speed)
{
    return device_descriptor;
}

static const uint8_t *config_descriptor_callback(uint8_t speed)
{
    return config_descriptor;
}

static const uint8_t *device_quality_descriptor_callback(uint8_t speed)
{
    return device_quality_descriptor;
}

static const char *string_descriptor_callback(uint8_t speed, uint8_t index)
{
    if (index > 3) {
        return NULL;
    }
    return string_descriptors[index];
}

const struct usb_descriptor cdc_ecm_descriptor = {
    .device_descriptor_callback = device_descriptor_callback,
    .config_descriptor_callback = config_descriptor_callback,
    .device_quality_descriptor_callback = device_quality_descriptor_callback,
    .string_descriptor_callback = string_descriptor_callback
};
#else
/*!< global descriptor */
static const uint8_t cdc_ecm_descriptor[] = {
    USB_DEVICE_DESCRIPTOR_INIT(USB_2_0, 0xEF, 0x02, 0x01, USBD_VID, USBD_PID, 0x0100, 0x01),
    USB_CONFIG_DESCRIPTOR_INIT(USB_CONFIG_SIZE, 0x02, 0x01, USB_CONFIG_BUS_POWERED, USBD_MAX_POWER),
    CDC_ECM_DESCRIPTOR_INIT(0x00, CDC_INT_EP, CDC_OUT_EP, CDC_IN_EP, CDC_MAX_MPS, CDC_ECM_ETH_STATISTICS_BITMAP, CONFIG_CDC_ECM_ETH_MAX_SEGSZE, 0, 0, CDC_ECM_MAC_STRING_INDEX),
    ///////////////////////////////////////
    /// string0 descriptor
    ///////////////////////////////////////
    USB_LANGID_INIT(USBD_LANGID_STRING),
    ///////////////////////////////////////
    /// string1 descriptor
    ///////////////////////////////////////
    0x14,                       /* bLength */
    USB_DESCRIPTOR_TYPE_STRING, /* bDescriptorType */
    'C', 0x00,                  /* wcChar0 */
    'h', 0x00,                  /* wcChar1 */
    'e', 0x00,                  /* wcChar2 */
    'r', 0x00,                  /* wcChar3 */
    'r', 0x00,                  /* wcChar4 */
    'y', 0x00,                  /* wcChar5 */
    'U', 0x00,                  /* wcChar6 */
    'S', 0x00,                  /* wcChar7 */
    'B', 0x00,                  /* wcChar8 */
    ///////////////////////////////////////
    /// string2 descriptor
    ///////////////////////////////////////
    0x2E,                       /* bLength */
    USB_DESCRIPTOR_TYPE_STRING, /* bDescriptorType */
    'C', 0x00,                  /* wcChar0 */
    'h', 0x00,                  /* wcChar1 */
    'e', 0x00,                  /* wcChar2 */
    'r', 0x00,                  /* wcChar3 */
    'r', 0x00,                  /* wcChar4 */
    'y', 0x00,                  /* wcChar5 */
    'U', 0x00,                  /* wcChar6 */
    'S', 0x00,                  /* wcChar7 */
    'B', 0x00,                  /* wcChar8 */
    ' ', 0x00,                  /* wcChar9 */
    'C', 0x00,                  /* wcChar10 */
    'D', 0x00,                  /* wcChar11 */
    'C', 0x00,                  /* wcChar12 */
    ' ', 0x00,                  /* wcChar13 */
    'E', 0x00,                  /* wcChar14 */
    'C', 0x00,                  /* wcChar15 */
    'M', 0x00,                  /* wcChar16 */
    ' ', 0x00,                  /* wcChar17 */
    'D', 0x00,                  /* wcChar18 */
    'E', 0x00,                  /* wcChar19 */
    'M', 0x00,                  /* wcChar20 */
    'O', 0x00,                  /* wcChar21 */
    ///////////////////////////////////////
    /// string3 descriptor
    ///////////////////////////////////////
    0x16,                       /* bLength */
    USB_DESCRIPTOR_TYPE_STRING, /* bDescriptorType */
    '2', 0x00,                  /* wcChar0 */
    '0', 0x00,                  /* wcChar1 */
    '2', 0x00,                  /* wcChar2 */
    '2', 0x00,                  /* wcChar3 */
    '1', 0x00,                  /* wcChar4 */
    '2', 0x00,                  /* wcChar5 */
    '3', 0x00,                  /* wcChar6 */
    '4', 0x00,                  /* wcChar7 */
    '5', 0x00,                  /* wcChar8 */
    '6', 0x00,                  /* wcChar9 */
    ///////////////////////////////////////
    /// string4 descriptor
    ///////////////////////////////////////
    0x1A,                       /* bLength */
    USB_DESCRIPTOR_TYPE_STRING, /* bDescriptorType */
    'a', 0x00,                  /* wcChar0 */
    'a', 0x00,                  /* wcChar1 */
    'b', 0x00,                  /* wcChar2 */
    'b', 0x00,                  /* wcChar3 */
    'c', 0x00,                  /* wcChar4 */
    'c', 0x00,                  /* wcChar5 */
    'd', 0x00,                  /* wcChar6 */
    'd', 0x00,                  /* wcChar7 */
    'e', 0x00,                  /* wcChar8 */
    'e', 0x00,                  /* wcChar9 */
    'f', 0x00,                  /* wcChar10 */
    'f', 0x00,                  /* wcChar11 */
#ifdef CONFIG_USB_HS
    ///////////////////////////////////////
    /// device qualifier descriptor
    ///////////////////////////////////////
    0x0a,
    USB_DESCRIPTOR_TYPE_DEVICE_QUALIFIER,
    0x00,
    0x02,
    0x00,
    0x00,
    0x00,
    0x40,
    0x00,
    0x00,
#endif
    0x00
};
#endif

const uint8_t mac[6] = { 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff };

volatile bool cdc_ecm_tx_done = false;

void usbd_cdc_ecm_data_send_done(uint32_t len)
{
    cdc_ecm_tx_done = true; // suggest you to use semaphore in os
}

#ifdef RT_USING_LWIP

#ifndef RT_LWIP_DHCP
#error cdc_ecm must enable RT_LWIP_DHCP
#endif

#ifndef LWIP_USING_DHCPD
#error cdc_ecm must enable LWIP_USING_DHCPD
#endif

#include <rtthread.h>
#include <rtdevice.h>
#include <netif/ethernetif.h>
#include <dhcp_server.h>

struct eth_device cdc_ecm_dev;

static rt_err_t rt_usbd_cdc_ecm_control(rt_device_t dev, int cmd, void *args)
{
    switch (cmd) {
        case NIOCTL_GADDR:

            /* get mac address */
            if (args) {
                uint8_t *mac_dev = (uint8_t *)args;
                rt_memcpy(mac_dev, mac, 6);
                mac_dev[5] = ~mac_dev[5]; /* device mac can't same as host. */
            } else
                return -RT_ERROR;

            break;

        default:
            break;
    }

    return RT_EOK;
}

struct pbuf *rt_usbd_cdc_ecm_eth_rx(rt_device_t dev)
{
    return usbd_cdc_ecm_eth_rx();
}

rt_err_t rt_usbd_cdc_ecm_eth_tx(rt_device_t dev, struct pbuf *p)
{
    int ret;

    cdc_ecm_tx_done = false;
    ret = usbd_cdc_ecm_eth_tx(p);
    if (ret == 0) {
        while (!cdc_ecm_tx_done) {
        }
        return RT_EOK;
    } else
        return -RT_ERROR;
}

void cdc_ecm_lwip_init(void)
{
    cdc_ecm_dev.parent.control = rt_usbd_cdc_ecm_control;
    cdc_ecm_dev.eth_rx = rt_usbd_cdc_ecm_eth_rx;
    cdc_ecm_dev.eth_tx = rt_usbd_cdc_ecm_eth_tx;

    eth_device_init(&cdc_ecm_dev, "u0");

    eth_device_linkchange(&cdc_ecm_dev, RT_TRUE);
    dhcpd_start("u0");
}

void usbd_cdc_ecm_data_recv_done(uint32_t len)
{
    eth_device_ready(&cdc_ecm_dev);
}

#else
#include "netif/etharp.h"
#include "lwip/init.h"
#include "lwip/netif.h"
#include "lwip/pbuf.h"

#include "dhserver.h"
#include "dnserver.h"

/*Static IP ADDRESS: IP_ADDR0.IP_ADDR1.IP_ADDR2.IP_ADDR3 */
#define IP_ADDR0      (uint8_t)192
#define IP_ADDR1      (uint8_t)168
#define IP_ADDR2      (uint8_t)7
#define IP_ADDR3      (uint8_t)1

/*NETMASK*/
#define NETMASK_ADDR0 (uint8_t)255
#define NETMASK_ADDR1 (uint8_t)255
#define NETMASK_ADDR2 (uint8_t)255
#define NETMASK_ADDR3 (uint8_t)0

/*Gateway Address*/
#define GW_ADDR0      (uint8_t)0
#define GW_ADDR1      (uint8_t)0
#define GW_ADDR2      (uint8_t)0
#define GW_ADDR3      (uint8_t)0

const ip_addr_t ipaddr = IPADDR4_INIT_BYTES(IP_ADDR0, IP_ADDR1, IP_ADDR2, IP_ADDR3);
const ip_addr_t netmask = IPADDR4_INIT_BYTES(NETMASK_ADDR0, NETMASK_ADDR1, NETMASK_ADDR2, NETMASK_ADDR3);
const ip_addr_t gateway = IPADDR4_INIT_BYTES(GW_ADDR0, GW_ADDR1, GW_ADDR2, GW_ADDR3);

#define NUM_DHCP_ENTRY 3

static dhcp_entry_t entries[NUM_DHCP_ENTRY] = {
    /* mac    ip address        subnet mask        lease time */
    { { 0 }, { 192, 168, 7, 2 }, { 255, 255, 255, 0 }, 24 * 60 * 60 },
    { { 0 }, { 192, 168, 7, 3 }, { 255, 255, 255, 0 }, 24 * 60 * 60 },
    { { 0 }, { 192, 168, 7, 4 }, { 255, 255, 255, 0 }, 24 * 60 * 60 }
};

static dhcp_config_t dhcp_config = {
    { 192, 168, 7, 1 }, /* server address */
    67,                 /* port */
    { 192, 168, 7, 1 }, /* dns server */
    "cherry",           /* dns suffix */
    NUM_DHCP_ENTRY,     /* num entry */
    entries             /* entries */
};

static bool dns_query_proc(const char *name, ip_addr_t *addr)
{
    if (strcmp(name, "cdc_ecm.cherry") == 0 || strcmp(name, "www.cdc_ecm.cherry") == 0) {
        addr->addr = ipaddr.addr;
        return true;
    }
    return false;
}

static struct netif cdc_ecm_netif; //network interface

/* Network interface name */
#define IFNAME0        'E'
#define IFNAME1        'X'

err_t linkoutput_fn(struct netif *netif, struct pbuf *p)
{
    int ret;

    cdc_ecm_tx_done = false;
    ret = usbd_cdc_ecm_eth_tx(p);
    if (ret == 0) {
        while (!cdc_ecm_tx_done) {
        }
        return ERR_OK;
    } else
        return ERR_BUF;
}

err_t cdc_ecm_if_init(struct netif *netif)
{
    LWIP_ASSERT("netif != NULL", (netif != NULL));

    netif->mtu = 1500;
    netif->flags = NETIF_FLAG_BROADCAST | NETIF_FLAG_ETHARP | NETIF_FLAG_LINK_UP | NETIF_FLAG_UP;
    netif->state = NULL;
    netif->name[0] = IFNAME0;
    netif->name[1] = IFNAME1;
    netif->output = etharp_output;
    netif->linkoutput = linkoutput_fn;
    return ERR_OK;
}

err_t cdc_ecm_if_input(struct netif *netif)
{
    err_t err;
    struct pbuf *p;

    p = usbd_cdc_ecm_eth_rx();
    if (p != NULL) {
        err = netif->input(p, netif);
        if (err != ERR_OK) {
            pbuf_free(p);
        }
    } else {
        return ERR_BUF;
    }
    return err;
}

void cdc_ecm_lwip_init(void)
{
    struct netif *netif = &cdc_ecm_netif;

    lwip_init();

    netif->hwaddr_len = 6;
    memcpy(netif->hwaddr, mac, 6);
    netif->hwaddr[5] = ~netif->hwaddr[5]; /* device mac can't same as host. */

    netif = netif_add(netif, &ipaddr, &netmask, &gateway, NULL, cdc_ecm_if_init, netif_input);
    netif_set_default(netif);
    while (!netif_is_up(netif)) {
    }

    while (dhserv_init(&dhcp_config)) {
    }

    while (dnserv_init(IP_ADDR_ANY, 53, dns_query_proc)) {
    }
}

void usbd_cdc_ecm_data_recv_done(uint32_t len)
{
}

void cdc_ecm_input_poll(void)
{
    cdc_ecm_if_input(&cdc_ecm_netif);
}
#endif

static void usbd_event_handler(uint8_t busid, uint8_t event)
{
    switch (event) {
        case USBD_EVENT_RESET:
            break;
        case USBD_EVENT_CONNECTED:
            break;
        case USBD_EVENT_DISCONNECTED:
            break;
        case USBD_EVENT_RESUME:
            break;
        case USBD_EVENT_SUSPEND:
            break;
        case USBD_EVENT_CONFIGURED:
            break;
        case USBD_EVENT_SET_REMOTE_WAKEUP:
            break;
        case USBD_EVENT_CLR_REMOTE_WAKEUP:
            break;

        default:
            break;
    }
}

struct usbd_interface intf0;
struct usbd_interface intf1;

/* ecm only supports in linux, and you should input the following command
 *
 * sudo ifconfig enxaabbccddeeff up
 * sudo dhcpclient enxaabbccddeeff
*/
void cdc_ecm_init(uint8_t busid, uintptr_t reg_base)
{
    cdc_ecm_lwip_init();

#ifdef CONFIG_USBDEV_ADVANCE_DESC
    usbd_desc_register(busid, &cdc_ecm_descriptor);
#else
    usbd_desc_register(busid, cdc_ecm_descriptor);
#endif
    usbd_add_interface(busid, usbd_cdc_ecm_init_intf(&intf0, CDC_INT_EP, CDC_OUT_EP, CDC_IN_EP));
    usbd_add_interface(busid, usbd_cdc_ecm_init_intf(&intf1, CDC_INT_EP, CDC_OUT_EP, CDC_IN_EP));
    usbd_initialize(busid, reg_base, usbd_event_handler);
}

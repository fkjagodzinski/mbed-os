/* mbed Microcontroller Library
 * Copyright (c) 2018-2018 ARM Limited
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef USBPHYHW_H
#define USBPHYHW_H

#include "mbed.h"
#include "USBPhy.h"
#include "USBEndpoints_STM32.h"
#include "PeripheralPins.h"

#if defined(TARGET_DISCO_F746NG)
#if (MBED_CONF_TARGET_USB_SPEED == 1) // Defined in json configuration file
#define TARGET_DISCO_F746NG_OTG_HS
#else
#define TARGET_DISCO_F746NG_OTG_FS
#endif
#endif

#if !defined(MBED_CONF_TARGET_USB_SPEED)
#if defined (USB)
#define MBED_CONF_TARGET_USB_SPEED USB_NO_OTG
#elif defined(USB_OTG_FS)
#define MBED_CONF_TARGET_USB_SPEED USB_FS
#else
#define MBED_CONF_TARGET_USB_SPEED USB_HS
#endif
#endif /* !defined(MBED_CONF_TARGET_USB_SPEED) */

#if (MBED_CONF_TARGET_USB_SPEED == USB_NO_OTG)
#define USBHAL_IRQn  USB_IRQn
#elif (MBED_CONF_TARGET_USB_SPEED == USB_FS)
#define USBHAL_IRQn  OTG_FS_IRQn
#else
#define USBHAL_IRQn  OTG_HS_IRQn
#endif

class USBPhyHw : public USBPhy {
public:
    USBPhyHw();
    virtual ~USBPhyHw();
    virtual void init(USBPhyEvents *events);
    virtual void deinit();
    virtual bool powered();
    virtual void connect();
    virtual void disconnect();
    virtual void configure();
    virtual void unconfigure();
    virtual void sof_enable();
    virtual void sof_disable();
    virtual void set_address(uint8_t address);
    virtual void remote_wakeup();
    virtual const usb_ep_table_t *endpoint_table();

    virtual uint32_t ep0_set_max_packet(uint32_t max_packet);
    virtual void ep0_setup_read_result(uint8_t *buffer, uint32_t size);
    virtual void ep0_read(uint8_t *data, uint32_t size);
    virtual uint32_t ep0_read_result();
    virtual void ep0_write(uint8_t *buffer, uint32_t size);
    virtual void ep0_stall();

    virtual bool endpoint_add(usb_ep_t endpoint, uint32_t max_packet, usb_ep_type_t type);
    virtual void endpoint_remove(usb_ep_t endpoint);
    virtual void endpoint_stall(usb_ep_t endpoint);
    virtual void endpoint_unstall(usb_ep_t endpoint);

    virtual bool endpoint_read(usb_ep_t endpoint, uint8_t *data, uint32_t size);
    virtual uint32_t endpoint_read_result(usb_ep_t endpoint);
    virtual bool endpoint_write(usb_ep_t endpoint, uint8_t *data, uint32_t size);
    virtual void endpoint_abort(usb_ep_t endpoint);

    virtual void process();

    USBPhyEvents *events;
    bool sof_enabled;

    uint8_t epComplete[NUMBER_OF_PHYSICAL_ENDPOINTS];
    PCD_HandleTypeDef hpcd;

private:

    static void _usbisr(void);
};

#endif

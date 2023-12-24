#!/bin/sh

while [ "$(/usr/bin/getprop vendor.service.nvram_init)" != "Ready" ]; do
    sleep 1
done

hexdump -s 0 -n 6 -ve '/1 "%02X:"' /mnt/vendor/nvdata/APCFG/APRDEB/BT_Addr | sed "s/:$//g" > /var/lib/bluetooth/board-address


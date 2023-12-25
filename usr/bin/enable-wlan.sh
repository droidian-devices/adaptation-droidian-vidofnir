#!/bin/bash

while [ "`getprop vendor.connsys.driver.ready`" != "yes" ]; do
        sleep 1
done

sleep 10

rmmod wlan_drv_gen4m_6789
rmmod wmt_chrdev_wifi

cd /lib/modules/$(uname -r)

insmod wmt_chrdev_wifi.ko
insmod wlan_drv_gen4m_6789.ko

echo 0 > /dev/wmtWifi
echo 1 > /dev/wmtWifi

exit 0

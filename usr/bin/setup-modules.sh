#!/bin/bash

[ -d "/lib/modules/$(uname -r)" ] && exit 0

mkdir -p /lib/modules/$(uname -r)/
cp /vendor/lib/modules/* /lib/modules/$(uname -r)/

cp /usr/lib/adaptation-droidian-vidofnir/modules.load /lib/modules/$(uname -r)/

depmod -a

systemctl restart systemd-modules-load

exit 0

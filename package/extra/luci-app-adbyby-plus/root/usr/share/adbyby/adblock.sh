#!/bin/sh

wget-ssl --no-check-certificate -O /tmp/adnew.conf 'https://easylist-downloads.adblockplus.org/easylistchina+easylist.txt' &&
  /usr/share/adbyby/ad-update

rm -f /tmp/adbyby.updated
sleep 10
/etc/init.d/adbyby restart

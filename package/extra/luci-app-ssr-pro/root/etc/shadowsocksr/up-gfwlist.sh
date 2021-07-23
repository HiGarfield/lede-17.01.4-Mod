#!/bin/sh

mkdir -p /var/lock
lock -n /var/lock/up-gfwlist.lck || exit 0

/etc/shadowsocksr/gen-gfwlist.sh >/tmp/ol-gfw.txt

if [ -s "/tmp/ol-gfw.txt" ]; then
	sort -u /etc/shadowsocksr/base-gfwlist.txt /tmp/ol-gfw.txt >/tmp/china-banned
else
	sort -u /etc/shadowsocksr/base-gfwlist.txt /etc/gfwlist/china-banned >/tmp/china-banned
fi

if (! cmp -s /tmp/china-banned /etc/gfwlist/china-banned); then
	if [ -s "/tmp/china-banned" ]; then
		mv -f /tmp/china-banned /etc/gfwlist/china-banned
		echo "Update GFW-List Done!"
	fi
else
	echo "GFW-List No Change!"
fi

rm -f /tmp/ol-gfw.txt
/etc/init.d/ssrpro restart

lock -u /var/lock/up-gfwlist.lck

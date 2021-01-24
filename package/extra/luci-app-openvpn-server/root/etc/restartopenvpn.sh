#/bin/sh

mkdir -p /var/lock
lock /var/lock/openvpn.lock

if [ ! -f "/etc/openvpn/ca.crt" ] ||
	[ ! -f "/etc/openvpn/ca.key" ] ||
	[ ! -f "/etc/openvpn/server.crt" ] ||
	[ ! -f "/etc/openvpn/server.key" ] ||
	[ ! -f "/etc/openvpn/dh1024.pem" ] ||
	[ ! -f "/etc/openvpn/client1.crt" ] ||
	[ ! -f "/etc/openvpn/client1.key" ]; then
	/etc/openvpncert.sh >/dev/null 2>&1
fi

/etc/init.d/openvpn restart
lock -u /var/lock/openvpn.lock

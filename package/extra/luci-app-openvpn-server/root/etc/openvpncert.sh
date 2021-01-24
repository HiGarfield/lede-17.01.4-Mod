#!/bin/sh

function rand_str() {
	(base64 /dev/urandom | tr -dc 'A-Za-z' | head -c $1) 2>/dev/null
}

function rand_str_upper() {
	(rand_str $1 | tr 'a-z' 'A-Z') 2>/dev/null
}

function rand_str_lower() {
	(rand_str $1 | tr 'A-Z' 'a-z') 2>/dev/null
}

function rand_easy_rsa_vars() {
	local KEY_PROVINCE="$(rand_str_upper 6)"
	local KEY_CITY="$(rand_str 8)"
	local KEY_ORG="$(rand_str 8)"
	local KEY_EMAIL="$(rand_str_lower 8)@$(rand_str_lower 4).$(rand_str_lower 3)"
	local KEY_OU="$(rand_str 8)"
	sed -i \
		-e "s/^[[:space:]]*export[[:space:]][[:space:]]*KEY_PROVINCE=.*$/export KEY_PROVINCE=\"$KEY_PROVINCE\"/" \
		-e "s/^[[:space:]]*export[[:space:]][[:space:]]*KEY_CITY=.*$/export KEY_CITY=\"$KEY_CITY\"/" \
		-e "s/^[[:space:]]*export[[:space:]][[:space:]]*KEY_ORG=.*$/export KEY_ORG=\"$KEY_ORG\"/" \
		-e "s/^[[:space:]]*export[[:space:]][[:space:]]*KEY_EMAIL=.*$/export KEY_EMAIL=\"$KEY_EMAIL\"/" \
		-e "s/^[[:space:]]*export[[:space:]][[:space:]]*KEY_OU=.*$/export KEY_OU=\"$KEY_OU\"/" \
		-e '/^[[:space:]]*$/d' \
		/etc/easy-rsa/vars
}

rand_easy_rsa_vars

. /etc/easy-rsa/vars

clean-all
pkitool --initca
build-dh
pkitool --server server
pkitool client1
openvpn --genkey --secret ta.key
cp -f /etc/easy-rsa/keys/ca.crt /etc/openvpn/
cp -f /etc/easy-rsa/keys/ca.key /etc/openvpn/
cp -f /etc/easy-rsa/keys/server.crt /etc/openvpn/
cp -f /etc/easy-rsa/keys/server.key /etc/openvpn/
cp -f /etc/easy-rsa/keys/dh1024.pem /etc/openvpn/
cp -f /etc/easy-rsa/keys/client1.crt /etc/openvpn/
cp -f /etc/easy-rsa/keys/client1.key /etc/openvpn/

echo "OpenVPN Cert renew successfully"

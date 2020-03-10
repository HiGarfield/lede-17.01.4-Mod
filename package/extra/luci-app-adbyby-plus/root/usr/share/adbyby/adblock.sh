#!/bin/sh
	wan_mode=`uci get adbyby.@adbyby[0].wan_mode 2>/dev/null`
	if [ $wan_mode -eq 1 ]; then
		rm -f /tmp/dnsmasq.adblock
		wget-ssl --no-check-certificate -O- https://easylist-downloads.adblockplus.org/easylistchina+easylist.txt | grep ^\|\|[^\*]*\^$ | sed -e 's:||:address\=\/:' -e 's:\^:/0\.0\.0\.0:' > /tmp/dnsmasq.adblock
		if [ -s "/tmp/dnsmasq.adblock" ];then
			sed -i '/youku.com/d' /tmp/dnsmasq.adblock
			if ( ! cmp -s /tmp/dnsmasq.adblock /usr/share/adbyby/dnsmasq.adblock );then
				mv /tmp/dnsmasq.adblock /usr/share/adbyby/dnsmasq.adblock
				/usr/share/adbyby/adupdate.sh restartdnsmasq
			else
				rm -f /tmp/dnsmasq.adblock
				/usr/share/adbyby/adupdate.sh
			fi
		fi
	else
		/usr/share/adbyby/adupdate.sh
	fi

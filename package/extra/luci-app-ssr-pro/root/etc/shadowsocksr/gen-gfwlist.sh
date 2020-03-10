#!/bin/sh

generate_china_banned()
{
	local GFWLIST_URL
	GFWLIST_URL=$(uci -q get ssrpro.@ssrpro[0].gfwlist_url)
	[ -z "$GFWLIST_URL" ] && GFWLIST_URL="https://raw.githubusercontent.com/gfwlist/gfwlist/master/gfwlist.txt"

	wget-ssl --no-check-certificate "$GFWLIST_URL" -O /tmp/gfwlist.b64 >&2

	if [ $? -ne 0 ] || [ ! -s /tmp/gfwlist.b64 ]; then
		rm -f /tmp/gfwlist.b64 >&2
		exit 1
	fi

	wget-ssl --no-check-certificate "$GFWLIST_URL" -O /tmp/gfwlist2.b64 >&2
	if [ $? -ne 0 ] || [ ! -s /tmp/gfwlist2.b64 ]; then
		rm -f /tmp/gfwlist2.b64 >&2
		exit 1
	fi

	if ( ! cmp -s /tmp/gfwlist.b64 /tmp/gfwlist2.b64 ); then
		rm -f /tmp/gfwlist.b64 >&2
		rm -f /tmp/gfwlist2.b64 >&2
		exit 1
	fi
	rm -f /tmp/gfwlist2.b64 >&2

	base64 -d /tmp/gfwlist.b64 | sort -u |
		sed 's#!.\+##; s#|##g; s#@##g; s#https\?:\/\/##;' |
		sed '/\*/d; /apple\.com/d; /sina\.cn/d; /sina\.com\.cn/d; /baidu\.com/d; /byr\.cn/d; /jlike\.com/d; /weibo\.com/d; /zhongsou\.com/d; /youdao\.com/d; /sogou\.com/d; /so\.com/d; /soso\.com/d; /aliyun\.com/d; /taobao\.com/d; /jd\.com/d; /qq\.com/d' |
		sed '/^[0-9]\+\.[0-9]\+\.[0-9]\+\.[0-9]\+$/d' |
		grep '^[0-9a-zA-Z\.-]\+$' | grep '\.' | sed 's#^\.\+##' | sort -u |
		awk '
BEGIN { prev = "________"; }  {
	cur = $0;
	if (index(cur, prev) == 1 && substr(cur, 1 + length(prev) ,1) == ".") {
	} else {
		print cur;
		prev = cur;
	}
}' | sort -u	
	rm -f /tmp/gfwlist.b64 >&2
}

generate_china_banned

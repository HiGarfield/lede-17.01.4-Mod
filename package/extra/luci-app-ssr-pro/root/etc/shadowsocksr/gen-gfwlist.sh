#!/bin/sh

generate_china_banned() {
	local GFWLIST_URL=$(uci -q get ssrpro.@ssrpro[0].gfwlist_url 2>/dev/null)
	[ -z "$GFWLIST_URL" ] &&
		GFWLIST_URL="https://raw.githubusercontent.com/gfwlist/gfwlist/master/gfwlist.txt"

	wget-ssl -q --no-check-certificate -O- "$GFWLIST_URL" |
		base64 -d |
		sed -n '{
			s/^||\?//g;
			s/https\?:\/\///g;
			s/^\.//g;
			s/\*.*//g;
			s/\/.*//g;
			s/%.*//g;
			s/:[0-9]\+$//g;
			/^$/d;
			/^!.*/d;
			/^@.*/d;
			/^\[.*/d;
			/\.$/d;
			/^[^\.]*$/d;
			/^[0-9\.]*$/d;
			/\*/d;
			/:/d;
			/aliyun\.com/d;
			/apple\.com/d;
			/baidu\.com/d;
			/bing\.com/d;
			/chinaso\.com/d;
			/chinaz\.com/d;
			/cn\.gravatar\.com/d;
			/haosou\.com/d;
			/ip\.cn/d;
			/jd\.com/d;
			/jike\.com/d;
			/gov\.cn/d;
			/qq\.com/d;
			/sina\.cn/d;
			/sina\.com\.cn/d;
			/sogou\.com/d;
			/so\.com/d;
			/soso\.com/d;
			/taobao\.com/d;
			/tencent\.com/d;
			/uluai\.com\.cn/d;
			/weibo\.com/d;
			/yahoo\.cn/d;
			/youdao\.com/d;
			/zhongsou\.com/d;
			p
		}' |
		sort -u
}

generate_china_banned

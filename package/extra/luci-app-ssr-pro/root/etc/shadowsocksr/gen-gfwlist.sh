#!/bin/sh

DEFAULT_GFWLIST_URL='https://raw.githubusercontent.com/gfwlist/gfwlist/master/gfwlist.txt'
GFWLIST_URL="$(uci -q get ssrpro.@ssrpro[0].gfwlist_url)"
[ -n "$GFWLIST_URL" ] || GFWLIST_URL="$DEFAULT_GFWLIST_URL"

fetch_gfwlist() {
	first_line="$(wget-ssl -qO- "$GFWLIST_URL" 2>/dev/null | sed -n '1p')"
	case "$first_line" in
		\[*|!*)
			wget-ssl -qO- "$GFWLIST_URL" 2>/dev/null
		;;
		*)
			wget-ssl -qO- "$GFWLIST_URL" 2>/dev/null | base64 -d 2>/dev/null
		;;
	esac
}

extract_domains() {
	awk '
	function emit(h, n, a, i) {
		h = tolower(h)
		gsub(/\r/, "", h)
		gsub(/\\\./, ".", h)
		sub(/^[*.]+/, "", h)
		sub(/\.+$/, "", h)
		if (h == "" || h ~ /^[0-9.]+$/) return
		n = split(h, a, ".")
		if (n < 2) return
		for (i = 1; i <= n; i++) {
			if (a[i] == "" || a[i] ~ /[^a-z0-9-]/ || a[i] ~ /^-/ || a[i] ~ /-$/) return
		}
		print h
	}
	{
		gsub(/\r/, "", $0)
		if ($0 == "" || $0 ~ /^!/ || $0 ~ /^\[/ || $0 ~ /^@@/ || $0 ~ /^\/.*\/$/) next
		s = $0
		sub(/^\|\|/, "", s)
		sub(/^\|/, "", s)
		sub(/^[a-zA-Z]+:\/\//, "", s)
		if (match(s, /([A-Za-z0-9-]+\.)+[A-Za-z0-9-]+/))
			emit(substr(s, RSTART, RLENGTH))
	}'
}

print_extra_domains() {
	cat <<'EOF'
1e100.net
abc.xyz
android.com
app-measurement.com
appspot.com
blogblog.com
blogger.com
blogspot.ae
blogspot.al
blogspot.am
blogspot.ba
blogspot.be
blogspot.bg
blogspot.bj
blogspot.ca
blogspot.cf
blogspot.ch
blogspot.cl
blogspot.co.at
blogspot.co.id
blogspot.co.il
blogspot.co.ke
blogspot.co.nz
blogspot.co.uk
blogspot.co.za
blogspot.com
blogspot.com.ar
blogspot.com.au
blogspot.com.br
blogspot.com.by
blogspot.com.co
blogspot.com.cy
blogspot.com.ee
blogspot.com.eg
blogspot.com.es
blogspot.com.mt
blogspot.com.ng
blogspot.com.tr
blogspot.com.uy
blogspot.cv
blogspot.cz
blogspot.de
blogspot.dk
blogspot.fi
blogspot.fr
blogspot.gr
blogspot.hk
blogspot.hr
blogspot.hu
blogspot.ie
blogspot.in
blogspot.is
blogspot.it
blogspot.jp
blogspot.kr
blogspot.li
blogspot.lt
blogspot.lu
blogspot.md
blogspot.mk
blogspot.mr
blogspot.mx
blogspot.my
blogspot.nl
blogspot.no
blogspot.pe
blogspot.pt
blogspot.qa
blogspot.re
blogspot.ro
blogspot.rs
blogspot.ru
blogspot.se
blogspot.sg
blogspot.si
blogspot.sk
blogspot.sn
blogspot.td
blogspot.tw
blogspot.ug
chrome.com
chromium.org
doubleclick.net
firebase.com
firebaseio.com
g.co
ggpht.com
gmail.com
goo.gl
google-analytics.com
google.com
google.com
google.ad
google.ae
google.com.af
google.com.ag
google.al
google.am
google.co.ao
google.com.ar
google.as
google.at
google.com.au
google.az
google.ba
google.com.bd
google.be
google.bf
google.bg
google.com.bh
google.bi
google.bj
google.com.bn
google.com.bo
google.com.br
google.bs
google.bt
google.co.bw
google.by
google.com.bz
google.ca
google.cd
google.cf
google.cg
google.ch
google.ci
google.co.ck
google.cl
google.cm
google.cn
google.com.co
google.co.cr
google.com.cu
google.cv
google.com.cy
google.cz
google.de
google.dj
google.dk
google.dm
google.com.do
google.dz
google.com.ec
google.ee
google.com.eg
google.es
google.com.et
google.fi
google.com.fj
google.fm
google.fr
google.ga
google.ge
google.gg
google.com.gh
google.com.gi
google.gl
google.gm
google.gr
google.com.gt
google.gy
google.com.hk
google.hn
google.hr
google.ht
google.hu
google.co.id
google.ie
google.co.il
google.im
google.co.in
google.iq
google.is
google.it
google.je
google.com.jm
google.jo
google.co.jp
google.co.ke
google.com.kh
google.ki
google.kg
google.co.kr
google.com.kw
google.kz
google.la
google.com.lb
google.li
google.lk
google.co.ls
google.lt
google.lu
google.lv
google.com.ly
google.co.ma
google.md
google.me
google.mg
google.mk
google.ml
google.com.mm
google.mn
google.com.mt
google.mu
google.mv
google.mw
google.com.mx
google.com.my
google.co.mz
google.com.na
google.com.ng
google.com.ni
google.ne
google.nl
google.no
google.com.np
google.nr
google.nu
google.co.nz
google.com.om
google.com.pa
google.com.pe
google.com.pg
google.com.ph
google.com.pk
google.pl
google.pn
google.com.pr
google.ps
google.pt
google.com.py
google.com.qa
google.ro
google.ru
google.rw
google.com.sa
google.com.sb
google.sc
google.se
google.com.sg
google.sh
google.si
google.sk
google.com.sl
google.sn
google.so
google.sm
google.sr
google.st
google.com.sv
google.td
google.tg
google.co.th
google.com.tj
google.tl
google.tm
google.tn
google.to
google.com.tr
google.tt
google.com.tw
google.co.tz
google.com.ua
google.co.ug
google.co.uk
google.com.uy
google.co.uz
google.com.vc
google.co.ve
google.co.vi
google.com.vn
google.vu
google.ws
google.rs
google.co.za
google.co.zm
google.co.zw
google.cat
googleadservices.com
googleapis.cn
googleapis.com
googledrive.com
googlemail.com
googleoptimize.com
googlesyndication.com
googletagmanager.com
googletagservices.com
googletraveladservices.com
googleusercontent.com
googlevideo.com
googleweblight.com
googlezip.net
gstatic.com
gvt1.com
gvt2.com
gvt3.com
recaptcha.net
waymo.com
widevine.com
withgoogle.com
youtu.be
youtube-nocookie.com
youtube.com
ytimg.com
EOF
}

{
	fetch_gfwlist | extract_domains
	print_extra_domains
} | awk '
{
	d = tolower($0)
	gsub(/\r/, "", d)
	sub(/^[*.]+/, "", d)
	sub(/\.+$/, "", d)
	if (d != "" && d ~ /^([a-z0-9-]+\.)+[a-z0-9-]+$/)
		print d
}' | sort -u
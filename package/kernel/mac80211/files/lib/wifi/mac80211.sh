#!/bin/sh
. /lib/netifd/mac80211.sh

append DRIVERS "mac80211"

lookup_phy() {
	[ -n "$phy" ] && {
		[ -d "/sys/class/ieee80211/$phy" ] && return
	}

	local devpath
	config_get devpath "$device" path
	[ -n "$devpath" ] && {
		phy="$(mac80211_path_to_phy "$devpath")"
		[ -n "$phy" ] && return
	}

	local macaddr="$(config_get "$device" macaddr | tr 'A-Z' 'a-z')"
	[ -n "$macaddr" ] && {
		for _phy in /sys/class/ieee80211/*; do
			[ -e "$_phy" ] || continue

			[ "$macaddr" = "$(cat "${_phy}/macaddress")" ] || continue
			phy="${_phy##*/}"
			return
		done
	}
	phy=
	return
}

find_mac80211_phy() {
	local device="$1"

	config_get phy "$device" phy
	lookup_phy
	[ -n "$phy" ] && [ -d "/sys/class/ieee80211/$phy" ] || {
		echo "PHY for wifi device $1 not found"
		return 1
	}
	config_set "$device" phy "$phy"

	config_get macaddr "$device" macaddr
	[ -z "$macaddr" ] && {
		config_set "$device" macaddr "$(cat "/sys/class/ieee80211/${phy}/macaddress")"
	}

	return 0
}

check_mac80211_device() {
	config_get phy "$1" phy
	[ -z "$phy" ] && {
		find_mac80211_phy "$1" >/dev/null || return 0
		config_get phy "$1" phy
	}
	[ "$phy" = "$dev" ] && found=1
}

check_devidx() {
	case "$1" in
	radio[0-9]*)
		local idx="${1#radio}"
		[ "$devidx" -ge "${1#radio}" ] && devidx=$((idx + 1))
		;;
	esac
}

detect_mac80211() {
	devidx=0
	config_load wireless
	config_foreach check_devidx wifi-device

	# A dual band chip such as the MT7615D registers one phy that covers
	# both bands and a second phy that handles 5 GHz only.  The 2.4 GHz
	# band is only reachable through the first one, so that phy has to be
	# set up as 2.4 GHz.  Leaving it on 5 GHz would put both radios on
	# 5 GHz and leave 2.4 GHz unused.
	local has_5g_only_phy=0
	for _scan_dev in /sys/class/ieee80211/*; do
		[ -e "$_scan_dev" ] || continue
		_scan_phy="${_scan_dev##*/}"
		_scan_info="$(iw phy "$_scan_phy" info 2>/dev/null)"
		echo "$_scan_info" | grep -q '2412' && continue
		echo "$_scan_info" | grep -q '5[[:digit:]]\{3\}[[:space:]]\+MHz' && has_5g_only_phy=1
	done

	for _dev in /sys/class/ieee80211/*; do
		[ -e "$_dev" ] || continue

		dev="${_dev##*/}"

		found=0
		config_foreach check_mac80211_device wifi-device
		[ "$found" -gt 0 ] && continue

		mode_band="g"
		channel="11"
		htmode=
		ht_capab=
		local wifi_5ghz=

		iw phy "$dev" info | grep -q 'Capabilities:' && htmode="HT40"

		local detected_channel_80211a=$(iw phy "$dev" info | grep '\*[[:space:]]\+5[[:digit:]]\{3\}[[:space:]]\+MHz[[:space:]]\+\[' | grep -v '(disabled)' -m 1 | sed 's/[^[]*\[\|\].*//g')

		# This phy offers both bands while another phy covers 5 GHz only,
		# so it is the 2.4 GHz side of a dual band chip.  Stay on 2.4 GHz
		# even though 5 GHz channels are available here as well.
		if [ "$has_5g_only_phy" -eq 1 ] &&
		   iw phy "$dev" info | grep -q '2412'
		then
			detected_channel_80211a=""
		fi

		echo "$detected_channel_80211a" | grep -q "^[[:digit:]]\+$" && {
			mode_band="a"
			channel="$detected_channel_80211a"
			iw phy "$dev" info | grep -q 'VHT Capabilities' && htmode="VHT80"
			wifi_5ghz="_5G"
		}

		[ -n "$htmode" ] && ht_capab="set wireless.radio${devidx}.htmode=${htmode}"

		local ht40_noscan=
		[ "$htmode" = "HT40" ] && ht40_noscan="set wireless.radio${devidx}.noscan=1"

		path="$(mac80211_phy_to_path "$dev")"
		if [ -n "$path" ]; then
			dev_id="set wireless.radio${devidx}.path='$path'"
		else
			dev_id="set wireless.radio${devidx}.macaddr=$(cat "/sys/class/ieee80211/${dev}/macaddress")"
		fi

		local mac_addr="$(awk -F ':' '{print toupper($5$6)}' "/sys/class/ieee80211/${dev}/macaddress")"
		local ssid="LEDE_${mac_addr}${wifi_5ghz}"

		uci -q batch <<-EOF
			set wireless.radio${devidx}=wifi-device
			set wireless.radio${devidx}.type=mac80211
			set wireless.radio${devidx}.channel=${channel}
			set wireless.radio${devidx}.hwmode=11${mode_band}
			${dev_id}
			${ht_capab}
			${ht40_noscan}
			set wireless.radio${devidx}.country=CN

			set wireless.default_radio${devidx}=wifi-iface
			set wireless.default_radio${devidx}.device=radio${devidx}
			set wireless.default_radio${devidx}.network=lan
			set wireless.default_radio${devidx}.mode=ap
			set wireless.default_radio${devidx}.ssid=${ssid}
			set wireless.default_radio${devidx}.encryption=none

			commit wireless
		EOF

		devidx=$((devidx + 1))
	done
}

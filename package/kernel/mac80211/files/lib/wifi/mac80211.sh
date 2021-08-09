#!/bin/sh
append DRIVERS "mac80211"

lookup_phy() {
	[ -n "$phy" ] && {
		[ -d "/sys/class/ieee80211/$phy" ] && return
	}

	local devpath
	config_get devpath "$device" path
	[ -n "$devpath" ] && {
		for phy in $(ls /sys/class/ieee80211 2>/dev/null); do
			case "$(readlink -f "/sys/class/ieee80211/$phy/device")" in
			*$devpath) return ;;
			esac
		done
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

detect_mac80211() {
	devidx=0
	config_load wireless
	while :; do
		config_get type "radio$devidx" type
		[ -n "$type" ] || break
		devidx=$((devidx + 1))
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

		echo "$detected_channel_80211a" | grep -q "^[[:digit:]]\+$" && {
			mode_band="a"
			channel="$detected_channel_80211a"
			iw phy "$dev" info | grep -q 'VHT Capabilities' && htmode="VHT80"
			wifi_5ghz="_5G"
		}

		[ -n "$htmode" ] && ht_capab="set wireless.radio${devidx}.htmode=${htmode}"

		local ht40_noscan=
		[ "$htmode" = "HT40" ] && ht40_noscan="set wireless.radio${devidx}.noscan=1"

		if [ -x /usr/bin/readlink ] && [ -h "/sys/class/ieee80211/${dev}" ]; then
			path="$(readlink -f "/sys/class/ieee80211/${dev}/device")"
		else
			path=
		fi
		if [ -n "$path" ]; then
			path="${path##/sys/devices/}"
			case "$path" in
			platform*/pci*) path="${path##platform/}" ;;
			esac
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

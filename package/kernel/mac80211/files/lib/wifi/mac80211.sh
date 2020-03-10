#!/bin/sh
append DRIVERS "mac80211"

lookup_phy() {
	[ -n "$phy" ] && {
		[ -d /sys/class/ieee80211/$phy ] && return
	}

	local devpath
	config_get devpath "$device" path
	[ -n "$devpath" ] && {
		for phy in $(ls /sys/class/ieee80211 2>/dev/null); do
			case "$(readlink -f /sys/class/ieee80211/$phy/device)" in
				*$devpath) return;;
			esac
		done
	}

	local macaddr="$(config_get "$device" macaddr | tr 'A-Z' 'a-z')"
	[ -n "$macaddr" ] && {
		for _phy in /sys/class/ieee80211/*; do
			[ -e "$_phy" ] || continue

			[ "$macaddr" = "$(cat ${_phy}/macaddress)" ] || continue
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
	[ -n "$phy" -a -d "/sys/class/ieee80211/$phy" ] || {
		echo "PHY for wifi device $1 not found"
		return 1
	}
	config_set "$device" phy "$phy"

	config_get macaddr "$device" macaddr
	[ -z "$macaddr" ] && {
		config_set "$device" macaddr "$(cat /sys/class/ieee80211/${phy}/macaddress)"
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
		devidx=$(($devidx + 1))
	done

	for _dev in /sys/class/ieee80211/*; do
		[ -e "$_dev" ] || continue

		dev="${_dev##*/}"

		found=0
		config_foreach check_mac80211_device wifi-device
		[ "$found" -gt 0 ] && continue

		mode_band="g"
		# LEDE 17.01中mt7603e驱动无法使用auto信道，故设置为11。其余设备正常设置为auto。
		[ -n "$(cat /sys/class/ieee80211/${dev}/device/uevent 2>/dev/null | grep 'DRIVER=mt7603e')" ] && channel="11" || channel="auto"
		htmode=""
		ht_capab=""

		iw phy "$dev" info | grep -q 'Capabilities:' && htmode="HT40"
		iw phy "$dev" info | grep -q '2412 MHz' || { mode_band="a"; channel="auto"; }


		# LEDE 17.01中QCA9558使用HT40频宽会导致2.4GHz丢失
		[ -n "$(cat /sys/class/ieee80211/${dev}/device/uevent 2>/dev/null | grep 'MODALIAS=platform:qca955x_wmac')" ] && htmode="HT20"

		vht_cap=$(iw phy "$dev" info | grep -c 'VHT Capabilities')
		cap_5ghz=$(iw phy "$dev" info | grep -c "Band 2")
		[ "$vht_cap" -gt 0 -a "$cap_5ghz" -gt 0 ] && {
			mode_band="a";
			channel="auto"
			htmode="VHT80"
		}
		
		wifi_5ghz=""
		[ "$mode_band" == "a" ] && wifi_5ghz="_5G"

		[ -n "$htmode" ] && ht_capab="set wireless.radio${devidx}.htmode=${htmode}"

		ht40_noscan=""		
		[ "$htmode" == "HT40" ] && ht40_noscan="set wireless.radio${devidx}.noscan=1"

		if [ -x /usr/bin/readlink -a -h /sys/class/ieee80211/${dev} ]; then
			path="$(readlink -f /sys/class/ieee80211/${dev}/device)"
		else
			path=""
		fi
		if [ -n "$path" ]; then
			path="${path##/sys/devices/}"
			case "$path" in
				platform*/pci*) path="${path##platform/}";;
			esac
			dev_id="set wireless.radio${devidx}.path='$path'"
		else
			dev_id="set wireless.radio${devidx}.macaddr=$(cat /sys/class/ieee80211/${dev}/macaddress)"
		fi

		mac_addr=`cat /sys/class/ieee80211/${dev}/macaddress|awk -F ":" '{print $5""$6 }'| tr a-z A-Z`
		ssid_name="LEDE_${mac_addr}${wifi_5ghz}"

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
			set wireless.default_radio${devidx}.ssid=${ssid_name}
			set wireless.default_radio${devidx}.encryption=none
EOF
		uci -q commit wireless

		devidx=$(($devidx + 1))
	done
}

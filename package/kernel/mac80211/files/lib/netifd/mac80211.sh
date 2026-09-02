#!/bin/sh
#
# Helpers shared by the wifi detection and the netifd mac80211 handler.
#

# Resolve a phy name to a path that is unique for this phy.
#
# A single device can register more than one phy, e.g. the two bands of
# an MT7615D.  All of those phys resolve to the same device link, so
# matching on the device path alone would bind every radio of the
# device to the first phy.  The index of the phy below
#	/sys/class/ieee80211/<phy>/device/ieee80211
# is therefore appended as "+1", "+2", ... to keep the paths unique.
mac80211_phy_to_path() {
	local phy="$1"
	[ -x /usr/bin/readlink -a -h /sys/class/ieee80211/${phy} ] || return
	local path="$(readlink -f /sys/class/ieee80211/${phy}/device)"
	[ -n "$path" ] || return
	path="${path##/sys/devices/}"
	case "$path" in
	platform*/pci*) path="${path##platform/}";;
	esac
	local p
	local seq=""
	for p in $(ls /sys/class/ieee80211/$phy/device/ieee80211 2>/dev/null); do
		[ "$p" = "$phy" ] && {
			echo "$path${seq:++$seq}"
			break
		}
		seq=$((${seq:-0} + 1))
	done
}

# Inverse of mac80211_phy_to_path().
mac80211_path_to_phy() {
	local path="$1"
	local p
	for p in $(ls /sys/class/ieee80211 2>/dev/null); do
		local cur="$(mac80211_phy_to_path "$p")"
		case "$cur" in
		*$path) echo "$p"; return;;
		esac
	done
}

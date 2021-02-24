#!/bin/bash

gen_conf() {
	#设备完整名称
	local device="$1"

	#调整cpu_type
	local cpu_type
	if [ $# -eq 2 ]; then
		cpu_type="$2"
	fi

	local target2="$(echo "$device" | sed 's/_DEVICE_.*$//g')"
	local target1="$(echo "$target2" | sed 's/_.*$//g')"

	local device_short="$(echo "$device" | sed 's/^.*_DEVICE_//g')"

	echo "Processing: device $device_short"

	mkdir -p ./conf
	# 文件不删空有可能导致配置错误
	rm -rf tmp/ .config .config.old
	# LEDE源码有坑，以下这行是处理LEDE源码坑，避免make defconfig的时候出错。
	make -C scripts/config clean >/dev/null 2>&1

	cat >.config <<-EOF
		CONFIG_TARGET_${target1}=y
		CONFIG_TARGET_${target2}=y
		CONFIG_TARGET_${device}=y
	EOF

	make defconfig >/dev/null

	local line_str
	local line_str_rep

	if [ -n "$cpu_type" ]; then
		line_str="$(grep '^CONFIG_TARGET_OPTIMIZATION=' .config)"
		line_str_rep="$(echo "$line_str" | sed "s/-mtune=[^[[:space:]\"]*/-mtune=${cpu_type}/g")"
		sed -i "s/$line_str/$line_str_rep/g" .config
	fi
	cp -f .config "conf/.config.$device_short"
	rm -f .config .config.old
	echo ""
	echo "Complete: written to conf/.config.$device_short"
	echo ""
}

process_devices() {
	devices=$1
	local cpu_type=$2
	local n=${#devices[@]}
	for i in $(seq 0 $(expr $n - 1)); do
		gen_conf "${devices[i]}" $cpu_type
	done
}

###################################################
rm -f conf/.config.*
###################################################

### ar71xx 74kc without USB
export _CONF_ROUTER_WITHOUT_USB_PORT_=1

devices=(
	"ar71xx_generic_DEVICE_tl-wr841-v8"
	"ar71xx_generic_DEVICE_tl-wr841-v8-cn"
	"ar71xx_generic_DEVICE_tl-wr941nd-v6"
	"ar71xx_generic_DEVICE_tl-wr941nd-v6-cn"
	"ar71xx_generic_DEVICE_tl-wr941n-v7"
)
process_devices $devices "74kc"

### ar71xx 24kc without USB
devices=(
	"ar71xx_generic_DEVICE_tl-wr841-v9"
)
process_devices $devices

export -n _CONF_ROUTER_WITHOUT_USB_PORT_

./def_config.sh

###################################################

### ar71xx 74kc with USB
devices=(
	"ar71xx_generic_DEVICE_csac"
	"ar71xx_generic_DEVICE_domywifi-dw33d"
	"ar71xx_generic_DEVICE_hq55-v1"
	"ar71xx_generic_DEVICE_tl-wdr7500-v3"
	"ar71xx_generic_DEVICE_tl-wdr4310-v1"
	"ar71xx_generic_DEVICE_tl-wr1041n-v2"
	"ar71xx_generic_DEVICE_tl-wr842n-v2"
	"ar71xx_generic_DEVICE_tl-wr842n-v2-cn"
)
process_devices $devices "74kc"

# ar71xx 24kc with USB
devices=(
	"ar71xx_generic_DEVICE_tl-wr941nd-v2"
)
process_devices $devices

### 7621 with USB
devices=(
	"ramips_mt7621_DEVICE_wr1200js"
	"ramips_mt7621_DEVICE_newifi-d2"
	"ramips_mt7621_DEVICE_a3004ns"
)
process_devices $devices "1004kc"

### 7620 with USB
devices=(
	"ramips_mt7620_DEVICE_y1s"
)
process_devices $devices

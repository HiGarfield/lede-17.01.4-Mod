-- Copyright (C) 2021 dz <dingzhong110@gmail.com>

local m,s,o
local sys = require "luci.sys"
local uci = require "luci.model.uci".cursor()

m = Map("easymesh")

function detect_Node()
	local data = {}
	local lps = luci.util.execi(" batctl n 2>/dev/null | tail +2 | sed 's/^[ ][ ]*//g' | sed 's/[ ][ ]*/ /g' | sed 's/$/ /g' ")
	for value in lps do
		local row = {}
		local pos = string.find(value, " ")
		local IFA = string.sub(value, 1, pos - 1)
		local value = string.sub(value, pos + 1, string.len(value))
		pos = string.find(value, " ")
		local pos = string.find(value, " ")
		local Neighbora = string.sub(value, 1, pos - 1)
		local value = string.sub(value, pos + 1, string.len(value))
		pos = string.find(value, " ")
		local pos = string.find(value, " ")
		local lastseena = string.sub(value, 1, pos - 1)
		local value = string.sub(value, pos + 1, string.len(value))
		pos = string.find(value, " ")
		row["IF"] = IFA
		row["Neighbor"] = Neighbora
		row["lastseen"] = lastseena
		table.insert(data, row)
	end
	return data
end
local Nodes = luci.sys.exec("batctl n 2>/dev/null| tail +3 | wc -l")
local Node = detect_Node()
v = m:section(Table, Node, "" ,"<b>" .. translate("Active node") .. "：" .. Nodes .. "</b>")
v:option(DummyValue, "IF", translate("IF"))
v:option(DummyValue, "Neighbor", translate("Neighbor"))
v:option(DummyValue, "lastseen", translate("lastseen"))

-- Basic
s = m:section(TypedSection, "easymesh", translate("Settings"), translate("General Settings"))
s.anonymous = true

---- Eanble
o = s:option(Flag, "enabled", translate("Enable"), translate("Enable or disable EASY MESH"))
o.default = 0
o.rmempty = false

o = s:option(ListValue, "role", translate("role"))
o:value("off", translate("off"))
o:value("server", translate("host MESH"))
o:value("client", translate("son MESH"))
o.rmempty = false

apRadio = s:option(ListValue, "apRadio", translate("MESH Radio device"), translate("The radio device which MESH use"))
uci:foreach("wireless", "wifi-device",
							function(s)
								apRadio:value(s['.name'])
							end)
apRadio:value("all", translate("ALL"))
o.default = "radio0"
o.rmempty = false

---- mesh
o = s:option(Value, "mesh_id", translate("MESH ID"))
o.default = "easymesh"
o.description = translate("MESH ID")

enable = s:option(Flag, "encryption", translate("Encryption"), translate(""))
enable.default = 0
enable.rmempty = false

o = s:option(Value, "key", translate("Key"), translate("Mesh Key"))
o.default = "easymesh"
o.password = true
o:depends("encryption", 1)

---- kvr
enable = s:option(Flag, "kvr", translate("K/V/R"), translate(""))
enable.default = 1
enable.rmempty = false

o = s:option(Value, "mobility_domain", translate("Mobility Domain"), translate("4-character hexadecimal ID"))
o.default = "5555"
o.datatype = "and(hexstring,rangelength(4,4))"
o:depends("kvr", 1)

o = s:option(Value, "rssi_val", translate("Threshold for an good RSSI"))
o.default = "-65"
o.datatype = "range(-120,-1)"
o:depends("kvr", 1)

o = s:option(Value, "low_rssi_val", translate("Threshold for an bad RSSI"))
o.default = "-80"
o.datatype = "range(-120,-1)"
o:depends("kvr", 1)

-- 2.4G wifi 802.11r
enable = s:option(Flag, "r_2g", translate("2.4g wifi enable r"), translate("Enable 802.11r for 2.4G"))
enable.default = 1
enable.rmempty = false
enable:depends("kvr", 1)

---- 802.11F
--enable = s:option(Flag, "iapp", translate("inter-access point protocol"), translate("Wireless Access Points (APs) running on different vendors can communicate with each other"))
--enable.default = 0
--enable.rmempty = false

---- ap_mode
enable = s:option(Flag, "ap_mode", translate("AP MODE Enable"), translate("Enable or disable AP MODE"))
enable.default = 0
enable.rmempty = false

o = s:option(Value, "ipaddr", translate("IPv4-Address"))
o.default = "192.168.1.10"
o.datatype = "ip4addr"
o:depends("ap_mode", 1)

o = s:option(Value, "netmask", translate("IPv4 netmask"))
o.default = "255.255.255.0"
o.datatype = "ip4addr"
o:depends("ap_mode", 1)

o = s:option(Value, "gateway", translate("IPv4 gateway"))
o.default = "192.168.1.1"
o.datatype = "ip4addr"
o:depends("ap_mode", 1)

o = s:option(Value, "dns", translate("Use custom DNS servers"))
o.default = "192.168.1.1"
o.datatype = "ip4addr"
o:depends("ap_mode", 1)

return m

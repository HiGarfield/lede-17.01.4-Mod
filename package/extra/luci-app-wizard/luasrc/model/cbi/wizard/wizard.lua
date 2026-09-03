--[[
LuCI - Lua Configuration Interface - Quick Setup

This page does not keep any settings of its own: every value is read
from the network and wireless configuration when the page is rendered
and written straight back to those files when the form is submitted.
]]--

local uci = require("luci.model.uci").cursor()
local sys = require("luci.sys")

-- Access point interfaces shown and edited by the wizard.
local wifi_ifaces = {}
uci:foreach("wireless", "wifi-iface",
	function(s)
		if s.mode == "ap" then
			wifi_ifaces[#wifi_ifaces + 1] = s[".name"]
		end
	end)

local has_wifi = (#wifi_ifaces > 0)

-- Make sure the interfaces we write to exist.
local function ensure_interface(name)
	if not uci:get("network", name) then
		uci:set("network", name, "interface")
	end
end

local function set_opt(section, option, value)
	if value ~= nil and value ~= "" then
		uci:set("network", section, option, value)
	else
		uci:delete("network", section, option)
	end
end

local m = SimpleForm("wizard",
	translate("Initial Router Setup"),
	translate("If you are using this router for the first time, please configure it here."))

m.reset = false

--
-- WAN
--

local wan = m:section(SimpleSection, translate("WAN Settings"),
	translate("Three different ways to access the Internet, please choose according to your own situation."))

local wan_proto = wan:option(ListValue, "wan_proto", translate("Protocol"))
wan_proto:value("dhcp", translate("DHCP client"))
wan_proto:value("static", translate("Static address"))
wan_proto:value("pppoe", translate("PPPoE"))
function wan_proto.cfgvalue(self, section)
	return uci:get("network", "wan", "proto") or "dhcp"
end

local wan_user = wan:option(Value, "wan_pppoe_user", translate("Broadband Username"))
wan_user:depends({wan_proto = "pppoe"})
function wan_user.cfgvalue(self, section)
	return uci:get("network", "wan", "username")
end

local wan_pass = wan:option(Value, "wan_pppoe_pass", translate("Broadband Password"))
wan_pass.password = true
wan_pass:depends({wan_proto = "pppoe"})
function wan_pass.cfgvalue(self, section)
	return uci:get("network", "wan", "password")
end

local wan_ipaddr = wan:option(Value, "wan_ipaddr", translate("IPv4 address"))
wan_ipaddr.datatype = "ip4addr"
wan_ipaddr:depends({wan_proto = "static"})
function wan_ipaddr.cfgvalue(self, section)
	return uci:get("network", "wan", "ipaddr")
end

local wan_netmask = wan:option(Value, "wan_netmask", translate("IPv4 netmask"))
wan_netmask.datatype = "ip4addr"
wan_netmask:value("255.255.255.0")
wan_netmask:value("255.255.0.0")
wan_netmask:value("255.0.0.0")
wan_netmask:depends({wan_proto = "static"})
function wan_netmask.cfgvalue(self, section)
	return uci:get("network", "wan", "netmask")
end

local wan_gateway = wan:option(Value, "wan_gateway", translate("IPv4 gateway"))
wan_gateway.datatype = "ip4addr"
wan_gateway:depends({wan_proto = "static"})
function wan_gateway.cfgvalue(self, section)
	return uci:get("network", "wan", "gateway")
end

local wan_dns = wan:option(DynamicList, "wan_dns", translate("Use custom DNS servers"))
wan_dns.datatype = "ip4addr"
wan_dns:depends({wan_proto = "static"})
function wan_dns.cfgvalue(self, section)
	return uci:get("network", "wan", "dns")
end

--
-- Wireless
--

local wifi_ssid, wifi_key

if has_wifi then
	local wl = m:section(SimpleSection, translate("Wireless Settings"),
		translate("Set the router's wireless name and password. For more advanced settings, please go to the Network-Wireless page."))

	wifi_ssid = wl:option(Value, "wifi_ssid",
		translate("<abbr title=\"Extended Service Set Identifier\">Wireless Name (ESSID)</abbr>"))
	wifi_ssid.datatype = "maxlength(32)"
	function wifi_ssid.cfgvalue(self, section)
		return uci:get("wireless", wifi_ifaces[1], "ssid")
	end

	wifi_key = wl:option(Value, "wifi_key", translate("Wireless Password"))
	wifi_key.datatype = "wpakey"
	wifi_key.password = true
	function wifi_key.cfgvalue(self, section)
		return uci:get("wireless", wifi_ifaces[1], "key")
	end
end

--
-- LAN
--

local lan = m:section(SimpleSection, translate("LAN Settings"))

local lan_ipaddr = lan:option(Value, "lan_ipaddr", translate("IPv4 address"))
lan_ipaddr.datatype = "ip4addr"
function lan_ipaddr.cfgvalue(self, section)
	return uci:get("network", "lan", "ipaddr")
end

local lan_netmask = lan:option(Value, "lan_netmask", translate("IPv4 netmask"))
lan_netmask.datatype = "ip4addr"
lan_netmask:value("255.255.255.0")
lan_netmask:value("255.255.0.0")
lan_netmask:value("255.0.0.0")
function lan_netmask.cfgvalue(self, section)
	return uci:get("network", "lan", "netmask")
end

--
-- Write the submitted values straight into the network/wireless config
--

local function formvalue(field, section)
	local val = field:formvalue(section)
	if type(val) == "table" then
		return table.concat(val, " ")
	end
	return val
end

local function apply_wizard(section)
	local proto   = formvalue(wan_proto, section) or "dhcp"
	local user    = formvalue(wan_user, section)
	local pass    = formvalue(wan_pass, section)
	local ipaddr  = formvalue(wan_ipaddr, section)
	local netmask = formvalue(wan_netmask, section)
	local gateway = formvalue(wan_gateway, section)
	local dns     = formvalue(wan_dns, section)
	local ssid    = has_wifi and formvalue(wifi_ssid, section) or nil
	local key     = has_wifi and formvalue(wifi_key, section) or nil
	local lanaddr = formvalue(lan_ipaddr, section)
	local lannet  = formvalue(lan_netmask, section)

	ensure_interface("wan")
	ensure_interface("lan")

	local ifname = uci:get("network", "wan", "ifname")

	-- Drop every protocol specific option so that switching the
	-- protocol cannot leave stale settings behind.
	local stale = {"ipaddr", "netmask", "gateway", "username", "password",
	               "dns", "peerdns", "keepalive", "mtu"}
	for _, opt in ipairs(stale) do
		uci:delete("network", "wan", opt)
	end

	uci:set("network", "wan", "proto", proto)
	uci:set("network", "wan", "metric", "40")
	if ifname then
		uci:set("network", "wan", "ifname", ifname)
	end

	if proto == "static" then
		set_opt("wan", "ipaddr", ipaddr)
		set_opt("wan", "netmask", netmask)
		set_opt("wan", "gateway", gateway)
	elseif proto == "pppoe" then
		set_opt("wan", "username", user)
		set_opt("wan", "password", pass)
		uci:set("network", "wan", "keepalive", "5 5")
		uci:set("network", "wan", "mtu", "1492")
		uci:set("network", "wan", "ipv6", "1")

		if uci:get("network", "wan6") then
			uci:set("network", "wan6", "ifname", "@wan")
			uci:set("network", "wan6", "reqaddress", "try")
			uci:set("network", "wan6", "reqprefix", "auto")
		end
	end

	if dns and #dns > 0 then
		uci:set("network", "wan", "peerdns", "0")
		uci:set("network", "wan", "dns", dns)
	end

	set_opt("lan", "ipaddr", lanaddr)
	set_opt("lan", "netmask", lannet)

	if has_wifi and ssid and #ssid > 0 then
		for _, iface in ipairs(wifi_ifaces) do
			uci:set("wireless", iface, "ssid", ssid)
			uci:set("wireless", iface, "encryption", "psk2")
			if key and #key > 0 then
				uci:set("wireless", iface, "key", key)
			end
		end
	end

	uci:commit("network")
	uci:commit("wireless")
end

-- Remember the section id handed to us by CBI and flag the form as
-- changed.  The write itself happens once, in the form handler below.
local section_id = 1
local dirty = false

local function mark_dirty(self, section, value)
	section_id = section
	dirty = true
	return true
end

wan_proto.write = mark_dirty
wan_user.write = mark_dirty
wan_pass.write = mark_dirty
wan_ipaddr.write = mark_dirty
wan_netmask.write = mark_dirty
wan_gateway.write = mark_dirty
wan_dns.write = mark_dirty
lan_ipaddr.write = mark_dirty
lan_netmask.write = mark_dirty
if has_wifi then
	wifi_ssid.write = mark_dirty
	wifi_key.write = mark_dirty
end

function m.handle(self, state, data)
	if state == FORM_VALID and dirty then
		apply_wizard(section_id)
		sys.call("/etc/init.d/network reload >/dev/null 2>&1 &")
	end
	return true
end

return m

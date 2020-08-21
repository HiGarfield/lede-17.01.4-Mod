local sys = require "luci.sys"
local ipc = require "luci.ip"
local ifaces = sys.net:devices()

m = Map("arpbind", translate("IP/MAC Binding"), 
		translatef("ARP is used to convert a network address (e.g. an IPv4 address) to a physical address such as a MAC address. Here you can add some static ARP binding rules."))

s = m:section(TypedSection, "arpbind", translate("Rules"))
s.template = "cbi/tblsection"
s.anonymous = true
s.addremove = true

ip = s:option(Value, "ipaddr", translate("IPv4 Address"))
ip.datatype = "ip4addr"
ip.optional = false

mac = s:option(Value, "macaddr", translate("MAC Address"))
mac.datatype = "list(macaddr)"
mac.optional = false

ipc.neighbors({ family = 4 }, function(n)
	if n.mac and n.dest then
		ip:value(n.dest:string())
		mac:value(n.mac, "%s (%s)" %{ n.mac, n.dest:string() })
	end
end)

a = s:option(ListValue, "ifname", translate("Interface"))
for _, iface in ipairs(ifaces) do
	if iface ~= "lo" then
		a:value(iface)
	end
end
a.default = "br-lan"
a.rmempty = false

return m

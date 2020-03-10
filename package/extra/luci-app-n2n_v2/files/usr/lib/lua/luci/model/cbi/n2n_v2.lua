local m,s
local SYS  = require "luci.sys"

if SYS.call("pidof edge > /dev/null") == 0 then
	Status = translate("<strong><font color=green>Edge is running</font></strong>")
else
	Status = translate("<strong><font color=red>Edge is not running</font></strong>")
end

if SYS.call("pidof supernode > /dev/null") == 0 then
	Statussupernode = translate("<strong><font color=green>Supernode is running</font></strong>")
else
	Statussupernode = translate("<strong><font color=red>Supernode is not running</font></strong>")
end

m = Map("n2n_v2")
m.title	= translate("N2N v2 VPN")
m.description = translate("n2n is a layer-two peer-to-peer virtual private network (VPN) which allows users to exploit features typical of P2P applications at network instead of application level.")

s=m:section(TypedSection,"edge",translate("N2N Edge Settings"))
s.anonymous=true
s.description = translate(string.format("%s<br /><br />", Status))
switch=s:option(Flag,"enabled",translate("Enable"))
switch.rmempty=false
tunname=s:option(Value,"tunname",translate("TUN desvice name"))
tunname.optional=false
mode=s:option(ListValue,"mode",translate("Interface mode"))
mode:value("dhcp")
mode:value("static")
ipaddr=s:option(Value,"ipaddr",translate("Interface IP address"))
ipaddr.optional=false
ipaddr.datatype = "ip4addr"
ipaddr:depends("mode","static")
ipaddr.rmempty = true
netmask=s:option(Value,"netmask",translate("Interface netmask"))
netmask.optional=false
netmask.datatype = "ip4addr"
netmask:depends("mode","static")
netmask.rmempty = true
supernode=s:option(Value,"supernode",translate("Supernode Address"),
translate("Support domain name"))
supernode.optional=false
supernode.password=true
supernode.datatype = "host"
supernode.rmempty=false
port=s:option(Value,"port",translate("Supernode Port"))
port.datatype="range(1,65535)"
port.optional=false
port.rmempty=false
community=s:option(Value,"community",translate("N2N Community name"))
community.optional=false
key=s:option(Value,"key",translate("Encryption key"))
key.password = true
route=s:option(Flag,"route",translate("Enable packet forwarding"))
route.rmempty=false
plugin_localip = s:option(Flag, "plugin_localip", translate("Plug-in local IP"),
translate("Add local IP to bypass between same nat problem"))
localip=s:option(Value,"localip",translate("Local IP"))
localip.rmempty = true
localip.datatype = "ip4addr"
localip:depends("plugin_localip", "1")
plugin_interval = s:option(Flag, "plugin_interval", translate("Plug-in Interval"),
translate("Set the NAT hole-punch interval (If not open, default 20 seconds)"))
interval=s:option(Value,"interval",translate("Interval (Seconds)"))
interval.default = 20
interval.datatype = "uinteger"
interval:depends("plugin_interval", "1")
plugin_checkip = s:option(Flag, "plugin_checkip", translate("Plug-in CheckIP"),
translate("Open the IP detection function, restart the N2N when PING doesn't specify IP"))
checkip=s:option(Value,"checkip",translate("CheckIp"))
checkip.rmempty = true
checkip.datatype = "ip4addr"
checkip:depends("plugin_checkip", "1")
time_checkip = s:option(ListValue, "time_checkip", translate("Check Interval (Minutes)"))
for s=1,60 do
time_checkip:value(s)
end
time_checkip:depends("plugin_checkip", "1")
s2=m:section(TypedSection,"supernode",translate("N2N Supernode Settings"))
s2.anonymous=true
s2.description = translate(string.format("%s<br /><br />", Statussupernode))
switch=s2:option(Flag,"enabled",translate("Enable"))
switch.rmempty=false
port=s2:option(Value,"port",translate("Port"))
port.datatype="range(1,65535)"
port.optional=false

return m

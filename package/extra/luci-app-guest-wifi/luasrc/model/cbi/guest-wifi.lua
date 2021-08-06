require("luci.tools.webadmin")

m = Map("guest-wifi", translate("Guest-wifi"))

s = m:section(NamedSection, "config", "guest-wifi", translate("Config"), translate(
    "You can set guest wifi here. The wifi will be disconnected when enabling/disabling. When creating guest wifi or modifying the settings, please check both \"Enabled\" and \"Create/Remove\". When removing guest wifi, please uncheck \"Enabled\" and check \"Create/Remove\"."))

enabled = s:option(Flag, "enabled", translate("Enabled"), translate("Enable guest wifi"))
enabled.default = false
enabled.optional = false
enabled.rmempty = false

create = s:option(Flag, "create", translate("Create/Remove"), translate(
    "Check this to create guest wifi with \"Enabled\" checked; Check this to remove guest wifi with \"Enabled\" unchecked."))
create.default = false
create.optional = false
create.rmempty = false

device = s:option(ListValue, "device", translate("Define device"), translate("Define device of guest wifi"))
x = luci.model.uci.cursor()
x:foreach("wireless", "wifi-device", function(s)
    device:value(s[".name"])
end)
device.default = "radio0"
device.rmempty = false

wifi_name = s:option(Value, "wifi_name", translate("Wifi name"), translate("Define the name of guest wifi"))
wifi_name.default = "Guest-WiFi"
wifi_name.rmempty = false

interface_ip = s:option(Value, "interface_ip", translate("Interface IP address"),
    translate("Define IP address for guest wifi"))
interface_ip.datatype = "ip4addr"
interface_ip.default = "192.168.4.1"
interface_ip.rmempty = false

encryption = s:option(ListValue, "encryption", translate("Encryption"), translate("Define encryption of guest wifi"))
encryption:value("psk", "WPA-PSK")
encryption:value("psk2", "WPA2-PSK")
encryption:value("none", translate("No Encryption"))
encryption.rmempty = false
encryption.default = "psk2"

passwd = s:option(Value, "passwd", translate("Password"), translate("Define the password of guest wifi"))
passwd.password = true
passwd.default = "guestnetwork"
passwd.rmempty = false
passwd:depends("encryption", "psk")
passwd:depends("encryption", "psk2")

isolate = s:option(Flag, "isolate", translate("Isolation"), translate("Enalbe or disable isolation"))
isolate.default = true
passwd.rmempty = true

start = s:option(Value, "start", translate("Start address"),
    translate("Lowest leased address as offset from the network address"))
start.default = "50"
start.rmempty = true
start.datatype = "and(uinteger,range(2,254))"

limit = s:option(Value, "limit", translate("Client Limit"), translate("Maximum number of leased addresses"))
limit.default = "200"
limit.rmempty = true
limit.datatype = "and(uinteger,range(1,253))"

leasetime = s:option(Value, "leasetime", translate("DHCP lease time"),
    translate("Expiry time of leased addresses, minimum is 2 minutes (2m)"))
leasetime.default = "1h"
leasetime.rmempty = true

return m

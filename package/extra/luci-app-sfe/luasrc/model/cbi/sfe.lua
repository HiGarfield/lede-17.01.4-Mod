local m, s, o
local SYS = require "luci.sys"

if SYS.call("lsmod | grep -q fast_classifier") == 0 then
    Status = translate("<strong><font color=\"green\">Shortcut forwarding engine is running</font></strong>")
else
    Status = translate("<strong><font color=\"red\">Shortcut forwarding engine is not running</font></strong>")
end

m = Map("sfe")
m.title = translate("Shortcut Forwarding Engine Acceleration Settings")
m.description = translate("Opensource Qualcomm Shortcut FE driver (Fast Path)")

s = m:section(NamedSection, "config", "sfe", "")
s.addremove = false
s.anonymous = true
s.description = string.format("%s<br /><br />", Status)

o = s:option(Flag, "enabled", translate("Enable"))
o.default = 0
o.rmempty = false

o = s:option(Flag, "bridge", translate("Bridge acceleration"))
o.default = 0
o.rmempty = false
o.description = translate("Enable bridge acceleration")

o = s:option(Flag, "ipv6", translate("IPv6 acceleration"))
o.default = 0
o.rmempty = false
o.description = translate("Enable IPv6 acceleration")

o = s:option(Value, "offload_at_pkts", translate("Accelerate at packets"))
o.description = translate("Start acceleration after how many packets, defaultly 128, at least 4")
o.datatype = "and(uinteger,min(4))"
o.rmempty = true
o.placeholder = 128

function m.on_after_commit(self)
    os.execute("/etc/init.d/sfe restart >/dev/null 2>&1 &")
end

return m

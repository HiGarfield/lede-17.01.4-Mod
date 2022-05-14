local m, s, enabled, bridge, ipv6, offload_at_pkts
local SYS = require "luci.sys"

if SYS.call("lsmod | grep -qw \'^fast_classifier\'") == 0 then
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

enabled = s:option(Flag, "enabled", translate("Enable"))
enabled.rmempty = false

bridge = s:option(Flag, "bridge", translate("Bridge acceleration"))
bridge.description = translate("Enable bridge acceleration")
bridge:depends("enabled", "1")

offload_at_pkts = s:option(Value, "offload_at_pkts", translate("Accelerate at packets"))
offload_at_pkts.description = translate("Start acceleration after how many packets, defaultly 128, at least 4")
offload_at_pkts.datatype = "and(uinteger,min(4))"
offload_at_pkts.placeholder = 128
offload_at_pkts:depends("enabled", "1")

function m.on_after_commit(self)
    os.execute("/etc/init.d/sfe restart >/dev/null 2>&1 &")
end

return m

local sys = require "luci.sys"

m = Map("wol", translate("Wake on LAN"), translate(
            "Wake on LAN is a mechanism to remotely boot computers in the local network."))

s = m:section(TypedSection, "hosts", translate("Hosts"))
s.template = "cbi/tblsection"
s.anonymous = true
s.addremove = true

iface = s:option(ListValue, "iface", translate("Interface"))
iface.default = "br-lan"
iface:value("", translate("Broadcast on all interfaces"))

for _, e in ipairs(sys.net.devices()) do if e ~= "lo" then iface:value(e) end end

hostname = s:option(Value, "hostname", translate("Hostname"))
macaddr = s:option(Value, "macaddr", translate("MAC address"))

sys.net.mac_hints(function(mac, name)
    macaddr:value(mac, "%s (%s)" % {mac, name})
    hostname:value(name)
end)

btn = s:option(Button, "wake", "")
btn.inputtitle = translate("Wake up host")
btn.inputstyle = "apply"
btn.write = function(self, section)
    local host = macaddr:formvalue(section)
    if host and host:match("^[a-fA-F0-9:]+$") then
        local interface = iface:formvalue(section)
        local cmd = "/usr/bin/etherwake -D%s %q" %
                        {(interface ~= "" and " -i %q" % interface or ""), host}
        local msg = "%s %s." % {translate("Starting WoL utility:"), cmd}
        local p = io.popen(cmd .. " 2>&1")
        if p then
            while true do
                local l = p:read("*l")
                if l then
                    if #l > 100 then
                        l = l:sub(1, 100) .. "..."
                    end
                    msg = msg .. " " .. l
                else
                    break
                end
            end
            p:close()
        end
        m.message = msg
    end
end

return m

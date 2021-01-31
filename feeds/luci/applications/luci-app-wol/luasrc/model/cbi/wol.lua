local sys = require "luci.sys"
local fs = require "nixio.fs"
local uci = require("luci.model.uci").cursor()

m = Map("wol", translate("Wake on LAN"),
        translate("Wake on LAN is a mechanism to remotely boot computers in the local network."))

local has_ewk = fs.access("/usr/bin/etherwake")
local has_wol = fs.access("/usr/bin/wol")

s = m:section(TypedSection, "hosts", translate("Hosts"))
s.template = "cbi/tblsection"
s.anonymous = true
s.addremove = true

if has_ewk and has_wol then
    bin = s:option(ListValue, "binary", translate("WoL program"))
    bin:value("/usr/bin/etherwake", "Etherwake")
    bin:value("/usr/bin/wol", "WoL")
end

if has_ewk then
    iface = s:option(ListValue, "iface", translate("Interface"))
    iface.default = "br-lan"

    if has_wol then
        iface:depends("binary", "/usr/bin/etherwake")
    end

    iface:value("", translate("Broadcast on all interfaces"))

    for _, e in ipairs(sys.net.devices()) do
        if e ~= "lo" then
            iface:value(e)
        end
    end
end

hostname = s:option(Value, "hostname", translate("Hostname"))
macaddr = s:option(Value, "macaddr", translate("MAC address"))

sys.net.mac_hints(function(mac, name)
    macaddr:value(mac, "%s (%s)" % {mac, name})
    hostname:value(name)
end)

btn = s:option(Button, "wake", "")
btn.inputtitle = translate("Wake up host")
btn.inputstyle = "apply"
function btn.write(self, section)
    local host = uci:get("wol", section, "macaddr")
    if host and #host > 0 and host:match("^[a-fA-F0-9:]+$") then
        local cmd
        local util = uci:get("wol", section, "binary") or (has_ewk and "/usr/bin/etherwake" or "/usr/bin/wol")
        if util == "/usr/bin/etherwake" then
            local iface = uci:get("wol", section, "iface")
            cmd = "%s -D%s %q" % {util, (iface ~= "" and " -i %q" % iface or ""), host}
        else
            cmd = "%s -v %q" % {util, host}
        end

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

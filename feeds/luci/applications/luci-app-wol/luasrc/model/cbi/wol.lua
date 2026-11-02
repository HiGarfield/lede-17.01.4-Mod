local sys = require "luci.sys"

m = Map("wol", translate("Wake on LAN"), translate(
            "Wake on LAN is a mechanism to remotely boot computers in the local network."))

s = m:section(TypedSection, "hosts", translate("Saved hosts"))
s.description = translate("Hosts saved in the configuration.")
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

local function wake_host(host, interface)
    if not host or not host:match("^[a-fA-F0-9:]+$") then return end
    local cmd = "/usr/bin/etherwake -D%s %q" %
                    {(interface and interface ~= "" and " -i %q" % interface or
                      ""), host}
    local msg = "%s %s." % {translate("Starting WoL utility:"), cmd}
    local p = io.popen(cmd .. " 2>&1")
    if p then
        while true do
            local l = p:read("*l")
            if l then
                if #l > 100 then l = l:sub(1, 100) .. "..." end
                msg = msg .. " " .. l
            else
                break
            end
        end
        p:close()
    end
    m.message = msg
end

btn = s:option(Button, "wake", "")
btn.inputtitle = translate("Wake up host")
btn.inputstyle = "apply"
btn.write = function(self, section)
    wake_host(macaddr:formvalue(section), iface:formvalue(section))
end

local tdata = {temporary = {}}
local known = {}
local hosts = {}
local saved_iface
local saved_mac

m.uci:foreach("wol", "hosts", function(h)
    local mac = h.macaddr

    if mac and mac ~= "" then
        if not known[mac] then
            known[mac] = true

            if h.hostname and h.hostname ~= "" then
                hosts[#hosts + 1] = {mac, "%s (%s)" % {mac, h.hostname}}
            else
                hosts[#hosts + 1] = {mac, mac}
            end
        end

        -- Remember the first saved host so the temporary host starts with the
        -- same selection the saved hosts table shows.
        if not saved_mac then
            saved_mac = mac
            saved_iface = h.iface
        end
    end
end)

sys.net.mac_hints(function(mac, name)
    if not known[mac] then
        known[mac] = true
        hosts[#hosts + 1] = {mac, "%s (%s)" % {mac, name}}
    end
end)

ts = m:section(Table, tdata, translate("Temporary host"))
ts.description = translate(
                     "Choose the host to wake up or enter a custom MAC address to use")
ts.template = "cbi/tblsection"
ts.anonymous = true

tiface = ts:option(ListValue, "iface", translate("Interface"))
tiface.default = saved_iface or "br-lan"
tiface.rmempty = true
tiface:value("", translate("Broadcast on all interfaces"))

for _, e in ipairs(sys.net.devices()) do if e ~= "lo" then tiface:value(e) end end

-- The known hosts are offered as suggestions, a custom address can be typed.
thost = ts:option(Value, "macaddr", translate("MAC address"))
thost.default = saved_mac
thost.rmempty = true
for _, h in ipairs(hosts) do thost:value(h[1], h[2]) end

-- The temporary host is never written to the UCI configuration.
tiface.write = function() end
tiface.remove = function() end
thost.write = function() end
thost.remove = function() end

tbtn = ts:option(Button, "wake", "")
tbtn.inputtitle = translate("Wake up host")
tbtn.inputstyle = "apply"
tbtn.write = function(self, section)
    wake_host(thost:formvalue(section), tiface:formvalue(section))
end

return m

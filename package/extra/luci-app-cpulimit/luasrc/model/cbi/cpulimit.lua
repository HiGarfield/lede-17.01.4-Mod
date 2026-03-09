local fs = require "nixio.fs"
local util = require "luci.util"

m = Map("cpulimit", translate("Cpulimit"), translate("Limit CPU usage of selected processes"))

s = m:section(TypedSection, "list", translate("Settings"))
s.template = "cbi/tblsection"
s.anonymous = true
s.addremove = true

-- Enable option
enable = s:option(Flag, "enabled", translate("Enable"))
enable.rmempty = false

-- OpenWrt system process blacklist
-- These processes are critical to system operation and should never be limited
local ignore = {

    -- shells
    ash = true,
    sh = true,
    bash = true,
    login = true,
    askfirst = true,

    -- init/system
    init = true,
    procd = true,
    watchdog = true,

    -- ubus/rpc
    ubusd = true,
    rpcd = true,
    ubus = true,

    -- network core
    netifd = true,
    odhcpd = true,
    odhcp6c = true,
    dnsmasq = true,

    -- wifi
    hostapd = true,
    wpa_supplicant = true,

    -- logging/cron
    logd = true,
    crond = true,
    syslogd = true,
    klogd = true,

    -- ssh/web
    dropbear = true,
    uhttpd = true,

    -- firewall/network helpers
    firewall = true,
    mwan3 = true,
    mwan3track = true,

    -- system services
    ntpd = true,
    urngd = true,
    watchcat = true,

    -- storage
    blockd = true,

    -- prevent limiting cpulimit itself
    cpulimit = true
}

-- Scan /proc to collect running process names
local function get_process_list()

    local result = {}
    local seen = {}

    for pid in fs.dir("/proc") do
        -- Only numeric directories represent process IDs
        if pid:match("^%d+$") then
            local cmdline = fs.readfile("/proc/" .. pid .. "/cmdline")
            if cmdline and #cmdline > 0 then
                -- Extract executable path
                local cmd = cmdline:match("^[^%z]+")
                if cmd then
                    -- Strip path
                    cmd = cmd:match("([^/]+)$") or cmd
                    -- Remove login shell prefix "-"
                    cmd = cmd:gsub("^%-", "")
                    -- Ignore kernel threads like [kworker]
                    if not cmd:match("^%[") then
                        -- Filter ignored processes
                        if not ignore[cmd] and not seen[cmd] then
                            seen[cmd] = true
                            result[#result + 1] = cmd
                        end

                    end
                end
            end
        end
    end

    table.sort(result)
    return result
end

-- Executable name option
exename = s:option(Value, "exename", translate("Executable name"))

exename.rmempty = false

for _, name in ipairs(get_process_list()) do
    exename:value(name)
end

-- Function to count CPU cores by reading /proc/stat
local function get_cpu_cores()
    local count = 0
    for line in io.lines("/proc/stat") do
        if line:match("^cpu[0-9]") then
            count = count + 1
        end
    end
    return count or 1
end

local max_percentage = get_cpu_cores() * 100

-- CPU limit option
limit = s:option(Value, "limit", translate("Limit (%)"),
    string.format(translate("percentage of a single core (0-%d)"), max_percentage))

limit.rmempty = false
limit.datatype = string.format("and(uinteger,range(0,%d))", max_percentage)
limit.default = 50

for p = 10, max_percentage, 10 do
    limit:value(p)
end

-- Include child processes option
include_children = s:option(Flag, "include_children", translate("Include children"),
    translate("Also limit child processes"))

include_children.default = include_children.disabled

return m

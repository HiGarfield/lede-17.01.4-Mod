
local map, s = ...

o = s:taboption("general", Value, "ifname", translate("Interface"), translate("If you want to route mesh traffic over a wired network device, then please select it at this device selector. If you want to assign the batman-adv interface to a Wi-fi mesh then do not select the device here, but rather go to the wireless settings and select this interface as a network from there."))
o.template = "cbi/network_ifacelist_"
o.nobridges = true
o.noaliases = false

o = s:taboption("general", ListValue, "master", translate("Batman Device"), translate("This is the batman-adv device where you want to link the physical device from above to. Firstly you need to creat a \"Batman Device\" named bat0, then select it at this device selector."));
local sl = luci.util.execi("ls /sys/class/net |egrep '^bat'") or { }
for v in sl do
    o:value((v or "nil"))
end

o = s:taboption("general", Value, "mtu", translate("Override MTU"), translate("1532-9200"))
o.default = "1532"
o.datatype    = "max(9200)"


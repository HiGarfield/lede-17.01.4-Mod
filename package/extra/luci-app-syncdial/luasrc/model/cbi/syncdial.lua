--[[
Sync Dial Luci configuration page.
Copyright (C) 2015 GuoGuo <gch981213@gmail.com>
]]--

local fs = require "nixio.fs"

local cmd = "mwan3 status | grep -c \"is online and tracking is active\""
local shellpipe = io.popen(cmd,"r")
local ifnum = shellpipe:read("*a")
shellpipe:close()


m = Map("syncdial", translate("Add virtual WAN interface"),
        translate("Add multiple virtual WAN interfaces using macvlan.").."<br />"..translate("Number of interfaces online:").." "..ifnum..".")

s = m:section(TypedSection, "syncdial", translate(" "))
s.anonymous = true

switch = s:option(Flag, "enabled", translate("Enable"))
switch.rmempty = false


wannum = s:option(Value, "wannum", translate("Number of virtual WAN interfaces"))
wannum.datatype = "range(0,20)"
wannum.optional = false

s:option(Flag, "old_frame", translate("Use old macvlan method")).rmempty = false

o = s:option(DummyValue, "_redial", translate("Redial"))
o.template = "syncdial/redial_button"
o.width    = "10%"

return m



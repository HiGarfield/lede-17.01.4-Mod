--[[
Sync Dial Luci configuration page.
Copyright (C) 2015 GuoGuo <gch981213@gmail.com>
]] --
local fs = require "nixio.fs"

local cmd = "mwan3 status | grep -c \"interface vwan[[:digit:]]\\+ is online\""
local shellpipe = io.popen(cmd, "r")
local ifnum = shellpipe:read("*a")
shellpipe:close()

m = Map("syncdial", translate("Add virtual WAN interface"),
    translate("Add multiple virtual WAN interfaces using macvlan.") .. "<br />" ..
        translate("Number of online interfaces:") .. " " .. ifnum .. ".")

s = m:section(TypedSection, "syncdial", translate(" "))
s.anonymous = true

switch = s:option(Flag, "enabled", translate("Enable"))
switch.rmempty = false

wannum = s:option(Value, "wannum", translate("Number of virtual WAN interfaces"))
wannum.datatype = "range(0,20)"
wannum.optional = false

diagchk = s:option(Flag, "dialchk", translate("Enable dial check"))
diagchk.rmempty = false

diagnum = s:option(Value, "dialnum", translate("Minimal number of online interfaces"),
    translate("Redial if the number of online interfaces is less than this value"))
diagnum.datatype = "range(0,21)"
diagnum.optional = false

dialwait = s:option(Value, "dialwait", translate("Time waiting before redial"), translate("Minimal value: 5 seconds"))
dialwait.datatype = "and(uinteger,min(5))"
dialwait.optional = false

s:option(Flag, "old_frame", translate("Use old macvlan method")).rmempty = false

o = s:option(DummyValue, "_redial", translate("Redial"))
o.template = "syncdial/redial_button"
o.width = "10%"

return m

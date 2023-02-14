--[[
Sync Dial Luci configuration page.
Copyright (C) 2015 GuoGuo <gch981213@gmail.com>
]] --

m = Map("syncdial", translate("Add virtual WAN interface"),
    translate("Add multiple virtual WAN interfaces using macvlan."))

m:section(SimpleSection).template = "syncdial/syncdial_status"

s = m:section(NamedSection, "config", "syncdial", translate("Config"))
s.anonymous = true

enabled = s:option(Flag, "enabled", translate("Enable"))
enabled.rmempty = false

wannum = s:option(Value, "wannum", translate("Number of virtual WAN interfaces"))
wannum.datatype = "and(uinteger,range(0,20))"
wannum.optional = false

dialchk = s:option(Flag, "dialchk", translate("Enable dial check"))
dialchk.rmempty = false

minonlinenum = s:option(Value, "minonlinenum", translate("Minimal number of online interfaces"),
    translate("Redial if the number of online interfaces is less than this value"))
minonlinenum.datatype = "and(uinteger,range(0,20))"
minonlinenum.optional = false

dialwait = s:option(Value, "dialwait", translate("Time waiting before redial"), translate("Minimal value: 5 seconds"))
dialwait.datatype = "and(uinteger,min(5))"
dialwait.optional = false

o = s:option(DummyValue, "_redial", translate("Redial"))
o.template = "syncdial/redial_button"
o.width = "10%"

return m

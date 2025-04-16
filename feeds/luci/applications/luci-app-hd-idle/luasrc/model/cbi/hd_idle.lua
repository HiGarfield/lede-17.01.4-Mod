-- Copyright 2008 Yanira <forum-2008@email.de>
-- Licensed to the public under the Apache License 2.0.
require("nixio.fs")

m = Map("hd-idle", translate("HDD Idle"),
    translate("HDD Idle is a utility program for spinning-down external " .. "disks after a period of idle time."))

s = m:section(TypedSection, "hd-idle", translate("Settings"))
s.anonymous = true
s.addremove = true
s.template = "cbi/tblsection"
s.optional = false

enabled = s:option(Flag, "enabled", translate("Enable"))
enabled.default = 1
enabled.optional = false

disk = s:option(Value, "disk", translate("Disk"))
disk.optional = false
for dev in nixio.fs.glob("/dev/[sh]d[a-z]") do
    if nixio.fs.stat(dev, "type") == "blk" then
        disk:value(nixio.fs.basename(dev))
    end
end

idle_time_interval = s:option(Value, "idle_time_interval", translate("Idle time"))
idle_time_interval.default = 10
idle_time_interval.optional = false
idle_time_interval.maxlength = 6
idle_time_interval.datatype = 'uinteger'

unit = s:option(ListValue, "idle_time_unit", translate("Idle time unit"))
unit.default = "minutes"
unit:value("minutes", translate("min"))
unit:value("hours", translate("h"))
unit.optional = false

spindown = s:option(Button, "spin_down_immediately")
spindown.inputstyle = "apply"
spindown.render = function(self, section, scope)
    if nixio.fs.stat("/dev/%s" % self.map:get(section, "disk"), "type") == "blk" then
        self.title = translate("Spin down immediately")
        self.template = "cbi/button"
    else
        self.template = "cbi/dvalue"
    end
    Button.render(self, section, scope)
end
spindown.write = function(self, section)
    local disk_dev = self.map:get(section, "disk")
    if luci.sys.call("hd-idle -t '%s'" % disk_dev) == 0 then
        m.message = translate("Disk %s has been spun down.") % disk_dev
    else
        m.message = translate("Fails to spin down disk %s.") % disk_dev
    end
end

delete_device = s:option(Button, "delete_device")
delete_device.inputstyle = "apply"
delete_device.render = function(self, section, scope)
    if nixio.fs.stat("/dev/%s" % self.map:get(section, "disk"), "type") == "blk" then
        self.title = translate("Delete device")
        self.template = "cbi/button"
    else
        self.template = "cbi/dvalue"
    end
    Button.render(self, section, scope)
end
delete_device.write = function(self, section)
    local disk_dev = self.map:get(section, "disk")
    if luci.sys.call("hd-idle -t '%s' && echo 1 >'/sys/block/%s/device/delete' 2>/dev/null" % {disk_dev, disk_dev}) == 0 then
        m.message = translate("Disk %s has deleted.") % disk_dev
    else
        m.message = translate("Fails to delete disk %s.") % disk_dev
    end
end

return m

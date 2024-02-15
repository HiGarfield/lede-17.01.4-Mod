-- Copyright 2008 Steven Barth <steven@midlink.org>
-- Copyright 2008 Jo-Philipp Wich <jow@openwrt.org>
-- Licensed to the public under the Apache License 2.0.

m = Map("samba", translate("Network Shares"))

s = m:section(TypedSection, "samba", "Samba")
s.anonymous = true

s:tab("general",  translate("General Settings"))
s:tab("template", translate("Edit Template"))

s:taboption("general", Value, "name", translate("Hostname"))
s:taboption("general", Value, "description", translate("Description"))
s:taboption("general", Value, "workgroup", translate("Workgroup"))
s:taboption("general", Flag, "homes", translate("Share home-directories"),
        translate("Allow system users to reach their home directories via " ..
                "network shares")).default = 0
s:taboption("general", Flag, "auto_share", translate("Auto share"),
		translate("Allow auto share when a storage is mounted")).default = 1
auto_share_ro = s:taboption("general", Flag, "auto_share_ro",
		translate("Auto share with read-only permission"))
auto_share_ro.default = 0
auto_share_ro:depends("auto_share", "1")

local auto_share_enabled = false
luci.model.uci.cursor():foreach("samba", "samba", function(section)
    if section.auto_share == "1" then auto_share_enabled = true end
end)

if auto_share_enabled then
    local auto_shared_paths = {}
    luci.model.uci.cursor("/var/run/config"):foreach("samba", "sambashare",
                                                     function(section)
        table.insert(auto_shared_paths, {
            name = section.name,
            path = section.path,
            read_only = translate(section.read_only),
            guest_ok = translate(section.guest_ok),
            create_mask = section.create_mask,
            dir_mask = section.dir_mask
        })
    end)
    if next(auto_shared_paths) then
        table.sort(auto_shared_paths, function(a, b)
            if a ~= nil and b ~= nil and a.name ~= nil and b.name ~= nil then
                return a.name < b.name
            else
                return false
            end
        end)
    end

    v =
        m:section(Table, auto_shared_paths, translate("Auto Shared Directories"))
    v:option(DummyValue, "name", translate("Name"))
    v:option(DummyValue, "path", translate("Path"))
    v:option(DummyValue, "read_only", translate("Read-only"))
    v:option(DummyValue, "guest_ok", translate("Allow guests"))
    v:option(DummyValue, "create_mask", translate("Create mask"),
             translate("Mask for new files"))
    v:option(DummyValue, "dir_mask", translate("Directory mask"),
             translate("Mask for new directories"))
end

tmpl = s:taboption("template", Value, "_tmpl",
	translate("Edit the template that is used for generating the samba configuration."), 
	translate("This is the content of the file '/etc/samba/smb.conf.template' from which your samba configuration will be generated. " ..
		"Values enclosed by pipe symbols ('|') should not be changed. They get their values from the 'General Settings' tab."))

tmpl.template = "cbi/tvalue"
tmpl.rows = 20

function tmpl.cfgvalue(self, section)
	return nixio.fs.readfile("/etc/samba/smb.conf.template")
end

function tmpl.write(self, section, value)
	value = value:gsub("\r\n?", "\n")
	nixio.fs.writefile("/etc/samba/smb.conf.template", value)
end


s = m:section(TypedSection, "sambashare", translate("Shared Directories"))
s.anonymous = true
s.addremove = true
s.template = "cbi/tblsection"

s:option(Value, "name", translate("Name"))
pth = s:option(Value, "path", translate("Path"))
if nixio.fs.access("/etc/config/fstab") then
        pth.titleref = luci.dispatcher.build_url("admin", "system", "fstab")
end

s:option(Value, "users", translate("Allowed users")).rmempty = true

ro = s:option(Flag, "read_only", translate("Read-only"))
ro.rmempty = false
ro.enabled = "yes"
ro.disabled = "no"

go = s:option(Flag, "guest_ok", translate("Allow guests"))
go.rmempty = false
go.enabled = "yes"
go.disabled = "no"

cm = s:option(Value, "create_mask", translate("Create mask"),
        translate("Mask for new files"))
cm.rmempty = true
cm.size = 4

dm = s:option(Value, "dir_mask", translate("Directory mask"),
        translate("Mask for new directories"))
dm.rmempty = true
dm.size = 4


return m

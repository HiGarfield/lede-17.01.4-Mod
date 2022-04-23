m = Map("cpulimit", translate("Cpulimit"), translate("Use cpulimit to limit CPU usage of a process."))
s = m:section(TypedSection, "list", translate("Settings"))
s.template = "cbi/tblsection"
s.anonymous = true
s.addremove = true

enable = s:option(Flag, "enabled", translate("enable", "enable"))
enable.optional = false
enable.rmempty = false

exename = s:option(Value, "exename", translate("exename"), translate("name of the executable program file. CAN NOT BE A PATH!"))
exename.optional = false
exename.rmempty = false
exename.default = ""
local pscmd = "ps | awk '{print $5}' | sed -e '1d' -e '/^\\\[/d' -e '/^\{/d' -e 's#^.*/##g' -e '/^sed$/d' -e '/^awk$/d' -e '/^hostapd$/d' -e '/^pppd$/d' -e '/^mwan3$/d' -e '/^sleep$/d' -e '/^sort$/d' -e '/^ps$/d' -e '/^uniq$/d' -e '/^crond$/d' -e '/^dnsmasq$/d' -e '/^netifd$/d' -e '/^procd$/d' -e '/^rpcd$/d' -e '/^ubusd$/d' -e '/^-ash$/d'  -e '/^dropbear$/d' | sort -u"
for psvalue in luci.util.exec(pscmd):gmatch("[^\r\n]+") do
	exename:value(psvalue)
end

local maxpercentage = luci.util.exec("cpulimit -h | grep 'percentage of cpu allowed from 0 to' | sed 's/.*percentage of cpu allowed from 0 to \\(.*\\) (required)/\\1/g'")
limit = s:option(Value, "limit", translate("limit (%)"), string.format(translate("percentage of a single core (0-%d)"), maxpercentage))
limit.optional = false
limit.rmempty = false
limit.datatype = string.format("and(uinteger,range(0,%d))",maxpercentage)
limit.default = "50"
for percentage = 10, 100, 10 do
	limit:value(percentage)
end

return m

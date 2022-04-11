local fs = require "nixio.fs"
local NXFS = require "nixio.fs"
local WLFS = require "nixio.fs"
local SYS  = require "luci.sys"
local ND = SYS.exec("cat /etc/gfwlist/china-banned | wc -l")
local conf = "/etc/shadowsocksr/base-gfwlist.txt"

m = Map("ssrpro")
m.title	= translate("Shadowsocksr Transparent Proxy")
m.description = translate("A fast secure tunnel proxy that help you get through firewalls on your router")

m:section(SimpleSection).template  = "ssrpro/ssrpro_status"

s = m:section(TypedSection, "ssrpro")
s.anonymous = true

-- ---------------------------------------------------

s:tab("basic",  translate("Base Setting"))


switch = s:taboption("basic",Flag, "enabled", translate("Enable"))
switch.rmempty = false

proxy_mode = s:taboption("basic",ListValue, "proxy_mode", translate("Proxy Mode"))
proxy_mode:value("M", translate("Base on GFW-List Auto Proxy Mode(Recommend)"))
proxy_mode:value("S", translate("Bypassing China Manland IP Mode(Be caution when using P2P download！)"))
proxy_mode:value("G", translate("Global Mode"))
proxy_mode:value("V", translate("Overseas users watch China video website Mode"))

gfwlist_url = s:taboption("basic", Value, "gfwlist_url", translate("GFW-List URL"),	translate("The URL to download GFW-List: default https://raw.githubusercontent.com/gfwlist/gfwlist/master/gfwlist.txt"))
gfwlist_url.datatype = "string"
gfwlist_url.default = ""
gfwlist_url.placeholder = "https://raw.githubusercontent.com/gfwlist/gfwlist/master/gfwlist.txt"
gfwlist_url.optional = true

cronup = s:taboption("basic", Flag, "cron_mode", translate("Auto Update GFW-List"),
	translate(string.format(translate("GFW-List Lines: <strong><font color=\"blue\">%s</font></strong> Lines"), ND)))
cronup.default = 0
cronup.rmempty = false

updatead = s:taboption("basic", Button, "updatead", translate("Manually force update GFW-List"), translate("Note: It needs to download and convert the rules. The background process may takes 60-120 seconds to run. <br / > After completed it would automatically refresh, please do not duplicate click!"))
updatead.inputtitle = translate("Manually force update GFW-List")
updatead.inputstyle = "apply"
updatead.write = function()
	SYS.call("nohup sh /etc/shadowsocksr/up-gfwlist.sh > /tmp/gfwupdate.log 2>&1 &")
end

safe_dns_tcp = s:taboption("basic",Flag, "safe_dns_tcp", translate("DNS uses TCP"),
	translate("Through the server transfer mode inquires DNS pollution prevention (Safer and recommended)"))
safe_dns_tcp.rmempty = false

safe_dns = s:taboption("basic",Value, "safe_dns", translate("Safe DNS"),
	translate("8.8.4.4 is recommended"))
safe_dns.datatype = "ip4addr"
safe_dns.default = "8.8.4.4"
safe_dns.optional = false

safe_dns_port = s:taboption("basic",Value, "safe_dns_port", translate("Safe DNS Port"),
	translate("Foreign DNS on UDP port 53 might be polluted"))
safe_dns_port.datatype = "range(1,65535)"
safe_dns_port.placeholder = "53"
safe_dns_port.optional = true

s:tab("main",  translate("Server Setting"))

server = s:taboption("main",Value, "server", translate("Server Address"))
server.optional = false
server.datatype = "host"
server.rmempty = false

server_port = s:taboption("main",Value, "server_port", translate("Server Port"))
server_port.datatype = "range(1,65535)"
server_port.optional = false
server_port.rmempty = false

password = s:taboption("main",Value, "password", translate("Password"))
password.password = true

method = s:taboption("main",ListValue, "method", translate("Encryption Method"))
method:value("table")
method:value("rc4")
method:value("rc4-md5")
method:value("aes-128-cfb")
method:value("aes-192-cfb")
method:value("aes-256-cfb")
method:value("aes-128-ctr")
method:value("aes-192-ctr")
method:value("aes-256-ctr")
method:value("bf-cfb")
method:value("camellia-128-cfb")
method:value("camellia-192-cfb")
method:value("camellia-256-cfb")
method:value("cast5-cfb")
method:value("des-cfb")
method:value("idea-cfb")
method:value("rc2-cfb")
method:value("seed-cfb")
method:value("salsa20")
method:value("chacha20")
method:value("chacha20-ietf")

protocol = s:taboption("main",ListValue, "protocol", translate("Protocol"))
protocol:value("orgin")
protocol:value("auth_sha1")
protocol:value("auth_sha1_v2")
protocol:value("auth_sha1_v4")
protocol:value("auth_aes128_md5")
protocol:value("auth_aes128_sha1")
protocol:value("auth_chain_a")
protocol:value("auth_chain_b")
protocol:value("auth_chain_c")
protocol:value("auth_chain_d")
protocol:value("auth_chain_e")
protocol:value("auth_chain_f")

protoparam = s:taboption("main",Value, "protoparam", translate("Protocol Param"))
protoparam.optional = true
protoparam.rmempty = true

obfs = s:taboption("main",ListValue, "obfs", translate("Obfs Param"))
obfs:value("plain")
obfs:value("http_simple")
obfs:value("http_post")
obfs:value("tls1.2_ticket_auth")

plugin_param = s:taboption("main",Flag, "plugin_param", translate("Plug-in parameters"),
	translate("Incorrect use of this parameter will cause IP to be blocked. Please use it with care"))
plugin_param:depends("obfs", "http_simple")
plugin_param:depends("obfs", "http_post")
plugin_param:depends("obfs", "tls1.2_ticket_auth")

obfs_param = s:taboption("main",Value, "obfs_param", translate("Confusing plug-in parameters"))
obfs_param.rmempty = true
obfs_param:depends("plugin_param", "1")

s:tab("list",  translate("User-defined GFW-List"))
gfwlist = s:taboption("list", TextValue, "conf")
gfwlist.description = translate("<br />（!）Note: When the domain name is entered and will automatically merge with the online GFW-List. Please manually update the GFW-List list after applying.")
gfwlist.rows = 13
gfwlist.wrap = "off"
gfwlist.cfgvalue = function(self, section)
	return NXFS.readfile(conf) or ""
end
gfwlist.write = function(self, section, value)
	NXFS.writefile(conf, value:gsub("\r\n", "\n"))
end

local addipconf = "/etc/shadowsocksr/addinip.txt"

s:tab("addip",  translate("GFW-List Add-in IP"))
gfwaddin = s:taboption("addip", TextValue, "addipconf")
gfwaddin.description = translate("<br />（!）Note: IP add-in to GFW-List. Such as Telegram Messenger")
gfwaddin.rows = 13
gfwaddin.wrap = "off"
gfwaddin.cfgvalue = function(self, section)
	return NXFS.readfile(addipconf) or ""
end
gfwaddin.write = function(self, section, value)
	NXFS.writefile(addipconf, value:gsub("\r\n", "\n"))
end

s:tab("status",  translate("Status and Tools"))
s:taboption("status", DummyValue,"opennewwindow" , 
	translate("<input type=\"button\" class=\"cbi-button cbi-button-apply\" value=\"IP111.CN\" onclick=\"window.open('http://www.ip111.cn/')\" />"))

t=m:section(TypedSection,"acl_rule",translate("<strong>Client Proxy Mode Settings</strong>"),
translate("Proxy mode settings can be set to specific LAN clients ( <font color=blue> No Proxy, Global Proxy, Game Mode</font>) . Does not need to be set by default."))
t.template="cbi/tblsection"
t.sortable=true
t.anonymous=true
t.addremove=true
e=t:option(Value,"ipaddr",translate("IP Address"))
e.width="40%"
e.datatype="ip4addr"
e.placeholder="0.0.0.0/0"
luci.ip.neighbors({ family = 4 }, function(entry)
	if entry.reachable then
		e:value(entry.dest:string())
	end
end)

e=t:option(ListValue,"filter_mode",translate("Proxy Mode"))
e.width="40%"
e.default="disable"
e.rmempty=false
e:value("disable",translate("No Proxy"))
e:value("global",translate("Global Proxy"))
e:value("game",translate("Game Mode"))

return m

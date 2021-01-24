-- require("luci.tools.webadmin")
mp = Map("openvpn", "OpenVPN Server", translate("An easy config OpenVPN Server Web-UI"))

mp:section(SimpleSection).template = "openvpn/openvpn_status"

s = mp:section(TypedSection, "openvpn")
s.anonymous = true
s.addremove = false

s:tab("basic", translate("Base Setting"))

o = s:taboption("basic", Flag, "enabled", translate("Enable"))

port = s:taboption("basic", Value, "port", translate("Port"))
port.datatype = "range(1,65535)"

ddns = s:taboption("basic", Value, "ddns", translate("WAN DDNS or IP"))
ddns.datatype = "string"
ddns.default = "exmple.com"
ddns.rmempty = false

localnet = s:taboption("basic", Value, "server", translate("Client Network"))
localnet.datatype = "string"
localnet.description = translate("VPN Client Network IP with subnet")

proto = s:taboption("basic", Value, "proto", translate("Protocol"))
proto.datatype = "string"
proto:value("tcp4")
proto:value("udp4")
proto:value("tcp6")
proto:value("udp6")
proto.default = "tcp4"

comp_lzo = s:taboption("basic", Value, "comp_lzo", translate("Compress data with lzo"))
comp_lzo.datatype = "string"
comp_lzo:value("adaptive")
comp_lzo:value("yes")
comp_lzo:value("no")
comp_lzo.default = "adaptive"
comp_lzo.description = translate("Compress data with lzo")

auth_user_pass_verify = s:taboption("basic", Value, "auth_user_pass_verify", translate("User password verify"))
auth_user_pass_verify.datatype = "string"
auth_user_pass_verify.description = translate(
                                        "Default: /etc/openvpn/server/checkpsw.sh via-env, leave it empty to disable")

script_security = s:taboption("basic", Value, "script_security",
                      translate("script_security: to use with user and password"))
script_security.datatype = "range(1,3)"
script_security:value("1")
script_security:value("2")
script_security:value("3")
script_security.description = translate("Default 3, leave it empty to disable")

duplicate_cn = s:taboption("basic", Flag, "duplicate_cn", translate("Multiple clients share certificates and keys"))
duplicate_cn.description = translate(
                               "This option allows multiple clients to connect using the same certificate and key and assign different IP addresses")
client_to_client = s:taboption("basic", Flag, "client_to_client", translate("Clients are mutually accessible"))
client_to_client.description = translate(
                                   "Allow clients to see each other, otherwise multiple clients can only access the server and cannot connect to each other")
username_as_common_name = s:taboption("basic", Flag, "username_as_common_name", translate("Username as common name"))
username_as_common_name.description = translate("Use the UserName provided by the client as the Common Name")
client_cert_not_required = s:taboption("basic", Flag, "client_cert_not_required", translate("Client cert not required"))
client_cert_not_required.description = translate(
                                           "After this option is enabled, the client does not need cert and key. If this option is not enabled, cert and key and user password double verification are required.")

list = s:taboption("basic", DynamicList, "push")
list.title = translate("Client Settings")
list.datatype = "string"
list.description = translate("Set route 192.168.1.0 255.255.255.0 and dhcp-option DNS 192.168.1.1 base on your router")

local o
o = s:taboption("basic", Button, "certificate", translate("OpenVPN Client config file"))
o.inputtitle = translate("Download .ovpn file")
o.description = translate("If you use user password verification only, remember to delete the key and cert.")
o.inputstyle = "reload"
o.write = function()
    luci.sys.call("sh /etc/genovpn.sh 2>&1 >/dev/null")
    Download()
end

s:tab("code", translate("Client code"))
local conf = "/etc/ovpnadd.conf"
local NXFS = require "nixio.fs"
o = s:taboption("code", TextValue, "conf")
o.description = translate(
                    "Here is the code that you want to add to the .ovpn file. If you use user password verification, you need to add auth-user-pass")
o.rows = 13
o.wrap = "off"
o.cfgvalue = function(self, section)
    return NXFS.readfile(conf) or ""
end
o.write = function(self, section, value)
    NXFS.writefile(conf, value:gsub("\r\n", "\n"))
end

s:tab("passwordfile", translate("User and password"))
local pass = "/etc/openvpn/server/psw-file"
local NXFS = require "nixio.fs"
o = s:taboption("passwordfile", TextValue, "pass")
o.description = translate("Each line contains a pair of user and password, separated by a space")
o.rows = 13
o.wrap = "off"
o.cfgvalue = function(self, section)
    return NXFS.readfile(pass) or ""
end
o.write = function(self, section, value)
    NXFS.writefile(pass, value:gsub("\r\n", "\n"))
end

s:tab("checkpsw", translate("Verification script"))
local checkpswconf = "/etc/openvpn/server/checkpsw.sh"
local NXFS = require "nixio.fs"
o = s:taboption("checkpsw", TextValue, "checkpswconf")
o.description = translate("Verification script")
o.rows = 13
o.wrap = "off"
o.cfgvalue = function(self, section)
    return NXFS.readfile(checkpswconf) or ""
end
o.write = function(self, section, value)
    NXFS.writefile(checkpswconf, value:gsub("\r\n", "\n"))
end

function Download()
    local t, e
    t = nixio.open("/tmp/my.ovpn", "r")
    luci.http.header('Content-Disposition', 'attachment; filename="my.ovpn"')
    luci.http.prepare_content("application/octet-stream")
    while true do
        e = t:read(nixio.const.buffersize)
        if (not e) or (#e == 0) then
            break
        else
            luci.http.write(e)
        end
    end
    t:close()
    luci.http.close()
end

o = s:taboption("basic", Button, "gen_cert", translate("Generate OpenVPN Cert"))
o.inputstyle = "apply"
o.inputtitle = translate("Generate OpenVPN Cert")
o.description = translate(
                    "Click this button to regenerate the OpenVPN cert. Generating an OpenVPN cert can take a long time. Please be patient, the OpenVPN server will be restarted automatically after the cert is generated successfully, please do not click this button twice.")
o.write = function()
    os.execute(
        "/etc/init.d/openvpn stop && rm -f /etc/openvpn/ca.crt /etc/openvpn/ca.key /etc/openvpn/client1.crt /etc/openvpn/client1.key /etc/openvpn/dh1024.pem /etc/openvpn/server.crt /etc/openvpn/server.key && /etc/restartopenvpn.sh &")
end

function mp.on_after_commit(self)
    os.execute(
        "uci -q set firewall.openvpn.dest_port=$(uci -q get openvpn.myvpn.port) && uci commit firewall && /etc/init.d/firewall restart >/dev/null 2>&1")
    os.execute("/etc/init.d/openvpn stop && /etc/restartopenvpn.sh &")
end

return mp

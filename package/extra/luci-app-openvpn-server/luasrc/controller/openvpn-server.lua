
module("luci.controller.openvpn-server", package.seeall)

function index()
	if not nixio.fs.access("/etc/config/openvpn") then
		return
	end
	
	local page

	entry({"admin", "vpn", "openvpn-server"}, cbi("openvpn-server/openvpn-server"), _("OpenVPN Server"))
end

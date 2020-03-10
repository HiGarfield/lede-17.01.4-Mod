
module("luci.controller.n2n_v2", package.seeall)

function index()
	if not nixio.fs.access("/etc/config/n2n_v2") then
		return
	end
	
	entry({"admin", "vpn"}, firstchild(), "VPN", 45).dependent = false
	
	local page

	entry({"admin", "vpn", "n2n_v2"}, cbi("n2n_v2"), _("N2N v2 VPN"), 80).dependent=false
end

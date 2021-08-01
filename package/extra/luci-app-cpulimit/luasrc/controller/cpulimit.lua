module("luci.controller.cpulimit", package.seeall)

function index()
	if not nixio.fs.access("/etc/config/cpulimit") then
		return
	end
	
	local page = entry({"admin", "system", "cpulimit"}, cbi("cpulimit"), _("Cpulimit"), 65)
	page.dependent = true	
end

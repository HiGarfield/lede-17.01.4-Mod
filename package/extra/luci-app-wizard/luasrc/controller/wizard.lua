module("luci.controller.wizard", package.seeall)

function index()
	if not nixio.fs.access("/etc/config/wizard") then
		return
	end


	entry({"admin", "initsetup"}, alias("admin", "initsetup", "wizard"), _("Inital Setup"), 29).index = true
	entry({"admin", "initsetup", "wizard"}, cbi("wizard/wizard"), _("Quick Setup"), 1)
end

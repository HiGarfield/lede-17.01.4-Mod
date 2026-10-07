--[[
Copyright (C) 2020 Nick Hainke <vincent@systemli.org>

Backport of the JS based luci-app-dawn (openwrt/luci, master) to the
Lua based LuCI found in LEDE 17.01.

Licensed under the Apache License, Version 2.0
]]--

module("luci.controller.dawn", package.seeall)

function index()
	-- Status -> DAWN (Network Overview / Hearing Map)
	entry({"admin", "status", "dawn"}, firstchild(), _("DAWN"), 60)

	entry({"admin", "status", "dawn", "network_overview"},
		template("dawn/network_overview"), _("Network Overview"), 1)

	entry({"admin", "status", "dawn", "hearing_map"},
		template("dawn/hearing_map"), _("Hearing Map"), 2)

	-- Fragments without menu title (a node without title is not visible in
	-- the menu, see dispatcher.lua node_visible()). They render the same
	-- templates as the pages above but without header and footer and are
	-- polled by htdocs/luci-static/resources/dawn.js.
	entry({"admin", "status", "dawn", "network_overview_body"},
		call("action_network_overview_body")).leaf = true

	entry({"admin", "status", "dawn", "hearing_map_body"},
		call("action_hearing_map_body")).leaf = true

	-- Network -> DAWN (configuration)
	entry({"admin", "network", "dawn"}, cbi("dawn/dawn"), _("DAWN"), 60)
end

function action_network_overview_body()
	luci.template.render("dawn/network_overview_body")
end

function action_hearing_map_body()
	luci.template.render("dawn/hearing_map_body")
end

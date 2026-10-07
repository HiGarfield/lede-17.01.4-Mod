--[[
Copyright (C) 2020 Nick Hainke <vincent@systemli.org>

Model layer of the LEDE 17.01 backport of luci-app-dawn.

It queries the "dawn" ubus object (get_network / get_hearing_map), formats
the raw values exactly like the upstream JS version does and returns plain
tables which are consumed

  * by the Lua templates (luasrc/view/dawn/*.htm) for the initial render
  * and by the JSON endpoints (controller/dawn.lua) for the periodic refresh

Licensed under the Apache License, Version 2.0
]]--

module("luci.model.dawn", package.seeall)

local util = require "luci.util"
local sys  = require "luci.sys"
local i18n = require "luci.i18n"

-- Refresh interval (seconds) used by the Javascript part
local REFRESH = 5

local function translate(s)
	return i18n.translate(s) or s
end

local function tobool(v)
	return (v ~= nil and v ~= false and v ~= 0 and v ~= "0" and v ~= "")
end

local function sorted_keys(t)
	local keys = {}
	for k in pairs(t) do
		keys[#keys + 1] = tostring(k)
	end
	table.sort(keys)
	return keys
end

-- Call a ubus method and return the decoded reply (or nil on error).
local function ubus_call(object, method)
	local ok, res = pcall(util.ubus, object, method, {})
	if ok and type(res) == "table" then
		return res
	end

	-- Fallback for setups where the Lua ubus binding is not usable
	local out = sys.exec(string.format("ubus call %s %s 2>/dev/null",
		object, method))

	if out and #out > 0 then
		local ok2, jsonc = pcall(require, "luci.jsonc")
		if ok2 and jsonc then
			local data = jsonc.parse(out)
			if type(data) == "table" then
				return data
			end
		end
	end

	return nil
end

-- Is the DAWN daemon reachable through ubus?
function is_available()
	local ok, sig = pcall(util.ubus, "dawn")

	if ok and type(sig) == "table" and type(sig["dawn"]) == "table" then
		return tobool(sig["dawn"]["get_network"]) and
		       tobool(sig["dawn"]["get_hearing_map"])
	end

	local out = sys.exec("ubus list dawn 2>/dev/null")
	return (out and out:match("dawn") ~= nil)
end

-- Hostname hints (DHCP leases, /etc/hosts) - the upstream version uses
-- luci-rpc:getHostHints() for this, sys.net.host_hints() is its Lua
-- counterpart in LEDE 17.01.
local function host_hints()
	local hints = {}
	local ok, res = pcall(sys.net.host_hints)

	if ok and type(res) == "table" then
		for k, v in pairs(res) do
			if type(v) == "table" and v.name and v.name ~= "" then
				hints[tostring(k):lower()] = { name = v.name }
			end
		end
	end

	return hints
end

local function hostname_from_mac(mac, hints)
	mac = tostring(mac)

	if type(hints) == "table" then
		local hint = hints[mac:lower()]
		if hint and hint.name and hint.name ~= "" then
			return string.format("%s (%s)", hint.name, mac)
		end
	end

	return mac
end

-- Formatting helpers, they mirror dawn-common.js / the upstream views

-- The package ships luci.tools.ieee80211; use it when present and fall back
-- to the identical local implementation otherwise.
local ieee80211 = nil
do
	local ok, mod = pcall(require, "luci.tools.ieee80211")
	if ok and mod and type(mod.frequency_to_channel) == "function" then
		ieee80211 = mod
	end
end

local function channel_from_frequency(freq)
	freq = tonumber(freq) or 0

	if ieee80211 then
		return ieee80211.frequency_to_channel(freq)
	end

	if freq <= 2400 then
		return 0
	elseif freq == 2484 then
		return 14
	elseif freq < 2484 then
		return (freq - 2407) / 5
	elseif freq >= 4910 and freq <= 4980 then
		return (freq - 4000) / 5
	elseif freq <= 5935 then
		return (freq - 5000) / 5
	elseif freq <= 45000 then
		return (freq - 5950) / 5
	elseif freq >= 58320 and freq <= 64800 then
		return (freq - 56160) / 2160
	end

	return 0
end

local function format_channel(freq)
	local ch = channel_from_frequency(freq)
	if ch == math.floor(ch) then
		return tostring(math.floor(ch))
	end
	return string.format("%.1f", ch)
end

local function format_frequency(freq)
	freq = tonumber(freq) or 0

	return string.format("%.3f GHz (%s: %s)",
		freq / 1000, translate("Channel"), format_channel(freq))
end

local function format_utilization(u)
	return string.format("%.2f%%", (tonumber(u) or 0) / 2.55)
end

local function available_text(v)
	return tobool(v) and translate("Available") or translate("Not available")
end

local function yes_text(v)
	return tobool(v) and translate("Yes") or translate("No")
end

local function num_or(v, alt)
	local n = tonumber(v)
	return n and n or alt
end

-- Table headers (translated), used by the templates and by the JS refresh

function network_overview_headers()
	return {
		{ title = translate("Access Point") },
		{ title = translate("Interface") },
		{ title = translate("MAC") },
		{ title = translate("Utilization") },
		{ title = translate("Frequency") },
		{ title = translate("Stations Connected") },
		{ title = translate("HT"),  tooltip = translate("High Throughput") },
		{ title = translate("VHT"), tooltip = translate("Very High Throughput") },
		{ title = translate("Clients") }
	}
end

function client_headers()
	return {
		{ title = translate("Client") },
		{ title = translate("HT"),  tooltip = translate("High Throughput") },
		{ title = translate("VHT"), tooltip = translate("Very High Throughput") },
		{ title = translate("Signal") }
	}
end

function hearing_map_headers()
	return {
		{ title = translate("Client") },
		{ title = translate("Access Point") },
		{ title = translate("Frequency") },
		{ title = translate("HT"),  tooltip = translate("High Throughput") },
		{ title = translate("VHT"), tooltip = translate("Very High Throughput") },
		{ title = translate("Signal") },
		{ title = translate("RCPI"), tooltip = translate("Received Channel Power Indicator") },
		{ title = translate("RSNI"), tooltip = translate("Received Signal to Noise Indicator") },
		{ title = translate("Channel Utilization") },
		{ title = translate("Connected to Network") },
		{ title = translate("Score") }
	}
end

function refresh_interval()
	return REFRESH
end

-- Status -> DAWN -> Network Overview
function network_overview_data()
	local rv = {
		available      = false,
		refresh        = REFRESH,
		headers        = network_overview_headers(),
		client_headers = client_headers(),
		networks       = {}
	}

	if not is_available() then
		return rv
	end

	local net = ubus_call("dawn", "get_network")
	if type(net) ~= "table" then
		return rv
	end

	local hints = host_hints()
	local networks = {}

	for _, ssid in ipairs(sorted_keys(net)) do
		local aps_tbl = net[ssid]
		local aps = {}

		if type(aps_tbl) == "table" then
			for _, mac in ipairs(sorted_keys(aps_tbl)) do
				local ap = aps_tbl[mac]
				local clients = {}

				if type(ap) == "table" then
					for _, cmac in ipairs(sorted_keys(ap)) do
						local client = ap[cmac]
						if type(client) == "table" then
							clients[#clients + 1] = {
								client = hostname_from_mac(cmac, hints),
								ht     = available_text(client.ht),
								vht    = available_text(client.vht),
								signal = num_or(client.signal, 0)
							}
						end
					end

					local host = ap.hostname
					if host == nil or host == "" then
						host = mac
					end

					aps[#aps + 1] = {
						host    = host,
						iface   = ap.iface or "",
						mac     = tostring(mac),
						util    = format_utilization(ap.channel_utilization),
						freq    = format_frequency(ap.freq),
						num_sta = num_or(ap.num_sta, 0),
						ht      = available_text(ap.ht_support),
						vht     = available_text(ap.vht_support),
						clients = clients
					}
				end
			end
		end

		networks[#networks + 1] = {
			ssid   = tostring(ssid),
			aps    = aps,
			empty  = translate("No access points available.")
		}
	end

	rv.available = true
	rv.networks  = networks

	return rv
end

-- Status -> DAWN -> Hearing Map
function hearing_map_data()
	local rv = {
		available = false,
		refresh   = REFRESH,
		headers   = hearing_map_headers(),
		networks  = {}
	}

	if not is_available() then
		return rv
	end

	local map = ubus_call("dawn", "get_hearing_map")
	local net = ubus_call("dawn", "get_network")

	if type(map) ~= "table" or type(net) ~= "table" then
		return rv
	end

	-- Access point names and the set of connected clients are taken from
	-- the network overview data, exactly like the upstream view does.
	local ap_hints, connected = {}, {}

	for ssid, aps_tbl in pairs(net) do
		if type(aps_tbl) == "table" then
			connected[ssid] = {}
			for mac, ap in pairs(aps_tbl) do
				if type(ap) == "table" then
					mac = tostring(mac):lower()
					if ap.hostname and ap.hostname ~= "" then
						ap_hints[mac] = { name = ap.hostname }
					end
					for cmac, client in pairs(ap) do
						if type(client) == "table" then
							connected[ssid][tostring(cmac):lower()] = true
						end
					end
				end
			end
		end
	end

	local hints = host_hints()
	local networks = {}

	for _, ssid in ipairs(sorted_keys(map)) do
		local clients_tbl = map[ssid]
		local rows = {}

		if type(clients_tbl) == "table" then
			for _, cmac in ipairs(sorted_keys(clients_tbl)) do
				local aps_tbl = clients_tbl[cmac]

				if type(aps_tbl) == "table" then
					for _, amac in ipairs(sorted_keys(aps_tbl)) do
						local e = aps_tbl[amac]

						-- entries without frequency are skipped (upstream
						-- checks for frequency == 0 as well)
						if type(e) == "table" and tonumber(e.freq) ~= 0 then
							rows[#rows + 1] = {
								client    = hostname_from_mac(cmac, hints),
								ap        = hostname_from_mac(amac, ap_hints),
								freq      = format_frequency(e.freq),
								ht        = available_text(
									tobool(e.ht_capabilities) and tobool(e.ht_support)),
								vht       = available_text(
									tobool(e.vht_capabilities) and tobool(e.vht_support)),
								signal    = num_or(e.signal, 0),
								rcpi      = num_or(e.rcpi, 0),
								rsni      = num_or(e.rsni, 0),
								util      = format_utilization(e.channel_utilization),
								connected = yes_text(
									connected[ssid] and
									connected[ssid][tostring(cmac):lower()]),
								score     = num_or(e.score, 0)
							}
						end
					end
				end
			end
		end

		networks[#networks + 1] = {
			ssid  = tostring(ssid),
			rows  = rows,
			empty = translate("No clients connected.")
		}
	end

	rv.available = true
	rv.networks  = networks

	return rv
end

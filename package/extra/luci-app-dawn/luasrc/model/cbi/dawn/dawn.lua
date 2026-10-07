--[[
Copyright (C) 2020 Nick Hainke <vincent@systemli.org>

Configuration page of the LEDE 17.01 backport of luci-app-dawn.
Ported from htdocs/luci-static/resources/view/network/dawn.js

Note: the Lua based CBI of LEDE 17.01 knows no "placeholder" attribute,
therefore the upstream placeholder texts are appended to the option
description as "(Default Value: ...)".

Licensed under the Apache License, Version 2.0
]]--

local function dflt(desc, value)
	return desc .. " (" .. translate("Default Value") .. ": " .. value .. ")"
end

-- LEDE 17.01's cbi.js implements the "integer" datatype as !!Int(value) which
-- evaluates to false for the value 0. Every metric that may legitimately be 0
-- (several of them default to 0) would therefore be rejected by the client
-- side check. Those options get no datatype but a server side validator which
-- handles 0 and negative values correctly.
local function int_opt(o)
	o.validate = function(self, value)
		if value == nil or value == "" then
			return value
		end

		local n = tonumber(value)
		if n == nil or math.floor(n) ~= n then
			return nil, translate("Expecting an integer value")
		end

		return value
	end
end

-- IEEE 802.11 status codes, used for the deny association / authentication
-- reason options.
local status_codes = {
	{ '0', 'SUCCESS' },
	{ '1', 'UNSPECIFIED_FAILURE' },
	{ '2', 'TDLS_WAKEUP_ALTERNATE' },
	{ '3', 'TDLS_WAKEUP_REJECT' },
	{ '5', 'SECURITY_DISABLED' },
	{ '6', 'UNACCEPTABLE_LIFETIME' },
	{ '7', 'NOT_IN_SAME_BSS' },
	{ '10', 'CAPS_UNSUPPORTED' },
	{ '11', 'REASSOC_NO_ASSOC' },
	{ '12', 'ASSOC_DENIED_UNSPEC' },
	{ '13', 'NOT_SUPPORTED_AUTH_ALG' },
	{ '14', 'UNKNOWN_AUTH_TRANSACTION' },
	{ '15', 'CHALLENGE_FAIL' },
	{ '16', 'AUTH_TIMEOUT' },
	{ '17', 'AP_UNABLE_TO_HANDLE_NEW_STA' },
	{ '18', 'ASSOC_DENIED_RATES' },
	{ '19', 'ASSOC_DENIED_NOSHORT' },
	{ '22', 'SPEC_MGMT_REQUIRED' },
	{ '23', 'PWR_CAPABILITY_NOT_VALID' },
	{ '24', 'SUPPORTED_CHANNEL_NOT_VALID' },
	{ '25', 'ASSOC_DENIED_NO_SHORT_SLOT_TIME' },
	{ '27', 'ASSOC_DENIED_NO_HT' },
	{ '28', 'R0KH_UNREACHABLE' },
	{ '29', 'ASSOC_DENIED_NO_PCO' },
	{ '30', 'ASSOC_REJECTED_TEMPORARILY' },
	{ '31', 'ROBUST_MGMT_FRAME_POLICY_VIOLATION' },
	{ '32', 'UNSPECIFIED_QOS_FAILURE' },
	{ '33', 'DENIED_INSUFFICIENT_BANDWIDTH' },
	{ '34', 'DENIED_POOR_CHANNEL_CONDITIONS' },
	{ '35', 'DENIED_QOS_NOT_SUPPORTED' },
	{ '37', 'REQUEST_DECLINED' },
	{ '38', 'INVALID_PARAMETERS' },
	{ '39', 'REJECTED_WITH_SUGGESTED_CHANGES' },
	{ '40', 'INVALID_IE' },
	{ '41', 'GROUP_CIPHER_NOT_VALID' },
	{ '42', 'PAIRWISE_CIPHER_NOT_VALID' },
	{ '43', 'AKMP_NOT_VALID' },
	{ '44', 'UNSUPPORTED_RSN_IE_VERSION' },
	{ '45', 'INVALID_RSN_IE_CAPAB' },
	{ '46', 'CIPHER_REJECTED_PER_POLICY' },
	{ '47', 'TS_NOT_CREATED' },
	{ '48', 'DIRECT_LINK_NOT_ALLOWED' },
	{ '49', 'DEST_STA_NOT_PRESENT' },
	{ '50', 'DEST_STA_NOT_QOS_STA' },
	{ '51', 'ASSOC_DENIED_LISTEN_INT_TOO_LARGE' },
	{ '52', 'INVALID_FT_ACTION_FRAME_COUNT' },
	{ '53', 'INVALID_PMKID' },
	{ '54', 'INVALID_MDIE' },
	{ '55', 'INVALID_FTIE' },
	{ '56', 'REQUESTED_TCLAS_NOT_SUPPORTED' },
	{ '57', 'INSUFFICIENT_TCLAS_PROCESSING_RESOURCES' },
	{ '58', 'TRY_ANOTHER_BSS' },
	{ '59', 'GAS_ADV_PROTO_NOT_SUPPORTED' },
	{ '60', 'NO_OUTSTANDING_GAS_REQ' },
	{ '61', 'GAS_RESP_NOT_RECEIVED' },
	{ '62', 'STA_TIMED_OUT_WAITING_FOR_GAS_RESP' },
	{ '63', 'GAS_RESP_LARGER_THAN_LIMIT' },
	{ '64', 'REQ_REFUSED_HOME' },
	{ '65', 'ADV_SRV_UNREACHABLE' },
	{ '67', 'REQ_REFUSED_SSPN' },
	{ '68', 'REQ_REFUSED_UNAUTH_ACCESS' },
	{ '72', 'INVALID_RSNIE' },
	{ '73', 'U_APSD_COEX_NOT_SUPPORTED' },
	{ '74', 'U_APSD_COEX_MODE_NOT_SUPPORTED' },
	{ '75', 'BAD_INTERVAL_WITH_U_APSD_COEX' },
	{ '76', 'ANTI_CLOGGING_TOKEN_REQ' },
	{ '77', 'FINITE_CYCLIC_GROUP_NOT_SUPPORTED' },
	{ '78', 'CANNOT_FIND_ALT_TBTT' },
	{ '79', 'TRANSMISSION_FAILURE' },
	{ '80', 'REQ_TCLAS_NOT_SUPPORTED' },
	{ '81', 'TCLAS_RESOURCES_EXCHAUSTED' },
	{ '82', 'REJECTED_WITH_SUGGESTED_BSS_TRANSITION' },
	{ '83', 'REJECT_WITH_SCHEDULE' },
	{ '84', 'REJECT_NO_WAKEUP_SPECIFIED' },
	{ '85', 'SUCCESS_POWER_SAVE_MODE' },
	{ '86', 'PENDING_ADMITTING_FST_SESSION' },
	{ '87', 'PERFORMING_FST_NOW' },
	{ '88', 'PENDING_GAP_IN_BA_WINDOW' },
	{ '89', 'REJECT_U_PID_SETTING' },
	{ '92', 'REFUSED_EXTERNAL_REASON' },
	{ '93', 'REFUSED_AP_OUT_OF_MEMORY' },
	{ '94', 'REJECTED_EMERGENCY_SERVICE_NOT_SUPPORTED' },
	{ '95', 'QUERY_RESP_OUTSTANDING' },
	{ '96', 'REJECT_DSE_BAND' },
	{ '97', 'TCLAS_PROCESSING_TERMINATED' },
	{ '98', 'TS_SCHEDULE_CONFLICT' },
	{ '99', 'DENIED_WITH_SUGGESTED_BAND_AND_CHANNEL' },
	{ '100', 'MCCAOP_RESERVATION_CONFLICT' },
	{ '101', 'MAF_LIMIT_EXCEEDED' },
	{ '102', 'MCCA_TRACK_LIMIT_EXCEEDED' },
	{ '103', 'DENIED_DUE_TO_SPECTRUM_MANAGEMENT' },
	{ '104', 'ASSOC_DENIED_NO_VHT' },
	{ '105', 'ENABLEMENT_DENIED' },
	{ '106', 'RESTRICTION_FROM_AUTHORIZED_GDB' },
	{ '107', 'AUTHORIZATION_DEENABLED' },
	{ '112', 'FILS_AUTHENTICATION_FAILURE' },
	{ '113', 'UNKNOWN_AUTHENTICATION_SERVER' },
}

local function status_code(section, name, title, desc)
	local o = section:option(ListValue, name, title, desc)
	for _, c in ipairs(status_codes) do
		o:value(c[1], c[2])
	end
	return o
end

m = Map("dawn", translate("DAWN"), translate("DAWN Form Configuration."))

--
-- Local
--

s = m:section(TypedSection, "local", translate("Local"))
s.anonymous = true
s.addremove = false

o = s:option(ListValue, "loglevel",
	translate("Log Level"),
	translate("Verbosity of messages in syslog"))

o:value("0", translate("Deeper tracing to fix bugs - for debugging"))
o:value("1", translate("More info to help trace where algorithms may be going wrong - for debugging"))
o:value("2", translate("Reporting on standard behaviour"))
o:value("3", translate("Standard behaviour always worth reporting"))
o:value("4", translate("Something appears wrong, but recoverable"))
o:value("5", translate("Serious malfunction / unexpected behaviour"))
o.default = "2"

--
-- Hostapd
--

s = m:section(TypedSection, "hostapd", translate("Hostapd"))
s.anonymous = true
s.addremove = false

o = s:option(Value, "hostapd_dir",
	translate("Hostapd dir"),
	translate("Path to hostapd runtime information"))
o.default = "/var/run/hostapd"

--
-- Network
--

s = m:section(TypedSection, "network", translate("Network"))
s.anonymous = true
s.addremove = false

o = s:option(Value, "broadcast_ip",
	translate("Broadcast IP"),
	translate("IP address for broadcast and multicast"))
o.datatype = "ipaddr"

o = s:option(Value, "broadcast_port",
	translate("Broadcast PORT"),
	dflt(translate("IP port for broadcast and multicast"), "1025"))
o.datatype = "port"
o.default = "1025"

o = s:option(ListValue, "network_option",
	translate("Network option"),
	translate("Method of networking between DAWN instances"))
o:value("0", translate("Broadcast"))
o:value("1", translate("Multicast"))
o:value("2", translate("TCP with UMDNS discovery"))
o:value("3", translate("TCP w/out UMDNS discovery"))
o.default = "2"

o = s:option(Value, "server_ip",
	translate("Server IP"),
	translate("IP address when not using UMDNS"))
o.datatype = "ipaddr"

o = s:option(Value, "tcp_port",
	translate("TCP port"),
	dflt(translate("Port for TCP networking"), "1026"))
o.datatype = "port"
o.default = "1026"

--
-- Times
--

s = m:section(TypedSection, "times", translate("Times"),
	translate("All timer values are in seconds. They are the main mechanism for DAWN collecting and managing much of the data that it relies on."))
s.anonymous = true
s.addremove = false

o = s:option(Value, "con_timeout",
	translate("Connection Timeout"),
	dflt(translate("Timespan until a connection is seen as disconnected"), "60"))
o.datatype = "uinteger"

o = s:option(Value, "remove_ap",
	translate("Remove AP"),
	dflt(translate("Timer to remove expired AP entries from core data set"), "460"))
o.datatype = "uinteger"

o = s:option(Value, "remove_client",
	translate("Remove Client"),
	dflt(translate("Timer to remove expired client entries from core data set"), "15"))
o.datatype = "uinteger"

o = s:option(Value, "remove_probe",
	translate("Remove Probe"),
	dflt(translate("Timer to remove expired PROBE and BEACON entries from core data set"), "30"))
o.datatype = "uinteger"

o = s:option(Value, "update_beacon_reports",
	translate("Update Beacon reports"),
	dflt(translate("Timer to ask all connected clients for a new BEACON REPORT"), "20"))
o.datatype = "uinteger"

o = s:option(Value, "update_chan_util",
	translate("Update Channel utilization"),
	dflt(translate("Timer to get recent channel utilization figure for each local BSSID"), "5"))
o.datatype = "uinteger"

o = s:option(Value, "update_client",
	translate("Update Client"),
	dflt(translate("Timer to refresh local connection information and send revised NEIGHBOR REPORT to all clients"), "10"))
o.datatype = "uinteger"

o = s:option(Value, "update_hostapd",
	translate("Update Hostapd"),
	dflt(translate("Timer to (re-)register for hostapd messages for each local BSSID"), "10"))
o.datatype = "uinteger"

o = s:option(Value, "update_tcp_con",
	translate("Update TCP connections"),
	dflt(translate("Timer to refresh / remove the TCP connections to other DAWN instances found via uMDNS"), "10"))
o.datatype = "uinteger"

--
-- Global Metric
--

s = m:section(NamedSection, "global", "metric", translate("Global Metric"))
s.addremove = false

o = s:option(Value, "bandwidth_threshold",
	translate("Bandwidth Threshold (Mbits/s)"),
	dflt(translate("Maximum reported AP-client bandwidth permitted when kicking. Set to zero to disable the check."), "6"))
o.datatype = "uinteger"

o = s:option(Value, "chan_util_avg_period",
	translate("Average channel utilization"),
	dflt(translate("Number of sampling periods to average channel utilization values over"), "3"))
o.datatype = "uinteger"

o = status_code(s, "deny_assoc_reason",
	translate("Deny Association reason"),
	dflt(translate("802.11 code used when ASSOCIATION is denied"), "AP_UNABLE_TO_HANDLE_NEW_STA"))
o.default = "17"

o = status_code(s, "deny_auth_reason",
	translate("Deny auth reason"),
	dflt(translate("802.11 code used when AUTHENTICATION is denied"), "UNSPECIFIED_FAILURE"))
o.default = "1"

o = s:option(Value, "disassoc_nr_length",
	translate("Disassociate Neighbor Report length"),
	dflt(translate("Number of entries to include in a 802.11v DISASSOCIATE Neighbor Report"), "6"))
o.datatype = "uinteger"

o = s:option(Value, "duration",
	translate("DURATION"),
	dflt(translate("802.11k BEACON request DURATION parameter"), "0"))
o.datatype = "uinteger"

o = s:option(Flag, "eval_assoc_req",
	translate("Evaluated Association Req"),
	translate("Control whether ASSOCIATION frames are evaluated for rejection"))

o = s:option(Flag, "eval_auth_req",
	translate("Evaluated Auth Req"),
	translate("Control whether AUTHENTICATION frames are evaluated for rejection"))

o = s:option(Flag, "eval_probe_req",
	translate("Evaluated Probe Req"),
	translate("Control whether PROBE frames are evaluated for rejection"))

o = s:option(ListValue, "kicking",
	translate("Kicking"),
	dflt(translate("Method to select clients to move to better AP"), "Both"))
o:value("0", translate("Disabled"))
o:value("1", translate("RSSI Comparison"))
o:value("2", translate("Absolute RSSI"))
o:value("3", translate("Both"))

o = s:option(Value, "kicking_threshold",
	translate("Kicking Threshold"),
	dflt(translate("Minimum score difference to consider kicking to alternate AP"), "20"))
o.datatype = "uinteger"

o = s:option(Value, "max_station_diff",
	translate("Max Station Diff"),
	dflt(translate('Number of connected stations to consider "better" for use_station_count'), "1"))
o.datatype = "uinteger"

o = s:option(Value, "min_number_to_kick",
	translate("Min Number To Kick"),
	dflt(translate("Number of consecutive times a client should be evaluated as ready to kick before actually doing it"), "3"))
o.datatype = "uinteger"

o = s:option(Value, "min_probe_count",
	translate("Min Probe Count"),
	dflt(translate("Number of times a client should retry PROBE before acceptance"), "3"))
o.datatype = "uinteger"

o = s:option(Value, "neighbors",
	translate("Neighbors"),
	translate('Space separated list of MACS to use in "static" AP Neighbor Report'))

o = s:option(Value, "rrm_mode",
	translate("RRM Mode"),
	dflt(translate("Preferred order for using Passive, Active or Table 802.11k BEACON information"), "PAT"))

o = s:option(ListValue, "set_hostapd_nr",
	translate("Set Hostapd Neighbor Report"),
	dflt(translate("Method used to set Neighbor Report on AP"), "Disabled"))
o:value("0", translate("Disabled"))
o:value("1", translate("Static"))
o:value("2", translate("Dynamic"))

o = s:option(Flag, "use_station_count",
	translate("Use Station Count"),
	translate("Compare connected station counts when considering kicking"))

--
-- Band Metrics
--

local bands = {
	{ "802_11g", translate("2.4G Band Metric"), "80" },
	{ "802_11a", translate("5G Band Metric"), "100" }
}

for _, band in ipairs(bands) do
	s = m:section(NamedSection, band[1], "metric", band[2])
	s.addremove = false

	o = s:option(Value, "ap_weight",
		translate("Ap Weight"),
		dflt(translate("Per AP weighting"), "0"))
	o.datatype = "uinteger"

	o = s:option(Value, "chan_util",
		translate("Channel Utilization"),
		dflt(translate("Score increment if channel utilization is below chan_util_val"), "0"))
	int_opt(o)

	o = s:option(Value, "chan_util_val",
		translate("Channel Utilization Value"),
		dflt(translate("Upper threshold for good channel utilization"), "140"))
	o.datatype = "uinteger"

	o = s:option(Value, "ht_support",
		translate("HT Support"),
		dflt(translate("Score increment if HT is supported"), "5"))
	int_opt(o)

	o = s:option(Value, "initial_score",
		translate("Initial Score"),
		dflt(translate("Base score for AP based on operating band"), band[3]))
	int_opt(o)

	o = s:option(Value, "low_rssi",
		translate("Low RSSI"),
		dflt(translate("Score addition when signal is below threshold"), "-15"))
	int_opt(o)

	o = s:option(Value, "low_rssi_val",
		translate("Low RSSI Value"),
		dflt(translate("Threshold for bad RSSI"), "-80"))
	int_opt(o)

	o = s:option(Value, "max_chan_util",
		translate("Max Channel Utilization"),
		dflt(translate("Score increment if channel utilization is above max_chan_util_val"), "-15"))
	int_opt(o)

	o = s:option(Value, "max_chan_util_val",
		translate("Max Channel Utilization Value"),
		dflt(translate("Lower threshold for bad channel utilization"), "170"))
	o.datatype = "uinteger"

	o = s:option(Value, "no_ht_support",
		translate("No HT Support"),
		dflt(translate("Score increment if HT is not supported"), "0"))
	int_opt(o)

	o = s:option(Value, "no_vht_support",
		translate("No VHT Support"),
		dflt(translate("Score increment if VHT is not supported"), "0"))
	int_opt(o)

	o = s:option(Value, "rssi_center",
		translate("RSSI Center"),
		dflt(translate("Midpoint for weighted RSSI evaluation"), "-70"))
	int_opt(o)

	o = s:option(Value, "rssi",
		translate("RSSI"),
		dflt(translate("Score addition when signal exceeds threshold"), "-15"))
	int_opt(o)

	o = s:option(Value, "rssi_val",
		translate("RSSI Value"),
		dflt(translate("Threshold for a good RSSI"), "-60"))
	int_opt(o)

	o = s:option(Value, "rssi_weight",
		translate("RSSI Weight"),
		dflt(translate("Per dB increment for weighted RSSI evaluation"), "0"))
	int_opt(o)

	o = s:option(Value, "vht_support",
		translate("VHT Support"),
		dflt(translate("Score increment if VHT is supported"), "5"))
	int_opt(o)
end

return m

--[[
IEEE 802.11 helpers for the LEDE 17.01 backport of luci-app-dawn.

LEDE 17.01 ships no luci.tools.ieee80211, the older Lua based luci-app-dawn
shipped its own copy, so this package does the same. The channel arithmetic
was taken from the current upstream JS implementation (dawn-common.js) which,
in contrast to the very first Lua version, also knows about the 6 GHz band.

Licensed under the Apache License, Version 2.0
]]--

module("luci.tools.ieee80211", package.seeall)

function frequency_to_channel(freq)
	freq = tonumber(freq) or 0

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
	else
		return 0
	end
end

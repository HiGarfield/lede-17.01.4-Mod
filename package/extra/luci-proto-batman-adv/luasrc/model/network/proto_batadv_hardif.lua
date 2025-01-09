local netmod = luci.model.network
local interface = luci.model.network.interface
local device = luci.util.class(netmod.interface)

local proto = netmod:register_protocol("batadv_hardif")

function proto.get_i18n(self)
	return luci.i18n.translate("Batman Interface")
end

function proto.ifname(self)
	local base = netmod._M.protocol
	local ifname = base.ifname(self)
	if ifname == nil then
		ifname = "batadv_hardif-" .. self.sid
	end
	return ifname
end

function proto.get_interface(self)
	return interface(self:ifname(), self)
end

function proto.opkg_package(self)
	return "kmod-batman-adv"
end

function proto.is_floating(self)
	return true
end

function proto.is_virtual(self)
	return true
end

netmod:register_pattern_virtual("^bat.+$")



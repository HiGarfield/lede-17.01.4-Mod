
local map, section = ...

section:tab("mesh", translate("Mesh Routing"))

o = section:taboption("general", ListValue, "gw_mode", translate("Gateway Mode"), translate("A batman-adv node can either run in server mode (sharing its internet connection with the mesh) or in client mode (searching for the most suitable internet connection in the mesh) or having the gateway support turned off entirely (which is the default setting)."))
o:value("off", translate("Off"))
o:value("server", translate("Server"))
o:value("client", translate("Client"))
o.default = "off"

o = section:taboption("mesh", ListValue, "routing_algo", translate("Routing Algorithm"), translate("The algorithm that is used to discover mesh routes"))
o:value("BATMAN_IV", "BATMAN_IV")
o:value("BATMAN_V", "BATMAN_V")
o.default = "BATMAN_IV"

o = section:taboption("mesh", Flag, "aggregated_ogms", translate("Aggregate Originator Messages"), translate("reduces overhead by collecting and aggregating originator messages in a single packet rather than many small ones"))
o.ucioption = "aggregated_ogms"
o.default = o.enabled

o = section:taboption("mesh", Value, "orig_interval", translate("Originator Interval"), translate("The value specifies the interval (milliseconds) in which batman-adv floods the network with its protocol information."))
o.default = "1000"
o.datatype = "min(1)"

o = section:taboption("mesh", Flag, "ap_isolation", translate("Access Point Isolation"), translate("Prevents one wireless client to talk to another. This setting only affects packets without any VLAN tag (untagged packets)."))
o.ucioption = "ap_isolation"
o.default = o.disabled

o = section:taboption("mesh", Flag, "bonding", translate("Bonding Mode"), translate("When running the mesh over multiple WiFi interfaces per node batman-adv is capable of optimizing the traffic flow to gain maximum performance."))
o.ucioption = "bonding"
o.default = o.disabled

o = section:taboption("mesh", Flag, "bridge_loop_avoidance", translate("Avoid Bridge Loops"), translate("In bridged LAN setups it is advisable to enable the bridge loop avoidance in order to avoid broadcast loops that can bring the entire LAN to a standstill."))
o.ucioption = "bridge_loop_avoidance"
o.default = o.enabled

o = section:taboption("mesh", Flag, "distributed_arp_table", translate("Distributed ARP Table"), translate("When enabled the distributed ARP table forms a mesh-wide ARP cache that helps non-mesh clients to get ARP responses much more reliably and without much delay."))
o.ucioption = "distributed_arp_table"
o.default = o.enabled

o = section:taboption("mesh", Flag, "fragmentation", translate("Fragmentation"), translate("Batman-adv has a built-in layer 2 fragmentation for unicast data flowing through the mesh which will allow to run batman-adv over interfaces / connections that don\'t allow to increase the MTU beyond the standard Ethernet packet size of 1500 bytes. When the fragmentation is enabled batman-adv will automatically fragment over-sized packets and defragment them on the other end. Per default fragmentation is enabled and inactive if the packet fits but it is possible to deactivate the fragmentation entirely."))
o.ucioption = "fragmentation"
o.default = o.enabled

o = section:taboption("mesh", Value, "hop_penalty", translate("Hop Penalty"), translate("The hop penalty setting allows to modify batman-adv\'s preference for multihop routes vs. short routes. The value is applied to the TQ of each forwarded OGM, thereby propagating the cost of an extra hop (the packet has to be received and retransmitted which costs airtime)"))
o.ucioption = "hop_penalty"
o.datatype = "min(1)"
o.default = "30"

o = section:taboption("mesh", Flag, "multicast_mode", translate("Multicast Mode"), translate("Enables more efficient, group aware multicast forwarding infrastructure in batman-adv."))
o.ucioption = "multicast_mode"
o.default = o.enabled

o = section:taboption("mesh", Flag, "network_coding", translate("Network Coding"), translate("When enabled network coding increases the WiFi throughput by combining multiple frames into a single frame, thus reducing the needed air time."))
o.ucioption = "network_coding"
o.default = o.enabled


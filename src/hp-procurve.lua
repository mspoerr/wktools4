-- HP Procurve Parser for wktools
-- For reference and documentation
-- !! This script contains only minimum error handling !!
--
-- wktools provides the ParserDB object for all function calls.
-- Print string: "ParserDB:printString(string)"
--
-- !!! This script is only working when using this show command sequence exactly as it is:
-- show version
-- show system
-- show interfaces config
-- show interfaces brief
-- show interfaces all
-- show trunks
-- show vlan
-- show ip
-- show arp
-- show mac-address
-- show spanning-tree
-- show cdp neighbor detail
-- show lldp info rem all
-- show ip route
-- show ip ospf neighbor
-- show module
-- show interfaces transceiver
-- show flash
-- show power-over-ethernet
-- show power-over-ethernet brief
--
-- Trim Whitespace function
function trim(s)
  return s:gsub("^%s+", ""):gsub("%s+$", "")
end

-- Load String to parse; The file was already loaded by wktools
data = ParserDB:getStringToParse()


-- Get Positions for all interesting show commands
pos01, pos010 = data:find("show version.-show system")
pos02, pos020 = data:find("show system.-show interfaces config")
pos03, pos030 = data:find("show interfaces config.-show interfaces brief")
pos04, pos040 = data:find("show interfaces brief.-show interfaces all")
pos05, pos050 = data:find("show interfaces all.-show trunks")
pos06, pos060 = data:find("show trunks.-show vlan")
pos07, pos070 = data:find("show vlan.-show ip")
pos08, pos080 = data:find("show ip.-show arp")
pos09, pos090 = data:find("show arp.-show mac%-address")
pos10, pos100 = data:find("show mac%-address.-show spanning%-tree")
pos11, pos110 = data:find("show spanning%-tree.-show cdp neighbor detail")
pos12, pos120 = data:find("show cdp neighbor detail.-show lldp info rem all")
pos13, pos130 = data:find("show lldp info rem all.-show ip route.")
pos14, pos140 = data:find("show ip route.-show ip ospf neighbor")
pos15, pos150 = data:find("show ip ospf neighbor.-show module")
pos16, pos160 = data:find("show module.-show interfaces transceiver")
pos17, pos170 = data:find("show interfaces transceiver.-show power%-over%-ethernet")
pos18, pos180 = data:find("show power%-over%-ethernet.-show power%-over%-ethernet brief")
pos19, pos190 = data:find("show power%-over%-ethernet brief.*")

if pos01 ~= nil then
    pos01, pos010 = data:find("Image stamp:.*Boot Image:")
end

-- Position pos04 after table header of "show interfaces brief" table
if pos04 ~= nil then
    postemp, pos04 = data:find("%- %+ %-.-\n", pos04)
end

-- Position pos06 after table header of "show trunks" table
if pos06 ~= nil then
    postemp, pos06 = data:find("%- %+ %-.-\n", pos06)
end

-- Position pos07 after table header of "show vlan" table
if pos07 ~= nil then
    postemp, pos07 = data:find("%- %+ %-.-\n", pos07)
end
-- Position pos08 after table header of "show ip" table
if pos08 ~= nil then
    postemp, pos08 = data:find("%- %+ %-.-\n", pos08)
end

-- Position pos14 after table header of "show ip route" table
if pos14 ~= nil then
    postemp, pos14 = data:find("%-%-%-.-\n", pos14)
end

-- Position pos09 after table header of "show arp" table
if pos09 ~= nil then
    postemp, pos09 = data:find("%-%-%-.-\n", pos09)
end

-- Position pos10 after table header of "show mac-address" table
if pos10 ~= nil then
    postemp, pos10 = data:find("%-%-%-.-\n", pos10)
end

-- Position pos17 after table header of "show interfaces transceiver" table
if pos17 ~= nil then
    postemp, pos17 = data:find("%-%-%-.-\n", pos17)
end


-- Extract the needed show data to work with it later on
ver = data:sub(pos01, pos010)			-- OK
sys = data:sub(pos02, pos020)			-- OK
icon = data:sub(pos03, pos030)			-- Parser not included
ibrief = data:sub(pos04, pos040)		-- OK
iall = data:sub(pos05, pos050)			-- OK
trunk = data:sub(pos06, pos060)			-- OK
vlan = data:sub(pos07, pos070)			-- OK
ip = data:sub(pos08, pos080)			-- OK
arp = data:sub(pos09, pos090)			-- OK
mac = data:sub(pos10, pos100)			-- OK
stp = data:sub(pos11, pos110)			-- OK
cdp = data:sub(pos12, pos120)			-- OK - Some devices are not included due to lack of needed data
lldp = data:sub(pos13, pos130)			-- OK - Some devices are not included due to lack of needed data
iproute = data:sub(pos14, pos140)		-- OK
ospf = data:sub(pos15, pos150)			-- Parser not included
module = data:sub(pos16, pos160)		-- OK
itrans = data:sub(pos17, pos170)		-- OK
poe = data:sub(pos18, pos180)			-- OK
poebrief = data:sub(pos19, pos190)		-- OK

--------------------------------------------------------------------------------------------------------------

-- "show version" Parser
-- Image File
ParserDB:printString("show version & show system\r\n")
apos1, apos2 = ver:find("/%g*")
bootimage = ver:sub(apos1, apos2)

-- Version
apos1, apos2 = ver:find("%u%.%g*")
version = ver:sub(apos1, apos2)

-- "show system" Parser
-- Hostname
apos1, apos2 = sys:find("System Name        : ")
apos1, apos2 = sys:find("%g*", apos2+1)
hostname = sys:sub(apos1, apos2)

-- SW Version
apos1, apos2 = sys:find("Software revision  : ")
apos1, apos2 = sys:find("%g*", apos2+1)
version = sys:sub(apos1, apos2)

-- S/N
apos1, apos2 = sys:find("Serial Number      : ")
apos1, apos2 = sys:find("%g*", apos2+1)
sn = sys:sub(apos1, apos2)

-- Base MAC
apos1, apos2 = sys:find("Base MAC Addr      : ")
apos1, apos2 = sys:find("%g*", apos2+1)
basemac = sys:sub(apos1, apos2)
-- MAC Address Normalization in case format is not like aaaa.bbbb.cccc
basemac = ParserDB:macAddChange(basemac)

-- SNMP Location
apos1, apos2 = sys:find("System Location    :")
apos1, apos2 = sys:find("%g*", apos2+2)
location = sys:sub(apos1, apos2)

-- Insert Device
-- Set the following values for type and deviceType
-- 1 Router
-- 2 Switch
-- 3 Firewall
-- 4 AccessPoint
-- 8 L3 Switch
-- 6 VoiceGateway
-- 11 Phone
-- 12 End-system
-- 13 ATA
-- 14 Camera
dev_id = ParserDB:insertDevice(hostname, "2", "2", "")
ParserDB:insertSnmp(location, dev_id)

--------------------------------------------------------------------------------------------------------------

function split(inputstr, sep)
		
		if sep == nil then
                sep = "%s"
        end
        t={} ; i=1
        for str in string.gmatch(inputstr, "([^"..sep.."]+)") do
                t[i] = str
                i = i + 1
        end
        return t
end

--------------------------------------------------------------------------------------------------------------

-- "show interfaces brief" && "show interfaces all " Parser
-- "show interfaces config" is not needed at the moment
ParserDB:printString("show interfaces brief & show interfaces all\r\n")

function readIntfAll(intfName)
	ipos1, ipos2 = iall:find("for port " .. intfName .. ".-Utilization Tx.-\n")
	currentInt = iall:sub(ipos1, ipos2)
	
	-- Interface Name/Description
	ipos1, ipos2 = currentInt:find("Name  :")
	ipos1, ipos2 = currentInt:find("[%g ]-", ipos2+2)
	intfDescription = currentInt:sub(ipos1, ipos2)

	
	-- MAC Address if available
	macAddress = ""
	ipos1, ipos2 = currentInt:find("MAC Address      : ")
	if ipos1 ~= nil then
	    ipos1, ipos2 = currentInt:find("%g*", ipos2+1)
	    macAddress = currentInt:sub(ipos1, ipos2)
	end
	
	-- Errors
	ipos1, ipos2 = currentInt:find("FCS Rx          : ")
	ipos1, ipos2 = currentInt:find("[%d ]*", ipos2+1)
	local eFcs = currentInt:sub(ipos1, ipos2)
	
	ipos1, ipos2 = currentInt:find("Alignment Rx    : ")
	ipos1, ipos2 = currentInt:find("[%d ]*", ipos2+1)
	local eAlign = currentInt:sub(ipos1, ipos2)
	
	ipos1, ipos2 = currentInt:find("Runts Rx        : ")
	ipos1, ipos2 = currentInt:find("[%d ]*", ipos2+1)
	local eRunts = currentInt:sub(ipos1, ipos2)
	
	ipos1, ipos2 = currentInt:find("Drops Tx        : ")
	ipos1, ipos2 = currentInt:find("[%d ]*", ipos2+1)
	local eDrops = currentInt:sub(ipos1, ipos2)
	
	ipos1, ipos2 = currentInt:find("Collisions Tx   : ")
	ipos1, ipos2 = currentInt:find("[%d ]*", ipos2+1)
	local eColl = currentInt:sub(ipos1, ipos2)
	
	ipos1, ipos2 = currentInt:find("Late Colln Tx   : ")
	ipos1, ipos2 = currentInt:find("[%d ]*", ipos2+1)
	local eLateColl = currentInt:sub(ipos1, ipos2)
	
	ipos1, ipos2 = currentInt:find("Giants Rx       : ")
	ipos1, ipos2 = currentInt:find("[%d ]*", ipos2+1)
	local eGiants = currentInt:sub(ipos1, ipos2)
	
	ipos1, ipos2 = currentInt:find("Excessive Colln : ")
	ipos1, ipos2 = currentInt:find("[%d ]*", ipos2+1)
	local eExcessColl = currentInt:sub(ipos1, ipos2)
	
	ipos1, ipos2 = currentInt:find("Total Rx Errors : ")
	ipos1, ipos2 = currentInt:find("[%d ]*", ipos2+1)
	local eRxTotal = currentInt:sub(ipos1, ipos2)
	
	ipos1, ipos2 = currentInt:find("Deferred Tx     : ")
	ipos1, ipos2 = currentInt:find("[%d ]*", ipos2+1)
	local eDefer = currentInt:sub(ipos1, ipos2)
	
	ipos1, ipos2 = currentInt:find("Discard Rx      : ")
	ipos1, ipos2 = currentInt:find("[%d ]*", ipos2+1)
	local eDiscard = currentInt:sub(ipos1, ipos2)
	
	ipos1, ipos2 = currentInt:find("Out Queue Len   : ")
	ipos1, ipos2 = currentInt:find("[%d ]*", ipos2+1)
	local eOutQueue = currentInt:sub(ipos1, ipos2)
	
	ipos1, ipos2 = currentInt:find("Unknown Protos  : ")
	ipos1, ipos2 = currentInt:find("[%d ]*", ipos2+1)
	local eUnknown = currentInt:sub(ipos1, ipos2)
	
	ipos1, ipos2 = currentInt:find("Utilization Rx  : ")
	ipos1, ipos2 = currentInt:find("[%d ]*", ipos2+1)
	local eRxUtil = currentInt:sub(ipos1, ipos2)
	
	ipos1, ipos2 = currentInt:find("Utilization Tx  : ")
	ipos1, ipos2 = currentInt:find("[%d ]*", ipos2+1)
	local eTxUtil = currentInt:sub(ipos1, ipos2)
		
	return currentInt, intfDescription, macAddress, eFcs, eAlign, eRunts, eDrops, eColl, eLateColl, eGiants, eExcessColl, eRxTotal, eDefer, eDiscard, eOutQueue, eUnknown, eRxUtil, eTxUtil
end

for line in ibrief:gmatch(".-\n") do 
    -- 1) Split line and save the following values: Port, Type, Enabled, Status, Mode, 
	-- 2) Find Port in "show interfaces all" and read the following data: Name, MAC Address, Err Counter
	-- 3) Write Interface into DB
	
	if #line > 10 then
		local tble = split(line, "%s")
		local intfName = tble[1]
		local intfType, intfMode, intfEna, intfStatus, speed, duplex
		if tble[2] == "|" then
			-- Type and Mode empty
			intfEna = tble[4]
			intfStatus = tble[5]
			intfMode = ""
			speed = ""
			duplex = ""
			intfType = ""
		else
			intfEna = tble[5]
			intfStatus = tble[6]
			intfMode = tble[7]
			p1, p2 = intfMode:find("D")
			speed = intfMode:sub(0, p2-2)
			duplex = intfMode:sub(p2-1, #intfMode)
			intfType = tble[2]
		end
		
		iSets = intfSets()
		
		p1 = intfName:find("%-")
		if p1 ~= nil then
			intfName = intfName:sub(1, p1-1)
		end
		
		-- Read Details for this interface:
		currentInt, intfDescription, macAddress, eFcs, eAlign, eRunts, eDrops, eColl, eLateColl, eGiants, eExcessColl, eRxTotal, eDefer, eDiscard, eOutQueue, eUnknown, eRxUtil, eTxUtil = readIntfAll(intfName)
		
		-- Interface Normalizers:
		-- intfNameChange to shorten the Interface Name if needed/possible
		-- intfTypeCheck to unify the Interface operational speed/type
		-- intfPhlCheck to check whether the interface is a phyical or logical interface
		iSets.intfName = ParserDB:intfNameChange(intfName)
		iSets.intfType = ParserDB:intfTypeCheck(intfName, intfMode)
		iSets.phl = ParserDB:intfPhlCheck(intfName, intfMode)
		iSets.nameif = ""
		iSets.macAddress = ParserDB:macAddChange(macAddress)
		iSets.ipAddress = ""
		iSets.subnetMask = ""
		iSets.duplex = duplex
		iSets.speed = speed
		iSets.status = intfStatus
		iSets.description = intfDescription
		iSets.l2l3 = "L2"
		iSets.dev_id = dev_id
		
		-- Insert Interface to DB
		intf_id = ParserDB:insertInterface(iSets)
		
		-- Insert Interface Statistics to DB
		errLvl = tonumber(eFcs) + tonumber(eUnknown) + tonumber(eRunts) + tonumber(eGiants) + tonumber(eColl) + tonumber(eLateColl) + tonumber(eDefer) + tonumber(eAlign) + tonumber(eDrops) + tonumber(eExcessColl) + tonumber(eDiscard) + tonumber(eOutQueue)
		iLoad = tonumber(eRxUtil)*1,25 + tonumber(eTxUtil)*1,25
		iStats = intfStats()
		iStats.errLvl = trim(tostring(errLvl))
		iStats.lastClear = ""
		iStats.loadLvl = trim(tostring(iLoad))
		iStats.iCrc = trim(eFcs)
		iStats.iFrame = trim(eUnknown)
		iStats.iOverrun = ""
		iStats.iIgnored = ""
		iStats.iWatchdog = ""
		iStats.iPause = ""
		iStats.iDribbleCondition = ""
		iStats.ibuffer = ""
		iStats.l2decodeDrops = trim(eDrops)
		iStats.runts = trim(eRunts)
		iStats.giants = trim(eGiants)
		iStats.throttles = ""
		iStats.ierrors = trim(eRxTotal)
		iStats.underrun = ""
		iStats.oerrors = ""
		iStats.oCollisions = trim(eColl)
		iStats.oBabbles = ""
		iStats.oLateColl = trim(eLateColl)
		iStats.oDeferred = trim(eDefer)
		iStats.oLostCarrier = ""
		iStats.oNoCarrier = ""
		iStats.oPauseOutput = ""
		iStats.oBufferSwapped = ""
		iStats.resets = ""
		iStats.obuffer = ""
		iStats.lastInput = ""
		iStats.intf_id = intf_id			
		
		ParserDB:insertIntfStats(iStats)
		
	end
end

--------------------------------------------------------------------------------------------------------------

-- "show trunks" Parser
ParserDB:printString("show trunks\r\n")

trks = {}
trunkIntfId = ""
for line in trunk:gmatch(".-\n") do 
    -- 1) Split line and save the following values: Port, Name, Group
	-- 2) Find intf_id's for Ports 
	-- 3) Write Trunk Interface to DB
	-- 4) Mark Interfaces in DB

	if #line > 10 then
		local tble = split(line, "%s")
		local intfName = tble[1]
		local intfDescription, group
		if tble[4] == "|" then
			-- Type and Mode empty
			group = tble[5]
			intfDescription = ""
		else
			group = tble[6]
			intfDescription = tble[3]
		end
		
		if trks[group] == nil then
			trks[group] = true
			
			iSets = intfSets()
			iSets.intfName = ParserDB:intfNameChange(group)
			iSets.intfType = ParserDB:intfTypeCheck(group, "")
			iSets.phl = ParserDB:intfPhlCheck(group, "")
			iSets.nameif = ""
			iSets.macAddress = ""
			iSets.ipAddress = ""
			iSets.subnetMask = ""
			iSets.duplex = ""
			iSets.speed = ""
			iSets.status = "Up"
			iSets.description = intfDescription
			iSets.l2l3 = "L2"
			iSets.dev_id = dev_id
		
			trunkIntfId = ParserDB:insertInterface(iSets)
		end
		ParserDB:updateChannelMember(trunkIntfId, intfName, dev_id)
	end
end

--------------------------------------------------------------------------------------------------------------

-- "show vlan" && "show ip" Parser
ParserDB:printString("show vlan & show ip\r\n")

function readVlanDetails(vlanName)
	local vpos1, vpos2 = vlan:find("%d-%s-" .. vlanName .. ".-|")
	if vpos1 ~= nil then
		local vline = vlan:sub(vpos1, vpos2)
		local vtble = split(vline, "%s")
		return vtble[2], vtble[1]
	else
		return "", ""
	end
end



for line in ip:gmatch(".-\n") do 
	-- Lookup Vlan-Nr, Vlan-Name in "show vlan" for all Lines in "show ip table"
	-- The connection between the two tables is the VLAN Name
	if #line > 45 then
		str3 = line:sub(3, 3)
		-- Ignore Lines with table headers
		if str3 ~= "-" then
			local tble = split(line, "%s")
			
			-- Secondary IP addresses are only written into the Subnet Table (DB)
			-- Ignore Table headers
			if ((tble[5] ~= "IP") and (tble[4] ~= nil)) then
				ParserDB:printString(line)
				ParserDB:printString(tble[3])
				if tble[1] == "|" then	-- Handle Secondary IP addresses
					l3intf_id = l3intf_id -- Interface ID is taken from last time
					ipa = tble[3]
					-- Change Subnetmask to /Bit format
					mask = ParserDB:maskToBits(tble[4])
					-- Insert Subnet into Subnet Table (DB)
					ParserDB:insertSubnet(ipa, mask, l3intf_id)
				else	-- Handle Primary IP addresses
					vName = tble[1]
					-- Delete ... at the end of the VLAN Name in "show ip" output
					vName = vName:gsub("%.%.%.", "")
					-- Add the escape character % to the special characters (-.%+*()[]^$); Otherwise they would be used as Patterns 
					vName = vName:gsub("[%-%.%%%+%*%(%)%[%]%^%$]", "%%%0")
					
					if tble[3] == "Disabled" then
						ipa = ""
						mask = ""				
					else
						ipa = tble[4]
						-- Change Subnetmask to /Bit format
						mask = ParserDB:maskToBits(tble[5])
					end
					vlanName, vlanNr = readVlanDetails(vName)
					-- Fill DB Interface Table with L3 Interface Details if IP is available
					if ipa ~= "" then
						iSets = intfSets()
						if vlanNr ~= "" then
							iName = "Vlan" .. vlanNr
							iNameIf = vlanName
							iDescr = vlanName
						else
							iName = vName
							iNameIf = ""
							iDescr = ""
						end
						iSets.intfName = ParserDB:intfNameChange(iName)
						iSets.intfType = ParserDB:intfTypeCheck(iName, "")
						iSets.phl = ParserDB:intfPhlCheck(iName, "")
						iSets.nameif = iNameIf
						iSets.macAddress = ""
						iSets.ipAddress = ipa
						iSets.subnetMask = mask
						iSets.duplex = ""
						iSets.speed = ""
						iSets.status = "Up"
						iSets.description = iDescr
						iSets.l2l3 = "L3"
						iSets.dev_id = dev_id
					
						l3intf_id = ParserDB:insertInterface(iSets)
						-- Insert Subnet into Subnet Table (DB)
						ParserDB:insertSubnet(ipa, mask, l3intf_id)
					end
				
					-- Fill DB VLAN Table; STP Instance not available, so leave the option blank
					if vlanName ~= "" then
						ParserDB:insertVlan(vlanNr, dev_id, "")
					end
				end
			end
		end
	end
end
ParserDB:printString("show vlan & show ip - ENDE\r\n")

--------------------------------------------------------------------------------------------------------------

ParserDB:printString("show ip route\r\n")

-- "show ip route" Parser
for line in iproute:gmatch(".-\n") do 
	-- Split Routing Table
	-- Normalize Subnetmask
	-- Ignore 127.0.0.x networks
	-- Insert into DB Routing Table
	if #line > 10 then
		local rtble = split(line, "%s/")
		local subnet = rtble[1]
		-- Mask is needed in /Prefix format; If in SubnetMask format, then use ParserDB:maskToBits function to change format
		local mask = rtble[2]
		local gateway = rtble[3]
		local vlan = rtble[4]
		local rProt
		local pos = vlan:find("%d")
		if pos == nil then
			rProt = rtble[4]
			vlan = ""
		else
			rProt = rtble[5]
		end
		-- Discard 127.0.0.x IPs
		pos = subnet:find("127%.0%.0%.%d-")
		if (pos == nil) and (rProt ~= "connected") then
			-- Insert Route to Routing Table; Keep type field empty, as it is optional at the moment
			-- Ignore Connected Routes				
			-- If interface name is not available, then leave it empty
			local interface = ""
			if vlan ~= "" then
				interface = "Vlan" .. vlan
			end
			ParserDB:insertRoute(rProt, subnet, mask, gateway, interface, "", dev_id)
		end
	end
end

--------------------------------------------------------------------------------------------------------------

-- "show arp" Parser
ParserDB:printString("show arp\r\n")

for line in arp:gmatch(".-\n") do 
	-- Lookup Vlan-Nr, Vlan-Name in "show vlan" for all Lines in "show ip table"
	-- The connection between the two tables is the VLAN Name
	if #line > 10 then
		local atble = split(line, "%s")
		local ipAddress = atble[1]
		local macAddress = ParserDB:macAddChange(atble[2])
		local port = atble[4]
		-- Problem: HP uses the physical switchport (L2) in the ARP table; But it should be the logical port (L3) instead;
		-- -> Port ID must be read from Subnet Table
		local i_id = ParserDB:nextHopIntfCheck(ipAddress, dev_id)
		
		-- Insert ARP entry to neighbor table (DB)
		ParserDB:insertNeighbor(i_id, macAddress, ipAddress, "")
	end
end

--------------------------------------------------------------------------------------------------------------

-- "show mac-address" Parser
ParserDB:printString("show mac-address\r\n")

for line in mac:gmatch(".-\n") do 
	-- Split entries, normalize MAC Address and insert into DB neighbor Table
	if #line > 10 then
		local mtble = split(line, "%s")
		local macAddress = ParserDB:macAddChange(mtble[1])
		-- Don't forget to always use intfNameChange to normalize the Interface Names
		local port = ParserDB:intfNameChange(mtble[2])
		local vlan = mtble[3]
		-- First the Interface ID is needed
		iID = ParserDB:getIntfID(dev_id, port, true)
		-- Then the MAC address can be written into DB; Leave IP Address and Self empty when only adding CAM entries
		ParserDB:insertNeighbor(iID, macAddress, "", "")
	end
end

--------------------------------------------------------------------------------------------------------------

-- "show spanning-tree" Parser
-- 1) Global STP infos
-- STP Protocol
ParserDB:printString("show spanning-tree\r\n")


function pvstParser()
	apos1, apos2 = stp:find("Mode%s*: ")
	apos1, apos2 = stp:find("%g*", apos2+2)
	stpProtocol = stp:sub(apos1+1, apos2-1)

	-- Switch MAC Address
	apos1, apos2 = stp:find("Switch MAC Address%s*: ")
	apos1, apos2 = stp:find("%g*", apos2+1)
	switchSTPMac = stp:sub(apos1, apos2)
	switchSTPMac = ParserDB:macAddChange(switchSTPMac)

	-- Insert Switch MAC and STP Protocol to device table
	ParserDB:updateSTP(switchSTPMac, stpProtocol, dev_id)

	-- 2) Interface related STP infos
	tpos, apos1 = stp:find("%- %-%-.-\n", apos1)
	stpIntf = stp:sub(apos1, #stp)

	for line in stpIntf:gmatch(".-\n") do 
		-- Split entries, normalize MAC Address and insert into DB stp_status Table
		if #line > 40 then
			local stble = split(line, "%s")
			-- Only insert STP Status for Non-Edge Ports
			if (stble[5] ~= nil) then
				-- OK
				local port = stble[5]
				port = ParserDB:intfNameChange(port)
				local stpStatus = ""
				local designatedBridge = stble[2]
				designatedBridge = ParserDB:macAddChange(designatedBridge)
				
				-- It is very important to set STP Transition Count to "0" instead to ""
				ParserDB:insertSTPStatus(dev_id, port, stpStatus, designatedBridge, designatedBridge, "0", stble[1], true, "0")
			end
		end
	end
end

function mstParser()
	apos1, apos2 = stp:find("%(.-%)")
	stpProtocol = stp:sub(apos1+1, apos2-1)
	apos1, apos2 = stp:find("Force Version :")

	apos1, apos2 = stp:find("%g*", apos2+2)
	stpProtocol = stpProtocol .. "/" .. stp:sub(apos1, apos2)
	ParserDB:printString("show spanning-tree 2\r\n")

	-- Switch MAC Address
	apos1, apos2 = stp:find("Switch MAC Address : ")
	apos1, apos2 = stp:find("%g*", apos2+1)
	switchSTPMac = stp:sub(apos1, apos2)
	switchSTPMac = ParserDB:macAddChange(switchSTPMac)
	ParserDB:printString("show spanning-tree 1\r\n")

	-- Insert Switch MAC and STP Protocol to device table
	ParserDB:updateSTP(switchSTPMac, stpProtocol, dev_id)


	-- Switch Priority
	apos1, apos2 = stp:find("Switch Priority%s*: ")
	apos1, apos2 = stp:find("%g*", apos2+1)
	switchSTPPrio = stp:sub(apos1, apos2)


	-- Root MAC Address (for MST only CST Root)
	apos1, apos2 = stp:find("Root MAC Address : ")
	apos1, apos2 = stp:find("%g*", apos2+1)
	rootMac = stp:sub(apos1, apos2)
	-- Normalize MAC address
	rootMac = ParserDB:macAddChange(rootMac)

	-- Root Priority (for MST only CST Root)
	apos1, apos2 = stp:find("Root Priority%s*: ")
	apos1, apos2 = stp:find("%g*", apos2+1)
	rootPrio = stp:sub(apos1, apos2)

	-- Root Port (for MST only CST Root)
	apos1, apos2 = stp:find("Root Port%s*: ")
	apos1, apos2 = stp:find("[%g ]*", apos2+1)
	rootPort = stp:sub(apos1, apos2)
	if rootPort == "This switch is root" then
		rootPort = ""
	else
		-- Normalize Interface Name
		rootPort = ParserDB:intfNameChange(rootPort)
	end

	-- Insert STP Instance
	stpInstance = ParserDB:insertSTPInstance(switchSTPPrio, rootPort)
	-- Insert Dummy VLAN 9999; This VLAN marks the CST
	ParserDB:insertVlan("9999", dev_id, stpInstance)

	ParserDB:printString("show spanning-tree Intf Infos\r\n")

	-- 2) Interface related STP infos
	tpos, apos1 = stp:find("%- %+ %-.-\n", apos1)
	stpIntf = stp:sub(apos1, #stp)

	for line in stpIntf:gmatch(".-\n") do 
		-- Split entries, normalize MAC Address and insert into DB stp_status Table
		if #line > 10 then
			local stble = split(line, "%s|")
			-- Only insert STP Status for Non-Edge Ports
			if ((stble[6] ~= nil) and (stble[9] == "No")) then
				-- OK
				local port = stble[1]
				port = ParserDB:intfNameChange(port)
				local stpStatus = stble[5]
				local designatedBridge = stble[6]
				designatedBridge = ParserDB:macAddChange(designatedBridge)
				
				-- It is very important to set STP Transition Count to "0" instead to ""
				ParserDB:insertSTPStatus(dev_id, port, stpStatus, designatedBridge, designatedBridge, "0", "9999", true)
			end
		end
	end
end

apos1, apos2 = stp:find("%(.-%)")
stpProtocol = stp:sub(apos1+1, apos2-1)
apos1, apos2 = stp:find("Force Version :")

-- Check which STP Output is used
if (apos1 ~= nil) then
	mstParser()
else
	pvstParser()
end




--------------------------------------------------------------------------------------------------------------

-- "show module" Parser
-- 1) Chassis Info
ParserDB:printString("show module\r\n")

apos1, apos2 = module:find("Status and Counters %- Module Information.-Slot Module Description")
chassis = module:sub(apos1, apos2)
apos1, apos2 = chassis:find("Chassis: ")
apos1, apos2 = chassis:find("%g*%s%g*", apos2+1)
if apos1 ~= nil then
	chassisType = chassis:sub(apos1, apos2)
	apos1, apos2 = chassis:find("Serial Number:%s*")
	apos1, apos2 = chassis:find("%g*", apos2+1)
	chassisSN = chassis:sub(apos1, apos2)
	position = "box"
	-- Insert Chassis information and save id; Important: Always use "box" for Chassis position because of wktools internal special handling
	chassis_id = ParserDB:insertHardware(position, chassisType, "", chassisSN, "", "", bootimage, version, dev_id, "")
end
apos1, apos2 = chassis:find("Management Module: ")
if apos1 ~= nil then
	moduleDescr = "Management Module"
	apos1, apos2 = chassis:find("%g*", apos2+1)
	moduleType = chassis:sub(apos1, apos2)
	apos1, apos2 = chassis:find("Serial Number:%s*", apos2)
	apos1, apos2 = chassis:find("%g*", apos2+1)
	moduleSN = chassis:sub(apos1, apos2)	
	position = "MM"
	-- Attach modules to the chassis by passing the DB id of the cahssis...
	ParserDB:insertHardware(position, moduleType, moduleDescr, moduleSN, "", "", "", "", dev_id, chassis_id)
end

-- 2) Module Info
apos1, apos2 = module:find("%-%-%-%-.-show interfaces transceiver")
ptemp, apos1 = module:find(".-\n", apos1)
modules = module:sub(apos1, apos2)
for line in modules:gmatch(".-\n") do 
	-- Split entries, 
	if #line > 77 then
		position = trim(line:sub(1, 7))
		moduleType = trim(line:sub(8, 16))
		if moduleType:find("ProCurve") ~= nil then
			moduleType = trim(line:sub(8, 23))
			moduleDescr = trim(line:sub(24, 46))
		else
			moduleDescr = trim(line:sub(17, 46))
		end
		moduleSN = trim(line:sub(47, 61))
		hwVersion = trim(line:sub(77, #line))
		-- Attach modules to the chassis by passing the DB id of the cahssis...
		ParserDB:insertHardware(position, moduleType, moduleDescr, moduleSN, hwVersion, "", "", "", dev_id, chassis_id)
	end
end
	
--------------------------------------------------------------------------------------------------------------

-- "show interfaces transceiver" Parser
ParserDB:printString("show interfaces transceiver\r\n")

for line in itrans:gmatch(".-\n") do 
	-- Split entries and insert into HW Table
	if #line > 10 then
		local ittble = split(line, "%s|")
		if ittble[5] ~= nil then
			position = ittble[1]
			moduleType = ittble[3]
			moduleDescr = ittble[2] .. "; Part#: " .. ittble[5]
			moduleSN = ittble[4]
			hwVersion = ""
			ParserDB:insertHardware(position, moduleType, moduleDescr, moduleSN, hwVersion, "", "", "", dev_id, chassis_id)
		end
	end
end
	
--------------------------------------------------------------------------------------------------------------

-- "show cdp neighbor detail" Parser

ParserDB:printString("show cdp neighbor detail\r\n")

apos1 = 0
apos2 = 0
while true do
	apos1, apos2 = cdp:find("Port .-Version      :.-\n", apos2)
	if apos1 == nil then
		break
	end
	cdpNeighbor = cdp:sub(apos1, apos2)
	local cpos1, cpos2 = cdpNeighbor:find("Device ID : ")
	cpos1, cpos2 = cdpNeighbor:find("[%g ]*", cpos2+1)
	deviceID = cdpNeighbor:sub(cpos1, cpos2)
	deviceID = trim(deviceID)

	-- Skip HP Procurve Devices since CDP information is not sufficient; These devices will be discovered with "show lldp..."
	tpos1 = deviceID:find("%g%g %g%g %g%g %g%g %g%g %g%g")
	if tpos1 == nil then
		cpos1, cpos2 = cdpNeighbor:find("Port : ")
		cpos1, cpos2 = cdpNeighbor:find("%g*", cpos2+1)	
		localPort = cdpNeighbor:sub(cpos1, cpos2)
		localPort = ParserDB:intfNameChange(localPort)

		cpos1, cpos2 = cdpNeighbor:find("Address      : ")
		cpos1, cpos2 = cdpNeighbor:find("[%g]*", cpos2+1)	
		ipAddress = cdpNeighbor:sub(cpos1, cpos2)

		cpos1, cpos2 = cdpNeighbor:find("Platform     : ")
		cpos1, cpos2 = cdpNeighbor:find("[%g ]*", cpos2+1)	
		platform = cdpNeighbor:sub(cpos1, cpos2)

		cpos1, cpos2 = cdpNeighbor:find("Capability   : ")
		cpos1, cpos2 = cdpNeighbor:find("[%g ]*", cpos2+1)	
		capability = cdpNeighbor:sub(cpos1, cpos2)
		-- Set the following values for type and deviceType
		-- 1 Router
		-- 2 Switch
		-- 3 Firewall
		-- 4 AccessPoint
		-- 8 L3 Switch
		-- 6 VoiceGateway
		-- 11 Phone
		-- 12 End-system
		-- 13 ATA
		-- 14 Camera
		-- 99 Unknown
		if capability == "Switch" then
			nType = "2"
		elseif capability == "Router Switch" then
			nType = "8"
		else
			nType = "99"
		end

		cpos1, cpos2 = cdpNeighbor:find("Device Port  : ")
		cpos1, cpos2 = cdpNeighbor:find("[%g%-%_ ]*", cpos2+1)	
		remotePort = cdpNeighbor:sub(cpos1, cpos2)
		remotePort = ParserDB:intfNameChange(remotePort)

		cpos1, cpos2 = cdpNeighbor:find("Version      : ")
		cpos1, cpos2 = cdpNeighbor:find("[%g ]*", cpos2+1)	
		version = cdpNeighbor:sub(cpos1, cpos2)

		-- Add neighbor to CDP table
		ParserDB:insertCDP(hostname, deviceID, deviceID, localPort, ipAddress,remotePort, nType, platform, version, dev_id)
		ParserDB:printString("CDP Insert")
	end
end

--------------------------------------------------------------------------------------------------------------

-- "show lldp..." Parser
ParserDB:printString("show lldp...\r\n")

apos1 = 0
apos2 = 0
while true do
	apos3 = apos2
	apos1, apos2 = lldp:find("Local Port.-%-%-%-%-", apos2)
	if apos1 == nil then
		apos1, apos2 = lldp:find("Local Port", apos3)
		if apos1 == nil then
			break
		else
			apos2 = #lldp
		end
	end
	lldpNeighbor = lldp:sub(apos1, apos2)
	
	local cpos1, cpos2 = lldpNeighbor:find("SysName      : ")
	if cpos1 ~= nil then
		cpos1, cpos2 = lldpNeighbor:find("[%g ]*", cpos2+1)
		deviceID = lldpNeighbor:sub(cpos1, cpos2)
	else
		deviceID = ""
	end
	
	deviceID = trim(deviceID)
	
	cpos1, cpos2 = lldpNeighbor:find("ChassisId    : ")
	cpos1, cpos2 = lldpNeighbor:find("[%w ]*", cpos2+1)
	chassisID = lldpNeighbor:sub(cpos1, cpos2)
		
	if deviceID == "" then
		deviceID = chassisID
		alternateDeviceID = deviceID
	else
		-- Concatenate deviceID and chassisID
		-- This is for switches with Default Hostname to keep them apart
		alternateDeviceID = deviceID .. "." .. chassisID:gsub(" ", "")
	end

	tpos1 = deviceID:find("%g%g %g%g %g%g %g%g %g%g %g%g")
	if tpos1 == nil then
		cpos1, cpos2 = lldpNeighbor:find("Local Port   : ")
		cpos1, cpos2 = lldpNeighbor:find("%g*", cpos2+1)	
		localPort = lldpNeighbor:sub(cpos1, cpos2)
		localPort = ParserDB:intfNameChange(localPort)

		cpos1, cpos2 = lldpNeighbor:find("Address : ")
		cpos1, cpos2 = lldpNeighbor:find("[%g]*", cpos2+1)	
		ipAddress = lldpNeighbor:sub(cpos1, cpos2)

		cpos1, cpos2 = lldpNeighbor:find("System Descr : ")
		cpos1, cpos2 = lldpNeighbor:find("[%g ]*", cpos2+1)	
		platform = lldpNeighbor:sub(cpos1, cpos2)

		cpos1, cpos2 = lldpNeighbor:find("System Capabilities Enabled    : ")
		if cpos1 ~= nil then
			cpos1, cpos2 = lldpNeighbor:find("[%g ]*", cpos2+1)	
			capability = lldpNeighbor:sub(cpos1, cpos2)
		else
			capability = ""
		end
		
		-- Set the following values for type and deviceType
		-- 1 Router
		-- 2 Switch
		-- 3 Firewall
		-- 4 AccessPoint
		-- 8 L3 Switch
		-- 6 VoiceGateway
		-- 11 Phone
		-- 12 End-system
		-- 13 ATA
		-- 14 Camera
		-- 99 Unknown
		if capability == "bridge" then
			nType = "2"
		elseif capability == "bridge, router" then
			nType = "8"
		else
			nType = "12"
		end

		cpos1, cpos2 = lldpNeighbor:find("PortId       : ")
		cpos1, cpos2 = lldpNeighbor:find("[%g ]*", cpos2+1)	
		remotePort = lldpNeighbor:sub(cpos1, cpos2)
		remotePort = ParserDB:intfNameChange(remotePort)

		cpos1, cpos2 = lldpNeighbor:find("PortDescr    : ")
		cpos1, cpos2 = lldpNeighbor:find("[%g ]*", cpos2+1)	
		portDescr = lldpNeighbor:sub(cpos1, cpos2)
		if (portDescr ~= "") then
			portDescr = ParserDB:intfNameChange(portDescr)
			remotePort = portDescr
		end

		version = ""

		-- Add neighbor to CDP table
		ParserDB:insertCDP(hostname, deviceID, alternateDeviceID, localPort, ipAddress,remotePort, nType, platform, version, dev_id)
		
		ParserDB:printString("DeviceID: " .. deviceID .. "  //  alternateDeviceID: " .. alternateDeviceID)
	end
end

--------------------------------------------------------------------------------------------------------------

-- "show power-over-ethernet" Parser
ParserDB:printString("show power-over-ethernet\r\n")

apos1, apos2 = poe:find("Total Available Power  :")
if apos1 ~= nil then
	-- Total PoE
	apos1, apos2 = poe:find("[%g ]*", apos2+1)
	poeTotal = trim(poe:sub(apos1, apos2))

	-- Used PoE
	apos1, apos2 = poe:find("Total used Power       :")
	apos1, apos2 = poe:find("[%g ]*", apos2+1)
	poeUsed = trim(poe:sub(apos1, apos2))

	-- Remaining PoE
	apos1, apos2 = poe:find("Total Remaining Power  :")
	apos1, apos2 = poe:find("[%g ]*", apos2+1)
	poeRemain = trim(poe:sub(apos1, apos2))
	
	ParserDB:insertPoeDev("ON", poeUsed, poeTotal, poeRemain, dev_id)
end

--------------------------------------------------------------------------------------------------------------

-- "show power-over-ethernet" Parser
ParserDB:printString("show power-over-ethernet brief\r\n")

for line in poebrief:gmatch(".-\n") do 
	-- Split entries and insert into Intf Table
	if #line > 10 then
		
		local potble = split(line, "%s")
		if potble[11] ~= nil then
			port = potble[1]
			poeCurrent = potble[8]
			poeMax = potble[6]
			poeStat = potble[3]

			ParserDB:insertPoeIntf(poeStat, poeCurrent, poeMax, "", port, dev_id)
		end
	end
end

--------------------------------------------------------------------------------------------------------------

-- Global --
ParserDB:markNeighbor(dev_id)


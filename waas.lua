-- Cisco WAAS Parser for wktools
-- For reference and documentation; Only tested with one version of Cisco WAAS devices
-- !! This script contains only minimum error handling !!
--
-- wktools provides the ParserDB object for all function calls.
-- Print string: "ParserDB:printString(string)"
--
-- !!! This script is only working when using this show command sequence exactly as it is:
-- show version
-- show invent
-- show interface inlineport 1/1 wan
-- show interface inlineport 1/1 lan
-- show interface gigabitEthernet 1/0
-- show cdp neighbors detail
-- show hardware
-- show snmp stats
-- show arp
-- 
-- Trim Whitespace function
function trim(s)
  return s:gsub("^%s+", ""):gsub("%s+$", "")
end

-- Split String function
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


-- Load String to parse; The file was already loaded by wktools
data = ParserDB:getStringToParse()


-- Get Positions for all interesting show commands
pos00, pos000 = data:find(".-#")
pos01, pos010 = data:find("show version.-show invent")
pos02, pos020 = data:find("show invent.-show interface")
pos03, pos030 = data:find("show interface.-show cdp neighbors detail")
pos06, pos060 = data:find("show cdp neighbors detail.-show hardware")
pos07, pos070 = data:find("show hardware.-show snmp stats")
pos08, pos080 = data:find("show snmp stats.-show arp")
pos09, pos090 = data:find("show arp.*")

-- Extract the needed show data to work with it later on
host = data:sub(pos00, pos000)			-- OK
ver = data:sub(pos01, pos010)			-- OK
invent = data:sub(pos02, pos020)		-- OK
iComplete = data:sub(pos03, pos030)		-- OK
cdp = data:sub(pos06, pos060)			-- OK
hardware = data:sub(pos07, pos070)		-- Not implemented
snmp = data:sub(pos08, pos080)			-- OK
arp = data:sub(pos09, pos090)			-- 

--------------------------------------------------------------------------------------------------------------

-- extract hostname
ParserDB:printString("Hostname\r\n")
apos1, apos2 = host:find("waas")
apos1, apos2 = host:find("%g.-#", apos2+1)
hostname = host:sub(apos1, apos2-1)

-- "show version" Parser
-- Version
ParserDB:printString("\r\nshow version\r\n")
apos1, apos2 = ver:find("Version: ")
apos1, apos2 = ver:find("%g*", apos2+1)
version = ver:sub(apos1, apos2)

-- Device ID
apos1, apos2 = ver:find("Device Id: ")
apos1, apos2 = ver:find("%g*", apos2+1)
devID = ver:sub(apos1, apos2)
-- MAC Address Normalization in case format is not like aaaa.bbbb.cccc
devID = ParserDB:macAddChange(devID)

-- "show inventory" Parser
ParserDB:printString("\r\nshow inventory\r\n")
apos1, apos2 = invent:find("Name: ")
apos1, apos2 = invent:find("%g*", apos2+1)
hwloc = invent:sub(apos1, apos2)

apos1, apos2 = invent:find("DESCR:.-PID:")
hwdescr = invent:sub(apos1+7, apos2-4)
hwdescr = trim(hwdescr)

apos1, apos2 = invent:find("PID: ")
apos1, apos2 = invent:find("%g*", apos2+1)
hwpid = invent:sub(apos1, apos2)

apos1, apos2 = invent:find("SN: ")
apos1, apos2 = invent:find("%g*", apos2+1)
hwsn = invent:sub(apos1, apos2)

-- "show snmp stats" Parser
ParserDB:printString("\r\nshow snmp stats\r\n")
apos1, apos2 = snmp:find("Location: ")
apos1, apos2 = snmp:find("%g*", apos2+1)
snmploc = snmp:sub(apos1, apos2)

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
dev_id = ParserDB:insertDevice(hostname, "12", "12", "")
ParserDB:insertSnmp(snmploc, dev_id)

-- Insert HW Information
position = "box"
-- Insert Chassis information and save id; Important: Always use "box" for Chassis position because of wktools internal special handling
chassis_id = ParserDB:insertHardware(position, hwpid, hwdescr, hwsn, "", "", "", version, dev_id, "")

--------------------------------------------------------------------------------------------------------------
-- Helper Function for Interface Normalizer since WAAS interface names are different and therefore Interface normalizer from wktools is not usable
function intfNormalizer(currentInt)
	-- Split Intf-Name and number
	ipos1, ipos2 = currentInt:find("[%a ]*")
	local intfName = currentInt:sub(ipos1, ipos2)
	local ifName = ""
	
	if intfName:match("nline") then
		ifName = "Inline"
	elseif intfName:match("igabit") then
		ifName = "Gi"
	end

	currentInt = currentInt:gsub(intfName, ifName)
	currentInt = currentInt:gsub("%s", "/")
	
	return currentInt
end

--------------------------------------------------------------------------------------------------------------

-- "show interface inlineport 1/1 wan" && "show interface inlineport 1/1 lan" && "show interface gigabitEthernet 1/0" Parser
ParserDB:printString("show interfaces brief & show interfaces all\r\n")

function readIntfAll(iall)
	ipos1, ipos2 = iall:find("show interface ")
	ipos1, ipos2 = iall:find("[%g ]*", ipos2+1)
	currentInt = iall:sub(ipos1, ipos2)
	ipos1, ipos2 = currentInt:find("%g*")

	currentInt = intfNormalizer(currentInt)
	
	-- MAC Address if available
	local macAddress = ""
	ipos1, ipos2 = iall:find("Ethernet Address                    : ")
	if ipos1 ~= nil then
	    ipos1, ipos2 = iall:find("%g*", ipos2+1)
	    macAddress = iall:sub(ipos1, ipos2)
	end

	-- IP Address if available
	local ipAddress = ""
	ipos1, ipos2 = iall:find("Internet Address                    : ")
	if ipos1 ~= nil then
	    ipos1, ipos2 = iall:find("%g*", ipos2+1)
	    ipAddress = iall:sub(ipos1, ipos2)
		if ipAddress == "--" then
			ipAddress = ""
		end
	end

	-- Subnetmask if available
	local mask = ""
	ipos1, ipos2 = iall:find("Netmask                             : ")
	if ipos1 ~= nil then
	    ipos1, ipos2 = iall:find("%g*", ipos2+1)
	    mask = iall:sub(ipos1, ipos2)
		if mask == "--" then
			mask = ""
		end
	end

	-- Admin State if available
	local iStatus = ""
	ipos1, ipos2 = iall:find("Admin State                         : ")
	if ipos1 ~= nil then
	    ipos1, ipos2 = iall:find("%g*", ipos2+1)
	    iStatus = iall:sub(ipos1, ipos2)
	end

	-- Operational State if available
	local iopStatus = ""
	ipos1, ipos2 = iall:find("Operation State                     : ")
	if ipos1 ~= nil then
	    ipos1, ipos2 = iall:find("%g*", ipos2+1)
	    iopStatus = iall:sub(ipos1, ipos2)
		iopStatus = trim(iopStatus)
		if iopStatus == "Running" then
			iopStatus = "Up"
		end
	end
	
	-- Errors
	ipos1, ipos2 = iall:find("Input Errors                        : ")
	ipos1, ipos2 = iall:find("%d*", ipos2+1)
	local iErr = iall:sub(ipos1, ipos2)
	
	ipos1, ipos2 = iall:find("Input Packets Dropped               : ")
	ipos1, ipos2 = iall:find("%d*", ipos2+1)
	local iDrop = iall:sub(ipos1, ipos2)

	ipos1, ipos2 = iall:find("Output Errors                       : ")
	ipos1, ipos2 = iall:find("%d*", ipos2+1)
	local oErr = iall:sub(ipos1, ipos2)

	ipos1, ipos2 = iall:find("Output Packets Dropped              : ")
	ipos1, ipos2 = iall:find("%d*", ipos2+1)
	local oDrop = iall:sub(ipos1, ipos2)

	ipos1, ipos2 = iall:find("Output Packets Dropped              : ")
	ipos1, ipos2 = iall:find("%d*", ipos2+1)
	local oDrop = iall:sub(ipos1, ipos2)

	-- Spped + Duplex
	ipos1, ipos2 = iall:find("Full Duplex                         : ")
	ipos1, ipos2 = iall:find("%g*", ipos2+1)
	local duplex = iall:sub(ipos1, ipos2)
	if duplex == "Yes" then
		duplex = "Full-Duplex"
	else
		duplex = "Half-Duplex"
	end

	ipos1, ipos2 = iall:find("Speed                               : ")
	ipos1, ipos2 = iall:find("[%g ]*", ipos2+1)
	local speed = iall:sub(ipos1, ipos2)
		
	return currentInt, macAddress, ipAddress, mask, iStatus, iopStatus, iErr, iDrop, oErr, oDrop, duplex, speed
end

for iall in iComplete:gmatch("show.-#") do 
    -- 1) Split all Interface output into chunks - one per interface 
	-- 2) Find all needed data
	-- 3) Write Interface into DB
	
	if #iall > 160 then		
		-- Read Details for this interface:
		currentInt, macAddress, ipAddress, mask, iStatus, iopStatus, iErr, iDrop, oErr, oDrop, duplex, speed = readIntfAll(iall)

		iSets = intfSets()

		-- Interface Normalizers:
		-- intfNameChange to shorten the Interface Name if needed/possible
		-- intfTypeCheck to unify the Interface operational speed/type
		-- intfPhlCheck to check whether the interface is a phyical or logical interface
		iSets.intfName = ParserDB:intfNameChange(currentInt)
		iSets.intfType = ParserDB:intfTypeCheck(currentInt, speed)
		iSets.phl = ParserDB:intfPhlCheck(currentInt, speed)
		iSets.nameif = ""
		iSets.macAddress = ParserDB:macAddChange(macAddress)
		iSets.ipAddress = ipAddress
		iSets.subnetMask = mask
		iSets.duplex = duplex
		iSets.speed = speed
		iSets.status = iopStatus
		iSets.description = ""
		iSets.l2l3 = "L2"
		iSets.dev_id = dev_id
		
		-- Insert Interface to DB
		intf_id = ParserDB:insertInterface(iSets)
		
		-- Insert Interface Statistics to DB
		errLvl = tonumber(iErr) + tonumber(iDrop) + tonumber(oErr) + tonumber(oDrop)
		iStats = intfStats()
		iStats.errLvl = trim(tostring(errLvl))
		iStats.lastClear = ""
		iStats.loadLvl = "0"
		iStats.iCrc = ""
		iStats.iFrame = ""
		iStats.iOverrun = ""
		iStats.iIgnored = ""
		iStats.iWatchdog = ""
		iStats.iPause = ""
		iStats.iDribbleCondition = ""
		iStats.ibuffer = ""
		iStats.l2decodeDrops = ""
		iStats.runts = ""
		iStats.giants = ""
		iStats.throttles = ""
		iStats.ierrors = trim(iErr)
		iStats.underrun = ""
		iStats.oerrors = trim(oErr)
		iStats.oCollisions = ""
		iStats.oBabbles = ""
		iStats.oLateColl = ""
		iStats.oDeferred = ""
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

-- "show cdp neighbor detail" Parser
ParserDB:printString("show cdp neighbor detail\r\n")

apos1 = 0
apos2 = 0
while true do
	apos1, apos2 = cdp:find("Device ID:.-Duplex.-\n", apos2)
	if apos1 == nil then
		break
	end
	cdpNeighbor = cdp:sub(apos1, apos2)
	
	local cpos1, cpos2 = cdpNeighbor:find("Device ID: ")
	cpos1, cpos2 = cdpNeighbor:find("[%g ]*", cpos2+1)
	adeviceID = cdpNeighbor:sub(cpos1, cpos2)
	cpos1, cpos2 = adeviceID:find("%g-%.")
	deviceID = adeviceID:sub(cpos1, cpos2-1)
	

	cpos1, cpos2 = cdpNeighbor:find("IP address: ")
	cpos1, cpos2 = cdpNeighbor:find("[%g]*", cpos2+1)	
	ipAddress = cdpNeighbor:sub(cpos1, cpos2)

	cpos1, cpos2 = cdpNeighbor:find("Platform: ")
	cpos1, cpos2 = cdpNeighbor:find("[%g ]*,", cpos2+1)	
	platform = cdpNeighbor:sub(cpos1, cpos2-1)

	cpos1, cpos2 = cdpNeighbor:find("Capabilities: ")
	cpos1, cpos2 = cdpNeighbor:find("[%g ]*", cpos2+1)	
	capability = cdpNeighbor:sub(cpos1, cpos2)
	capability = trim(capability)
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
	if capability == "Switch IGMP" then
		nType = "2"
	elseif capability == "Router Switch" then
		nType = "8"
	elseif capability == "Router Switch IGMP" then
		nType = "8"
	else
		nType = "99"
	end
	


	cpos1, cpos2 = cdpNeighbor:find("Interface: ")
	cpos1, cpos2 = cdpNeighbor:find("%g*", cpos2+1)	
	localPort = cdpNeighbor:sub(cpos1, cpos2-1)
	localPort = intfNormalizer(localPort)
	localPort = ParserDB:intfNameChange(localPort)

	cpos1, cpos2 = cdpNeighbor:find("Port ID %(outgoing port%): ")
	cpos1, cpos2 = cdpNeighbor:find("[%g ]*", cpos2+1)	
	remotePort = cdpNeighbor:sub(cpos1, cpos2)
	remotePort = ParserDB:intfNameChange(remotePort)

	cpos1, cpos2 = cdpNeighbor:find("Version :")
	cpos1, cpos2 = cdpNeighbor:find("[%g ]*", cpos2+1)	
	version = cdpNeighbor:sub(cpos1, cpos2)

	-- Add neighbor to CDP table
	ParserDB:insertCDP(hostname, deviceID, adeviceID, localPort, ipAddress,remotePort, nType, platform, version, dev_id)
end

--------------------------------------------------------------------------------------------------------------

-- WAE-ZRCH1#show arp
-- Protocol  Address          Flags      Hardware Addr     Type  Interface
-- Internet  10.11.1.6        Adj        D4:85:64:5E:8D:10 ARPA  InlinePort 1/1/wan
-- Internet  10.11.1.142      Adj        18:3D:A2:86:0F:F8 ARPA  InlinePort 1/1/wan
-- Internet  10.11.1.254      Adj        A4:18:75:B7:D3:41 ARPA  InlinePort 1/1/wan
-- Internet  10.11.1.1        Adj        00:50:56:B2:4B:4D ARPA  InlinePort 1/1/wan

-- "show arp" Parser
ParserDB:printString("show arp\r\n")
i = 0
for line in arp:gmatch(".-\n") do 
	if (#line > 10) and (i > 1) then
		local atble = split(line, "%s")
		local ipAddress = atble[2]
		local macAddress = ParserDB:macAddChange(atble[4])
		local port = atble[6]..atble[7]
		port = intfNormalizer(port)
		
		-- Get intf_id for Interface-Name
		local i_id = ParserDB:getIntfID(dev_id, port, true)

		-- Insert ARP entry to neighbor table (DB)
		ParserDB:insertNeighbor(i_id, macAddress, ipAddress, "")
	end
	i=i+1
end

--------------------------------------------------------------------------------------------------------------

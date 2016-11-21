dev_id = ParserDB:insertDevice("TestBox", "55", "55", "0x0000")
ParserDB:schreibeLog("Device-ID = " .. dev_id .. "\r\n")
dev_id = ParserDB:insertDevice("TestBox1", "551", "551", "0x0001")
ParserDB:schreibeLog("Device-ID = " .. dev_id .. "\r\n")
dev_id = ParserDB:insertDevice("TestBox1", dev_id, dev_id, "0x0001")
ParserDB:schreibeLog("Device-ID = " .. dev_id .. "\r\n")

ganzeDatei = ParserDB:getStringToParse()
ParserDB:schreibeLog(ganzeDatei)

iStats = intfStats()
iStats.errLvl = "1234567"
ParserDB:schreibeLog("Err-Lvl " .. iStats.errLvl .. "\r\n")

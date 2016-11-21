#include "class_wkmVisioAusgabe.h"
#ifdef WKTOOLS_MAPPER

#include <boost/lexical_cast.hpp>

#include "visio.h"
#include "quickref.h"

#include <iostream>
#include <fstream>

// Function-Pointer für Aufruf der Koordinaten-Funktionen
void(WkmVisioAusgabe::*xyCoord)(std::string, std::string, std::string, bool);

WkmVisioAusgabe::WkmVisioAusgabe(std::string aDat, WkmDB *db)
{
	dieDB = db;
	aDatName = aDat;

	// Falls ie .vdx Endung fehlt, wird sie hinugefügt
	if (aDatName.find(".vdx", aDatName.length()-5) == aDatName.npos)
	{
		aDatName += ".vdx";
	}
	nodeID = 100000;
	pId = 0;
	pageID = "0";
	pageName = "L2-1";

	vdxAusgabe.rdbuf()->open(aDatName.c_str(), std::ios_base::out | std::ios_base::binary);
	//vdxAusgabe << visioVdxStart;
	vdxAusgabe.write(visioVdxStart, VISIOVDXSTART_LEN);
}


WkmVisioAusgabe::~WkmVisioAusgabe()
{
	vdxAusgabe.write(visioVdxQuickRef, VISIOVDXQUICKREF_LEN);
	vdxAusgabe << "</Pages></VisioDocument>";
	vdxAusgabe.close();
}


void WkmVisioAusgabe::doIt(std::string gmlFile, std::string page, int istyle=0, int t=0)
{
	// tpye 0: Default Ausgabe -> Nodes mit Default-Info und Edges ohne Shape Daten
	// type 1: Ausgabe für Edges mit Shape Daten für L3 Routing Infos und Nodes zusätzlich mit L3 Routing Infos
	type = t;
	
	intfStyle = istyle;
	gmlInput = gmlFile;
	pageName = page;

	if (pageName.find("L2 ") != pageName.npos)
	{
		xyCoord = &WkmVisioAusgabe::insertL2Coord;
	}
	else if (pageName == "L3-Organic")
	{
		xyCoord = &WkmVisioAusgabe::insertL3Coord;
	}
	else if (pageName == "L3RP-Organic")
	{
		xyCoord = &WkmVisioAusgabe::insertL3RPCoord;
	}
	else
	{
		xyCoord = NULL;
	}

	
	vdxAusgabe << "\r\n<Page ID=\"";
	vdxAusgabe << pageID << "\" NameU=\"" << pageName << "\" ViewScale=\"1\" ViewCenterX=\"4.1092519685039\" ViewCenterY=\"6.8208661417323\">";
	vdxAusgabe << "\r\n<PageSheet LineStyle=\"0\" FillStyle=\"0\" TextStyle=\"0\">\r\n<PageProps>\r\n<PageWidth>8.26771653543307</PageWidth> ";
	vdxAusgabe << "\r\n<PageHeight>11.69291338582677</PageHeight><PageScale Unit=\"MM\">0.03937007874015748</PageScale>";
	vdxAusgabe << "\r\n<DrawingScale Unit=\"MM\">0.03937007874015748</DrawingScale></PageProps></PageSheet><Shapes>";

	pId++;
	pageID = boost::lexical_cast<std::string>(pId);
	
	size_t pos1 = 0;
	size_t pos2 = 0;
	size_t clusterPos = gmlInput.find("rootcluster");

	// Nodes auslesen
	while (pos1 < clusterPos)
	{
		pos1 = gmlInput.find("node", pos2);
		if (pos1 < clusterPos)
		{
			pos1 = gmlInput.find("id", pos1) + 3;
			pos2 = gmlInput.find_first_of("\r\n", pos1);
			std::string id = gmlInput.substr(pos1, pos2-pos1);
			if (id == "0")
			{
				id = "99999";
			}
			
			pos1 = gmlInput.find("label", pos2) + 7;
			pos2 = gmlInput.find("\"", pos1);
			std::string label = gmlInput.substr(pos1, pos2-pos1);

			pos1 = gmlInput.find("x", pos2) + 2;
			pos2 = gmlInput.find(".", pos1);
			std::string x = gmlInput.substr(pos1, pos2-pos1);
			x.insert(x.length()-2, ".");
			float xint = boost::lexical_cast<float>(x);
			xint = xint/2;
			x = boost::lexical_cast<std::string>(xint);

			pos1 = gmlInput.find("y", pos2) + 2;
			pos2 = gmlInput.find(".", pos1);
			std::string y = gmlInput.substr(pos1, pos2-pos1);
			y.insert(y.length()-2, ".");
			float yint = boost::lexical_cast<float>(y);
			yint = yint/2;
			y = boost::lexical_cast<std::string>(yint);


			pos1 = gmlInput.find("fill", pos2) + 6;
			pos2 = gmlInput.find("\"", pos1);
			std::string farbe = gmlInput.substr(pos1+1, pos2-pos1-1);

			std::string nodeXML = farbeNodeUmsetzer(farbe, id, x, y, label);

			vdxAusgabe << nodeXML;
		}
	}
	
	std::string connects = "\r\n<Connects>";			// Connects am Ende vom XML File
	pos1 = pos2 = 0;
	// Edges auslesen
	while (pos1 < clusterPos)
	{
		pos1 = gmlInput.find("edge", pos2);
		if (pos1 < gmlInput.npos)
		{
			pos1 = gmlInput.find("source", pos1) + 7;
			pos2 = gmlInput.find_first_of("\r\n", pos1);
			std::string source = gmlInput.substr(pos1, pos2-pos1);
			if (source == "0")
			{
				source = "99999";
			}

			
			pos1 = gmlInput.find("target", pos2) + 7;
			pos2 = gmlInput.find_first_of("\r\n", pos1);
			std::string target = gmlInput.substr(pos1, pos2-pos1);
			if (target == "0")
			{
				target = "99999";
			}

			std::string subID1 = boost::lexical_cast<std::string>(nodeID);
			nodeID++;
			std::string nodeXML = "\r\n<Shape ID=\"" + subID1 + "\" Type=\"Shape\" Master=\"10\">";
			if (type == 1)
			{
				if (interfaces.find(source) != interfaces.end() && interfaces.find(target) != interfaces.end())
				{
					nodeXML += "<LinePattern>5</LinePattern>";
				}
				if (cryptoInterfaces.find(source) != cryptoInterfaces.end() && cryptoInterfaces.find(target) != cryptoInterfaces.end())
				{
					nodeXML += "<LineWeight>0.04166666666666666</LineWeight><LineColor>#00b050</LineColor>";
					// TODO: Linien dicker zeichnen  - nodeXML += "<LinePattern>5</LinePattern>";
				}
			}

			nodeXML += "<Geom IX=\"0\"><MoveTo IX=\"" + source + "\"></MoveTo><LineTo IX=\"" + target + "\"></LineTo></Geom>";

			//nodeXML += "<Prop NameU=\"Row_" + subID1 + "\" ID=\"" + subID1 + "\"><Value Unit=\"STR\">" + "TEST" + "</Value><Prompt F=\"No Formula\" />";
			//nodeXML += "<Label>Test</Label><Format>@</Format><SortKey F=\"No Formula\" /><Type>0</Type><Invisible F=\"No Formula\">0</Invisible>";
			//nodeXML += "<Verify F=\"No Formula\">0</Verify><LangID>3079</LangID><Calendar F=\"No Formula\">0</Calendar></Prop>";

			nodeXML += "</Shape>";

			vdxAusgabe << nodeXML;

			connects += "\r\n<Connect FromSheet=\"" + subID1 + "\" FromCell=\"BeginX\" FromPart=\"9\" ToSheet=\"" + source + "\" ToCell=\"PinX\" ToPart=\"3\" />"; 
			connects += "\r\n<Connect FromSheet=\"" + subID1 + "\" FromCell=\"EndX\" FromPart=\"12\" ToSheet=\"" + target + "\" ToCell=\"PinX\" ToPart=\"3\" />"; 

		}
	}
	connects += "\r\n</Connects>";

	vdxAusgabe << "\r\n</Shapes>";
	vdxAusgabe << connects;
	vdxAusgabe << "\r\n</Page>";
}


std::string WkmVisioAusgabe::farbeNodeUmsetzer(std::string farbe, std::string id, std::string x, std::string y, std::string label)
{
	// Koordinaten Helper //
	bool device = true;			// Bei false ist es ein Interface
	std::string dbId = "0";		// dev_id oder intf_id

	// Den CDP Zusatz vom Nodename löschen
	size_t lzPos = label.find(" (CDP)");
	if (lzPos != label.npos)
	{
		dbId = label.substr(0, lzPos);
	}
	else
	{
		dbId = label;
	}
	// ... //
	
	std::string hostname = "";
	
	std::string sString = "SELECT hostname,type,dataSource,foStatus FROM device WHERE dev_id=" + label + ";";
	std::vector<std::vector<std::string>> hnret = dieDB->query(sString.c_str());
	std::vector<std::vector<std::string>>::iterator hnit = hnret.begin();

	if (hnit < hnret.end())
	{
		if (hnit->at(0) != "")
		{
			hostname += hnit->at(0);
		}
		else
		{
			hostname = label;
		}
	}

	std::string ret = "\r\n";
	if (farbe == "90EE90")		// Router
	{
		std::string subID1 = boost::lexical_cast<std::string>(nodeID);
		nodeID++;
		std::string subID2 = boost::lexical_cast<std::string>(nodeID);
		nodeID++;

		ret += "<Shape ID=\"" + id + "\" Type=\"Group\" Master=\"0\"><XForm><PinX>";
		ret += x + "</PinX><PinY>" + y + "</PinY></XForm>";
		ret += shapeData(label, "L3");
		ret += "<Shapes><Shape ID=\"";
		ret += subID1 + "\" NameU=\"Router (colored)." + id + "\" Type=\"Foreign\" MasterShape=\"6\"></Shape><Shape ID=\"";
		ret += subID2 + "\" Type=\"Shape\" MasterShape=\"7\"><Text>" + hostname + "</Text></Shape></Shapes></Shape>";
	}
	else if (farbe == "90EE91")		// VoiceRouter
	{
		std::string subID1 = boost::lexical_cast<std::string>(nodeID);
		nodeID++;
		std::string subID2 = boost::lexical_cast<std::string>(nodeID);
		nodeID++;
		
		ret += "<Shape ID=\"" + id + "\" Type=\"Group\" Master=\"11\"><XForm><PinX>";
		ret += x + "</PinX><PinY>" + y + "</PinY></XForm>";
		ret += shapeData(label, "L3");
		ret += "<Shapes><Shape ID=\"";
		ret += subID1 + "\" NameU=\"VoiceEnabled Router." + id + "\" Type=\"Foreign\" MasterShape=\"6\"></Shape><Shape ID=\"";
		ret += subID2 + "\" Type=\"Shape\" MasterShape=\"7\"><Text>" + hostname + "</Text></Shape></Shapes></Shape>";
	}
	else if (farbe == "66CDAA")		// L2 Switch
	{
		std::string subID1 = boost::lexical_cast<std::string>(nodeID);
		nodeID++;
		std::string subID2 = boost::lexical_cast<std::string>(nodeID);
		nodeID++;

		ret += "<Shape ID=\"" + id + "\" Type=\"Group\" Master=\"3\"><XForm><PinX>";
		ret += x + "</PinX><PinY>" + y + "</PinY></XForm>";
		ret += shapeData(label, "L2");
		ret += "<Shapes><Shape ID=\"";
		ret += subID1 + "\" NameU=\"Workgroup Switch (colored)." + id + "\" Type=\"Foreign\" MasterShape=\"6\"></Shape><Shape ID=\"";
		ret += subID2 + "\" Type=\"Shape\" MasterShape=\"7\"><Text>" + hostname + "</Text></Shape></Shapes></Shape>";
	}
	else if (farbe == "66CDAB")		// L2 Switch Root
	{
		std::string subID1 = boost::lexical_cast<std::string>(nodeID);
		nodeID++;
		std::string subID2 = boost::lexical_cast<std::string>(nodeID);
		nodeID++;

		ret += "<Shape ID=\"" + id + "\" Type=\"Group\" Master=\"12\"><XForm><PinX>";
		ret += x + "</PinX><PinY>" + y + "</PinY></XForm>";
		ret += shapeData(label, "L2");
		ret += "<Shapes><Shape ID=\"";
		ret += subID1 + "\" NameU=\"L2Root." + id + "\" Type=\"Foreign\" MasterShape=\"6\"></Shape><Shape ID=\"";
		ret += subID2 + "\" Type=\"Shape\" MasterShape=\"7\"><Text>" + hostname + "</Text></Shape></Shapes></Shape>";
	}
	else if (farbe == "66CDAC")		// L3 Switch
	{
		std::string subID1 = boost::lexical_cast<std::string>(nodeID);
		nodeID++;
		std::string subID2 = boost::lexical_cast<std::string>(nodeID);
		nodeID++;

		ret += "<Shape ID=\"" + id + "\" Type=\"Group\" Master=\"2\"><XForm><PinX>";
		ret += x + "</PinX><PinY>" + y + "</PinY></XForm>";
		ret += shapeData(label, "L2");
		ret += "<Shapes><Shape ID=\"";
		ret += subID1 + "\" NameU=\"L3Switch." + id + "\" Type=\"Foreign\" MasterShape=\"6\"></Shape><Shape ID=\"";
		ret += subID2 + "\" Type=\"Shape\" MasterShape=\"7\"><Text>" + hostname + "</Text></Shape></Shapes></Shape>";
	}
	else if (farbe == "66CDAD")		// L3 Switch Root
	{
		std::string subID1 = boost::lexical_cast<std::string>(nodeID);
		nodeID++;
		std::string subID2 = boost::lexical_cast<std::string>(nodeID);
		nodeID++;

		ret += "<Shape ID=\"" + id + "\" Type=\"Group\" Master=\"13\"><XForm><PinX>";
		ret += x + "</PinX><PinY>" + y + "</PinY></XForm>";
		ret += shapeData(label, "L2");
		ret += "<Shapes><Shape ID=\"";
		ret += subID1 + "\" NameU=\"L3Root." + id + "\" Type=\"Foreign\" MasterShape=\"6\"></Shape><Shape ID=\"";
		ret += subID2 + "\" Type=\"Shape\" MasterShape=\"7\"><Text>" + hostname + "</Text></Shape></Shapes></Shape>";
	}
	else if (farbe == "66CDDD")		// Accesspoint
	{
		std::string subID1 = boost::lexical_cast<std::string>(nodeID);
		nodeID++;
		std::string subID2 = boost::lexical_cast<std::string>(nodeID);
		nodeID++;

		ret += "<Shape ID=\"" + id + "\" Type=\"Group\" Master=\"8\"><XForm><PinX>";
		ret += x + "</PinX><PinY>" + y + "</PinY></XForm>";
		ret += shapeData(label, "L3");
		ret += "<Shapes><Shape ID=\"";
		ret += subID1 + "\" NameU=\"Access Point." + id + "\" Type=\"Foreign\" MasterShape=\"6\"></Shape><Shape ID=\"";
		ret += subID2 + "\" Type=\"Shape\" MasterShape=\"7\"><Text>" + hostname + "</Text></Shape></Shapes></Shape>";
	}
	else if (farbe == "87CEFA")		// Firewall
	{

		std::string subID1 = boost::lexical_cast<std::string>(nodeID);
		nodeID++;
		std::string subID2 = boost::lexical_cast<std::string>(nodeID);
		nodeID++;

		ret += "<Shape ID=\"" + id + "\" Type=\"Group\" Master=\"4\"><XForm><PinX>";
		ret += x + "</PinX><PinY>" + y + "</PinY></XForm>";
		ret += shapeData(label, "L3");
		// Failover Status
		if (hnit->at(3) != "")
		{
			hostname += " / " + hnit->at(3);
		}
		ret += "<Shapes><Shape ID=\"";
		ret += subID1 + "\" NameU=\"PIX (right)." + id + "\" Type=\"Foreign\" MasterShape=\"6\"></Shape><Shape ID=\"";
		ret += subID2 + "\" Type=\"Shape\" MasterShape=\"7\"><Text>" + hostname + "</Text></Shape></Shapes></Shape>";
	}
	else if (farbe == "FFB6C1")		// Endgerät
	{
		std::string subID1 = boost::lexical_cast<std::string>(nodeID);
		nodeID++;
		std::string subID2 = boost::lexical_cast<std::string>(nodeID);
		nodeID++;

		ret += "<Shape ID=\"" + id + "\" Type=\"Group\" Master=\"6\"><XForm><PinX>";
		ret += x + "</PinX><PinY>" + y + "</PinY></XForm>";
		ret += shapeData(label, "E");
		ret += "<Shapes><Shape ID=\"";
		ret += subID1 + "\" NameU=\"File Server." + id + "\" Type=\"Foreign\" MasterShape=\"6\"></Shape><Shape ID=\"";
		ret += subID2 + "\" Type=\"Shape\" MasterShape=\"7\"><Text>" + hostname + "</Text></Shape></Shapes></Shape>";
	}
	else if (farbe == "A9A9A9")		// IP Phone
	{
		// Check ob snmpLoc befüllt. Wenn ja, dann als Node Name verwenden
		std::string sString = "SELECT snmpLoc FROM device WHERE dev_id=" + hostname + " ";
		sString += "AND dev_id > (SELECT MAX(dev_id) FROM rControl WHERE dev_id < (SELECT MAX(dev_id) FROM rControl));";
		std::vector<std::vector<std::string>> sqlRet = dieDB->query(sString.c_str());
		std::vector<std::vector<std::string>>::iterator it = sqlRet.begin();

		std::string ipPhoneLabel = hostname;
		if (it != sqlRet.end())
		{
			if (sqlRet[0][0] != "")
			{
				ipPhoneLabel = sqlRet[0][0];
			}
		}

		std::string subID1 = boost::lexical_cast<std::string>(nodeID);
		nodeID++;
		std::string subID2 = boost::lexical_cast<std::string>(nodeID);
		nodeID++;

		ret += "<Shape ID=\"" + id + "\" Type=\"Group\" Master=\"7\"><XForm><PinX>";
		ret += x + "</PinX><PinY>" + y + "</PinY></XForm>";
		ret += shapeData(label, "P");
		ret += "<Shapes><Shape ID=\"";
		ret += subID1 + "\" NameU=\"Phone." + id + "\" Type=\"Foreign\" MasterShape=\"6\"></Shape><Shape ID=\"";
		ret += subID2 + "\" Type=\"Shape\" MasterShape=\"7\"><Text>" + ipPhoneLabel + "</Text></Shape></Shapes></Shape>";
	}
	else if (farbe == "A9A9A6")		// ATA
	{
		std::string subID1 = boost::lexical_cast<std::string>(nodeID);
		nodeID++;
		std::string subID2 = boost::lexical_cast<std::string>(nodeID);
		nodeID++;

		ret += "<Shape ID=\"" + id + "\" Type=\"Group\" Master=\"14\"><XForm><PinX>";
		ret += x + "</PinX><PinY>" + y + "</PinY></XForm>";
		ret += shapeData(label, "P");
		ret += "<Shapes><Shape ID=\"";
		ret += subID1 + "\" NameU=\"ATA." + id + "\" Type=\"Foreign\" MasterShape=\"6\"></Shape><Shape ID=\"";
		ret += subID2 + "\" Type=\"Shape\" MasterShape=\"7\"><Text>" + hostname + "</Text></Shape></Shapes></Shape>";
	}
	else if (farbe == "A9A9A5")		// Camera
	{
		std::string subID1 = boost::lexical_cast<std::string>(nodeID);
		nodeID++;
		std::string subID2 = boost::lexical_cast<std::string>(nodeID);
		nodeID++;

		ret += "<Shape ID=\"" + id + "\" Type=\"Group\" Master=\"16\"><XForm><PinX>";
		ret += x + "</PinX><PinY>" + y + "</PinY></XForm>";
		ret += shapeData(label, "E");
		ret += "<Shapes><Shape ID=\"";
		ret += subID1 + "\" NameU=\"Camera." + id + "\" Type=\"Foreign\" MasterShape=\"6\"></Shape><Shape ID=\"";
		ret += subID2 + "\" Type=\"Shape\" MasterShape=\"7\"><Text>" + hostname + "</Text></Shape></Shapes></Shape>";
	}
	else if (farbe == "A9A9A7")		// Netzwerk
	{
		ret += "<Shape ID=\"" + id + "\" Type=\"Group\" Master=\"5\"><XForm><PinX>";
		ret += x + "</PinX><PinY>" + y + "</PinY></XForm>";
		ret += shapeData(label, "N");
		ret += "<Text><cp IX=\"0\" />" + label + "</Text></Shape>";
	}
	else if (farbe == "A9A9A0")		// Cloud White
	{
		std::string subID1 = boost::lexical_cast<std::string>(nodeID);
		nodeID++;
		std::string subID2 = boost::lexical_cast<std::string>(nodeID);
		nodeID++;

		ret += "<Shape ID=\"" + id + "\" Type=\"Group\" Master=\"17\"><XForm><PinX>";
		ret += x + "</PinX><PinY>" + y + "</PinY></XForm>";
		ret += shapeDataCloud(label, "N");
		ret += "<Shapes><Shape ID=\"";
		ret += subID1 + "\" NameU=\"Network Cloud (white)." + id + "\" Type=\"Foreign\" MasterShape=\"6\"></Shape><Shape ID=\"";
		ret += subID2 + "\" Type=\"Shape\" MasterShape=\"7\"><Text>" + hostname + "</Text></Shape></Shapes></Shape>";
	}
	else if (farbe == "A9A9A1")		// Cloud
	{
		std::string subID1 = boost::lexical_cast<std::string>(nodeID);
		nodeID++;
		std::string subID2 = boost::lexical_cast<std::string>(nodeID);
		nodeID++;

		ret += "<Shape ID=\"" + id + "\" Type=\"Group\" Master=\"19\"><XForm><PinX>";
		ret += x + "</PinX><PinY>" + y + "</PinY></XForm>";
		ret += shapeDataCloud(label, "N");
		ret += "<Shapes><Shape ID=\"";
		ret += subID1 + "\" NameU=\"Network Cloud." + id + "\" Type=\"Foreign\" MasterShape=\"6\"></Shape><Shape ID=\"";
		ret += subID2 + "\" Type=\"Shape\" MasterShape=\"7\"><Text>" + hostname + "</Text></Shape></Shapes></Shape>";
	}
	else if (farbe == "A9A9A8")		// Cloud Gold
	{
		std::string subID1 = boost::lexical_cast<std::string>(nodeID);
		nodeID++;
		std::string subID2 = boost::lexical_cast<std::string>(nodeID);
		nodeID++;

		ret += "<Shape ID=\"" + id + "\" Type=\"Group\" Master=\"18\"><XForm><PinX>";
		ret += x + "</PinX><PinY>" + y + "</PinY></XForm>";
		ret += shapeDataCloud(label, "D");
		ret += "<Shapes><Shape ID=\"";
		ret += subID1 + "\" NameU=\"Network Cloud (gold)." + id + "\" Type=\"Foreign\" MasterShape=\"6\"></Shape><Shape ID=\"";
		ret += subID2 + "\" Type=\"Shape\" MasterShape=\"7\"><Text>" + hostname + "</Text></Shape></Shapes></Shape>";
	}
	else							// Interface
	{
		device = false;
		std::string interfaceFarbe = "";
		std::string errorFarbe = "";
		std::string duplexFarbe = "";
		std::string loadFarbe = "";
		std::string stpFarbe = "";

		intfLoad = false;
		intfErr = false;
		intfStp = false;
		intfDuplex = false;

		std::size_t intfid_pos = label.find("-");
		std::string intfID = label.substr(0, intfid_pos);
		label.erase(0, intfid_pos+1);

		dbId = intfID;

		// Interface Indikator
		if (farbe[0] == '0')				// Normal
		{
			interfaceFarbe = "#0101DF";
		}
		else if (farbe[0] == '1')			// Muss kontrolliert werden
		{
			interfaceFarbe = "#edf92e";
		}
		else if (farbe[0] == '2')			// Unknown Neighbor
		{
			interfaceFarbe = "#00b0f0";
		}
		else if (farbe[0] == '3')			// STP Blocking
		{
			interfaceFarbe = "#ff7f50";
		}
		else if (farbe[0] == '4')			// STP Mixed
		{
			interfaceFarbe = "#f08080";
		}
		else if (farbe[0] == '5')			// VPN
		{
			interfaceFarbe = "#6D7B8D";
		}

		insertIntfColor(interfaceFarbe, intfID);

		// Error Indikator
		if (farbe[2] == '0')				// Normal
		{
			errorFarbe = "#008000";
		}
		else if (farbe[2] == '1')			// Warning
		{
			errorFarbe = "#ffa500";
			intfErr = true;
		}
		else if (farbe[2] == '2')			// Error
		{
			errorFarbe = "#ff0000";
			intfErr = true;
		}

		// Load Indikator
		if (farbe[3] == '0')				// Normal
		{
			loadFarbe = "#008000";
		}
		else if (farbe[3] == '1')			// Warning
		{
			loadFarbe = "#ffa500";
			intfLoad = true;
		}
		else if (farbe[3] == '2')			// Error
		{
			loadFarbe = "#ff0000";
			intfLoad = true;
		}

		// Duplex Indikator
		if (farbe[4] == '0')				// Normal
		{
			duplexFarbe = "#008000";
		}
		else if (farbe[4] == '1')			// Warning
		{
			duplexFarbe = "#ffa500";
			intfDuplex = true;
		}

		// STP Transition Indikator
		if (farbe[5] == '0')				// Normal
		{
			stpFarbe = "#008000";
		}
		else if (farbe[5] == '1')			// Warning
		{
			stpFarbe = "#ffa500";
			intfStp = true;
		}
		else if (farbe[5] == '2')			// Error
		{
			stpFarbe = "#ff0000";
			intfStp = true;
		}

		std::string subID1 = boost::lexical_cast<std::string>(nodeID);
		nodeID++;
		std::string subID2 = boost::lexical_cast<std::string>(nodeID);
		nodeID++;
		std::string subID3 = boost::lexical_cast<std::string>(nodeID);
		nodeID++;
		std::string subID4 = boost::lexical_cast<std::string>(nodeID);
		nodeID++;
		std::string subID6 = boost::lexical_cast<std::string>(nodeID);
		nodeID++;
		std::string subID5 = boost::lexical_cast<std::string>(nodeID);
		nodeID+=5;

		if (intfStyle == WKM_AUSGABE_INTF_STYLE_NORMAL)
		{
			ret += "<Shape ID=\"" + id + "\" Type=\"Group\" Master=\"15\"><XForm><PinX>" + x + "</PinX><PinY>";
			ret += y + "</PinY><Width>0.501968503937006</Width><Height>0.2345175603049621</Height></XForm>";
			ret += shapeDataIntf(label, "", intfID, id);
			ret += "<Shapes><Shape ID=\"" + subID1 + "\" Type=\"Group\" MasterShape=\"6\"><Shapes>";
			ret += "<Shape ID=\"" + subID2 + "\" Type=\"Shape\" MasterShape=\"7\"><Fill><FillForegnd>";
			ret += stpFarbe + "</FillForegnd></Fill></Shape><Shape ID=\"" + subID3 + "\" Type=\"Shape\" MasterShape=\"8\">";
			ret += "<Fill><FillForegnd>" + duplexFarbe + "</FillForegnd></Fill></Shape><Shape ID=\"";
			ret += subID4 + "\" Type=\"Shape\" MasterShape=\"9\"><Fill><FillForegnd>" + loadFarbe + "</FillForegnd></Fill></Shape>";
			ret += "<Shape ID=\"" + subID5 + "\" Type=\"Shape\" MasterShape=\"10\"><Fill><FillForegnd>";
			ret += errorFarbe + "</FillForegnd></Fill></Shape><Shape ID=\"" + subID6 + "\" Type=\"Shape\" MasterShape=";
			ret += "\"11\"><Fill><FillForegnd>" + interfaceFarbe + "</FillForegnd></Fill><Text>" + label + "</Text></Shape>";
			ret += "</Shapes></Shape></Shapes></Shape>";
		}
		else if (intfStyle == WKM_AUSGABE_INTF_STYLE_REDUCED)
		{
			ret += "<Shape ID=\"" + id + "\" Type=\"Group\" Master=\"21\"><XForm><PinX>" + x + "</PinX><PinY>" + y + "</PinY></XForm>";
			ret += shapeDataIntf(label, "", intfID, id);
			ret += "<Shapes><Shape ID=\"" + subID1 + "\" Type=\"Shape\" MasterShape=\"6\">";
			ret += "</Shape>";
			ret += "<Shape ID=\"" + subID2 + "\" Type=\"Shape\" MasterShape=\"7\">";
			ret += "<Fill><FillForegnd>" + interfaceFarbe + "</FillForegnd><FillBkgnd>#3c8aff</FillBkgnd><FillPattern>1</FillPattern></Fill><Char IX=\"0\"><Size>0.06944444444444445</Size></Char>";
			ret += "<Text><cp IX=\"0\"/>" + label +"</Text></Shape></Shapes></Shape>";
		}
		else if (intfStyle == WKM_AUSGABE_INTF_STYLE_REDUCED_NOLINE)
		{
			if (interfaceFarbe == "#ffffff")
			{
				interfaceFarbe = "#e0ffff";
			}
			ret += "<Shape ID=\"" + id + "\" Type=\"Group\" Master=\"20\"><XForm><PinX>" + x + "</PinX><PinY>" + y + "</PinY></XForm>";
			ret += shapeDataIntf(label, "", intfID, id);
			ret += "<Shapes><Shape ID=\"" + subID1 + "\" Type=\"Shape\" MasterShape=\"6\">";
			ret += "</Shape>";
			ret += "<Shape ID=\"" + subID2 + "\" Type=\"Shape\" MasterShape=\"7\">";
			ret += "<Fill><FillForegnd>" + interfaceFarbe + "</FillForegnd><FillBkgnd>#3c8aff</FillBkgnd><FillPattern>1</FillPattern></Fill><Char IX=\"0\"><Size>0.06944444444444445</Size></Char>";
			ret += "<Text><cp IX=\"0\"/>" + label +"</Text></Shape></Shapes></Shape>";
		}
	}

	if (xyCoord != NULL)
	{
		(*this.*xyCoord)(x, y, dbId, device);
	}

	return ret;

}

void WkmVisioAusgabe::insertIntfColor(std::string farbe, std::string id)
{
	std::string iString = "UPDATE interfaces SET intfColor='" + farbe + "' WHERE intf_id=" + id + ";";
	dieDB->query(iString.c_str());
}


void WkmVisioAusgabe::insertL2Coord(std::string x, std::string y, std::string id, bool device)
{
	std::string iString = "";
	if (device)
	{
		iString = "UPDATE device SET l2x=" + x + " ,l2y=" + y + " WHERE dev_id=" + id + ";";
	}
	else
	{
		iString = "UPDATE interfaces SET l2x=" + x + " ,l2y=" + y + " WHERE intf_id=" + id + ";";
	}
	dieDB->query(iString.c_str());
}

void WkmVisioAusgabe::insertL3Coord(std::string x, std::string y, std::string id, bool device)
{
	std::string iString = "";
	if (device)
	{
		iString = "UPDATE device SET l3x=" + x + " ,l3y=" + y + " WHERE dev_id=" + id + ";";
	}
	else
	{
		iString = "UPDATE interfaces SET l3x=" + x + " ,l3y=" + y + " WHERE intf_id=" + id + ";";
	}
	dieDB->query(iString.c_str());
}

void WkmVisioAusgabe::insertL3RPCoord(std::string x, std::string y, std::string id, bool device)
{
	std::string iString = "";
	if (device)
	{
		iString = "UPDATE device SET l3rx=" + x + " ,l3ry=" + y + " WHERE dev_id=" + id + ";";
	}
	else
	{
		iString = "UPDATE interfaces SET l3rx=" + x + " ,l3ry=" + y + " WHERE intf_id=" + id + ";";
	}
	dieDB->query(iString.c_str());
}



std::string WkmVisioAusgabe::shapeData(std::string nodeName, std::string type)
{
	// Type: 
	// L2: Switch
	// L3: NW Gerät, kein Switch
	// E: Endgerät
	// P: Phone
	// N: Netzwerk
	// C: Wolke
	std::string ausgabe = "";
	int propID = 10;
	std::string pid = boost::lexical_cast<std::string>(propID);

	// Den CDP Zusatz vom Nodename löschen
	size_t lzPos = nodeName.find(" (CDP)");
	bool cdp = false;
	if (lzPos != nodeName.npos)
	{
		nodeName = nodeName.substr(0, lzPos);
		cdp = true;
	}

	// Hostname
	std::string hostname = "";
	std::string hnLabel = "Hostname";
	if (type == "N")
	{
		hnLabel = "Subnet";
		hostname = nodeName;
	}
	else
	{
		if (type == "C")
		{
			hnLabel = "Cloud";

		}
		std::string sString = "SELECT hostname FROM device WHERE dev_id=" + nodeName;
		std::vector<std::vector<std::string>> ret = dieDB->query(sString.c_str());
		std::vector<std::vector<std::string>>::iterator it = ret.begin();

		propID++;
		pid = boost::lexical_cast<std::string>(propID);
		if (it < ret.end())
		{
			hostname = it->at(0);
		}
	}

	ausgabe += "<Prop NameU=\"Row_" + pid + "\" ID=\"" + pid + "\"><Value Unit=\"STR\">" + hostname + "</Value><Prompt F=\"No Formula\" />";
	ausgabe += "<Label>" + hnLabel + "</Label><Format>@</Format><SortKey F=\"No Formula\" /><Type>0</Type><Invisible F=\"No Formula\">0</Invisible>";
	ausgabe += "<Verify F=\"No Formula\">0</Verify><LangID>3079</LangID><Calendar F=\"No Formula\">0</Calendar></Prop>";

	// Failover Status
	std::string sString = "SELECT foStatus FROM device WHERE dev_id=" + nodeName;
	std::vector<std::vector<std::string>> ret = dieDB->query(sString.c_str());
	std::vector<std::vector<std::string>>::iterator it = ret.begin();

	propID++;
	pid = boost::lexical_cast<std::string>(propID);
	if (it < ret.end())
	{
		if (it->at(0) != "")
		{
			std::string foStatus = it->at(0);

			ausgabe += "<Prop NameU=\"Row_" + pid + "\" ID=\"" + pid + "\"><Value Unit=\"STR\">" + foStatus + "</Value><Prompt F=\"No Formula\" />";
			ausgabe += "<Label>Failover Status</Label><Format>@</Format><SortKey F=\"No Formula\" /><Type>0</Type><Invisible F=\"No Formula\">0</Invisible>";
			ausgabe += "<Verify F=\"No Formula\">0</Verify><LangID>3079</LangID><Calendar F=\"No Formula\">0</Calendar></Prop>";
		}
	}
	
	// SNMP Location
	sString = "SELECT snmpLoc,dataSource FROM device WHERE dev_id=" + nodeName;
	ret = dieDB->query(sString.c_str());
	it = ret.begin();

	propID++;
	pid = boost::lexical_cast<std::string>(propID);
	if (it < ret.end())
	{
		if (it->at(0) != "")
		{
			std::string snmpLoc = it->at(0);
			while (snmpLoc.find("&") != snmpLoc.npos)
			{
				snmpLoc.replace(snmpLoc.find("&"), 1, "+");
			}

			if (type == "P")
			{
				ausgabe += "<Prop NameU=\"Row_" + pid + "\" ID=\"" + pid + "\"><Value Unit=\"STR\">" + snmpLoc + "</Value><Prompt F=\"No Formula\" />";
				ausgabe += "<Label>Extension</Label><Format>@</Format><SortKey F=\"No Formula\" /><Type>0</Type><Invisible F=\"No Formula\">0</Invisible>";
				ausgabe += "<Verify F=\"No Formula\">0</Verify><LangID>3079</LangID><Calendar F=\"No Formula\">0</Calendar></Prop>";
			}
			else if (it->at(1) == "2") // Endgeräte, die aus der ARP Table ausgelesen wurden
			{
				ausgabe += "<Prop NameU=\"Row_" + pid + "\" ID=\"" + pid + "\"><Value Unit=\"STR\">" + it->at(0) + "</Value><Prompt F=\"No Formula\" />";
				ausgabe += "<Label>MAC Vendor</Label><Format>@</Format><SortKey F=\"No Formula\" /><Type>0</Type><Invisible F=\"No Formula\">0</Invisible>";
				ausgabe += "<Verify F=\"No Formula\">0</Verify><LangID>3079</LangID><Calendar F=\"No Formula\">0</Calendar></Prop>";
			}
			else
			{
				ausgabe += "<Prop NameU=\"Row_" + pid + "\" ID=\"" + pid + "\"><Value Unit=\"STR\">" + it->at(0) + "</Value><Prompt F=\"No Formula\" />";
				ausgabe += "<Label>SNMP Location</Label><Format>@</Format><SortKey F=\"No Formula\" /><Type>0</Type><Invisible F=\"No Formula\">0</Invisible>";
				ausgabe += "<Verify F=\"No Formula\">0</Verify><LangID>3079</LangID><Calendar F=\"No Formula\">0</Calendar></Prop>";
			}
		}
	}

	// DeviceType
	sString = "SELECT hwInfo.type FROM hwInfo INNER JOIN hwlink ON hwlink.hwInfo_hwinf_id=hwInfo.hwinf_id INNER JOIN device ON device.dev_id=hwlink.device_dev_id ";
	sString += "WHERE dev_id=" + nodeName;
	ret = dieDB->query(sString.c_str());
	it = ret.begin();

	propID++;
	pid = boost::lexical_cast<std::string>(propID);
	if (it < ret.end())
	{
		if (it->at(0) != "")
		{
			ausgabe += "<Prop NameU=\"Row_" + pid + "\" ID=\"" + pid + "\"><Value Unit=\"STR\">" + it->at(0) + "</Value><Prompt F=\"No Formula\" />";
			ausgabe += "<Label>Device Type</Label><Format>@</Format><SortKey F=\"No Formula\" /><Type>0</Type><Invisible F=\"No Formula\">0</Invisible>";
			ausgabe += "<Verify F=\"No Formula\">0</Verify><LangID>3079</LangID><Calendar F=\"No Formula\">0</Calendar></Prop>";
		}
	}

	// S/N
	sString = "SELECT hwInfo.sn FROM hwInfo INNER JOIN hwlink ON hwlink.hwInfo_hwinf_id=hwInfo.hwinf_id INNER JOIN device ON device.dev_id=hwlink.device_dev_id ";
	sString += "WHERE dev_id=" + nodeName;
	ret = dieDB->query(sString.c_str());
	it = ret.begin();

	propID++;
	pid = boost::lexical_cast<std::string>(propID);
	if (it < ret.end())
	{
		if (it->at(0) != "")
		{
			ausgabe += "<Prop NameU=\"Row_" + pid + "\" ID=\"" + pid + "\"><Value Unit=\"STR\">" + it->at(0) + "</Value><Prompt F=\"No Formula\" />";
			ausgabe += "<Label>S/N</Label><Format>@</Format><SortKey F=\"No Formula\" /><Type>0</Type><Invisible F=\"No Formula\">0</Invisible>";
			ausgabe += "<Verify F=\"No Formula\">0</Verify><LangID>3079</LangID><Calendar F=\"No Formula\">0</Calendar></Prop>";
		}
	}

	// SW Version
	sString = "SELECT hwInfo.sw_version FROM hwInfo INNER JOIN hwlink ON hwlink.hwInfo_hwinf_id=hwInfo.hwinf_id INNER JOIN device ON device.dev_id=hwlink.device_dev_id ";
	sString += "WHERE dev_id=" + nodeName;
	ret = dieDB->query(sString.c_str());
	it = ret.begin();

	propID++;
	pid = boost::lexical_cast<std::string>(propID);
	if (it < ret.end())
	{
		if (it->at(0) != "")
		{
			std::string swver = it->at(0);
			while (swver.find("&") != swver.npos)
			{
				swver.replace(swver.find("&"), 1, "+");
			}

			ausgabe += "<Prop NameU=\"Row_" + pid + "\" ID=\"" + pid + "\"><Value Unit=\"STR\">" + swver + "</Value><Prompt F=\"No Formula\" />";
			ausgabe += "<Label>SW Version</Label><Format>@</Format><SortKey F=\"No Formula\" /><Type>0</Type><Invisible F=\"No Formula\">0</Invisible>";
			ausgabe += "<Verify F=\"No Formula\">0</Verify><LangID>3079</LangID><Calendar F=\"No Formula\">0</Calendar></Prop>";
		}
	}


	// VSS Infos
	// VSL
	std::string vsl = "";
	bool vslInfos = false;
	sString = "SELECT intfName FROM interfaces INNER JOIN devInterface ON devInterface.interfaces_int_id=interfaces.intf_id ";
	sString += "INNER JOIN device ON device.dev_id=devInterface.device_dev_id WHERE phl=2 AND dev_id=" + nodeName;
	ret = dieDB->query(sString.c_str());
	for(std::vector<std::vector<std::string>>::iterator it = ret.begin(); it < ret.end(); ++it)
	{
		vsl += it->at(0) + ", ";
		vslInfos = true;
	}
	
	if (vslInfos)
	{
		propID++;
		pid = boost::lexical_cast<std::string>(propID);
		ausgabe += "<Prop NameU=\"Row_" + pid + "\" ID=\"" + pid + "\"><Value Unit=\"STR\">" + vsl + "</Value><Prompt F=\"No Formula\" />";
		ausgabe += "<Label>VSL Interfaces</Label><Format>@</Format><SortKey F=\"No Formula\" /><Type>0</Type><Invisible F=\"No Formula\">0</Invisible>";
		ausgabe += "<Verify F=\"No Formula\">0</Verify><LangID>3079</LangID><Calendar F=\"No Formula\">0</Calendar></Prop>";
	}

	
	// Switch -> STP Infos
	sString = "SELECT stpBridgeID,stpProtocol FROM device WHERE dev_id=" + nodeName;
	ret = dieDB->query(sString.c_str());
	it = ret.begin();
	
	propID++;
	pid = boost::lexical_cast<std::string>(propID);
	if (it < ret.end())
	{
		if (it->at(1) != "")
		{
			ausgabe += "<Prop NameU=\"Row_" + pid + "\" ID=\"" + pid + "\"><Value Unit=\"STR\">" + it->at(0) + "</Value><Prompt F=\"No Formula\" />";
			ausgabe += "<Label>Bridge ID</Label><Format>@</Format><SortKey F=\"No Formula\" /><Type>0</Type><Invisible F=\"No Formula\">0</Invisible>";
			ausgabe += "<Verify F=\"No Formula\">0</Verify><LangID>3079</LangID><Calendar F=\"No Formula\">0</Calendar></Prop>";

			propID++;
			pid = boost::lexical_cast<std::string>(propID);
			ausgabe += "<Prop NameU=\"Row_" + pid + "\" ID=\"" + pid + "\"><Value Unit=\"STR\">" + it->at(1) + "</Value><Prompt F=\"No Formula\" />";
			ausgabe += "<Label>STP Protocol</Label><Format>@</Format><SortKey F=\"No Formula\" /><Type>0</Type><Invisible F=\"No Formula\">0</Invisible>";
			ausgabe += "<Verify F=\"No Formula\">0</Verify><LangID>3079</LangID><Calendar F=\"No Formula\">0</Calendar></Prop>";
		}
	}

	// Config Register
	sString = "SELECT confReg FROM device WHERE dev_id=" + nodeName;
	ret = dieDB->query(sString.c_str());
	it = ret.begin();

	propID++;
	pid = boost::lexical_cast<std::string>(propID);
	if (it < ret.end())
	{
		if (it->at(0) != "")
		{
			ausgabe += "<Prop NameU=\"Row_" + pid + "\" ID=\"" + pid + "\"><Value Unit=\"STR\">" + it->at(0) + "</Value><Prompt F=\"No Formula\" />";
			ausgabe += "<Label>Configuration Register</Label><Format>@</Format><SortKey F=\"No Formula\" /><Type>0</Type><Invisible F=\"No Formula\">0</Invisible>";
			ausgabe += "<Verify F=\"No Formula\">0</Verify><LangID>3079</LangID><Calendar F=\"No Formula\">0</Calendar></Prop>";
		}
	}

	// Switch -> Anzeigen für welche VLANs der Switch Root ist
	bool rootBridge = false;
	std::string rootVlans = "";
	sString = "SELECT DISTINCT vlan FROM vlan ";
	sString += "INNER JOIN vlan_stpInstanz ON vlan_stpInstanz.vlan_vlan_id=vlan.vlan_id ";
	sString += "INNER JOIN stpInstanz ON stpInstanz.stp_id=vlan_stpInstanz.stpInstanz_stp_id ";
	sString += "INNER JOIN vlan_has_device ON vlan_has_device.vlan_vlan_id=vlan.vlan_id ";
	sString += "INNER JOIN device ON device.dev_id=vlan_has_device.device_dev_id ";
	sString += "WHERE rootPort LIKE '' ";
	sString += "AND dev_id=" + nodeName;

	ret = dieDB->query(sString.c_str());
	for(std::vector<std::vector<std::string>>::iterator it = ret.begin(); it < ret.end(); ++it)
	{
		if (it->at(0) != "")
		{
			if (it->at(0).length() == 4)			
			{
				// MST Operation
				if (it->at(0) == "9999")		// MST IST Root
				{
					rootVlans += "MST IST, ";
				}
				else if (it->at(0)[0] == '9')
				{
					std::string mstid = "MST" + it->at(0).substr(1, 3);
					rootVlans += mstid + ", ";;
				}
				else
				{
					rootVlans += it->at(0) + ", ";
				}
			}
			else
			{
				rootVlans += it->at(0) + ", ";
			}
			rootBridge = true;
		}
	}

	if (rootBridge)
	{
		propID++;
		pid = boost::lexical_cast<std::string>(propID);
		ausgabe += "<Prop NameU=\"Row_" + pid + "\" ID=\"" + pid + "\"><Value Unit=\"STR\">" + rootVlans + "</Value><Prompt F=\"No Formula\" />";
		ausgabe += "<Label>Root Bridge for Vlans</Label><Format>@</Format><SortKey F=\"No Formula\" /><Type>0</Type><Invisible F=\"No Formula\">0</Invisible>";
		ausgabe += "<Verify F=\"No Formula\">0</Verify><LangID>3079</LangID><Calendar F=\"No Formula\">0</Calendar></Prop>";
	}


	// IP Adressen vom Gerät auslesen
	sString = "SELECT intfName,ipAddress,subnetMask,nameif FROM interfaces ";
	sString += "INNER JOIN devInterface ON interfaces.intf_id=devInterface.interfaces_int_id ";
	sString += "INNER JOIN device ON device.dev_id=devInterface.device_dev_id ";
	sString += "WHERE dev_id=" + nodeName;
//	sString += "AND l2l3 LIKE 'L3' ";
	sString += " AND intf_id > (SELECT MAX(intf_id) FROM rControl WHERE intf_id < (SELECT MAX(intf_id) FROM rControl));";
	ret = dieDB->query(sString.c_str());
	for(std::vector<std::vector<std::string>>::iterator it = ret.begin(); it < ret.end(); ++it)
	{
		propID++;
		pid = boost::lexical_cast<std::string>(propID);
		std::string intName = it->at(0);
		std::string fwIntfName = it->at(3);
		if (fwIntfName != "")
		{
			intName = fwIntfName + " | " + intName;
		}
		if (cdp)
		{
			intName = "Mgmt IP (CDP)";
		}

		// vrf Name auslesen, falls vorhanden
		sString = "SELECT DISTINCT vrfName FROM vrf INNER JOIN vrfLink ON vrfLink.vrf_vrf_id=vrf.vrf_id ";
		sString += "INNER JOIN interfaces ON interfaces.intf_id=vrfLink.interfaces_intf_id ";
		sString += "WHERE interfaces.intfName LIKE '" + intName + "' ";
		sString += "AND intf_id > (SELECT MAX(intf_id) FROM rControl WHERE intf_id < (SELECT MAX(intf_id) FROM rControl));";
		std::vector<std::vector<std::string>> ret1 = dieDB->query(sString.c_str());
		if (!ret1.empty())
		{
			intName += " | vrf " + ret1[0][0];
		}

		if (it->at(1) != "")
		{
			ausgabe += "<Prop NameU=\"Row_" + pid + "\" ID=\"" + pid + "\"><Value Unit=\"STR\">" + it->at(1) + "/" + it->at(2) + "</Value><Prompt F=\"No Formula\" />";
			ausgabe += "<Label>" + intName + "</Label><Format>@</Format><SortKey F=\"No Formula\" /><Type>0</Type><Invisible F=\"No Formula\">0</Invisible>";
			ausgabe += "<Verify F=\"No Formula\">0</Verify><LangID>3079</LangID><Calendar F=\"No Formula\">0</Calendar></Prop>";
		}
	}

	// Routing Protokoll Infos
	sString = "SELECT DISTINCT rp FROM rpNeighbor INNER JOIN rpNeighborLink ON rpNeighborLink.rpNeighbor_rpNeighbor_id=rpNeighbor.rpNeighbor_id ";
	sString += "INNER JOIN interfaces ON interfaces.intf_id=rpNeighborlink.interfaces_intf_id INNER JOIN devInterface ON devInterface.interfaces_int_id=interfaces.intf_id ";
	sString += "INNER JOIN device ON device.dev_id=devInterface.device_dev_id WHERE dev_id=" + nodeName;
	
	ret = dieDB->query(sString.c_str());
	
	std::string l3rps = "";
	bool l3rpInfos = false;
	for (std::vector<std::vector<std::string>>::iterator it = ret.begin(); it < ret.end(); ++it)
	{
		l3rps += it->at(0) + ", ";
		l3rpInfos = true;
	}

	pid = boost::lexical_cast<std::string>(propID);
	if (l3rpInfos)
	{
		propID++;
		ausgabe += "<Prop NameU=\"Row_" + pid + "\" ID=\"" + pid + "\"><Value Unit=\"STR\">" + l3rps + "</Value><Prompt F=\"No Formula\" />";
		ausgabe += "<Label>Active Routing protocols</Label><Format>@</Format><SortKey F=\"No Formula\" /><Type>0</Type><Invisible F=\"No Formula\">0</Invisible>";
		ausgabe += "<Verify F=\"No Formula\">0</Verify><LangID>3079</LangID><Calendar F=\"No Formula\">0</Calendar></Prop>";
	}


	sString = "SELECT bgpAS FROM device WHERE dev_id=" + nodeName;
	ret = dieDB->query(sString.c_str());
	it = ret.begin();

	propID++;
	pid = boost::lexical_cast<std::string>(propID);
	if (it < ret.end())
	{
		if (it->at(0) != "")
		{
			std::string bgpAS = it->at(0);

			ausgabe += "<Prop NameU=\"Row_" + pid + "\" ID=\"" + pid + "\"><Value Unit=\"STR\">" + bgpAS + "</Value><Prompt F=\"No Formula\" />";
			ausgabe += "<Label>Local BGP AS#</Label><Format>@</Format><SortKey F=\"No Formula\" /><Type>0</Type><Invisible F=\"No Formula\">0</Invisible>";
			ausgabe += "<Verify F=\"No Formula\">0</Verify><LangID>3079</LangID><Calendar F=\"No Formula\">0</Calendar></Prop>";
		}
	}


	// Telefon Infos
	sString = "SELECT voiceVlan, cm1, cm2, dscpSig, dscpConf, dscpCall, defGW FROM ipPhoneDet ";
	sString += "INNER JOIN ipphonelink ON ipphonelink.ipPhoneDet_ipphone_id=ipPhoneDet.ipphone_id ";
	sString += "INNER JOIN device ON device.dev_id=ipphonelink.device_dev_id WHERE dev_id=" + nodeName;
	ret = dieDB->query(sString.c_str());
	for(std::vector<std::vector<std::string>>::iterator it = ret.begin(); it < ret.end(); ++it)
	{
		if (it->at(1) != "")
		{
			propID++;
			pid = boost::lexical_cast<std::string>(propID);
			ausgabe += "<Prop NameU=\"Row_" + pid + "\" ID=\"" + pid + "\"><Value Unit=\"STR\">" + it->at(1) + "</Value><Prompt F=\"No Formula\" />";
			ausgabe += "<Label>Call Manager 1</Label><Format>@</Format><SortKey F=\"No Formula\" /><Type>0</Type><Invisible F=\"No Formula\">0</Invisible>";
			ausgabe += "<Verify F=\"No Formula\">0</Verify><LangID>3079</LangID><Calendar F=\"No Formula\">0</Calendar></Prop>";
			
			propID++;
			pid = boost::lexical_cast<std::string>(propID);
			ausgabe += "<Prop NameU=\"Row_" + pid + "\" ID=\"" + pid + "\"><Value Unit=\"STR\">" + it->at(2) + "</Value><Prompt F=\"No Formula\" />";
			ausgabe += "<Label>Call Manager 2</Label><Format>@</Format><SortKey F=\"No Formula\" /><Type>0</Type><Invisible F=\"No Formula\">0</Invisible>";
			ausgabe += "<Verify F=\"No Formula\">0</Verify><LangID>3079</LangID><Calendar F=\"No Formula\">0</Calendar></Prop>";
			
			propID++;
			pid = boost::lexical_cast<std::string>(propID);
			ausgabe += "<Prop NameU=\"Row_" + pid + "\" ID=\"" + pid + "\"><Value Unit=\"STR\">" + it->at(0) + "</Value><Prompt F=\"No Formula\" />";
			ausgabe += "<Label>Voice VLAN</Label><Format>@</Format><SortKey F=\"No Formula\" /><Type>0</Type><Invisible F=\"No Formula\">0</Invisible>";
			ausgabe += "<Verify F=\"No Formula\">0</Verify><LangID>3079</LangID><Calendar F=\"No Formula\">0</Calendar></Prop>";

			propID++;
			pid = boost::lexical_cast<std::string>(propID);
			ausgabe += "<Prop NameU=\"Row_" + pid + "\" ID=\"" + pid + "\"><Value Unit=\"STR\">" + it->at(3) + "</Value><Prompt F=\"No Formula\" />";
			ausgabe += "<Label>DSCP Signalling</Label><Format>@</Format><SortKey F=\"No Formula\" /><Type>0</Type><Invisible F=\"No Formula\">0</Invisible>";
			ausgabe += "<Verify F=\"No Formula\">0</Verify><LangID>3079</LangID><Calendar F=\"No Formula\">0</Calendar></Prop>";

			propID++;
			pid = boost::lexical_cast<std::string>(propID);
			ausgabe += "<Prop NameU=\"Row_" + pid + "\" ID=\"" + pid + "\"><Value Unit=\"STR\">" + it->at(5) + "</Value><Prompt F=\"No Formula\" />";
			ausgabe += "<Label>DSCP Call</Label><Format>@</Format><SortKey F=\"No Formula\" /><Type>0</Type><Invisible F=\"No Formula\">0</Invisible>";
			ausgabe += "<Verify F=\"No Formula\">0</Verify><LangID>3079</LangID><Calendar F=\"No Formula\">0</Calendar></Prop>";
			
			propID++;
			pid = boost::lexical_cast<std::string>(propID);
			ausgabe += "<Prop NameU=\"Row_" + pid + "\" ID=\"" + pid + "\"><Value Unit=\"STR\">" + it->at(6) + "</Value><Prompt F=\"No Formula\" />";
			ausgabe += "<Label>Default Gateway</Label><Format>@</Format><SortKey F=\"No Formula\" /><Type>0</Type><Invisible F=\"No Formula\">0</Invisible>";
			ausgabe += "<Verify F=\"No Formula\">0</Verify><LangID>3079</LangID><Calendar F=\"No Formula\">0</Calendar></Prop>";
		}
	}

	// PoE Infos
	sString = "SELECT poeStatus, poeCurrent, poeRemaining, poeMax FROM device WHERE dev_id=" + nodeName;
	ret = dieDB->query(sString.c_str());
	for(std::vector<std::vector<std::string>>::iterator it = ret.begin(); it < ret.end(); ++it)
	{
		if (it->at(0) == "ON")
		{
			propID++;
			pid = boost::lexical_cast<std::string>(propID);
			ausgabe += "<Prop NameU=\"Row_" + pid + "\" ID=\"" + pid + "\"><Value Unit=\"STR\">" + it->at(1) + "</Value><Prompt F=\"No Formula\" />";
			ausgabe += "<Label>PoE Used (W)</Label><Format>@</Format><SortKey F=\"No Formula\" /><Type>0</Type><Invisible F=\"No Formula\">0</Invisible>";
			ausgabe += "<Verify F=\"No Formula\">0</Verify><LangID>3079</LangID><Calendar F=\"No Formula\">0</Calendar></Prop>";

			if (it->at(2) != "")
			{
				propID++;
				pid = boost::lexical_cast<std::string>(propID);
				ausgabe += "<Prop NameU=\"Row_" + pid + "\" ID=\"" + pid + "\"><Value Unit=\"STR\">" + it->at(2) + "</Value><Prompt F=\"No Formula\" />";
				ausgabe += "<Label>PoE Remaining (W)</Label><Format>@</Format><SortKey F=\"No Formula\" /><Type>0</Type><Invisible F=\"No Formula\">0</Invisible>";
				ausgabe += "<Verify F=\"No Formula\">0</Verify><LangID>3079</LangID><Calendar F=\"No Formula\">0</Calendar></Prop>";
			}
		}
	}

	return ausgabe;
}


std::string WkmVisioAusgabe::shapeDataCloud(std::string nodeName, std::string type)
{
	// Type: 
	// N: Routen in Shape Daten vermerken
	// D: Default Network -> Keine Routen in Shape Daten
	std::string ausgabe = "";
	int propID = 10;
	std::string pid = boost::lexical_cast<std::string>(propID);

	if (type == "N")
	{
		// Netze in der Cloud
		std::string sString = "SELECT network,subnet,protocol FROM l3routes WHERE nextHop IN(SELECT ipAddress FROM interfaces INNER JOIN devInterface ON devInterface.interfaces_int_id = interfaces.intf_id ";
		sString += "WHERE devInterface.device_dev_id IN(SELECT dI_dev_id FROM rpneighborship WHERE dI_dev_id1=" + nodeName + "))";
		std::vector<std::vector<std::string>> ret = dieDB->query(sString.c_str());
		for (std::vector<std::vector<std::string>>::iterator it = ret.begin(); it < ret.end(); ++it)
		{
			propID++;
			pid = boost::lexical_cast<std::string>(propID);
			std::string nName = it->at(0) + "" + it->at(1) + " // P: " + it->at(2);
			ausgabe += "<Prop NameU=\"Row_" + pid + "\" ID=\"" + pid + "\"><Value Unit=\"STR\">" + nName + "</Value><Prompt F=\"No Formula\" />";
			ausgabe += "<Label>Network</Label><Format>@</Format><SortKey F=\"No Formula\" /><Type>0</Type><Invisible F=\"No Formula\">0</Invisible>";
			ausgabe += "<Verify F=\"No Formula\">0</Verify><LangID>3079</LangID><Calendar F=\"No Formula\">0</Calendar></Prop>";
		}
	}
	return ausgabe;
}


std::string WkmVisioAusgabe::shapeDataIntf(std::string intfName, std::string type, std::string intfID, std::string id)
{
	std::string ausgabe = "";
	int propID = 0;
	std::string pid = boost::lexical_cast<std::string>(propID);

	ausgabe += "<Prop NameU=\"Row_" + pid + "\" ID=\"" + pid + "\"><Value Unit=\"STR\">" + intfName + "</Value><Prompt F=\"No Formula\" />";
	ausgabe += "<Label>Interface Name</Label><Format>@</Format><SortKey F=\"No Formula\" /><Type>0</Type><Invisible F=\"No Formula\">0</Invisible>";
	ausgabe += "<Verify F=\"No Formula\">0</Verify><LangID>3079</LangID><Calendar F=\"No Formula\">0</Calendar></Prop>";

	// FW Nameif
	propID++;
	pid = boost::lexical_cast<std::string>(propID);
	std::string sString = "SELECT nameif FROM interfaces WHERE intf_id=" + intfID + ";";
	std::vector<std::vector<std::string>> ret = dieDB->query(sString.c_str());
	if (!ret.empty())
	{
		if (ret[0][0] != "")
		{
			ausgabe += "<Prop NameU=\"Row_" + pid + "\" ID=\"" + pid + "\"><Value Unit=\"STR\">" + ret[0][0] + "</Value><Prompt F=\"No Formula\" />";
			ausgabe += "<Label>NameIf</Label><Format>@</Format><SortKey F=\"No Formula\" /><Type>0</Type><Invisible F=\"No Formula\">0</Invisible>";
			ausgabe += "<Verify F=\"No Formula\">0</Verify><LangID>3079</LangID><Calendar F=\"No Formula\">0</Calendar></Prop>";
		}
	}

	// Zugehöriges Gerät
	propID++;
	pid = boost::lexical_cast<std::string>(propID);
	sString = "SELECT hostname FROM device INNER JOIN devInterface ON device.dev_id = devInterface.device_dev_id WHERE devInterface.interfaces_int_id=" + intfID + ";";
	ret = dieDB->query(sString.c_str());
	if (!ret.empty())
	{
		ausgabe += "<Prop NameU=\"Row_" + pid + "\" ID=\"" + pid + "\"><Value Unit=\"STR\">" + ret[0][0] + "</Value><Prompt F=\"No Formula\" />";
		ausgabe += "<Label>Device</Label><Format>@</Format><SortKey F=\"No Formula\" /><Type>0</Type><Invisible F=\"No Formula\">0</Invisible>";
		ausgabe += "<Verify F=\"No Formula\">0</Verify><LangID>3079</LangID><Calendar F=\"No Formula\">0</Calendar></Prop>";
	}
	
	// Interface Description
	propID++;
	pid = boost::lexical_cast<std::string>(propID);
	sString = "SELECT description FROM interfaces WHERE intf_id=" + intfID + ";";
	ret = dieDB->query(sString.c_str());
	if (!ret.empty())
	{
		std::string descr = ret[0][0];
		while (descr.find("&") != descr.npos)
		{
			descr.replace(descr.find("&"), 1, "+");
		}
		ausgabe += "<Prop NameU=\"Row_" + pid + "\" ID=\"" + pid + "\"><Value Unit=\"STR\">" + descr + "</Value><Prompt F=\"No Formula\" />";
		ausgabe += "<Label>Interface Description</Label><Format>@</Format><SortKey F=\"No Formula\" /><Type>0</Type><Invisible F=\"No Formula\">0</Invisible>";
		ausgabe += "<Verify F=\"No Formula\">0</Verify><LangID>3079</LangID><Calendar F=\"No Formula\">0</Calendar></Prop>";
	}

	// IP Adresse
	propID++;
	pid = boost::lexical_cast<std::string>(propID);
	sString = "SELECT ipAddress FROM interfaces WHERE intf_id=" + intfID + ";";
	ret = dieDB->query(sString.c_str());
	if (!ret.empty())
	{
		std::string ipAddress = ret[0][0];
		if (ipAddress != "")
		{
			sString = "SELECT subnetMask FROM interfaces WHERE intf_id=" + intfID + ";";
			ret = dieDB->query(sString.c_str());
			if (!ret.empty())
			{
				ipAddress = ipAddress + "/" + ret[0][0];
			}
			ausgabe += "<Prop NameU=\"Row_" + pid + "\" ID=\"" + pid + "\"><Value Unit=\"STR\">" + ipAddress + "</Value><Prompt F=\"No Formula\" />";
			ausgabe += "<Label>IP Address</Label><Format>@</Format><SortKey F=\"No Formula\" /><Type>0</Type><Invisible F=\"No Formula\">0</Invisible>";
			ausgabe += "<Verify F=\"No Formula\">0</Verify><LangID>3079</LangID><Calendar F=\"No Formula\">0</Calendar></Prop>";
		}
	}

	// MAC Adresse
	propID++;
	pid = boost::lexical_cast<std::string>(propID);
	sString = "SELECT macAddress FROM interfaces WHERE intf_id=" + intfID + ";";
	ret = dieDB->query(sString.c_str());
	if (!ret.empty())
	{
		std::string macAddress = ret[0][0];
		if (macAddress != "")
		{
			ausgabe += "<Prop NameU=\"Row_" + pid + "\" ID=\"" + pid + "\"><Value Unit=\"STR\">" + macAddress + "</Value><Prompt F=\"No Formula\" />";
			ausgabe += "<Label>MAC Address</Label><Format>@</Format><SortKey F=\"No Formula\" /><Type>0</Type><Invisible F=\"No Formula\">0</Invisible>";
			ausgabe += "<Verify F=\"No Formula\">0</Verify><LangID>3079</LangID><Calendar F=\"No Formula\">0</Calendar></Prop>";
		}
	}

	
	// CDP Neighbor Info
	sString = "SELECT nName,nIntf,nIntfIP FROM cdp INNER JOIN clink ON clink.cdp_cdp_id=cdp.cdp_id WHERE interfaces_intf_id=" + intfID + ";";
	ret = dieDB->query(sString.c_str());
	
	for(std::vector<std::vector<std::string>>::iterator it = ret.begin(); it < ret.end(); ++it)
	{		
		propID++;
		pid = boost::lexical_cast<std::string>(propID);
		std::string nName = it->at(0) + " // " + it->at(1) + " // " + it->at(2);
		ausgabe += "<Prop NameU=\"Row_" + pid + "\" ID=\"" + pid + "\"><Value Unit=\"STR\">" + nName + "</Value><Prompt F=\"No Formula\" />";
		ausgabe += "<Label>CDP Neighbor</Label><Format>@</Format><SortKey F=\"No Formula\" /><Type>0</Type><Invisible F=\"No Formula\">0</Invisible>";
		ausgabe += "<Verify F=\"No Formula\">0</Verify><LangID>3079</LangID><Calendar F=\"No Formula\">0</Calendar></Prop>";
	}

	// VRF
	propID++;
	pid = boost::lexical_cast<std::string>(propID);
	sString = "SELECT DISTINCT vrfName FROM vrf INNER JOIN vrfLink ON vrfLink.vrf_vrf_id=vrf.vrf_id ";
	sString += "INNER JOIN interfaces ON interfaces.intf_id=vrfLink.interfaces_intf_id WHERE interfaces.intf_id=" + intfID + ";";
	ret = dieDB->query(sString.c_str());
	if (!ret.empty())
	{
		ausgabe += "<Prop NameU=\"Row_" + pid + "\" ID=\"" + pid + "\"><Value Unit=\"STR\">" + ret[0][0] + "</Value><Prompt F=\"No Formula\" />";
		ausgabe += "<Label>VRF</Label><Format>@</Format><SortKey F=\"No Formula\" /><Type>0</Type><Invisible F=\"No Formula\">0</Invisible>";
		ausgabe += "<Verify F=\"No Formula\">0</Verify><LangID>3079</LangID><Calendar F=\"No Formula\">0</Calendar></Prop>";
	}


	// Interface Speed
	sString = "SELECT speed FROM interfaces WHERE intf_id=" + intfID + ";";
	ret = dieDB->query(sString.c_str());
	if (!ret.empty())
	{
		if (ret[0][0] != "")
		{
			propID++;
			pid = boost::lexical_cast<std::string>(propID);
			ausgabe += "<Prop NameU=\"Row_" + pid + "\" ID=\"" + pid + "\"><Value Unit=\"STR\">" + ret[0][0] + "</Value><Prompt F=\"No Formula\" />";
			ausgabe += "<Label>Speed</Label><Format>@</Format><SortKey F=\"No Formula\" /><Type>0</Type><Invisible F=\"No Formula\">0</Invisible>";
			ausgabe += "<Verify F=\"No Formula\">0</Verify><LangID>3079</LangID><Calendar F=\"No Formula\">0</Calendar></Prop>";
		}
	}
	// Interface Duplex
	sString = "SELECT duplex FROM interfaces WHERE intf_id=" + intfID + ";";
	ret = dieDB->query(sString.c_str());
	if (!ret.empty())
	{
		if (ret[0][0] != "")
		{
			propID++;
			pid = boost::lexical_cast<std::string>(propID);
			ausgabe += "<Prop NameU=\"Row_" + pid + "\" ID=\"" + pid + "\"><Value Unit=\"STR\">" + ret[0][0] + "</Value><Prompt F=\"No Formula\" />";
			ausgabe += "<Label>Duplex</Label><Format>@</Format><SortKey F=\"No Formula\" /><Type>0</Type><Invisible F=\"No Formula\">0</Invisible>";
			ausgabe += "<Verify F=\"No Formula\">0</Verify><LangID>3079</LangID><Calendar F=\"No Formula\">0</Calendar></Prop>";
		}
	}

	// Interface Type
	std::string intfType = "";
	sString = "SELECT intfType FROM interfaces WHERE intf_id=" + intfID + ";";
	ret = dieDB->query(sString.c_str());
	if (!ret.empty())
	{
		intfType = ret[0][0];
	}
	
	if (intfType == "Channel")
	{
		// Channel Members
		bool channel = false;
		std::string channelmembers = "";
		sString = "SELECT intfName FROM interfaces WHERE channel_intf_id=" + intfID + " AND phl=1;";
		ret = dieDB->query(sString.c_str());
		for(std::vector<std::vector<std::string>>::iterator it = ret.begin(); it < ret.end(); ++it)
		{
			channelmembers += it->at(0) + ", ";
			channel = true;
		}

		if (channel)
		{
			propID++;
			pid = boost::lexical_cast<std::string>(propID);
			ausgabe += "<Prop NameU=\"Row_" + pid + "\" ID=\"" + pid + "\"><Value Unit=\"STR\">" + channelmembers + "</Value><Prompt F=\"No Formula\" />";
			ausgabe += "<Label>Channel Members</Label><Format>@</Format><SortKey F=\"No Formula\" /><Type>0</Type><Invisible F=\"No Formula\">0</Invisible>";
			ausgabe += "<Verify F=\"No Formula\">0</Verify><LangID>3079</LangID><Calendar F=\"No Formula\">0</Calendar></Prop>";
		}
	}

	// Routing Protokoll Infos
	sString = "SELECT DISTINCT rp FROM rpNeighbor INNER JOIN rpNeighborLink ON rpNeighborLink.rpNeighbor_rpNeighbor_id=rpNeighbor.rpNeighbor_id ";
	sString += "WHERE rpNeighborlink.interfaces_intf_id=" + intfID + ";";

	ret = dieDB->query(sString.c_str());

	std::string l3rps = "";
	bool l3rpInfos = false;
	for (std::vector<std::vector<std::string>>::iterator it = ret.begin(); it < ret.end(); ++it)
	{
		l3rps += it->at(0) + ", ";
		l3rpInfos = true;
	}

	pid = boost::lexical_cast<std::string>(propID);
	if (l3rpInfos)
	{
		propID++;
		ausgabe += "<Prop NameU=\"Row_" + pid + "\" ID=\"" + pid + "\"><Value Unit=\"STR\">" + l3rps + "</Value><Prompt F=\"No Formula\" />";
		ausgabe += "<Label>Active Routing protocols</Label><Format>@</Format><SortKey F=\"No Formula\" /><Type>0</Type><Invisible F=\"No Formula\">0</Invisible>";
		ausgabe += "<Verify F=\"No Formula\">0</Verify><LangID>3079</LangID><Calendar F=\"No Formula\">0</Calendar></Prop>";
	}

	sString = "SELECT area,nwType,cost FROM interfaces  WHERE intf_id=" + intfID + ";";
	ret = dieDB->query(sString.c_str());
	std::string area = "";
	std::string nwType = "";
	std::string cost = "";

	if (!ret.empty())
	{
		std::vector<std::vector<std::string>>::iterator it = ret.begin();
		area = it->at(0);
		nwType = it->at(1);
		cost = it->at(2);

		if (area != "")
		{
			propID++;
			pid = boost::lexical_cast<std::string>(propID);
			ausgabe += "<Prop NameU=\"Row_" + pid + "\" ID=\"" + pid + "\"><Value Unit=\"STR\">" + area + "</Value><Prompt F=\"No Formula\" />";
			ausgabe += "<Label>OSPF Area</Label><Format>@</Format><SortKey F=\"No Formula\" /><Type>0</Type><Invisible F=\"No Formula\">0</Invisible>";
			ausgabe += "<Verify F=\"No Formula\">0</Verify><LangID>3079</LangID><Calendar F=\"No Formula\">0</Calendar></Prop>";
		}
		if (nwType != "")
		{
			propID++;
			pid = boost::lexical_cast<std::string>(propID);
			ausgabe += "<Prop NameU=\"Row_" + pid + "\" ID=\"" + pid + "\"><Value Unit=\"STR\">" + nwType + "</Value><Prompt F=\"No Formula\" />";
			ausgabe += "<Label>OSPF Network Type</Label><Format>@</Format><SortKey F=\"No Formula\" /><Type>0</Type><Invisible F=\"No Formula\">0</Invisible>";
			ausgabe += "<Verify F=\"No Formula\">0</Verify><LangID>3079</LangID><Calendar F=\"No Formula\">0</Calendar></Prop>";
		}
		if (cost != "")
		{
			propID++;
			pid = boost::lexical_cast<std::string>(propID);
			ausgabe += "<Prop NameU=\"Row_" + pid + "\" ID=\"" + pid + "\"><Value Unit=\"STR\">" + cost + "</Value><Prompt F=\"No Formula\" />";
			ausgabe += "<Label>OSPF Intf Cost</Label><Format>@</Format><SortKey F=\"No Formula\" /><Type>0</Type><Invisible F=\"No Formula\">0</Invisible>";
			ausgabe += "<Verify F=\"No Formula\">0</Verify><LangID>3079</LangID><Calendar F=\"No Formula\">0</Calendar></Prop>";
		}
	}


	sString = "SELECT processID FROM ospf INNER JOIN ospfIntflink ON ospfIntflink.ospf_ospf_id=ospf.ospf_id	WHERE ospfIntflink.interfaces_intf_id=" + intfID + ";";
	ret = dieDB->query(sString.c_str());
	std::string ospfProcId = "";
	if (!ret.empty())
	{
		ospfProcId = ret[0][0];
		if (ospfProcId != "")
		{
			propID++;
			pid = boost::lexical_cast<std::string>(propID);
			ausgabe += "<Prop NameU=\"Row_" + pid + "\" ID=\"" + pid + "\"><Value Unit=\"STR\">" + ospfProcId + "</Value><Prompt F=\"No Formula\" />";
			ausgabe += "<Label>OSPF Process ID</Label><Format>@</Format><SortKey F=\"No Formula\" /><Type>0</Type><Invisible F=\"No Formula\">0</Invisible>";
			ausgabe += "<Verify F=\"No Formula\">0</Verify><LangID>3079</LangID><Calendar F=\"No Formula\">0</Calendar></Prop>";
		}
	}

	//sString = "SELECT DISTINCT rp FROM rpNeighbor INNER JOIN rpNeighborLink ON rpNeighborLink.rpNeighbor_rpNeighbor_id=rpNeighbor.rpNeighbor_id ";
	//sString += "WHERE rpNeighborlink.interfaces_intf_id=" + intfID + ";";

	//ret = dieDB->query(sString.c_str());


	
	sString = "SELECT rp,rpNeighborState,rpNeighborAS,rpNeighborUp,rpNeighborId FROM rpNeighbor INNER JOIN rpNeighborLink ON rpNeighborLink.rpNeighbor_rpNeighbor_id=rpNeighbor.rpNeighbor_id ";
	sString += "WHERE rpNeighborlink.interfaces_intf_id=" + intfID + ";";
	ret = dieDB->query(sString.c_str());
	
	for (std::vector<std::vector<std::string>>::iterator it = ret.begin(); it < ret.end(); ++it)
	{
		if (it->at(0) == "OSPF")
		{
			if (it->at(1) != "FULL")
			{
				interfaces.insert(pss(id, id));
			}
		}
		else if (it->at(0) == "BGP")
		{
			try
			{
				boost::lexical_cast<int>(it->at(1));
			}
			catch (boost::bad_lexical_cast &)
			{
				interfaces.insert(pss(id, id));
			}
		}
		propID++;
		pid = boost::lexical_cast<std::string>(propID);

		std::string label = it->at(0) + " Neighbor " + it->at(4);
		std::string value = "State: " + it->at(1) + " // Time: " + it->at(3);
		if (it->at(0) == "BGP")
		{
			value = "AS#: " + it->at(2) + " // " + value;
		}
		ausgabe += "<Prop NameU=\"Row_" + pid + "\" ID=\"" + pid + "\"><Value Unit=\"STR\">" + value + "</Value><Prompt F=\"No Formula\" />";
		ausgabe += "<Label>" + label + "</Label><Format>@</Format><SortKey F=\"No Formula\" /><Type>0</Type><Invisible F=\"No Formula\">0</Invisible>";
		ausgabe += "<Verify F=\"No Formula\">0</Verify><LangID>3079</LangID><Calendar F=\"No Formula\">0</Calendar></Prop>";
	}
	
	// Nexus Infos
	std::string owner = "";
	std::string portProfile = "";
	std::string boundTo = "";
	sString = "SELECT owner,portProfile,boundTo_id FROM interfaces WHERE intf_id=" + intfID + ";";
	ret = dieDB->query(sString.c_str());
	if (!ret.empty())
	{
		std::vector<std::vector<std::string>>::iterator it = ret.begin();
		owner = it->at(0);
		portProfile = it->at(1);
		boundTo = it->at(2);

		if (owner != "")
		{
			propID++;
			pid = boost::lexical_cast<std::string>(propID);
			ausgabe += "<Prop NameU=\"Row_" + pid + "\" ID=\"" + pid + "\"><Value Unit=\"STR\">" + owner + "</Value><Prompt F=\"No Formula\" />";
			ausgabe += "<Label>Owner</Label><Format>@</Format><SortKey F=\"No Formula\" /><Type>0</Type><Invisible F=\"No Formula\">0</Invisible>";
			ausgabe += "<Verify F=\"No Formula\">0</Verify><LangID>3079</LangID><Calendar F=\"No Formula\">0</Calendar></Prop>";
		}
		if (portProfile != "")
		{
			propID++;
			pid = boost::lexical_cast<std::string>(propID);
			ausgabe += "<Prop NameU=\"Row_" + pid + "\" ID=\"" + pid + "\"><Value Unit=\"STR\">" + portProfile + "</Value><Prompt F=\"No Formula\" />";
			ausgabe += "<Label>Port Profile</Label><Format>@</Format><SortKey F=\"No Formula\" /><Type>0</Type><Invisible F=\"No Formula\">0</Invisible>";
			ausgabe += "<Verify F=\"No Formula\">0</Verify><LangID>3079</LangID><Calendar F=\"No Formula\">0</Calendar></Prop>";
		}
		if (boundTo != "")
		{
			sString = "SELECT intfName FROM interfaces WHERE intf_id=" + boundTo + ";";
			ret = dieDB->query(sString.c_str());
			if (!ret.empty())
			{
				boundTo = ret[0][0];
				if (boundTo != "")
				{
					propID++;
					pid = boost::lexical_cast<std::string>(propID);
					ausgabe += "<Prop NameU=\"Row_" + pid + "\" ID=\"" + pid + "\"><Value Unit=\"STR\">" + boundTo + "</Value><Prompt F=\"No Formula\" />";
					ausgabe += "<Label>Bound Interface</Label><Format>@</Format><SortKey F=\"No Formula\" /><Type>0</Type><Invisible F=\"No Formula\">0</Invisible>";
					ausgabe += "<Verify F=\"No Formula\">0</Verify><LangID>3079</LangID><Calendar F=\"No Formula\">0</Calendar></Prop>";
				}
			}
		}
	}

	// vPC ID
	bool vpc = false;
	std::string vpcID = "";
	sString = "SELECT vpc_id FROM interfaces WHERE intf_id=" + intfID + ";";
	ret = dieDB->query(sString.c_str());
	for(std::vector<std::vector<std::string>>::iterator it = ret.begin(); it < ret.end(); ++it)
	{
		vpcID += it->at(0);
		if (vpcID != "")
		{
			vpc = true;
		}
	}

	if (vpc)
	{
		propID++;
		pid = boost::lexical_cast<std::string>(propID);
		ausgabe += "<Prop NameU=\"Row_" + pid + "\" ID=\"" + pid + "\"><Value Unit=\"STR\">" + vpcID + "</Value><Prompt F=\"No Formula\" />";
		ausgabe += "<Label>vPC ID</Label><Format>@</Format><SortKey F=\"No Formula\" /><Type>0</Type><Invisible F=\"No Formula\">0</Invisible>";
		ausgabe += "<Verify F=\"No Formula\">0</Verify><LangID>3079</LangID><Calendar F=\"No Formula\">0</Calendar></Prop>";
	}
	
	// Blocking für Vlans...
	bool foundBlocking = false;
	std::string blockingVlans = "";
	sString = "SELECT DISTINCT vlan FROM vlan ";
	sString += "INNER JOIN int_vlan ON int_vlan.vlan_vlan_id=vlan.vlan_id ";
	sString += "INNER JOIN stp_status ON stp_status.stp_status_id=int_vlan.stp_status_stp_status_id ";
	sString += "WHERE int_vlan.interfaces_intf_id=" + intfID;
	sString += " AND stp_status.stpIntfStatus LIKE '%locking%';";

	ret = dieDB->query(sString.c_str());
	for(std::vector<std::vector<std::string>>::iterator it = ret.begin(); it < ret.end(); ++it)
	{
		foundBlocking = true;
		if (it->at(0).length() == 4)			
		{
			// MST Operation
			if (it->at(0) == "9999")		// MST IST Root
			{
				blockingVlans += "MST IST, ";
			}
			else if (it->at(0)[0] == '9')
			{
				std::string mstid = "MST" + it->at(0).substr(1, 3);
				blockingVlans += mstid + ", ";;
			}
			else
			{
				blockingVlans += it->at(0) + ", ";
			}
		}
		else
		{
			blockingVlans += it->at(0) + ", ";
		}
	}

	if (foundBlocking)
	{
		propID++;
		pid = boost::lexical_cast<std::string>(propID);
		ausgabe += "<Prop NameU=\"Row_" + pid + "\" ID=\"" + pid + "\"><Value Unit=\"STR\">" + blockingVlans + "</Value><Prompt F=\"No Formula\" />";
		ausgabe += "<Label>Blocking for VLANs</Label><Format>@</Format><SortKey F=\"No Formula\" /><Type>0</Type><Invisible F=\"No Formula\">0</Invisible>";
		ausgabe += "<Verify F=\"No Formula\">0</Verify><LangID>3079</LangID><Calendar F=\"No Formula\">0</Calendar></Prop>";
	}

	// Forwarding für Vlans...
	bool foundForwarding = false;
	std::string forwardingVlans = "";
	sString = "SELECT DISTINCT vlan FROM vlan ";
	sString += "INNER JOIN int_vlan ON int_vlan.vlan_vlan_id=vlan.vlan_id ";
	sString += "INNER JOIN stp_status ON stp_status.stp_status_id=int_vlan.stp_status_stp_status_id ";
	sString += "WHERE int_vlan.interfaces_intf_id=" + intfID;
	sString += " AND stp_status.stpIntfStatus LIKE '%orwarding%';";

	ret = dieDB->query(sString.c_str());
	for(std::vector<std::vector<std::string>>::iterator it = ret.begin(); it < ret.end(); ++it)
	{
		foundForwarding = true;
		if (it->at(0).length() == 4)			
		{
			// MST Operation
			if (it->at(0) == "9999")		// MST IST Root
			{
				forwardingVlans += "MST IST, ";
			}
			else if (it->at(0)[0] == '9')
			{
				std::string mstid = "MST" + it->at(0).substr(1, 3);
				forwardingVlans += mstid + ", ";;
			}
			else
			{
				forwardingVlans += it->at(0) + ", ";
			}
		}
		else
		{
			forwardingVlans += it->at(0) + ", ";
		}

		
	}

	if (foundForwarding)
	{
		propID++;
		pid = boost::lexical_cast<std::string>(propID);
		ausgabe += "<Prop NameU=\"Row_" + pid + "\" ID=\"" + pid + "\"><Value Unit=\"STR\">" + forwardingVlans + "</Value><Prompt F=\"No Formula\" />";
		ausgabe += "<Label>Forwarding for VLANs</Label><Format>@</Format><SortKey F=\"No Formula\" /><Type>0</Type><Invisible F=\"No Formula\">0</Invisible>";
		ausgabe += "<Verify F=\"No Formula\">0</Verify><LangID>3079</LangID><Calendar F=\"No Formula\">0</Calendar></Prop>";
	}

	// Interface Anzeigen darstellen
	if (intfErr)
	{
		sString = "SELECT errLvl FROM interfaces WHERE intf_id=" + intfID + ";";
		ret = dieDB->query(sString.c_str());
		if (!ret.empty())
		{
			propID++;
			pid = boost::lexical_cast<std::string>(propID);
			ausgabe += "<Prop NameU=\"Row_" + pid + "\" ID=\"" + pid + "\"><Value Unit=\"STR\">" + ret[0][0] + "</Value><Prompt F=\"No Formula\" />";
			ausgabe += "<Label>Error Level</Label><Format>@</Format><SortKey F=\"No Formula\" /><Type>0</Type><Invisible F=\"No Formula\">0</Invisible>";
			ausgabe += "<Verify F=\"No Formula\">0</Verify><LangID>3079</LangID><Calendar F=\"No Formula\">0</Calendar></Prop>";
		}
	}
	if (intfLoad)
	{
		sString = "SELECT loadLvl FROM interfaces WHERE intf_id=" + intfID + ";";
		ret = dieDB->query(sString.c_str());
		if (!ret.empty())
		{
			propID++;
			pid = boost::lexical_cast<std::string>(propID);
			ausgabe += "<Prop NameU=\"Row_" + pid + "\" ID=\"" + pid + "\"><Value Unit=\"STR\">" + ret[0][0] + "</Value><Prompt F=\"No Formula\" />";
			ausgabe += "<Label>Load Level</Label><Format>@</Format><SortKey F=\"No Formula\" /><Type>0</Type><Invisible F=\"No Formula\">0</Invisible>";
			ausgabe += "<Verify F=\"No Formula\">0</Verify><LangID>3079</LangID><Calendar F=\"No Formula\">0</Calendar></Prop>";
		}
	}
	if (intfStp)
	{
		sString = "SELECT MAX (stpTransitionCount) FROM stp_status";
		sString += " INNER JOIN int_vlan ON int_vlan.stp_status_stp_status_id=stp_status.stp_status_id";
		sString += " INNER JOIN interfaces ON interfaces.intf_id=int_vlan.interfaces_intf_id";
		sString += " WHERE interfaces.intf_id=" + intfID + ";";
		ret = dieDB->query(sString.c_str());
		if (!ret.empty())
		{
			propID++;
			pid = boost::lexical_cast<std::string>(propID);
			ausgabe += "<Prop NameU=\"Row_" + pid + "\" ID=\"" + pid + "\"><Value Unit=\"STR\">" + ret[0][0] + "</Value><Prompt F=\"No Formula\" />";
			ausgabe += "<Label>STP Transition Count</Label><Format>@</Format><SortKey F=\"No Formula\" /><Type>0</Type><Invisible F=\"No Formula\">0</Invisible>";
			ausgabe += "<Verify F=\"No Formula\">0</Verify><LangID>3079</LangID><Calendar F=\"No Formula\">0</Calendar></Prop>";
		}
	}

	//if (intfDuplex)
	//{
	//	sString = "SELECT duplex FROM interfaces WHERE intf_id=" + intfID + ";";
	//	ret = dieDB->query(sString.c_str());
	//	if (!ret.empty())
	//	{
	//		propID++;
	//		pid = boost::lexical_cast<std::string>(propID);
	//		ausgabe += "<Prop NameU=\"Row_" + pid + "\" ID=\"" + pid + "\"><Value Unit=\"STR\">" + ret[0][0] + "</Value><Prompt F=\"No Formula\" />";
	//		ausgabe += "<Label>Duplex</Label><Format>@</Format><SortKey F=\"No Formula\" /><Type>0</Type><Invisible F=\"No Formula\">0</Invisible>";
	//		ausgabe += "<Verify F=\"No Formula\">0</Verify><LangID>3079</LangID><Calendar F=\"No Formula\">0</Calendar></Prop>";
	//	}
	//}

	// Interface Switchport Infos
	//////////////////////////////////////////////////////////////////////////
	// 1. l2AdminMode
	propID++;
	pid = boost::lexical_cast<std::string>(propID);
	sString = "SELECT l2AdminMode FROM interfaces WHERE intf_id=" + intfID + ";";
	ret = dieDB->query(sString.c_str());
	if (!ret.empty())
	{
		if (ret[0][0] != "")
		{
			ausgabe += "<Prop NameU=\"Row_" + pid + "\" ID=\"" + pid + "\"><Value Unit=\"STR\">" + ret[0][0] + "</Value><Prompt F=\"No Formula\" />";
			ausgabe += "<Label>L2 Admin Mode</Label><Format>@</Format><SortKey F=\"No Formula\" /><Type>0</Type><Invisible F=\"No Formula\">0</Invisible>";
			ausgabe += "<Verify F=\"No Formula\">0</Verify><LangID>3079</LangID><Calendar F=\"No Formula\">0</Calendar></Prop>";
		}
	}
	// 2. l2mode
	bool accessPort = false;
	propID++;
	pid = boost::lexical_cast<std::string>(propID);
	sString = "SELECT l2Mode FROM interfaces WHERE intf_id=" + intfID + ";";
	ret = dieDB->query(sString.c_str());
	if (!ret.empty())
	{
		if (ret[0][0] != "")
		{
			ausgabe += "<Prop NameU=\"Row_" + pid + "\" ID=\"" + pid + "\"><Value Unit=\"STR\">" + ret[0][0] + "</Value><Prompt F=\"No Formula\" />";
			ausgabe += "<Label>L2 Operational Mode</Label><Format>@</Format><SortKey F=\"No Formula\" /><Type>0</Type><Invisible F=\"No Formula\">0</Invisible>";
			ausgabe += "<Verify F=\"No Formula\">0</Verify><LangID>3079</LangID><Calendar F=\"No Formula\">0</Calendar></Prop>";

			if (ret[0][0] == "static access")
			{
				accessPort = true;
			}
		}
	}
	// 3. l2encap
	if (!accessPort)
	{
		propID++;
		pid = boost::lexical_cast<std::string>(propID);
		sString = "SELECT l2encap FROM interfaces WHERE intf_id=" + intfID + ";";
		ret = dieDB->query(sString.c_str());
		if (!ret.empty())
		{
			if (ret[0][0] != "")
			{
				ausgabe += "<Prop NameU=\"Row_" + pid + "\" ID=\"" + pid + "\"><Value Unit=\"STR\">" + ret[0][0] + "</Value><Prompt F=\"No Formula\" />";
				ausgabe += "<Label>L2 Encapsulation</Label><Format>@</Format><SortKey F=\"No Formula\" /><Type>0</Type><Invisible F=\"No Formula\">0</Invisible>";
				ausgabe += "<Verify F=\"No Formula\">0</Verify><LangID>3079</LangID><Calendar F=\"No Formula\">0</Calendar></Prop>";
			}
		}
		// 8. allowedVlan
		propID++;
		pid = boost::lexical_cast<std::string>(propID);
		sString = "SELECT allowedVlan FROM interfaces WHERE intf_id=" + intfID + ";";
		ret = dieDB->query(sString.c_str());
		if (!ret.empty())
		{
			if (ret[0][0] != "")
			{
				ausgabe += "<Prop NameU=\"Row_" + pid + "\" ID=\"" + pid + "\"><Value Unit=\"STR\">" + ret[0][0] + "</Value><Prompt F=\"No Formula\" />";
				ausgabe += "<Label>Allowed Vlan</Label><Format>@</Format><SortKey F=\"No Formula\" /><Type>0</Type><Invisible F=\"No Formula\">0</Invisible>";
				ausgabe += "<Verify F=\"No Formula\">0</Verify><LangID>3079</LangID><Calendar F=\"No Formula\">0</Calendar></Prop>";
			}
		}
	}
	// 4. dtp
	propID++;
	pid = boost::lexical_cast<std::string>(propID);
	sString = "SELECT dtp FROM interfaces WHERE intf_id=" + intfID + ";";
	ret = dieDB->query(sString.c_str());
	if (!ret.empty())
	{
		if (ret[0][0] != "")
		{
			ausgabe += "<Prop NameU=\"Row_" + pid + "\" ID=\"" + pid + "\"><Value Unit=\"STR\">" + ret[0][0] + "</Value><Prompt F=\"No Formula\" />";
			ausgabe += "<Label>DTP</Label><Format>@</Format><SortKey F=\"No Formula\" /><Type>0</Type><Invisible F=\"No Formula\">0</Invisible>";
			ausgabe += "<Verify F=\"No Formula\">0</Verify><LangID>3079</LangID><Calendar F=\"No Formula\">0</Calendar></Prop>";
		}
	}
	// 5. nativeAccess
	propID++;
	pid = boost::lexical_cast<std::string>(propID);
	sString = "SELECT nativeAccess FROM interfaces WHERE intf_id=" + intfID + ";";
	ret = dieDB->query(sString.c_str());
	if (!ret.empty())
	{
		if (ret[0][0] != "")
		{
			if (accessPort)
			{
				ausgabe += "<Prop NameU=\"Row_" + pid + "\" ID=\"" + pid + "\"><Value Unit=\"STR\">" + ret[0][0] + "</Value><Prompt F=\"No Formula\" />";
				ausgabe += "<Label>Access Vlan</Label><Format>@</Format><SortKey F=\"No Formula\" /><Type>0</Type><Invisible F=\"No Formula\">0</Invisible>";
				ausgabe += "<Verify F=\"No Formula\">0</Verify><LangID>3079</LangID><Calendar F=\"No Formula\">0</Calendar></Prop>";
			}
			else
			{
				ausgabe += "<Prop NameU=\"Row_" + pid + "\" ID=\"" + pid + "\"><Value Unit=\"STR\">" + ret[0][0] + "</Value><Prompt F=\"No Formula\" />";
				ausgabe += "<Label>Native Vlan</Label><Format>@</Format><SortKey F=\"No Formula\" /><Type>0</Type><Invisible F=\"No Formula\">0</Invisible>";
				ausgabe += "<Verify F=\"No Formula\">0</Verify><LangID>3079</LangID><Calendar F=\"No Formula\">0</Calendar></Prop>";
			}
		}
	}
	// 6. voiceVlan
	propID++;
	pid = boost::lexical_cast<std::string>(propID);
	sString = "SELECT voiceVlan FROM interfaces WHERE intf_id=" + intfID + ";";
	ret = dieDB->query(sString.c_str());
	if (!ret.empty())
	{
		if (ret[0][0] != "")
		{
			if (ret[0][0] != "none")
			{
				ausgabe += "<Prop NameU=\"Row_" + pid + "\" ID=\"" + pid + "\"><Value Unit=\"STR\">" + ret[0][0] + "</Value><Prompt F=\"No Formula\" />";
				ausgabe += "<Label>Voice Vlan</Label><Format>@</Format><SortKey F=\"No Formula\" /><Type>0</Type><Invisible F=\"No Formula\">0</Invisible>";
				ausgabe += "<Verify F=\"No Formula\">0</Verify><LangID>3079</LangID><Calendar F=\"No Formula\">0</Calendar></Prop>";
			}
		}
	}
	// 7. opPrivVlan
	propID++;
	pid = boost::lexical_cast<std::string>(propID);
	sString = "SELECT opPrivVlan FROM interfaces WHERE intf_id=" + intfID + ";";
	ret = dieDB->query(sString.c_str());
	if (!ret.empty())
	{
		if (ret[0][0] != "")
		{
			if (ret[0][0] != "none")
			{
				ausgabe += "<Prop NameU=\"Row_" + pid + "\" ID=\"" + pid + "\"><Value Unit=\"STR\">" + ret[0][0] + "</Value><Prompt F=\"No Formula\" />";
				ausgabe += "<Label>Operational Private Vlan</Label><Format>@</Format><SortKey F=\"No Formula\" /><Type>0</Type><Invisible F=\"No Formula\">0</Invisible>";
				ausgabe += "<Verify F=\"No Formula\">0</Verify><LangID>3079</LangID><Calendar F=\"No Formula\">0</Calendar></Prop>";
			}
		}
	}

	// Interface PoE Infos
	std::string poeStatus = "";
	std::string poeWatt = "";
	std::string poeWattmax = "";
	std::string poeDevice = "";
	sString = "SELECT poeStatus,poeWatt,poeWattmax,poeDevice FROM interfaces WHERE intf_id=" + intfID + ";";
	ret = dieDB->query(sString.c_str());
	if (!ret.empty())
	{
		std::vector<std::vector<std::string>>::iterator it = ret.begin();
		poeStatus = it->at(0);
		poeWatt = it->at(1);
		poeWattmax = it->at(2);
		poeDevice = it->at(3);

		if (poeStatus != "")
		{
			propID++;
			pid = boost::lexical_cast<std::string>(propID);
			ausgabe += "<Prop NameU=\"Row_" + pid + "\" ID=\"" + pid + "\"><Value Unit=\"STR\">" + poeStatus + "</Value><Prompt F=\"No Formula\" />";
			ausgabe += "<Label>PoE Status</Label><Format>@</Format><SortKey F=\"No Formula\" /><Type>0</Type><Invisible F=\"No Formula\">0</Invisible>";
			ausgabe += "<Verify F=\"No Formula\">0</Verify><LangID>3079</LangID><Calendar F=\"No Formula\">0</Calendar></Prop>";
		}
		if (poeWatt != "")
		{
			propID++;
			pid = boost::lexical_cast<std::string>(propID);
			ausgabe += "<Prop NameU=\"Row_" + pid + "\" ID=\"" + pid + "\"><Value Unit=\"STR\">" + poeWatt + "</Value><Prompt F=\"No Formula\" />";
			ausgabe += "<Label>PoE Watt</Label><Format>@</Format><SortKey F=\"No Formula\" /><Type>0</Type><Invisible F=\"No Formula\">0</Invisible>";
			ausgabe += "<Verify F=\"No Formula\">0</Verify><LangID>3079</LangID><Calendar F=\"No Formula\">0</Calendar></Prop>";
		}
		if (poeWattmax != "")
		{
			propID++;
			pid = boost::lexical_cast<std::string>(propID);
			ausgabe += "<Prop NameU=\"Row_" + pid + "\" ID=\"" + pid + "\"><Value Unit=\"STR\">" + poeWattmax + "</Value><Prompt F=\"No Formula\" />";
			ausgabe += "<Label>PoE Watt Max.</Label><Format>@</Format><SortKey F=\"No Formula\" /><Type>0</Type><Invisible F=\"No Formula\">0</Invisible>";
			ausgabe += "<Verify F=\"No Formula\">0</Verify><LangID>3079</LangID><Calendar F=\"No Formula\">0</Calendar></Prop>";
		}
		if (poeDevice != "")
		{
			propID++;
			pid = boost::lexical_cast<std::string>(propID);
			ausgabe += "<Prop NameU=\"Row_" + pid + "\" ID=\"" + pid + "\"><Value Unit=\"STR\">" + poeDevice + "</Value><Prompt F=\"No Formula\" />";
			ausgabe += "<Label>PoE Device</Label><Format>@</Format><SortKey F=\"No Formula\" /><Type>0</Type><Invisible F=\"No Formula\">0</Invisible>";
			ausgabe += "<Verify F=\"No Formula\">0</Verify><LangID>3079</LangID><Calendar F=\"No Formula\">0</Calendar></Prop>";
		}
	}

	// Crypto Infos
	// Interface in die Map eintragen
	if (intfName.find("C-MAP") != intfName.npos)
	{
		cryptoInterfaces.insert(pss(id, id));
	}

	sString = "SELECT intfName,cryptoMapTag,cryptoLocalIP FROM interfaces WHERE intf_id IN(SELECT channel_intf_id FROM interfaces WHERE intf_id=" + intfID + ");";
	ret = dieDB->query(sString.c_str());
	for (std::vector<std::vector<std::string>>::iterator it = ret.begin(); it < ret.end(); ++it)
	{
		if (it->at(1) != "")
		{
			propID++;
			pid = boost::lexical_cast<std::string>(propID);
			ausgabe += "<Prop NameU=\"Row_" + pid + "\" ID=\"" + pid + "\"><Value Unit=\"STR\">" + it->at(0) + "</Value><Prompt F=\"No Formula\" />";
			ausgabe += "<Label>Bound to Interface</Label><Format>@</Format><SortKey F=\"No Formula\" /><Type>0</Type><Invisible F=\"No Formula\">0</Invisible>";
			ausgabe += "<Verify F=\"No Formula\">0</Verify><LangID>3079</LangID><Calendar F=\"No Formula\">0</Calendar></Prop>";

			propID++;
			pid = boost::lexical_cast<std::string>(propID);
			ausgabe += "<Prop NameU=\"Row_" + pid + "\" ID=\"" + pid + "\"><Value Unit=\"STR\">" + it->at(1) + "</Value><Prompt F=\"No Formula\" />";
			ausgabe += "<Label>Crypto Map Tag</Label><Format>@</Format><SortKey F=\"No Formula\" /><Type>0</Type><Invisible F=\"No Formula\">0</Invisible>";
			ausgabe += "<Verify F=\"No Formula\">0</Verify><LangID>3079</LangID><Calendar F=\"No Formula\">0</Calendar></Prop>";

			propID++;
			pid = boost::lexical_cast<std::string>(propID);
			ausgabe += "<Prop NameU=\"Row_" + pid + "\" ID=\"" + pid + "\"><Value Unit=\"STR\">" + it->at(2) + "</Value><Prompt F=\"No Formula\" />";
			ausgabe += "<Label>Local Crypto IP</Label><Format>@</Format><SortKey F=\"No Formula\" /><Type>0</Type><Invisible F=\"No Formula\">0</Invisible>";
			ausgabe += "<Verify F=\"No Formula\">0</Verify><LangID>3079</LangID><Calendar F=\"No Formula\">0</Calendar></Prop>";

			sString = "SELECT peer,status,seqnr,local_net,remote_net,vrf,pfs_grp,pkt_encaps,pkt_decaps,pkt_encrypt,pkt_decrypt,send_err,rcv_err,path_mtu,esp_i,ah_i,esp_o,ah_o,pcp_i,pcp_o,settings ";
			sString += "FROM crypto INNER JOIN cryptolink ON cryptolink.crypto_crypto_id = crypto.crypto_id WHERE cryptolink.interfaces_intf_id=" + intfID + ";";
			std::vector<std::vector<std::string>> ret1 = dieDB->query(sString.c_str());
			for (std::vector<std::vector<std::string>>::iterator it1 = ret1.begin(); it1 < ret1.end(); ++it1)
			{
				std::string wert[] = { it1->at(0), it1->at(1), it1->at(2), it1->at(3), it1->at(4), it1->at(5), it1->at(6), it1->at(7), it1->at(8), it1->at(9), it1->at(10), it1->at(11), it1->at(12), it1->at(13), it1->at(14), it1->at(15), it1->at(16), it1->at(17), it1->at(18), it1->at(19), it1->at(20) };
				std::string beschreibung[] = { "PEER IP", "Status", "Crypto Map Seq-Nr", "Local Network", "Remote Network", "VRF", "PFS Group", "#Packets Encaps", "#Packets Decaps", "#Packets Encrypted", "#Packets Decrypted", "#Send Errors", "#Receive Errors", "Path MTU", "Inbound ESP SA", "Inbound AH SA", "Outbound ESP SA", "Outbound AH SA", "Inbound PCP SA", "Outbound PCP SA", "Settings" };
				std::size_t anzahl = 21;

				for (int i = 0; i < anzahl; i++)
				{
					if (wert[i] != "")
					{
						propID++;
						pid = boost::lexical_cast<std::string>(propID);
						ausgabe += "<Prop NameU=\"Row_" + pid + "\" ID=\"" + pid + "\"><Value Unit=\"STR\">" + wert[i] + "</Value><Prompt F=\"No Formula\" />";
						ausgabe += "<Label>" + beschreibung[i] + "</Label><Format>@</Format><SortKey F=\"No Formula\" /><Type>0</Type><Invisible F=\"No Formula\">0</Invisible>";
						ausgabe += "<Verify F=\"No Formula\">0</Verify><LangID>3079</LangID><Calendar F=\"No Formula\">0</Calendar></Prop>";
					}
				}
			}
		}
	}
	return ausgabe;
}


#endif // WKTOOLS_MAPPER
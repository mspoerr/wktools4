#include "class_wkmAusgabe.h"
#ifdef WKTOOLS_MAPPER

#include <ogdf/layered/OptimalHierarchyClusterLayout.h>
#include <ogdf/layered/SugiyamaLayout.h>
#include <ogdf/layered/OptimalRanking.h>
#include <ogdf/layered/MedianHeuristic.h>
#include <ogdf/energybased/FMMMLayout.h>

#include <ogdf/planarity/PlanarizationLayout.h>
#include <ogdf/planarity/VariableEmbeddingInserter.h>
#include <ogdf/planarity/FastPlanarSubgraph.h>
#include <ogdf/orthogonal/OrthoLayout.h>
#include <ogdf/planarity/EmbedderMinDepthMaxFaceLayers.h>
#include <ogdf/cluster/ClusterOrthoLayout.h>
#include <ogdf/cluster/ClusterPlanarizationLayout.h>

#include <ogdf/basic/Graph.h>
#include <ogdf/basic/GraphAttributes.h>
#include <ogdf/fileformats/GraphIO.h>



#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <map>

#include <boost/lexical_cast.hpp>
#include <boost/format.hpp>


WkmAusgabe::WkmAusgabe(WkmDB *db, WkLog *logA, std::string asgbe)
{
	dieDB = db;
	logAusgabe = logA;
	ausgabe = asgbe;
	edges = "";
	nodes = "";
	clusters = "\nrootcluster [";
	cdp = true;
	cdp_hosts = true;
	unknownSTP = true;
	unknownRP = true;
#ifdef _WINDOWS_
	ausgabeVz = ausgabe.substr(0, ausgabe.rfind("\\")+1);
#else
	ausgabeVz = ausgabe.substr(0, ausgabe.rfind("/")+1);
#endif

}



WkmAusgabe::~WkmAusgabe()
{

}


int WkmAusgabe::doIt()
{
	vdxA = new WkmVisioAusgabe(ausgabe, dieDB);
	int ret = 1;
	markiereEintraege();

	if (l2)
	{
		ret = doItL2(false);
	}

	if (l3)
	{
		//hier = false;
		ret = doItL3();
	}

	if (l3routing)
	{
		ret = doItL3Routing();
	}
	
	delete vdxA;

	// Report ausgeben
	schreibeLog(WkLog::WkLog_NORMALTYPE, "\n\nREPORT:\n=======\n", "", WkLog::WkLog_ROT, WkLog::WkLog_FETT);
	report();
	return ret;
}


int WkmAusgabe::doItL2(bool perVLAN)
{
	neighborEmpty = true;

	// VLANs auslesen wenn perVLAN true
	std::string sString = "";
	std::vector<std::vector<std::string>> vlans;
	if (perVLAN)
	{
		sString = "SELECT DISTINCT vlan FROM vlan WHERE vlan_id > (SELECT MAX(vlan_id) FROM rControl WHERE vlan_id < (SELECT MAX(vlan_id) FROM rControl))";
		vlans = dieDB->query(sString.c_str());
	}
	else
	{
		std::vector<std::string> dummy;
		dummy.push_back("");
		vlans.push_back(dummy);
	}

	for (std::vector<std::string> vlanID : vlans)
	{
		nodes = "";
		edges = "";
		clusters = "\nrootcluster [";

		// Hinweis zur Interface Farbkodierung
		// Interface Farbe:
		// 000000
		// 1|||||	...Interface: 0: Normal; 1: Muss kontrolliert werden; 2: Unknown Neigbor; 3: STP Blocking; 4: STP Mixed; 5: VPN
		//  2||||	...For Future Use
		//   3|||	...Error: 0: OK; 1: WARNING; 2: PROBLEM
		//    4||	...Load: 0: OK; 1: WARNING; 2: PROBLEM
		//     5|	...Duplex: 0: FULL; 1: HALF
		//      6	...STP Transitions: 0: OK; 1: WARNING; 2: PROBLEM

		// GML Rahmenstrings initialisieren
		std::string interfaceG_Neu_Start = "\ngraphics [\nfill \"#";																	// Normales Interface mit neuer Kodierung START
		std::string interfaceG_Neu_Ende = "\" \ntype \"rectangle\" \nw 100.0000000000 \nh 100.0000000000\n]";							// Normales Interface mit neuer Kodierung ENDE
		std::string interfaceFarbe = "000000";																							// Farbcode START

		std::string nodeStartG = "node [\nid ";
		std::string nodeLabelAG = "\nlabel \"";
		std::string nodeLabelEG = "\"";
		std::string nodeEndeG = "\n]\n";
		std::string edgeSourceG = "edge [\nsource ";
		std::string edgeTargetG = "\ntarget ";
		std::string edgeEndeG = "\n]\n";
		std::string clusterStart = "\ncluster [\nid ";
		std::string clusterVertexStart = "\nvertex \"";
		std::string clusterVertexEnd = "\"";
		std::string clusterEnd = "\n]";

		int nId = 0;		// Node ID
		int edgeId = 0;		// Edge ID
		int clusterId = 0;	// Cluster-ID
		int tClusterId = 10000;
		std::vector<std::string> clusterStrings;		// Cluster Strings, die dann zum Schluss zusammengefasst werden
	
		std::map<std::string,int> nIdsCluster;			// Sammlung aller Node IDs um mehrfache Einträge als Node zu verhindern und die Clusterobjekte richtig zuzuordnen
		std::map<std::string,int>::iterator nItCluster;	// Iterator für die Node/Cluster Sammlung
		typedef std::pair <std::string,int> psi;		// Pair für das Einfügen in die Map
	
		std::map<std::string,std::string> nIds;				// Sammlung aller Node IDs um mehrfache Einträge als Node zu verhindern
		std::map<std::string,std::string>::iterator nIt;	// Iterator für die Node Sammlung
		typedef std::pair <std::string,std::string> pss;	// Pair für das Einfügen in die Map
	
		std::multimap <std::string, std::string> IntfData;	// Sammlung aller Interfaces mit Fehlern und Warnungen
		std::multimap <std::string, std::string>::iterator IntfDataIter, IntfDataAnfang, IntfDataEnde;
	
		std::string nId1 = "";	// Node ID 1 -> für die Edge Beschreibung
		std::string nId2 = "";	// Node ID 2 -> für die Edge Beschreibung
		std::string nId3 = "";	// Node ID 3 -> für die Edge Beschreibung
		std::string nId4 = "";	// Node ID 4 -> für die Edge Beschreibung
		std::string nIDString = "";		// Temp Node ID als String

		std::string tempClusterId = "";	// Cluster ID für den Interface/Error Cluster

		bool blocking1 = false;		// Intf 1: STP Blocking?
		bool stpMix1 = false;		// Intf 1: Mehrere STP Stati auf einem Interface
		bool blocking2 = false;		// Intf 2: STP Blocking?
		bool stpMix2 = false;		// Intf 2: Mehrere STP Stati auf einem Interface
		bool stpRoot1 = false;		// Intf 1: Root Port
		bool stpRoot2 = false;		// Intf 2: Root Port


		// neighborship Tabelle auslesen
		if (perVLAN)
		{
			sString = "SELECT dI_dev_id,dI_intf_id,dI_dev_id1,dI_intf_id1,flag FROM neighborship WHERE n_id > (SELECT MAX(n_id) FROM rControl WHERE n_id < (SELECT MAX(n_id) FROM rControl)) ";
			sString += "AND n_id IN(SELECT n_id FROM neighborship WHERE dI_dev_id IN(SELECT dev_id FROM device INNER JOIN vlan_has_device ON vlan_has_device.device_dev_id = device.dev_id ";
			sString += "INNER JOIN vlan ON vlan.vlan_id = vlan_has_device.vlan_vlan_id WHERE vlan.vlan = " + vlanID[0] + ") AND dI_dev_id1 IN(SELECT dev_id FROM device ";
			sString += "INNER JOIN vlan_has_device ON vlan_has_device.device_dev_id = device.dev_id	INNER JOIN vlan ON vlan.vlan_id = vlan_has_device.vlan_vlan_id ";
			sString += "WHERE vlan.vlan = " + vlanID[0] + "))";
		}
		else
		{
			sString = "SELECT dI_dev_id,dI_intf_id,dI_dev_id1,dI_intf_id1,flag FROM neighborship ";
			sString += "WHERE n_id > (SELECT MAX(n_id) FROM rControl WHERE n_id < (SELECT MAX(n_id) FROM rControl));";
		}
		std::vector<std::vector<std::string>> devs = dieDB->query(sString.c_str());
		for(std::vector<std::vector<std::string>>::iterator it = devs.begin(); it < devs.end(); ++it)
		{
			bool intf1Existiert = false;
			bool intf2Existiert = false;
			// Falls der Eintrag nicht ausgegeben werden soll
			if (it->at(4) == "99")
			{
				continue;
			}

			// Interface Farbe zurücksetzen
			interfaceFarbe = "000000";

			// STP Interface Infos zurücksetzen
			stpMix1 = stpMix2 = blocking1 = blocking2 = false;

			sString = "SELECT device.hostname,device.type FROM device WHERE dev_id=" + it->at(0) + ";";
			std::vector<std::vector<std::string>> ret = dieDB->query(sString.c_str());
			std::vector<std::vector<std::string>>::iterator ti = ret.begin();
			if (ti == ret.end())
			{
				break;
			}
			neighborEmpty = false;

			std::string hh1 = ret[0][0];
			std::string type1 = ret[0][1];
			std::string h1 =it->at(0);
		
			nIt = nIds.find(it->at(0));
			if (nIt == nIds.end())
			{
				nIDString = boost::lexical_cast<std::string>(nId);
				nIdsCluster.insert(psi(it->at(0), clusterId));
				nIds.insert(pss(it->at(0), nIDString));
				nodes += nodeStartG + nIDString + nodeLabelAG + h1 + nodeLabelEG + setDevType(type1) + nodeEndeG;
				nId1 = nIDString;
				nId++;
				clusterStrings.push_back(clusterStart + boost::lexical_cast<std::string>(clusterId));
				clusterStrings[clusterId] += clusterVertexStart + nId1 + clusterVertexEnd;
				clusterId++;
			}
			else
			{
				nId1 = nIt->second;
			}

			std::string i1 = "0";
			if (it->at(1) != "0")
			{
				sString = "SELECT intfName, duplex, errLvl, loadLvl FROM interfaces WHERE interfaces.intf_id=" + it->at(1) + ";";
				std::vector<std::vector<std::string>> reti = dieDB->query(sString.c_str());
				i1 = it->at(1) + "-" + reti[0][0];
				nIt = nIds.find(it->at(1));
				if (nIt == nIds.end())
				{
					intf1Existiert = false;
					// STP Status vom Interface feststellen und anzeigen
					sString = "SELECT DISTINCT stpIntfStatus FROM stp_status ";
					sString += "INNER JOIN int_vlan ON int_vlan.stp_status_stp_status_id=stp_status.stp_status_id ";
					sString += "INNER JOIN interfaces ON interfaces.intf_id=int_vlan.interfaces_intf_id ";
					sString += "WHERE interfaces.intf_id=" + it->at(1) + ";";
					std::vector<std::vector<std::string>> rets = dieDB->query(sString.c_str());
					for (std::vector<std::vector<std::string>>::iterator itSTP = rets.begin(); itSTP < rets.end(); ++itSTP)
					{
						if (itSTP->at(0).find("ocking") != itSTP->at(0).npos)
						{
							blocking1 = true;
						}
						if (itSTP->at(0).find("root forw") != itSTP->at(0).npos)
						{
							stpRoot1 = true;
						}
					
					}
					if (rets.size() > 1)
					{
						if (blocking1)
						{
							stpMix1 = true;
						}
					}
					if (stpRoot1)
					{
						i1 += " (R)";
						stpRoot1 = false;
					}

					nId2 = boost::lexical_cast<std::string>(nId);
					nIds.insert(pss(it->at(1), nId2));

					nId++;

					tempClusterId = boost::lexical_cast<std::string>(tClusterId);
					tClusterId++;
				
					nItCluster = nIdsCluster.find(it->at(0));
					clusterStrings[nItCluster->second] += clusterStart + tempClusterId;
					// Cluster Strings weiterbauen
					clusterStrings[nItCluster->second] += clusterVertexStart + nId2 + clusterVertexEnd;


					if (reti[0][1].find("Half-duplex") != reti[0][1].npos)
					{
						interfaceFarbe[4] = '1';
					}
				
					int err = 0;
					try
					{
						err = boost::lexical_cast<int>(reti[0][2]);
					}
					catch (boost::bad_lexical_cast &)
					{
						err = 0;			
					}
				
					if (err > 1000)
					{
						interfaceFarbe[2] = '2';
					}
					else if (err > 10)
					{
						interfaceFarbe[2] = '1';
					}
				
				
					int load = 0;
					try
					{
						load = boost::lexical_cast<int>(reti[0][3]);
					}
					catch (boost::bad_lexical_cast &)
					{
						load = 0;			
					}

					if (load > 200)
					{
						interfaceFarbe[3] = '2';
					}
					else if (load > 100)
					{
						interfaceFarbe[3] = '1';
					}
				
					// STP Transition Count auslesen
					sString = "SELECT stpTransitionCount FROM stp_status";
					sString += " INNER JOIN int_vlan ON int_vlan.stp_status_stp_status_id=stp_status.stp_status_id";
					sString += " INNER JOIN interfaces ON interfaces.intf_id=int_vlan.interfaces_intf_id";
					sString += " WHERE interfaces.intf_id=" + it->at(1);
					sString += " AND stpTransitionCount>15;";
					
					std::vector<std::vector<std::string>> rett = dieDB->query(sString.c_str());
					if (!rett.empty())
					{
						interfaceFarbe[5] = '2';
					}


					// Inneren Cluster abschließen
					clusterStrings[nItCluster->second] += clusterEnd;
				}
				else
				{
					nId2 = nIt->second;
					intf1Existiert = true;
				}
			
			}
			else
			{
				nId2 = boost::lexical_cast<std::string>(nId);
				nId++;
			}

			std::string h2 = "0";
			std::string i2 = "0";
			std::string hh2 = "0";

			if (it->at(2) != "0")
			{
				// Vom vorigen Edge...
				if (!intf1Existiert)
				{
					if ((it->at(4) == "") || (it->at(4) == "0"))
					{
						if (stpMix1)
						{
							interfaceFarbe[0] = '4';
						}
						else if (blocking1)
						{
							interfaceFarbe[0] = '3';
						}
						else
						{
							interfaceFarbe[0] = '0';
						}
					}
					else if (it->at(4) == "1")
					{
						interfaceFarbe[0] = '1';
					}
					else
					{
						interfaceFarbe[0] = '0';
					}
					nodes += nodeStartG + nId2 + nodeLabelAG + i1 + nodeLabelEG + interfaceG_Neu_Start + interfaceFarbe + interfaceG_Neu_Ende + nodeEndeG;
				}

				interfaceFarbe = "000000";
			
			
				sString = "SELECT device.hostname,device.type,dataSource FROM device WHERE dev_id=" + it->at(2) + ";";
				ret = dieDB->query(sString.c_str());
				hh2 = ret[0][0];
				h2 = it->at(2);
				std::string type2 = ret[0][1];
				std::string source = ret[0][2];		// Data Source -> i.e. CDP
				//if (source == "1")
				//{
				//	h2 += " (CDP)";
				//}

				nIt = nIds.find(it->at(2));
				if (nIt == nIds.end())
				{
					nIDString = boost::lexical_cast<std::string>(nId);
					nIdsCluster.insert(psi(it->at(2), clusterId));
					nIds.insert(pss(it->at(2), nIDString));
					nodes += nodeStartG + nIDString + nodeLabelAG + it->at(2) + nodeLabelEG + setDevType(type2) + nodeEndeG;
					nId3 = nIDString;
					nId++;
					clusterStrings.push_back(clusterStart + boost::lexical_cast<std::string>(clusterId));
					clusterStrings[clusterId] += clusterVertexStart + nId3 + clusterVertexEnd;
				
					tempClusterId = boost::lexical_cast<std::string>(tClusterId);
					tClusterId++;

					clusterId++;
				}
				else
				{
					nId3 = nIt->second;
				}

				if (it->at(3) != "0")
				{
					sString = "SELECT intfName, duplex, errLvl, loadLvl FROM interfaces WHERE interfaces.intf_id=" + it->at(3) + ";";
					std::vector<std::vector<std::string>> reti = dieDB->query(sString.c_str());
					i2 = it->at(3) + "-" + reti[0][0];
					nIt = nIds.find(it->at(3));
					if (nIt == nIds.end())
					{
						intf2Existiert = false;
						// STP Status vom Interface feststellen und anzeigen
						sString = "SELECT DISTINCT stpIntfStatus FROM stp_status ";
						sString += "INNER JOIN int_vlan ON int_vlan.stp_status_stp_status_id=stp_status.stp_status_id ";
						sString += "INNER JOIN interfaces ON interfaces.intf_id=int_vlan.interfaces_intf_id ";
						sString += "WHERE interfaces.intf_id=" + it->at(3) + ";";
						std::vector<std::vector<std::string>> rets = dieDB->query(sString.c_str());
						for(std::vector<std::vector<std::string>>::iterator itSTP = rets.begin(); itSTP < rets.end(); ++itSTP)
						{
							if (itSTP->at(0).find("ocking") != itSTP->at(0).npos)
							{
								blocking2 = true;
							}
							if (itSTP->at(0).find("root forw") != itSTP->at(0).npos)
							{
								stpRoot2 = true;
							}
						}
						if (rets.size() > 1)
						{
							if (blocking2)
							{
								stpMix2 = true;
							}
						}

						nId4 = boost::lexical_cast<std::string>(nId);
						nIds.insert(pss(it->at(3), nId4));

						if (stpRoot2)
						{
							i2 += " (R)";
							stpRoot2 = false;
						}

						nId++;

						if (reti[0][1].find("Half-duplex") != reti[0][1].npos)
						{
							interfaceFarbe[4] = '1';
						}

						int err = 0;
						try
						{
							err = boost::lexical_cast<int>(reti[0][2]);
						}
						catch (boost::bad_lexical_cast &)
						{
							err = 0;			
						}

						if (err > 1000)
						{
							interfaceFarbe[2] = '2';
						}
						else if (err > 10)
						{
							interfaceFarbe[2] = '1';
						}
						int load = 0;
						try
						{
							load = boost::lexical_cast<int>(reti[0][3]);
						}
						catch (boost::bad_lexical_cast &)
						{
							load = 0;			
						}

						if (load > 200)
						{
							interfaceFarbe[3] = '2';
						}
						else if (load > 100)
						{
							interfaceFarbe[3] = '1';
						}
					
						if ((it->at(4) == "") || (it->at(4) == "0"))
						{
							if (stpMix2)
							{
								interfaceFarbe[0] = '4';
							}
							else if (blocking2)
							{
								interfaceFarbe[0] = '3';
							}
							else
							{
								interfaceFarbe[0] = '0';
							}
						}
						else if (it->at(4) == "1")
						{
							interfaceFarbe[0] = '1';
						}
						else
						{
							interfaceFarbe[0] = '0';
						}
					}
					else
					{
						nId4 = nIt->second;
						intf2Existiert = true;
					}
				}
				else
				{
					nId4 = boost::lexical_cast<std::string>(nId);
					nId++;
				
					if (it->at(4) == "")
					{
						interfaceFarbe[0] = '0';
					}
					else if (it->at(4) == "1")
					{
						interfaceFarbe[0] = '1';
					}
				}

				if (!intf2Existiert)
				{
					// STP Transition Count auslesen
					sString = "SELECT stpTransitionCount FROM stp_status";
					sString += " INNER JOIN int_vlan ON int_vlan.stp_status_stp_status_id=stp_status.stp_status_id";
					sString += " INNER JOIN interfaces ON interfaces.intf_id=int_vlan.interfaces_intf_id";
					sString += " WHERE interfaces.intf_id=" + it->at(3);
					sString += " AND stpTransitionCount>15;";

					std::vector<std::vector<std::string>> rett = dieDB->query(sString.c_str());
					if (!rett.empty())
					{
						interfaceFarbe[5] = '2';
					}
					nodes += nodeStartG + nId4 + nodeLabelAG + i2 + nodeLabelEG + interfaceG_Neu_Start + interfaceFarbe + interfaceG_Neu_Ende + nodeEndeG;
				}



				// Cluster Strings weiterbauen
				nItCluster = nIdsCluster.find(it->at(2));
				clusterStrings[nItCluster->second] += clusterVertexStart + nId4 + clusterVertexEnd;


				// Edges einfügen
				std::string edgeGraph1 = "";
				std::string edgeGraph2 = "";
				if (blocking1)
				{
					edgeGraph1 = "\ngraphics [\ntype \"dotted\"\n]";
				}
				if (stpMix1)
				{
					edgeGraph1 = "\ngraphics [\ntype \"dashed\"\n]";
				}
				if (blocking2)
				{
					edgeGraph2 = "\ngraphics [\ntype \"dotted\"\n]";
				}
				if (stpMix2)
				{
					edgeGraph2 = "\ngraphics [\ntype \"dashed\"\n]";
				}
			
				if (!intf1Existiert)
				{
					edges += edgeSourceG + nId1 + edgeTargetG + nId2 + edgeGraph1 + edgeEndeG;
				}
				edges += edgeSourceG + nId2 + edgeTargetG + nId4 + edgeEndeG;
				if (!intf2Existiert)
				{
					edges += edgeSourceG + nId4 + edgeTargetG + nId3 + edgeGraph2 + edgeEndeG;
				}
			}
			else
			{
				interfaceFarbe[0] = '2';
				nodes += nodeStartG + nId2 + nodeLabelAG + i1 + nodeLabelEG + interfaceG_Neu_Start + interfaceFarbe + interfaceG_Neu_Ende + nodeEndeG;
				edges += edgeSourceG + nId1 + edgeTargetG + nId2 + edgeEndeG;
			}

			std::string nachbaren = hh1 + "-" + i1 + " <--> " + hh2 + "-" + i2 + "\n";
			schreibeLog(WkLog::WkLog_NORMALTYPE, nachbaren, "", WkLog::WkLog_SCHWARZ);

		}
		// Clusterstrings abschließen und zusammenführen
		for (int i=0; i<clusterStrings.size(); i++)
		{
			clusters += clusterStrings[i] += clusterEnd;
		}
		clusters += clusterEnd;

		std::string pageName = "L2 Main";
		if (perVLAN)
		{
			pageName = "L2 VLAN " + vlanID[0];
		}

		if (!zeichneGraph(pageName))
		{
			schreibeLog(WkLog::WkLog_ZEIT, "6203: Unable to draw graph!\r\n", "6203", 
				WkLog::WkLog_ROT, WkLog::WkLog_FETT);
		}

	}


	return 1;
}


void WkmAusgabe::schreibeLog(int type, std::string logEintrag, std::string log2, int farbe, int format, int groesse)
{
	if (logAusgabe != NULL)
	{
		WkLog::evtData evtDat;
		evtDat.farbe = farbe;
		evtDat.format = format;
		evtDat.groesse = groesse;
		evtDat.logEintrag = logEintrag;
		evtDat.logEintrag2 = log2;
		evtDat.type = type;

		LogEvent evt(EVT_LOG_MELDUNG);
		evt.SetData(evtDat);
		wxPostEvent(logAusgabe, evt);
	}
}


int WkmAusgabe::zeichneGraph(std::string filename)
{
	// Prüfen, ob das GML leer ist
	if (neighborEmpty)
	{
		return false;
	}

	int type = 0;			// Default Type
	if (filename == "L3RP")
	{
		type = 1;			// L3 Routing Type
	}
	
	ogdf::Graph G;
	ogdf::GraphAttributes GA(G,
		ogdf::GraphAttributes::nodeGraphics | ogdf::GraphAttributes::edgeGraphics | ogdf::GraphAttributes::edgeType | ogdf::GraphAttributes::edgeArrow |
		ogdf::GraphAttributes::nodeLabel | ogdf::GraphAttributes::edgeStyle | 
		ogdf::GraphAttributes::nodeStyle | ogdf::GraphAttributes::nodeTemplate);

	ogdf::ClusterGraph CG(G);
	ogdf::ClusterGraphAttributes CGA(CG, 		
		ogdf::GraphAttributes::nodeGraphics | ogdf::GraphAttributes::edgeGraphics | ogdf::GraphAttributes::edgeType | ogdf::GraphAttributes::edgeArrow |
		ogdf::GraphAttributes::nodeLabel | ogdf::GraphAttributes::edgeStyle | 
		ogdf::GraphAttributes::nodeStyle | ogdf::GraphAttributes::nodeTemplate);


	std::string sg = "directed 0 graph [\n";
	sg += nodes;
	sg += edges;
	sg += "]";
	sg += clusters;
	
	std::istringstream iss;
	iss.str(sg);

#ifdef DEBUG
	schreibeLog(WkLog::WkLog_NORMALTYPE, sg, "", WkLog::WkLog_SCHWARZ);
	// Bei Problemen sleep, damit der Input GML String kopiert werden kann
	//Sleep(5000);
#endif

	if (!ogdf::GraphIO::readGML(CGA, CG, G, iss))
	{
		schreibeLog(WkLog::WkLog_ZEIT, "6202: Internal GML error!\r\n", "6202",
			WkLog::WkLog_ROT, WkLog::WkLog_FETT);
		return 0;
	}

	//if( !CGA.readClusterGML(iss,CG,G))
	//{
	//	schreibeLog(WkLog::WkLog_ZEIT, "6202: Internal GML error!\r\n", "6202", 
	//		WkLog::WkLog_ROT, WkLog::WkLog_FETT);
	//	return 0;
	//}

	// Ausgabe Streams und Filenamen
	std::ostringstream oss1 (std::ostringstream::out);
	std::ostringstream oss2 (std::ostringstream::out);
	//std::string af1 = ausgabe + "hierarchical-" + filename + ".gml";
	//std::string af2 = ausgabe + "organic-" + filename + ".gml";

	size_t pos = 0;

	std::string str1 = "";
	std::string str2 = "";
	std::string str3 = "";

	if (ortho)
	{
		schreibeLog(WkLog::WkLog_ZEIT, "6202: Orthogonal Layout disabled due to 3rd party Lib problems!\r\n", "6202",
			WkLog::WkLog_ROT, WkLog::WkLog_FETT);
	 // Orthogonal Layout
	    ogdf::ClusterPlanarizationLayout cpl;
 
	    ogdf::ClusterOrthoLayout *col = new ogdf::ClusterOrthoLayout;
	    col->separation(20.0);
	    col->cOverhang(0.4);
	    col->setOptions(2+4);
	    cpl.setPlanarLayouter(col);
	 
	   
		try
		{
			cpl.call(G, CGA, CG);
		}
		catch (...)
		{
			schreibeLog(WkLog::WkLog_ZEIT, "6202: Orthogonal Layout not possible!\r\n", "6202",
				WkLog::WkLog_ROT, WkLog::WkLog_FETT);
			return 0;
		}
	 
		ogdf::GraphIO::writeGML(CGA, oss1);
		//CGA.writeGML(oss1);
		str1 = oss1.str();

		std::string fname = filename + "-Orthogonal";
		vdxA->doIt(str1, fname, intfStyle, type);
 
	}
	
	if (hier)
	{
		//// Hierarchical Layout
		schreibeLog(WkLog::WkLog_ZEIT, "6202: Hierarchical Layout disabled due to 3rd party Lib problems!\r\n", "6202",
			WkLog::WkLog_ROT, WkLog::WkLog_FETT);
		//ogdf::SugiyamaLayout SL;
		//SL.setRanking(new ogdf::OptimalRanking);
		//SL.setCrossMin(new ogdf::MedianHeuristic);

		//ogdf::OptimalHierarchyClusterLayout *ohl = new ogdf::OptimalHierarchyClusterLayout;
		//
		//double layerDist = 0;
		//double nodeDist = 0;
		//double weightBal = 0;
		//try
		//{
		//	layerDist = boost::lexical_cast<double>(ld);
		//}
		//catch (boost::bad_lexical_cast &)
		//{
		//	layerDist = 50.0;			
		//}
		//try
		//{
		//	nodeDist = boost::lexical_cast<double>(nd);
		//}
		//catch (boost::bad_lexical_cast &)
		//{
		//	nodeDist = 50.0;			
		//}
		//try
		//{
		//	weightBal = boost::lexical_cast<double>(wb);
		//}
		//catch (boost::bad_lexical_cast &)
		//{
		//	weightBal = 0.1;			
		//}

		//
		//ohl->layerDistance(layerDist);
		//ohl->nodeDistance(nodeDist);
		//ohl->weightBalancing(weightBal);
		//SL.setClusterLayout(ohl);
		//try
		//{
		//	SL.call(CGA);
		//}
		//catch (...)
		//{
		//	schreibeLog(WkLog::WkLog_ZEIT, "6202: Hierarchical Layout not possible!\r\n", "6202",
		//		WkLog::WkLog_ROT, WkLog::WkLog_FETT);
		//	return 0;
		//}
		//
		//ogdf::GraphIO::writeGML(CGA, oss1);
		//str1 = oss1.str();

		//std::string fname = filename + "-Hierarchical";
		//vdxA->doIt(str1, fname, intfStyle, type);
	}
	
	if (org)
	{
		// Organic Layout
		CG.setUpdateDepth(true);
		ogdf::FMMMLayout fmmm;

		double unitEdgeL = 0;
		try
		{
			unitEdgeL = boost::lexical_cast<double>(uel);
		}
		catch (boost::bad_lexical_cast &)
		{
			unitEdgeL = 100.0;			
		}

		
		fmmm.useHighLevelOptions(true);
		fmmm.unitEdgeLength(unitEdgeL); 
		fmmm.qualityVersusSpeed(ogdf::FMMMLayout::qvsGorgeousAndEfficient);

		fmmm.call(CGA);
		ogdf::GraphIO::writeGML(CGA, oss2);

		str2 = oss2.str();
		pos = 0;

		std::string fname = filename + "-Organic";
		vdxA->doIt(str2, fname, intfStyle, type);

		//ofstream ausgabe2(af2.c_str(), std::ios_base::out | std::ios_base::binary);
		//ausgabe2 << str2;
		//ausgabe2.close();

	}

	return 1;
}


std::string WkmAusgabe::setDevType(std::string devT)
{
	std::string routerG = "\ngraphics [\nfill \"#90ee90\" \ntype \"oval\" \nw 400.0000000000 \nh 200.0000000000\n]";
	std::string vrouterG = "\ngraphics [\nfill \"#90ee91\" \ntype \"oval\" \nw 400.0000000000 \nh 200.0000000000\n]";
	std::string switchG = "\ngraphics [\nfill \"#66cdaa\" \ntype \"oval\" \nw 400.0000000000 \nh 200.0000000000\n]";
	std::string switchRG = "\ngraphics [\nfill \"#66cdab\" \ntype \"oval\" \nw 400.0000000000 \nh 200.0000000000\n]";
	std::string l3switchG = "\ngraphics [\nfill \"#66cdac\" \ntype \"oval\" \nw 400.0000000000 \nh 200.0000000000\n]";
	std::string l3switchRG = "\ngraphics [\nfill \"#66cdad\" \ntype \"oval\" \nw 400.0000000000 \nh 200.0000000000\n]";
	std::string apG = "\ngraphics [\nfill \"#66cddd\" \ntype \"oval\" \nw 400.0000000000 \nh 200.0000000000\n]";
	std::string firewallG = "\ngraphics [\nfill \"#87cefa\" \ntype \"oval\" \nw 400.0000000000 \nh 200.0000000000\n]";
	std::string endgeraetG = "\ngraphics [\nfill \"#ffb6c1\" \ntype \"oval\" \nw 400.0000000000 \nh 200.0000000000\n]";
	std::string unknownG = "\ngraphics [\nfill \"#a9a9a8\" \ntype \"oval\" \nw 400.0000000000 \nh 200.0000000000\n]";
	std::string phoneG = "\ngraphics [\nfill \"#a9a9a9\" \nw 200.0000000000 \nh 200.0000000000\n]";
	std::string ataG = "\ngraphics [\nfill \"#a9a9a6\" \nw 200.0000000000 \nh 200.0000000000\n]";
	std::string cameraG = "\ngraphics [\nfill \"#a9a9a5\" \nw 200.0000000000 \nh 200.0000000000\n]";
	std::string netzG = "\ngraphics [\nfill \"#a9a9a7\" \nw 700.0000000000 \nh 100.0000000000\n]";
	std::string cloudgG = "\ngraphics [\nfill \"#a9a9a8\" \ntype \"oval\" \nw 500.0000000000 \nh 200.0000000000\n]";
	std::string cloudwG = "\ngraphics [\nfill \"#a9a9a0\" \ntype \"oval\" \nw 500.0000000000 \nh 200.0000000000\n]";
	std::string cloudG = "\ngraphics [\nfill \"#a9a9a1\" \ntype \"oval\" \nw 500.0000000000 \nh 200.0000000000\n]";

	if (devT == "1")
	{
		return routerG;
	}
	else if (devT == "2")
	{
		return switchG;
	}
	else if (devT == "3")
	{
		return firewallG;
	}
	else if (devT == "4")
	{
		return apG;
	}
	else if (devT == "5")
	{
		return switchG;
	}
	else if (devT == "6")
	{
		return vrouterG;
	}
	else if (devT == "7")
	{
		return switchRG;
	}
	else if (devT == "8")
	{
		return l3switchG;
	}
	else if (devT == "9")
	{
		return l3switchRG;
	}
	else if (devT == "10")
	{
		return routerG;
	}
	else if (devT == "11")
	{
		return phoneG;
	}
	else if (devT == "12")
	{
		return endgeraetG;
	}
	else if (devT == "13")
	{
		return ataG;
	}
	else if (devT == "14")
	{
		return cameraG;
	}
	else if (devT == "20")
	{
		return switchG;
	}
	else if (devT == "21")
	{
		return switchG;
	}
	else if (devT == "22")
	{
		return switchRG;
	}
	else if (devT == "23")
	{
		return switchRG;
	}
	else if (devT == "30")
	{
		return netzG;
	}
	else if (devT == "40")
	{
		return cloudgG;
	}
	else if (devT == "41")
	{
		return cloudwG;
	}
	else if (devT == "42")
	{
		return cloudG;
	}
	else if (devT == "99")
	{
		return unknownG;
	}
	else
	{
		return endgeraetG;
	}
}


void WkmAusgabe::sets(bool c, bool ch, bool u, bool urp, bool layer2, bool layer3, bool layer3routing, bool o, bool h, bool g, std::string unitEdgeLength,	
					  std::string lDistance, std::string nDistance, std::string weightBalancing, bool es, int istyle)
{
	cdp = c;
	cdp_hosts = ch;
	unknownSTP = u;
	unknownRP = urp;
	l2 = layer2;
	l3 = layer3;
	l3routing = layer3routing;
	hier = h;
	org = o;
	ortho = g;
	nd = nDistance;
	ld = lDistance;
	wb = weightBalancing;
	uel = unitEdgeLength;
	endsystems = es;
	intfStyle = istyle;
}


void WkmAusgabe::markiereEintraege()
{
	std::string flagString = "0";
	if (!cdp)
	{
		flagString = "99";
	}

	std::string sString = "UPDATE OR REPLACE neighborship SET flag= " + flagString + " WHERE n_id IN ( ";
	sString += "SELECT n_id FROM neighborship WHERE dI_dev_id1 IN (SELECT dev_id FROM device WHERE dataSource=1 AND type<10));";
	dieDB->query(sString.c_str());

	flagString = "0";
	if (!cdp_hosts)
	{
		flagString = "99";
	}
	sString = "UPDATE OR REPLACE neighborship SET flag= " + flagString + " WHERE n_id IN ( ";
	sString += "SELECT n_id FROM neighborship WHERE dI_dev_id1 IN (SELECT dev_id FROM device WHERE dataSource=1 AND type>10));";
	dieDB->query(sString.c_str());

	flagString = "0";
	if (!unknownSTP)
	{
		flagString = "99";
	}
	sString = "UPDATE OR REPLACE neighborship SET flag= " + flagString + " WHERE n_id IN ( ";
	sString += "SELECT n_id FROM neighborship WHERE dI_dev_id1=0);";
	dieDB->query(sString.c_str());

	flagString = "0";
	if (!endsystems)
	{
		flagString = "99";
	}
	sString = "UPDATE OR REPLACE neighborship SET flag= " + flagString + " WHERE n_id IN ( ";
	sString += "SELECT n_id FROM neighborship WHERE dI_dev_id1 IN (SELECT dev_id FROM device WHERE dataSource=2));";
	dieDB->query(sString.c_str());

	flagString = "0";
	if (!unknownRP)
	{
		flagString = "99";
	}
	sString = "UPDATE OR REPLACE neighborship SET flag= " + flagString + " WHERE n_id IN ( ";
	sString += "SELECT n_id FROM neighborship WHERE dI_dev_id1 IN (SELECT dev_id FROM device WHERE dataSource=3));";
	dieDB->query(sString.c_str());
}


int WkmAusgabe::doItL3()
{
	neighborEmpty = true;

	nodes = "";
	edges = "";
	clusters = "\nrootcluster [";
	
	// Hinweis zur Interface Farbkodierung
	// Interface Farbe:
	// 000000
	// 1|||||	...Interface: 0: Normal; 1: Muss kontrolliert werden; 2: Unknown Neigbor; 3: STP Blocking; 4: STP Mixed
	//  2||||	...For Future Use
	//   3|||	...Error: 0: OK; 1: WARNING; 2: PROBLEM
	//    4||	...Load: 0: OK; 1: WARNING; 2: PROBLEM
	//     5|	...Duplex: 0: FULL; 1: HALF
	//      6	...STP Transitions: 0: OK; 1: WARNING; 2: PROBLEM

	// GML Rahmenstrings initialisieren
	std::string interfaceG_Neu_Start = "\ngraphics [\nfill \"#";														// Normales Interface mit neuer Kodierung START
	std::string interfaceG_Neu_Ende = "\" \ntype \"rectangle\" \nw 100.0000000000 \nh 100.0000000000\n]";				// Normales Interface mit neuer Kodierung ENDE
	std::string interfaceFarbe = "000000";																				// Farbcode START
	std::string nodeStartG = "node [\nid ";
	std::string nodeLabelAG = "\nlabel \"";
	std::string nodeLabelEG = "\"";
	std::string nodeEndeG = "\n]\n";
	std::string edgeSourceG = "edge [\nsource ";
	std::string edgeTargetG = "\ntarget ";
	std::string edgeEndeG = "\n]\n";
	std::string clusterStart = "\ncluster [\nid ";
	std::string clusterVertexStart = "\nvertex \"";
	std::string clusterVertexEnd = "\"";
	std::string clusterEnd = "\n]";

	int nId = 0;		// Node ID
	int edgeId = 0;		// Edge ID
	int clusterId = 0;	// Cluster-ID
	int tClusterId = 10000;
	std::vector<std::string> clusterStrings;		// Cluster Strings, die dann zum Schluss zusammengefasst werden

	std::map<std::string,int> nIdsCluster;		// Sammlung aller Node IDs um mehrfache Einträge als Node zu verhindern und die Clusterobjekte richtig zuzuordnen
	std::map<std::string,int>::iterator nItCluster;		// Iterator für die Node/Cluster Sammlung
	typedef std::pair <std::string,int> psi;		// Pair für das Einfügen in die Map

	std::map<std::string,std::string> nIds;			// Sammlung aller Node IDs um mehrfache Einträge als Node zu verhindern
	std::map<std::string,std::string>::iterator nIt;		// Iterator für die Node Sammlung
	typedef std::pair <std::string,std::string> pss;		// Pair für das Einfügen in die Map

	std::multimap <std::string, std::string> IntfData;	// Sammlung aller Interfaces mit Fehlern und Warnungen
	std::multimap <std::string, std::string>::iterator IntfDataIter, IntfDataAnfang, IntfDataEnde;

	std::string nId1 = "";		// Node ID 1 -> für die Edge Beschreibung
	std::string nId2 = "";		// Node ID 2 -> für die Edge Beschreibung
	std::string nId3 = "";		// Node ID 3 -> für die Edge Beschreibung
	std::string nIDString = "";	// Temp Node ID als String

	std::string tempClusterId = "";	// Cluster ID für den Interface/Error Cluster


	// ipSubnet Tabelle auslesen
	std::string sString = "SELECT DISTINCT subnet,mask FROM ipSubnet ";
	sString += "WHERE ipSubnet_id > (SELECT MAX(ipSubnet_id) FROM rControl WHERE ipSubnet_id < (SELECT MAX(ipSubnet_id) FROM rControl));";

	std::vector<std::vector<std::string>> netze = dieDB->query(sString.c_str());
	for(std::vector<std::vector<std::string>>::iterator it = netze.begin(); it < netze.end(); ++it)
	{
		// Interface Farbe zurücksetzen
		interfaceFarbe = "000000";

		std::string h1 = it->at(0) + " / " + it->at(1);
		std::string type1 = "30";
		
		nIt = nIds.find(h1);
		if (nIt == nIds.end())
		{
			nIDString = boost::lexical_cast<std::string>(nId);
			nIdsCluster.insert(psi(it->at(0), clusterId));
			nIds.insert(pss(it->at(0), nIDString));
			nodes += nodeStartG + nIDString + nodeLabelAG + h1 + nodeLabelEG + setDevType(type1) + nodeEndeG;
			nId1 = nIDString;
			nId++;
			clusterStrings.push_back(clusterStart + boost::lexical_cast<std::string>(clusterId));
			clusterStrings[clusterId] += clusterVertexStart + nId1 + clusterVertexEnd;
			clusterId++;
		}
		else
		{
			nId1 = nIt->second;
		}

		// Interfaces finden, die eine IP Adresse in dem Subnet haben
		std::string i1 = "";
		std::string h2 = "";
		std::string hh2 = "";
		std::string type2 = "";

		sString = "SELECT intf_id, intfName, duplex, errLvl, loadLvl, ipAddress FROM interfaces ";
		sString += "INNER JOIN intfSubnet ON intfSubnet.interfaces_intf_id=interfaces.intf_id ";
		sString += "INNER JOIN ipSubnet ON ipSubnet.ipSubnet_id=intfSubnet.ipSubnet_ipSubnet_id ";
		sString += "WHERE ipSubnet.subnet LIKE '" + it->at(0) +"' AND status LIKE 'UP' ";
		sString += "AND intf_id > (SELECT MAX(intf_id) FROM rControl WHERE intf_id < (SELECT MAX(intf_id) FROM rControl));";

		std::vector<std::vector<std::string>> intfs = dieDB->query(sString.c_str());
		for(std::vector<std::vector<std::string>>::iterator it1 = intfs.begin(); it1 < intfs.end(); ++it1)
		{
			// Zugehörigen Hostnamen auslesen
			sString = "SELECT hostname, type, dataSource, dev_id from device ";
			sString += "INNER JOIN devInterface ON devInterface.device_dev_id=device.dev_id ";
			sString += "WHERE devInterface.interfaces_int_id=" + it1->at(0) + ";";
			std::vector<std::vector<std::string>> dev = dieDB->query(sString.c_str());

			// Wenn cdp Endgeräte nicht ausgegeben werden sollen, dann überspringen
			int dType = boost::lexical_cast<int>(dev[0][1]);
			if ((dev[0][2] == "1") && (!cdp) && (dType < 10))
			{
				continue;
			}

			if ((dev[0][2] == "1") && (!cdp_hosts) && (dType > 10))
			{
				continue;
			}

			
			h2 = dev[0][3];
			hh2 = dev[0][0];
			type2 = dev[0][1];			

			nIt = nIds.find(h2);
			if (nIt == nIds.end())
			{
				nIDString = boost::lexical_cast<std::string>(nId);
				nIdsCluster.insert(psi(h2, clusterId));
				nIds.insert(pss(h2, nIDString));
				nodes += nodeStartG + nIDString + nodeLabelAG + h2 + nodeLabelEG + setDevType(type2) + nodeEndeG;
				nId3 = nIDString;
				nId++;
				clusterStrings.push_back(clusterStart + boost::lexical_cast<std::string>(clusterId));
				clusterStrings[clusterId] += clusterVertexStart + nId3 + clusterVertexEnd;
				clusterId++;
			}
			else
			{
				nId3 = nIt->second;
			}

			std::string dup = it1->at(2);
			if (dup.find("Half-duplex") != dup.npos)
			{
				interfaceFarbe[4] = '1';
			}

			int err = 0;
			try
			{
				err = boost::lexical_cast<int>(it1->at(3));
			}
			catch (boost::bad_lexical_cast &)
			{
				err = 0;			
			}

			if (err > 1000)
			{
				interfaceFarbe[2] = '2';
			}
			else if (err > 10)
			{
				interfaceFarbe[2] = '1';
			}


			int load = 0;
			try
			{
				load = boost::lexical_cast<int>(it1->at(4));
			}
			catch (boost::bad_lexical_cast &)
			{
				load = 0;			
			}

			if (load > 200)
			{
				interfaceFarbe[3] = '2';
			}
			else if (load > 100)
			{
				interfaceFarbe[2] = '1';
			}

			i1 = it1->at(0) + "-" + it1->at(1);
			
			// IP Adresse verwerten
			std::string ipA = "";
			if (it1->at(5) != "")
			{
				size_t ipPos = it1->at(5).rfind(".");
				if (ipPos != it1->at(5).npos)
				{
					ipA = it1->at(5).substr(ipPos, it1->at(5).length() - ipPos);
					i1 += " / " + ipA;
				}
			}

			nId2 = boost::lexical_cast<std::string>(nId);
			nId++;
			nodes += nodeStartG + nId2 + nodeLabelAG + i1 + nodeLabelEG + interfaceG_Neu_Start + interfaceFarbe + interfaceG_Neu_Ende + nodeEndeG;

			tempClusterId = boost::lexical_cast<std::string>(tClusterId);
			tClusterId++;

			nItCluster = nIdsCluster.find(it->at(0));
			clusterStrings[nItCluster->second] += clusterStart + tempClusterId;
			// Cluster Strings weiterbauen
			clusterStrings[nItCluster->second] += clusterVertexStart + nId2 + clusterVertexEnd;


			// Inneren Cluster abschließen
			clusterStrings[nItCluster->second] += clusterEnd;

			// Edges einfügen
			edges += edgeSourceG + nId1 + edgeTargetG + nId2 + edgeEndeG;
			edges += edgeSourceG + nId2 + edgeTargetG + nId3 + edgeEndeG;

			// Fehlerdaten Edges einfügen
			IntfDataIter = IntfData.begin();
			while (IntfDataIter != IntfData.end())
			{
				edges += edgeSourceG + IntfDataIter->first + edgeTargetG + IntfDataIter->second + edgeEndeG;
				IntfDataIter++;
			}
			IntfData.clear();
			
			// Ausgabe
			std::string nachbaren = hh2 + "-" + i1 + " --> " + h1 + "\n";
			schreibeLog(WkLog::WkLog_NORMALTYPE, nachbaren, "", WkLog::WkLog_SCHWARZ);

			neighborEmpty = false;
		}

	}
	// Clusterstrings abschließen und zusammenführen
	for (int i=0; i<clusterStrings.size(); i++)
	{
		clusters += clusterStrings[i] += clusterEnd;
	}
	clusters += clusterEnd;

	if (!zeichneGraph("L3"))
	{
		schreibeLog(WkLog::WkLog_ZEIT, "6203: Unable to draw graph!\r\n", "6203", 
			WkLog::WkLog_ROT, WkLog::WkLog_FETT);
	}
	return 1;
}


// Report erstellen
void WkmAusgabe::report(int auswertung /* = 0 */)
{
	bool debugAusgabe = false;
#ifdef DEBUG
	debugAusgabe = true;
#endif

	reportEndSystems();
	reportIPT();
	reportSTP();
	reportL2();
	reportL2Devs();
	reportIntfErr();
	reportInventory();
	reportL3RP();
	reportL3RPneighbors();
	reportL3ospf();
	reportDot1xIntfStat();
	reportDot1xFailed();
	reportDiff("neighborship");
	reportDiff("rpneighborship");

	// Channel Error
	//////////////////////////////////////////////////////////////////////////
	std::string sString = "SELECT intf_id,intfName,device.hostname,channel_intf_id FROM interfaces ";
	sString += "INNER JOIN devInterface ON devInterface.interfaces_int_id=interfaces.intf_id ";
	sString += "INNER JOIN device ON device.dev_id=devInterface.device_dev_id ";
	sString += "WHERE channel_intf_id NOT NULL AND status NOT LIKE 'UP' AND channel_intf_id IN ( ";
	sString += "SELECT intf_id FROM interfaces WHERE intfType LIKE 'Channel' AND status LIKE 'UP') ";
	sString += "AND intf_id > (SELECT MAX(intf_id) FROM rControl WHERE intf_id < (SELECT MAX(intf_id) FROM rControl));";
	std::vector<std::vector<std::string>> s0 = dieDB->query(sString.c_str());
	if (!s0.empty())
	{
		int nummer = 0;
		schreibeLog(WkLog::WkLog_NORMALTYPE, "\nChannel Errors:\nFor active Channels the following Channel Members are down:\n", "", WkLog::WkLog_BLAU, WkLog::WkLog_FETT);
		schreibeLog(WkLog::WkLog_NORMALTYPE, boost::str(boost::format("%-5s%-40s%-25s%-25s\n") % "Nr." % "Hostname" % "Interface" % "Channel"), "", WkLog::WkLog_BLAU);
		for(std::vector<std::vector<std::string>>::iterator it = s0.begin(); it < s0.end(); ++it)
		{
			nummer++;
			sString = "SELECT intfName FROM interfaces WHERE intf_id=" + it->at(3);
			std::vector<std::vector<std::string>> s01 = dieDB->query(sString.c_str());
			if (!s01.empty())
			{
				schreibeLog(WkLog::WkLog_NORMALTYPE, boost::str(boost::format("%-5s%-40s%-25s%-25s\n") % boost::lexical_cast<std::string>(nummer) % it->at(2) % it->at(1) % s01[0][0]), "", WkLog::WkLog_SCHWARZ);
			}
		}
	}


	
	// Statistik
	//////////////////////////////////////////////////////////////////////////
	schreibeLog(WkLog::WkLog_NORMALTYPE, "\nStatistics\n", "", WkLog::WkLog_BLAU, WkLog::WkLog_FETT);
	schreibeLog(WkLog::WkLog_NORMALTYPE, boost::str(boost::format("%-13s%-13s%-13s%-13s%-13s%-13s\n") % "#Devices" % "#Neighbships" % "#CDP Neighb" % "# Act Intf" % "#Total Intf" % "#DeviceTypes"), "", WkLog::WkLog_BLAU);

	sString = "SELECT COUNT (dev_id) FROM device ";
	sString += "WHERE dev_id > (SELECT MAX(dev_id) FROM rControl WHERE dev_id < (SELECT MAX(dev_id) FROM rControl));";
	std::vector<std::vector<std::string>> s1 = dieDB->query(sString.c_str());
	std::vector<std::vector<std::string>>::iterator i1 = s1.begin();	

	sString = "SELECT COUNT (intf_id) FROM interfaces WHERE phl=1 ";
	sString += "AND intf_id > (SELECT MAX(intf_id) FROM rControl WHERE intf_id < (SELECT MAX(intf_id) FROM rControl));";
	std::vector<std::vector<std::string>> s2 = dieDB->query(sString.c_str());
	std::vector<std::vector<std::string>>::iterator i2 = s2.begin();	
	
	sString = "SELECT COUNT (intf_id) FROM interfaces WHERE phl=1 AND status LIKE 'UP' ";
	sString += "AND intf_id > (SELECT MAX(intf_id) FROM rControl WHERE intf_id < (SELECT MAX(intf_id) FROM rControl));";
	std::vector<std::vector<std::string>> s21 = dieDB->query(sString.c_str());
	std::vector<std::vector<std::string>>::iterator i21 = s21.begin();	
		
	sString = "SELECT COUNT (n_id) FROM neighborship ";
	sString += "WHERE n_id > (SELECT MAX(n_id) FROM rControl WHERE n_id < (SELECT MAX(n_id) FROM rControl));";
	std::vector<std::vector<std::string>> s3 = dieDB->query(sString.c_str());
	std::vector<std::vector<std::string>>::iterator i3 = s3.begin();	

	sString = "SELECT COUNT (dev_id) FROM device ";
	sString += "WHERE dev_id > (SELECT MAX(dev_id) FROM rControl WHERE dev_id < (SELECT MAX(dev_id) FROM rControl)) AND dataSource=1;";
	std::vector<std::vector<std::string>> s4 = dieDB->query(sString.c_str());
	std::vector<std::vector<std::string>>::iterator i4 = s4.begin();	

	sString = "select count(distinct hwtype) from device ";
	sString += "WHERE dev_id > (SELECT MAX(dev_id) FROM rControl WHERE dev_id < (SELECT MAX(dev_id) FROM rControl));";
	std::vector<std::vector<std::string>> s5 = dieDB->query(sString.c_str());
	std::vector<std::vector<std::string>>::iterator i5 = s5.begin();	
	
	schreibeLog(WkLog::WkLog_NORMALTYPE, boost::str(boost::format("%-13s%-13s%-13s%-13s%-13s%-13s\n") % i1->at(0) % i3->at(0) % i4->at(0) % i21->at(0) % i2->at(0) % i5->at(0)), "", WkLog::WkLog_SCHWARZ);
}


void WkmAusgabe::reportDiff(std::string dbtable)
{
	bool debugAusgabe = false;
#ifdef DEBUG
	debugAusgabe = true;
#endif
	// Diff über die letzte und vorletzte Auswertung
	//////////////////////////////////////////////////////////////////////////
	// Diff nur machen, wenn mehr als ein Durchlauf gemacht wurde
	std::string sString = "SELECT dev_id FROM rControl";
	std::vector<std::vector<std::string>> dbg = dieDB->query(sString.c_str());	
	if (dbg.size() > 2)
	{
		// Temp Tabellen anlegen
		sString = "CREATE TEMP TABLE aktuell (h1 TEXT, h2 TEXT, i1 TEXT, i2 TEXT)";
		dbg = dieDB->query(sString.c_str());
		sString = "CREATE TEMP TABLE vgl (h1 TEXT, h2 TEXT, i1 TEXT, i2 TEXT)";
		dbg = dieDB->query(sString.c_str());

		// Letzte Auswertung in die Tabelle eintragen
		sString = "SELECT dI_dev_id,dI_intf_id,dI_dev_id1,dI_intf_id1 FROM " + dbtable;
		sString += " WHERE dI_dev_id > (SELECT MAX(dev_id) FROM rControl WHERE dev_id < (SELECT MAX(dev_id) FROM rControl));";

		dbg = dieDB->query(sString.c_str());
		for(std::vector<std::vector<std::string>>::iterator dbgIt = dbg.begin(); dbgIt < dbg.end(); ++dbgIt)
		{
			sString = "SELECT device.hostname FROM device WHERE dev_id=" + dbgIt->at(0) + ";";
			std::vector<std::vector<std::string>> dbg1 = dieDB->query(sString.c_str());
			std::string h1 = dbg1[0][0];
			std::string h2;
			if (dbgIt->at(2) != "0")
			{
				sString = "SELECT device.hostname FROM device WHERE dev_id=" + dbgIt->at(2) + ";";
				dbg1 = dieDB->query(sString.c_str());
				h2 = dbg1[0][0];
			}
			else
			{
				h2 = "UNKNOWN";
			}
			sString = "SELECT intfName FROM interfaces WHERE interfaces.intf_id=" + dbgIt->at(1) + ";";
			dbg1 = dieDB->query(sString.c_str());
			std::string i1 = "UNKNOWN";
			if (!dbg1.empty())
			{
				i1 = dbg1[0][0];
			}
			std::string i2;
			if (dbgIt->at(3) != "0")
			{
				sString = "SELECT intfName FROM interfaces WHERE interfaces.intf_id=" + dbgIt->at(3) + ";";
				dbg1 = dieDB->query(sString.c_str());
				i2 = dbg1[0][0];
			}
			else
			{
				i2 = "UNKNOWN";
			}
			sString = "INSERT INTO aktuell (h1, h2, i1, i2) VALUES ('" + h1 + "', '" + h2 + "', '" + i1 + "', '" + i2 + "')";
			dieDB->query(sString.c_str());			

			if (debugAusgabe)
			{
				std::string nachbaren = h1 + "-" + i1 + " <--> " + h2 + "-" + i2 + "\n";
				schreibeLog(WkLog::WkLog_NORMALTYPE, nachbaren, "", WkLog::WkLog_SCHWARZ);
			}
		}


		std::string sString = "SELECT dev_id FROM rControl";
		std::vector<std::vector<std::string>> dbg = dieDB->query(sString.c_str());	
		std::vector<std::vector<std::string>>::iterator it = dbg.end()-3;


		// Auswertung, mit der verglichen werden soll, in die Tabelle eintragen
		if (debugAusgabe)
		{
			schreibeLog(WkLog::WkLog_NORMALTYPE, "Last run\n", "", WkLog::WkLog_SCHWARZ);
		}
		sString = "SELECT dI_dev_id,dI_intf_id,dI_dev_id1,dI_intf_id1 FROM " + dbtable;
		sString += " WHERE dI_dev_id > " + it->at(0) + " AND dI_dev_id <= (SELECT MAX(dev_id) FROM rControl WHERE dev_id < (SELECT MAX(dev_id) FROM rControl));";

		dbg = dieDB->query(sString.c_str());
		for(std::vector<std::vector<std::string>>::iterator dbgIt = dbg.begin(); dbgIt < dbg.end(); ++dbgIt)
		{
			sString = "SELECT device.hostname FROM device WHERE dev_id=" + dbgIt->at(0) + ";";
			std::vector<std::vector<std::string>> dbg1 = dieDB->query(sString.c_str());
			std::string h1 = dbg1[0][0];
			std::string h2;
			if (dbgIt->at(2) != "0")
			{
				sString = "SELECT device.hostname FROM device WHERE dev_id=" + dbgIt->at(2) + ";";
				dbg1 = dieDB->query(sString.c_str());
				h2 = dbg1[0][0];
			}
			else
			{
				h2 = "UNKNOWN";
			}
			sString = "SELECT intfName FROM interfaces WHERE interfaces.intf_id=" + dbgIt->at(1) + ";";
			dbg1 = dieDB->query(sString.c_str());
			std::string i1 = "UNKNOWN";
			if (!dbg1.empty())
			{
				i1 = dbg1[0][0];
			}
			std::string i2;
			if (dbgIt->at(3) != "0")
			{
				sString = "SELECT intfName FROM interfaces WHERE interfaces.intf_id=" + dbgIt->at(3) + ";";
				dbg1 = dieDB->query(sString.c_str());
				i2 = dbg1[0][0];
			}
			else
			{
				i2 = "UNKNOWN";
			}

			sString = "INSERT INTO vgl (h1, h2, i1, i2) VALUES ('" + h1 + "', '" + h2 + "', '" + i1 + "', '" + i2 + "')";
			dieDB->query(sString.c_str());			

			if (debugAusgabe)
			{
				std::string nachbaren = h1 + "-" + i1 + " <--> " + h2 + "-" + i2 + "\n";
				schreibeLog(WkLog::WkLog_NORMALTYPE, nachbaren, "", WkLog::WkLog_SCHWARZ);
			}
		}

		// DIFF über die beiden TEMP Tabellen
		if (dbtable == "rpneighborship")
		{
			schreibeLog(WkLog::WkLog_NORMALTYPE, "\nL3 Routing Neighborship DIFF between last (A) and penultimate (B) run:\n", "", WkLog::WkLog_BLAU, WkLog::WkLog_FETT);
		}
		else if (dbtable == "neighborship")
		{
			schreibeLog(WkLog::WkLog_NORMALTYPE, "\nL2 DIFF between last (A) and penultimate (B) run:\n", "", WkLog::WkLog_BLAU, WkLog::WkLog_FETT);
		}

		sString = "SELECT MIN(aktuell) as aktuell, h1, h2, i1, i2 ";
		sString += "FROM (SELECT 'A' as aktuell, aktuell.h1, aktuell.h2, aktuell.i1, aktuell.i2 ";
		sString += "FROM aktuell UNION ALL SELECT 'B' as vgl, vgl.h1, vgl.h2, vgl.i1, vgl.i2 ";
		sString += "FROM vgl) tmp GROUP BY h1, h2, i1, i2 ";
		sString += "HAVING COUNT(*) = 1 ORDER BY h1";
		dbg = dieDB->query(sString.c_str());
		for(std::vector<std::vector<std::string>>::iterator it = dbg.begin(); it < dbg.end(); ++it)
		{
			std::string diffs = it->at(0) + ": " + it->at(1) + "-" + it->at(3) + " <--> " + it->at(2) + "-" + it->at(4) + "\n";
			schreibeLog(WkLog::WkLog_NORMALTYPE, diffs, "", WkLog::WkLog_SCHWARZ);
		}

		// Temp Tabellen wieder löschen
		sString = "DROP TABLE aktuell ";
		dbg = dieDB->query(sString.c_str());
		sString = "DROP TABLE vgl";
		dbg = dieDB->query(sString.c_str());

	}
}


void WkmAusgabe::reportIntfErr()
{
	// Temp Tabellen anlegen
	std::string sString = "CREATE TEMP TABLE aktIntfErr (hostname TEXT, interface TEXT, duplex TEXT, errlvl TEXT, loadlvl TEXT, cdpn TEXT, cdpntype TEXT)";
	std::vector<std::vector<std::string>> dbg = dieDB->query(sString.c_str());
	sString = "CREATE TEMP TABLE vglIntfErr (hostname TEXT, interface TEXT, duplex TEXT, errlvl TEXT, loadlvl TEXT, cdpn TEXT, cdpntype TEXT)";
	dbg = dieDB->query(sString.c_str());
	// Diff nur machen, wenn mehr als ein Durchlauf gemacht wurde
	bool intfErrDiff = false;
	sString = "SELECT dev_id FROM rControl";
	dbg = dieDB->query(sString.c_str());	
	if (dbg.size() > 2)
	{
		intfErrDiff = true;
	}
	//////////////////////////////////////////////////////////////////////////

	sString = "SELECT dev_id, hostname FROM device ";
	sString += "WHERE dev_id > (SELECT MAX(dev_id) FROM rControl WHERE dev_id < (SELECT MAX(dev_id) FROM rControl));";

	dbg = dieDB->query(sString.c_str());
	schreibeLog(WkLog::WkLog_NORMALTYPE, "\nInterfaces with more than 1000 errors, load > 100 or half duplex\n", "", WkLog::WkLog_BLAU, WkLog::WkLog_FETT);

	schreibeLog(WkLog::WkLog_NORMALTYPE, boost::str(boost::format("%-5s%-25s%-10s%-20s%-11s%-12s%-8s%-30s%-25s%-25s\n") % "Nr." % "Hostname" % "Interface" % "Duplex" % "ErrorLvl" % "CountLastCl" % "LoadLvl" % "CDP Neighbor" % "CDP Neigh Type" % "Description"), "", WkLog::WkLog_BLAU);

	int nummer = 0;		// Fortlaufende Nummer/Zeilennummer

	for(std::vector<std::vector<std::string>>::iterator dbgIt = dbg.begin(); dbgIt < dbg.end(); ++dbgIt)
	{
		sString = "SELECT intfName,duplex,errLvl,loadLvl,description,intf_id,lastClear FROM interfaces ";
		sString += "INNER JOIN devInterface ON devInterface.interfaces_int_id=interfaces.intf_id ";
		sString += "INNER JOIN device ON device.dev_id=devInterface.device_dev_id ";
		sString += "WHERE intf_id > (SELECT MAX(intf_id) FROM rControl WHERE intf_id < (SELECT MAX(intf_id) FROM rControl)) ";
		sString += "AND device.dev_id= " + dbgIt->at(0) + " AND (errLvl >1000 OR loadLvl >100 OR duplex LIKE '%alf%') ";
		sString += "ORDER BY errLvl DESC ";
		std::vector<std::vector<std::string>> intf = dieDB->query(sString.c_str());

		for(std::vector<std::vector<std::string>>::iterator it = intf.begin(); it < intf.end(); ++it)
		{
			nummer++;
			sString = "SELECT nName,nIntf,platform FROM cdp INNER JOIN clink ON clink.cdp_cdp_id=cdp.cdp_id ";
			sString += "INNER JOIN interfaces ON interfaces.intf_id=clink.interfaces_intf_id INNER JOIN devInterface ON devInterface.interfaces_int_id=interfaces.intf_id ";
			sString += "WHERE devInterface.interfaces_int_id= " + it->at(5);
			std::vector<std::vector<std::string>> cdpNeigh = dieDB->query(sString.c_str());
			std::vector<std::vector<std::string>>::iterator cdpIt = cdpNeigh.begin();
			std::string cdpN = "";
			std::string cdpT = "";
			if (cdpIt < cdpNeigh.end())
			{
				cdpN = cdpIt->at(0) + " // " + cdpIt->at(1);
				cdpT = cdpIt->at(2);
			}

			std::string ausgabeString = boost::str(boost::format("%-5s%-25s%-10s%-20s%-11s%-12s%-8s%-30s%-25s%-25s\n") % boost::lexical_cast<std::string>(nummer) % dbgIt->at(1) % it->at(0) % it->at(1) % it->at(2) % it->at(6) % it->at(3) % cdpN % cdpT % it->at(4));
			schreibeLog(WkLog::WkLog_NORMALTYPE, ausgabeString, "", WkLog::WkLog_SCHWARZ);

			// Interface Fehler Diff über die letzte und vorletzte Auswertung - Aktuelle Werte in die Temporäre Tabelle eintragen
			//////////////////////////////////////////////////////////////////////////
			if (intfErrDiff)
			{
				sString = "INSERT INTO aktIntfErr (hostname, interface, duplex, errlvl, loadlvl, cdpn, cdpntype) VALUES ('" +  dbgIt->at(1) + "', '" + it->at(0) + "', '" + it->at(1) + "', '" + it->at(2) + "', '" + it->at(3) + "', '" + cdpN + "', '" + cdpT + "')";
				dieDB->query(sString.c_str());			
			}
		}
	}

	// Interface Fehler Diff über die letzte und vorletzte Auswertung - Werte vom vorletzten Durchlauf in die Temp Tabelle eintragen
	//////////////////////////////////////////////////////////////////////////
	if (intfErrDiff)
	{
		sString = "SELECT dev_id FROM rControl";
		dbg = dieDB->query(sString.c_str());	
		std::vector<std::vector<std::string>>::iterator it = dbg.end()-3;

		sString = "SELECT dev_id, hostname FROM device ";
		sString += "WHERE dev_id > " + it->at(0) + " AND dev_id <= (SELECT MAX(dev_id) FROM rControl WHERE dev_id < (SELECT MAX(dev_id) FROM rControl));";

		dbg = dieDB->query(sString.c_str());
		schreibeLog(WkLog::WkLog_NORMALTYPE, "\nInterfaces Error Diff\n", "", WkLog::WkLog_BLAU, WkLog::WkLog_FETT);

		for(std::vector<std::vector<std::string>>::iterator dbgIt = dbg.begin(); dbgIt < dbg.end(); ++dbgIt)
		{
			sString = "SELECT intfName,duplex,errLvl,loadLvl,description,intf_id,lastClear FROM interfaces ";
			sString += "INNER JOIN devInterface ON devInterface.interfaces_int_id=interfaces.intf_id ";
			sString += "INNER JOIN device ON device.dev_id=devInterface.device_dev_id ";
			//sString += "WHERE intf_id > (SELECT MAX(intf_id) FROM rControl WHERE intf_id < (SELECT MAX(intf_id) FROM rControl)) ";
			sString += "WHERE device.dev_id= " + dbgIt->at(0) + " AND (errLvl >1000 OR loadLvl >100 OR duplex LIKE '%alf%') ";
			sString += "ORDER BY errLvl DESC ";
			std::vector<std::vector<std::string>> intf = dieDB->query(sString.c_str());

			for (std::vector<std::vector<std::string>>::iterator it = intf.begin(); it < intf.end(); ++it)
			{
				sString = "SELECT nName,nIntf,platform FROM cdp INNER JOIN clink ON clink.cdp_cdp_id=cdp.cdp_id ";
				sString += "INNER JOIN interfaces ON interfaces.intf_id=clink.interfaces_intf_id ";
				sString += "WHERE interfaces.intf_id= " + it->at(5);
				std::vector<std::vector<std::string>> cdpNeigh = dieDB->query(sString.c_str());
				std::vector<std::vector<std::string>>::iterator cdpIt = cdpNeigh.begin();
				std::string cdpN = "";
				std::string cdpT = "";
				if (cdpIt < cdpNeigh.end())
				{
					cdpN = cdpIt->at(0) + " // " + cdpIt->at(1);
					cdpT = cdpIt->at(2);
				}

				sString = "INSERT INTO vglIntfErr (hostname, interface, duplex, errlvl, loadlvl, cdpn, cdpntype) VALUES ('" +  dbgIt->at(1) + "', '" + it->at(0) + "', '" + it->at(1) + "', '" + it->at(2) + "', '" + it->at(3) + "', '" + cdpN + "', '" + cdpT + "')";
				dieDB->query(sString.c_str());
			}
		}

		// DIFF über die beiden TEMP Tabellen
		schreibeLog(WkLog::WkLog_NORMALTYPE, "\nDIFF between last (A) and penultimate (B) run:\n", "", WkLog::WkLog_BLAU, WkLog::WkLog_FETT);
		schreibeLog(WkLog::WkLog_NORMALTYPE, boost::str(boost::format("%-5s%-25s%-10s%-15s%-11s%-8s%-30s%-25s\n") % "Run" % "Hostname" % "Interface" % "Duplex" % "ErrorLvl" % "LoadLvl" % "CDP Neighbor" % "CDP Neigh Type"), "", WkLog::WkLog_BLAU);

		sString = "SELECT MIN(aktIntfErr) as aktIntfErr, hostname, interface, duplex, errlvl, loadlvl, cdpn, cdpntype ";
		sString += "FROM (SELECT 'A' as aktIntfErr, aktIntfErr.hostname, aktIntfErr.interface, aktIntfErr.duplex, aktIntfErr.errlvl, aktIntfErr.loadlvl, aktIntfErr.cdpn, aktIntfErr.cdpntype ";
		sString += "FROM aktIntfErr UNION ALL SELECT 'B' as vglIntfErr, vglIntfErr.hostname, vglIntfErr.interface, vglIntfErr.duplex, vglIntfErr.errlvl, vglIntfErr.loadlvl, vglIntfErr.cdpn, vglIntfErr.cdpntype ";
		sString += "FROM vglIntfErr) tmp GROUP BY hostname, interface, duplex, errlvl, loadlvl, cdpn, cdpntype ";
		sString += "HAVING COUNT(*) = 1 ORDER BY hostname";
		dbg = dieDB->query(sString.c_str());
		for (std::vector<std::vector<std::string>>::iterator it = dbg.begin(); it < dbg.end(); ++it)
		{
			std::string ausgabeString = boost::str(boost::format("%-5s%-25s%-10s%-15s%-11s%-8s%-30s%-25s\n") % it->at(0) % it->at(1) % it->at(2) % it->at(3) % it->at(4) % it->at(5) % it->at(6) % it->at(7));
			schreibeLog(WkLog::WkLog_NORMALTYPE, ausgabeString, "", WkLog::WkLog_SCHWARZ);
		}
	}
	// Temp Tabellen wieder löschen
	sString = "DROP TABLE vglIntfErr ";
	dbg = dieDB->query(sString.c_str());
	sString = "DROP TABLE aktIntfErr";
	dbg = dieDB->query(sString.c_str());

	// Detaillierter Fehler Report
	// csv File öffnen
	std::string dateiname = ausgabeVz + "intfErrorReport.csv";
	std::ofstream intfErrCsv(dateiname.c_str());
	// Ausgabe
	schreibeLog(WkLog::WkLog_NORMALTYPE, "\nDetailed Interface error report\n", "", WkLog::WkLog_BLAU, WkLog::WkLog_FETT);
	schreibeLog(WkLog::WkLog_NORMALTYPE, boost::str(boost::format("%-5s%-25s%-20s%-10s%-10s%-10s%-10s%-10s%-10s%-10s%-10s%-10s%-10s%-10s%-10s%-10s%-10s%-10s%-10s%-10s%-10s%-10s%-10s%-10s%-10s%-10s%-10s%-10s%-12s%-12s%-12s%-30s%-25s%-25s\n") % "Nr." % "Hostname" % "Interface" % "iErrors" % "iCRC" % "iFrame" % "iOverrun" % "iIgnored" % "iWatchdog" % "iPause" % "iDribbleC" % "iBuffer" % "l2Decode" % "runts" % "giants" % "throttles" % "underrun" % "oErrors" % "collision" % "babbles" % "lateColl" % "deferred" % "lostCar." % "noCar." % "oPause" % "bufSwap" % "resets" % "oBuffer" % "LastClear" % "Speed" % "Duplex" % "CDP Neighbor" % "CDP Neigh Type" % "Intf Description"), "", WkLog::WkLog_BLAU);

	intfErrCsv << "Hostname;Interface;iErrors;iCRC;iFrame;iOverrun;iIgnored;iWatchdog;iPause;iDribbleC;iBuffer;l2Decode;runts;giants;Throttles;underrun;oErrors;collision;babbles;lateColl;deferred;lostCar.;noCar.;oPause;bufSwap;resets;oBuffer;LastClear;Speed;Duplex;CDP Neighbor;CDP Neigh Type;Intf Description\n";

	sString = "SELECT intf_id,intfName,ierrors,iCrc,iFrame,iOverrun,iIgnored,iWatchdog,iPause,iDribbleCondition,ibuffer,l2decodeDrops,runts,giants,throttles,underrun,oerrors,oCollisions,oBabbles,oLateColl,oDeferred,oLostCarrier,oNoCarrier,oPauseOutput,oBufferSwapped,throttles,resets,obuffer,description,lastClear,speed,duplex FROM interfaces ";
	sString += "WHERE ((iCrc != '0' AND iCrc !='') OR (iFrame != '0' AND iFrame !='') OR (iOverrun != '0' AND iOverrun !='') OR (iIgnored != '0' AND iIgnored !='') ";
	sString += "OR (iWatchdog != '0' AND iWatchdog !='') OR (iPause != '0' AND iPause !='') OR (iDribbleCondition != '0' AND iDribbleCondition !='') OR (ibuffer != '0' AND ibuffer !='') ";
	sString += "OR (l2decodeDrops != '0' AND l2decodeDrops !='') OR (runts != '0' AND runts !='') OR (giants != '0' AND giants !='') OR (throttles != '0' AND throttles !='') ";
	sString += "OR (ierrors != '0' AND ierrors !='') OR (underrun != '0' AND underrun !='') OR (oerrors != '0' AND oerrors !='') OR (oCollisions != '0' AND oCollisions !='') ";
	sString += "OR (oBabbles != '0' AND oBabbles !='') OR (oLateColl != '0' AND oLateColl !='') OR (oDeferred != '0' AND oDeferred !='') OR (oLostCarrier != '0' AND oLostCarrier !='') ";
	sString += "OR (oNoCarrier != '0' AND oNoCarrier !='') OR (oPauseOutput != '0' AND oPauseOutput !='') OR (oBufferSwapped != '0' AND oBufferSwapped !='') ";
	sString += "OR (resets != '0' AND resets !='') OR (obuffer != '0' AND obuffer !='')) ";
	sString += "AND intf_id > (SELECT MAX(intf_id) FROM rControl WHERE intf_id < (SELECT MAX(intf_id) FROM rControl));";


	std::vector<std::vector<std::string>> l2errDet = dieDB->query(sString.c_str());
	nummer = 0;
	for (std::vector<std::vector<std::string>>::iterator it = l2errDet.begin(); it < l2errDet.end(); ++it)
	{
		sString = "SELECT nName,nIntf,platform FROM cdp INNER JOIN clink ON clink.cdp_cdp_id=cdp.cdp_id ";
		sString += "INNER JOIN interfaces ON interfaces.intf_id=clink.interfaces_intf_id ";
		sString += "WHERE interfaces.intf_id=" + it->at(0);
		std::vector<std::vector<std::string>> cdpNeigh = dieDB->query(sString.c_str());
		std::vector<std::vector<std::string>>::iterator cdpIt = cdpNeigh.begin();
		std::string cdpN = "";
		std::string cdpT = "";
		if (cdpIt < cdpNeigh.end())
		{
			cdpN = cdpIt->at(0) + " // " + cdpIt->at(1);
			cdpT = cdpIt->at(2);
		}

		sString = "SELECT hostname FROM device INNER JOIN devInterface ON devInterface.device_dev_id=device.dev_id ";
		sString += "INNER JOIN interfaces ON interfaces.intf_id=devInterface.interfaces_int_id WHERE intf_id=" + it->at(0);
		std::vector<std::vector<std::string>> l2errDet2 = dieDB->query(sString.c_str());
		std::vector<std::vector<std::string>>::iterator it2 = l2errDet2.begin();
		nummer++;
		if (it2 != l2errDet2.end())
		{
			std::string ausgabeString = boost::str(boost::format("%-5s%-25s%-10s%-10s%-10s%-10s%-10s%-10s%-10s%-10s%-10s%-10s%-10s%-10s%-10s%-10s%-10s%-10s%-10s%-10s%-10s%-10s%-10s%-10s%-10s%-10s%-10s%-10s%-12s%-12s%-12s%-30s%-25s%-25s\n") % boost::lexical_cast<std::string>(nummer) % it2->at(0) % it->at(1) % it->at(2) % it->at(3) % it->at(4) % it->at(5) % it->at(6) % it->at(7) % it->at(8) % it->at(9) % it->at(10) % it->at(11) % it->at(12) % it->at(13) % it->at(14) % it->at(15) % it->at(16) % it->at(17) % it->at(18) % it->at(19) % it->at(20) % it->at(21) % it->at(22) % it->at(23) % it->at(24) % it->at(26) % it->at(27) % it->at(29) % it->at(30) % it->at(31) % cdpN % cdpT % it->at(28));
			schreibeLog(WkLog::WkLog_NORMALTYPE, ausgabeString, "", WkLog::WkLog_SCHWARZ);

			for (int i = 0; i < 29; i++)
			{
				if (it->at(i) == "")
				{
					it->at(i) = "0";
				}
			}
			
			intfErrCsv << it2->at(0) << ";" << it->at(1) << ";" << it->at(2) << ";" << it->at(3) << ";" << it->at(4) << ";" << it->at(5) << ";" << it->at(6) << ";" << it->at(7) << ";" << it->at(8) << ";" << it->at(9) << ";" << it->at(10) << ";" << it->at(11) << ";" << it->at(12) << ";" << it->at(13) << ";" << it->at(14) << ";" << it->at(15) << ";" << it->at(16) << ";" << it->at(17) << ";" << it->at(18) << ";" << it->at(19) << ";" << it->at(20) << ";" << it->at(21) << ";" << it->at(22) << ";" << it->at(23) << ";" << it->at(24) << ";" << it->at(26) << ";" << it->at(27) << ";" << it->at(29) << ";" << it->at(30) << ";" << it->at(31) << ";" << cdpN << ";" << cdpT << ";" << it->at(28) << "\n";
		}
	}
	// csv File schließen
	intfErrCsv.close();
}


void WkmAusgabe::reportEndSystems()
{
	std::string sString = "SELECT dev_id,hostname,snmpLoc FROM device WHERE dataSource=2 ";
	sString += "AND dev_id > (SELECT MAX(dev_id) FROM rControl WHERE dev_id < (SELECT MAX(dev_id) FROM rControl));";
	std::vector<std::vector<std::string>> esRep = dieDB->query(sString.c_str());
	if (!esRep.empty())
	{
		// csv File öffnen
		std::string dateiname = ausgabeVz + "endSystemReport.csv";
		std::ofstream endgeraeteCsv(dateiname.c_str());
		endgeraeteCsv << "Hostname;IP Address;MAC Address;Vendor;Connected to Device;Interface\n";
		// Ausgabe
		schreibeLog(WkLog::WkLog_NORMALTYPE, "\nAll end systems connected to Access Ports on Switches\n", "", WkLog::WkLog_BLAU, WkLog::WkLog_FETT);
		schreibeLog(WkLog::WkLog_NORMALTYPE, boost::str(boost::format("%-7s%-40s%-17s%-17s%-50s%-40s%-30s\n") % "Nr." % "Hostname" % "IP Address" % "MAC Address" % "Vendor" % "Connected to Device" % "Interface"), "", WkLog::WkLog_BLAU);
		int nummer = 0;
		for(std::vector<std::vector<std::string>>::iterator it = esRep.begin(); it < esRep.end(); ++it)
		{
			sString = "SELECT macAddress,ipAddress FROM interfaces INNER JOIN devInterface ON devInterface.interfaces_int_id=interfaces.intf_id ";
			sString += "WHERE devInterface.device_dev_id=" + it->at(0);
			std::vector<std::vector<std::string>> esIntf = dieDB->query(sString.c_str());
			std::vector<std::vector<std::string>>::iterator it1 = esIntf.begin();
			if (esIntf.empty())
			{
				continue;
			}

			std::string nwDevice = "UNKNOWN";
			std::string nwInterface = "UNKNOWN";
			sString = "SELECT dI_dev_id,dI_intf_id FROM neighborship WHERE dI_dev_id1=" + it->at(0);
			std::vector<std::vector<std::string>> nRep = dieDB->query(sString.c_str());
			std::vector<std::vector<std::string>>::iterator it2 = nRep.begin();
			if (!nRep.empty())
			{
				sString = "SELECT intfName FROM interfaces WHERE intf_id=" + it2->at(1);
				std::vector<std::vector<std::string>> iRep = dieDB->query(sString.c_str());
				std::vector<std::vector<std::string>>::iterator it3 = iRep.begin();
				nwInterface = it3->at(0);

				sString = "SELECT hostname FROM device WHERE dev_id=" + it2->at(0);
				std::vector<std::vector<std::string>> hRep = dieDB->query(sString.c_str());
				std::vector<std::vector<std::string>>::iterator it4 = hRep.begin();
				nwDevice = it4->at(0);
			}


			nummer++;
			std::string ausgabeString = boost::str(boost::format("%-7s%-40s%-17s%-17s%-50s%-40s%-30s\n") % boost::lexical_cast<std::string>(nummer) % it->at(1) % it1->at(1) % it1->at(0) % it->at(2) % nwDevice % nwInterface);
			schreibeLog(WkLog::WkLog_NORMALTYPE, ausgabeString, "", WkLog::WkLog_SCHWARZ);

			endgeraeteCsv << it->at(1) << ";" << it1->at(1) << ";" << it1->at(0) << ";" << it->at(2) << ";" << nwDevice << ";" << nwInterface << "\n";
		}
		endgeraeteCsv.close();
	}
}


void WkmAusgabe::reportIPT()
{
	// csv File öffnen
	std::string dateiname = ausgabeVz + "ipTelReport.csv";
	std::ofstream iptCsv(dateiname.c_str());
	// Ausgabe
	schreibeLog(WkLog::WkLog_NORMALTYPE, "\nDetailled IP Phone Report\n", "", WkLog::WkLog_BLAU, WkLog::WkLog_FETT);
	schreibeLog(WkLog::WkLog_NORMALTYPE, boost::str(boost::format("%-5s%-10s%-17s%-17s%-15s%-13s%-20s%-10s%-17s%-17s%-10s%-10s%-30s\n") % "Nr." % "Extension" % "Hostname" % "IP Address" % "Device Type" % "S/N" % "SW Version" % "VoiceVLAN" % "CM 1" % "CM 2" % "DSCP Sig"  % "DSCP Call"  % "CDP Neighbor"), "", WkLog::WkLog_BLAU);
	std::string sString = "SELECT dev_id,snmpLoc,hostname FROM device ";

	iptCsv << "Extension;Hostname;IP Address;Device Type;S/N;SW Version;VoiceVLAN;CM 1;CM 2;DSCP Sig;DSCP Call;CDP Neighbor\n";

	sString += "WHERE hwType=11 AND dev_id > (SELECT MAX(dev_id) FROM rControl WHERE dev_id < (SELECT MAX(dev_id) FROM rControl));";

	std::vector<std::vector<std::string>> ippRep = dieDB->query(sString.c_str());
	int nummer = 0;
	for(std::vector<std::vector<std::string>>::iterator it = ippRep.begin(); it < ippRep.end(); ++it)
	{
		sString = "SELECT hwInfo.type,hwInfo.sn,hwInfo.sw_version FROM hwInfo INNER JOIN hwlink ON hwlink.hwInfo_hwinf_id=hwInfo.hwinf_id ";
		sString += "WHERE module LIKE 'box' AND hwlink.device_dev_id = " + it->at(0) + ";";
		std::vector<std::vector<std::string>> devHwRep = dieDB->query(sString.c_str());
		std::vector<std::vector<std::string>>::iterator it0 = devHwRep.begin();

		sString = "SELECT voiceVlan, cm1, cm2, dscpSig, dscpConf, dscpCall, defGW FROM ipPhoneDet ";
		sString += "INNER JOIN ipphonelink ON ipphonelink.ipPhoneDet_ipphone_id=ipPhoneDet.ipphone_id ";
		sString += "INNER JOIN device ON device.dev_id=ipphonelink.device_dev_id WHERE device.dev_id= " + it->at(0);
		std::vector<std::vector<std::string>> ippRep1 = dieDB->query(sString.c_str());
		std::vector<std::vector<std::string>>::iterator it1 = ippRep1.begin();

		sString = "SELECT ipAddress FROM interfaces ";
		sString += "INNER JOIN devInterface ON devInterface.interfaces_int_id=interfaces.intf_id ";
		sString += "INNER JOIN device ON device.dev_id=devInterface.device_dev_id WHERE device.dev_id= " + it->at(0);
		std::vector<std::vector<std::string>> intf = dieDB->query(sString.c_str());
		std::vector<std::vector<std::string>>::iterator it2 = intf.begin();

		sString = "SELECT nName,nIntf,platform FROM cdp INNER JOIN clink ON clink.cdp_cdp_id=cdp.cdp_id ";
		sString = "INNER JOIN interfaces ON interfaces.intf_id=clink.interfaces_intf_id INNER JOIN devInterface ON devInterface.interfaces_int_id=interfaces.intf_id ";
		sString += "WHERE devInterface.device_dev_id= " + it->at(0);
		std::vector<std::vector<std::string>> cdpNeigh = dieDB->query(sString.c_str());
		std::vector<std::vector<std::string>>::iterator cdpIt = cdpNeigh.begin();
		std::string cdpN = "";
		std::string cdpT = "";
		if (cdpIt < cdpNeigh.end())
		{
			cdpN = cdpIt->at(0) + " // " + cdpIt->at(1);
			cdpT = cdpIt->at(2);
		}


		nummer++;
		std::string ausgabeString = boost::str(boost::format("%-5s%-10s%-17s%-17s%-15s%-13s%-20s%-10s%-17s%-17s%-10s%-10s%-30s\n") % boost::lexical_cast<std::string>(nummer) % it->at(1) % it->at(2) % it2->at(0) % it0->at(0) % it0->at(1) % it0->at(2) % it1->at(0) % it1->at(1) % it1->at(2) % it1->at(3) % it1->at(5) % cdpN);
		schreibeLog(WkLog::WkLog_NORMALTYPE, ausgabeString, "", WkLog::WkLog_SCHWARZ);

		iptCsv << it->at(1) << ";" << it->at(2) << ";" << it2->at(0) << ";" << it0->at(0) << ";" << it0->at(1) << ";" << it0->at(2) << ";" << it1->at(0) << ";" << it1->at(1) << ";" << it1->at(2) << ";" << it1->at(3) << ";" << it1->at(5) << ";" << cdpN << "\n";
	}
	iptCsv.close();
}


void WkmAusgabe::reportInventory()
{
	// csv File öffnen
	std::string dateiname = ausgabeVz + "deviceReport.csv";
	std::ofstream deviceCsv(dateiname.c_str());
	std::size_t subNummer = 0;
	std::string position = "";
	std::string zeilenNummer = "";
	int farbe = WkLog::WkLog_SCHWARZ;
	// Ausgabe
	schreibeLog(WkLog::WkLog_NORMALTYPE, "\nDevices\n", "", WkLog::WkLog_BLAU, WkLog::WkLog_FETT);
	schreibeLog(WkLog::WkLog_NORMALTYPE, boost::str(boost::format("%-7s%-40s%-25s%-20s%-60s%-50s%-20s%-10s%-20s\n") % "Nr." % "Hostname" % "Device Type" % "S/N" % "Position" % "Description" % "SW Version" % "ConfReg" % "SNMP Location"), "", WkLog::WkLog_BLAU);
	std::string sString = "SELECT hostname,snmpLoc,dev_id,confReg FROM device ";
	sString += "WHERE dev_id > (SELECT MAX(dev_id) FROM rControl WHERE dev_id < (SELECT MAX(dev_id) FROM rControl));";
	deviceCsv << "Hostname;Device Type;S/N;Position;Description;SW Version;SNMP Location\n";

	std::vector<std::vector<std::string>> devRep = dieDB->query(sString.c_str());
	int nummer = 0;
	for(std::vector<std::vector<std::string>>::iterator it = devRep.begin(); it < devRep.end(); ++it)
	{
		subNummer = 0;
		sString = "SELECT hwInfo.type,hwInfo.sn,hwInfo.sw_version,hwInfo.module,hwInfo.description FROM hwInfo INNER JOIN hwlink ON hwlink.hwInfo_hwinf_id=hwInfo.hwinf_id ";
		sString += "WHERE hwlink.device_dev_id = " + it->at(2) + ";";

		std::vector<std::vector<std::string>> devHwRep = dieDB->query(sString.c_str());
		for(std::vector<std::vector<std::string>>::iterator it1 = devHwRep.begin(); it1 < devHwRep.end(); ++it1)
		{
			if (devHwRep.empty())
			{
				continue;
			}
			if (it1->at(3) == "box")
			{
				position = "Chassis";
				nummer++;
				zeilenNummer = boost::lexical_cast<std::string>(nummer);
				farbe = WkLog::WkLog_SCHWARZ;
			}
			else
			{
				position = it1->at(3);
				subNummer++;
				zeilenNummer = boost::lexical_cast<std::string>(nummer);
				zeilenNummer += "." + boost::lexical_cast<std::string>(subNummer);
				farbe = WkLog::WkLog_GRAU;
			}

			std::string ausgabeString = boost::str(boost::format("%-7s%-40s%-25s%-20s%-60s%-50s%-20s%-10s%-20s\n") % zeilenNummer % it->at(0) % it1->at(0) % it1->at(1) % position % it1->at(4) % it1->at(2) % it->at(3) % it->at(1));
			schreibeLog(WkLog::WkLog_NORMALTYPE, ausgabeString, "", farbe);

			deviceCsv << it->at(0) << ";" << it1->at(0) << ";" << it1->at(1) << ";" << position << ";" << it1->at(4) << ";" << it1->at(2) << ";" << it->at(3) << ";" << it->at(1) << "\n";
		}
	}
	deviceCsv.close();
}


void WkmAusgabe::reportSTP()
{
	// STP Transition Count
	//////////////////////////////////////////////////////////////////////////
	schreibeLog(WkLog::WkLog_NORMALTYPE, "\nInterfaces with STP Transition Count > 15\n", "", WkLog::WkLog_BLAU, WkLog::WkLog_FETT);
	schreibeLog(WkLog::WkLog_NORMALTYPE, boost::str(boost::format("%-5s%-25s%-10s%-15s\n") % "Nr." % "Hostname" % "Interface" % "STP TransCnt"), "", WkLog::WkLog_BLAU);

	std::string sString = "SELECT DISTINCT intf_id,intfName FROM interfaces	INNER JOIN int_vlan ON int_vlan.interfaces_intf_id=interfaces.intf_id ";
	sString += "INNER JOIN stp_status ON int_vlan.stp_status_stp_status_id=stp_status.stp_status_id WHERE stp_status.stpTransitionCount>15 ";
	sString += "AND intf_id > (SELECT MAX(intf_id) FROM rControl WHERE intf_id < (SELECT MAX(intf_id) FROM rControl));";

	std::vector<std::vector<std::string>> stp = dieDB->query(sString.c_str());

	int nummer = 0;
	for(std::vector<std::vector<std::string>>::iterator it = stp.begin(); it < stp.end(); ++it)
	{
		sString = "SELECT MAX (stpTransitionCount) FROM stp_status ";
		sString += "INNER JOIN int_vlan ON int_vlan.stp_status_stp_status_id=stp_status.stp_status_id ";
		sString += "INNER JOIN interfaces ON int_vlan.interfaces_intf_id=interfaces_intf_id ";
		sString += "WHERE intf_id=" + it->at(0);
		std::vector<std::vector<std::string>> stp1 = dieDB->query(sString.c_str());
		for(std::vector<std::vector<std::string>>::iterator it1 = stp1.begin(); it1 < stp1.end(); ++it1)
		{
			nummer++;
			sString = "SELECT hostname FROM device INNER JOIN devInterface ON devInterface.device_dev_id=device.dev_id ";
			sString += "INNER JOIN interfaces ON interfaces.intf_id=devInterface.interfaces_int_id WHERE intf_id=" + it1->at(0);
			std::vector<std::vector<std::string>> stp2 = dieDB->query(sString.c_str());
			std::vector<std::vector<std::string>>::iterator it2 = stp2.begin();

			if (it2 != stp2.end())
			{
				std::string ausgabeString = boost::str(boost::format("%-5s%-25s%-10s%-15s\n") % boost::lexical_cast<std::string>(nummer) % it2->at(0) % it->at(1) % it1->at(0));
				schreibeLog(WkLog::WkLog_NORMALTYPE, ausgabeString, "", WkLog::WkLog_SCHWARZ);
			}
		}
	}
}


void WkmAusgabe::reportL2Devs()
{
	// Detailierter L2 Report
	//////////////////////////////////////////////////////////////////////////
	std::string dateiname = ausgabeVz + "l2Detail.csv";
	std::ofstream l2Det(dateiname.c_str());
	l2Det << "Hostname;SNMP Location;Device Type;S/N;SW Version;Bridge-ID;STP Protocol;#STP Blocking Vlans;#Ports with STP Transition Count > 15;#Ports with Errors;#Ports with Errors > 100;#Total L2 Ports;#Port Active;#ErrorDisabled Ports;#Never Input Ports;#Active PoE Ports;PoE Current; PoE Max; PoE Remaining; dot1x Status; #dot1x enabled ports; #dotx1 disabled ports;Root for VLANS;Bootfile\n";
	schreibeLog(WkLog::WkLog_NORMALTYPE, "\nDetailled L2 Device Report\n", "", WkLog::WkLog_BLAU, WkLog::WkLog_FETT);
	schreibeLog(WkLog::WkLog_NORMALTYPE, boost::str(boost::format("%-5s%-40s%-30s%-25s%-15s%-20s%-16s%-10s%-10s%-10s%-10s%-10s%-10s%-10s%-10s%-10s%-10s%-12s%-12s%-12s%-15s%-10s%-10s%-40s%-50s\n") % "Nr." % "Hostname" % "SNMP Location" % "Device Type" % "S/N" % "SW Version" % "Bridge-ID" % "STP Prot." % "#STP B" % "#STP TC" % "#Err L" % "#Err H" % "#Ports" % "#Port A" % "#Err-D" % "#NUP" % "#PoE A" %  "PoE C" %  "PoE M" %  "PoE R" % "1xStat" % "#1xP" % "#1xIP" % "Root for VLANS" % "Bootfile"), "", WkLog::WkLog_BLAU);
	std::string sString = "SELECT DISTINCT dev_id,hostname,stpBridgeID,stpProtocol,snmpLoc,hwInfo.sn,hwInfo.type,hwInfo.bootfile,hwInfo.sw_version,poeCurrent,poeMax,poeRemaining,dot1xSysAuthCtrl FROM device ";
	sString += "INNER JOIN hwlink ON hwlink.device_dev_id=device.dev_id	INNER JOIN hwInfo ON hwInfo.hwinf_id=hwlink.hwInfo_hwinf_id ";
	sString += "WHERE stpProtocol NOT LIKE '' AND hwInfo.module LIKE 'box' ";
	sString += "AND dev_id > (SELECT MAX(dev_id) FROM rControl WHERE dev_id < (SELECT MAX(dev_id) FROM rControl));";
	std::vector<std::vector<std::string>> l2dRep = dieDB->query(sString.c_str());
	int nummer = 0;
	for(std::vector<std::vector<std::string>>::iterator it = l2dRep.begin(); it < l2dRep.end(); ++it)
	{
		sString = "SELECT COUNT (errLvl) FROM interfaces INNER JOIN devInterface ON devInterface.interfaces_int_id=interfaces.intf_id ";
		sString += "WHERE devInterface.device_dev_id=" + it->at(0) + " AND errLvl > 100";
		sString += " AND devInterface.device_dev_id > (SELECT MAX(dev_id) FROM rControl WHERE dev_id < (SELECT MAX(dev_id) FROM rControl));";
		std::vector<std::vector<std::string>> l2dE100 = dieDB->query(sString.c_str());
		std::vector<std::vector<std::string>>::iterator it1 = l2dE100.begin();
		sString = "SELECT COUNT (errLvl) FROM interfaces INNER JOIN devInterface ON devInterface.interfaces_int_id=interfaces.intf_id ";
		sString += "WHERE devInterface.device_dev_id=" + it->at(0) + " AND errLvl > 0";
		sString += " AND devInterface.device_dev_id > (SELECT MAX(dev_id) FROM rControl WHERE dev_id < (SELECT MAX(dev_id) FROM rControl));";
		std::vector<std::vector<std::string>> l2dE = dieDB->query(sString.c_str());
		std::vector<std::vector<std::string>>::iterator it2 = l2dE.begin();

		sString = "SELECT COUNT (stpTransitionCount) FROM stp_status INNER JOIN int_vlan ON int_vlan.stp_status_stp_status_id=stp_status.stp_status_id ";
		sString += "INNER JOIN interfaces ON interfaces.intf_id=int_vlan.interfaces_intf_id	INNER JOIN devInterface ON devInterface.interfaces_int_id=interfaces.intf_id ";
		sString += "WHERE devInterface.device_dev_id=" + it->at(0) + " AND stpTransitionCount > 15";
		sString += " AND devInterface.device_dev_id > (SELECT MAX(dev_id) FROM rControl WHERE dev_id < (SELECT MAX(dev_id) FROM rControl));";
		std::vector<std::vector<std::string>> l2dTC = dieDB->query(sString.c_str());
		std::vector<std::vector<std::string>>::iterator it3 = l2dTC.begin();

		sString = "SELECT COUNT (stpIntfStatus) FROM stp_status INNER JOIN int_vlan ON int_vlan.stp_status_stp_status_id=stp_status.stp_status_id ";
		sString += "INNER JOIN interfaces ON interfaces.intf_id=int_vlan.interfaces_intf_id	INNER JOIN devInterface ON devInterface.interfaces_int_id=interfaces.intf_id ";
		sString += "WHERE devInterface.device_dev_id=" + it->at(0) + " AND stpIntfStatus LIKE '%locking%'";
		sString += " AND devInterface.device_dev_id > (SELECT MAX(dev_id) FROM rControl WHERE dev_id < (SELECT MAX(dev_id) FROM rControl));";
		std::vector<std::vector<std::string>> l2dB = dieDB->query(sString.c_str());
		std::vector<std::vector<std::string>>::iterator it4 = l2dB.begin();

		std::string rootVlans = "";
		sString = "SELECT DISTINCT vlan FROM vlan ";
		sString += "INNER JOIN vlan_stpInstanz ON vlan_stpInstanz.vlan_vlan_id=vlan.vlan_id ";
		sString += "INNER JOIN stpInstanz ON stpInstanz.stp_id=vlan_stpInstanz.stpInstanz_stp_id ";
		sString += "INNER JOIN vlan_has_device ON vlan_has_device.vlan_vlan_id=vlan.vlan_id ";
		sString += "INNER JOIN device ON device.dev_id=vlan_has_device.device_dev_id ";
		sString += "WHERE rootPort LIKE '' AND device.dev_id=" + it->at(0);
		sString += " AND dev_id > (SELECT MAX(dev_id) FROM rControl WHERE dev_id < (SELECT MAX(dev_id) FROM rControl));";
		std::vector<std::vector<std::string>> l2dR = dieDB->query(sString.c_str());
		for(std::vector<std::vector<std::string>>::iterator it5 = l2dR.begin(); it5 < l2dR.end(); ++it5)
		{
			if (it5->at(0) != "")
			{
				rootVlans += it5->at(0) + ", ";
			}
		}

		sString = "SELECT COUNT(l2l3) FROM interfaces INNER JOIN devInterface ON devInterface.interfaces_int_id=interfaces.intf_id WHERE l2l3 LIKE 'L2' AND status LIKE 'UP'  ";
		sString += "AND phl=1 AND devInterface.device_dev_id=" + it->at(0);
		sString += " AND devInterface.device_dev_id > (SELECT MAX(dev_id) FROM rControl WHERE dev_id < (SELECT MAX(dev_id) FROM rControl));";
		std::vector<std::vector<std::string>> l2stat = dieDB->query(sString.c_str());
		std::vector<std::vector<std::string>>::iterator it6 = l2stat.begin();

		sString = "SELECT COUNT(l2l3) FROM interfaces INNER JOIN devInterface ON devInterface.interfaces_int_id=interfaces.intf_id WHERE l2l3 LIKE 'L2'  ";
		sString += "AND phl=1 AND devInterface.device_dev_id=" + it->at(0);
		sString += " AND devInterface.device_dev_id > (SELECT MAX(dev_id) FROM rControl WHERE dev_id < (SELECT MAX(dev_id) FROM rControl));";
		std::vector<std::vector<std::string>> l2tot = dieDB->query(sString.c_str());
		std::vector<std::vector<std::string>>::iterator it7 = l2tot.begin();

		sString = "SELECT COUNT(l2l3) FROM interfaces INNER JOIN devInterface ON devInterface.interfaces_int_id=interfaces.intf_id WHERE l2l3 LIKE 'L2' AND status LIKE 'ERR-DISABLED'  ";
		sString += "AND phl=1 AND devInterface.device_dev_id=" + it->at(0);
		sString += " AND devInterface.device_dev_id > (SELECT MAX(dev_id) FROM rControl WHERE dev_id < (SELECT MAX(dev_id) FROM rControl));";
		std::vector<std::vector<std::string>> l2ed = dieDB->query(sString.c_str());
		std::vector<std::vector<std::string>>::iterator it8 = l2ed.begin();

		sString = "SELECT COUNT(l2l3) FROM interfaces INNER JOIN devInterface ON devInterface.interfaces_int_id=interfaces.intf_id WHERE l2l3 LIKE 'L2' AND lastInput LIKE '%ever%' ";
		sString += "AND phl=1 AND devInterface.device_dev_id=" + it->at(0);
		sString += " AND devInterface.device_dev_id > (SELECT MAX(dev_id) FROM rControl WHERE dev_id < (SELECT MAX(dev_id) FROM rControl));";
		std::vector<std::vector<std::string>> l2nu = dieDB->query(sString.c_str());
		std::vector<std::vector<std::string>>::iterator it9 = l2nu.begin();

		sString = "SELECT COUNT(l2l3) FROM interfaces INNER JOIN devInterface ON devInterface.interfaces_int_id=interfaces.intf_id WHERE poeStatus LIKE 'on' ";
		sString += "AND devInterface.device_dev_id=" + it->at(0);
		std::vector<std::vector<std::string>> l2poe = dieDB->query(sString.c_str());
		std::vector<std::vector<std::string>>::iterator it10 = l2poe.begin();

		sString = "SELECT COUNT(l2l3) FROM interfaces INNER JOIN devInterface ON devInterface.interfaces_int_id=interfaces.intf_id WHERE dot1x NOT LIKE 'OFF' ";
		sString += "AND devInterface.device_dev_id=" + it->at(0);
		std::vector<std::vector<std::string>> l2dot1xEnabled = dieDB->query(sString.c_str());
		std::vector<std::vector<std::string>>::iterator it11 = l2dot1xEnabled.begin();

		sString = "SELECT COUNT(l2l3) FROM interfaces INNER JOIN devInterface ON devInterface.interfaces_int_id=interfaces.intf_id WHERE dot1x LIKE 'OFF' ";
		sString += "AND devInterface.device_dev_id=" + it->at(0);
		std::vector<std::vector<std::string>> l2dot1xDisabled = dieDB->query(sString.c_str());
		std::vector<std::vector<std::string>>::iterator it12 = l2dot1xDisabled.begin();


		nummer++;
		std::string ausgabeString = boost::str(boost::format("%-5s%-40s%-30s%-25s%-15s%-20s%-16s%-10s%-10s%-10s%-10s%-10s%-10s%-10s%-10s%-10s%-10s%-12s%-12s%-12s%-15s%-10s%-10s%-40s%-50s\n") % boost::lexical_cast<std::string>(nummer) % it->at(1) % it->at(4) % it->at(6) % it->at(5) % it->at(8) % it->at(2) % it->at(3) % it4->at(0) % it3->at(0) % it2->at(0) % it1->at(0) % it7->at(0) % it6->at(0) % it8->at(0) % it9->at(0) % it10->at(0) % it->at(9) % it->at(10) % it->at(11) % it->at(12) % it11->at(0) % it12->at(0) % rootVlans % it->at(7));
		schreibeLog(WkLog::WkLog_NORMALTYPE, ausgabeString, "", WkLog::WkLog_SCHWARZ);

		l2Det << it->at(1) << ";" << it->at(4) << ";" << it->at(6) << ";" << it->at(5) << ";" << it->at(8) << ";" << it->at(2) << ";" << it->at(3) << ";" << it4->at(0) << ";" << it3->at(0) << ";" << it2->at(0) << ";" << it1->at(0) << ";" << it7->at(0) << ";" << it6->at(0) << ";" << it8->at(0) << ";" << it9->at(0) << ";" << it10->at(0) << ";" << it->at(9) << ";" << it->at(10) << ";" << it->at(11) << ";" << it->at(12) << ";" << it11->at(0) << ";" << it12->at(0) << ";" << rootVlans << ";" << it->at(7) << "\n";
	}
	l2Det.close();
	schreibeLog(WkLog::WkLog_NORMALTYPE, "\nCaption:\n", "", WkLog::WkLog_BLAU);
	schreibeLog(WkLog::WkLog_NORMALTYPE, "#STP B...# STP Blocking Ports/Vlans -> there may be several blocking Vlans per Port\n", "", WkLog::WkLog_SCHWARZ);
	schreibeLog(WkLog::WkLog_NORMALTYPE, "#STP TC..# STP Topology Changes\n", "", WkLog::WkLog_SCHWARZ);
	schreibeLog(WkLog::WkLog_NORMALTYPE, "#Err L...# Ports with more than 100 errors\n", "", WkLog::WkLog_SCHWARZ);
	schreibeLog(WkLog::WkLog_NORMALTYPE, "#Err H...# Ports with more than 1000 errors\n", "", WkLog::WkLog_SCHWARZ);
	schreibeLog(WkLog::WkLog_NORMALTYPE, "#Ports...# Total L2 Ports\n", "", WkLog::WkLog_SCHWARZ);
	schreibeLog(WkLog::WkLog_NORMALTYPE, "#Port A..# Áctive L2 Ports\n", "", WkLog::WkLog_SCHWARZ);
	schreibeLog(WkLog::WkLog_NORMALTYPE, "#Err-D...# Error Disabled Ports\n", "", WkLog::WkLog_SCHWARZ);
	schreibeLog(WkLog::WkLog_NORMALTYPE, "#NUP.....# Never Used L2 Ports (Last Input)\n", "", WkLog::WkLog_SCHWARZ);
	schreibeLog(WkLog::WkLog_NORMALTYPE, "#PoE A...# PoE active Ports\n", "", WkLog::WkLog_SCHWARZ);
	schreibeLog(WkLog::WkLog_NORMALTYPE, "PoE C...PoE current used Watts\n", "", WkLog::WkLog_SCHWARZ);
	schreibeLog(WkLog::WkLog_NORMALTYPE, "PoE M...PoE Maximum Watts\n", "", WkLog::WkLog_SCHWARZ);
	schreibeLog(WkLog::WkLog_NORMALTYPE, "PoE R...PoE Remaining Watts\n", "", WkLog::WkLog_SCHWARZ);
	schreibeLog(WkLog::WkLog_NORMALTYPE, "1xStat..802.1X Global device status\n", "", WkLog::WkLog_SCHWARZ);
	schreibeLog(WkLog::WkLog_NORMALTYPE, "#1xP....# of L2 ports where 802.1X is enabled\n", "", WkLog::WkLog_SCHWARZ);
	schreibeLog(WkLog::WkLog_NORMALTYPE, "#1xIP...# of L2 ports where 802.1X is disabled\n", "", WkLog::WkLog_SCHWARZ);
}


void WkmAusgabe::reportL2()
{
	// L2 Reports
	//////////////////////////////////////////////////////////////////////////
	schreibeLog(WkLog::WkLog_NORMALTYPE, "\nNoticeable Switchport Settings\n", "", WkLog::WkLog_BLAU, WkLog::WkLog_FETT);
	schreibeLog(WkLog::WkLog_NORMALTYPE, "Lists all Interfaces where Admin Mode != Operational Mode or Allowed Vlans != ALL\n", "", WkLog::WkLog_BLAU, WkLog::WkLog_NORMALFORMAT);
	schreibeLog(WkLog::WkLog_NORMALTYPE, boost::str(boost::format("%-5s%-25s%-10s%-20s%-20s%-15s\n") % "Nr." % "Hostname" % "Interface" % "AdminMode" % "OperationalMode" % "Allowed Vlan"), "", WkLog::WkLog_BLAU);

	std::string sString = "SELECT intf_id,intfName,l2AdminMode,l2Mode,allowedVlan FROM interfaces WHERE (l2AdminMode != l2Mode AND l2Mode !='down' AND l2AdminMode != '' OR (allowedVlan != 'ALL' AND allowedVlan !='')) ";
	sString += "AND intf_id > (SELECT MAX(intf_id) FROM rControl WHERE intf_id < (SELECT MAX(intf_id) FROM rControl));";

	std::vector<std::vector<std::string>> l2r1 = dieDB->query(sString.c_str());
	int nummer = 0;
	for(std::vector<std::vector<std::string>>::iterator it = l2r1.begin(); it < l2r1.end(); ++it)
	{
		// Manchmal sind zwar Op und Admin Mode gleich, werden aber unerschiedlich angezeigt (bei Trunk); Dann überspringen
		if ((it->at(2).find("trunk")!= it->at(2).npos) && (it->at(3).find("trunk")!= it->at(3).npos))
		{
			continue;
		}
		// Manchmal sind zwar Op und Admin Mode gleich, werden aber unerschiedlich angezeigt (in dem Fall bei Access); Dann überspringen
		if ((it->at(2).find("access")!= it->at(2).npos) && (it->at(3).find("access")!= it->at(3).npos))
		{
			continue;
		}

		nummer++;
		sString = "SELECT hostname FROM device INNER JOIN devInterface ON devInterface.device_dev_id=device.dev_id ";
		sString += "INNER JOIN interfaces ON interfaces.intf_id=devInterface.interfaces_int_id WHERE intf_id=" + it->at(0);
		std::vector<std::vector<std::string>> l2r2 = dieDB->query(sString.c_str());
		std::vector<std::vector<std::string>>::iterator it2 = l2r2.begin();

		if (it2 != l2r2.end())
		{
			std::string ausgabeString = boost::str(boost::format("%-5s%-25s%-10s%-20s%-20s%-15s\n") % boost::lexical_cast<std::string>(nummer) % it2->at(0) % it->at(1) % it->at(2) % it->at(3) % it->at(4));
			schreibeLog(WkLog::WkLog_NORMALTYPE, ausgabeString, "", WkLog::WkLog_SCHWARZ);
		}
	}
}


void WkmAusgabe::reportSTPStat()
{
	// Teil 3.1: STP Protokoll Check
	schreibeLog(WkLog::WkLog_NORMALTYPE, "\nSTP Protocol check\n", "", WkLog::WkLog_BLAU, WkLog::WkLog_FETT);
	schreibeLog(WkLog::WkLog_NORMALTYPE, boost::str(boost::format("%-5s%-10s%-10s%-50s\n") % "Nr." % "STP Prot" % "Count" % "Hosts"), "", WkLog::WkLog_BLAU);

	std::string sString = "SELECT stpProtocol, COUNT (stpProtocol) FROM device WHERE stpProtocol NOT LIKE '' GROUP BY stpProtocol";
	std::vector<std::vector<std::string>> stpProt = dieDB->query(sString.c_str());
	int nummer = 0;
	for(std::vector<std::vector<std::string>>::iterator it = stpProt.begin(); it < stpProt.end(); ++it)
	{
		std::string geraete = "";
		sString = "SELECT hostname FROM device WHERE stpProtocol LIKE '" + it->at(0) + "'";
		std::vector<std::vector<std::string>> hosts = dieDB->query(sString.c_str());
		for(std::vector<std::vector<std::string>>::iterator it1 = hosts.begin(); it1 < hosts.end(); ++it1)
		{
			geraete += it1->at(0) + ", ";
		}

		nummer++;

		std::string ausgabeString = boost::str(boost::format("%-5s%-10s%-10s%-50s\n") % boost::lexical_cast<std::string>(nummer) % it->at(0) % it->at(1) % geraete);
		schreibeLog(WkLog::WkLog_NORMALTYPE, ausgabeString, "", WkLog::WkLog_SCHWARZ);
	}
}


void WkmAusgabe::reportL3ospf()
{
	std::string zeilenNummer = "";
	int farbe = WkLog::WkLog_SCHWARZ;
	int nummer = 0;

	// Ausgabe
	schreibeLog(WkLog::WkLog_NORMALTYPE, "\nOSPF Process Details\n", "", WkLog::WkLog_BLAU, WkLog::WkLog_FETT);
	schreibeLog(WkLog::WkLog_NORMALTYPE, boost::str(boost::format("%-7s%-40s%-18s%-11s%-20s%-15s%-20s\n") % "Nr." % "Hostname" % "Router ID" % "ProcessID" % "vrf" % "SPF Last Exec" % "SPF Total Exec"), "", WkLog::WkLog_BLAU);

	std::string sString = "SELECT routerID,processID,vrf,spfLast,spfExecutions,hostname FROM ospf INNER JOIN ospflink ON ospflink.ospf_ospf_id=ospf_id INNER JOIN device ON ospflink.device_dev_id=device.dev_id ";
	sString += "WHERE ospf_id > (SELECT MAX(ospf_id) FROM rControl WHERE ospf_id < (SELECT MAX(ospf_id) FROM rControl));";

	std::vector<std::vector<std::string>> od = dieDB->query(sString.c_str());
	for (std::vector<std::vector<std::string>>::iterator odIt = od.begin(); odIt < od.end(); ++odIt)
	{
		nummer++;
		zeilenNummer = boost::lexical_cast<std::string>(nummer);

		std::string ausgabeString = boost::str(boost::format("%-7s%-40s%-18s%-11s%-20s%-15s%-20s\n") % zeilenNummer % odIt->at(5) % odIt->at(0) % odIt->at(1) % odIt->at(2) % odIt->at(3) % odIt->at(4));
		schreibeLog(WkLog::WkLog_NORMALTYPE, ausgabeString, "", farbe);
	}
}


void WkmAusgabe::reportL3RP()
{
	// csv File öffnen
	std::string dateiname = ausgabeVz + "routingProtocolReport.csv";
	std::ofstream rpCsv(dateiname.c_str());
	std::string zeilenNummer = "";
	int farbe = WkLog::WkLog_SCHWARZ;
	// L3 RP Ausgabe
	schreibeLog(WkLog::WkLog_NORMALTYPE, "\nRouting Protocol Overview\n", "", WkLog::WkLog_BLAU, WkLog::WkLog_FETT);
	schreibeLog(WkLog::WkLog_NORMALTYPE, boost::str(boost::format("%-7s%-40s%-9s%-9s%-9s%-9s%-9s%-9s%-9s%-9s\n") % "Nr." % "Hostname" % "#OSPF-N" % "#BGP-N" % "#EIGRP-N" % "RIP" % "#OSPF-I" % "#EIGRP-I" % "#RIP-I" % "BGP-AS#"), "", WkLog::WkLog_BLAU);
	std::string sString = "SELECT DISTINCT hostname,dev_id,bgpAS FROM device INNER JOIN devInterface ON devInterface.device_dev_id=device.dev_id ";
	sString += "INNER JOIN interfaces ON interfaces.intf_id=devInterface.interfaces_int_id INNER JOIN rpNeighborlink ON rpNeighborlink.interfaces_intf_id=interfaces.intf_id ";
	sString += "WHERE dev_id > (SELECT MAX(dev_id) FROM rControl WHERE dev_id < (SELECT MAX(dev_id) FROM rControl));";
	
	rpCsv << "Hostname;#OSPF Neigh;#BGP Neigh;#EIGRP Neigh;RIP Act;#OSPF Intf;#EIGRP Intf;#RIP Intf;BGP AS#\n";


	std::vector<std::vector<std::string>> devRep = dieDB->query(sString.c_str());
	int nummer = 0;
	for (std::vector<std::vector<std::string>>::iterator it = devRep.begin(); it < devRep.end(); ++it)
	{
		nummer++;
		zeilenNummer = boost::lexical_cast<std::string>(nummer);

		// # OSPF Nachbaren
		std::string ospfNachbaren = "";
		sString = "SELECT COUNT(rp) FROM rpNeighbor INNER JOIN rpNeighborlink ON rpNeighborlink.rpNeighbor_rpNeighbor_id=rpNeighbor.rpNeighbor_id ";
		sString += "INNER JOIN interfaces ON interfaces.intf_id=rpNeighborlink.interfaces_intf_id INNER JOIN devInterface ON interfaces.intf_id=devInterface.interfaces_int_id ";
		sString += "WHERE devInterface.device_dev_id = " + it->at(1) + " AND rpNeighbor.rp LIKE 'OSPF';";
		std::vector<std::vector<std::string>> onret = dieDB->query(sString.c_str());
		if (!onret.empty())
		{
			ospfNachbaren = onret[0][0];
		}
		// # BGP Nachbaren
		std::string bgpNachbaren = "";
		sString = "SELECT COUNT(rp) FROM rpNeighbor INNER JOIN rpNeighborlink ON rpNeighborlink.rpNeighbor_rpNeighbor_id=rpNeighbor.rpNeighbor_id ";
		sString += "INNER JOIN interfaces ON interfaces.intf_id=rpNeighborlink.interfaces_intf_id INNER JOIN devInterface ON interfaces.intf_id=devInterface.interfaces_int_id ";
		sString += "WHERE devInterface.device_dev_id = " + it->at(1) + " AND rpNeighbor.rp LIKE 'BGP';";
		std::vector<std::vector<std::string>> bnret = dieDB->query(sString.c_str());
		if (!bnret.empty())
		{
			bgpNachbaren = bnret[0][0];
		}
		// # EIGRP Nachbaren
		std::string eigrpNachbaren = "";
		sString = "SELECT COUNT(rp) FROM rpNeighbor INNER JOIN rpNeighborlink ON rpNeighborlink.rpNeighbor_rpNeighbor_id=rpNeighbor.rpNeighbor_id ";
		sString += "INNER JOIN interfaces ON interfaces.intf_id=rpNeighborlink.interfaces_intf_id INNER JOIN devInterface ON interfaces.intf_id=devInterface.interfaces_int_id ";
		sString += "WHERE devInterface.device_dev_id = " + it->at(1) + " AND rpNeighbor.rp LIKE 'EIGRP';";
		std::vector<std::vector<std::string>> enret = dieDB->query(sString.c_str());
		if (!enret.empty())
		{
			eigrpNachbaren = enret[0][0];
		}
		// # OSPF Interfaces
		std::string ospfIntf = "";
		sString = "SELECT COUNT(intf_id) FROM interfaces INNER JOIN ospfIntflink ON ospfIntflink.interfaces_intf_id=interfaces.intf_id ";
		sString += "INNER JOIN ospf ON ospf.ospf_id=ospfIntflink.ospf_ospf_id INNER JOIN ospflink ON ospflink.ospf_ospf_id=ospf.ospf_id ";
		sString += "WHERE ospflink.device_dev_id=" + it->at(1) + ";";
		std::vector<std::vector<std::string>> oiret = dieDB->query(sString.c_str());
		if (!enret.empty())
		{
			ospfIntf = oiret[0][0];
		}

		std::string ausgabeString = boost::str(boost::format("%-7s%-40s%-9s%-9s%-9s%-9s%-9s%-9s%-9s%-9s\n") % zeilenNummer % it->at(0) % ospfNachbaren % bgpNachbaren % eigrpNachbaren % "n/a" % ospfIntf % "n/a" % "n/a" % it->at(2));
		schreibeLog(WkLog::WkLog_NORMALTYPE, ausgabeString, "", farbe);

		rpCsv << it->at(0) << ";" << ospfNachbaren << ";" << bgpNachbaren << ";" << eigrpNachbaren << ";" << "n/a" << ";" << ospfIntf << ";" << "n/a" << ";" << "n/a" << ";" << it->at(2) << "\n";
	}
	schreibeLog(WkLog::WkLog_NORMALTYPE, "\nCaption:\n", "", WkLog::WkLog_BLAU);
	schreibeLog(WkLog::WkLog_NORMALTYPE, "#OSPF-N....# OSPF Neighbors\n", "", WkLog::WkLog_SCHWARZ);
	schreibeLog(WkLog::WkLog_NORMALTYPE, "#BGP-N.....# BGP Neighbors\n", "", WkLog::WkLog_SCHWARZ);
	schreibeLog(WkLog::WkLog_NORMALTYPE, "#EIGRP-N...# EIGRP Neighbors\n", "", WkLog::WkLog_SCHWARZ);
	schreibeLog(WkLog::WkLog_NORMALTYPE, "RIP........RIP active or inactive\n", "", WkLog::WkLog_SCHWARZ);
	schreibeLog(WkLog::WkLog_NORMALTYPE, "#OSPF-I....# OSPF Interfaces\n", "", WkLog::WkLog_SCHWARZ);
	schreibeLog(WkLog::WkLog_NORMALTYPE, "#EIGRP-I...# EIGRP Interfaces\n", "", WkLog::WkLog_SCHWARZ);
	schreibeLog(WkLog::WkLog_NORMALTYPE, "#RIP-I.....# RIP Interfaces\n", "", WkLog::WkLog_SCHWARZ);
	schreibeLog(WkLog::WkLog_NORMALTYPE, "BGP-AS#....Local BGP AS Number\n", "", WkLog::WkLog_SCHWARZ);
	
	rpCsv.close();
}


void WkmAusgabe::reportL3RPneighbors()
{
	// csv File öffnen
	std::string dateiname = ausgabeVz + "rpNeighborshipReport.csv";
	std::ofstream rpnCsv(dateiname.c_str());
	std::string zeilenNummer = "";
	int farbe = WkLog::WkLog_SCHWARZ;
	int nummer = 0;
	// Ausgabe
	schreibeLog(WkLog::WkLog_NORMALTYPE, "\nRouting Neighborships\n", "", WkLog::WkLog_BLAU, WkLog::WkLog_FETT);
	schreibeLog(WkLog::WkLog_NORMALTYPE, boost::str(boost::format("%-7s%-40s%-12s%-10s%-12s%-40s%-18s%-18s%-15s%-15s%-18s\n") % "Nr." % "Hostname" % "Interface" % "Protocol" % "State" % "Neighbor Hostname" % "Neighbor ID" % "Neighbor Address" % "#State Changes" % "Uptime" % "BGP Neighbor AS#"), "", WkLog::WkLog_BLAU);

	rpnCsv << "Hostname;Interface;Protocol;State;Neighbor Hostname;Neighbor ID;Neighbor Address; #State Changes;Uptime;BGP Neighbor AS#\n";

	std::string sString = "SELECT hostname,bgpAS,rp,rpNeighborState,rpNeighborStateChanges,rpNeighborId,rpNeighborAddress,rpNeighborIntf,rpNeighborAS,rpNeighborUp FROM rpNeighbor ";
	sString += "INNER JOIN rpNeighborlink ON rpNeighborlink.rpNeighbor_rpNeighbor_id=rpNeighbor.rpNeighbor_id INNER JOIN interfaces ON interfaces.intf_id=rpNeighborlink.interfaces_intf_id ";
	sString += "INNER JOIN devInterface ON devInterface.interfaces_int_id=interfaces.intf_id INNER JOIN device ON device.dev_id=devInterface.device_dev_id ";
	sString += "WHERE rpNeighbor_id > (SELECT MAX(rpNeighbor_id) FROM rControl WHERE rpNeighbor_id < (SELECT MAX(rpNeighbor_id) FROM rControl));";

	std::vector<std::vector<std::string>> rpn = dieDB->query(sString.c_str());
	for (std::vector<std::vector<std::string>>::iterator rpnIt = rpn.begin(); rpnIt < rpn.end(); ++rpnIt)
	{
		nummer++;
		zeilenNummer = boost::lexical_cast<std::string>(nummer);

		std::string neighborHost = "";
		sString = "SELECT hostname FROM device INNER JOIN devInterface ON devInterface.device_dev_id=device.dev_id ";
		sString += "INNER JOIN interfaces ON interfaces.intf_id=devInterface.interfaces_int_id WHERE interfaces.ipAddress LIKE '" + rpnIt->at(6) + "'";
		std::vector<std::vector<std::string>> rpneigh = dieDB->query(sString.c_str());
		if (!rpneigh.empty())
		{
			neighborHost = rpneigh[0][0];
		}
		
		std::string ausgabeString = boost::str(boost::format("%-7s%-40s%-12s%-10s%-12s%-40s%-18s%-18s%-15s%-15s%-18s\n") % zeilenNummer % rpnIt->at(0) % rpnIt->at(7) % rpnIt->at(2) % rpnIt->at(3) % neighborHost % rpnIt->at(5) % rpnIt->at(6) % rpnIt->at(4) % rpnIt->at(9) % rpnIt->at(8));
		schreibeLog(WkLog::WkLog_NORMALTYPE, ausgabeString, "", farbe);
		rpnCsv << rpnIt->at(0) << ";" << rpnIt->at(7) << ";" << rpnIt->at(2) << ";" << rpnIt->at(3) << ";" << neighborHost << ";" << rpnIt->at(5) << ";" << rpnIt->at(6) << ";" << rpnIt->at(4) << ";" << rpnIt->at(9) << ";" << rpnIt->at(8) << "\n";
	}

	rpnCsv.close();
}

void WkmAusgabe::reportDot1xIntfStat()
{
	std::string sString = "SELECT intf_id, dev_id, intfName, hostname, description, status, dot1xSysAuthCtrl, nativeAccess, lastInput, lastClear FROM interfaces INNER JOIN devInterface ON devInterface.interfaces_int_id = interfaces.intf_id ";
	sString += "INNER JOIN device ON device.dev_id = devInterface.device_dev_id WHERE device.dataSource IS NULL AND(dot1x LIKE 'OFF' OR mab LIKE 'OFF') AND l2Mode NOT LIKE '%trunk%' ";
	sString += "AND intf_id > (SELECT MAX(intf_id) FROM rControl WHERE intf_id < (SELECT MAX(intf_id) FROM rControl));";
	std::vector<std::vector<std::string>> dRep = dieDB->query(sString.c_str());
	if (!dRep.empty())
	{
		// csv File öffnen
		std::string dateiname = ausgabeVz + "dot1xReport.csv";
		std::ofstream dot1xCsv(dateiname.c_str());
		dot1xCsv << "Hostname;Interface;Status;Description;CDP Neighbor;CDP Neighbor Platform;Learned MAC Addresses;Corresponding IP;VLAN;Last Input;Last Clear\n";
		// Ausgabe
		schreibeLog(WkLog::WkLog_NORMALTYPE, "\nAll Non-Dot1x and Non-MAB Ports\n", "", WkLog::WkLog_BLAU, WkLog::WkLog_FETT);
		schreibeLog(WkLog::WkLog_NORMALTYPE, boost::str(boost::format("%-7s%-40s%-15s%-10s%-50s%-30s%-50s%-50s%-20s%-5s%-10s%-10s\n") % "Nr." % "Hostname" % "Interface" % "Status" % "Description" % "CDP Neighbor" % "CDP Neighbor Platform" % "Learned MAC Addresses" % "Corresponding IP" % "VLAN" % "LastInput" % "LastClear"), "", WkLog::WkLog_BLAU);
		int nummer = 0;
		for (std::vector<std::vector<std::string>>::iterator it = dRep.begin(); it < dRep.end(); ++it)
		{
			// Nur durchführen, wenn dot1x auf einem Gerät grundsätzlich aktiv ist
			if (it->at(6) == "Enabled")
			{
				// CDP Infos
				std::string cdpNeighbor = "";
				std::string cdpNeighDev = "";
				sString = "SELECT hostname, platform FROM cdp INNER JOIN clink ON clink.cdp_cdp_id = cdp.cdp_id INNER JOIN interfaces ON interfaces.intf_id = clink.interfaces_intf_id ";
				sString += "WHERE intf_id=" + it->at(0);

				std::vector<std::vector<std::string>> dIntf = dieDB->query(sString.c_str());
				std::vector<std::vector<std::string>>::iterator it1 = dIntf.begin();
				if (!dIntf.empty())
				{
					cdpNeighbor = it1->at(0);
					cdpNeighDev = it1->at(1);
				}


				// MAC Adressen
				std::string macAddresses = "";
				std::string ipAddresses = "";
				sString = "SELECT l2_addr FROM  neighbor INNER JOIN nlink ON nlink.neighbor_neighbor_id = neighbor.neighbor_id INNER JOIN interfaces ON interfaces.intf_id = nlink.interfaces_intf_id ";
				sString += "WHERE intf_id=" + it->at(0);

				std::vector<std::vector<std::string>> dMac = dieDB->query(sString.c_str());
				for (std::vector<std::vector<std::string>>::iterator it2 = dMac.begin(); it2 < dMac.end(); ++it2)
				{
					// MAC Adressen zusammenhängen
					macAddresses += it2->at(0) + " // ";

					// Zugehörige IP Adresse auslesen (falls verfügbar)
					sString = "SELECT l3_addr FROM neighbor WHERE l2_addr LIKE '" + it2->at(0) + "' "; 
					sString += "AND neighbor_id > (SELECT MAX(neighbor_id) FROM rControl WHERE neighbor_id < (SELECT MAX(neighbor_id) FROM rControl))" ;
					std::vector<std::vector<std::string>> dIP = dieDB->query(sString.c_str());
					if (!dIP.empty())
					{
						ipAddresses += dIP[0][0];
					}
					ipAddresses += " // ";
				}


				nummer++;
				std::string ausgabeString = boost::str(boost::format("%-7s%-40s%-15s%-10s%-50s%-30s%-50s%-50s%-20s%-5s%-10s%-10s\n") % boost::lexical_cast<std::string>(nummer) % it->at(3) % it->at(2) % it->at(5) % it->at(4) % cdpNeighbor % cdpNeighDev % macAddresses % ipAddresses % it->at(7) % it->at(8) % it->at(9));
				schreibeLog(WkLog::WkLog_NORMALTYPE, ausgabeString, "", WkLog::WkLog_SCHWARZ);

				dot1xCsv << it->at(3) << ";" << it->at(2) << ";" << it->at(5) << ";" << it->at(4) << ";" << cdpNeighbor << ";" << cdpNeighDev << ";" << macAddresses << ";" << ipAddresses << ";" << it->at(7) << ";" << it->at(8) << ";" << it->at(9) << "\n";
			}
		}
		dot1xCsv.close();
	}
}

void WkmAusgabe::reportDot1xFailed()
{
	// Report für alle Ports, die nicht erfolgreiche Authentications aufweisen
	
	std::string sString = "SELECT hostname,intfName,description,auth.status,auth.userMac FROM interfaces INNER JOIN intfAuth ON intfAuth.interfaces_intf_id = interfaces.intf_id ";
	sString += "INNER JOIN auth ON auth.auth_id = intfAuth.auth_auth_id INNER JOIN devInterface ON devInterface.interfaces_int_id = interfaces.intf_id ";
	sString += "INNER JOIN device ON device.dev_id = devInterface.device_dev_id WHERE auth.status NOT LIKE '%Success%' ";
	sString += "AND intf_id > (SELECT MAX(intf_id) FROM rControl WHERE intf_id < (SELECT MAX(intf_id) FROM rControl));";
	std::vector<std::vector<std::string>> dRep = dieDB->query(sString.c_str());
	if (!dRep.empty())
	{
		// csv File öffnen
		std::string dateiname = ausgabeVz + "dot1xFailedUserReport.csv";
		std::ofstream dot1xCsv(dateiname.c_str());
		dot1xCsv << "Hostname;Interface;Status;Description;MAC Addresses\n";
		// Ausgabe
		schreibeLog(WkLog::WkLog_NORMALTYPE, "\nAll Non-Dot1x and Non-MAB Ports\n", "", WkLog::WkLog_BLAU, WkLog::WkLog_FETT);
		schreibeLog(WkLog::WkLog_NORMALTYPE, boost::str(boost::format("%-7s%-40s%-15s%-10s%-50s%-15s\n") % "Nr." % "Hostname" % "Interface" % "Status" % "Description" % "MAC Addresses"), "", WkLog::WkLog_BLAU);
		int nummer = 0;
		for (std::vector<std::vector<std::string>>::iterator it = dRep.begin(); it < dRep.end(); ++it)
		{
			nummer++;
			std::string ausgabeString = boost::str(boost::format("%-7s%-40s%-15s%-10s%-50s%-15s\n") % boost::lexical_cast<std::string>(nummer) % it->at(0) % it->at(1) % it->at(2) % it->at(3) % it->at(4));
			schreibeLog(WkLog::WkLog_NORMALTYPE, ausgabeString, "", WkLog::WkLog_SCHWARZ);

			dot1xCsv << it->at(0) << ";" << it->at(1) << ";" << it->at(2) << ";" << it->at(3) << ";" << it->at(4) << "\n";
		}
		dot1xCsv.close();
	}
}

int WkmAusgabe::doItL3Routing()
{
	neighborEmpty = true;

	nodes = "";
	edges = "";
	clusters = "\nrootcluster [";

	// Hinweis zur Interface Farbkodierung
	// Interface Farbe:
	// 000000
	// 1|||||	...Interface: 0: Normal; 1: Muss kontrolliert werden; 2: Unknown Neigbor; 3: STP Blocking; 4: STP Mixed; 5: VPN
	//  2||||	...For Future Use
	//   3|||	...Error: 0: OK; 1: WARNING; 2: PROBLEM
	//    4||	...Load: 0: OK; 1: WARNING; 2: PROBLEM
	//     5|	...Duplex: 0: FULL; 1: HALF
	//      6	...STP Transitions: 0: OK; 1: WARNING; 2: PROBLEM

	// GML Rahmenstrings initialisieren
	std::string interfaceG_Neu_Start = "\ngraphics [\nfill \"#";																	// Normales Interface mit neuer Kodierung START
	std::string interfaceG_Neu_Ende = "\" \ntype \"rectangle\" \nw 100.0000000000 \nh 100.0000000000\n]";							// Normales Interface mit neuer Kodierung ENDE
	std::string interfaceFarbe = "000000";																							// Farbcode START

	std::string nodeStartG = "node [\nid ";
	std::string nodeLabelAG = "\nlabel \"";
	std::string nodeLabelEG = "\"";
	std::string nodeEndeG = "\n]\n";
	std::string edgeSourceG = "edge [\nsource ";
	std::string edgeTargetG = "\ntarget ";
	std::string edgeEndeG = "\n]\n";
	std::string clusterStart = "\ncluster [\nid ";
	std::string clusterVertexStart = "\nvertex \"";
	std::string clusterVertexEnd = "\"";
	std::string clusterEnd = "\n]";

	int nId = 0;		// Node ID
	int edgeId = 0;		// Edge ID
	int clusterId = 0;	// Cluster-ID
	int tClusterId = 10000;
	std::vector<std::string> clusterStrings;			// Cluster Strings, die dann zum Schluss zusammengefasst werden

	std::map<std::string, int> nIdsCluster;				// Sammlung aller Node IDs um mehrfache Einträge als Node zu verhindern und die Clusterobjekte richtig zuzuordnen
	std::map<std::string, int>::iterator nItCluster;	// Iterator für die Node/Cluster Sammlung
	typedef std::pair <std::string, int> psi;			// Pair für das Einfügen in die Map

	std::map<std::string, std::string> nIds;			// Sammlung aller Node IDs um mehrfache Einträge als Node zu verhindern
	std::map<std::string, std::string>::iterator nIt;	// Iterator für die Node Sammlung
	typedef std::pair <std::string, std::string> pss;	// Pair für das Einfügen in die Map

	std::multimap <std::string, std::string> IntfData;	// Sammlung aller Interfaces mit Fehlern und Warnungen
	std::multimap <std::string, std::string>::iterator IntfDataIter, IntfDataAnfang, IntfDataEnde;

	std::string nId1 = "";			// Node ID 1 -> für die Edge Beschreibung
	std::string nId2 = "";			// Node ID 2 -> für die Edge Beschreibung
	std::string nId3 = "";			// Node ID 3 -> für die Edge Beschreibung
	std::string nId4 = "";			// Node ID 4 -> für die Edge Beschreibung
	std::string nIDString = "";		// Temp Node ID als String

	std::string tempClusterId = "";	// Cluster ID für den Interface/Error Cluster

	// rpneighborship Tabelle auslesen
	std::string sString = "SELECT dI_dev_id,dI_intf_id,dI_dev_id1,dI_intf_id1,flag,rp FROM rpneighborship ";
	sString += "WHERE rpn_id > (SELECT MAX(rpn_id) FROM rControl WHERE rpn_id < (SELECT MAX(rpn_id) FROM rControl));";
	std::vector<std::vector<std::string>> devs = dieDB->query(sString.c_str());
	for (std::vector<std::vector<std::string>>::iterator it = devs.begin(); it < devs.end(); ++it)
	{
		bool direktOhneIntf = false;			// Direkte Verbindung der Nodes ohne Interfaces dazwischen
		bool intf1Existiert = false;
		bool intf2Existiert = false;
		// Falls der Eintrag nicht ausgegeben werden soll
		if (it->at(4) == "99")
		{
			continue;
		}

		// Interface Farbe zurücksetzen
		interfaceFarbe = "000000";

		sString = "SELECT device.hostname,device.type FROM device WHERE dev_id=" + it->at(0) + ";";
		std::vector<std::vector<std::string>> ret = dieDB->query(sString.c_str());
		std::vector<std::vector<std::string>>::iterator ti = ret.begin();
		if (ti == ret.end())
		{
			break;
		}
		neighborEmpty = false;

		std::string hh1 = ret[0][0];
		std::string h1 = it->at(0);
		std::string type1 = ret[0][1];

		nIt = nIds.find(it->at(0));
		if (nIt == nIds.end())
		{
			nIDString = boost::lexical_cast<std::string>(nId);
			nIdsCluster.insert(psi(it->at(0), clusterId));
			nIds.insert(pss(it->at(0), nIDString));
			nodes += nodeStartG + nIDString + nodeLabelAG + h1 + nodeLabelEG + setDevType(type1) + nodeEndeG;
			nId1 = nIDString;
			nId++;
			clusterStrings.push_back(clusterStart + boost::lexical_cast<std::string>(clusterId));
			clusterStrings[clusterId] += clusterVertexStart + nId1 + clusterVertexEnd;
			clusterId++;
		}
		else
		{
			nId1 = nIt->second;
		}

		std::string i1 = "UKNWN";
		if (it->at(1) != "0")
		{
			sString = "SELECT intfName, duplex, errLvl, loadLvl FROM interfaces WHERE interfaces.intf_id=" + it->at(1) + ";";
			ret = dieDB->query(sString.c_str());
			i1 = it->at(1) + "-" + ret[0][0];

			nIt = nIds.find(it->at(1));
			if (nIt == nIds.end())
			{
				intf1Existiert = false;

				nId2 = boost::lexical_cast<std::string>(nId);
				nIds.insert(pss(it->at(1), nId2));

				nId++;

				tempClusterId = boost::lexical_cast<std::string>(tClusterId);
				tClusterId++;

				nItCluster = nIdsCluster.find(it->at(0));
				clusterStrings[nItCluster->second] += clusterStart + tempClusterId;
				// Cluster Strings weiterbauen
				clusterStrings[nItCluster->second] += clusterVertexStart + nId2 + clusterVertexEnd;

				// Inneren Cluster abschließen
				clusterStrings[nItCluster->second] += clusterEnd;
			}
			else
			{
				nId2 = nIt->second;
				intf1Existiert = true;
			}

		}
		else
		{
			// Direkte Verbindung mit der Wolke
			direktOhneIntf = true;
			intf1Existiert = true;
			nId2 = boost::lexical_cast<std::string>(nId);
			nId++;
		}

		std::string h2 = "0";
		std::string hh2 = "0";
		std::string i2 = "0";

		if (it->at(2) != "0")
		{
			// Vom vorigen Edge...
			if (!intf1Existiert)
			{
				interfaceFarbe[0] = '0';
				nodes += nodeStartG + nId2 + nodeLabelAG + i1 + nodeLabelEG + interfaceG_Neu_Start + interfaceFarbe + interfaceG_Neu_Ende + nodeEndeG;
			}

			interfaceFarbe = "000000";


			sString = "SELECT device.hostname,device.type,dataSource FROM device WHERE dev_id=" + it->at(2) + ";";
			ret = dieDB->query(sString.c_str());
			h2 = it->at(2);
			hh2 = ret[0][0];
			std::string type2 = ret[0][1];
			std::string source = ret[0][2];		// Data Source -> i.e. CDP
			//if (source == "1")
			//{
			//	h2 += " (CDP)";
			//}

			nIt = nIds.find(it->at(2));
			if (nIt == nIds.end())
			{
				nIDString = boost::lexical_cast<std::string>(nId);
				nIdsCluster.insert(psi(it->at(2), clusterId));
				nIds.insert(pss(it->at(2), nIDString));
				nodes += nodeStartG + nIDString + nodeLabelAG + h2 + nodeLabelEG + setDevType(type2) + nodeEndeG;
				nId3 = nIDString;
				nId++;
				clusterStrings.push_back(clusterStart + boost::lexical_cast<std::string>(clusterId));
				clusterStrings[clusterId] += clusterVertexStart + nId3 + clusterVertexEnd;

				tempClusterId = boost::lexical_cast<std::string>(tClusterId);
				tClusterId++;

				clusterId++;
			}
			else
			{
				nId3 = nIt->second;
			}

			if (it->at(3) != "0")
			{
				sString = "SELECT intfName, duplex, errLvl, loadLvl FROM interfaces WHERE interfaces.intf_id=" + it->at(3) + ";";
				ret = dieDB->query(sString.c_str());
				i2 = it->at(3) + "-" + ret[0][0];

				nIt = nIds.find(it->at(3));
				if (nIt == nIds.end())
				{
					intf2Existiert = false;

					nId4 = boost::lexical_cast<std::string>(nId);
					nIds.insert(pss(it->at(3), nId4));

					nId++;

					if (ret[0][1].find("Half-duplex") != ret[0][1].npos)
					{
						interfaceFarbe[4] = '1';
					}


					if ((it->at(4) == "") || (it->at(4) == "0"))
					{
						interfaceFarbe[0] = '0';
					}
					else if (it->at(4) == "1")
					{
						interfaceFarbe[0] = '1';
					}
					else
					{
						interfaceFarbe[0] = '0';
					}
				}
				else
				{
					nId4 = nIt->second;
					intf2Existiert = true;
				}
			}
			else if (direktOhneIntf)
			{
				// Beide Interface-IDs sind "0" -> Direkte Verbindung der Device-Nodes ohne Interfaces dazwischen
				intf2Existiert = true;
			}
			else
			{
				nId4 = boost::lexical_cast<std::string>(nId);
				nId++;

				interfaceFarbe[0] = '0';
			}

			if (!intf2Existiert)
			{
				interfaceFarbe[5] = '2';
				nodes += nodeStartG + nId4 + nodeLabelAG + i2 + nodeLabelEG + interfaceG_Neu_Start + interfaceFarbe + interfaceG_Neu_Ende + nodeEndeG;
			}

			// Cluster Strings weiterbauen
			nItCluster = nIdsCluster.find(it->at(2));
			clusterStrings[nItCluster->second] += clusterVertexStart + nId4 + clusterVertexEnd;


			// Edges einfügen
			std::string edgeGraph1 = "";
			std::string edgeGraph2 = "";

			if (!direktOhneIntf)
			{
				if (!intf1Existiert)
				{
					edges += edgeSourceG + nId1 + edgeTargetG + nId2 + edgeGraph1 + edgeEndeG;
				}
				edges += edgeSourceG + nId2 + edgeTargetG + nId4 + edgeEndeG;
				if (!intf2Existiert)
				{
					edges += edgeSourceG + nId4 + edgeTargetG + nId3 + edgeGraph2 + edgeEndeG;
				}
			}
			else
			{
				edges += edgeSourceG + nId1 + edgeTargetG + nId3 + edgeGraph2 + edgeEndeG;
			}
		}
		else
		{
			interfaceFarbe[0] = '2';
			nodes += nodeStartG + nId2 + nodeLabelAG + i1 + nodeLabelEG + interfaceG_Neu_Start + interfaceFarbe + interfaceG_Neu_Ende + nodeEndeG;
			edges += edgeSourceG + nId1 + edgeTargetG + nId2 + edgeEndeG;
		}

		std::string nachbaren = hh1 + "-" + i1 + " <--> " + hh2 + "-" + i2 + "\n";
		schreibeLog(WkLog::WkLog_NORMALTYPE, nachbaren, "", WkLog::WkLog_SCHWARZ);

	}
	// Clusterstrings abschließen und zusammenführen
	for (int i=0; i<clusterStrings.size(); i++)
	{
		clusters += clusterStrings[i] += clusterEnd;
	}
	clusters += clusterEnd;

	if (!zeichneGraph("L3RP"))
	{
		schreibeLog(WkLog::WkLog_ZEIT, "6203: Unable to draw graph!\r\n", "6203",
			WkLog::WkLog_ROT, WkLog::WkLog_FETT);
	}
	return 1;
}

#endif // WKTOOLS_MAPPER


	
	
	
	

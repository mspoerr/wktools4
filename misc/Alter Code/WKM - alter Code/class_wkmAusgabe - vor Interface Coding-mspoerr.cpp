#include "class_wkmAusgabe.h"
#ifdef WKTOOLS_MAPPER

#include <ogdf/layered/OptimalHierarchyClusterLayout.h>
#include <ogdf/layered/SugiyamaLayout.h>
#include <ogdf/layered/OptimalRanking.h>
#include <ogdf/layered/MedianHeuristic.h>
#include <ogdf/energybased/FMMMLayout.h>
using namespace ogdf;

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <map>
using namespace std;

#include <boost/lexical_cast.hpp>


WkmAusgabe::WkmAusgabe(WkmDB *db, WkLog *logA, string asgbe)
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
}



WkmAusgabe::~WkmAusgabe()
{

}


int WkmAusgabe::doIt()
{
	int ret = 1;
	if (l2)
	{
		ret = doItL2();
	}

	if (l3)
	{
		hier = false;
		ret = doItL3();
	}

	return ret;
}


int WkmAusgabe::doItL2()
{
	markiereEintraege();

	// GML Rahmenstrings initialisieren
	string interfaceG = "\ngraphics [\nfill \"#faebd7\" \ntype \"rectangle\" \nw 50.0000000000 \nh 20.0000000000\n]";			// Normales Interface
	string interfaceGA = "\ngraphics [\nfill \"#4169e1\" \ntype \"rectangle\" \nw 50.0000000000 \nh 20.0000000000\n]";			// Interface, das kontrolliert werden muss (Anomaly)
	string interfaceGstpBlock = "\ngraphics [\nfill \"#696969\" \ntype \"rectangle\" \nw 50.0000000000 \nh 20.0000000000\n]";	// STP Blocking
	string interfaceGstpMix = "\ngraphics [\nfill \"#808080\" \ntype \"rectangle\" \nw 50.0000000000 \nh 20.0000000000\n]";		// STP Mixed
	string interfaceUnknownG = "\ngraphics [\nfill \"#a9a9a9\" \ntype \"oval\" \nw 50.0000000000 \nh 20.0000000000\n]";			// Unknown Neighbor, aber STP
	string interfaceErrorG = "\ngraphics [\nfill \"#ff0000\" \ntype \"oval\" \nw 20.0000000000 \nh 20.0000000000\n]";			// Fehler am Interface anzeigen
	string interfaceWarningG = "\ngraphics [\nfill \"#ffa500\" \ntype \"oval\" \nw 20.0000000000 \nh 20.0000000000\n]";			// Warnung am Interface anzeigen
	string processingError = "\ngraphics [\nfill \"#ff0000\" \ntype \"oval\" \nw 100.0000000000 \nh 100.0000000000\n]";			// Wenn es einen Logikfehler gibt
	string nodeStartG = "node [\nid ";
	string nodeLabelAG = "\nlabel \"";
	string nodeLabelEG = "\"";
	string nodeEndeG = "\n]\n";
	string edgeSourceG = "edge [\nsource ";
	string edgeTargetG = "\ntarget ";
	string edgeEndeG = "\n]\n";
	string clusterStart = "\ncluster [\nid ";
	string clusterVertexStart = "\nvertex \"";
	string clusterVertexEnd = "\"";
	string clusterEnd = "\n]";

	int nId = 0;		// Node ID
	int edgeId = 0;		// Edge ID
	int clusterId = 0;	// Cluster-ID
	int tClusterId = 10000;
	vector<string> clusterStrings;		// Cluster Strings, die dann zum Schluss zusammengefasst werden
	
	map<string,int> nIdsCluster;		// Sammlung aller Node IDs um mehrfache Einträge als Node zu verhindern und die Clusterobjekte richtig zuzuordnen
	map<string,int>::iterator nItCluster;		// Iterator für die Node/Cluster Sammlung
	typedef pair <string,int> psi;		// Pair für das Einfügen in die Map
	
	map<string,string> nIds;			// Sammlung aller Node IDs um mehrfache Einträge als Node zu verhindern
	map<string,string>::iterator nIt;		// Iterator für die Node Sammlung
	typedef pair <string,string> pss;		// Pair für das Einfügen in die Map
	
	multimap <string, string> IntfData;	// Sammlung aller Interfaces mit Fehlern und Warnungen
	multimap <string, string>::iterator IntfDataIter, IntfDataAnfang, IntfDataEnde;
	
	string nId1 = "";	// Node ID 1 -> für die Edge Beschreibung
	string nId2 = "";	// Node ID 2 -> für die Edge Beschreibung
	string nId3 = "";	// Node ID 3 -> für die Edge Beschreibung
	string nId4 = "";	// Node ID 4 -> für die Edge Beschreibung
	string nIDString = "";		// Temp Node ID als String

	string tempClusterId = "";	// Cluster ID für den Interface/Error Cluster

	bool blocking1 = false;		// Intf 1: STP Blocking?
	bool stpMix1 = false;		// Intf 1: Mehrere STP Stati auf einem Interface
	bool blocking2 = false;		// Intf 2: STP Blocking?
	bool stpMix2 = false;		// Intf 2: Mehrere STP Stati auf einem Interface


	// neighborship Tabelle auslesen
	string sString = "SELECT dI_dev_id,dI_intf_id,dI_dev_id1,dI_intf_id1,flag FROM neighborship;";
	vector<vector<string>> devs = dieDB->query(sString.c_str());
	for(vector<vector<string>>::iterator it = devs.begin(); it < devs.end(); ++it)
	{
		// Falls der Eintrag nicht ausgegeben werden soll
		if (it->at(4) == "99")
		{
			continue;
		}

		// STP Interface Infos zurücksetzen
		stpMix1 = stpMix2 = blocking1 = blocking2 = false;

		sString = "SELECT device.hostname,device.type FROM device WHERE dev_id=" + it->at(0) + ";";
		vector<vector<string>> ret = dieDB->query(sString.c_str());
		string h1 = ret[0][0];
		string type1 = ret[0][1];
		
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

		string i1 = "UNKNOWN";
		if (it->at(1) != "0")
		{
			// STP Status vom Interface feststellen und anzeigen
			sString = "SELECT DISTINCT stpIntfStatus FROM stp_status ";
			sString += "INNER JOIN int_vlan ON int_vlan.stp_status_stp_status_id=stp_status.stp_status_id ";
			sString += "INNER JOIN interfaces ON interfaces.intf_id=int_vlan.interfaces_intf_id ";
			sString += "WHERE interfaces.intf_id=" + it->at(1) + ";";
			ret = dieDB->query(sString.c_str());
			for(vector<vector<string>>::iterator itSTP = ret.begin(); itSTP < ret.end(); ++itSTP)
			{
				if (itSTP->at(0).find("ocking") != itSTP->at(0).npos)
				{
					blocking1 = true;
				}
			}
			if (ret.size() > 1)
			{
				stpMix1 = true;
			}
				
			
			sString = "SELECT intfName, duplex, errLvl, loadLvl FROM interfaces WHERE interfaces.intf_id=" + it->at(1) + ";";
			ret = dieDB->query(sString.c_str());
			i1 = ret[0][0];
			nId2 = boost::lexical_cast<std::string>(nId);
			nId++;

			tempClusterId = boost::lexical_cast<std::string>(tClusterId);
			tClusterId++;
			
			nItCluster = nIdsCluster.find(it->at(0));
			clusterStrings[nItCluster->second] += clusterStart + tempClusterId;
			// Cluster Strings weiterbauen
			clusterStrings[nItCluster->second] += clusterVertexStart + nId2 + clusterVertexEnd;

			if (ret[0][1].find("Half-duplex") != ret[0][1].npos)
			{
				nIDString = boost::lexical_cast<std::string>(nId);
				nodes += nodeStartG + nIDString + nodeLabelAG + "D" + nodeLabelEG + interfaceWarningG + nodeEndeG;
				IntfData.insert(pss(nId2, nIDString));
				clusterStrings[nItCluster->second] += clusterVertexStart + nIDString + clusterVertexEnd;
				nId++;
			}
			
			int err = 0;
			try
			{
				err = boost::lexical_cast<int>(ret[0][2]);
			}
			catch (boost::bad_lexical_cast &)
			{
				err = 0;			
			}
			
			if (err > 1000)
			{
				nIDString = boost::lexical_cast<std::string>(nId);
				nodes += nodeStartG + nIDString + nodeLabelAG + "E" + nodeLabelEG + interfaceErrorG + nodeEndeG;
				IntfData.insert(pss(nId2, nIDString));
				clusterStrings[nItCluster->second] += clusterVertexStart + nIDString + clusterVertexEnd;
				nId++;
			}
			else if (err > 10)
			{
				nIDString = boost::lexical_cast<std::string>(nId);
				nodes += nodeStartG + nIDString + nodeLabelAG + "E" + nodeLabelEG + interfaceWarningG + nodeEndeG;
				IntfData.insert(pss(nId2, nIDString));
				clusterStrings[nItCluster->second] += clusterVertexStart + nIDString + clusterVertexEnd;
				nId++;
			}
			
			
			int load = 0;
			try
			{
				load = boost::lexical_cast<int>(ret[0][3]);
			}
			catch (boost::bad_lexical_cast &)
			{
				load = 0;			
			}

			if (load > 200)
			{
				nIDString = boost::lexical_cast<std::string>(nId);
				nodes += nodeStartG + nIDString + nodeLabelAG + "L" + nodeLabelEG + interfaceErrorG + nodeEndeG;
				IntfData.insert(pss(nId2, nIDString));
				clusterStrings[nItCluster->second] += clusterVertexStart + nIDString + clusterVertexEnd;
				nId++;
			}
			else if (load > 100)
			{
				nIDString = boost::lexical_cast<std::string>(nId);
				nodes += nodeStartG + nIDString + nodeLabelAG + "L" + nodeLabelEG + interfaceWarningG + nodeEndeG;
				IntfData.insert(pss(nId2, nIDString));
				clusterStrings[nItCluster->second] += clusterVertexStart + nIDString + clusterVertexEnd;
				nId++;
			}
			
			// STP Transition Count auslesen
			sString = "SELECT stpTransitionCount FROM stp_status";
			sString += " INNER JOIN int_vlan ON int_vlan.stp_status_stp_status_id=stp_status.stp_status_id";
			sString += " INNER JOIN interfaces ON interfaces.intf_id=int_vlan.interfaces_intf_id";
			sString += " WHERE interfaces.intf_id=" + it->at(1);
			sString += " AND stpTransitionCount>15;";
				
			ret = dieDB->query(sString.c_str());
			if (!ret.empty())
			{
				nIDString = boost::lexical_cast<std::string>(nId);
				nodes += nodeStartG + nIDString + nodeLabelAG + "S" + nodeLabelEG + interfaceErrorG + nodeEndeG;
				IntfData.insert(pss(nId2, nIDString));
				clusterStrings[nItCluster->second] += clusterVertexStart + nIDString + clusterVertexEnd;
				nId++;
			}


			// Inneren Cluster abschließen
			clusterStrings[nItCluster->second] += clusterEnd;
			
		}
		else
		{
			nId2 = boost::lexical_cast<std::string>(nId);
			nId++;
		}

		string h2 = "UNKNOWN";
		string i2 = "UNKNOWN";

		if (it->at(2) != "0")
		{
			// Vom vorigen Edge...
			if ((it->at(4) == "") || (it->at(4) == "0"))
			{
				if (stpMix1)
				{
					nodes += nodeStartG + nId2 + nodeLabelAG + i1 + " (M)" + nodeLabelEG + interfaceGstpMix + nodeEndeG;
				}
				else if (blocking1)
				{
					nodes += nodeStartG + nId2 + nodeLabelAG + i1 + " (B)" + nodeLabelEG + interfaceGstpBlock + nodeEndeG;
				}
				else
				{
					nodes += nodeStartG + nId2 + nodeLabelAG + i1 + nodeLabelEG + interfaceG + nodeEndeG;
				}
			}
			else if (it->at(4) == "1")
			{
				nodes += nodeStartG + nId2 + nodeLabelAG + i1 + nodeLabelEG + interfaceGA + nodeEndeG;
			}
			else
			{
				nodes += nodeStartG + nId2 + nodeLabelAG + i1 + nodeLabelEG + processingError + nodeEndeG;
			}

			
			
			sString = "SELECT device.hostname,device.type,dataSource FROM device WHERE dev_id=" + it->at(2) + ";";
			ret = dieDB->query(sString.c_str());
			h2 = ret[0][0];
			string type2 = ret[0][1];
			string source = ret[0][2];		// Data Source -> i.e. CDP
			if (source == "1")
			{
				h2 += " (CDP)";
			}

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
				// STP Status vom Interface feststellen und anzeigen
				sString = "SELECT DISTINCT stpIntfStatus FROM stp_status ";
				sString += "INNER JOIN int_vlan ON int_vlan.stp_status_stp_status_id=stp_status.stp_status_id ";
				sString += "INNER JOIN interfaces ON interfaces.intf_id=int_vlan.interfaces_intf_id ";
				sString += "WHERE interfaces.intf_id=" + it->at(3) + ";";
				ret = dieDB->query(sString.c_str());
				for(vector<vector<string>>::iterator itSTP = ret.begin(); itSTP < ret.end(); ++itSTP)
				{
					if (itSTP->at(0).find("ocking") != itSTP->at(0).npos)
					{
						blocking2 = true;
					}
				}
				if (ret.size() > 1)
				{
					stpMix2 = true;
				}

				sString = "SELECT intfName, duplex, errLvl, loadLvl FROM interfaces WHERE interfaces.intf_id=" + it->at(3) + ";";
				ret = dieDB->query(sString.c_str());
				i2 = ret[0][0];
				nId4 = boost::lexical_cast<std::string>(nId);

				if (stpMix2)
				{
					nodes += nodeStartG + nId4 + nodeLabelAG + i2 + " (M)" + nodeLabelEG + interfaceGstpMix + nodeEndeG;
				}
				else if (blocking2)
				{
					nodes += nodeStartG + nId4 + nodeLabelAG + i2 + " (B)" + nodeLabelEG + interfaceGstpBlock + nodeEndeG;
				}
				else
				{
					nodes += nodeStartG + nId4 + nodeLabelAG + i2 + nodeLabelEG + interfaceG + nodeEndeG;
				}

				nId++;

				// Cluster Strings weiterbauen
				nItCluster = nIdsCluster.find(it->at(2));
				clusterStrings[nItCluster->second] += clusterStart + tempClusterId;
				clusterStrings[nItCluster->second] += clusterVertexStart + nId4 + clusterVertexEnd;


				if (ret[0][1].find("Half-duplex") != ret[0][1].npos)
				{
					nIDString = boost::lexical_cast<std::string>(nId);
					nodes += nodeStartG + nIDString + nodeLabelAG + "D" + nodeLabelEG + interfaceWarningG + nodeEndeG;
					IntfData.insert(pss(nId4, nIDString));
					clusterStrings[nItCluster->second] += clusterVertexStart + nIDString + clusterVertexEnd;
					nId++;
				}

				int err = 0;
				try
				{
					err = boost::lexical_cast<int>(ret[0][2]);
				}
				catch (boost::bad_lexical_cast &)
				{
					err = 0;			
				}

				if (err > 1000)
				{
					nIDString = boost::lexical_cast<std::string>(nId);
					nodes += nodeStartG + nIDString + nodeLabelAG + "E" + nodeLabelEG + interfaceErrorG + nodeEndeG;
					IntfData.insert(pss(nId4, nIDString));
					clusterStrings[nItCluster->second] += clusterVertexStart + nIDString + clusterVertexEnd;
					nId++;
				}
				else if (err > 10)
				{
					nIDString = boost::lexical_cast<std::string>(nId);
					nodes += nodeStartG + nIDString + nodeLabelAG + "E" + nodeLabelEG + interfaceWarningG + nodeEndeG;
					IntfData.insert(pss(nId4, nIDString));
					clusterStrings[nItCluster->second] += clusterVertexStart + nIDString + clusterVertexEnd;
					nId++;
				}
				int load = 0;
				try
				{
					load = boost::lexical_cast<int>(ret[0][3]);
				}
				catch (boost::bad_lexical_cast &)
				{
					load = 0;			
				}

				if (load > 200)
				{
					nIDString = boost::lexical_cast<std::string>(nId);
					nodes += nodeStartG + nIDString + nodeLabelAG + "L" + nodeLabelEG + interfaceErrorG + nodeEndeG;
					IntfData.insert(pss(nId4, nIDString));
					clusterStrings[nItCluster->second] += clusterVertexStart + nIDString + clusterVertexEnd;
					nId++;
				}
				else if (load > 100)
				{
					nIDString = boost::lexical_cast<std::string>(nId);
					nodes += nodeStartG + nIDString + nodeLabelAG + "L" + nodeLabelEG + interfaceWarningG + nodeEndeG;
					IntfData.insert(pss(nId4, nIDString));
					clusterStrings[nItCluster->second] += clusterVertexStart + nIDString + clusterVertexEnd;
					nId++;
				}
				clusterStrings[nItCluster->second] += clusterEnd;
			}
			else
			{
				nId4 = boost::lexical_cast<std::string>(nId);
				nId++;
				
				if (it->at(4) == "")
				{
					nodes += nodeStartG + nId4 + nodeLabelAG + i2 + nodeLabelEG + interfaceG + nodeEndeG;
				}
				else if (it->at(4) == "1")
				{
					nodes += nodeStartG + nId4 + nodeLabelAG + i2 + nodeLabelEG + interfaceGA + nodeEndeG;
				}


				// Cluster Strings weiterbauen
				nItCluster = nIdsCluster.find(it->at(2));
				clusterStrings[nItCluster->second] += clusterVertexStart + nId4 + clusterVertexEnd;
			}

			// Edges einfügen
			string edgeGraph1 = "";
			string edgeGraph2 = "";
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
			edges += edgeSourceG + nId1 + edgeTargetG + nId2 + edgeGraph1 + edgeEndeG;
			edges += edgeSourceG + nId2 + edgeTargetG + nId4 + edgeEndeG;
			edges += edgeSourceG + nId4 + edgeTargetG + nId3 + edgeGraph2 + edgeEndeG;
		}
		else
		{
			nodes += nodeStartG + nId2 + nodeLabelAG + i1 + nodeLabelEG + interfaceUnknownG + nodeEndeG;
			edges += edgeSourceG + nId1 + edgeTargetG + nId2 + edgeEndeG;
		}
		

		// Fehlerdaten Edges einfügen
		IntfDataIter = IntfData.begin();
		while (IntfDataIter != IntfData.end())
		{
			edges += edgeSourceG + IntfDataIter->first + edgeTargetG + IntfDataIter->second + edgeEndeG;
			IntfDataIter++;
		}
		IntfData.clear();

		string nachbaren = h1 + "-" + i1 + " <--> " + h2 + "-" + i2 + "\n";
		schreibeLog(WkLog::WkLog_NORMALTYPE, nachbaren, WkLog::WkLog_SCHWARZ);

	}
	// Clusterstrings abschließen und zusammenführen
	for (int i=0; i<clusterStrings.size(); i++)
	{
		clusters += clusterStrings[i] += clusterEnd;
	}
	clusters += clusterEnd;

	if (!zeichneGraph("L2"))
	{
		schreibeLog(WkLog::WkLog_ZEIT, "6203: Unable to draw graph!\r\n", 
			WkLog::WkLog_ROT, WkLog::WkLog_FETT);
	}
	return 1;
}


void WkmAusgabe::schreibeLog(int type, string logEintrag, int farbe, int format, int groesse)
{
	if (logAusgabe != NULL)
	{
		WkLog::evtData evtDat;
		evtDat.farbe = farbe;
		evtDat.format = format;
		evtDat.groesse = groesse;
		evtDat.logEintrag = logEintrag;
		evtDat.type = type;

		LogEvent evt(EVT_LOG_MELDUNG);
		evt.SetData(evtDat);
		wxPostEvent(logAusgabe, evt);
	}
}


int WkmAusgabe::zeichneGraph(string filename)
{
	Graph G;
	GraphAttributes GA(G,
		GraphAttributes::nodeGraphics | GraphAttributes::edgeGraphics | GraphAttributes::edgeType | GraphAttributes::edgeArrow |
		GraphAttributes::nodeLabel | GraphAttributes::nodeColor | 
		GraphAttributes::edgeColor | GraphAttributes::edgeStyle | 
		GraphAttributes::nodeStyle | GraphAttributes::nodeTemplate);

	ClusterGraph CG(G);
	ClusterGraphAttributes CGA(CG, 		
		GraphAttributes::nodeGraphics | GraphAttributes::edgeGraphics | GraphAttributes::edgeType | GraphAttributes::edgeArrow |
		GraphAttributes::nodeLabel | GraphAttributes::nodeColor | 
		GraphAttributes::edgeColor | GraphAttributes::edgeStyle | 
		GraphAttributes::nodeStyle | GraphAttributes::nodeTemplate);


	string sg = "directed 0 graph [\n";
	sg += nodes;
	sg += edges;
	sg += "]";
	sg += clusters;
	
	istringstream iss;
	iss.str(sg);

	schreibeLog(WkLog::WkLog_NORMALTYPE, sg, WkLog::WkLog_SCHWARZ);
//	Sleep(5000);

	if( !CGA.readClusterGML(iss,CG,G))
	{
		schreibeLog(WkLog::WkLog_ZEIT, "6202: Internal GML error!\r\n", 
			WkLog::WkLog_ROT, WkLog::WkLog_FETT);
		return 0;
	}


	// Ausgabe Streams und Filenamen
	ostringstream oss1 (ostringstream::out);
	ostringstream oss2 (ostringstream::out);
	string af1 = ausgabe + "hierarchical-" + filename + ".gml";
	string af2 = ausgabe + "organic-" + filename + ".gml";

	size_t pos = 0;

	if (hier)
	{
		// Hierarchical Layout
		SugiyamaLayout SL;
		SL.setRanking(new OptimalRanking);
		SL.setCrossMin(new MedianHeuristic);

		OptimalHierarchyClusterLayout *ohl = new OptimalHierarchyClusterLayout;
		ohl->layerDistance(15.0);
		ohl->nodeDistance(15.0);
		ohl->weightBalancing(0.1);
		SL.setClusterLayout(ohl);
		SL.call(CGA);

		//edge e;
		//ostringstream bla;
		//forall_edges(e,G)
		//{
		//	CGA.colorEdge(e) = "#4169e1";
		//	CGA.styleEdge(e) = ClusterGraphAttributes::EdgeStyle::esDash;
		//	CGA.arrowEdge(e) = ClusterGraphAttributes::EdgeArrow::last;
		//	bla << e << endl;
		//}
		//string blabla = bla.str();
		//schreibeLog(WkLog::WkLog_NORMALTYPE, blabla, WkLog::WkLog_SCHWARZ);
		
		CGA.writeGML(oss1);
		string str1 = oss1.str();

		ofstream ausgabe1(af1.c_str(), ios_base::out);
		ausgabe1 << str1;
		ausgabe1.close();
	}
	
	if (org)
	{
		// Organic Layout
		FMMMLayout fmmm;

		fmmm.useHighLevelOptions(true);
		fmmm.unitEdgeLength(150.0); 
		fmmm.qualityVersusSpeed(FMMMLayout::qvsGorgeousAndEfficient);

		fmmm.call(CGA);
		CGA.writeGML(oss2);

		string str2 = oss2.str();
		pos = 0;

		ofstream ausgabe2(af2.c_str(), ios_base::out);
		ausgabe2 << str2;
		ausgabe2.close();
	}

	return 1;
}


string WkmAusgabe::setDevType(std::string devT)
{
	string routerG = "\ngraphics [\nfill \"#90ee90\" \ntype \"oval\" \nw 120.0000000000 \nh 50.0000000000\n]";
	string switchG = "\ngraphics [\nfill \"#66cdaa\" \ntype \"oval\" \nw 120.0000000000 \nh 50.0000000000\n]";
	string firewallG = "\ngraphics [\nfill \"#87cefa\" \ntype \"oval\" \nw 120.0000000000 \nh 50.0000000000\n]";
	string endgeraetG = "\ngraphics [\nfill \"#ffb6c1\" \ntype \"oval\" \nw 120.0000000000 \nh 50.0000000000\n]";
	string unknownG = "\ngraphics [\nfill \"#a9a9a9\" \ntype \"oval\" \nw 120.0000000000 \nh 50.0000000000\n]";
	string phoneG = "\ngraphics [\nfill \"#a9a9a9\" \nw 50.0000000000 \nh 50.0000000000\n]";
	string netzG = "\ngraphics [\nfill \"#a9a9a9\" \nw 150.0000000000 \nh 20.0000000000\n]";

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
		return switchG;
	}
	else if (devT == "5")
	{
		return switchG;
	}
	else if (devT == "11")
	{
		return phoneG;
	}
	else if (devT == "12")
	{
		return endgeraetG;
	}
	else if (devT == "30")
	{
		return netzG;
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


void WkmAusgabe::sets(bool c, bool ch, bool u, bool layer2, bool layer3, bool o, bool h)
{
	cdp = c;
	cdp_hosts = ch;
	unknownSTP = u;
	l2 = layer2;
	l3 = layer3;
	hier = h;
	org = o;
}


void WkmAusgabe::markiereEintraege()
{
	string flagString = "0";
	if (!cdp)
	{
		flagString = "99";
	}

	string sString = "UPDATE OR REPLACE neighborship SET flag= " + flagString + " WHERE n_id IN ( ";
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
}


int WkmAusgabe::doItL3()
{
	nodes = "";
	edges = "";
	clusters = "\nrootcluster [";
	
	// GML Rahmenstrings initialisieren
	string interfaceG = "\ngraphics [\nfill \"#faebd7\" \ntype \"rectangle\" \nw 50.0000000000 \nh 20.0000000000\n]";			// Normales Interface
	string interfaceErrorG = "\ngraphics [\nfill \"#ff0000\" \ntype \"oval\" \nw 20.0000000000 \nh 20.0000000000\n]";			// Fehler am Interface anzeigen
	string interfaceWarningG = "\ngraphics [\nfill \"#ffa500\" \ntype \"oval\" \nw 20.0000000000 \nh 20.0000000000\n]";			// Warnung am Interface anzeigen
	string processingError = "\ngraphics [\nfill \"#ff0000\" \ntype \"oval\" \nw 100.0000000000 \nh 100.0000000000\n]";			// Wenn es einen Logikfehler gibt
	string nodeStartG = "node [\nid ";
	string nodeLabelAG = "\nlabel \"";
	string nodeLabelEG = "\"";
	string nodeEndeG = "\n]\n";
	string edgeSourceG = "edge [\nsource ";
	string edgeTargetG = "\ntarget ";
	string edgeEndeG = "\n]\n";
	string clusterStart = "\ncluster [\nid ";
	string clusterVertexStart = "\nvertex \"";
	string clusterVertexEnd = "\"";
	string clusterEnd = "\n]";

	int nId = 0;		// Node ID
	int edgeId = 0;		// Edge ID
	int clusterId = 0;	// Cluster-ID
	int tClusterId = 10000;
	vector<string> clusterStrings;		// Cluster Strings, die dann zum Schluss zusammengefasst werden

	map<string,int> nIdsCluster;		// Sammlung aller Node IDs um mehrfache Einträge als Node zu verhindern und die Clusterobjekte richtig zuzuordnen
	map<string,int>::iterator nItCluster;		// Iterator für die Node/Cluster Sammlung
	typedef pair <string,int> psi;		// Pair für das Einfügen in die Map

	map<string,string> nIds;			// Sammlung aller Node IDs um mehrfache Einträge als Node zu verhindern
	map<string,string>::iterator nIt;		// Iterator für die Node Sammlung
	typedef pair <string,string> pss;		// Pair für das Einfügen in die Map

	multimap <string, string> IntfData;	// Sammlung aller Interfaces mit Fehlern und Warnungen
	multimap <string, string>::iterator IntfDataIter, IntfDataAnfang, IntfDataEnde;

	string nId1 = "";		// Node ID 1 -> für die Edge Beschreibung
	string nId2 = "";		// Node ID 2 -> für die Edge Beschreibung
	string nId3 = "";		// Node ID 3 -> für die Edge Beschreibung
	string nIDString = "";	// Temp Node ID als String

	string tempClusterId = "";	// Cluster ID für den Interface/Error Cluster


	// ipSubnet Tabelle auslesen
	string sString = "SELECT DISTINCT subnet,mask FROM ipSubnet;";
	vector<vector<string>> netze = dieDB->query(sString.c_str());
	for(vector<vector<string>>::iterator it = netze.begin(); it < netze.end(); ++it)
	{
		string h1 = it->at(0) + " / " + it->at(1);
		string type1 = "30";
		
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
		string i1 = "";
		string h2 = "";
		string type2 = "";

		sString = "SELECT intf_id, intfName, duplex, errLvl, loadLvl FROM interfaces ";
		sString += "INNER JOIN intfSubnet ON intfSubnet.interfaces_intf_id=interfaces.intf_id ";
		sString += "INNER JOIN ipSubnet ON ipSubnet.ipSubnet_id=intfSubnet.ipSubnet_ipSubnet_id ";
		sString += "WHERE ipSubnet.subnet LIKE '" + it->at(0) +"';";
		vector<vector<string>> intfs = dieDB->query(sString.c_str());
		for(vector<vector<string>>::iterator it1 = intfs.begin(); it1 < intfs.end(); ++it1)
		{
			// Zugehörigen Hostnamen auslesen
			sString = "SELECT hostname, type from device ";
			sString += "INNER JOIN devInterface ON devInterface.device_dev_id=device.dev_id ";
			sString += "WHERE devInterface.interfaces_int_id=" + it1->at(0) + ";";
			vector<vector<string>> dev = dieDB->query(sString.c_str());

			h2 = dev[0][0];
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

			i1 = it1->at(1);
			nId2 = boost::lexical_cast<std::string>(nId);
			nId++;
			nodes += nodeStartG + nId2 + nodeLabelAG + i1 + nodeLabelEG + interfaceG + nodeEndeG;

			
			tempClusterId = boost::lexical_cast<std::string>(tClusterId);
			tClusterId++;

			nItCluster = nIdsCluster.find(it->at(0));
			clusterStrings[nItCluster->second] += clusterStart + tempClusterId;
			// Cluster Strings weiterbauen
			clusterStrings[nItCluster->second] += clusterVertexStart + nId2 + clusterVertexEnd;

			string dup = it1->at(2);
			if (dup.find("Half-duplex") != dup.npos)
			{
				nIDString = boost::lexical_cast<std::string>(nId);
				nodes += nodeStartG + nIDString + nodeLabelAG + "D" + nodeLabelEG + interfaceWarningG + nodeEndeG;
				IntfData.insert(pss(nId2, nIDString));
				clusterStrings[nItCluster->second] += clusterVertexStart + nIDString + clusterVertexEnd;
				nId++;
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
				nIDString = boost::lexical_cast<std::string>(nId);
				nodes += nodeStartG + nIDString + nodeLabelAG + "E" + nodeLabelEG + interfaceErrorG + nodeEndeG;
				IntfData.insert(pss(nId2, nIDString));
				clusterStrings[nItCluster->second] += clusterVertexStart + nIDString + clusterVertexEnd;
				nId++;
			}
			else if (err > 10)
			{
				nIDString = boost::lexical_cast<std::string>(nId);
				nodes += nodeStartG + nIDString + nodeLabelAG + "E" + nodeLabelEG + interfaceWarningG + nodeEndeG;
				IntfData.insert(pss(nId2, nIDString));
				clusterStrings[nItCluster->second] += clusterVertexStart + nIDString + clusterVertexEnd;
				nId++;
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
				nIDString = boost::lexical_cast<std::string>(nId);
				nodes += nodeStartG + nIDString + nodeLabelAG + "L" + nodeLabelEG + interfaceErrorG + nodeEndeG;
				IntfData.insert(pss(nId2, nIDString));
				clusterStrings[nItCluster->second] += clusterVertexStart + nIDString + clusterVertexEnd;
				nId++;
			}
			else if (load > 100)
			{
				nIDString = boost::lexical_cast<std::string>(nId);
				nodes += nodeStartG + nIDString + nodeLabelAG + "L" + nodeLabelEG + interfaceWarningG + nodeEndeG;
				IntfData.insert(pss(nId2, nIDString));
				clusterStrings[nItCluster->second] += clusterVertexStart + nIDString + clusterVertexEnd;
				nId++;
			}

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
			string nachbaren = h2 + "-" + i1 + " --> " + h1 + "\n";
			schreibeLog(WkLog::WkLog_NORMALTYPE, nachbaren, WkLog::WkLog_SCHWARZ);
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
		schreibeLog(WkLog::WkLog_ZEIT, "6203: Unable to draw graph!\r\n", 
			WkLog::WkLog_ROT, WkLog::WkLog_FETT);
	}
	return 1;
}



#endif // WKTOOLS_MAPPER


	
	
	
	

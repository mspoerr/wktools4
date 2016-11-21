		// eBuf auf Fehlermeldung überprüfen: Fehlermeldungen beginnen immer mit CR/LF, außer bei Konsoleverbindungen.
		// Dort wird deshalb bei Verbindungsaufbau "logging synchronous" an den lines konfiguriert.
		// Es wird der Puffer ab "empfangsOffset" durchsucht, da bei Konsoleverbindungen die gesendeten Daten auch im Empfangspuffer stehen
		// und daher nicht mehr untersucht werden müssen.
		empfangsOffset = zuletztGesendet.length();
		if (debugSpecial2)
		{
			std::string fm = "\n2702: <Fehlererkennung><";
			fm += antwort[empfangsOffset+1];
			fm += "><";
			fm += eBuf[empfangsOffset+1];
			fm += "><";
			fm += boost::lexical_cast<std::string>(antwort.length());
			fm += "><";
			fm += boost::lexical_cast<std::string>(empfangsOffset);
			fm += "><<<<<";
			fm += antwort;
			fm += ">>>>>";
			schreibeLog(fm, DEBUGFEHLER);
		}
//		if ((eBuf[empfangsOffset + 1] == 0x0A) || (eBuf[empfangsOffset + 1] == 0x20))
		if ((antwort[empfangsOffset + 1] == 0x0A) || (antwort[empfangsOffset + 1] == 0x20))
		{
			if (debugSpecial2)
			{
				schreibeLog("\n2702: <Fehlererkennung#2>", DEBUGFEHLER);
			}
			size_t prozentPos, endePos = 0;
			std::string fehlermeldung = "2301: ";

			prozentPos = antwort.find("%");
			endePos = antwort.find_first_of("\r\n", prozentPos);
			if (prozentPos && (prozentPos != antwort.npos))
			{
				if (debugSpecial2)
				{
					schreibeLog("\n2702: <Fehlererkennung#3>", DEBUGFEHLER);
				}
				if ((antwort[prozentPos-1] == ' ') || (antwort[prozentPos-1] == '\n'))
				{
					if (debugSpecial2)
					{
						schreibeLog("\n2702: <Fehlererkennung#4>", DEBUGFEHLER);
					}
					bufStat = FEHLERMELDUNG;			
//					fehlermeldung += eBuf.substr(prozentPos, endePos);
					fehlermeldung += antwort.substr(prozentPos, endePos);
					fehlermeldung += " // Last command: ";
					fehlermeldung += zuletztGesendet;

					if (f2301)
					{
						schreibeLog(fehlermeldung, FEHLER, "2301");
					}
				}
			}

//			ALT:
//			prozentPos = eBuf.find("%", empfangsOffset);
//			endePos = eBuf.find_first_of("\r\n", prozentPos);
			
			//if (prozentPos && (prozentPos != eBuf.npos))
			//{
			//	if ((eBuf[prozentPos-1] == ' ') || (eBuf[prozentPos-1] == '\n'))
			//	{
			//		bufStat = FEHLERMELDUNG;			
			//		fehlermeldung += eBuf.substr(prozentPos, endePos);
			//		fehlermeldung += " // Last command: ";
			//		fehlermeldung += zuletztGesendet;

			//		if (f2301)
			//		{
			//			schreibeLog(fehlermeldung, FEHLER, "2301");
			//		}
			//	}
			//}
			else if (!prozentPos)
			{
				bufStat = FEHLERMELDUNG;			
				fehlermeldung += eBuf.substr(prozentPos, endePos);
				fehlermeldung += " // Last command: ";
				fehlermeldung += zuletztGesendet;

				if (f2301)
				{
					schreibeLog(fehlermeldung, FEHLER, "2301");
				}
			}
		}

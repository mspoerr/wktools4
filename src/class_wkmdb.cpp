#include "class_wkmdb.h"
#ifdef WKTOOLS_MAPPER

WkmDB::WkmDB(std::string fname, bool im)
{
	wkmdb = NULL;
	filename = fname;
	open(im);
	inmem = im;
}

WkmDB::~WkmDB()
{
	close();
}


bool WkmDB::open(bool im)
{
	sqlite3 *pFile;
	sqlite3_backup *pBackup;

	
	if (im)
	{
		sqlite3_open(":memory:", &wkmdb);		

		if(sqlite3_open(filename.c_str(), &pFile) == SQLITE_OK)
		{
			pBackup = sqlite3_backup_init(wkmdb, "main", pFile, "main");
			if (pBackup)
			{
				sqlite3_backup_step(pBackup, -1);
				sqlite3_backup_finish(pBackup);
			}
			sqlite3_close(pFile);
			return true;
		}
		sqlite3_close(pFile);
	}
	else
	{
		if(sqlite3_open(filename.c_str(), &wkmdb) == SQLITE_OK)
		{
			return true;
		}
	}

	return false;   
}

std::vector<std::vector<std::string> > WkmDB::query(const char* query)
{
	sqlite3_stmt *statement;
	std::vector<std::vector<std::string> > results;

	if(sqlite3_prepare_v2(wkmdb, query, -1, &statement, 0) == SQLITE_OK)
	{
		int cols = sqlite3_column_count(statement);
		int result = 0;
		while(true)
		{
			result = sqlite3_step(statement);

			if(result == SQLITE_ROW)
			{
				std::vector<std::string> values;
				for(int col = 0; col < cols; col++)
				{
					std::string  val;
					char * ptr = (char*)sqlite3_column_text(statement, col);

					if(ptr)
					{
						val = ptr;
					}
					else val = ""; // this can be commented out since std::string  val;
					// initialize variable 'val' to empty std::string anyway

					values.push_back(val);  // now we will never push NULL
				}
				results.push_back(values);
			}
			else
			{
				break;  
			}
		}

		sqlite3_finalize(statement);
	}

	error = sqlite3_errmsg(wkmdb);
	if(error != "not an error") std::cout << query << " " << error << std::endl;

	return results;  
}

bool WkmDB::close()
{

	if (inmem)
	{
		sqlite3 *pFile;
		sqlite3_backup *pBackup;
		
		if(sqlite3_open(filename.c_str(), &pFile) == SQLITE_OK)
		{
			pBackup = sqlite3_backup_init(pFile, "main", wkmdb, "main");
			if (pBackup)
			{
				sqlite3_backup_step(pBackup, -1);
				sqlite3_backup_finish(pBackup);
			}

			sqlite3_close(pFile);
			sqlite3_close(wkmdb);   

			return true;
		}

		sqlite3_close(pFile);
		sqlite3_close(wkmdb);   
	}
	else
	{
		sqlite3_close(wkmdb); 
	}
	return false;
}

#endif
#ifndef _INCLUDE_SOURCEMOD_SQLITE_RESULTSET_H
#define _INCLUDE_SOURCEMOD_SQLITE_RESULTSET_H

#include "SqliteHeaders.h"
#include "SqliteDriver.h"
#include "SqliteDatabase.h"
#include "SqliteQuery.h"

namespace SourceMod
{
	class SqliteResultSet : public IResultSet, public IResultRow
	{
		/** 
		 * IResultSet
		 */
	public:
		SqliteResultSet(SqliteQuery::SqliteResults &res);
		~SqliteResultSet();
	public:
		void FreeHandle();
	public: 
		unsigned int RowCount();
		unsigned int FieldCount();
		const char *FieldNumToName(unsigned int num);
		bool FieldNameToNum(const char *name, unsigned int *columnId);
	public:
		bool IsDone();
		IResultRow *GetRow();
		void NextRow();
		void Rewind();
	public:
		/** 
		 * IResultRow
		 */
	public:
		const char *GetString(unsigned int columnId);
		double GetDouble(unsigned int columnId);
		float GetFloat(unsigned int columnId);
		int GetInt(unsigned int columnId);
		bool IsNull(unsigned int columnId);
		const char *GetRaw(unsigned int columnId, size_t *length);
		bool NextResultSet();
	private:
		const char *GetStringSafe(unsigned int columnId);
	private:
		char **m_pResults;
		unsigned int m_Columns;
		unsigned int m_Rows;
		unsigned int m_CurRow;
		unsigned int m_CurIndex;
	};
};


#endif //_INCLUDE_SOURCEMOD_SQLITE_RESULTSET_H

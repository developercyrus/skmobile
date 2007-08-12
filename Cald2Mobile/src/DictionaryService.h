#pragma once

#include <string>
#include <vector>
#include <map>

#include "skfind/skfind.h"
#include "skfind/frontend/fs/fs.h"
#include "skfind/frontend/wordlist/wordlist.h"
#include "skfind/frontend/wordlist/wildcardwordlist.h"
#include "skfind/frontend/wordlist/regexpwordlist.h"
#include "skfind/frontend/index/index.h"

#include "stdint.h"

namespace mysk
{

	struct WordListItem
	{
		PRUint32	entryId;

		std::string label;

		std::string majorKey;
	};

	class WordListService
	{
	public :

		WordListService() {};

		virtual ~WordListService() {};

		virtual SKERR findWordListPosition(std::wstring const& sortKey, PRUint32 * pos);

		virtual SKERR findWordListPosition(std::string const& sortKey, PRUint32 * pos) = 0;

		virtual PRUint32 getWordListCount() = 0;

		virtual SKERR getWordListItem(PRUint32 position, WordListItem * item) = 0;

	};


	struct ResultItem
	{
		PRUint32	entryId;
		PRUint32	type;
		std::string	contextId;
		std::string label;
		PRUint32	clid;
	};

	typedef std::vector<ResultItem>		ResultList;

	typedef ResultList::const_iterator	ResultListIterator;

	class QueryService
	{
	public :

		QueryService() {};

		virtual ~QueryService() {};

		virtual SKERR query(std::wstring const& input, PRBool bUseFlex, PRUint8 types, PRBool bUseDeflect);

		virtual SKERR query(std::string const& input, PRBool bUseFlex, PRUint8 types, PRBool bUseDeflect) = 0;

		virtual PRUint32 getResultCount() = 0;

		virtual SKERR getResultItem(PRUint32 position, ResultItem * item) = 0;
	};

	class FileSystemService
	{
	public :

		FileSystemService() {};

		virtual ~FileSystemService() 
		{
			for(FilesystemCache::const_iterator it = filesystemCache.begin(); it != filesystemCache.end(); it++)
			{
				delete it->second;
			}
		};

		virtual SKERR prepare(std::string const& path, SKFileSystem ** fileSystem);

		virtual SKERR getFileData(std::string const& fileSystem, std::string const& path, skPtr<SKBinary> & pData);

		virtual SKERR getFileData(std::string const& fileSystem, std::string const& path, std::vector<uint8_t> & buffer);

		virtual SKERR getTextFileData(std::string const& fileSystem, std::string const& path, std::string & buffer);

		virtual SKERR copyFile(std::string const& fileSystem, std::string const& path, std::string const& destPath);

	protected :

		typedef std::map<std::string, SKFileSystem *> FilesystemCache;

		FilesystemCache filesystemCache;

	};

	class DictionaryService
	{
	public :

		DictionaryService()
			:	wordListService(NULL), queryService(NULL), fileSystemService(NULL) 
		{
		}

		virtual ~DictionaryService()
		{
			if(NULL != wordListService)
				delete wordListService;

			if(NULL != queryService)
				delete queryService;

			if(NULL != fileSystemService)
				delete fileSystemService;
		}

		void init(char const* skHome);

		WordListService * getWordListService()
		{
			return wordListService;
		}

		 void setWordListService(WordListService * s)
		{
			if(NULL != wordListService)
				delete wordListService;
			wordListService = s;
		}

		QueryService * getQueryService()
		{
			return queryService;
		}

		void setQueryService(QueryService * s)
		{
			if(NULL != queryService)
				delete queryService;
			queryService = s;
		}

		FileSystemService * getFileSystemService()
		{
			return fileSystemService;
		}

		void setFileSystemService(FileSystemService * s)
		{
			if(NULL != fileSystemService)
				delete fileSystemService;
			fileSystemService = s;
		}

	protected :

		PRBool				wordListServiceReady;

		WordListService *	wordListService;

		PRBool				queryServiceReady;

		QueryService *		queryService;

		PRBool				fileSystemServiceReady;

		FileSystemService * fileSystemService;

	};

	class SKWordListService : public WordListService
	{
	public :

		virtual SKERR init(char const* dataPath);

		virtual SKERR findWordListPosition(std::string const& sortKey, PRUint32 * pos);

		virtual PRUint32 getWordListCount();

		virtual SKERR getWordListItem(PRUint32 position, WordListItem * item);

	private :

		skPtr<SKIRecordSet>					wordListTable;

		skPtr<SKIField>						labelField;

		skPtr<SKIField>						majorKeyField;

	};

	class SKIndexQueryService : public QueryService
	{
	public :

		virtual SKERR init(char const* indexPath, char const* indexDataPath, char const* deflecIndexPath, char const* deflecTabPath);

		virtual SKERR query(std::string const& input, PRBool bUseFlex, PRUint8 types, PRBool bUseDeflect);

		virtual PRUint32 getResultCount();

		virtual SKERR getResultItem(PRUint32 position, ResultItem * item);

	protected :

		SKERR SortCursor(skPtr<SKCursor> const& pxiCursor, SKCursor** xiCursor);

		SKERR GetCursorFromTable(
			skPtr<SKIRecordSet> const& pxTable, 
			PRUint32 piPosition, 
			std::string const& psLinkField,
			std::string const& psField,
			SKCursor** xiCursor);

		skPtr<skIStringSimplifier>			simplifier;

		skPtr<SKIndex>						index;

		skPtr<SKIRecordSet>					indexData;

		skPtr<SKIField>						entryIdField;

		skPtr<SKIField>						typeField;

		skPtr<SKIField>						contextIdField;

		skPtr<SKIField>						labelField;

		skPtr<SKIField>						clidField;

		std::string							queryString;

		skPtr<SKCursor>						searchResult;

		skPtr<SKIndex>						deflectIndex;

		skPtr<SKIRecordSet>					deflecTab;

	};


}	// namespace mysk


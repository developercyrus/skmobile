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

		virtual PRUint32					findWordListPosition(std::wstring const& sortKey);

		virtual PRUint32					findWordListPosition(std::string const& sortKey) = 0;

		virtual PRUint32					getWordListCount() = 0;

		virtual WordListItem				getWordListItem(PRUint32 position) = 0;

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

		virtual void						query(std::wstring const& input, PRBool bUseFlex, PRUint8 types, PRBool bUseDeflect);

		virtual void						query(std::string const& input, PRBool bUseFlex, PRUint8 types, PRBool bUseDeflect) = 0;

		virtual PRUint32					getResultCount() = 0;

		virtual ResultItem					getResultItem(PRUint32 position) = 0;
	};
/*
	typedef	std::pair<std::string, std::string>		Content;

	class ContentService
	{
	public :

		ContentService() {};

		virtual ~ContentService() {};

		virtual Content						getContentById(PRUint32 entryId) = 0;

	};
*/
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

		virtual SKFileSystem *				prepare(std::string const& fileSystem);

		virtual void						getFileData(std::string const& fileSystem, std::string const& path, skPtr<SKBinary> & pData);

		virtual void						getFileData(std::string const& fileSystem, std::string const& path, std::vector<uint8_t> & buffer);

		virtual void						getTextFileData(std::string const& fileSystem, std::string const& path, std::string & buffer);

		virtual void						copyFile(std::string const& fileSystem, std::string const& path, std::string const& destPath);

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

		virtual void init(char const* skHome);

		virtual WordListService * getWordListService()
		{
			return wordListService;
		}

		virtual QueryService * getQueryService()
		{
			return queryService;
		}

		virtual FileSystemService * getFileSystemService()
		{
			return fileSystemService;
		}

	protected :

		WordListService *	wordListService;

		QueryService *		queryService;

		FileSystemService * fileSystemService;

	};

	class SKWordListService : public WordListService
	{
	public :

		SKWordListService(char const* dataPath);

		virtual PRUint32					findWordListPosition(std::string const& input);

		virtual PRUint32					getWordListCount();

		virtual WordListItem				getWordListItem(PRUint32 position);

	private :

		skPtr<SKIRecordSet>					wordListTable;

		skPtr<SKIField>						labelField;

		skPtr<SKIField>						majorKeyField;

	};

	class SKIndexQueryService : public QueryService
	{
	public :

		SKIndexQueryService(char const* indexPath, char const* indexDataPath, char const* deflecIndexPath, char const* deflecTabPath);

		virtual void						query(std::string const& input, PRBool bUseFlex, PRUint8 types, PRBool bUseDeflect);

		virtual PRUint32					getResultCount();

		virtual ResultItem					getResultItem(PRUint32 position);

	protected :

		skPtr<SKCursor>							SortCursor(skPtr<SKCursor> const& pxiCursor);

		skPtr<SKCursor>							GetCursorFromTable(
			skPtr<SKIRecordSet> const& pxTable, 
			PRUint32 piPosition, 
			std::string const& psLinkField,
			std::string const& psField);

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


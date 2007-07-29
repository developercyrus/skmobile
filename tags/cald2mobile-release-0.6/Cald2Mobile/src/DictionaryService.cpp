#include "DictionaryService.h"
#include "skcore/unicode/unicodesimplifier.h"

#include <atltime.h>
#include "unicode.h"

using namespace std;
using namespace mysk;



void DictionaryService::init(char const* skHome)
{
	SKERR err;
	SKEnvir *pEnvir = NULL;
	err = SKEnvir::GetEnvir(&pEnvir);
	if(noErr != err)
		return;

	err = pEnvir->Init(skHome);
	if(noErr != err)
		return;

	SKFactory *pFactory = NULL;                                             
	err = SKFactory::GetFactory(&pFactory);
	if(noErr != err)
		return;

	return;
}



PRUint32 WordListService::findWordListPosition(std::wstring const& input)
{
	size_t len = wcstombs(NULL, input.c_str(), 0);
	char * cinput = new char[len + 1];
	wcstombs(cinput, input.c_str(), len);
	string inputString(cinput, len);
	delete[] cinput;
	return this->findWordListPosition(inputString);
}



void QueryService::query(std::wstring const& input, PRBool bUseFlex, PRUint8 types, PRBool bUseDeflect)
{
	size_t len = wcstombs(NULL, input.c_str(), 0);
	char * cinput = new char[len + 1];
	wcstombs(cinput, input.c_str(), len);
	string inputString(cinput, len);
	delete[] cinput;
	this->query(inputString, bUseFlex, types, bUseDeflect);
}



SKWordListService::SKWordListService(char const* dataPath)
{
	SKERR err;

	SKFactoryGetRecordSet(dataPath, this->wordListTable, err);
	if(err != noErr)
		THROW_RUNTIME_EXCEPTION( "Failed to initial word list from " << dataPath );

	skPtr<SKIFldCollection> fieldCollection;
	err = this->wordListTable->GetFldCollection(fieldCollection.already_AddRefed());
	if(err != noErr)
		THROW_RUNTIME_EXCEPTION( "Failed to get the SKFldCollection of word list data from " << dataPath );

	err = fieldCollection->GetFieldByName("LABEL", this->labelField.already_AddRefed());
	if(err != noErr)
		THROW_RUNTIME_EXCEPTION( "Failed to get LABEL field in word list data.");

	err = fieldCollection->GetFieldByName("MAJORKEY", this->majorKeyField.already_AddRefed());
	if(err != noErr)
		THROW_RUNTIME_EXCEPTION( "Failed to get MAJORKEY field in word list data.");

}



PRUint32 SKWordListService::findWordListPosition(std::string const& sortKey)
{
	SKERR err;
	// Exact lookup
	if (sortKey.empty())
		return 0;
	PRUint32 id = 0;
	err = this->wordListTable->LookupText(sortKey.c_str(), skflmEXACT, &id);
	if(err == noErr)
	{
		// do nothing
	}
	else if(err == err_notfound)
	{
		// Last before lookup
		err = this->wordListTable->LookupText(sortKey.c_str(), skflmLASTBEFORE, &id);
		if(err != noErr)
			THROW_RUNTIME_EXCEPTION( "Failed to lookup '" << sortKey.c_str() << "' last before word list data.");
		if (id < this->getWordListCount() - 1) 
		{
			id += 1;
		}
		return id;
	}
	else
		THROW_RUNTIME_EXCEPTION( "Failed to lookup '" << sortKey.c_str() << "' in word list data.");


	// Check if items above the current one have the same key
	for(; id > 0; --id)
	{
		skPtr<SKIRecord> pRecord;
		err = this->wordListTable->GetRecord(id - 1, pRecord.already_AddRefed());
		if(err != noErr)
			THROW_RUNTIME_EXCEPTION( "Failed to get record whose id is " << (id - 1) << ".");

		skPtr<SKBinary> xiData;
		err=  pRecord->GetDataFieldValue(this->majorKeyField, xiData.already_AddRefed());
		if(err != noErr)
			THROW_RUNTIME_EXCEPTION( "Failed to get the majorKeyField of NO. " << (id - 1) << " record.");
		std::string majorKey((char const*)xiData->GetSharedData());
		if(majorKey != sortKey)
			break;
	}
	return id;
}



PRUint32 SKWordListService::getWordListCount()
{
	PRUint32 count = 0;
	this->wordListTable->GetCount(&count);
	return count;
}



WordListItem SKWordListService::getWordListItem(PRUint32 position)
{
	SKERR err;

	skPtr<SKIRecord> pRecord;
	err = this->wordListTable->GetRecord(position, pRecord.already_AddRefed());
	if(err != noErr)
		THROW_RUNTIME_EXCEPTION( "Failed to get NO." << position << " word list data." );

	WordListItem item;
	item.entryId = position;

	skPtr<SKBinary> labelValue;
	err = pRecord->GetDataFieldValue(this->labelField, labelValue.already_AddRefed());
	if(err != noErr)
		THROW_RUNTIME_EXCEPTION( "Failed to get the label field of NO." << position << " index data." );
	item.label = (char const*)labelValue->GetSharedData();

	skPtr<SKBinary> majorKeyValue;
	err = pRecord->GetDataFieldValue(this->majorKeyField, majorKeyValue.already_AddRefed());
	if(err != noErr)
		THROW_RUNTIME_EXCEPTION( "Failed to get the majorKey field of NO." << position << " index data." );
	item.majorKey = (char const*)majorKeyValue->GetSharedData();

	return item;
}



SKIndexQueryService::SKIndexQueryService(char const* indexPath, char const* indexDataPath, char const* deflecIndexPath, char const* deflecTabPath)
{
	SKERR err;

	*this->simplifier.already_AddRefed() = sk_CreateInstance(skStringUnicodeSimplifier)
		("uni_case,uni_comp");

	*this->index.already_AddRefed() = sk_CreateInstance(SKIndex)();

	err = this->index->SetFileName(indexPath);
	if(err != noErr)
		THROW_RUNTIME_EXCEPTION( "Failed to initial index from " << indexPath );

	SKFactoryGetRecordSet(indexDataPath, this->indexData, err);
	if(err != noErr)
		THROW_RUNTIME_EXCEPTION( "Failed to initial index data from " << indexDataPath );

	skPtr<SKIFldCollection> fieldCollection;
	err = this->indexData->GetFldCollection(fieldCollection.already_AddRefed());
	if(err != noErr)
		THROW_RUNTIME_EXCEPTION( "Failed to get the SKFldCollection of index data from " << indexDataPath );

	err = fieldCollection->GetFieldByName("ENTRYID", this->entryIdField.already_AddRefed());
	if(err != noErr)
		THROW_RUNTIME_EXCEPTION( "Failed to get ENTRYID field in index data.");

	err = fieldCollection->GetFieldByName("TYPE", this->typeField.already_AddRefed());
	if(err != noErr)
		THROW_RUNTIME_EXCEPTION( "Failed to get TYPE field in index data.");

	err = fieldCollection->GetFieldByName("CONTEXTID", this->contextIdField.already_AddRefed());
	if(err != noErr)
		THROW_RUNTIME_EXCEPTION( "Failed to get CONTEXTID field in index data.");

	err = fieldCollection->GetFieldByName("LABEL", this->labelField.already_AddRefed());
	if(err != noErr)
		THROW_RUNTIME_EXCEPTION( "Failed to get LABEL field in index data.");

	err = fieldCollection->GetFieldByName("CLID", this->clidField.already_AddRefed());
	if(err != noErr)
		THROW_RUNTIME_EXCEPTION( "Failed to get CLID field in index data.");

	*this->deflectIndex.already_AddRefed() = sk_CreateInstance(SKIndex)();

	err = this->deflectIndex->SetFileName(deflecIndexPath);
	if(err != noErr)
		THROW_RUNTIME_EXCEPTION( "Failed to initial index from " << indexPath );

	SKFactoryGetRecordSet(deflecTabPath, this->deflecTab, err);
	if(err != noErr)
		THROW_RUNTIME_EXCEPTION( "Failed to initial deflecTab from " << deflecTabPath );
}



void SKIndexQueryService::query(std::string const& input, PRBool bUseFlex, PRUint8 types, PRBool bUseDeflect)
{
	CTime begin, end;
	CTimeSpan searchTime, getTime, countTime, filterTime, sortTime;

	SKERR err;
	skPtr<SKIndexResult> pIndexResult;
	
	begin = CTime::GetCurrentTime();
	err = this->index->SearchExpression(input.c_str(), bUseFlex, this->simplifier, NULL, pIndexResult.already_AddRefed());
	end = CTime::GetCurrentTime();
	searchTime = end - begin;
	if(err != noErr)
		THROW_RUNTIME_EXCEPTION ( "Failed to query. queryString = " << input.c_str() << ", bUseFlex = " << bUseFlex) ;

	begin = CTime::GetCurrentTime();
	err = pIndexResult->GetDocumentList(this->searchResult.already_AddRefed());
	end = CTime::GetCurrentTime();
	getTime = end - begin;
	if(err != noErr)
		THROW_RUNTIME_EXCEPTION ( "Failed to get document list. queryString = " << input.c_str() << ", bUseFlex = " << bUseFlex) ;


	if (bUseDeflect) 
	{
		if (input.find('*') != -1 || input.find('?') != -1) 
		{
			skPtr<SKIndexResult> oDeflectResult;
			err = this->deflectIndex->SearchExpression(
				input.c_str(), 
				FALSE, 
				this->simplifier, 
				NULL, 
				oDeflectResult.already_AddRefed());
			if(err != noErr)
				THROW_RUNTIME_EXCEPTION ( "Failed to query deflect index. queryString = " << input.c_str()) ;

			skPtr<SKCursor> oDeflectedCursor;
			err = oDeflectResult->GetDocumentList(oDeflectedCursor.already_AddRefed());
			if(err != noErr)
				THROW_RUNTIME_EXCEPTION ( "Failed to get deflected document list. queryString = " << input.c_str()) ;

			PRUint32 count = 0;
			err = oDeflectedCursor->GetCount(&count);
			if(err != noErr)
				THROW_RUNTIME_EXCEPTION ( "Failed to get the count of deflected document list. queryString = " << input.c_str()) ;
			for (PRUint32 i = 0; i < count; i++) 
			{
				PRUint32 iPosition = 0;
				oDeflectedCursor->GetElement(i, &iPosition);
				skPtr<SKCursor> oComplCursor = GetCursorFromTable(this->deflecTab, iPosition, "r_clid", "clid");
				oComplCursor = SortCursor(oComplCursor);
				searchResult->Merge(oComplCursor, skfopOR);
			}
		} 
		else 
		{
			PRUint32 pos = 0;
			err = this->deflecTab->LookupText(input.c_str(), skflmEXACT, &pos);
			if(err != err_notfound)
			{
				if(err != noErr)
					THROW_RUNTIME_EXCEPTION ( "Failed to lookup '" << input.c_str() << "' in  deflecTab.") ;
				skPtr<SKCursor> xiDeflectCursor = GetCursorFromTable(this->deflecTab, pos, "r_clid", "clid");
				xiDeflectCursor = SortCursor(xiDeflectCursor);
				searchResult->Merge(xiDeflectCursor, skfopOR);
			}
		}
	}


	PRUint32 count = 0;
	begin = CTime::GetCurrentTime();
	err = this->searchResult->GetCount(&count);
	end = CTime::GetCurrentTime();
	countTime = end - begin;
	if(err != noErr)
		THROW_RUNTIME_EXCEPTION ( "Failed to get the count of document list. queryString = " << input.c_str() << ", bUseFlex = " << bUseFlex) ;
	if(0 == count)
		return;

	skPtr<SKRecordFilterUNumBitField> xiRFUN(new SKRecordFilterUNumBitField);
	xiRFUN->SetDefaultPolicy (false);
	xiRFUN->SetField ("TYPE");
	for(PRUint8 i = 0; i < 8; i++)
	{
		// 0	head word
		// 1	phrase
		// 2	HWDLIST
		// 3	PHRVERB
		// 4	SENSE
		// 5	SENSE
		// 6	FULLDEFT
		// 7	FULLEXAMP
		if(0 != (0x01 & (types >> i)))
			xiRFUN->AddAcceptedValue (i);
	}
	xiRFUN->SetRecordSet( this->indexData );

	skPtr<SKCursorFilterRecordWrapper> xiCursorFilter(new SKCursorFilterRecordWrapper);
	xiCursorFilter->SetRecordFilter(xiRFUN, true);

	begin = CTime::GetCurrentTime();
	err = this->searchResult->Filter(xiCursorFilter);
	end = CTime::GetCurrentTime();
	filterTime = end - begin;

	if(err != noErr)
		THROW_RUNTIME_EXCEPTION ( "Failed to filter result. queryString = " << input.c_str() << ", bUseFlex = " << bUseFlex) ;

	skPtr<SKRecordComparatorUNumField> xiComparator(new SKRecordComparatorUNumField);
	xiComparator->SetField("TYPE");
	xiComparator->SetRecordSet( this->indexData );

	skPtr<SKRecordComparatorUNumField> xiComparatorId(new SKRecordComparatorUNumField);
	xiComparatorId->SetField("CLID");
	xiComparatorId->SetRecordSet( this->indexData );

	skPtr<SKRecordComparatorChain> xiRecordComparatorChain(new SKRecordComparatorChain);
	xiRecordComparatorChain->SetRecordSet( this->indexData );
	xiRecordComparatorChain->AddComparator(xiComparator);
	xiRecordComparatorChain->AddComparator(xiComparatorId);

	skPtr<SKCursorComparatorRecordWrapper> xiCursorComparator(new SKCursorComparatorRecordWrapper);
	xiCursorComparator->SetRecordComparator(xiRecordComparatorChain, true);

	begin = CTime::GetCurrentTime();
	err = this->searchResult->Sort(xiCursorComparator);
	end = CTime::GetCurrentTime();
	sortTime = end - begin;

	if(err != noErr)
		THROW_RUNTIME_EXCEPTION ( "Failed to sort result. queryString = " << input.c_str() << ", bUseFlex = " << bUseFlex) ;

	this->queryString = input;
}



PRUint32 SKIndexQueryService::getResultCount()
{
	PRUint32 count = 0;
	SKERR err = this->searchResult->GetCount(&count);
	if(err != noErr)
		THROW_RUNTIME_EXCEPTION( "Failed to get the count of search result." );
	return count;
}



ResultItem SKIndexQueryService::getResultItem(PRUint32 position)
{
	SKERR err = noErr;

	PRUint32 element;
	err = this->searchResult->GetElement(position, &element);
	if(err != noErr)
		THROW_RUNTIME_EXCEPTION( "Failed to get NO." << position << " element." );

	skPtr<SKIRecord> pRecord;
	err = this->indexData->GetRecord(element, pRecord.already_AddRefed());
	if(err != noErr)
		THROW_RUNTIME_EXCEPTION( "Failed to get NO." << element << " index data." );

	ResultItem result;

	err = pRecord->GetUNumFieldValue(this->entryIdField, &result.entryId);
	if(err != noErr)
		THROW_RUNTIME_EXCEPTION( "Failed to get the entryId field of NO." << element << " index data." );

	err = pRecord->GetUNumFieldValue(this->typeField, &result.type);
	if(err != noErr)
		THROW_RUNTIME_EXCEPTION( "Failed to get the type field of NO." << element << " index data." );

	skPtr<SKBinary> contextIdValue;
	err = pRecord->GetDataFieldValue(this->contextIdField, contextIdValue.already_AddRefed());
	if(err != noErr)
		THROW_RUNTIME_EXCEPTION( "Failed to get the contextId field of NO." << element << " index data." );
	result.contextId = (char const*)contextIdValue->GetSharedData();

	skPtr<SKBinary> labelValue;
	err = pRecord->GetDataFieldValue(this->labelField, labelValue.already_AddRefed());
	if(err != noErr)
		THROW_RUNTIME_EXCEPTION( "Failed to get the label field of NO." << element << " index data." );
	result.label = (char const*)labelValue->GetSharedData();

	err = pRecord->GetUNumFieldValue(this->clidField, &result.clid);
	if(err != noErr)
		THROW_RUNTIME_EXCEPTION( "Failed to get the clid field of NO." << element << " index data." );

	return result;
}



skPtr<SKCursor> SKIndexQueryService::SortCursor(skPtr<SKCursor> const& pxiCursor)
{
	SKERR err;
	skPtr<SKCursor> xiCursor;
	err = pxiCursor->Clone(xiCursor.already_AddRefed());
	if(err != noErr)
		THROW_RUNTIME_EXCEPTION( "Failed to clone cursor." );

	err = xiCursor->Sort(NULL);
	if(err != noErr)
		THROW_RUNTIME_EXCEPTION( "Failed to sort cursor." );

	return xiCursor;
}



skPtr<SKCursor> SKIndexQueryService::GetCursorFromTable(
	skPtr<SKIRecordSet> const& pxTable, 
	PRUint32 piPosition, 
	std::string const& psLinkField,
	std::string const& psField)
{
	SKERR err;
	skPtr<SKIRecord> pRecord;
	err = pxTable->GetRecord(piPosition, pRecord.already_AddRefed());
	if(err != noErr)
		THROW_RUNTIME_EXCEPTION( "Failed to get record whose position is " << piPosition << " ." );

	skPtr<SKIFldCollection> pFldCollection;
	err = pxTable->GetFldCollection(pFldCollection.already_AddRefed());
	if(err != noErr)
		THROW_RUNTIME_EXCEPTION( "Failed to get FldCollection." );

	skPtr<SKIField> pField;
	err = pFldCollection->GetFieldByName(psLinkField.c_str(), pField.already_AddRefed());
	if(err != noErr)
		THROW_RUNTIME_EXCEPTION( "Failed to get filed whose name is " << psLinkField.c_str() << "." );

	skPtr<SKIRecordSet> xiRs;
	err=  pRecord->GetLinkFieldValue(pField, xiRs.already_AddRefed());
	if(err != noErr)
		THROW_RUNTIME_EXCEPTION( "Failed to get LinkFieldValue." );
	
	skPtr<SKIFldCollection> pRsFldCollection;
	err=  xiRs->GetFldCollection(pRsFldCollection.already_AddRefed());
	if(err != noErr)
		THROW_RUNTIME_EXCEPTION( "Failed to get FldCollection." );

	skPtr<SKIField> xiField;
	if (psField.empty())
	{
		err = pRsFldCollection->GetFieldByPosition(0, xiField.already_AddRefed());
		if(err != noErr)
			THROW_RUNTIME_EXCEPTION( "Failed to get the first field." );
	}
	else
	{
		err = pRsFldCollection->GetFieldByName(psField.c_str(), xiField.already_AddRefed());
		if(err != noErr)
			THROW_RUNTIME_EXCEPTION( "Failed to get the field whose name is " << psField.c_str() << "." );
	}
	PRUint32 count = 0;
	err = xiRs->GetCount(&count);
	if(err != noErr)
		THROW_RUNTIME_EXCEPTION( "Failed to get count." );

	skPtr<SKCursor> pCursor;
	err = xiRs->ExtractCursor(xiField, 0, count, pCursor.already_AddRefed());
	if(err != noErr)
		THROW_RUNTIME_EXCEPTION( "Failed to extract cursor." );

	return pCursor;
}



SKFileSystem * FileSystemService::prepare(std::string const& fileSystem)
{
	FilesystemCache::const_iterator it = filesystemCache.find(fileSystem);
	if(filesystemCache.end() != it)
	{
		// already exist
		return it->second;
	}

	SKFileSystem * pFileSystem = new SKFileSystem;
	SKERR err = pFileSystem->SetFileName(fileSystem.c_str());
	if(err != noErr)
		THROW_RUNTIME_EXCEPTION( "Failed to load entry filesystem ('" << fileSystem.c_str() << "')." );

	this->filesystemCache.insert(FilesystemCache::value_type(fileSystem, pFileSystem));
	return pFileSystem;
}



void FileSystemService::getFileData(std::string const& fileSystem, std::string const& path, skPtr<SKBinary> & pData)
{
	SKERR err = noErr;

	SKFileSystem * pSKFileSystem = prepare(fileSystem);

	skPtr<SKFSFile> pFile;
	if(path.at(0) == '@')
	{
		string idString = path.substr(1);
		PRUint32 id = atoi(idString.c_str());

		// get file by id
		err = pSKFileSystem->GetFile (id, pFile.already_AddRefed());
		if(noErr != err)
			THROW_RUNTIME_EXCEPTION( "Failed to get '" << path.c_str() << "' file in " << fileSystem.c_str() << " filesystem." );
	}
	else
	{
		skPtr<SKFSDirectory> pRootDir;
		err = pSKFileSystem->GetRootDir(pRootDir.already_AddRefed());
		if(err != noErr)
			THROW_RUNTIME_EXCEPTION( "Failed to get root directory" );

		string::size_type pos = string::npos;
		string::size_type offset = 0;
		skPtr<SKFSDirectory> pCurrentDir = pRootDir;
		skPtr<SKFSDirectory> pSubDir;
		for(pos = path.find('/', offset); pos != string::npos; offset = pos + 1, pos = path.find('/', offset))
		{
			string dirName = path.substr(offset, pos - offset);
			err = pCurrentDir->GetDir(dirName.c_str(), pSubDir.already_AddRefed());
			if(err != noErr)
				THROW_RUNTIME_EXCEPTION( "Failed to get sub directory " << path.substr(0, pos).c_str());
			pCurrentDir = pSubDir;
		}

		string filename = path.substr(offset);
		err = pCurrentDir->GetFile(filename.c_str(), pFile.already_AddRefed());
		if(err != noErr)
			THROW_RUNTIME_EXCEPTION( "Failed to get file " << path.c_str());
	}

	err = pFile->GetData(pData.already_AddRefed());
	if(err != noErr)
		THROW_RUNTIME_EXCEPTION( "Failed to get file data " << pFile->GetSharedPath() 
			<< "/" << pFile->GetSharedName());
}



void FileSystemService::getFileData(std::string const& fileSystem, std::string const& path, std::vector<uint8_t>& buffer)
{
	skPtr<SKBinary> pData;
	this->getFileData(fileSystem, path, pData);

	uint8_t * data = (uint8_t *)pData->GetSharedData();
	pData->GetSize();
	std::copy(data, data + pData->GetSize(), std::back_inserter(buffer));
}



void FileSystemService::getTextFileData(std::string const& fileSystem, std::string const& path, std::string& buffer)
{
	skPtr<SKBinary> pData;
	this->getFileData(fileSystem, path, pData);

	buffer = (char const*)pData->GetSharedData();
}



void FileSystemService::copyFile(std::string const& fileSystem, std::string const& path, std::string const& destPath)
{
	skPtr<SKBinary> pData;
	this->getFileData(fileSystem, path, pData);

	SKERR err = pData->WriteToFile(destPath.c_str());
	if(err != noErr)
		THROW_RUNTIME_EXCEPTION( "Failed to copy '" << path.c_str() << "' to '" << destPath.c_str() << "'.");

}

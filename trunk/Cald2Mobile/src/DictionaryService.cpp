#include "DictionaryService.h"
#include "skcore/unicode/unicodesimplifier.h"

#include <atltime.h>

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



SKERR WordListService::findWordListPosition(std::wstring const& input, PRUint32 * pos)
{
	size_t len = wcstombs(NULL, input.c_str(), 0);
	char * cinput = new char[len + 1];
	wcstombs(cinput, input.c_str(), len);
	string inputString(cinput, len);
	delete[] cinput;
	return this->findWordListPosition(inputString, pos);
}



SKERR QueryService::query(std::wstring const& input, PRBool bUseFlex, PRUint8 types, PRBool bUseDeflect)
{
	size_t len = wcstombs(NULL, input.c_str(), 0);
	char * cinput = new char[len + 1];
	wcstombs(cinput, input.c_str(), len);
	string inputString(cinput, len);
	delete[] cinput;
	return this->query(inputString, bUseFlex, types, bUseDeflect);
}



SKERR SKWordListService::init(char const* dataPath)
{
	SKERR err;

	SKFactoryGetRecordSet(dataPath, this->wordListTable, err);
	if(err != noErr)
	{
		SK_TRACE(SK_LOG_INFO, "Failed to initial word list from %s .", dataPath);
		return err;
	}

	skPtr<SKIFldCollection> fieldCollection;
	err = this->wordListTable->GetFldCollection(fieldCollection.already_AddRefed());
	if(err != noErr)
	{
		SK_TRACE(SK_LOG_INFO, "Failed to get the SKFldCollection of word list data from %s .", dataPath);
		return err;
	}

	err = fieldCollection->GetFieldByName("LABEL", this->labelField.already_AddRefed());
	if(err != noErr)
	{
		SK_TRACE(SK_LOG_INFO, "Failed to get LABEL field in word list data.");
		return err;
	}

	err = fieldCollection->GetFieldByName("MAJORKEY", this->majorKeyField.already_AddRefed());
	if(err != noErr)
	{
		SK_TRACE(SK_LOG_INFO, "Failed to get MAJORKEY field in word list data.");
		return err;
	}
	return noErr;
}



SKERR SKWordListService::findWordListPosition(std::string const& sortKey, PRUint32 * pos)
{
	SKERR err;
	// Exact lookup
	if (sortKey.empty())
	{
		*pos = 0;
		return noErr;
	}

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
		{
			SK_TRACE(SK_LOG_INFO, "Failed to lookup '%s' last before word list data.", sortKey.c_str());
			return err;
		}
		if (id < this->getWordListCount() - 1) 
		{
			id += 1;
		}
		*pos = id;
		return noErr;
	}
	else
	{
		SK_TRACE(SK_LOG_INFO, "Failed to lookup '%s' in word list data.", sortKey.c_str());
		return err;
	}


	// Check if items above the current one have the same key
	for(; id > 0; --id)
	{
		skPtr<SKIRecord> pRecord;
		err = this->wordListTable->GetRecord(id - 1, pRecord.already_AddRefed());
		if(err != noErr)
		{
			SK_TRACE(SK_LOG_INFO, "Failed to get record whose id is %d.", (id - 1));
			return err;
		}

		skPtr<SKBinary> xiData;
		err=  pRecord->GetDataFieldValue(this->majorKeyField, xiData.already_AddRefed());
		if(err != noErr)
		{
			SK_TRACE(SK_LOG_INFO, "Failed to get the majorKeyField of NO.%d record.", (id - 1));
			return err;
		}
		std::string majorKey((char const*)xiData->GetSharedData());
		if(majorKey != sortKey)
			break;
	}
	*pos = id;
	return noErr;
}



PRUint32 SKWordListService::getWordListCount()
{
	PRUint32 count = 0;
	this->wordListTable->GetCount(&count);
	return count;
}



SKERR SKWordListService::getWordListItem(PRUint32 position, WordListItem * item)
{
	SKERR err;

	skPtr<SKIRecord> pRecord;
	err = this->wordListTable->GetRecord(position, pRecord.already_AddRefed());
	if(err != noErr)
	{
		SK_TRACE(SK_LOG_INFO, "Failed to get NO.%d word list data.", position);
		return err;
	}

	item->entryId = position;

	skPtr<SKBinary> labelValue;
	err = pRecord->GetDataFieldValue(this->labelField, labelValue.already_AddRefed());
	if(err != noErr)
	{
		SK_TRACE(SK_LOG_INFO, "Failed to get the label field of NO.%d index data.", position);
		return err;
	}
	item->label = (char const*)labelValue->GetSharedData();

	skPtr<SKBinary> majorKeyValue;
	err = pRecord->GetDataFieldValue(this->majorKeyField, majorKeyValue.already_AddRefed());
	if(err != noErr)
	{
		SK_TRACE(SK_LOG_INFO, "Failed to get the majorKey field of NO.%d index data.", position);
		return err;
	}
	item->majorKey = (char const*)majorKeyValue->GetSharedData();

	return noErr;
}



SKERR SKIndexQueryService::init(char const* indexPath, char const* indexDataPath, char const* deflecIndexPath, char const* deflecTabPath)
{
	SKERR err;

	*this->simplifier.already_AddRefed() = sk_CreateInstance(skStringUnicodeSimplifier)
		("uni_case,uni_comp");

	*this->index.already_AddRefed() = sk_CreateInstance(SKIndex)();

	err = this->index->SetFileName(indexPath);
	if(err != noErr)
	{
		SK_TRACE(SK_LOG_INFO, "Failed to initial index from %s .", indexPath);
		return err;
	}

	SKFactoryGetRecordSet(indexDataPath, this->indexData, err);
	if(err != noErr)
	{
		SK_TRACE(SK_LOG_INFO, "Failed to initial index data from %s .", indexDataPath);
		return err;
	}

	skPtr<SKIFldCollection> fieldCollection;
	err = this->indexData->GetFldCollection(fieldCollection.already_AddRefed());
	if(err != noErr)
	{
		SK_TRACE(SK_LOG_INFO, "Failed to get the SKFldCollection of index data.");
		return err;
	}

	err = fieldCollection->GetFieldByName("ENTRYID", this->entryIdField.already_AddRefed());
	if(err != noErr)
	{
		SK_TRACE(SK_LOG_INFO, "Failed to get the ENTRYID field of index data.");
		return err;
	}

	err = fieldCollection->GetFieldByName("TYPE", this->typeField.already_AddRefed());
	if(err != noErr)
	{
		SK_TRACE(SK_LOG_INFO, "Failed to get the TYPE field of index data.");
		return err;
	}

	err = fieldCollection->GetFieldByName("CONTEXTID", this->contextIdField.already_AddRefed());
	if(err != noErr)
	{
		SK_TRACE(SK_LOG_INFO, "Failed to get the CONTEXTID field of index data.");
		return err;
	}

	err = fieldCollection->GetFieldByName("LABEL", this->labelField.already_AddRefed());
	if(err != noErr)
	{
		SK_TRACE(SK_LOG_INFO, "Failed to get the LABEL field of index data.");
		return err;
	}

	err = fieldCollection->GetFieldByName("CLID", this->clidField.already_AddRefed());
	if(err != noErr)
	{
		SK_TRACE(SK_LOG_INFO, "Failed to get the CLID field of index data.");
		return err;
	}

	*this->deflectIndex.already_AddRefed() = sk_CreateInstance(SKIndex)();

	err = this->deflectIndex->SetFileName(deflecIndexPath);
	if(err != noErr)
	{
		SK_TRACE(SK_LOG_INFO, "Failed to initial deflect index from %s.", deflecIndexPath);
		return err;
	}

	SKFactoryGetRecordSet(deflecTabPath, this->deflecTab, err);
	if(err != noErr)
	{
		SK_TRACE(SK_LOG_INFO, "Failed to initial deflect table from %s.", deflecTabPath);
		return err;
	}
	return noErr;
}



SKERR SKIndexQueryService::query(std::string const& input, PRBool bUseFlex, PRUint8 types, PRBool bUseDeflect)
{
	DWORD begin, end;
	DWORD searchTime, getTime, countTime, filterTime, sortTime;

	SKERR err;
	skPtr<SKIndexResult> pIndexResult;
	
	begin = ::GetTickCount();;
	err = this->index->SearchExpression(input.c_str(), bUseFlex, this->simplifier, NULL, pIndexResult.already_AddRefed());
	end = ::GetTickCount();;
	searchTime = end - begin;
	if(err != noErr) 
	{
		SK_TRACE(SK_LOG_INFO, "Failed to query. error = %d, queryString = %s, bUseFlex = %d .", 
			err, input.c_str(), bUseFlex);
		return err;
	}
	SK_TRACE(SK_LOG_DEBUG, "SearchExpression '%s' in %d ms.", input.c_str(), searchTime);

	begin = ::GetTickCount();;
	err = pIndexResult->GetDocumentList(this->searchResult.already_AddRefed());
	end = ::GetTickCount();;
	getTime = end - begin;
	if(err != noErr)
	{
		SK_TRACE(SK_LOG_INFO, "Failed to get document list. error = %d, queryString = %s, bUseFlex = %d .", 
			err, input.c_str(), bUseFlex);
		return err;
	}
	SK_TRACE(SK_LOG_DEBUG, "GetDocumentList '%s' in %d ms.", input.c_str(), getTime);

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
			{
				SK_TRACE(SK_LOG_INFO, "Failed to query deflect index. queryString = %s .", input.c_str()) ;
				return err;
			}

			skPtr<SKCursor> oDeflectedCursor;
			err = oDeflectResult->GetDocumentList(oDeflectedCursor.already_AddRefed());
			if(err != noErr)
			{
				SK_TRACE(SK_LOG_INFO, "Failed to query deflected document list. queryString = %s .", input.c_str()) ;
				return err;
			}
				
			PRUint32 count = 0;
			err = oDeflectedCursor->GetCount(&count);
			if(err != noErr)
			{
				SK_TRACE(SK_LOG_INFO, "Failed to get the count of deflected document list. queryString = %s .", input.c_str()) ;
				return err;
			}
			for (PRUint32 i = 0; i < count; i++) 
			{
				PRUint32 iPosition = 0;
				err = oDeflectedCursor->GetElement(i, &iPosition);
				if(err != noErr)
				{
					SK_TRACE(SK_LOG_INFO, "Failed to get the %d element of deflected document list. queryString = %s .", 
						i, input.c_str()) ;
					return err;
				}
				skPtr<SKCursor> oComplCursor;
				err = GetCursorFromTable(this->deflecTab, iPosition, "r_clid", "clid", oComplCursor.already_AddRefed());
				if(err != noErr)
				{
					SK_TRACE(SK_LOG_INFO, "Failed to GetCursorFromTable. queryString = %s .", input.c_str()) ;
					return err;
				}

				skPtr<SKCursor> xiCursor;
				err = SortCursor(oComplCursor, xiCursor.already_AddRefed());
				if(err != noErr)
				{
					SK_TRACE(SK_LOG_INFO, "Failed to SortCursor. queryString = %s .", input.c_str()) ;
					return err;
				}

				err = searchResult->Merge(xiCursor, skfopOR);
				if(err != noErr)
				{
					SK_TRACE(SK_LOG_INFO, "Failed to merge. queryString = %s .", input.c_str()) ;
					return err;
				}
			}
		} 
		else 
		{
			PRUint32 pos = 0;
			err = this->deflecTab->LookupText(input.c_str(), skflmEXACT, &pos);
			if(err != err_notfound)
			{
				if(err != noErr)
				{
					SK_TRACE(SK_LOG_INFO, "Failed to lookup '%s' in deflect table.", input.c_str()) ;
					return err;
				}
				skPtr<SKCursor> xiDeflectCursor;
				err = GetCursorFromTable(this->deflecTab, pos, "r_clid", "clid", xiDeflectCursor.already_AddRefed());
				if(err != noErr)
				{
					SK_TRACE(SK_LOG_INFO, "Failed to GetCursorFromTable. queryString = %s .", input.c_str()) ;
					return err;
				}

				skPtr<SKCursor> xiSortedDeflectCursor;
				err = SortCursor(xiDeflectCursor, xiSortedDeflectCursor.already_AddRefed());
				if(err != noErr)
				{
					SK_TRACE(SK_LOG_INFO, "Failed to SortCursor. queryString = %s .", input.c_str()) ;
					return err;
				}

				err = searchResult->Merge(xiDeflectCursor, skfopOR);
				if(err != noErr)
				{
					SK_TRACE(SK_LOG_INFO, "Failed to merge. queryString = %s .", input.c_str()) ;
					return err;
				}
			}
		}
	}


	PRUint32 count = 0;
	begin = ::GetTickCount();;
	err = this->searchResult->GetCount(&count);
	end = ::GetTickCount();;
	countTime = end - begin;
	if(err != noErr)
	{
		SK_TRACE(SK_LOG_INFO, "Failed to get the count of document list. queryString = %s .", input.c_str()) ;
		return err;
	}
	if(0 == count)
		return noErr;

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
	err = xiRFUN->SetRecordSet( this->indexData );
	if(err != noErr)
	{
		SK_TRACE(SK_LOG_INFO, "Failed to SetRecordSet. queryString = %s .", input.c_str()) ;
		return err;
	}

	skPtr<SKCursorFilterRecordWrapper> xiCursorFilter(new SKCursorFilterRecordWrapper);
	err = xiCursorFilter->SetRecordFilter(xiRFUN, true);
	{
		SK_TRACE(SK_LOG_INFO, "Failed to SetRecordFilter. queryString = %s .", input.c_str()) ;
		return err;
	}

	begin = ::GetTickCount();;
	err = this->searchResult->Filter(xiCursorFilter);
	end = ::GetTickCount();;
	filterTime = end - begin;
	if(err != noErr)
	{
		SK_TRACE(SK_LOG_INFO, "Failed to filter result. queryString = %s .", input.c_str()) ;
		return err;
	}
	SK_TRACE(SK_LOG_DEBUG, "Filter '%s' in %d ms.", input.c_str(), filterTime);

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

	begin = ::GetTickCount();;
	err = this->searchResult->Sort(xiCursorComparator);
	end = ::GetTickCount();;
	sortTime = end - begin;
	SK_TRACE(SK_LOG_DEBUG, "Sort '%s' in %d ms.", input.c_str(), sortTime);

	if(err != noErr)
	{
		SK_TRACE(SK_LOG_INFO, "Failed to sort result. queryString = %s .", input.c_str()) ;
		return err;
	}

	this->queryString = input;

	return noErr;
}



PRUint32 SKIndexQueryService::getResultCount()
{
	PRUint32 count = 0;
	SKERR err = this->searchResult->GetCount(&count);
	if(err != noErr)
	{
		SK_TRACE(SK_LOG_INFO, "Failed to get result count.") ;
		return err;
	}
	return count;
}



 SKERR SKIndexQueryService::getResultItem(PRUint32 position, ResultItem * item)
{
	SKERR err = noErr;

	PRUint32 element;
	err = this->searchResult->GetElement(position, &element);
	if(err != noErr)
	{
		SK_TRACE(SK_LOG_INFO, "Failed to get NO.%d element.", position) ;
		return err;
	}

	skPtr<SKIRecord> pRecord;
	err = this->indexData->GetRecord(element, pRecord.already_AddRefed());
	if(err != noErr)
	{
		SK_TRACE(SK_LOG_INFO, "Failed to get NO.%d index data.", element) ;
		return err;
	}

	err = pRecord->GetUNumFieldValue(this->entryIdField, &(item->entryId));
	if(err != noErr)
	{
		SK_TRACE(SK_LOG_INFO, "Failed to the entryId field of NO.%d index data.", element) ;
		return err;
	}

	err = pRecord->GetUNumFieldValue(this->typeField, &(item->type));
	if(err != noErr)
	{
		SK_TRACE(SK_LOG_INFO, "Failed to the type field of NO.%d index data.", element) ;
		return err;
	}

	skPtr<SKBinary> contextIdValue;
	err = pRecord->GetDataFieldValue(this->contextIdField, contextIdValue.already_AddRefed());
	if(err != noErr)
	{
		SK_TRACE(SK_LOG_INFO, "Failed to the contextId field of NO.%d index data.", element) ;
		return err;
	}
	item->contextId = (char const*)contextIdValue->GetSharedData();

	skPtr<SKBinary> labelValue;
	err = pRecord->GetDataFieldValue(this->labelField, labelValue.already_AddRefed());
	if(err != noErr)
	{
		SK_TRACE(SK_LOG_INFO, "Failed to the label field of NO.%d index data.", element) ;
		return err;
	}
	item->label = (char const*)labelValue->GetSharedData();

	err = pRecord->GetUNumFieldValue(this->clidField, &(item->clid));
	if(err != noErr)
	{
		SK_TRACE(SK_LOG_INFO, "Failed to the clid field of NO.%d index data.", element) ;
		return err;
	}

	return noErr;
}



SKERR SKIndexQueryService::SortCursor(skPtr<SKCursor> const& pxiCursor, SKCursor ** xiCursor)
{
	SKERR err;
	err = pxiCursor->Clone(xiCursor);
	if(err != noErr)
	{
		SK_TRACE(SK_LOG_INFO, "Failed to clone cursor.") ;
		return err;
	}

	err = (*xiCursor)->Sort(NULL);
	if(err != noErr)
	{
		SK_TRACE(SK_LOG_INFO, "Failed to sort cursor.") ;
		return err;
	}

	return noErr;
}



SKERR SKIndexQueryService::GetCursorFromTable(
	skPtr<SKIRecordSet> const& pxTable, 
	PRUint32 piPosition, 
	std::string const& psLinkField,
	std::string const& psField,
	SKCursor** pCursor)
{
	SKERR err;
	skPtr<SKIRecord> pRecord;
	err = pxTable->GetRecord(piPosition, pRecord.already_AddRefed());
	if(err != noErr)
	{
		SK_TRACE(SK_LOG_INFO, "Failed to get record whose position is %d.", piPosition) ;
		return err;
	}

	skPtr<SKIFldCollection> pFldCollection;
	err = pxTable->GetFldCollection(pFldCollection.already_AddRefed());
	if(err != noErr)
	{
		SK_TRACE(SK_LOG_INFO, "Failed to get FldCollection.") ;
		return err;
	}

	skPtr<SKIField> pField;
	err = pFldCollection->GetFieldByName(psLinkField.c_str(), pField.already_AddRefed());
	if(err != noErr)
	{
		SK_TRACE(SK_LOG_INFO, "Failed to get filed whose name is %s.", psLinkField.c_str()) ;
		return err;
	}

	skPtr<SKIRecordSet> xiRs;
	err=  pRecord->GetLinkFieldValue(pField, xiRs.already_AddRefed());
	if(err != noErr)
	{
		SK_TRACE(SK_LOG_INFO, "Failed to get LinkFieldValue.") ;
		return err;
	}
	
	skPtr<SKIFldCollection> pRsFldCollection;
	err=  xiRs->GetFldCollection(pRsFldCollection.already_AddRefed());
	if(err != noErr)
	{
		SK_TRACE(SK_LOG_INFO, "Failed to get FldCollection.") ;
		return err;
	}

	skPtr<SKIField> xiField;
	if (psField.empty())
	{
		err = pRsFldCollection->GetFieldByPosition(0, xiField.already_AddRefed());
		if(err != noErr)
		{
			SK_TRACE(SK_LOG_INFO, "Failed to get the first field.") ;
			return err;
		}
	}
	else
	{
		err = pRsFldCollection->GetFieldByName(psField.c_str(), xiField.already_AddRefed());
		if(err != noErr)
		{
			SK_TRACE(SK_LOG_INFO, "Failed to get the field whose name is %s.", psField.c_str()) ;
			return err;
		}
	}
	PRUint32 count = 0;
	err = xiRs->GetCount(&count);
	if(err != noErr)
	{
		SK_TRACE(SK_LOG_INFO, "Failed to get count.") ;
		return err;
	}

	err = xiRs->ExtractCursor(xiField, 0, count, pCursor);
	if(err != noErr)
	{
		SK_TRACE(SK_LOG_INFO, "Failed to extract cursor.") ;
		return err;
	}

	return noErr;
}



SKERR FileSystemService::prepare(std::string const& fileSystemPath, SKFileSystem ** fileSystem)
{
	FilesystemCache::const_iterator it = filesystemCache.find(fileSystemPath);
	if(filesystemCache.end() != it)
	{
		// already exist
		*fileSystem = it->second;
		return noErr;
	}

	SKFileSystem * pFileSystem = new SKFileSystem;
	SKERR err = pFileSystem->SetFileName(fileSystemPath.c_str());
	if(err != noErr)
	{
		SK_TRACE(SK_LOG_INFO,  "Failed to load entry filesystem ('%s'). error = %d", 
			fileSystemPath.c_str(), err);
		return err;
	}

	this->filesystemCache.insert(FilesystemCache::value_type(fileSystemPath, pFileSystem));
	*fileSystem = pFileSystem;
	return noErr;
}



SKERR FileSystemService::getFileData(std::string const& fileSystem, std::string const& path, skPtr<SKBinary> & pData)
{
	SKERR err = noErr;

	SKFileSystem * pSKFileSystem = NULL;
	err = prepare(fileSystem, &pSKFileSystem);
	if(noErr != err)
		return err;

	skPtr<SKFSFile> pFile;
	if(path.at(0) == '@')
	{
		string idString = path.substr(1);
		PRUint32 id = atoi(idString.c_str());

		// get file by id
		SK_TRACE(SK_LOG_DEBUG, "getFileData : get file by id (%d)", id);
		err = pSKFileSystem->GetFile (id, pFile.already_AddRefed());
		if(noErr != err)
		{
			SK_TRACE(SK_LOG_INFO,  "Failed to get '%s' file in '%s' file system. error = %d", 
				path.c_str(), fileSystem.c_str(), err);
			return err;
		}
	}
	else
	{
		SK_TRACE(SK_LOG_DEBUG, "getFileData : get root dir");
		skPtr<SKFSDirectory> pRootDir;
		err = pSKFileSystem->GetRootDir(pRootDir.already_AddRefed());
		if(err != noErr)
		{
			SK_TRACE(SK_LOG_INFO,  "Failed to get root directory. error = %d", err);
			return err;
		}

		string::size_type pos = string::npos;
		string::size_type offset = 0;
		skPtr<SKFSDirectory> pCurrentDir = pRootDir;
		skPtr<SKFSDirectory> pSubDir;
		for(pos = path.find('/', offset); pos != string::npos; offset = pos + 1, pos = path.find('/', offset))
		{
			string dirName = path.substr(offset, pos - offset);
			SK_TRACE(SK_LOG_DEBUG, "getFileData : get dir (%s)", dirName.c_str());
			err = pCurrentDir->GetDir(dirName.c_str(), pSubDir.already_AddRefed());
			if(err != noErr)
			{
				SK_TRACE(SK_LOG_INFO,  "Failed to get the %s sub directory. error = %d", 
					path.substr(0, pos).c_str(), err);
				return err;
			}
			pCurrentDir = pSubDir;
		}

		string filename = path.substr(offset);
		SK_TRACE(SK_LOG_DEBUG, "getFileData : get file (%s)", filename.c_str());
		err = pCurrentDir->GetFile(filename.c_str(), pFile.already_AddRefed());
		if(err != noErr)
		{
			SK_TRACE(SK_LOG_INFO,  "Failed to get the %s file. error = %d", path.c_str(), err);
			return err;
		}
	}

	SK_TRACE(SK_LOG_DEBUG, "getFileData : get file data");
	err = pFile->GetData(pData.already_AddRefed());
	if(err != noErr)
	{
		SK_TRACE(SK_LOG_INFO,  "Failed to get the file data (%s/%s). error = %d", 
			pFile->GetSharedPath(), pFile->GetSharedName(), err);
		return err;
	}

	return noErr;
}



SKERR FileSystemService::getFileData(std::string const& fileSystem, std::string const& path, std::vector<uint8_t>& buffer)
{
	SKERR err = noErr;

	skPtr<SKBinary> pData;
	err = this->getFileData(fileSystem, path, pData);
	if(noErr != err)
		return err;

	uint8_t * data = (uint8_t *)pData->GetSharedData();
	pData->GetSize();
	std::copy(data, data + pData->GetSize(), std::back_inserter(buffer));
	return noErr;
}



SKERR FileSystemService::getTextFileData(std::string const& fileSystem, std::string const& path, std::string& buffer)
{
	SKERR err = noErr;

	skPtr<SKBinary> pData;
	err = this->getFileData(fileSystem, path, pData);
	if(noErr != err)
		return err;

	buffer = (char const*)pData->GetSharedData();
	return noErr;
}



SKERR FileSystemService::copyFile(std::string const& fileSystem, std::string const& path, std::string const& destPath)
{
	SKERR err = noErr;

	skPtr<SKBinary> pData;
	err = this->getFileData(fileSystem, path, pData);
	if(noErr != err)
		return err;

	err = pData->WriteToFile(destPath.c_str());
	if(err != noErr)
	{
		SK_TRACE(SK_LOG_INFO, "Failed to copy %s' to '%s'.", path.c_str(), destPath.c_str());
		return err;
	}

	return noErr;
}

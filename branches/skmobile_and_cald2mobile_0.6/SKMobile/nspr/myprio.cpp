
#include "prtypes.h"
#include "prlog.h"
#include "prio.h"

#include <atlbase.h>
#include <atlconv.h>

using namespace ATL;

PRFileDesc* PR_Open(const char *name, PRIntn osflags, PRIntn mode)
{
	HANDLE file;
	PRInt32 access = 0;
	PRInt32 flags = 0;
	PRInt32 flag6 = 0;

	if (osflags & PR_SYNC) flag6 = FILE_FLAG_WRITE_THROUGH;

	if (osflags & PR_RDONLY || osflags & PR_RDWR)
		access |= GENERIC_READ;
	if (osflags & PR_WRONLY || osflags & PR_RDWR)
		access |= GENERIC_WRITE;

	if ( osflags & PR_CREATE_FILE && osflags & PR_EXCL )
		flags = CREATE_NEW;
	else if (osflags & PR_CREATE_FILE) {
		if (osflags & PR_TRUNCATE)
			flags = CREATE_ALWAYS;
		else
			flags = OPEN_ALWAYS;
	} else {
		if (osflags & PR_TRUNCATE)
			flags = TRUNCATE_EXISTING;
		else
			flags = OPEN_EXISTING;
	}

	file = CreateFile(CA2W(name, CP_UTF8),
		access,
		FILE_SHARE_READ|FILE_SHARE_WRITE,
		NULL,
		flags,
		flag6,
		NULL);

	if (file == INVALID_HANDLE_VALUE) {
		return NULL;
	}
	PRFileDesc * fd = new PRFileDesc;
	fd->handle = file;
	return fd;
}


PRStatus    PR_Close(PRFileDesc *fd)
{
	if(CloseHandle(fd->handle))
	{
		delete fd;
		return PR_SUCCESS;
	}
	else
		return PR_FAILURE;
}


PRInt32		PR_Read(PRFileDesc *fd, void *buf, PRInt32 amount)
{
	DWORD bytes;
	int rv, err;

	rv = ReadFile(
		fd->handle,
		(LPVOID)buf,
		amount,
		&bytes,
		NULL);

	if (rv == 0) 
	{
		err = GetLastError();
		/* ERROR_HANDLE_EOF can only be returned by async io */
		PR_ASSERT(err != ERROR_HANDLE_EOF);
		if (err == ERROR_BROKEN_PIPE)
			return 0;
		else {
			return -1;
		}
	}
	return bytes;
}


PRInt32		PR_Write(PRFileDesc *fd,const void *buf,PRInt32 amount)
{
	HANDLE f = fd->handle;
	DWORD bytes;
	int rv;

	rv = WriteFile(
		fd->handle,
		buf,
		amount,
		&bytes,
		NULL );

	if (rv == 0) 
	{
		return -1;
	}
	return bytes;
}


PRStatus	PR_Delete(const char *name)
{
	BOOL rc = DeleteFile(CA2W(name, CP_UTF8));
	if (rc) {
		return PR_SUCCESS;
	} else {
		return PR_FAILURE;
	}
}


PROffset32	PR_Seek(PRFileDesc *fd, PROffset32 offset, PRSeekWhence whence)
{
	DWORD moveMethod;
	PROffset32 rv;

	switch (whence) {
		case PR_SEEK_SET:
			moveMethod = FILE_BEGIN;
			break;
		case PR_SEEK_CUR:
			moveMethod = FILE_CURRENT;
			break;
		case PR_SEEK_END:
			moveMethod = FILE_END;
			break;
		default:
			return -1;
	}

	rv = SetFilePointer(fd->handle, offset, NULL, moveMethod);
	return rv;
}


PRStatus PR_Access(const char *name, PRAccessHow how)
{
	switch (how) {
	case PR_ACCESS_EXISTS:
		{
			  DWORD attr = ::GetFileAttributes(CA2W(name, CP_UTF8));
			  if(0xFFFFFFFF == attr)
				  return PR_FAILURE;
			  else
				  return PR_SUCCESS;
		}
		break;
	default:
		return PR_FAILURE;
	}
	return PR_FAILURE;
}


PRStatus PR_GetFileInfo(const char *fn, PRFileInfo *info)
{
	HANDLE hFindFile;
	WIN32_FIND_DATA findFileData;

	if (NULL == fn || '\0' == *fn) {
		return PR_FAILURE;
	}

	/*
	* FindFirstFile() expands wildcard characters.  So
	* we make sure the pathname contains no wildcard.
	*/
	if (NULL != strpbrk(fn, "?*")) {
		return PR_FAILURE;
	}

	hFindFile = FindFirstFile(CA2W(fn, CP_UTF8), &findFileData);
	if (INVALID_HANDLE_VALUE == hFindFile) {
		/*
		* FindFirstFile() does not work correctly on root directories.
		* It also doesn't work correctly on a pathname that ends in a
		* slash.  So we first check to see if the pathname specifies a
		* root directory.  If not, and if the pathname ends in a slash,
		* we remove the final slash and try again.
		*/
		return PR_FAILURE;
	}

	FindClose(hFindFile);

	if (findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
		info->type = PR_FILE_DIRECTORY;
	} else {
		info->type = PR_FILE_FILE;
	}

	PROffset64 size = findFileData.nFileSizeHigh;
	size = (size << 32) + findFileData.nFileSizeLow;
	info->size = (PROffset32)size;

	return PR_SUCCESS;
}


PRStatus PR_GetOpenFileInfo(PRFileDesc * fd, PRFileInfo *info)
{
	int rv;

	BY_HANDLE_FILE_INFORMATION hinfo;

	rv = GetFileInformationByHandle((HANDLE)fd->handle, &hinfo);
	if (rv == FALSE) {
		return PR_FAILURE;
	}

	if (hinfo.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		info->type = PR_FILE_DIRECTORY;
	else
		info->type = PR_FILE_FILE;

	PROffset64 size = hinfo.nFileSizeHigh;
	size = (size << 32) + hinfo.nFileSizeLow;
	info->size = (PROffset32)size;

	return PR_SUCCESS;
}


PRStatus PR_MkDir(const char *name, PRIntn mode)
{
	/* XXXMB - how to translate the "mode"??? */
	BOOL rc = CreateDirectory(CA2W(name, CP_UTF8), NULL);
	if (rc) {
		return PR_SUCCESS;
	} else {
		return PR_FAILURE;
	}
}
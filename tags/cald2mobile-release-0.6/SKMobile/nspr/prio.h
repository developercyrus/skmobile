#pragma once

#define PR_RDONLY       0x01
#define PR_WRONLY       0x02
#define PR_RDWR         0x04
#define PR_CREATE_FILE  0x08
#define PR_APPEND       0x10
#define PR_TRUNCATE     0x20
#define PR_SYNC         0x40
#define PR_EXCL         0x80


#define PR_fprintf		fprintf
#define PR_vfprintf		vfprintf

struct PRFileDesc {
	HANDLE handle;
};

PRFileDesc* PR_Open(const char *name, PRIntn flags, PRIntn mode);
PRStatus    PR_Close(PRFileDesc *fd);
PRInt32		PR_Read(PRFileDesc *fd, void *buf, PRInt32 amount);
PRInt32		PR_Write(PRFileDesc *fd,const void *buf,PRInt32 amount);
PRStatus	PR_Delete(const char *name);

typedef enum PRSeekWhence {
	PR_SEEK_SET = 0,
	PR_SEEK_CUR = 1,
	PR_SEEK_END = 2
} PRSeekWhence;

PROffset32	PR_Seek(PRFileDesc *fd, PROffset32 offset, PRSeekWhence whence);


typedef enum PRAccessHow {
	PR_ACCESS_EXISTS = 1,
	PR_ACCESS_WRITE_OK = 2,
	PR_ACCESS_READ_OK = 3
} PRAccessHow;

PRStatus PR_Access(const char *name, PRAccessHow how);


typedef enum PRFileType
{
	PR_FILE_FILE = 1,
	PR_FILE_DIRECTORY = 2,
	PR_FILE_OTHER = 3
} PRFileType;

struct PRFileInfo {
	PRFileType type;        /* Type of file */
	PROffset32 size;        /* Size, in bytes, of file's contents */
};

PRStatus PR_GetFileInfo(const char *fn, PRFileInfo *info);

PRStatus PR_GetOpenFileInfo(PRFileDesc * fd, PRFileInfo *info);

PRStatus PR_MkDir(const char *name, PRIntn mode);
/*
** strbuf.h
**
** Contains all header information for string buffering functions
*/

#ifndef __STRBUF_H__
#define __STRBUF_H__

/* Forward declarations */

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdlib.h>
#include <crtdbg.h>

/* # of buffers to start with & the size of each buffer */
#define STRBUF_STARTNBR 100
#define STRBUF_SIZE 4096

typedef struct StrBufSegment StrBufSegment;

typedef struct StrBuffer
{
	StrBufSegment		*head;								/* Pointer to first segment */
	StrBufSegment		*tail;								/* Pointer to end segment */
	unsigned int		length;								/* Length of string */
	unsigned int		offset;								/* Relative position of start text */
} StrBuffer;

//#pragma pack(push, 1)
typedef struct StrBufSegment
{
	StrBufSegment *next;
	char data[STRBUF_SIZE - 16];
} StrBufSegment;
//#pragma pack(pop)

#define StrBuf_IsEmpty(sb)					((sb)->length == 0)
#define StrBuf_Length(sb)					((sb)->length)
#define	StrBuf_String(sb)					((sb)->string)

/* String buffer functions */

/* Called at beginning, sets up initial buffers */
void			StrBuf_Init();
void			StrBuf_Destroy();

/* Called whenever a buffer segment is needed */
StrBufSegment	*StrBufSeg_Alloc();

/* Called whenever a buffer segment can be freed */
void			StrBufSeg_Free(StrBufSegment *sbsTarget);

/* Adds text to buffer, returns TRUE if successful */
BOOL			StrBuf_Add(StrBuffer *sbTarget, const char *szSource, int nBufLen);

/* Deletes text from buffer */
unsigned int	StrBuf_Delete(StrBuffer *sbTarget, int nChars);

/* Empties the specified buffer */
void StrBuf_Empty(StrBuffer *sbTarget);

/* Copies up to <nChars> characters from beginning of buffer to szOutput. Returns # of bytes copied */
unsigned int	StrBuf_Get(StrBuffer *sbTarget, char *szOutput, int nChars);

/* Returns a pointer to the largest contigious block of data in the buffer */
char			*StrBuf_GetPtr(StrBuffer *sbTarget, int *pnLength);

/* Clone of Srfbuf_GetPtr but instead returns position of CRLF */
char			*StrBuf_GetPtrCRLF(StrBuffer *sbTarget, int *pnCRLFStartPos, int *pnCRLFEndPos, int nLength, char *szOverflow);

#endif		/* __STRBUF_H__ */
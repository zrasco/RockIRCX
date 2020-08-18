/*
** strbuf.cpp
**
** This file contains all functions for the string buffering system used by 
** RockIRCX
*/

#include "strbuf.h"
#include <stdlib.h>

#define SBSSIZ sizeof(((StrBufSegment *)(NULL))->data)

StrBufSegment sbsAvail;
static unsigned int g_szBufferCount = 0;
static unsigned int g_szBufOnFL = 0;

void StrBuf_Init()
/*
** StrBuf_Init()
** Sets up everything used locally
*/
{
	LPVOID *lpBlock;

	StrBufSegment *sbsptr = &sbsAvail;
	int nCount;
	size_t test = sizeof(StrBufSegment);

	sbsAvail.next = NULL;

	for (nCount = 0; nCount < STRBUF_STARTNBR; nCount++)
	{
		sbsptr->next = (StrBufSegment*)(malloc(sizeof(StrBufSegment)));
		
		if (sbsptr->next)
		{
			sbsptr = sbsptr->next;
			g_szBufferCount++;
			g_szBufOnFL++;
		}
	}

	sbsptr->next = NULL;

}

void StrBuf_Destroy()
/*
** Called when program ends, de-allocates string buffers
*/
{
	StrBufSegment *sbsptr;

	while (sbsAvail.next)
	{
		sbsptr = sbsAvail.next;

		sbsAvail.next = sbsAvail.next->next;

		free(sbsptr);
		g_szBufferCount--;
	}
}

StrBufSegment *StrBufSeg_Alloc()
/*
** StrBuf_Alloc()
** Grabs a free buffer or allocates a new one
*/
{
	StrBufSegment *sbsLocal;

	if (sbsAvail.next)
	/* Grab a buffer from the available buffers list */
	{
		sbsLocal = sbsAvail.next;

		sbsAvail.next = sbsAvail.next->next;

		sbsLocal->next = NULL;

		g_szBufOnFL--;
	}
	else
	/* Create new buffer */
	{
		sbsLocal = (StrBufSegment*)(malloc(sizeof(StrBufSegment)));
		g_szBufferCount++;
	}

	return sbsLocal;
}

void StrBuf_Empty(StrBuffer *sbTarget)
/*
** StrBuf_Empty()
** Frees buffers specified
*/
{
	StrBufSegment *sbsptr;

	while (sbTarget->head)
	{
		sbsptr = sbTarget->head->next;
		StrBufSeg_Free(sbTarget->head);
		sbTarget->head = sbsptr;
	}

	sbTarget->offset = 0;
	sbTarget->length = 0;

	sbTarget->tail = NULL;
}

void StrBufSeg_Free(StrBufSegment *sbsTarget)
/*
** StrBufSeg_Free()
** Returns a buffer segment to the free list
*/
{
	StrBufSegment *sbsOldFront = sbsAvail.next;

	/* Attach segment to beginning of free list */
	sbsAvail.next = sbsTarget;
	sbsTarget->next = sbsOldFront;

	g_szBufOnFL++;
}

BOOL StrBuf_Add(StrBuffer *sbTarget, const char *szSource, int nBufLen)
/*
** Strbuf_Add()
** Adds the user-supplied buffer pointed to by szSource to the targeted string buffer
** Returns TRUE if successful
*/
{
	StrBufSegment **h, *d;

	int nOffset, nChunk;

	if (nBufLen == 0)
		return FALSE;

	nOffset = (sbTarget->offset + sbTarget->length) % SBSSIZ;

	/*
	** Locate the last non-empty buffer. If the last buffer is
	** full, the loop will terminate with 'd==NULL'. This loop
	** assumes that the 'dyn->length' field is correctly
	** maintained, as it should--no other check really needed.
	*/
	if (!sbTarget->length)
		h = &(sbTarget->head);
	else
	{
	  if (nOffset)
		  h = &(sbTarget->tail);
	  else
		  h = &(sbTarget->tail->next);
	}
	/*
	** Append users data to buffer, allocating buffers as needed
	*/

	nChunk = SBSSIZ - nOffset;
	sbTarget->length += nBufLen;

	for ( ; nBufLen > 0; h = &(d->next))
	{
		d = *h;

		if (d == NULL)
		{
			d = StrBufSeg_Alloc();

			if (d == NULL)
				return FALSE;

			sbTarget->tail = d;
			*h = d;
			d->next = NULL;
		}

		if (nChunk > nBufLen)
			nChunk = nBufLen;

		memcpy(&d->data[0] + nOffset, szSource, nChunk);

		nBufLen -= nChunk;
		szSource += nChunk;
		nOffset = 0;
		nChunk = SBSSIZ;
	}

	return TRUE;

}

/* Copies up to <nChars> characters from beginning of buffer to szOutput. Returns # of bytes copied */
unsigned int StrBuf_Get(StrBuffer *sbTarget, char *szOutput, int nChars)
{
	int	nMoved = 0;
	int	nChunk;
	char *b;

	while (nChars > 0 && (b = StrBuf_GetPtr(sbTarget, &nChunk)) != NULL)
	{
		if (nChunk > nChars)
			nChunk = nChars;

		memcpy(szOutput,b,nChunk);
		StrBuf_Delete(sbTarget,nChunk);

		szOutput += nChunk;
		nChars -= nChunk;
		nMoved += nChunk;
	}
	return nMoved;
}

/* Returns a pointer to the largest contigious block of data in the buffer */
char *StrBuf_GetPtr(StrBuffer *sbTarget, int *pnLength)
{
	if (sbTarget->head == NULL)
	{
		sbTarget->tail = NULL;
		*pnLength = 0;
		return NULL;
	}

	*pnLength = SBSSIZ - sbTarget->offset;

	if (*pnLength > sbTarget->length)
		*pnLength = sbTarget->length;

	return (&sbTarget->head->data[0] + sbTarget->offset);
}

/* Clone of Srfbuf_GetPtr but instead of length returns position of CRLF */
char *StrBuf_GetPtrCRLF(StrBuffer *sbTarget, int *pnCRLFStartPos, int *pnCRLFEndPos, int nLength, char *szOverflow)
{
	StrBufSegment *d, *p;
	char *s, *n;
	int	dlen, i, copy, ov_offset = 0, nPos = 0, nVal;
	BOOL bCRLF;

getmsg_init:

	bCRLF = FALSE;

	*pnCRLFStartPos = 0;
	d = sbTarget->head;
	dlen = sbTarget->length;
	i = SBSSIZ - sbTarget->offset;

	if (i <= 0)
		return NULL;
	
	copy = 0;
	
	if (d && dlen)
		s = sbTarget->offset + &d->data[0];
	else
		return NULL;

	if (i > dlen)
		i = dlen;
	while (nLength > 0 && dlen > 0)
	{
		dlen--;

		if (*s == '\n' || *s == '\r')
		{
			*pnCRLFStartPos = nPos;

			if (!dlen || ((*(s + 1) != '\r') && (*(s + 1) != '\n')))
			/* End of string, or CR/LF by itself */
				*pnCRLFEndPos = *pnCRLFStartPos;

			if (dlen && *(s + 1) == '\n')
				*pnCRLFEndPos = *pnCRLFStartPos + 1;

			bCRLF = TRUE;
			copy = sbTarget->length - dlen;

			if (copy == 1)
			{
				StrBuf_Delete(sbTarget, 1);
				goto getmsg_init;
			}

			if (ov_offset)
			/* Last buffer segment */
			{
				nVal = (nPos % SBSSIZ);

				memcpy(&szOverflow[ov_offset], &d->data[0],nVal);
				ov_offset += nVal;
			}

			break;
		}

		nLength--;
		if (!--i)
		{
			p = d;
			if ((d = d->next))
			/* Go to the next buffer segment */
			{
				/* Copy the block contents to the overflow buffer */
				
				if (p == sbTarget->head)
				/* First buffer segment */
				{
					memcpy(&szOverflow[ov_offset], &p->data[0] + sbTarget->offset,SBSSIZ - sbTarget->offset);
					ov_offset += SBSSIZ - sbTarget->offset;
				}
				else
				/* Intermediate buffer segment */
				{
					memcpy(&szOverflow[ov_offset], &p->data[0],SBSSIZ);
					ov_offset += SBSSIZ;
				}

				nPos++;
				s = &d->data[0];
				i = __min(SBSSIZ, dlen);
			}
		}
		else
		{
			s++;
			nPos++;
		}
	}

	if (bCRLF)
	/* Point to beginning of buffer or to overflow buffer */
	{
		if (!ov_offset)
			return (&sbTarget->head->data[0] + sbTarget->offset);
		else
			return szOverflow;
	}
	else
		return NULL;
}

/* Deletes text from buffer */
unsigned int StrBuf_Delete(StrBuffer *sbTarget, int nChars)
/*
** StrBuf_Delete()
** Deletes text from buffer up to <nChars>, freeing buffers as available
*/
{
	StrBufSegment *d;
	int nChunk;

	if (nChars > sbTarget->length)
		nChars = sbTarget->length;
	nChunk = SBSSIZ - sbTarget->offset;

	while (nChars > 0)
	{
		if (nChunk > nChars)
			nChunk = nChars;

		nChars -= nChunk;
		sbTarget->offset += nChunk;
		sbTarget->length -= nChunk;

		if (sbTarget->offset == SBSSIZ || sbTarget->length == 0)
		{
			d = sbTarget->head;
			sbTarget->head = d->next;
			sbTarget->offset = 0;
			StrBufSeg_Free(d);
		}

		nChunk = SBSSIZ;
	}

	if (sbTarget->head == NULL)
	{
		sbTarget->length = 0;
		sbTarget->tail = 0;
	}

	return 0;
}
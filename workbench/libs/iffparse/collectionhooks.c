/*
    (C) 1995-96 AROS - The Amiga Replacement OS
    $Id$

    Desc: Hook functions neede for ColectionChunk() entry handlers.
    Lang: English.
*/

#include "iffparse_intern.h"



/****************************/
/* CollectionLCI Purge func  */
/****************************/

ULONG CollectionPurgeFunc
(
    struct Hook 	    * hook,
    struct LocalContextItem * lci,
    ULONG		      p,
    struct IFFParseBase     * IFFParseBase
)
{
    struct CIPtr	   *ciptr;

    struct CollectionItem  *node,
			  *nextnode;

    /* Read the LCI's userdata
    */
    ciptr = (struct CIPtr*)LocalItemData(lci);

    /* Free all collectionitems in the linked list */
    node = ciptr->FirstCI;

    while (node)
    {
	nextnode = node->ci_Next;
	FreeMem(node->ci_Data, node->ci_Size);

	FreeMem(node,sizeof (struct CollectionItem));

	node = nextnode;
    }

    /* Free the local item itself */
    FreeLocalItem(lci);

    return (NULL);
}



/**********************************/
/* CollectionChunk entry-handler  */
/**********************************/

struct CF_ResourceInfo
{
    struct LocalContextItem *LCI;
    APTR		    Buffer;
    LONG		    BufferSize;
    BOOL		    LCIStored;
    struct CollectionItem    *CollItem;
};


VOID CF_FreeResources (struct CF_ResourceInfo * ri,
	struct IFFParseBase * IFFParseBase)
{
    if (ri->LCIStored)  Remove((struct Node*)ri->LCI);
    if (ri->LCI)        FreeLocalItem(ri->LCI);
    if (ri->Buffer)     FreeMem(ri->Buffer,   ri->BufferSize);
    if (ri->CollItem)   FreeMem(ri->CollItem,  sizeof (struct CollectionItem));

    return;
}


LONG CollectionFunc
(
    struct Hook 	* hook,
    struct IFFHandle	* iff,
    APTR		  p,
    struct IFFParseBase * IFFParseBase
)
{
    extern struct Hook collectionpurgehook;

    struct LocalContextItem *lci;

    struct ContextNode	    *cn;
    struct CIPtr	    *ciptr;
    struct CollectionItem    *collitem;

    struct CF_ResourceInfo resinfo = {0}; /* = {0} is important */



    LONG   type,
	  id,
	  size;

    LONG  bytesread,
	  err;

    APTR buf;

    /* The Chunk that caused us to be invoked is always the top chunk */
    cn = TopChunk(iff);

    type   = cn->cn_Type;
    id	  = cn->cn_ID;

    /* IMPORTANT !! For collectionchunks we MUST check if a collection is allready present,
    if so there is no clever to add a new one */

    lci = FindLocalItem
    (
	iff,
	type,
	id,
	IFFLCI_COLLECTION
    );

    if (!lci)
    {

	/* Allocate new LCI for containing the property */

	lci = AllocLocalItem
	(
	    type,
	    id,
	    IFFLCI_COLLECTION,
	    sizeof (struct CIPtr)
	);
	if (!lci) return (IFFERR_NOMEM);

	resinfo.LCI = lci;

	/* Store the new LCI into top of stack */

	err = StoreLocalItem(iff,lci,IFFSLI_ROOT);

	if (err)
	{
	    CF_FreeResources(&resinfo, IFFParseBase);
	    return (err);
	}
	resinfo.LCIStored = TRUE;

	SetLocalItemPurge(lci,(struct Hook *)&collectionpurgehook);

    }

    /* Allocate a new CollectionItem */

    collitem = (struct CollectionItem*)AllocMem
    (
	sizeof (struct CollectionItem),
	MEMF_ANY|MEMF_CLEAR
    );

    if (!collitem)
    {
	CF_FreeResources(&resinfo, IFFParseBase);
	return (IFFERR_NOMEM);
    }

    resinfo.CollItem = collitem;



    /* Allocate buffer to read chunk into */
    size = cn->cn_Size;

    buf = AllocMem
    (
	size,
	MEMF_ANY
    );

    if (!buf)
    {
	CF_FreeResources(&resinfo, IFFParseBase);
	return (IFFERR_NOMEM);
    }

    resinfo.Buffer = buf;
    resinfo.BufferSize = size;


    /* Read chunk into the buffer */

    bytesread = ReadChunkBytes
    (
	iff,
	buf,
	size
    );

    /* Sucess ? */
    if (bytesread != size)
    {
	CF_FreeResources(&resinfo, IFFParseBase);

	/* IFFERR_.. ? */
	if (bytesread >= 0)
	    err = IFFERR_MANGLED;
    }


    /* Get pointer to first ContextItem from LCIs userdata */
    ciptr = (struct CIPtr*)LocalItemData(lci);

    /* Update pointers in linked list of collectionitems */
    collitem->ci_Next = ciptr->FirstCI;
    ciptr->FirstCI = collitem;

    collitem->ci_Data = buf;
    collitem->ci_Size = size;


    return (NULL);
}

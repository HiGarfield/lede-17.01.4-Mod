#include <string.h>
#include <assert.h>
#include "bst.h"
#include "utils.h"
#include "logs.h"

PRIFUNC Bst_NodeHead *GetUnusedNode(Bst *t)
{
    if( t->FreeList == NULL )
    {
        return t->Nodes.Add(&(t->Nodes),
                            NULL,
                            sizeof(Bst_NodeHead) + t->ElementLength,
                            TRUE
                            );
    } else {
        Bst_NodeHead *ret = t->FreeList;

        t->FreeList = ret->Right;

        return ret;
    }
}

PRIFUNC const void *InsertNode(Bst          *t,
                               Bst_NodeHead *ParentNode,
                               int          CompareResult,
                               const void   *Data
                               )
{
    Bst_NodeHead *NewNode = GetUnusedNode(t);

    if( NewNode == NULL )
    {
        return NULL;
    }

    /* Set parent node */
    if( ParentNode == NULL )
    {
        /* Insert as the root */
        t->Root = NewNode;
    } else {
        /* Non-root */
        if( CompareResult <= 0 )
        {
            assert(ParentNode->Left == NULL);
            ParentNode->Left = NewNode;
        } else {
            assert(ParentNode->Right == NULL);
            ParentNode->Right = NewNode;
        }
    }

    /* Set the new child node */
    NewNode->Parent = ParentNode;
    NewNode->Left = NULL;
    NewNode->Right = NULL;

    /* Copy the data */
    memcpy(NewNode + 1, Data, t->ElementLength);

    /* Return the data position */
    return (const void *)(NewNode + 1);
}

PUBFUNC const void *Bst_Add(Bst *t, const void *Data)
{
    if( t->Root == NULL )
    {
        /* Insert as root */
		return InsertNode(t, NULL, 0, Data);
    } else {
        /* Non-root, finding the currect place to insert */
        Bst_NodeHead *Current = t->Root;

        while( TRUE )
        {
            int CompareResult = (t->Compare)(Data, (const void *)(Current + 1));

            if( CompareResult <= 0 )
            {
                /* Left branch */
                Bst_NodeHead *Left = Current->Left;
                if( Left == NULL )
                {
                    /* Insert as a left child */
                    return InsertNode(t, Current, CompareResult, Data);
                }

                Current = Left;
            } else {
                /* Right branch */
                Bst_NodeHead *Right = Current->Right;
                if( Right == NULL )
                {
                    /* Insert as a right child */
                    return InsertNode(t, Current, CompareResult, Data);
                }

                Current = Right;
            }
        }
    }
}

PUBFUNC const void *Bst_Search(Bst *t, const void *Key, const void *Last)
{
    Bst_NodeHead *Current;

    /* Set the starting point */
    if( Last == NULL )
    {
        /* root as the starting point */
        Current = t->Root;
    } else {
        Current = (((Bst_NodeHead *)Last) - 1)->Left;
    }

    while( Current != NULL )
    {
        int CompareResult = (t->Compare)(Key, (const void *)(Current + 1));

        if( CompareResult == 0 )
        {
            return (const void *)(Current + 1);
        } else if( CompareResult < 0 ){
            Current = Current->Left;
        } else /** CompareResult > 0 */{
            Current = Current->Right;
        }
    }

    return NULL;
}

PRIFUNC void Bst_Enum_Inner(Bst *t,
                            Bst_NodeHead *n,
                            Bst_Enum_Callback cb,
                            void *Arg,
                            BOOL *StopFlag
                            )
{
    if( n == NULL || *StopFlag )
    {
        return;
    }

    *StopFlag = cb(t, n + 1, Arg) != 0;

    Bst_Enum_Inner(t, n->Left, cb, Arg, StopFlag);
    Bst_Enum_Inner(t, n->Right, cb, Arg, StopFlag);
}

PUBFUNC void Bst_Enum(Bst *t, Bst_Enum_Callback cb, void *Arg)
{
    BOOL StopFlag = FALSE;
    Bst_Enum_Inner(t, t->Root, cb, Arg, &StopFlag);
}

PUBFUNC const void *Bst_Minimum(Bst *t, const void *Subtree)
{
    Bst_NodeHead *Current;

    if( Subtree == NULL )
    {
        /* Starting with the root */
        if( t->Root == NULL )
        {
            /* Empty tree */
            return NULL;
        }

        Current = t->Root;
    } else {
        Current = ((Bst_NodeHead *)Subtree) - 1;
    }

    while( Current->Left != NULL )
    {
        Current = Current->Left;
    }

    return (const void *)(Current + 1);
}

PUBFUNC const void *Bst_Successor(Bst *t, const void *Last)
{
    Bst_NodeHead *Current = ((Bst_NodeHead *)Last) - 1;

    if( Current->Right != NULL )
    {
        return Bst_Minimum(t, (Current->Right) + 1);
    } else {
        Bst_NodeHead *Parent = Current->Parent;

        while( Parent != NULL && Parent->Left != Current )
        {
            Current = Parent;
            Parent = Parent->Parent;
        }

        return Parent == NULL ? NULL : (const void *)(Parent + 1);
    }
}

PUBFUNC void Bst_Delete(Bst *t, const void *Node)
{
    Bst_NodeHead *Current = ((Bst_NodeHead *)Node) - 1;
    Bst_NodeHead *ActuallyRemoved, *Child;

    /* Finding the node that will be actually removed. */
    if( Current->Left == NULL || Current->Right == NULL )
    {
        /* If Current has one or no child */
        ActuallyRemoved = Current;
    } else {
        /* If Current has two child */
        ActuallyRemoved = ((Bst_NodeHead *)Bst_Successor(t, Current + 1)) - 1;
    }

    /* If ActuallyRemoved:
        has two child, impossible case,
        has only one child, get the child,
        or no child, set it to NULL
    */
    if( ActuallyRemoved->Left != NULL )
    {
        Child = ActuallyRemoved->Left;
    } else {
        Child = ActuallyRemoved->Right;
    }

    /* If ActuallyRemoved has one child ( Child != NULL ) */
    if( Child != NULL )
    {
        /* Set the child's parent to its parent's parent */
        Child->Parent = ActuallyRemoved->Parent;
    }

    if( ActuallyRemoved->Parent == NULL )
    {
        /* If ActuallyRemoved is the root */

        t->Root = Child;
    } else {
        /* Or not the root */

        if( ActuallyRemoved->Parent->Left == ActuallyRemoved )
        {
            /* If ActuallyRemoved is a left child */
            ActuallyRemoved->Parent->Left = Child;
        } else {
            /* Or a right child */
            ActuallyRemoved->Parent->Right = Child;
        }
    }

    if( ActuallyRemoved != Current )
    {
        /* Replace Current with ActuallyRemoved */

        /*memcpy(Current + 1, ActuallyRemoved + 1, t->ElementLength);*/

        Bst_NodeHead *CurrentParent = Current->Parent;
        Bst_NodeHead *CurrentLeft = Current->Left;
        Bst_NodeHead *CurrentRight = Current->Right;

        /* Parent */
        if( CurrentParent != NULL )
        {
            if( CurrentParent->Left == Current )
            {
                CurrentParent->Left = ActuallyRemoved;
            } else {
                CurrentParent->Right = ActuallyRemoved;
            }
        } else {
            t->Root = ActuallyRemoved;
        }

        /* Left Child */
        CurrentLeft->Parent = ActuallyRemoved;

        /* Right Child */
        if( CurrentRight != NULL )
        {
            CurrentRight->Parent = ActuallyRemoved;
        }

        /* ActuallyRemoved */
        ActuallyRemoved->Parent = CurrentParent;
        ActuallyRemoved->Left = CurrentLeft;
        ActuallyRemoved->Right = CurrentRight;

        ActuallyRemoved = Current;
    }

    ActuallyRemoved->Right = t->FreeList;
    t->FreeList = ActuallyRemoved;

    return;
}

PUBFUNC void Bst_Reset(Bst *t)
{
	t->Nodes.Clear(&(t->Nodes));
	t->Root = NULL;
	t->FreeList = NULL;
}

PUBFUNC void Bst_Free(Bst *t)
{
    t->Nodes.Free(&(t->Nodes));
}

int Bst_Init(Bst *t, int ElementLength, CompareFunc Compare)
{
	t->Compare = Compare;
	t->Root = NULL;
	t->FreeList = NULL;
	t->ElementLength = ElementLength;

	if( StableBuffer_Init(&(t->Nodes)) != 0 )
    {
        return -497;
    }

    t->Add = Bst_Add;
    t->Delete = Bst_Delete;
    t->Enum = Bst_Enum;
    t->Free = Bst_Free;
    t->Minimum = Bst_Minimum;
    t->Reset = Bst_Reset;
    t->Search = Bst_Search;
    t->Successor = Bst_Successor;

	return 0;
}

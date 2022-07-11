/*
 *  OpenGL SimpleSprite Class Suite
 *  (c) 2004 Scott Lahteine.
 *
 *  SS_Templates.h
 *
 *  $Id: SS_Templates.h,v 1.1 2007/03/02 08:05:45 slurslee Exp $
 *
 *  Derived from the book
 *  Data Structures for Game Programmers
 *  by Ron Penton
 *  ISBN 1-931841-94-2
 *
 *  TLinkedList is intended to contain whole data structures
 *  so it is best for lists which maintain their own data.
 *
 *  TObjectList is specifically designed for object pointers.
 *  It disposes the object when the node is disposed.
 *
 */

#ifndef __SS_TEMPLATES_H__
#define __SS_TEMPLATES_H__

#include <Carbon/Carbon.h>

#include <assert.h>
#include <stdlib.h>
//#include <stdio.h>
#include <SDL.h>
#include <stdexcept>

// forward declarations
template<class T> class TListNode;
template<class T> class TLinkedList;
template<class T> class TIterator;


//
// TArray
// Simple array class
//
template<class T>
class TArray
{
public:
    T           *m_array;
    UInt16      m_count;
    UInt16      block_size;

    TArray()                        { Init(); }
    TArray(const TArray &src)       { Init(); *this = src; }
    virtual ~TArray()               { Resize(0); }

    void Init()
    {
        m_array     = NULL;
        m_count     = 0;
        block_size  = 10;
    }

    inline UInt16   Size()          { return m_count; }
    virtual inline void Clear()     { Resize(0); }

    inline void ZeroElements()      { if (m_count) memset(m_array, 0, m_count * sizeof(T)); }

    inline void ZeroElements(UInt16 index, UInt16 size)
    {
        if (index >= m_count)
            return;

        if (index + size > m_count)     // 0 + 9 > 7?
            size = m_count - index;     // size = 7 - 0

        if (size)
            memset(&m_array[index], 0, size * sizeof(T));
    }

    //
    // Resize
    //
    void Resize(UInt16 size, bool inFront=false, bool noZero=false)
    {
        if (inFront)
            ExpandOrContract(0, size, noZero);
        else
            ExpandOrContract(m_count - size, size, noZero);
    }

    //
    // ExpandOrContract
    //
    // +size : Adds space ahead of the given index.
    // -size : Removes space beginning with the given index.
    //
    // Examples:
    //
    // ExpandOrContract(0, -1);     Delete Element 0
    // ExpandOrContract(0, 21);     Insert 21 elements before element 0
    // ExpandOrContract(12, 1);     Assuming m_count==12, adds one to the end
    //
    virtual void ExpandOrContract(UInt16 index, SInt16 size, bool noZero=false)
    {
        if (size == 0) return;

        if (size < 0)
            MoveElements(index - size, index, m_count - index);

        UInt16 old_blocks = (m_count + block_size - 1) / block_size;
        UInt16 new_blocks = (m_count + size + block_size - 1) / block_size;

        if (old_blocks != new_blocks)
        {
            if (m_array)
            {
                if (size)
                    m_array = (T*)realloc(m_array, sizeof(T) * new_blocks * block_size);
                else
                {
                    free(m_array);
                    m_array = NULL;
                }
            }
            else
            {
                if (size)
                {
                    m_array = (T*)malloc(sizeof(T) * new_blocks * block_size);      // this trounces "front"
                }
            }
        }

        // Insert new empty elements when expanding
        if (size > 0)
        {
            if (index < m_count)
                MoveElements(index, index + size, m_count - index);

            if (!noZero)
                ZeroElements(index, size);
        }

        m_count += size;
    }

    //
    // CopyElements / MoveElements
    //
    // Make copies of the elements. In a normal array this
    // copies whole elements from one address to another.
    // In an Object Array the elements are pointers, which
    // are copied or moved without making new objects.
    //
    inline void CopyElements(const T *src, T *dst, UInt16 size)     { if (src != dst) memmove(dst, src, size * sizeof(T)); }
    inline void CopyElements(UInt16 src, T *dst, UInt16 size)       { CopyElements(&m_array[src], dst, size); }
    inline void CopyElements(const T *src, UInt16 dst, UInt16 size) { CopyElements(src, &m_array[dst], size); }
    inline void MoveElements(UInt16 src, UInt16 dst, UInt16 size)   { CopyElements(&m_array[src], &m_array[dst], size); }

    //
    // Element
    //
    virtual T* Element(UInt16 index)
    {
        if (index < m_count)
            return &m_array[index];
        else
            return NULL;
    }

    //
    // operator[]
    //
    T& operator[](unsigned i)               { return m_array[i]; }
    const T& operator[](unsigned i) const   { return m_array[i]; }

    //
    // Append / Prepend / PopFirst / PopLast
    //
    virtual inline void Append(T& element)                  { InsertBefore(m_count, element); }
    virtual inline void Append(T *element, UInt16 count=1)  { InsertBefore(m_count, element, count); }
    virtual inline void Prepend(T& element)                 { InsertBefore(0, element); }
    virtual inline void Prepend(T *element, UInt16 count=1) { InsertBefore(0, element, count); }
    virtual inline void PopFirst(UInt16 count=1)            { Delete(0, count); }
    virtual inline void PopLast(UInt16 count=1)             { Delete(m_count - count, count); }
    virtual inline void Delete(UInt16 index, UInt16 count=1) { ExpandOrContract(index, -count); }

    //
    // InsertBefore / InsertAfter
    //
    virtual void InsertBefore(UInt16 index, T *element, UInt16 count=1)
    {
        ExpandOrContract(index, count, true);
        CopyElements(element, &m_array[index], count);
    }

    inline void InsertBefore(UInt16 index, T& element)                  { InsertBefore(index, &element); }
    inline void InsertAfter(UInt16 index, T& element)                   { InsertBefore(index+1, &element); }
    inline void InsertAfter(UInt16 index, T *element, UInt16 count=1)   { InsertBefore(index+1, element, count); }


    //
    // operator=
    // Copies the data from another array into this one.
    // Any existing array elements are thrown away first.
    //
    virtual TArray<T>& operator=(const TArray<T>& src)
    {
        if (this != &src)
        {
            Clear();

            if (src.m_count)
            {
                block_size  = src.block_size;
                Resize(src.m_count, false, true);
                CopyElements(&src[0], m_array, src.m_count);
            }
        }

        return *this;
    }
};


#pragma mark -
//
// TObjectArray
// Special array class for arrays of object pointers.
// New member objects are instantiated as the buffer size changes.
// Removed nodes are destroyed.
//
template<class T>
class TObjectArray : public TArray<T*>
{
public:
    TObjectArray()                          {}
    TObjectArray(const TObjectArray &src)   { *this = src; }
    ~TObjectArray()                         { Clear(); }

    //
    // Clear
    //
    inline void Clear()                     { DisposeAll(); this->Resize(0); }

    virtual void ExpandOrContract(UInt16 index, SInt16 size, bool noZero=false)
    {
        if (size < 0)
            DisposeMembers(index, index - size - 1);

        TArray<T*>::ExpandOrContract(index, size, noZero);
    }

    //
    // ConstructMembers
    //
    void ConstructMembers(UInt16 start, UInt16 end)
    {
        for (int i=start; i<=end; i++)
            this->m_array[i] = new T;
    }

    //
    // DisposeMembers / DisposeAll
    //
    void DisposeMembers(UInt16 start, UInt16 end)
    {
        if (start >= this->m_count)
            start = this->m_count - 1;

        if (end >= this->m_count)
            end = this->m_count - 1;

        if (start > end) { UInt16 s = start; start = end; end = s; }

        for (int i=start; i<=end; i++)
        {
            if (this->m_array[i] != NULL)
            {
                delete this->m_array[i];
                this->m_array[i] = NULL;
            }
        }
    }

    inline void DisposeAll()                { if (this->m_count) DisposeMembers(0, this->m_count - 1); }

    //
    // ConstructCopies
    // Makes duplicates of an array of objects
    // Fails if the buffers overlap.
    //
    //  Remember, every pointer is also an array.
    //  So this takes two pointers to arrays
    //
    inline void ConstructCopies(T* src[], T* dst[], UInt16 size)
    {
        if (size)
        {
            if (dst >= src + size || src >= dst + size)
            {
                for (int i=0; i<size; i++)
                    dst[i] = new T(*src[i]);
            }
            else
                throw "Misuse of ConstructCopies.";
        }
    }

    //
    // Object returns the object pointer of the element
    // of the element, as in the TArray class.
    // . . . And yet the signature is the same. Hmmm . . .
    //
    T* Object(UInt16 index)
    {
        if (index < this->m_count)
            return this->m_array[index];
        else
            return NULL;
    }

    //
    // operator[]
    // Returns the object reference (not a reference to the pointer)
    //
    T& operator[](unsigned i)               { return *this->m_array[i]; }
    const T& operator[](unsigned i) const   { return *this->m_array[i]; }

    //
    // AppendCopy / PrependCopy / InsertCopyBefore / InsertCopyAfter
    //
    inline void AppendCopy(T& element)                                      { AppendCopy(&element); }
    inline void PrependCopy(T& element)                                     { PrependCopy(&element); }

    inline void AppendCopy(T *element, UInt16 count=1)                      { InsertCopyBefore(this->m_count, element, count); }
    inline void PrependCopy(T *element, UInt16 count=1)                     { InsertCopyBefore(0, element, count); }

    virtual void InsertCopyBefore(UInt16 index, T** elemPtr, UInt16 count=1)
    {
        ExpandOrContract(index, count, true);
        ConstructCopies(elemPtr, &this->m_array[index], count);
    }

    inline void InsertCopyAfter(UInt16 index, T** elemPtr, UInt16 count=1)  { InsertCopyBefore(index+1, elemPtr, count); }

    inline void InsertCopyBefore(UInt16 index, T* element)                  { InsertCopyBefore(index, &element); }      // Calls list version
    inline void InsertCopyAfter(UInt16 index, T* element)                   { InsertCopyBefore(index+1, &element); }    // address of an auto ok?

    inline void InsertCopyBefore(UInt16 index, T& elemRef)                  { InsertCopyBefore(index, &elemRef); }      // Calls pointer version
    inline void InsertCopyAfter(UInt16 index, T& elemRef)                   { InsertCopyBefore(index+1, &elemRef); }    // address of a ref ok?

    //
    // Delete / PopFirst / PopLast
    //
    inline void Delete(UInt16 index, UInt16 count=1)
    {
        DisposeMembers(index, index+count-1);
        ExpandOrContract(index, -count);
    }

    inline void PopFirst(UInt16 count=1)    { if (count < this->m_count) { Delete(0, count); } else Clear(); }
    inline void PopLast(UInt16 count=1)     { if (count < this->m_count) { Delete(this->m_count-count, count); } else Clear(); }

    //
    // operator=
    //
    TObjectArray<T>& operator=(const TObjectArray<T>& src)
    {
        if (this != &src)
        {
            Clear();

            if (src.m_count)
            {
                this->block_size    = src.block_size;
                Resize(src.m_count, false, true);
                ConstructCopies(&src[0], &this->m_array[0], src.m_count);
            }
        }

        return *this;
    }
};

#pragma mark -
//
// TListNode
// Doubly-linked list node class
//
template<class T>
class TListNode
{
public:
    T                   m_data;
    TListNode<T>*       m_next;
    TListNode<T>*       m_prev;
    TLinkedList<T>* m_container;

    virtual ~TListNode() {}

    //
    // Unlink
    // Remove a node from the list it is in
    //
    virtual void Unlink()
    {
        DEBUGF(2, "[%08X] TListNode::Unlink()\n", this);

        if ( m_prev ) m_prev->m_next = m_next;
        if ( m_next ) m_next->m_prev = m_prev;
//      m_prev = m_next = NULL;
    }

    //
    // Migrate
    // Move a node to the end of a different list
    //
    virtual void Migrate(TLinkedList<T> *new_container)
    {
        m_container->Remove(this, false);
        new_container->Append(this);
    }

    //
    // RemoveSelf
    // Remove a node from the list it is in
    //
    virtual void RemoveSelf()
    {
        DEBUGF(2, "[%08X] TListNode::RemoveSelf()\n", this);

        m_container->Remove(this);
    }

    //
    // InsertAfter
    // Create and add a node after the this node
    //
    inline void InsertAfter( T p_data )
    {
        DEBUGF(2, "[%08X] TListNode::InsertAfter(p_data)\n", this);

        TListNode<T>    *newnode = new TListNode<T>;
        newnode->m_data = p_data;
        InsertAfter( newnode );
    }


    //
    // InsertAfter
    // Link an existing node after this node
    //
    void InsertAfter( TListNode<T> *newnode )
    {
        DEBUGF(2, "[%08X] TListNode::InsertAfter(node)\n", this);

        // set up newnode's pointers.
        newnode->m_next = m_next;
        newnode->m_prev = this;

        // if there is a node after this one, make it point to
        // newnode
        if ( m_next != NULL )
            m_next->m_prev = newnode;

        // make the current node point to newnode.
        m_next = newnode;
    }

    //
    // InsertBefore
    // Create and add a node before the this node
    //
    inline void InsertBefore( T p_data )
    {
        DEBUGF(2, "[%08X] TListNode::InsertBefore(p_data)\n", this);

        TListNode<T>    *newnode = new TListNode<T>;
        newnode->m_data = p_data;
        InsertBefore( newnode );
    }

    //
    // InsertBefore
    // Link an existing node before this node
    //
    void InsertBefore( TListNode<T> *newnode )
    {
        DEBUGF(2, "[%08X] TListNode::InsertBefore(node)\n", this);

        // set up newnode's pointers.
        newnode->m_next = this;
        newnode->m_prev = m_prev;

        // if there is a node before this one, make it point to
        // newnode
        if ( m_prev != NULL )
            m_prev->m_next = newnode;

        // make the current node point to newnode.
        m_prev = newnode;
    }
};


#pragma mark -
//
// TLinkedList
// Description: The Doubly-linked list container.
//
template<class T>
class TLinkedList
{
public:

    TListNode<T>*   m_head;
    TListNode<T>*   m_tail;
    Uint16          m_count;

    //
    // TLinkedList
    // Constructor; creates an empty list
    //
    TLinkedList()
    {
        m_head = NULL;
        m_tail = NULL;
        m_count = 0;
    }

    //
    // TLinkedList
    // Destructor; destroys every node
    //
    virtual ~TLinkedList() { Clear(); }

    //
    // Clear deletes all nodes and their local data
    //
    void Clear()
    {
        DEBUGF(2, "[%08X] TLinkedList::Clear()\n", this);

        // temporary node pointers.
        TListNode<T>* node = m_head;
        TListNode<T>* next;

        m_head = NULL;
        m_tail = NULL;
        m_count = 0;

        while( node )
        {
            // save the pointer to the next node.
            next = node->m_next;

            // delete the current node.
            delete node;

            // make the next node the current node.
            node = next;
        }
    }

    //
    // Append
    // Add a new node to the end of a list (and return the node reference)
    //
    TListNode<T>* Append( T p_data )
    {
        DEBUGF(2, "[%08X] TLinkedList::Append(p_data)\n", this);

        // if there is a head node (ie: list is not empty)
        if ( m_head )
        {
            // insert a new node after the tail, and reset the tail.
            m_tail->InsertAfter( p_data );
            m_tail = m_tail->m_next;
        }
        else
        {
            // create a new head node.
            m_head = m_tail = new TListNode<T>;
            m_head->m_data = p_data;
            m_head->m_next = NULL;
            m_head->m_prev = NULL;
        }

        m_count++;
        m_tail->m_container = this;

        return m_tail;
    }

    //
    // Append
    // Add an existing node to the end of a list
    //
    void Append( TListNode<T> *newnode )
    {
        DEBUGF(2, "[%08X] TLinkedList::Append(node)\n", this);

        // if there is a head node (ie: list is not empty)
        if ( m_head )
        {
            // insert a new node after the tail, and reset the tail.
            m_tail->InsertAfter( newnode );
            m_tail = newnode;
        }
        else
        {
            m_head = m_tail = newnode;
            newnode->m_next = newnode->m_prev = NULL;
        }

        m_count++;
        newnode->m_container = this;
    }

    //
    // Prepend
    // Add a new node to the beginning of a list
    //
    TListNode<T>* Prepend( T p_data )
    {
        DEBUGF(2, "[%08X] TLinkedList::Prepent(p_data)\n", this);

        // if there is no head node (ie: list is empty)
        if ( m_head )
        {
            // insert a new node before the head, and reset the head.
            m_head->InsertBefore( p_data );
            m_head = m_head->m_prev;
        }
        else
        {
            // create a new head node.
            m_head = m_tail = new TListNode<T>;
            m_head->m_data = p_data;
            m_head->m_next = NULL;
            m_head->m_prev = NULL;
        }
        m_count++;
        m_head->m_container = this;

        return m_head;
    }

    //
    // InsertAfter
    // Inserts data after the iterator, or at the end
    //  of the list if iterator is invalid.
    //
    TListNode<T>* InsertAfter( TIterator<T>& p_iterator, T p_data )
    {
        DEBUGF(2, "[%08X] TLinkedList::InertAfter(iter, p_data)\n", this);

        TListNode<T> *node, *newnode;

        if ((node = p_iterator.m_node))
        {
            // insert the data after the iterator
            node->InsertAfter( p_data );

            // if the iterator was the tail of the list,
            // update the tail pointer
            if ( node == m_tail )
                m_tail = m_tail->m_next;

            // increment the count
            m_count++;
            newnode = p_iterator.m_node->m_next;
        }
        else
        {
            Append( p_data );
            newnode = m_tail;
        }


        newnode->m_container = this;
        return newnode;
    }

    //
    // InsertBefore
    // Insert data before the iterator, or prepend
    //  to the beginning of the list if invalid.
    //
    TListNode<T>* InsertBefore( TIterator<T>& p_iterator, T p_data )
    {
        DEBUGF(2, "[%08X] TLinkedList::InsertBefore(iter, p_data)\n", this);

        TListNode<T> *node, *newnode;

        if ((node = p_iterator.m_node))
        {
            // insert the data before the iterator
            node->InsertBefore( p_data );

            // if the iterator was the head of the list,
            // reset the head pointer.
            if ( node == m_head )
                m_head = m_head->m_prev;

            // increment the count
            m_count++;
            newnode = node->m_prev;
        }
        else
        {
            Prepend( p_data );
            newnode = m_head;
        }

        newnode->m_container = this;

        return newnode;
    }

    //
    // RemoveHead
    // Remove the first node in the list
    //
    void RemoveHead()
    {
        DEBUGF(2, "[%08X] TLinkedList::RemoveHead()\n", this);

        TListNode<T>* node = NULL;

        if ( m_head )
        {
            // make node point to the next node.
            node = m_head->m_next;

            // then delete the head, and make the pointer
            // point to node.
            delete m_head;
            m_head = node;

            // if the head is null, then we've just deleted the only node
            // in the list. set the tail to 0.
            // if not, set the previous pointer to 0.
            if ( m_head )
                m_head->m_prev = NULL;
            else
                m_tail = NULL;

            m_count--;
        }
    }

    //
    // RemoveTail
    // Remove the last node in the list
    //
    void RemoveTail()
    {
        DEBUGF(2, "[%08X] TLinkedList::RemoveTail()\n", this);

        TListNode<T>* node = NULL;

        if ( m_tail )
        {
            // make node point to the next node.
            node = m_tail->m_prev;

            // then delete the head, and make the pointer
            // point to node.
            delete m_tail;
            m_tail = node;

            // if the tail is null, then we've just deleted the only node
            // in the list. set the head to 0.
            // if not, set the next pointer to 0.
            if ( m_tail)
                m_tail->m_next = NULL;
            else
                m_head = NULL;

            m_count--;
        }
    }

    //
    // Remove(node)
    //
    void Remove( TListNode<T> *node, bool del=true )
    {
        DEBUGF(2, "[%08X] TLinkedList::Remove(%08X)\n", this, node);

        // if the node we want to remove is the head or the tail
        // nodes, then move the head or tail to the next or
        // previous node.
        if ( node == m_head )
            m_head = m_head->m_next;

        if ( node == m_tail )
            m_tail = m_tail->m_prev;

        // unlink and delete the node.
        node->Unlink();

        if (del)
            delete node;

        m_count--;
    }

    //
    // Remove
    // Remove the node that the iterator points to and
    //  move the iterator to the next node
    //
    void Remove( TIterator<T>& p_iterator )
    {
        DEBUGF(2, "[%08X] TLinkedList::Remove(iter)\n", this);

        // temporary node pointer.
        TListNode<T>    *node;

        // if node is invalid, do nothing.
        if ( ! p_iterator.m_node )
            return;

        // save the pointer to the node we want to delete.
        node = p_iterator.m_node;

        // move the iterator forward to the next valid node
        p_iterator.Next();

        // remove and dispose the node
        Remove(node);
    }

    //
    // Remove(T)
    // Remove the node that contains a certain piece of data.
    // Depends on operator== method of the node's data type.
    // This is an expensive method for long lists.
    //
    void Remove(T p_data, bool del=true)
    {
        DEBUGF(2, "[%08X] TLinkedList::Remove(p_data)\n", this);

        // temporary node pointers.
        TListNode<T>* node = m_head;
        TListNode<T>* next;

        while( node )
        {
            next = node->m_next;

            // Remove a matching object and be done with thee
            if (node->m_data == p_data) {       // operator== applies for objects, but not pointers
                Remove(node, del);
                break;
            }

            node = next;
        }
    }

    //
    // Exchange two nodes - actually much simpler for objects.
    //  Beware using this on items that track their nodes,
    //
    void ExchangeData( TListNode<T> *node1, TListNode<T> *node2 )
    {
        if (node1 != node2)
        {
            T temp = (*node1).m_data;
            (*node1).m_data = (*node2).m_data;
            (*node1).m_data = temp;
        }
    }

    //
    // Exchange two nodes - not as simple as it seems
    //
    void Exchange( TListNode<T> *node1, TListNode<T> *node2 )
    {
        if (node1 == node2) return;

        TListNode<T> *prev1 = node1->m_prev;    // before 1
        TListNode<T> *next1 = node1->m_next;    // after 1
        TListNode<T> *prev2 = node2->m_prev;    // before 2
        TListNode<T> *next2 = node2->m_next;    // after 2

        if (next1 == node2)
        {
            node2->m_prev = prev1;
            node2->m_next = node1;
            node1->m_next = next2;
            node1->m_prev = node2;

            if (prev1)
                prev1->m_next = node2;
            else
                m_head = node2;

            if (next2)
                next2->m_prev = node1;
            else
                m_tail = node1;
        }
        else if (next2 == node1)
        {
            node1->m_prev = prev2;
            node1->m_next = node2;
            node2->m_next = next1;
            node2->m_prev = node1;
            prev2->m_next = node1;
            next1->m_prev = node2;

            if (prev2)
                prev2->m_next = node1;
            else
                m_head = node1;

            if (next1)
                next1->m_prev = node2;
            else
                m_tail = node2;
        }
        else
        {
            node1->m_prev = prev2;
            node1->m_next = next2;
            node2->m_prev = prev1;
            node2->m_next = next1;

            if (prev1)
                prev1->m_next = node2;
            else
                m_head = node2;

            if (next1)
                next1->m_prev = node2;
            else
                m_tail = node2;

            if (prev2)
                prev2->m_next = node1;
            else
                m_head = node1;

            if (next2)
                next2->m_prev = node1;
            else
                m_tail = node1;
        }
    }

    //
    // GetIterator
    // Get an iterator pointing to the beginning of the list
    //
    TIterator<T> GetIterator() const
    {
        DEBUGF(2, "[%08X] TLinkedList::GetIterator()\n", this);

        return TIterator<T>( (TLinkedList<T>*)this, m_head );
    }


    //
    // Size
    // Return the size of the list
    //
    Uint16 Size()
    {
        return m_count;
    }


    //
    // SaveToDisk
    // Save the linked list to disk
    //
    bool SaveToDisk( char* p_filename )
    {
        FILE*           outfile = NULL;
        TListNode<T>    *itr = m_head;

        // open the file
        outfile = fopen( p_filename, "wb" );

        // return if it couldn't be opened
        if ( ! outfile )
            return false;

        // write the size of the list first
        fwrite( &m_count, sizeof( Uint16 ), 1, outfile );

        // now loop through and write the list.
        while ( itr ) {
            fwrite( &(itr->m_data), sizeof( T ), 1, outfile );
            itr = itr->m_next;
        }

        fclose( outfile );

        // return success.
        return true;
    }


    //
    // ReadFromDisk
    // Read a linked list from a file
    //
    bool ReadFromDisk( char* p_filename )
    {
        FILE*   infile = NULL;
        T       buffer;
        Uint16  count = 0;

        // open the file
        infile = fopen( p_filename, "rb" );

        // return if it couldn't be opened
        if ( ! infile ) return false;

        // read the size of the list first
        fread( &count, sizeof( Uint16 ), 1, infile );

        // now loop through and read the list.
        while ( count ) {
            fread( &buffer, sizeof( T ), 1, infile );
            Append( buffer );
            count--;
        }

        fclose( infile );

        // return success.
        return true;
    }

};

#pragma mark -
template<class T>
class TObjectList : public TLinkedList<T>
{
public:
    //
    // Dispose(node)
    // Remove and delete the node and its data object
    //
    void Dispose( TListNode<T> *node )
    {
        DEBUGF(2, "[%08X] TObjectList::Dispose(%08X)\n", this, node);

        if (node->m_data)
            delete node->m_data;

        this->Remove(node);
    }

    //
    // Dispose
    // Dispose the node that the iterator points to and
    //  move the iterator to the next node
    //
    void Dispose( TIterator<T>& p_iterator )
    {
        DEBUGF(2, "[%08X] TObjectList::Dispose(iter)\n", this);

        // temporary node pointer.
        TListNode<T>    *node;

        // if node is invalid, do nothing.
        if ( ! p_iterator.m_node )
            return;

        // save the pointer to the node we want to delete.
        node = p_iterator.m_node;

        // move the iterator forward to the next valid node
        p_iterator.Next();

        // dispose the node
        Dispose(node);
    }

    //
    // Dispose(T)
    // Dispose the node that refers to a given instance.
    // This is an expensive method for long lists.
    //
    void Dispose(T p_data)
    {
        DEBUGF(2, "[%08X] TObjectList::Dispose(p_data)\n", this);

        // temporary node pointers.
        TListNode<T>* node = this->m_head;

        while( node )
        {
            // Remove a matching object and be done with thee
            if (node->m_data == p_data) {       // operator== applies for objects, but not pointers
                Dispose(node);
                break;
            }

            node = node->m_next;
        }
    }

    //
    // DisposeAll
    // delete all nodes and all referenced objects
    //
    void DisposeAll()
    {
        DEBUGF(2, "[%08X] TObjectList::Clear()\n", this);

        // temporary node pointers.
        TListNode<T> *node = this->m_head, *next;

        this->m_head = NULL;
        this->m_tail = NULL;
        this->m_count = 0;

        while( node )
        {
            // save the pointer to the next node.
            next = node->m_next;

            // delete the object pointed to by the node
            if (node->m_data)
                delete node->m_data;

            // delete the current node.
            delete node;

            // make the next node the current node.
            node = next;
        }
    }

};

#pragma mark -
//
// TIterator
// Description: The basic linked list iterator class.
//
template<class T>
class TIterator
{
public:
    TListNode<T>    *m_node;
    TLinkedList<T>  *m_list;


    //
    // TIterator
    // Constructor; creates an iterator that points to the given list and node
    //
    TIterator( TLinkedList<T>* p_list=NULL, TListNode<T>* p_node=NULL )
    {
        m_list = p_list;
        m_node = p_node;
    }


    //
    // Start
    // Reset the iterator to the beginning of the list
    //
    inline void Start()
    {
        DEBUGF(2, "[%08X] TIterator::Start()\n", this);

        if ( m_list ) m_node = m_list->m_head;
    }

    //
    // End
    // Reset the iterator to the end of the list
    //
    inline void End()
    {
        if ( m_list ) m_node = m_list->m_tail;
    }


    //
    // Next
    // Move the iterator forward by one position
    //
    inline void Next()
    {
        DEBUGF(2, "[%08X] TIterator::Next()\n", this);

        if ( m_node ) m_node = m_node->m_next;
    }


    //
    // NextItem
    // Move the iterator forward by one position
    //
    inline T NextItem()
    {
        DEBUGF(2, "[%08X] TIterator::NextItem()\n", this);

        if ( m_node ) {
            TListNode<T> *o_node = m_node;
            m_node = m_node->m_next;
            return o_node->m_data;
        }

        return NULL;
    }


    //
    // Previous
    // Move the iterator backward by one position
    //
    inline void Previous()
    {
        DEBUGF(2, "[%08X] TIterator::Previous()\n", this);

        if ( m_node ) m_node = m_node->m_prev;
    }


    //
    // PreviousItem
    // Move the iterator backward by one position
    //
    inline T PreviousItem()
    {
        DEBUGF(2, "[%08X] TIterator::PreviousItem()\n", this);

        if ( m_node ) {
            TListNode<T> *o_node = m_node;
            m_node = m_node->m_prev;
            return o_node->m_data;
        }

        return NULL;
    }


    //
    // Item
    // Return the item the iterator is pointing to
    //
    inline T& Item()
    {
        DEBUGF(2, "[%08X] TIterator::Item()\n", this);

        return m_node->m_data;
    }


    //
    // IsValid
    // Return whether the node is valid
    //
    inline bool IsValid()
    {
        DEBUGF(2, "[%08X] TIterator::IsValid()\n", this);

        return (m_node != NULL);
    }


    //
    // operator==
    // Determine if two iterators point to the same node
    //
    bool operator==( TIterator<T>& p_rhs )
    {
        return ( m_node == p_rhs.m_node && m_list == p_rhs.m_list );
    }

};

#endif

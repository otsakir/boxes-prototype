#ifndef LISTPOOL_H
#define LISTPOOL_H

// T is the content class, I is the index type used to point inside the items array
template <class T, class I=int>
class ListPool
{
public:
    typedef I Index; // index type for this ListPool. Get the type from the template parameter
    class ListItem {
    private:
        Index previous;
        Index next;
        bool used;
    public:
        T content;

        friend class ListPool;
    };

private:
    ListItem* items;
    int capacity;
    Index iUsed; // items used
    Index iAvailable; // items available
    int usedCount;
    int availableCount;
public:

    ListPool(int capacity) : capacity(capacity), items(new ListItem[capacity])  {
        Index previous = -1;
        for (int i=0; i<capacity; i++) {
            items[i].previous = -1;
            items[i].next = i+1;
            items[i].used = false;

            previous = i;
        }
        items[capacity-1].next = -1; // mark the end

        iUsed = -1;
        usedCount = 0;
        iAvailable = 0; // point to the first item in the array by definition
    }

    // returns -1 if no more items available
    Index use() {
        if (iAvailable == -1) {
            return -1;
        }
        Index i = iAvailable;
        iAvailable = items[i].next;
        if (iUsed == -1) {
            items[i].next = iUsed; // i.e. -1
            iUsed = i;
            items[i].previous = -1; // it's the first one. Nothing comes before it.
        } else {
            items[i].next = iUsed;
            items[iUsed].previous = i;
            iUsed = i;
            items[i].previous = -1;
        }
        usedCount ++;
        items[i].used = true;
        return i;
    }

    // returns false if item not used. 'true' if successfully released
    bool release(Index i) {
        // TODO - check if this is on the Used list. In debug mode go through the list until iUsed is found or capacity exceeded. ?
        if (!items[i].used)
            return false;
        // remove from Used
        Index n = items[i].next;
        Index p = items[i].previous;
        if (n != -1)
            items[n].previous = p;
        if (p != -1)
            items[p].next = n;
        if (iUsed == i) {
            iUsed = items[i].next;
        }
        // put to Available
        if (iAvailable != -1) {
            items[iAvailable].previous = i;
            items[i].next = iAvailable;
            items[i].previous = -1;
            iAvailable = i;
        } else {
            iAvailable = i;
            items[i].previous = -1;
            items[i].next = -1;
        }
        usedCount --;
        items[i].used = false;
        return true;
    }


    inline Index get(T& newItem) {
        Index index = use();
        if (index != -1)
                newItem = items[index].content;
        return index;
    }

    // return index of an empty item and fill a pointer reference to the item content
    // return -1 if no empty item and does not touch the pointer reference
    inline Index getp(T*& newItemp) {
        Index index = use();
        if (index != -1)
                newItemp = &(items[index].content);
        return index;

    }

    // true if no more available items to return
    bool dry() {
        return (iAvailable == -1);
    }

    Index iter() {
        return iUsed;
    }

    // returns item pointed by it and advances it to next one
    // returns -1 when no more items
    Index nextp(Index it, T*& nextItem) {
        if (it != -1) {
            nextItem = &(items[it].content);
            return items[it].next;
        } else {
            return -1;
        }
    }

    inline int getUsedCount() {
        return usedCount;
    }


    ~ListPool() {
        if (items)
            delete [] items;
    }
};



#endif // LISTPOOL_H

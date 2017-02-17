/*
 HeapSkew.h: A skew heap implementation
 Copyright 1999 G. Andrew Stone
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Neither the name of the copyright holder nor the
      names of its contributors may be used to endorse or promote products
      derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/


#ifndef ARDUINOPC_HEAPSKEW_H
#define ARDUINOPC_HEAPSKEW_H

// T must have a > comparison operator.
// T must have a HeapSkewElement class with the variable name skewChildren; i.e. "HeapSkew<T>::HeapSkewElement  skewChildren;"
template<typename T> 
class HeapSkew
{
public:

    void push(T& t);            // Add A new element onto the heap.
    T& pop();            // Remove the top element of the heap.
    T& front(void);            // Return the top element of the heap.
    void merge (HeapSkew& h);     // All elements of 'h will be merged into 'this.  So 'h will end empty.

    T& clear();            // Remove all elements, return the root of the tree of removed nodes.

    HeapSkew() :
        m_root(0) 
    {

    }

    ~HeapSkew() 
    {
        
    }

    class HeapSkewElement
    {
    public:
        HeapSkewElement() : 
            left{0}, 
            right{0} 
        {

        }
        
        T* left;
        T* right;
    };

    // Statistics
    unsigned int lastOperationDepth;
    unsigned int maxOperationDepth;
      

private:

    T* merge (T* a, T* b);  

    void pointerSwap(T **a,T **b)
    {
        T *c;
        c = (*a);
        (*a) = (*b);
        (*b) = c;
    }
      

    T* m_root;
};


template<typename T> 
void HeapSkew<T>::push(T& t)
{
    //assert(&t);
    t.skewChildren.left = 0; 
    t.skewChildren.right = 0;
    this->m_root = merge(&t, this->m_root);
}

template<typename T> 
T& HeapSkew<T>::pop()
{
    T& ret = *this->m_root;
    if (this->m_root) {
        this->m_root = merge(this->m_root->skewChildren.left, this->m_root->skewChildren.right);
    }
    ret.skewChildren.left = 0;
    ret.skewChildren.right = 0;
    return ret;
}

template<typename T> 
T& HeapSkew<T>::front()
{
    return *this->m_root;
}

template<typename T> 
T& HeapSkew<T>::clear()
{
    T* returnNode = this->m_root;
    this->m_root  = nullptr;
    return *returnNode;  
}


template<typename T> 
void HeapSkew<T>::merge (HeapSkew& h)
{
    merge(h.m_root);
    h.m_root = 0;
}


template<typename T> T* HeapSkew<T>::merge (T* a, T* b)
{
    byte sizeCounter{0};
    T *returnNode{nullptr};
    lastOperationDepth = 0;
    if (a == 0) {
        return b;
    } else if (b == 0) {
        return a;
    }
  
    if (*a > *b)  {
        pointerSwap(&a,&b);  /* Make a the smallest. */
    }

    returnNode = a;

    while(b != 0) {
        if (a->skewChildren.right == 0) { 
            a->skewChildren.right = b; 
            break; 
        }
        if (*a->skewChildren.right > *b) {
            pointerSwap(&a->skewChildren.right, &b);
        }
        pointerSwap(&a->skewChildren.right, &a->skewChildren.left);
        a = a->skewChildren.left; /* Really the right, but I just swapped them. */
        sizeCounter++;
    }
  
    lastOperationDepth = sizeCounter;
    if (sizeCounter > maxOperationDepth) maxOperationDepth = sizeCounter;
    //  if (Count) AveHeapDepth = (99.0*AveHeapDepth+((float)SizeCtr))/100.0;
    return(returnNode);
  }



template<typename T> 
class HeapSkewMemoryManagement
{
  
    class Wrapper
    {
    public:
        typename HeapSkew<Wrapper>::HeapSkewElement skewChildren; 
        T userData;
        Wrapper(const T& data): userData(data) {}
        int operator > (const Wrapper& w) { 
            return (userData > w.userData);
        }
    };

    HeapSkew<Wrapper> sk;

public:
    void push(const T& t) 
    {
        sk.push(*(new Wrapper(t)));
    }

    T pop(void) 
    {
        Wrapper* w = &sk.pop();
        T ret = w->userData;
        delete w;
        return ret;
    }

    T& front(void)       
    {
        Wrapper* w = &sk.front();
        return w->userData;
    }

    void merge(HeapSkewMemoryManagement& h)
    {
        sk.merge(h.sk);
    }

};

#endif //ARDUINOPC_HEAPSKEW_H

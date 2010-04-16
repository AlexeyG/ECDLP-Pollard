#ifndef _AVLHELPERS_
#define _AVLHELPERS_

template <class T> class Unit
{
public:	
	Unit(const T& _Data, Unit* _Next);
	T Data;
	Unit* Next;
};

template <class T> Unit<T>::Unit(const T &_Data, Unit* _Next)
	: Data(_Data), Next(_Next)
{
}

template <class T> class Stack
{
public:
	Stack();
	void Push(const T& _Data);
	T Pop();
	bool Empty();
	void Clear();

private:	
	Unit<T>* Head;
};

template <class T> Stack<T>::Stack()
{
	Head = NULL;
}

template <class T> void Stack<T>::Push(const T &_Data)
{
	Unit<T>* U = new Unit<T>(_Data, Head);
	Head = U;
}

template <class T> T Stack<T>::Pop()
{
	Unit<T>* Temp = Head;
	T _Data = Temp->Data;
	Head = Temp->Next;
	delete Temp;
	return _Data;
}

template <class T> bool Stack<T>::Empty()
{
	return (Head == NULL)
}

template <class T> void Stack<T>::Clear()
{
	Unit<T>* P = Head, *Q;
	while (P != NULL)
	{
		Q = P->Next;
		delete P;
		P = Q;
	}
}

template <class T> class Node
{
public:	
	Node(const T& _key);
	Node* Child(char a);
	Node* left;
	Node* right;
	T dat;
	char d;
};

template <class T> Node<T>::Node(const T& _dat)
	: left(0), right(0), dat(_dat), d(0)
{
}

template <class T> Node<T>* Node<T>::Child(char a)
{
	if (a == 1)
		return right;
	else
		return left;
}

#endif

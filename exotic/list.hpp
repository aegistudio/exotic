#pragma once
/**
 * @file exotic/list.hpp
 * @author Haoran Luo
 * @brief Doubly Linked List
 *
 * Implementation of doubly linked list data structure in the exotic framework. 
 * A list node holds two pointers: one to its previous node and another to its 
 * next node. The list head's previous node and the tail's next node points to 
 * null pointer. The container holds the pointer to the head and tail node.
 */

#include "exotic/container.hpp"
#include "exotic/pointer.hpp"
#include "exotic/scope.hpp"

namespace exotic {

/// @brief Node in the linked list. Please embed it into your data structure.
template<template<typename> typename pointerType, template<typename> typename scopeType>
class listNode : private scopeType<listNode<pointerType, scopeType>>::nodeScopeType {
	typedef scopeType<listNode<pointerType, scopeType>> scopingType;
	typedef typename scopingType::nodeScopeType nodeScopeType;
	typedef typename scopingType::containerScopeType containerScopeType;
	
	/// The next node in the linked list. Will be null if it is currently 
	/// the last node in the linked list.
	pointerType<listNode> next;

	/// The previous node in the linked list. Will be null if it is currently 
	/// the first node in the list link.
	pointerType<listNode> previous;
	
	/// Insert a node after the node in the linked list.
	template<typename contextType>
	void insertAfter(contextType& ctx, listNode& node) noexcept {
		node.previous(ctx) = this;
		node.next = next;
		if(next != nullptr)
			next(ctx) -> previous(ctx) = &node;
		next(ctx) = &node;
	}
	
	/// Insert a node before the node in the linked list.
	template<typename contextType>
	void insertBefore(contextType& ctx, listNode& node) noexcept {
		node.previous = previous;
		node.next(ctx) = this;
		if(previous != nullptr)
			previous(ctx) -> next(ctx) = &node;
		previous(ctx) = &node;
	}
	
	/// Remove current node from the linked list.
	template<typename contextType>
	void removeFromList(contextType& ctx) noexcept {
		if(previous != nullptr) previous(ctx) -> next = next;
		if(next != nullptr) next(ctx) -> previous = previous;
		previous = next = nullptr;
	}
	
	/// The friend declaration.
	template<typename, typename, typename> friend class container;
	
	/// The ownership checking. Return -1 when the node is not contained by
	/// any container, return 0 when the node is contained by the specified
	/// container, and return 1 if the node is contained by other container.
	int containerCompare(containerScopeType* containerScope) const noexcept {
		if(previous == nullptr && next == nullptr) return -1;
		else return nodeScopeType::containedBy(containerScope)? 0 : 1;
	}
public:
	/// Construct a linked list node. By setting all node values
	///	to nullptr.
	listNode() noexcept {}
	
	/// The destructor for the node, by invoking scoped method.
	~listNode() noexcept {	nodeScopeType::remove(*this);	}
};

/**
 * @brief The container of the linked list. It should hold the first and the 
 * last node in the linked list, in order to provide O(1) iterator creation 
 * function().
 */
template<typename accessType, typename contextType, 
	template<typename> typename pointerType, template<typename> typename scopeType>
struct container<accessType, contextType, listNode<pointerType, scopeType>> : private contextType, 
	public scopeType<listNode<pointerType, scopeType>>::containerScopeType {
	/// The type of the linked list node.
	typedef listNode<pointerType, scopeType> nodeType;
	
	/// The type of the container scope.
	typedef typename scopeType<nodeType>::containerScopeType containerScopeType;
	
	/// The object type definition's forwarding.
	typedef typename accessType::objectType objectType;
	
	/// The forward iterator type definition's forwarding.
	typedef forwardIterator<accessType, contextType> forwardIteratorType;
	
	/// The backward iterator type definition's forwarding.
	typedef backwardIterator<accessType, contextType> backwardIteratorType;
	
	/// The type definition used for list iteration.
	typedef objectType value_type;
private:
	/// The first and last node in the linked list iterator. Therefore both 
	/// forward and backward access will be of time complexity O(1).
	pointerType<nodeType> first, last;
	
	/// Perform removal of a node in the context.
	void remove(nodeType& node) noexcept {
		if(first(*this) == &node) first = node.next;
		if(last(*this) == &node) last = node.previous;
		node.removeFromList(*this);
	}
public:
	/// The initialization of the container.
	container(contextType&& ctx = {}) noexcept: contextType(ctx) {}
	
	/// Check whether the container is empty.
	inline bool empty() const noexcept { return first == nullptr; }
	
	/// The destructor will iteratively reset all internal nodes.
	~container() noexcept {
		if(scopeType<nodeType>::containerRemove && !empty()) {
			nodeType* node = first(*this);
			while(node != nullptr) {
				node -> previous(*this) = nullptr;
				nodeType* nextNode = node -> next(*this);
				node -> next(*this) = nullptr;
				node = nextNode;
			}
		}
	}
	
	/// Retrieve the forward iterator's starting point via begin().
	forwardIteratorType begin() noexcept {
		return forwardIteratorType(*this, first(*this));
	}
	
	/// Retrieve the forward iterator's ending point via end().
	forwardIteratorType end() noexcept {
		return forwardIteratorType(*this, nullptr);
	}
	
	/// Retrieve the backward iterator's starting point via rbegin().
	backwardIteratorType rbegin() noexcept {
		return backwardIteratorType(*this, last(*this));
	}
	
	/// Retrieve the backward iterator's starting point via rbegin().
	backwardIteratorType rend() noexcept {
		return backwardIteratorType(*this, nullptr);
	}
	
	/// Retrieve the next node of the linked list node.
	nodeType* forward(nodeType* node) const noexcept {
		if(node == nullptr) return nullptr;
		else return node -> next(*this);
	}
	
	/// Retrieve the previous node of the linked list node.
	nodeType* backward(nodeType* node) const noexcept {
		if(node == nullptr) return nullptr;
		else return node -> previous(*this);
	}
	
	/// Iterate forward from a specified node. For modes that does not 
	/// have the container's information, it is possible for that node
	/// to create a wrong iterator (but such wrong method is still 
	/// required, you have to write carefully this way).
	forwardIteratorType from(objectType& object) noexcept {
		nodeType* node = &accessType::nodeOf(object);
		if(node -> containerCompare(this) != 0) return end();
		else return forwardIteratorType(*this, node);
	}
	
	/// Iterate backward from a specified node. The correctness of this 
	/// method is the same as the forward one.
	backwardIteratorType rfrom(objectType& object) noexcept {
		nodeType* node = &accessType::nodeOf(object);
		if(node -> containerCompare(this) != 0) return rend();
		else return backwardIteratorType(*this, node);
	}
private:
	// Check whether a specified forward and backward iterator is the iterator 
	// of this container.
	inline bool hasIterator(const forwardIteratorType& iter) const noexcept 
		{	return (&(iter.containerObject) == this);	}
	
	inline bool hasIterator(const backwardIteratorType& iter) const noexcept 
		{	return (&(iter.containerObject) == this);	}
	
	// Check whether the node can be inserted into this container.
	inline bool canInsert(const forwardIteratorType& iter, objectType& object) noexcept {
		if(!hasIterator(iter)) return false;
		nodeType& node = accessType::nodeOf(object);
		if(node.containerCompare(this) != -1) return false;
		return true;
	}
	
	inline bool canInsert(const backwardIteratorType& iter, objectType& object) noexcept {
		if(!hasIterator(iter)) return false;
		nodeType& node = accessType::nodeOf(object);
		if(node.containerCompare(this) != -1) return false;
		return true;
	}
	
	/// Do common work of insert-after, and return true if iterator specific 
	/// work is to be done.
	inline bool commonInsertAfter(nodeType& node, nodeType* current) noexcept {
		// Insertion happens at the end of the linked list.
		if(current == nullptr) {
			if(empty()) first(*this) = last(*this) = &node;
			else return true;	// Iterator specific code.
		}
		
		// Insertion happends at the last node of the list.
		else if(current == last(*this)) {
			last(*this) -> insertAfter(*this, node);
			last(*this) = &node;
		}
		
		// The most common insertion at the middle.
		else current -> insertAfter(*this, node);
		return false;
	}
public:
	/// Insert the node after the iterator's node.
	bool insertAfter(forwardIteratorType iterator, objectType& object) noexcept {
		if(!canInsert(iterator, object)) return false;
		nodeType& node = accessType::nodeOf(object);
		
		// Perform common insertion, and do our own work if iterator specific 
		// insertion is required.
		if(commonInsertAfter(node, iterator.current)) {
			last(*this) -> insertAfter(*this, node);
			last(*this) = &node;
		}
		node.update(this);
		return true;
	}
	
	/// Insert the node after the iterator's node.
	bool insertAfter(backwardIteratorType iterator, objectType& object) noexcept {
		if(!canInsert(iterator, object)) return false;
		nodeType& node = accessType::nodeOf(object);
		
		// Perform common insertion, and do our own work if iterator specific 
		// insertion is required.
		if(commonInsertAfter(node, iterator.current)) {
			first(*this) -> insertBefore(*this, node);
			first(*this) = &node;
		}
		node.update(this);
		return true;
	}
private:
	/// Do common work of insert-before, and return true if iterator specific work is 
	/// to be done.
	inline bool commonInsertBefore(nodeType& node, nodeType* current) noexcept {
		// Insertion happens at the end of the linked list.
		if(current == nullptr) {
			if(empty()) first(*this) = last(*this) = &node;
			else return true;
		}
		
		// Insertion happends at the beginning of the linked list.
		else if(current == first(*this)) {
			first(*this) -> insertBefore(*this, node);
			first(*this) = &node;
		}
		
		// The most common insertion at the middle.
		else current -> insertBefore(*this, node);
		return false;
	}
public:
	/// Insert the node before the forward iterator's node.
	bool insertBefore(forwardIteratorType iterator, objectType& object) noexcept {
		if(!canInsert(iterator, object)) return false;
		nodeType& node = accessType::nodeOf(object);
		
		// Perform common insertion, and do our own work if iterator specific 
		// insertion is required.
		if(commonInsertBefore(node, iterator.current)) {
			last(*this) -> insertAfter(*this, node);
			last(*this) = &node;
		}
		node.update(this);
		return true;
	}
	
	/// Insert the node before the backward iterator's node.
	bool insertBefore(backwardIteratorType iterator, objectType& object) noexcept {
		if(!canInsert(iterator, object)) return false;
		nodeType& node = accessType::nodeOf(object);
		
		// Perform common insertion, and do our own work if iterator specific 
		// insertion is required.
		if(commonInsertBefore(node, iterator.current)) {
			first(*this) -> insertBefore(*this, node);
			first(*this) = &node;
		}
		node.update(this);
		return true;
	}
	
	/// Erase a node and return the next node followed.
	template<typename iteratorType>
	iteratorType erase(iteratorType iterator) noexcept {
		if(iterator.current == nullptr) return iteratorType(*this, nullptr);
		else {
			// Directly return the end iterator if it is not inside the container.
			if(&(iterator.containerObject) != this) 
				return iteratorType(*this, nullptr);
			
			// Fetch the potential result first.
			iteratorType result = iterator;
			++ result;
			
			// Perform deletion then.
			nodeType* current = iterator.current;
			nodeType* next = current -> next;
			if(current == first) first = next;
			if(current == last) last = current -> previous;
			current -> removeFromList();
			current -> update(nullptr);
			return result;
		}
	}
	
	/// Perform push front operation, which will surely insert the node at the front.
	bool pushFront(objectType& object) noexcept {
		nodeType& node = accessType::nodeOf(object);
		if(node.containerCompare(this) != -1) return false;
		
		// If the list is currently empty, directly set the node.
		if(empty()) first(*this) = last(*this) = &node;
		
		// Otherwise insert it before the first node.
		else {
			first(*this) -> insertBefore(*this, node);
			first(*this) = &node;
		}
		node.update(this);
		return true;
	}
	
	/// Perform push back operation. which will surely insert the node at the back.
	void pushBack(objectType& object) noexcept {
		nodeType& node = accessType::nodeOf(object);
		if(node.containerCompare(this) != -1) return false;
		
		// If the list is currently empty, directly set the node.
		if(empty()) first(*this) = last(*this) = &node;
		
		// Otherwise insert it before the first node.
		else {
			last(*this) -> insertAfter(*this, node);
			last(*this) = &node;
		}
		node.update(this);
		return true;
	}
	
	/// Perform pop front operation, which will surely remove the node at the front.
	/// As the node is built into the object, we can safely return the pointer.
	objectType* popFront() noexcept {
		// If the list is currently empty, directly set the node.
		if(empty()) return nullptr;
		
		// Otherwise insert it before the first node.
		else {
			nodeType* removing = first(*this);
			first = removing -> next;
			if(first == nullptr) last = nullptr;
			removing -> removeFromList(*this);
			removing -> update(nullptr);
			return &accessType::objectOf(*removing);
		}
	}
	
	/// Perform pop back operation, which will surely remove the node at the back.
	objectType* popBack() noexcept {
		// If the list is currently empty, directly set the node.
		if(empty()) return nullptr;
		
		// Otherwise insert it before the first node.
		else {
			nodeType* removing = last(*this);
			last = removing -> previous;
			if(last == nullptr) first = nullptr;
			removing -> removeFromList(*this);
			removing -> update(nullptr);
			return &accessType::objectOf(*removing);
		}
	}
};
	
} // namespace exotic.
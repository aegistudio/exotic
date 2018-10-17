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

namespace exotic {

/// @brief Node in the linked list. Please embed it into your data structure.
template<template<typename> typename pointerType>
class listNode {
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
	template<typename, typename, typename> 
	friend class container;
public:
	/// Construct a linked list node. By setting all node values
	/// to nullptr.
	listNode() noexcept {}
};

/**
 * @brief The container of the linked list. It should hold the first and the 
 * last node in the linked list, in order to provide O(1) iterator creation 
 * function().
 */
template<typename accessType, typename contextType,
	template<typename> typename pointerType>
struct container<accessType, contextType, listNode<pointerType>> : private contextType {
	/// The type of the linked list node.
	typedef listNode<pointerType> nodeType;
	
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
public:
	/// The initialization of the container.
	container(contextType&& ctx = {}) noexcept: contextType(ctx) {}
	
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
	
	/// Check whether the container is empty.
	inline bool empty() const noexcept { return first == nullptr; }
private:
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
	void insertAfter(forwardIteratorType iterator, objectType& object) noexcept {
		nodeType& node = accessType::nodeOf(object);
		
		// Perform common insertion, and do our own work if iterator specific 
		// insertion is required.
		if(commonInsertAfter(node, iterator.current)) {
			last(*this) -> insertAfter(*this, node);
			last(*this) = &node;
		}
	}
	
	/// Insert the node after the iterator's node.
	void insertAfter(backwardIteratorType iterator, objectType& object) noexcept {
		nodeType& node = accessType::nodeOf(object);
		
		// Perform common insertion, and do our own work if iterator specific 
		// insertion is required.
		if(commonInsertAfter(node, iterator.current)) {
			first(*this) -> insertBefore(*this, node);
			first(*this) = &node;
		}
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
	void insertBefore(forwardIteratorType iterator, objectType& object) noexcept {
		nodeType& node = accessType::nodeOf(object);
		
		// Perform common insertion, and do our own work if iterator specific 
		// insertion is required.
		if(commonInsertBefore(node, iterator.current)) {
			last(*this) -> insertAfter(*this, node);
			last(*this) = &node;
		}
	}
	
	/// Insert the node before the backward iterator's node.
	void insertBefore(backwardIteratorType iterator, objectType& object) noexcept {
		nodeType& node = accessType::nodeOf(object);
		
		// Perform common insertion, and do our own work if iterator specific 
		// insertion is required.
		if(commonInsertBefore(node, iterator.current)) {
			first(*this) -> insertBefore(*this, node);
			first(*this) = &node;
		}
	}
	
	/// Erase a node and return the next node followed.
	template<typename iteratorType>
	iteratorType erase(iteratorType iterator) noexcept {
		if(iterator.current == nullptr) return iteratorType(*this, nullptr);
		else {
			// Fetch the potential result first.
			iteratorType result = iterator;
			++ result;
			
			// Perform deletion then.
			nodeType* current = iterator.current;
			nodeType* next = current -> next;
			if(current == first) first = next;
			if(current == last) last = current -> previous;
			current -> removeFromList();
			return result;
		}
	}
	
	/// Perform push front operation, which will surely insert the node at the front.
	void pushFront(objectType& object) noexcept {
		nodeType& node = accessType::nodeOf(object);
		
		// If the list is currently empty, directly set the node.
		if(empty()) first(*this) = last(*this) = &node;
		
		// Otherwise insert it before the first node.
		else {
			first(*this) -> insertBefore(*this, node);
			first(*this) = &node;
		}
	}
	
	/// Perform push back operation. which will surely insert the node at the back.
	void pushBack(objectType& object) noexcept {
		nodeType& node = accessType::nodeOf(object);
		
		// If the list is currently empty, directly set the node.
		if(empty()) first(*this) = last(*this) = &node;
		
		// Otherwise insert it before the first node.
		else {
			last(*this) -> insertAfter(*this, node);
			last(*this) = &node;
		}
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
			return &accessType::objectOf(*removing);
		}
	}
};
	
} // namespace exotic.
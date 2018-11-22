#pragma once
/**
 * @brief exotic/list.hpp
 * @author Haoran Luo
 * @brief The doubly-linked list definition.
 *
 * This file defines the (doubly) linked list structure. To enable the linked list that 
 * it could be destroyed or moved anytime they want without any extra information, the 
 * linked list container holds a sentinel nodes so that any internal nodes could remove 
 * themselves using normal unlink operations.
 */
#include "exotic/scope.hpp"
#include "exotic/node.hpp"
#include "exotic/iterator.hpp"

namespace exotic {

/**
 * @brief Defines a linked list node that should be embedded in some object.
 *
 * The fields and operations are set private to protect the structure, however users
 * could default-construct, destruct, move-construct and move-assign a node.
 */
template<typename cscopeType = exotic::scope::decoupled> class listNode {
	/// Forwarding the scope type definition (so that containers will get their field type).
	typedef cscopeType scopeType;
	
	// The previous and next linked list node.
	listNode *previous, *next;
	
	/// Unlink the linked list from the current container.
	void unlink() noexcept {
		if(previous != nullptr) previous -> next = next;
		if(next != nullptr) next -> previous = previous;
		previous = next = nullptr;
	};
	
	/// Insert the node after some node in the container.
	void insertAfter(listNode& node) noexcept {
		previous = &node; next = node.next;
		previous -> next = next -> previous = this;
	}
	
	/// Insert the node before some node in the container.
	void insertBefore(listNode& node) noexcept {
		next = &node; previous = node.previous;
		previous -> next = next -> previous = this;
	}
	
	/// Test whether the node is an orphan node.
	bool isOrphanNode() const noexcept { return previous == nullptr && next == nullptr; }
	
	/// Only the linked list container could have access to the internal field and methods.
	template<typename nodeType> friend class list;
public:
	/// The default constructor for the linked list.
	listNode() noexcept: previous(nullptr), next(nullptr) {}
	
	/// The destructor for the linked list.
	~listNode() noexcept {
		if(scopeType::destroyNode) unlink();
	}
	
	/// The swap method for the linked list, inter-changes the relation of the nodes.
	void swap(listNode& b) noexcept {
		// Retrieve the previous and next node first.
		listNode *ap = previous, *an = next;
		listNode *bp = b.previous, *bn = b.next;
		
		// Swap external the relations.
		if(ap != nullptr) ap -> next = &b;
		if(an != nullptr) an -> previous = &b;
		if(bp != nullptr) bp -> next = this;
		if(bn != nullptr) bn -> previous = this;
		
		// Swap the internal relations.
		previous = bp; next = bn;
		b.previous = ap; b.next = an;
	}
	
	/// Make the move assignment operator become the swapping operation.
	listNode& operator=(listNode&& b) noexcept {
		b.swap(*this);
		return *this;
	}
	
	/// Make the move constructor become the swap operation.
	listNode(listNode&& b) noexcept: previous(nullptr), next(nullptr) {
		b.swap(*this);
	}
	
	// The node can never be copy-assigned or copy-constructed.
	listNode(const listNode&) = delete;
	listNode& operator=(const listNode&) = delete;
};

/**
 * @brief Defines the container for the doubly linked list.
 *
 * The container forms a circular linked list in order to simplify the insertions and 
 * deletions. 
 * 
 * Generally, in order to speed up the creation of iterator, the container will never 
 * verify whether the list node is inside the container. So the program behavior is 
 * undefined if iterate from a node which is not inside the container. Maybe looping 
 * infinitely, or cause segmentation fault when attempting to write.
 */
template<typename idType> struct list {
	// Forward the definition of the internal node type.
	typedef typename idType::nodeType nodeType;
	
	// Forward the definition of the internal object.
	typedef typename idType::objectType objectType;
	
	// No userdata is required for iterating over linked list.
	typedef void userdataType;
private:
	// Retrieve the scope type of the node.
	typedef typename nodeType::scopeType scopeType;
	
	// Ensure that we are operating on a linked list node.
	static_assert(std::is_same<listNode<scopeType>, nodeType>::value,
		"Only a linked list node field could be specified to link list.");
		
	// The sentinel node storing the node relations.
	listNode<scopeType> sentinel;
public:
	// Defaultly construct a container for linked list.
	list() noexcept: sentinel() {
		// Make a circular linked reference.
		sentinel.previous = sentinel.next = &sentinel;
	}
	
	// Defaultly destruct the linked list.
	~list() noexcept {
		if(!scopeType::destroyContainer) return;
		
		// Simply set all internal node to orphan nodes.
		listNode<scopeType>* node = sentinel.next;
		while(node != &sentinel) {
			listNode<scopeType>* next = node -> next;
			node -> previous = node -> next = nullptr;
			node = next;
		}
		
		// Make the sentinel node orphan nodes.
		sentinel.next = sentinel.previous = nullptr;
	}
	
	/// Forwarding the mutable forward iterator's definition.
	typedef class forwardIterator<list, void, true> forwardIteratorType;
	
	/// Forwarding the mutable backward iterator's definition.
	typedef class backwardIterator<list, void, true> backwardIteratorType;
	
	/// Forwarding the const forward iterator's definition.
	typedef class forwardIterator<list, void, false> constForwardIteratorType;
	
	/// Forwarding the const backward iterator's definition.
	typedef class backwardIterator<list, void, false> constBackwardIteratorType;
private:
	/// Helper function for retrieving the begin node.
	listNode<scopeType>* beginForward() const noexcept {
		return (sentinel.next == &sentinel)? nullptr : sentinel.next;
	}

	/// Helper function for retrieving the backward begin node.
	listNode<scopeType>* beginBackward() const noexcept {
		return (sentinel.previous == &sentinel)? nullptr : sentinel.previous;
	}
	
	/// Helper function for returning a normal node or nullptr if orphan node expected.
	nodeType* nodePointerFrom(objectType& object) {
		nodeType* node = idType::node(&object);
		return node -> isOrphanNode()? nullptr : node;
	}
public:
	/// Create the forward iterator from the begining.
	forwardIteratorType begin() noexcept {
		return forwardIteratorType(*this, beginForward());
	}
	
	/// Creates the end iterator for forward iterating.
	forwardIteratorType end() noexcept {
		return forwardIteratorType(*this, nullptr);
	}

	/// Create the forward iterator right from a node. If the node is orphan, the 
	/// end iterator will be returned.
	forwardIteratorType from(objectType& object) noexcept {
		return forwardIteratorType(*this, nodePointerFrom(object));
	}
	
	/// Create the const forward iterator from the begining.
	constForwardIteratorType cbegin() noexcept {
		return constForwardIteratorType(*this, beginForward());
	}
	
	/// Creates the end iterator for forward iterating.
	constForwardIteratorType cend() noexcept {
		return constForwardIteratorType(*this, nullptr);
	}
	
	/// Create const the forward iterator right from a node.
	constForwardIteratorType cfrom(objectType& object) noexcept {
		return constForwardIteratorType(*this, nodePointerFrom(object));
	}
	
	/// Create the backward iterator from reverse begining.
	backwardIteratorType rbegin() noexcept {
		return backwardIteratorType(*this, beginBackward());
	}
	
	/// Create the end iterator for backward iterating.
	backwardIteratorType rend() noexcept {
		return backwardIteratorType(*this, nullptr);
	}
	
	/// Create the backward iterator right from a node. If the node is orphan, the 
	/// end iterator will be returned.
	backwardIteratorType rfrom(objectType& object) noexcept {
		return backwardIteratorType(*this, nodePointerFrom(object));
	}
	
	/// Create the const backward iterator from reverse begining.
	constBackwardIteratorType crbegin() noexcept {
		return constBackwardIteratorType(*this, beginBackward());
	}
	
	/// Create the const end iterator for backward iterating.
	constBackwardIteratorType crend() noexcept {
		return constBackwardIteratorType(*this, nullptr);
	}
	
	/// Create the const backward iterator right from a node.
	constBackwardIteratorType crfrom(objectType& object) noexcept {
		return constBackwardIteratorType(*this, nodePointerFrom(object));
	}
	
	// The iterator specific functions, which must be protected.
private:
	// Befriending the iterators and their base classes.
	friend forwardIteratorType;
	friend backwardIteratorType;
	friend constForwardIteratorType;
	friend constBackwardIteratorType;
	friend typename forwardIteratorType::baseType;
	friend typename backwardIteratorType::baseType;
	friend typename constForwardIteratorType::baseType;
	friend typename constBackwardIteratorType::baseType;
	
	/// Perform forwarding about the iterators.
	void iterateForward(nodeType*& node) const noexcept {
		if(node == nullptr) return;
		node = node -> next;
		if(node == &sentinel) node = nullptr;
	}
	
	/// Perform backward iterating about the iterators.
	void iterateBackward(nodeType*& node) const noexcept {
		if(node == nullptr) return;
		node = node -> previous;
		if(node == &sentinel) node = nullptr;
	}
	
	/// Perform equality comparison about the nodes.
	static bool equals(	const list& lit, const nodeType* l, 
						const list& rit, const nodeType* r) noexcept {
		if(l == nullptr && r == nullptr) return true;
		if(&lit != &rit) return false;
		return l == r;
	}
	
	/// Perform inequality comparison about the nodes.
	static bool notEquals(	const list& lit, const nodeType* l, 
							const list& rit, const nodeType* r) noexcept {
		return !equals(lit, l, rit, r);
	}
	
	/// Dereferencing the objects.
	const objectType* dereference(const nodeType* node) const noexcept {
		if(node == nullptr) return nullptr;
		else return idType::object(node);
	}
public:
	/// Test whether the container is empty.
	bool empty() const noexcept { return sentinel.next == &sentinel; }

	/// Pushing an object in front of the list, returning the insertion status.
	/// (When the node specified has been inside some list, false will be returned).
	bool pushFront(objectType& object) noexcept {
		nodeType* node = idType::node(&object);
		if(!node -> isOrphanNode()) return false;
		node -> insertAfter(sentinel);
		return true;
	}
	
	/// Popping an object from the front of the list, nullptr will be returned if
	/// there's no more element in the list. Returning pointer must be safe as the 
	/// life cycle is managed elsewhere.
	objectType* popFront() noexcept {
		if(empty()) return nullptr;
		nodeType* node = sentinel.next;
		node -> unlink();
		return idType::object(node);
	}
	
	/// Pushing an object to the back of the list, returning the insertion status.
	bool pushBack(objectType& object) noexcept {
		nodeType* node = idType::node(&object);
		if(!node->isOrphanNode()) return false;
		node -> insertBefore(sentinel);
		return true;
	}
	
	/// Popping an object from the back of the list.
	objectType* popBack() noexcept {
		if(empty()) return nullptr;
		nodeType* node = sentinel.previous;
		node -> unlink();
		return idType::object(node);
	}
	
	/// Insert a node right before the forward iterator, returning the insertion status.
	bool insert(forwardIteratorType iterator, objectType& object) noexcept {
		nodeType* node = idType::node(&object);
		if(!node->isOrphanNode()) return false;
		
		nodeType* next = iterator.current;
		if(next == nullptr) next = &sentinel;
		node -> insertBefore(*next);
		return true;
	}
	
	/// Insert a node right after the backward iterator, returning the insertion status.
	bool insert(backwardIteratorType iterator, objectType& object) noexcept {
		nodeType* node = idType::node(&object);
		if(!node->isOrphanNode()) return false;
		
		nodeType* previous = iterator.current;
		if(previous == nullptr) previous = &sentinel;
		node -> insertAfter(*previous);
		return true;
	}
	
	/// Remove the node right at the forward iterator's cursor, returning the next node.
	forwardIteratorType erase(forwardIteratorType iterator) noexcept {
		nodeType* node = iterator.current;
		if(node == nullptr) return forwardIteratorType(*this, node);
		nodeType* next = node -> next;
		node -> unlink();
		return forwardIteratorType(*this, next == &sentinel? nullptr : next);
	}
	
	/// Remove the node right at the backward iterator's cursor, returning the previous node.
	backwardIteratorType erase(backwardIteratorType iterator) noexcept {
		nodeType* node = iterator.current;
		if(node == nullptr) return backwardIteratorType(*this, node);
		nodeType* previous = node -> previous;
		node -> unlink();
		return backwardIteratorType(*this, previous == &sentinel? nullptr : previous);
	}
};

} // namespace exotic
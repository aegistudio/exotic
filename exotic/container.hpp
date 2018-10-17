#pragma once
/**
 * @file exotic/container.hpp
 * @author Haoran Luo 
 * @brief The basic definition of the embedded ADT (abstract Data Type) 
 * containers.
 *
 * The ambition of the exotic template libray aims at providing an 
 * ADT-built-into-object style of design, which is useful under the 
 * memory restriction (dynamic memory allocation is restricted or 
 * not recommended).
 * 
 * To achieve our ambition. The holistic design of the library decompose
 * an ADT into three parts:
 *
 * 1. Node: nearly all ADT requires some extra memory to maintain an 
 * object. Like the previous and back node in the link list, or left,
 * right and parent node in the red-black tree. For ADTs defined in our 
 * library, they should encapsulate the ADT into a node. And the object
 * usually embed the node as a field, or union.
 *
 * 2. Access: when a node is embedded into an object, we should know how 
 * to get a node pointer from an object pointer, and how to get an object
 * pointer from a node pointer. And the definition of such relation is 
 * called access.
 *
 * 3. Container: a container aggregates nodes, so it aggregates the objects
 * containing the node. Nearly all ADTs are accessed from a container.
 * We retrieve an iterator of a container to traverse all elements inside,
 * we use an iterator to insert a new object after an iterator, or remove 
 * an object via an iterator.
 *
 * Now the dependencies are clear: the ADTs in the library should first 
 * provide definition of ADT's node. Then the user of the ADTs provide 
 * an access to the node. Finally the container aggregates the nodes 
 * via the node and accessor and provide appropriate iterating and modifying
 * methods to user.
 *
 * And combined with the concepts from exotic/pointer.hpp, all containers
 * should hold a container context as well. In our library, all containers
 * should inherit from the context, construct from a right-value reference
 * of the context and use that data.
 *
 * @see exotic/pointer.hpp
 */
#include <cstddef>
#include <type_traits>

namespace exotic {

/**
 * concept accessType {
 *     // The definition of the containing object should be forwarded.
 *     typedef someObjectType objectType;
 *
 *     // The definition of the node type should be forwarded.
 *     typedef someNodeType nodeType;
 *
 *     // Retrieve the object from a provided node. The node is guaranteed
 *     // not to be bound to an null pointer.
 *     static objectType& objectOf(nodeType&) noexcept;
 *     static const objectType& constObjectOf(const nodeType&) noexcept;
 *
 *     // Retrieve the node from a provided object. The object is guaranteed
 *     // not to be bound to an null pointer.
 *     static nodeType& nodeOf(objectType&) noexcept;
 *     static const nodeType& constNodeOf(const objectType&) noexcept;
 * };
 */

/// Primary abstraction for all ADT containers in the library. The underlying
/// container should implement the 
template<typename accessType, typename contextType, 
	typename = typename accessType::nodeType>
class container {};

/// Holds the mostly used traits in the library. Like begin()/end() and 
/// rbegin()/rend() in the stl. The iterator trait always invokes certain
/// method inside the link list node.
namespace iterateTrait {
	
/// Perform forawrd iteration. Like the begin()/end() iteration in STL.
struct forward {
	/// The increment method is delegated to the static backward method.
	template<typename accessType, typename contextType> 
	static typename accessType::nodeType* next(
		const container<accessType, contextType>& containerObject,
		typename accessType::nodeType* currentNode) noexcept {
		return containerObject.forward(currentNode);
	}
};

/// Perform backward iteration. Like the rbegin()/rend() iteration in STL.
class backward {
	/// The increment method is delagated to the static backward method.
	template<typename accessType, typename contextType>
	static typename accessType::nodeType* next(
		const container<accessType, contextType>& containerObject,
		typename accessType::nodeType* currentNode) noexcept {
		return containerObject.backward(currentNode);
	}
};

}; // namespace exotic::iterateTrait
 
/// The common iterator built on the abstract data types and the member types.
template<typename accessType, typename contextType, typename iterateTraitType>
struct iterator {
	/// The node type that is maintained in the iterator.
	typedef typename accessType::nodeType nodeType;
	
	/// The object type that is accessed by the access type.
	typedef typename accessType::objectType objectType;
	
	/// The type of the container which provides access.
	typedef container<accessType, contextType> containerType;
	
private:
	/// The reference to the underlying container.
	const containerType& containerObject;

	/// The current visiting unit of the iterator. Please notice that when the 
	/// node is null pointer. It will always indicates the end of traversal.
	nodeType* current;

	/// Perform construction of the iterator.
	iterator(const containerType& containerObject, nodeType* initial) noexcept: 
		containerObject(containerObject), current(initial) {}
	
	/// Perform default construction of the iterator.
	iterator(const containerType& containerObject) noexcept: 
		containerObject(containerObject), current() {}
	
	/// Declare the container type to the the friend class.
	friend containerType;
public:
	/// The dereference operation of the iterator.
	objectType* operator->() noexcept
		{	return &accessType::objectOf(*current);	}
	
	/// The const-dereference operation of the iterator.
	const objectType* operator->() const noexcept 
		{	return &accessType::objectOf(*current);	}
	
	/// The asterisk operator.
	objectType& operator*() noexcept 
		{	return accessType::objectOf(*current);	}
	
	/// The const-asterisk operator.
	const objectType& operator*() const noexcept 
		{	return accessType::constObjectOf(*current);	}
	
	/// Perform increment operation of the iterator.
	iterator& operator++() noexcept { 
		current = iterateTraitType::template next<accessType>(
				containerObject, current); 
		return *this;
	}
	
	/// Perform equality comparison between the nodes.
	bool operator==(const iterator& r) const noexcept 
		{	return current == r.current;	}
	
	/// Perform inequality comparison between the nodes.
	bool operator!=(const iterator& r) const noexcept 
		{	return current != r.current;	}
};

/// Forward some special iterator traits. Like forward iterator and backward iterator.
template<typename accessType, typename contextType>
using forwardIterator = iterator<accessType, contextType, iterateTrait::forward>;
template<typename accessType, typename contextType>
using backwardIterator = iterator<accessType, contextType, iterateTrait::backward>;

} // namespace exotic
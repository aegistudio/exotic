#pragma once
/**
 * @file exotic/node.hpp
 * @author Haoran Luo
 * @brief Defines the node indentity and the list of node.
 *
 * In the library of exotic, we should embed the node of desired data structure (e.g. link 
 * list, red-black tree) into the object, and container of objects are formed by the 
 * inter-correlation of the beaded nodes.
 *
 * To identify a node inside an object for further use, as the node inside a class layout is 
 * relatively stable, we could create a 3-tuple of object type, node type and node's offset 
 * in the object's layout. Such identity is recorded as exotic::nodeid.
 */
#include <type_traits>
#include <cstddef>
#include <utility>
 
namespace exotic{

/// The 3-tuple for the information for a node inside object.
template<typename cobjectType, typename cnodeType, cnodeType cobjectType::*nodePointer>
struct nodeid {
	/// Forward the definition of the object.
	typedef cobjectType objectType;
	
	/// Forward the type of the node.
	typedef cnodeType nodeType;
private:
	/// The offset of a node inside an object.
	static constexpr size_t off(const objectType* obj) noexcept {
		return reinterpret_cast<size_t>(&(*obj.*nodePointer));
	}
public:
	/// Ensure that the char's are byte-sized.
	static_assert(sizeof(char) == 1, "The size of char should be byte.");
	
	/// Retrieve the object when the pointer of a node is known.
	static const objectType* object(const nodeType* n) noexcept {
		return reinterpret_cast<const objectType*>(
			reinterpret_cast<const char*>(n) - off(nullptr));
	}
	
	/// The non-const version of object retrieving.
	static objectType* object(nodeType* n) noexcept {
		return const_cast<objectType*>(object(static_cast<const nodeType*>(n)));
	}
	
	/// Retrieve the node with the pointer to the object.
	static const nodeType* node(const objectType* obj) noexcept {
		return &(*obj.*nodePointer);
	}
	
	/// The non-const version of node retrieving.
	static nodeType* node(objectType* obj) noexcept {
		return const_cast<nodeType*>(node(static_cast<const objectType*>(obj)));
	}
};

/// Check whether the defineNode() macro has already been defined.	
#ifndef defineNode

/// The macro for defining an abbreviation of a node information.
#define defineNode(abbreviateType, objectType, fieldName)\
struct abbreviateType : exotic::nodeid<objectType, \
	decltype(objectType::fieldName), &objectType::fieldName> {};
#else

/// Check whether it is required to remove define node in exotic.
#ifndef EXOTIC_NODEFINE_NODE
static_assert(false, "The define node macro has conflicted with the one on exotic. You "
	"could disable the one in exotic by defining macro EXOTIC_NODEFINE_NODE.");
#endif

#endif

/// The node list with object type specified. (Primary specification).
template<typename objectType, typename... nodeTypes> struct typedNodeList {};

/// A internal template node for the typed node list.
template<typename objectType, typename headNodeType, typename... tailNodeTypes>
struct typedNodeList<objectType, headNodeType, tailNodeTypes...> {
	/// Ensure that the current node is of the object's type.
	static_assert(std::is_same<typename headNodeType::objectType, objectType>::value,
		"The nodes specified in the list should be inside the same object.");
	
	/// The next template node of the typed node list.
	typedef typedNodeList<objectType, tailNodeTypes...> next;
	
	/// Perform operations on the node list.
	template<template<typename, typename> typename executorType, typename... args>
	static inline void execute(objectType* object, args&&... a) noexcept {
		executorType<objectType, typename headNodeType::nodeType>::execute(
			object, headNodeType::node(object), std::forward<args>(a)...);
		next::template execute<executorType, args...>(object, std::forward<args>(a)...);
	}
	
	/// Perform operation on the node list (const version). We have to write twice 
	/// otherwise the compiler won't be able to judge which version to call.
	template<template<typename, typename> typename executorType, typename... args>
	static inline void execute(const objectType* object, args&&... a) noexcept {
		executorType<objectType, typename headNodeType::nodeType>::execute(
			object, headNodeType::node(object), std::forward<args>(a)...);
		next::template execute<executorType, args...>(object, std::forward<args>(a)...);
	}
	
	/// Perform dual-object operations on the node list.
	template<template<typename, typename> typename executorType, typename... args>
	static inline void execute(objectType* aobj, objectType* bobj, args&&... a) noexcept {
		executorType<objectType, typename headNodeType::nodeType>::execute(
			aobj, headNodeType::node(aobj), bobj, headNodeType::node(bobj), std::forward<args>(a)...);
		next::template execute<executorType, args...>(aobj, bobj, std::forward<args>(a)...);
	}
	
	/// Perform dual-object operation on the node list (const version).
	template<template<typename, typename> typename executorType, typename... args>
	static inline void execute(const objectType* aobj, const objectType* bobj, args&&... a) noexcept {
		executorType<objectType, typename headNodeType::nodeType>::execute(
			aobj, headNodeType::node(aobj), bobj, headNodeType::node(bobj), std::forward<args>(a)...);
		next::template execute<executorType, args...>(aobj, bobj, std::forward<args>(a)...);
	}
};

/// The end of the typed node list.
template<typename objectType> struct typedNodeList<objectType> {
	/// The end of the perform operation.
	template<template<typename, typename> typename executorType, typename... args>
	static inline void execute(objectType* object, args&&...) noexcept {}
	
	/// The end of the perform operation (const version).
	template<template<typename, typename> typename executorType, typename... args>
	static inline void execute(const objectType* object, args&&...) noexcept {}
	
	/// The end of the dual-object perform operation.
	template<template<typename, typename> typename executorType, typename... args>
	static inline void execute(objectType*, objectType*, args&&...) noexcept {}
	
	/// The end of the dual-object perform operation (const version).
	template<template<typename, typename> typename executorType, typename... args>
	static inline void execute(const objectType*, const objectType*, args&&...) noexcept {}
};

/// The node list that wraps the typed node list. (Must have at least one type of course, 
/// and the object type is retrieved from the first type).
template<typename headNodeType, typename... tailNodeTypes> struct nodeList {
	typedef typename headNodeType::objectType objectType;
	
	/// Perform operations on the node list.
	template<template<typename, typename> typename executorType, typename... args>
	static inline void execute(objectType* object, args... a) noexcept {
		typedNodeList<objectType, headNodeType, tailNodeTypes...>
			::template execute<executorType, args...>(object, std::forward<args>(a)...);
	}
	
	/// Perform operation on the node list (const version).
	template<template<typename, typename> typename executorType, typename... args>
	static inline void execute(const objectType* object, args... a) noexcept {
		typedNodeList<objectType, headNodeType, tailNodeTypes...>
			::template execute<executorType, args...>(object, std::forward<args>(a)...);
	}
	
	/// Perform dual-object operations on the node list.
	template<template<typename, typename> typename executorType, typename... args>
	static inline void execute(objectType* aobj, objectType* bobj, args... a) noexcept {
		typedNodeList<objectType, headNodeType, tailNodeTypes...>
			::template execute<executorType, args...>(aobj, bobj, std::forward<args>(a)...);
	}
	
	/// Perform dual-object operation on the node list (const version).
	template<template<typename, typename> typename executorType, typename... args>
	static inline void execute(const objectType* aobj, const objectType* bobj, args... a) noexcept {
		typedNodeList<objectType, headNodeType, tailNodeTypes...>
			::template execute<executorType, args...>(aobj, bobj, std::forward<args>(a)...);
	}
};


/// Check whether the defineNodeList() macro has already been defined.	
#ifndef defineNodeList

/// The macro for defining an abbreviation of a node information.
#define defineNodeList(abbreviateType, ...)\
struct abbreviateType : exotic::nodeList<__VA_ARGS__> {};
#else

/// Check whether it is required to remove define node in exotic.
#ifndef EXOTIC_NODEFINE_NODELIST
static_assert(false, "The define node list macro has conflicted with the one on exotic. "
	"You could disable the one in exotic by defining macro EXOTIC_NODEFINE_NODELIST.");
#endif

#endif

} // namespace exotic
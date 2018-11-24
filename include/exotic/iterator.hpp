#pragma once
/**
 * @file exotic/iterator.hpp
 * @author Haoran Luo
 * @brief Defines the general iterators.
 *
 * To remove duplicated code about iterators, the file provides a unified interface for 
 * defining an iterator. The concrete definition of iterator is referred via metaprogramming.
 *
 * Generally, iteration template interface should be created, as following:
 * ```c++
 * concept iterable {
 *     // For maintaining iteration info at runtime.
 *     typedef <userdataType> userdataType;
 *     
 *     // With which the internal node will be presented.
 *     typedef <nodeType> nodeType;
 *
 *     // With which the object is present.
 *     typedef <objectType> objectType;
 *
 *     // To iterate in forward direction.
 *     void iterateForward(userdataType& userdata, nodeType*& current) noexcept;
 *
 *     // To iterate in backward direction.
 *     void iterateBackward(userdataType& userdata, nodeType*& current) noexcept;
 *
 *     // To perform dereferencing with the node type. (Usually return nullptr for invalid
 *     // location, to instantly tear down the program instead of running in unpredictable state.
 *     objectType* dereference(const userdataType& userdata, const nodeType* current) noexcept;
 *
 *     // To perform equality comparison, or to terminate the iteration if required.
 *     bool equals(const iterable& lit, const userdataType& lud, const nodeType* lc, 
 *                 const iterable& rit, const userdataType& rud, const nodeType* rc) noexcept;
 *
 *     // To perform unequality comparison, or to terminate the iteration if required.
 *     bool notEquals(const iterable& lit, const userdataType& lud, const nodeType* lc, 
 *                    const iterable& rit, const userdataType& rud, const nodeType* rc) noexcept;
 * };
 * ```
 * Specially, when userdata type is void, all userdata parameters will be ignored.
 */
#include <type_traits>
 
namespace exotic {
	
/// @brief The common behavior of the iterators. The placeholder type eliminates the possibility of 
/// comparing two different iterators. The mutable flag swith on or off the non-const version for 
/// dereferencing the pointers.
template<class iterableType, typename placeholderType, typename userdataType, bool mutableFlag>
struct iteratorBase {
	/// Which node type is the container about to iterate on.
	typedef typename iterableType::nodeType nodeType;
	
	/// Which object contains the node, linked by the nodeid<> template.
	typedef typename iterableType::objectType objectType;
protected:
	/// The constant reference to the iterable. As the iteration operations should always not 
	/// mutate the iterable / container.
	const iterableType& iterable;

	/// The current userdata.
	userdataType userdata;
	
	/// The current iterating node.
	nodeType* current;

	/// Befriending the iterable class for instantiating the template.
	friend iterableType;
	
	/// The non-trivial constructor, which is protected.
	iteratorBase(const iterableType& iterable, userdataType&& userdata, nodeType* current) noexcept: 
		iterable(iterable), userdata(userdata), current(current) {}
public:
	/// Compare whether two iterators point to the same position, or pointing to the same pseudo 
	/// position. (Mostly, we would always expect the end of an iterable and an orphan node to be 
	/// equal at the aspect of iteration, so that it presents that the orphan node points to the 
	/// end of the container).
	bool operator==(const iteratorBase& rhs) const noexcept {
		return iterableType::equals(iterable, userdata, current, rhs.iterable, rhs.userdata, rhs.current);
	}
	
	/// Compare whether two iterators point to different positions or pseudo positions.
	bool operator!=(const iteratorBase& rhs) const noexcept {
		return iterableType::notEquals(iterable, userdata, current, rhs.iterable, rhs.userdata, rhs.current);
	}
	
	/// Dereferencing the constant object. The nullptr could be returned in order to cause a 
	/// segmentation fault, indicating that the user uses the containers incorrectly. (And the 
	/// template provides no means of exception, so this would be the only way to dereference).
	const objectType& operator*() const noexcept {	return *iterable.dereference(userdata, current); }
	
	/// Pointer-style dereferencing the constant object.
	const objectType* operator->() const noexcept {	return iterable.dereference(userdata, current);	}
	
	/// Dereferencing the mutable object.
	template<bool forwardedMutableFlag = mutableFlag>
	typename std::enable_if<forwardedMutableFlag, objectType&>::type operator*() noexcept 
		{	return *const_cast<objectType*>(iterable.dereference(userdata, current)); }
	
	/// Pointer-style dereferencing the mutable object.
	template<bool forwardedMutableFlag = mutableFlag>
	typename std::enable_if<forwardedMutableFlag, objectType*>::type operator->() noexcept 
		{	return const_cast<objectType*>(iterable.dereference(userdata, current)); }
};

/// @brief The common behavior of the iterators, with the userdata type to be void. For this 
/// reason, almost every invocations to the iterable method will omit the userdata parameter.
template<class iterableType, typename placeholderType, bool mutableFlag>
struct iteratorBase<iterableType, placeholderType, void, mutableFlag> {
	/// Which node type is the container about to iterate on.
	typedef typename iterableType::nodeType nodeType;
	
	/// Which object contains the node, linked by the nodeid<> template.
	typedef typename iterableType::objectType objectType;
protected:
	/// The constant reference to the iterable. As the iteration operations should always not 
	/// mutate the iterable / container.
	const iterableType& iterable;
	
	/// The current iterating node.
	nodeType* current;
	
	/// Befriending the iterable class for instantiating the template.
	friend iterableType;
	
	/// The non-trivial constructor, which is protected, and the userdata need not to be provided.
	iteratorBase(const iterableType& iterable, nodeType* current) noexcept: 
		iterable(iterable), current(current) {}
public:
	/// Compare whether two iterators point to the same position or pseudo position.
	bool operator==(const iteratorBase& rhs) noexcept {	
		return iterableType::equals(iterable, current, rhs.iterable, rhs.current);
	}
	
	/// Compare whether two iterators point to different positions or pseudo positions.
	bool operator!=(const iteratorBase& rhs) noexcept {
		return iterableType::notEquals(iterable, current, rhs.iterable, rhs.current);
	}
	
	/// Dereferencing the constant object.
	const objectType& operator*() const noexcept {	return *iterable.dereference(current);	}
	
	/// Pointer-style dereferencing the constant object.
	const objectType* operator->() const noexcept {	return iterable.dereference(current);	}
	
	/// Dereferencing the mutable object.
	template<bool forwardedMutableFlag = mutableFlag>
	typename std::enable_if<forwardedMutableFlag, objectType&>::type operator*() noexcept 
		{	return *const_cast<objectType*>(iterable.dereference(current));	}
		
	/// Pointer-style dereferencing the mutable object.
	template<bool forwardedMutableFlag = mutableFlag>
	typename std::enable_if<forwardedMutableFlag, objectType*>::type operator->() noexcept 
		{	return const_cast<objectType*>(iterable.dereference(current));	}
};

// Placeholder for forward iterator.
class forwardIteratorType;

/// @brief Defines the general form of forward iterator. (You could easily set a common 
/// forward iterator to const forward iterator by setting mutable flag to false).
template<class iterableType, typename userdataType, bool mutableFlag> struct forwardIterator : 
	public iteratorBase<iterableType, forwardIteratorType, userdataType, mutableFlag> {
		
	/// Which node type is the container about to iterate on.
	typedef typename iterableType::nodeType nodeType;
	
	/// Forward the definition of the iterator base of this concrete iterator.
	typedef iteratorBase<iterableType, forwardIteratorType, userdataType, mutableFlag> baseType;
protected:
	// Only the iterable type could construct such iterator.
	friend iterableType;
	
	// The non-trivial constructor, which is protected.
	forwardIterator(const iterableType& iterable, userdataType&& userdata, nodeType* current) noexcept:
			baseType(iterable, userdata, current) {}
public:
	// Move forward in the iterator.
	forwardIterator& operator++() noexcept {
		baseType::iterable.iterateForward(baseType::userdata, baseType::current);
		return *this;
	}
};

/// @brief Defines the special form of forward iterator with void userdata type.
template<class iterableType, bool mutableFlag> struct forwardIterator<iterableType, void, mutableFlag> : 
	public iteratorBase<iterableType, forwardIteratorType, void, mutableFlag> {
	
	/// Which node type is the container about to iterate on.
	typedef typename iterableType::nodeType nodeType;
	
	/// Forward the definition of the iterator base of this concrete iterator.
	typedef iteratorBase<iterableType, forwardIteratorType, void, mutableFlag> baseType;
protected:
	// Only the iterable type could construct such iterator.
	friend iterableType;
	
	// The non-trivial constructor, which is protected, and the userdata is omitted.
	forwardIterator(const iterableType& iterable, nodeType* current) noexcept: baseType(iterable, current) {}
public:
	// Move forward in the iterator. The iterable needs not to accept in the userdata.
	forwardIterator& operator++() noexcept {
		baseType::iterable.iterateForward(baseType::current);
		return *this;
	}
};

// Placeholder for backward iterator.
class backwardIteratorType;

/// @brief Defines the general form of backward iterator. (You could easily set a common 
/// backward iterator to const backward iterator by setting mutable flag to false).
template<class iterableType, typename userdataType, bool mutableFlag> struct backwardIterator : 
	public iteratorBase<iterableType, backwardIteratorType, userdataType, mutableFlag> {
	
	/// Which node type is the container about to iterate on.
	typedef typename iterableType::nodeType nodeType;
	
	/// Forward the definition of the iterator base of this concrete iterator.
	typedef iteratorBase<iterableType, backwardIteratorType, userdataType, mutableFlag> baseType;
protected:
	// Only the iterable type could construct such iterator.
	friend iterableType;
	
	// The non-trivial constructor, which is protected.
	backwardIterator(const iterableType& iterable, userdataType&& userdata, nodeType* current) noexcept:
			baseType(iterable, userdata, current) {}
public:
	// Move backward in the iterator.
	backwardIterator& operator++() noexcept {
		baseType::iterable.iterateBackward(baseType::userdata, baseType::current);
		return *this;
	}
};

/// @brief Defines the special form of backward iterator with void userdata type.
template<class iterableType, bool mutableFlag> struct backwardIterator<iterableType, void, mutableFlag> : 
	public iteratorBase<iterableType, backwardIteratorType, void, mutableFlag> {
		
	/// Which node type is the container about to iterate on.
	typedef typename iterableType::nodeType nodeType;
	
	/// Forward the definition of the iterator base of this concrete iterator.
	typedef iteratorBase<iterableType, backwardIteratorType, void, mutableFlag> baseType;
protected:
	// Only the iterable type could construct such iterator.
	friend iterableType;
	
	// The non-trivial constructor, which is protected.
	backwardIterator(const iterableType& iterable, nodeType* current) noexcept: baseType(iterable, current) {}
public:
	// Move backward in the iterator. The iterable needs not to accept in the userdata.
	backwardIterator& operator++() noexcept {
		baseType::iterable.iterateBackward(baseType::current);
		return *this;
	}
};

};	// namespace exotic.
#pragma once
/**
 * @file exotic/pointer.hpp
 * @author Haoran Luo
 * @brief Pointer type abstractions.
 *
 * A largely abused term of pointer is an memory address (either virtual or 
 * physical) bound with a type. Such pointer is mandatory obviously, and we 
 * won't talk too much about it. We will talk about the case that such pointer
 * is not enough.
 *
 * If we use a data structure that might reallocate, like std::vector, we link
 * the elements inside the vector via some node, your program will surely
 * go into an insane state when data structure get reallocated.
 *
 * For such situations, we can easily store an index instead of a pointer.
 * Because the order of the elements won't get mutated even when the data 
 * structure get reallocated.
 *
 * So comes the generalization of the pointer. Here's the main operations 
 * about the pointers:
 * - (C) get the pointing object of the pointer.
 * - (C) set what the pointer is pointing at. 
 * - test whether two pointers are equal.
 * - test whether a pointer is null pointer.
 * - set the pointer to a null pointer.
 * - swap the contents of two pointers.
 * For the case that requires an index number (marked (C), it always requires 
 * the pointer / reference to the data structure as well.
 *
 * The pointer is what lies in a node. When someone wants to access the 
 * contextual operations on the object, it is required to instantiate the 
 * pointer into a pointer access, via passing in the pointer context.
 */
#include <type_traits>
 
namespace exotic {

/// Forward the access type of random pointer first.
template<typename targetType>
struct randomPointerAccess;

// This class have no use but just a work-around for omitting context.
struct randomPointerContext {};

/**
 * @brief The well known pointer, which stores the memory address bounded
 * with certain type.
 */
template<typename targetType>
class randomPointer {
	//// The pointer of the address.
	targetType* pointer;
public:
	/// The pointer context access should have access to the fields.
	friend struct randomPointerAccess<targetType>;
	
	/// Instantiate a pointer with null state.
	randomPointer() noexcept: pointer(nullptr) {}
	
	/// Test whether a pointer is null pointer.
	bool operator==(decltype(nullptr) nullPointer) const noexcept
		{ return pointer == nullptr; }
	
	/// Test whether two pointers are equal.
	bool operator==(randomPointer& a) const noexcept 
		{ return pointer == a.pointer; }
	
	/// Convert the random pointer into a pointer access type.
	template<typename contextType> randomPointerAccess<targetType> 
	operator()(contextType& context) noexcept {
		return randomPointerAccess<targetType>(*this);
	}
	
	/// Update the content of a pointer with another pointer
	randomPointer& operator&=(const randomPointer& a) noexcept  {
		pointer = a.pointer;
		return *this;
	}
	
	/// Swap the content of the random pointer.
	void swap(randomPointer& a) noexcept {
		targetType* tempPointer = pointer;
		pointer = a.pointer;
		a.pointer = tempPointer;
	}
};

/**
 * @brief The pointer access for random access pointer. The access actually
 * have nothing to do with the context, so the context is omitted.
 */
template<typename targetType>
struct randomPointerAccess {
	// The pointer type that should be stored.
	typedef randomPointer<targetType> pointerType;
private:
	// Stores the reference to the pointer type.
	pointerType& ref;
public:
	// Instantiate the pointer access.
	randomPointerAccess(pointerType& ref) noexcept: ref(ref) {}
	
	// Retrieve the containing object of the pointer.
	operator targetType*() noexcept { return ref.pointer; }
	
	// Retrieve the const containing of the pointer.
	operator targetType const*() const noexcept { return ref.pointer; }
	
	// Update the value of the pointer access.
	randomPointerAccess& operator=(targetType* newPointer) noexcept {
		ref.pointer = newPointer;
		return *this;
	}
};

/// Forward the access type of index pointer first.
template<typename targetType, typename indexType, typename contextType>
struct indexPointerAccess;

/**
 * @brief The alternative pointer built on index, which requires an abstract
 * virtual container in order to perform indexing.
 */
template<typename targetType, typename indexType>
class indexPointer {
	/// The index of the object. When it is null pointer, it will always be 0.
	indexType index;
public:
	/// The pointer context access should have access to the fields.
	template<typename, typename, typename>
	friend struct indexPointerAccess;
	
	/// Initialize into the null state.
	indexPointer(): index(0) {}
	
	/// Test whether a pointer is null pointer.
	bool operator==(decltype(nullptr) nullPointer) const noexcept
		{ return index == 0; }
	
	/// Test whether two pointers are equal.
	bool operator==(indexPointer& a) const noexcept 
		{ return index == a.index; }
	
	/// Convert the random pointer into a pointer access type.
	template<typename contextType> 
	indexPointerAccess<targetType, indexType, contextType> 
	operator()(contextType& context) noexcept {
		return indexPointerAccess<targetType, 
			indexType, contextType>(context, *this);
	}
	
	/// Update the content of a pointer with another pointer
	indexPointer& operator&=(const indexPointer& a) noexcept  {
		index = a.index;
		return *this;
	}
	
	/// Swap the content of the random pointer.
	void swap(indexPointer& a) noexcept {
		indexType tempIndex = index;
		index = a.index;
		a.index = tempIndex;
	}
};

/**
 * @brief The pointer access for index pointer. The access actually.
 */
template<typename targetType, typename indexType, typename pointerContextType>
struct indexPointerAccess {
	// The pointer type that should be stored.
	typedef indexPointer<targetType, indexType> pointerType;
	
	// The context type that should be stored.
	typedef pointerContextType contextType;
private:
	// The context of the pointer to access.
	contextType& ctx;
	
	// The pointer that should be modified.
	pointerType& ref;
public:
	// The initializer for the pointer access.
	indexPointerAccess(contextType& ctx, pointerType& ref) noexcept:
		ctx(ctx), ref(ref) {}
	
	// Retrieve the pointer of the object contained in the object.
	operator targetType*() noexcept {
		if(ref.index == 0) return nullptr;
		else return ctx.index(ref.index - 1);
	}
	
	// Retrieve the pointer of the object contained in the object.
	operator targetType* const() const noexcept {
		if(ref.index == 0) return nullptr;
		else return ctx.constIndex(ref.index - 1);
	}
	
	// Update the value of the index.
	indexPointerAccess& operator=(targetType* newPointer) noexcept {
		if(newPointer == nullptr) ref.index = 0;
		else ref.index = ctx.indexOf(newPointer) + 1;
		return *this;
	}
};

/**
 * @brief the default index context object. It is so simple that 
 * comments are nearly not required.
 */
template<typename vectorType, typename indexType>
class defaultIndexContext {
	// Just store the vector's content.
	vectorType& vector;
public:
	// The component type of each object, which could be directly inferred by the 
	// subscript expression.
	typedef typename std::decay<decltype(std::declval<vectorType>()[0])>::type targetType;

	// Initialize the context with an arbitrary content.
	defaultIndexContext(vectorType& vector) noexcept: vector(vector) {}
		
	// Attempt to index an object in the array.
	targetType* index(indexType idx) noexcept 
		{ return &vector[idx]; }
	
	// Attempt to index a const object in the array.
	targetType const* constIndex(indexType idx) const noexcept 
		{ return &vector[idx]; }
	
	// Evaluate the index of a target object.
	indexType indexOf(targetType* targetObject) noexcept {
		return (indexType)(targetObject - &vector[0]);
	}
};

} // namespace exotic.

// Just forward the swap methods into the std namespace.
namespace std {
template<typename targetType>
void swap(	exotic::randomPointer<targetType>& left, 
			exotic::randomPointer<targetType>& right) 
	noexcept {	left.swap(right);	}
	
template<typename targetType, typename indexType>
void swap(	exotic::indexPointer<targetType, indexType>& left,
			exotic::indexPointer<targetType, indexType>& right)
	noexcept {	left.swap(right);	}
}; // namespace std.
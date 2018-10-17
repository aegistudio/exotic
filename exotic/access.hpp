#pragma once
/**
 * @file exotic/access.hpp
 * @author Haoran Luo
 * @brief Commonly used access type definition.
 *
 * This file defines some basic member types that could be used as an access type.
 * @see exotic/container.hpp
 */

namespace exotic {

/**
 * @brief An access to normal member of a object, via provided member pointer. To 
 * simplify the code, please use the macro "normalMemberAccessDefine", which take 
 * into the aliased access type's name, the object class, and the field name.
 */
template<typename cobjectType, typename cnodeType, cnodeType cobjectType::* memberPointer>
struct normalMemberAccess {
	// Forward the access type definitions.
	typedef cobjectType objectType;
	typedef cnodeType nodeType;
private:
	/// The helper for getting offset from the object pointer. We use a pointer
	/// to prevent the compiler complaining about incomplete type.
	static constexpr size_t offset(const objectType* object) noexcept {
		return reinterpret_cast<size_t>(&(*object.*memberPointer));
	}
public:
	/// Retrieve the containing object from a node reference.
	static objectType& objectOf(nodeType& node) noexcept {
		return *reinterpret_cast<objectType*>(
			((char*)&node) - offset(nullptr));
	}
	
	/// Retrieve the const containing object from the (const) node reference.
	static const objectType& constObjectOf(const nodeType& node) noexcept {
		return *reinterpret_cast<const objectType*>(
			((const char*)&node) - offset(nullptr));
	}
	
	/// Retrieve the node from an object pointer.
	static nodeType& nodeOf(objectType& object) noexcept {
		return object.*memberPointer;
	}
	
	/// Retrieve the (const) node from a const object pointer.
	static const nodeType& nodeOf(const objectType& object) noexcept {
		return object.*memberPointer;
	}
};

#define normalMemberAccessDefine(caccessType, cobjectType, name) \
	typedef exotic::normalMemberAccess<cobjectType, \
	std::decay<decltype(cobjectType::name)>::type, &cobjectType::name> caccessType;
	
} // namespace exotic.
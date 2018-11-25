#pragma once
/**
 * @file exotic/key.hpp
 * @author Haoran Luo
 * @brief Defines the key node.
 *
 * In the library of exotic, the key-value relation is also built into the object to avoid
 * memory allocation. The key serves as the source of broadcasting a key update. 
 *
 * The term value nodes referes to the actual data structure nodes inside the object, because
 * the object serves as the value and the nodes inside the object fabricates the key-value
 * relcation.
 *
 * The value nodes should have `template<keyIdType, valueIdType> update()` and `valueSwap()` 
 * defined in order to work with the key node.
 */
#include <type_traits>
#include <utility>
#include "exotic/node.hpp"
 
namespace exotic {

/**
 * @brief A key is a combination of the internal object and the slot for broadcasting.
 *
 * The `operator==` must be implemented for the key type, so that the detection of update
 * could be detected. The key must also be no-throw move constructible and assignable.
 *
 * The keyIdType must be a node id type. So that the key node is capable of reverse lookup
 * the object when it should be updated.
 *
 * The nodeListType must be an instance of `exotic::nodeList`, with the key and the nodes 
 * specified in the list of the same object.
 */
template<typename ckeyType, typename keyIdType, typename nodeListType> struct keyNode {
	typedef ckeyType keyType;
private:
	static_assert(std::is_same<decltype(std::declval<keyType>() == std::declval<keyType>()),
		bool>::value, "The key must be capable of comparing equality.");
	static_assert(std::is_nothrow_move_constructible<keyType>::value &&
		std::is_nothrow_move_assignable<keyType>::value, 
		"The key must be no-throw move constructible and assignable.");
	
	/// Just store the key type here.
	keyType key;
	
	/// The helper template for broadcasting update. The nodeType must befriend 
	/// `exotic::keyNode` while providing private `template<keyIdType, valueIdType> update()`, 
	/// to avoid external modification.
	template<typename objectType, typename nodeIdType, typename nodeType> struct executorUpdate { 
		static void execute(objectType* obj, nodeType* node, 
			const keyType& paramOldKey, const keyType& paramNewKey) noexcept {
			node -> nodeType::template update<keyIdType, nodeIdType>(paramOldKey, paramNewKey);
		}
	};
	
	/// The helper template for performing kvswapping. The nodeType must befriend 
	/// `exotic::keyNode` type while providing private `valueSwap()`, to avoid external modification.
	template<typename objectType, typename nodeIdType, typename nodeType> struct executorKvswap {
		static void execute(objectType* aobj, nodeType* anode, 
			objectType* bobj, nodeType* bnode) noexcept {
			(*anode).valueSwap(*bnode);	
		}
	};
public:
	/// Defines the assignment operator.
	keyNode& operator=(keyType&& nkey) noexcept {
		// Bind the keys to const reference.
		const keyType& oldKey = key;
		const keyType& newKey = nkey;
		
		// Make sure both key and the node list are inside the same object.
		static_assert(std::is_same<typename keyIdType::objectType, 
			typename nodeListType::objectType>::value,
			"The key and the nodes should be inside the same object.");
		
		// Broadcast update on the key.
		nodeListType::template execute<executorUpdate, 
			const keyType&, const keyType&>(keyIdType::object(this), oldKey, newKey);
		
		// Finally apply the assignment.
		key = std::forward<keyType>(nkey);
		return *this;
	}
	
	/// Retrieve the key via strongly typed conversion.
	operator const keyType& () const noexcept { return key; }
	
	/// Defines the swap method, which will swap all value nodes linked by this node.
	void kvswap(keyNode& a) noexcept {
		using std::swap;
		swap(key, a.key);
		nodeListType::template execute<executorKvswap>(
			keyIdType::object(this), keyIdType::object(&a));
	}
	
	/// The default constructor, only enabled while the node is no-throw defaultly constructible.
	template<typename = typename std::is_nothrow_default_constructible<keyType>::type> 
	keyNode() noexcept: key() {}
	
	/// The non-default contructor, always enabled.
	keyNode(keyType&& key) noexcept: key(std::forward<keyType>(key)) {}
	
	/// Cannot copy construct the keynode, or copy assign a key node.
	keyNode(const keyNode&) = delete;
	keyNode& operator=(const keyNode&) = delete;
	
	/// Use the kvswap instead of invoking move constructor or assignment.
	keyNode(keyNode&&) = delete;
	keyNode& operator=(keyNode&&) = delete;
};

} // namespace exotic.
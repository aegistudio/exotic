#pragma once
/**
 * @brief exotic/rbtree.hpp
 * @author Haoran Luo
 * @brief The red-black tree definition.
 *
 * This file defines the red-black tree used in exotic. To incorporate with the exotic
 * key-value model, multiple node of the same value might be found. However, the first
 * node visited is guaranteed to be the node that is most recently modified.
 */

#include "exotic/key.hpp"
#include "exotic/scope.hpp"
 
namespace exotic {

/**
 * @brief Defines the core part of a red-black tree node.
 * 
 * The `exotic::rbtreeNodeBase` does not contains lifecycle related information, which 
 * should be managed by a `exotic::rbtreeNode<scopeType>` instead. This class just 
 * provides some easy-to-use functions for the `exotic::rbtreeNode<scopeType>`.
 */
class rbtreeNodeBase {
	// Befriending the exotic::rbtreeNode<scopeType> so that the concrete node have
	// full access to the private methods.
	template<typename scopeType> friend class rbtreeNode;
	
	/// @brief The possible flags that could appear in the flags field.
	enum rbtreeFlag {
		/// The node is not inside any red-black tree, or it is currently the sentinel
		/// node inside a red-black tree.
		rbfOrphan = 0,
		
		/// The node is inside the red-black tree, and there's only one object
		/// having the key. The node has color and child state.
		rbfSingle = 1,
		
		// The followed state are nodes inside the red-black tree, but there're
		// multiple objects sharing the same key.
		
		/// The node is currently inside the red-black tree (not the multinode's
		/// queue), so is called external node. The node has color and child state.
		rbfMulext = 2,
		
		/// The node is an internal node of a multinode, it could be modified by
		/// the multinode's flags. And such node does not have color or child state.
		rbfMulint = 3,
		
		/// The mask for testing the type of a node.
		rbfTypeMask = 3,
		
		/// The node is inside the multinode's queue. And is the front element.
		rbfMulintFront = 4,
		
		/// The node is inside the multinode's queue, And is the back element.
		rbfMulintBack = 8,
		
		/// Neither the front node nor the back node.
		rbfMulintInternal = 0,
		
		/// The mask for checking a multinode's position.
		rbfMulintMask = 12,
		
		/// The color of a rbtree node, is red.
		rbfColorRed = 0,
		
		/// The color of a rbtree node, is black.
		rbfColorBlack = 4,
		
		/// The mask for checking a multinode's color.
		rbfColorMask = 4,
	};
	
	/// The flags field for the rbtree.
	rbtreeFlag flags;
	
	/// The pointers by genre in the rbtree.
	union {
		/// Only valid when flags & rbfTypeMask == rbfOrphan, and is sentinel node.
		struct {
			// Just here to place holding the parent pointer.
			rbtreeNodeBase* _1;
			
			/// The direct root node by the setinel. Which only be black node.
			rbtreeNodeBase* root;
			
			// Just here to place holding the right pointer.
			rbtreeNodeBase* _2;
		} sentinel;
		
		/// Only valid when flags & rbfTypeMask == rbfSingle.
		struct {
			/// The pointer to the parent of this node.
			rbtreeNodeBase* parent;
			
			/// The left children of this node.
			rbtreeNodeBase* left;
			
			/// The right children of this node.
			rbtreeNodeBase* right;
		} single;
		
		/// Only valid when flags & rbfTypeMask == rbfMulext.
		struct {
			/// The pointer to parent of this node.
			rbtreeNodeBase* parent;
			
			/// The front of the queue.
			rbtreeNodeBase* back;
			
			/// The back of the queue.
			rbtreeNodeBase* front;
		} mulext;
		
		/// Only valid when flags & rbfTypeMask == rbfMulint.
		struct {
			/// The mulint node of this strip, only used when flags
			/// & rbfMulintMask != rbfMulintInternal.
			rbtreeNodeBase* mulext;
			
			/// The previous node in the queue, or the left children
			/// when the node is the queue head.
			rbtreeNodeBase* previous;
			
			/// The next node in the queue, or right children when the 
			/// node is the queue tail.
			rbtreeNodeBase* next;
		} mulint;
	} type;
	
	/// Retrieve the external node when it is an internal.
	/// @deprecated Slow function, just placed for correct implementation.
	static inline rbtreeNodeBase* deprecatedFetchMulext(rbtreeNodeBase* node) noexcept {
		// The case that the mulext could be fetched in place.
		if((node -> flags & rbfTypeMask) != rbfMulint) return node;
		if((node -> flags & rbfMulintMask) != rbfMulintInternal) return node -> type.mulint.mulext;
		
		// Traverse front and back simultenously to fetch the mulext node.
		rbtreeNodeBase* previous = node -> type.mulint.previous;
		rbtreeNodeBase* next = node -> type.mulint.next;
		while(true) {
			if((previous -> flags & rbfMulintMask) == rbfMulintFront) 
				return previous -> type.mulint.mulext;
			if((next -> flags & rbfMulintMask) == rbfMulintBack) 
				return next -> type.mulint.mulext;
			previous = previous -> type.mulint.previous;
			next = next -> type.mulint.next;
		}
		
		// Should not be here, or it indicates the data structure is corrupted.
		return nullptr;
	}
	
	/// The left children's address of this node, depending on the type of node.
	inline rbtreeNodeBase** left() noexcept {
		switch(flags & rbfTypeMask) {			
			case rbfSingle: {
				// The left children is just written inside the single node.
				return &(type.single.left);
			}
			
			case rbfMulext: {
				// The left children is the previous node of the queue head.
				return &(type.mulext.front -> type.mulint.previous);
			}
			
			case rbfMulint: {
				if(((flags & rbfMulintMask) & rbfMulintFront) == rbfMulintFront)
					// Current node is the multinode's previous node.
					return &(type.mulint.previous);
				else
					// Find the front node in the mutinode's link and return.
					return deprecatedFetchMulext(this) -> left();
			}
			
			case rbfOrphan: {
				// The root node of sentinel is always considered the left node.
				return &(type.sentinel.root);
			}
			
			default: {
				return nullptr;
			}
		}
	}
	
	/// The right children's address of this node, depending on the type of node.
	inline rbtreeNodeBase** right() noexcept {
		switch(flags & rbfTypeMask) {			
			case rbfSingle: {
				// The left children is just written inside the single node.
				return &(type.single.right);
			}
			
			case rbfMulext: {
				// The left children is the previous node of the queue head.
				return &(type.mulext.back -> type.mulint.next);
			}
			
			case rbfMulint: {
				if(((flags & rbfMulintMask) & rbfMulintBack) == rbfMulintBack)
					// Current node is the multinode's previous node.
					return &(type.mulint.next);
				else
					// Find the front node in the mutinode's link and return.
					return deprecatedFetchMulext(this) -> right();
			}
			
			default: {
				return nullptr;
			}
		}
	}
	
	/// The parent node's address of this node, depending on the type of node.
	inline rbtreeNodeBase** parent() noexcept {
		switch(flags & rbfTypeMask) {
			case rbfSingle: {
				// The parent node of this single node.
				return &(type.single.parent);
			}
			
			case rbfMulext: {
				// The parent node of this external node.
				return &(type.mulext.parent);
			}
			
			case rbfMulint: {
				// Find it in the external node.
				return deprecatedFetchMulext(this) -> parent();
			}
			
			default: {
				return nullptr;
			}
		}
	}
	
	/// The mutable field in the parent of this node, which is pointing to this node.
	inline rbtreeNodeBase** referred() noexcept {
		// Fetch the parent of this node.
		rbtreeNodeBase** parentNode = parent();
		
		// Fetch the pointing node of this node.
		rbtreeNodeBase** parentLeft = (*parentNode) -> left();
		if(*parentLeft == this) return parentLeft;
		else return (*parentNode) -> right();
	}
	
	/// Check whether current node is the root node, in place.
	inline bool isRoot() const noexcept {
		rbtreeFlag parentFlags;
		switch(flags & rbfTypeMask) {
			case rbfSingle: {
				parentFlags = type.single.parent -> flags;
			} break;
			
			case rbfMulext: {
				parentFlags = type.mulext.parent -> flags;
			} break;
			
			default:
				return false;
		}
		return (parentFlags & rbfTypeMask) == rbfOrphan;
	}
	
	/// Check whether the current node is red.
	static inline bool red(rbtreeNodeBase* node) noexcept {
		if(node == nullptr) return false;
		return (node -> flags & rbfColorMask) == rbfColorRed;
	}
	
	/// Check whether the current node is black.
	static inline bool black(rbtreeNodeBase* node) noexcept {
		if(node == nullptr) return true;
		return (node -> flags & rbfColorMask) == rbfColorBlack;
	}
	
	/// Flip the current node's color (red to black or black to red).
	inline void flipColor() noexcept {
		flags = static_cast<rbtreeFlag>(flags ^ rbfColorMask);
	}
	
	/// Exchange the color of two tree nodes. Neither node should be nil node (as you
	/// can't swap color with a nil node.
	inline void swapColor(rbtreeNodeBase& b) noexcept {
		if(((flags ^ b.flags) & rbfColorMask) != 0) {
			flipColor();
			b.flipColor();
		}
	}
public:
	/// Rebalance the tree as if the current node is the newly inserted red node, and 
	/// its parent is also a red node. Only useful when the node is not the mulint node.
	/// It is not possible that its grand parent does not exist with its parent being 
	/// red. However the uncle could be nil and being black. As the uncle will not be 
	/// recolored while it is black, this does not matter.
	/// It is not possible that the iteration ends at a nil node.
	void doubleRedResolve() noexcept;
	
	/// Rebalance the tree as if the current node is the double black node. The 
	/// rebalancing process will only terminate when there's some subtree that its black 
	/// height will not decrease or it is the subtree starting from root. 
	/// It is not possible that the iteration ends at a nil node.
	void doubleBlackResolve() noexcept;
};

} // namespace exotic.
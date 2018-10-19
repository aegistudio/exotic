#pragma once
/**
 * @file exotic/scope.hpp
 * @author Haoran Luo
 * @brief Container and pointer scope
 *
 * The scope describes the relation between container and nodes, in
 * this library, there're three situations of the scopes:
 * 1. Decoupled (Default scope): arbitrary node and be added to 
 * arbitrary container, a node can be destroyed when a container 
 * is still alive, and a container is destroyed while all of its 
 * containing node stays alive and could be added to another container.
 * 2. Cached: every node will live longer than any containers, and 
 * it could be added to another container if its previous container 
 * get destroyed.
 * 3. Symbiosis: every node will live as long as any containers, and 
 * they will be constructed and destructed just the same time as their
 * container.
 *
 * In this library, the solution for scope is: 
 * For decoupled, the container iteratively reset all nodes inside the 
 * container, and a node holds the reference to its container so that 
 * it could be removed from its container if destroyed. This approach 
 * is safe but it can takes up too much space, and have overhead of 
 * resetting all nodes from the container, but is the most flexible one.
 * For cached, container reset all nodes inside the container when destroyed, 
 * it still have overhead of removing node and is less flexible than the 
 * decoupled one, but each node does not have to hold any extra data so it 
 * is more space friendly.
 * For symbiosis, no destructor will be invoked and no space is required
 * to store the pointer to the container, so it is super fast when 
 * destructing. However, it is least flexible, and will get your program 
 * into crazy state if you don't obey the scope.
 */

namespace exotic {

// Forward declaration for friend class declaring.
template<typename nodeType> class virtualContainerHolder;

/// Provides the interface for removing nodes inside the container.
template<typename nodeType>
struct virtualContainer {
	/// Only the holder class could access the remove method.
	friend class virtualContainerHolder<nodeType>;
private:
	/// The interface for removing node from container.
	virtual void remove(nodeType& node) noexcept = 0;
};

/// The container holder for a node.
template<typename nodeType> 
struct virtualContainerHolder {
	/// Stores the content of the virtual container.
	virtualContainer<nodeType>* container;
	
	/// Constructor for the container holder.
	virtualContainerHolder() noexcept: container(nullptr) {}
	
	/// Perform removal while in the node type's destructor.
	void remove(nodeType& node) noexcept {
		if(container != nullptr) container -> remove(node);
	}
	
	/// Update container for the node.
	void update(virtualContainer<nodeType>* newContainer) noexcept {
		container = newContainer;
	}
};

// Forward declaration for friend class declaring.
template<typename nodeType> class nullContainerHolder;

/// The null container interface.
template<typename nodeType>
struct nullContainer {};

/// The container holder for those who needs no removal.
template<typename nodeType>
struct nullContainerHolder {
	/// Do nothing while remove.
	void remove(nodeType& node) noexcept {}
	
	/// Update container will also do nothing.
	void update(nullContainer<nodeType>* newContainer) noexcept {}
};


/// Definition of the scopes in the library.
namespace scope {
	/// The scope that containers and nodes have no lifecycle relation.
	template<typename nodeType>
	struct decoupled {
		// The nodes should hold reference to their container.
		typedef virtualContainerHolder<nodeType> nodeScopeType;
		typedef virtualContainer<nodeType> containerScopeType;
		
		/// Container should remove node when it is being destroyed.
		static constexpr bool containerRemove = true;
	};
	
	/// The scope that all nodes lives longer than their containers.
	template<typename nodeType>
	struct cached {
		// The nodes should hold reference to their container.
		typedef nullContainerHolder<nodeType> nodeScopeType;
		typedef nullContainer<nodeType> containerScopeType;
		
		/// Containers should still remove node when it is being destroyed.
		static constexpr bool containerRemove = true;
	};
	
	/// The scope that all nodes and their containers have the same lifecycle.
	template<typename nodeType>
	struct symbiosis {
		// The nodes should hold reference to their container.
		typedef nullContainerHolder<nodeType> nodeScopeType;
		typedef nullContainer<nodeType> containerScopeType;
		
		/// Containers should still remove node when it is being destroyed.
		static constexpr bool containerRemove = false;
	};
} // namespace exotic::scope.

} // namespace exotic.
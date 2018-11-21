#pragma once
/**
 * @file exotic/scope.hpp
 * @author Haoran Luo
 * @brief Node scope definition macro.
 *
 * This file defines the scopes for the nodes and containers. The scope 
 * describes the life cycle length relation of the nodes and containers.
 *
 * For some case that nodes live longer than any container, or the nodes
 * and containers are pre-defined, there's no need to invoke the destructor
 * of the nodes or container, so the program could be greatly accelerated,
 * especially for some complex data structures.
 */
namespace exotic {
namespace scope {
	
/**
 * The most common case is that the containers and nodes could be created
 * and destroyed at anytime. In this case, to make the program run correctly,
 * destructors of both containers and nodes should be invoked.
 */
struct decoupled {
	static constexpr bool destroyNode = true;
	static constexpr bool destroyContainer = true;
};


/**
 * In some case, the nodes and the containers are pre-allocated, and 
 * their relation changes when the program runs, and one becomes meaningless
 * when the other gets destroyed. So neither the destructor of node nor 
 * the container needs to be invoked.
 */
struct symbiosis {
	static constexpr bool destroyNode = false;
	static constexpr bool destroyContainer = false;
};

/**
 * Other cases are meaningless, because if either side get destroyed,
 * it will leave the other side an orphan node or an empty container.
 * So use exotic::scope::decoupled for other cases.
 */
 
} // namespace exotic::scope
} // namespace exotic
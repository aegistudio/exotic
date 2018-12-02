/**
 * @brief rbtree.cpp
 * @author Haoran Luo
 * @brief The red-black tree implementation.
 *
 * This file implements some methods inside the red-black tree that is nearly impossible 
 * to be inlined by the compiler. For interfaces that should be implemented by this 
 * class, see also the header file.
 * @see exotic/rbtree.hpp
 */
#include "exotic/rbtree.hpp"

namespace exotic {

/// Implementation for rbtreeNodeBase::doubleRedResolve().
void rbtreeNodeBase::doubleRedResolve(rbtreeNodeBase* node) noexcept {
	// Iteratively resolve red parent.
	while(red(node) && !node -> isRoot()) {
		// The parent node of current node.
		rbtreeNodeBase**  nodeParent  = node -> parent();
		rbtreeNodeBase*   parent      = *nodeParent;
		rbtreeNodeBase    **parentParent, **parentLeft, **parentRight;
		rbtreeNodeBase::fetchLinks(parent, parentParent, parentLeft, parentRight);
		
		// The grand parent node of current node.
		rbtreeNodeBase*   grand        = *parentParent;
		rbtreeNodeBase    **grandParent, **grandLeft, **grandRight;
		rbtreeNodeBase::fetchLinks(grand, grandParent, grandLeft, grandRight);
		
		// Judge the chirality of the subtree, and assign the uncle.
		// C   /G\    | C   /G\   (The chirality is true only if the parent node is
		// - /P\ /U\  | + /U\ /P\ on the right of grand parent).
		bool              chirality   = (parent == *grandRight);
		rbtreeNodeBase*   uncle       = chirality? *grandLeft : *grandRight;
			
		// Judge how to rotate by the uncle of current node.
		if(red(uncle)) {
			// Red uncle resolve.
			// C   /Gb\           /Gr\  | C  /Gb\           /Gr\ 
			// - /Pr\ Ur  --->  /Pb\ Ub | + Ur /Pr\ --->   Ub /Pb\
			//    Nr             Nr     |       Nr             Nr
			parent -> flipColor();    // Pr -> Pb.
			uncle  -> flipColor();    // Ur -> Ub.
			grand  -> flipColor();    // Gb -> Gr.
			node = grand;
		} else {
			// Fetch more information for black uncle resolving.
			// The proceeding algorithm requires these information.
			//      A      |        A
			// C   /G\     | C     /G\
			// -  Go  Gi   | +    Gi Go
			//   /P\  /U\  |     /U\ /P\
			//  Po Pi      |        Pi Po
			rbtreeNodeBase** parentInner = chirality? parentLeft : parentRight;
			rbtreeNodeBase** parentOuter = chirality? parentRight : parentLeft;
			rbtreeNodeBase** grandInner  = chirality? grandLeft : grandRight;
			rbtreeNodeBase** grandOuter  = chirality? grandRight : grandLeft;
			
			// Will change the subroot when it is black uncle.
			rbtreeNodeBase*  ancestor    = *grandParent;
			rbtreeNodeBase** subroot     = grand -> referred();
			
			// Check whether it is inner or outer node.
			if(node == *parentOuter) {
				// Black uncle resolve 1 (Terminate condition).
				//      A            A       |     A              A
				// C   /Gb\         /Pb\     | C  /Gb\           /Pb\
				// -  Go   Gi      Po   Pi   | + Gi   Go        Pi   Po
				//   /Pr\  Ub ---> Nr  /Gr\  |   Ub  /Pr\ ---> /Gr\  Nr
				//   Po Pi             Go Gi |       Pi Po     Gi Go
				//   Nr  c             c  Ub |       c  Nr     Ub c
				*subroot = parent; *parentParent = ancestor;  // P <-> A
				rbtreeNodeBase* c = *parentInner;             // may be null.
				*parentInner = grand; *grandParent = parent;  // P <-> G.
				if(*grandOuter = c) *(c -> parent()) = grand; // G <-> c (assign, test null).
				parent -> flipColor();                        // Pr -> Pb.
				grand -> flipColor();                         // Gb -> Gr.
				node = parent;
				break;
			} else {
				// Black uncle resolve 2 (Terminate condition).
				//      A              A       |     A                A
				//     /Gb\           /Nb\     |    /Gb\             /Nb\
				// C  Go   Gi       No    Ni   | C Gi   Go         Ni    No
				// - /Pr\  Ub ---> /Pr\  /Gr\  | + Ub  /Pr\  ---> /Gr\  /Pr\
				//  Po  Pi         Po Pi Go Gi |      Pi  Po      Gi Go Pi Po
				//  a  /Nr\        a  b  c  Ub |     /Nr\ a       Ub c  b  a
				//     No Ni                   |     Ni No
				//     b  c                    |     c  b
				rbtreeNodeBase    **nodeInner, **nodeOuter;
				rbtreeNodeBase::fetchLinks(node, nodeParent, nodeOuter, nodeInner, chirality);
				rbtreeNodeBase*   b = *nodeOuter;               // may be null.
				rbtreeNodeBase*   c = *nodeInner;               // may be null.
				*subroot = node; *nodeParent = ancestor;        // N <-> A.
				*nodeOuter = parent; *parentParent = node;      // N <-> P.
				*nodeInner = grand; *grandParent = node;        // N <-> G.
				if(*parentInner = b) *(b -> parent()) = parent; // P <-> b (assign, test null).
				if(*grandOuter = c) *(c -> parent()) = grand;   // G <-> c (assign, test null).
				node -> flipColor();                            // Nr -> Nb.
				grand -> flipColor();                           // Gb -> Gr.
				break;
			}
		}
	}
	
	// Increase the black height if the node is red root.
	if(red(node) && node -> isRoot()) node -> flipColor();
}

/// Implementation for rbtreeNodeBase::doubleBlackResolve().
void rbtreeNodeBase::doubleBlackResolve(rbtreeNodeBase* node) noexcept {
	// Iteratively resolve double black node.
	while(!node -> isRoot()) {
		// The parent node of current node.
		rbtreeNodeBase**  nodeParent  = node -> parent();
		rbtreeNodeBase*   parent      = *nodeParent;
		rbtreeNodeBase    **parentParent, **parentLeft, **parentRight;
		rbtreeNodeBase::fetchLinks(parent, parentParent, parentLeft, parentRight);
		
		// The ancestor node of parent.
		rbtreeNodeBase*   ancestor    = *parentParent;
		rbtreeNodeBase**  subroot     = parent -> referred();
		
		// Fetch the sibling node according to the node status.
		// The sibling will never be null otherwise the black height of current
		// subtree is 0, which is impossible.
		
		// Judge the chirality of the nodes and fetch the sibling.
		//      /P\     |     /P\
		// C  Po  Pi    | C  Pi  Po
		// - /Nb\ /S\   | + /S\  /Nb\ 
		//  No Ni Si So |  So Si Ni No
		bool              chirality   = (*parentRight == node);
		rbtreeNodeBase**  parentOuter = chirality? parentRight : parentLeft;
		rbtreeNodeBase**  parentInner = chirality? parentLeft : parentRight;
		rbtreeNodeBase*   sibling     = *parentInner;
		rbtreeNodeBase    **siblingParent, **siblingOuter, **siblingInner;
		rbtreeNodeBase::fetchLinks(sibling, siblingParent, siblingOuter, siblingInner, !chirality);
	
		// Check whether sibling is red, notice the operation could be performed
		// in-place (so no breaking loop is required).
		if(red(sibling)) {
			// Red sibling resolve. (note that the left child of sibling
			// will always be black, and will always have children).
			//       A               A     |        A              A
			//      /Pb\            /Sb\   |       /Pb\           /Sb\
			// C  Po    Pi         Si  So  | C   Pi    Po        So  Si
			// - /Nb\  /Sr\ --->  /Pr\ n+2 | +  /Sr\  /Nb\ ---> n+2 /Pr\
			//   No Ni Si So     Po  Pi    |   So Si  Ni No        Pi  Po
			//   n  n n+2 n+2   /Nb\ n+2   |  n+2 n+2 n  n        n+2 /Nb\
			//        (c)       n  n (c)   |      (c)             (c) n  n
			rbtreeNodeBase* c = *siblingInner;
			*parentInner = c; *(c -> parent()) = parent;     // Pb <-> c.
			*siblingInner = parent; *parentParent = sibling; // Pb <-> Sr.
			*subroot = sibling; *siblingParent = ancestor;   // Sr <-> A.
			parent -> flipColor();                           // Pb <-> Pr.
			sibling -> flipColor();                          // Sr <-> Sb.
			
			// Perform in-place substitution of sibling.
			sibling = c;
			rbtreeNodeBase::fetchLinks(c, siblingParent, siblingOuter, siblingInner, !chirality);
		}
			
		// Now sibling node must be red, but its children may not be.
		if(red(*siblingOuter)) {
			// Red outer node of sibling. (Terminate condition).
			//        A                   A          |         A                 A
			// C    / P \               / S \        | C     / P \             / S \
			// -  Po     Pi           Si      So     | +    Pi    Po         So     Si
			//   /Nb\   /Sb\   --->  /Pr\    /Srb\   |     /Sb\  /Nb\ ---> /Slb\   /Pr\
			//   No Ni  Si So       Po  Pi   Sri Sro |     So Si Ni No    Slo Sli  Pi Po
			//   n  n n+1 /Srr\    /Nb\ n+1  n+1 n+1 |  /Slr\ n+1 n  n    n+1 n+1 n+1 /Nb\
			//        (c) n+1 n+1  n  n (c)          | n+1 n+1 (c)                (c) n  n
			rbtreeNodeBase* c = *siblingInner;
			*parentInner = c; *(c -> parent()) = parent;     // P <-> c.
			*siblingInner = parent; *parentParent = sibling; // P <-> Sb.
			*subroot = sibling; *siblingParent = ancestor;   // S <-> A.
			(*sibling).swapColor(*parent);                   // Sb,P -> S,Pb.
			parent -> flipColor();                           // Pb -> Pr.
			(*siblingOuter) -> flipColor();                  // Sor -> Sob.
			break;
		}
		
		// Now the outer node of sibling is black, but its inner node may not be.
		if(red(*siblingInner)) {
			// Red inner node of sibling. (Terminate condition).
			// This is a double rotation, and result will be shown.
			// (c, d, e may not be null as n >= 1).
			//        A                    A         |         A                  A
			//      / P \                 /Sl\       |       / P \               /Sr\
			// C  Po     Pi            Ii      Io    | C   Pi      Po         Io      Ii
			// - /Nb\   /Sb\   --->   /Pb\    /Sb\   | +  /Sb\    /Nb\ --->  /Sb\    /Pb\
			//   No Ni  Si So        Po Pi    Si So  |   So  Si   Ni No     So  Si  Pi   Po
			//   n  n /Slr\ n+1    /Nb\ n+1  n+1 n+1 |  n+1 /Srr\ n  n      n+1 n+1 n+1 /Nb\ 
			//        Ii Io (e)    No Ni (c) (d) (e) |  (e) Io Ii           (e) (d) (c) Ni No
			//       n+1 n+1       n  n              |      n+1 n+1                     n  n
			//       (c) (d)                         |      (d) (c)
			rbtreeNodeBase*   inner       = *siblingInner;
			rbtreeNodeBase    **innerParent, **innerInner, **innerOuter;
			rbtreeNodeBase::fetchLinks(inner, innerParent, innerOuter, innerInner, !chirality);
			rbtreeNodeBase*   c           = *innerInner;
			rbtreeNodeBase*   d           = *innerOuter;
			
			*parentInner = c; *(c -> parent()) = parent;   // P <-> c.
			*siblingInner = d; *(d -> parent()) = sibling; // Sb <-> d.
			*innerInner = parent; *parentParent = inner;   // P <-> Sir.
			*innerOuter = sibling; *siblingParent = inner; // Sir <-> Sb.
			*subroot = inner; *innerParent = ancestor;     // A <-> Sir.
			(*inner).swapColor(*parent);                   // Sir,P -> Si,Pr.
			parent -> flipColor();                         // Pr -> Pb.
			break;
		}
		
		// Now both inner node and outer node is black. And sibling is black.
		if(red(parent)) {
			// Red parent while triple black. (Terminate condition).
			// C   / Pr \             / Pb \     | C    / Pr \            / Pb \
			// - /Nb\  b/Sb\b ---> /Nb\  b/Sr\b  | + b/Sb\b  /Nb\ ---> b/Sr\b  /Nb\
			//   n  n  n+1 n+1     n  n  n+1 n+1 |   n+1 n+1 n  n      n+1 n+1 n  n
			(*sibling).swapColor(*parent);
			break;
		} else {
			// Black parent while triple black.
			// C   / Pb \             / Pb \     | C    / Pb \            / Pb \
			// - /Nb\  b/Sb\b ---> /Nb\  b/Sr\b  | + b/Sb\b  /Nb\ ---> b/Sr\b  /Nb\
			//   n  n  n+1 n+1     n  n  n+1 n+1 |   n+1 n+1 n  n      n+1 n+1 n  n
			// Parent becomes the new double black node, as the subtree have decreased
			// its black height by 1.
			sibling -> flipColor();
			node = parent;
		}
	}
}

} // namespace exotic.
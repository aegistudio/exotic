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
		rbtreeNodeBase**  nodeParent  = node -> extparent();
		rbtreeNodeBase*   parent      = *nodeParent;
		rbtreeNodeBase    **parentParent, **parentLeft, **parentRight;
		rbtreeNodeBase::extfetchLinks(parent, parentParent, parentLeft, parentRight);
		
		// The grand parent node of current node.
		rbtreeNodeBase*   grand        = *parentParent;
		rbtreeNodeBase    **grandParent, **grandLeft, **grandRight;
		rbtreeNodeBase::extfetchLinks(grand, grandParent, grandLeft, grandRight);
		
		// Judge the chirality of the subtree, and assign the uncle.
		// C   /G\    | C   /G\   (The chirality is true only if the parent node is
		// - /P\ /U\  | + /U\ /P\ on the right of grand parent).
		bool              chirality   = (parent == *grandRight);
		rbtreeNodeBase*   uncle       = chirality? *grandLeft : *grandRight;
			
		// Judge how to rotate by the uncle of current node.
		if(red(uncle)) {
			// Red uncle resolve.
			// C   /Gb\           /Gr\  | C  /Gb\         /Gr\   |
			// - /Pr\ Ur  --->  /Pb\ Ub | + Ur /Pr\ ---> Ub /Pb\ |
			//    Nr             Nr     |       Nr           Nr  |
			parent -> flipColor();    // Pr -> Pb.
			uncle  -> flipColor();    // Ur -> Ub.
			grand  -> flipColor();    // Gb -> Gr.
			node = grand;
		} else {
			// Fetch more information for black uncle resolving.
			// The proceeding algorithm requires these information.
			//      A     |      A     |
			// C   /G\    | C   /G\    |
			// -  Go  Gi  | +  Gi Go   |
			//   /P\  /U\ |   /U\ /P\  |
			//  Po Pi     |      Pi Po |
			rbtreeNodeBase** parentInner = chirality? parentLeft : parentRight;
			rbtreeNodeBase** parentOuter = chirality? parentRight : parentLeft;
			rbtreeNodeBase** grandOuter  = chirality? grandRight : grandLeft;
			
			// Will change the subroot when it is black uncle.
			rbtreeNodeBase*  ancestor    = *grandParent;
			rbtreeNodeBase** subroot     = grand -> referred();
			
			// Check whether it is inner or outer node.
			if(node == *parentOuter) {
				// Black uncle resolve 1 (Terminate condition).
				//      A            A       |     A              A     |
				// C   /Gb\         /Pb\     | C  /Gb\           /Pb\   |
				// -  Go   Gi      Po   Pi   | + Gi   Go        Pi   Po |
				//   /Pr\  Ub ---> Nr  /Gr\  |   Ub  /Pr\ ---> /Gr\  Nr |
				//   Po Pi             Go Gi |       Pi Po     Gi Go    |
				//   Nr  c             c  Ub |       c  Nr     Ub c     |
				*subroot = parent; *parentParent = ancestor; // P <-> A
				rbtreeNodeBase* c = *parentInner;            // may be null.
				*parentInner = grand; *grandParent = parent; // P <-> G.
				if((*grandOuter = c) != nullptr)             // (assign, test null).
					*(c -> extparent()) = grand;             // G <-> c.
				parent -> flipColor();                       // Pr -> Pb.
				grand -> flipColor();                        // Gb -> Gr.
				node = parent;
				break;
			} else {
				// Black uncle resolve 2 (Terminate condition).
				//      A              A       |     A                A       |
				//     /Gb\           /Nb\     |    /Gb\             /Nb\     |
				// C  Go   Gi       No    Ni   | C Gi   Go         Ni    No   |
				// - /Pr\  Ub ---> /Pr\  /Gr\  | + Ub  /Pr\  ---> /Gr\  /Pr\  |
				//  Po  Pi         Po Pi Go Gi |      Pi  Po      Gi Go Pi Po |
				//  a  /Nr\        a  b  c  Ub |     /Nr\ a       Ub c  b  a  |
				//     No Ni                   |     Ni No                    |
				//     b  c                    |     c  b                     |
				rbtreeNodeBase    **nodeInner, **nodeOuter;
				rbtreeNodeBase::fetchLinks(node, nodeParent, nodeOuter, nodeInner, chirality);
				rbtreeNodeBase*   b = *nodeOuter;          // may be null.
				rbtreeNodeBase*   c = *nodeInner;          // may be null.
				*subroot = node; *nodeParent = ancestor;   // N <-> A.
				*nodeOuter = parent; *parentParent = node; // N <-> P.
				*nodeInner = grand; *grandParent = node;   // N <-> G.
				if((*parentInner = b) != nullptr)          // (assign, test null).
					*(b -> extparent()) = parent;          // P <-> b.
				if((*grandOuter = c) != nullptr)           // (assign, test null).
					*(c -> extparent()) = grand;           // G <-> c.
				node -> flipColor();                       // Nr -> Nb.
				grand -> flipColor();                      // Gb -> Gr.
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
		rbtreeNodeBase**  nodeParent  = node -> extparent();
		rbtreeNodeBase*   parent      = *nodeParent;
		rbtreeNodeBase    **parentParent, **parentLeft, **parentRight;
		rbtreeNodeBase::extfetchLinks(parent, parentParent, parentLeft, parentRight);
		
		// The ancestor node of parent.
		rbtreeNodeBase*   ancestor    = *parentParent;
		rbtreeNodeBase**  subroot     = parent -> referred();
		
		// Fetch the sibling node according to the node status.
		// The sibling will never be null otherwise the black height of current
		// subtree is 0, which is impossible.
		
		// Judge the chirality of the nodes and fetch the sibling.
		//      /P\     |     /P\      |
		// C  Po  Pi    | C  Pi  Po    |
		// - /Nb\ /S\   | + /S\  /Nb\  |
		//  No Ni Si So |  So Si Ni No |
		bool              chirality   = (*parentRight == node);
		rbtreeNodeBase**  parentInner = chirality? parentLeft : parentRight;
		rbtreeNodeBase*   sibling     = *parentInner;
		rbtreeNodeBase    **siblingParent, **siblingOuter, **siblingInner;
		rbtreeNodeBase::extfetchLinks(sibling, siblingParent, siblingOuter, siblingInner, !chirality);
	
		// Check whether sibling is red, notice the operation could be performed
		// in-place (so no breaking loop is required).
		if(red(sibling)) {
			// Red sibling resolve. (note that the left child of sibling
			// will always be black, and will always have children).
			//       A               A     |        A              A       |
			//      /Pb\            /Sb\   |       /Pb\           /Sb\     |
			// C  Po    Pi         Si  So  | C   Pi    Po        So  Si    |
			// - /Nb\  /Sr\ --->  /Pr\ n+2 | +  /Sr\  /Nb\ ---> n+2 /Pr\   |
			//   No Ni Si So     Po  Pi    |   So Si  Ni No        Pi  Po  |
			//   n  n n+2 n+2   /Nb\ n+2   |  n+2 n+2 n  n        n+2 /Nb\ |
			//        (c)       n  n (c)   |      (c)             (c) n  n |
			rbtreeNodeBase* c = *siblingInner;
			*parentInner = c; *(c -> extparent()) = parent;  // Pb <-> c.
			*siblingInner = parent; *parentParent = sibling; // Pb <-> Sr.
			*subroot = sibling; *siblingParent = ancestor;   // Sr <-> A.
			parent -> flipColor();                           // Pb <-> Pr.
			sibling -> flipColor();                          // Sr <-> Sb.
			
			// Perform in-place substitution of sibling.
			sibling = c;
			rbtreeNodeBase::extfetchLinks(c, siblingParent, siblingOuter, siblingInner, !chirality);
		}
			
		// Now sibling node must be red, but its children may not be.
		if(red(*siblingOuter)) {
			// Red outer node of sibling. (Terminate condition).
			//        A                   A          |         A                 A         |
			// C    / P \               / S \        | C     / P \             / S \       |
			// -  Po     Pi           Si      So     | +    Pi    Po         So     Si     |
			//   /Nb\   /Sb\   --->  /Pr\    /Srb\   |     /Sb\  /Nb\ ---> /Slb\   /Pr\    |
			//   No Ni  Si So       Po  Pi   Sri Sro |     So Si Ni No    Slo Sli  Pi Po   |
			//   n  n n+1 /Srr\    /Nb\ n+1  n+1 n+1 |  /Slr\ n+1 n  n    n+1 n+1 n+1 /Nb\ |
			//        (c) n+1 n+1  n  n (c)          | n+1 n+1 (c)                (c) n  n |
			rbtreeNodeBase* c = *siblingInner;
			*parentInner = c; *(c -> extparent()) = parent;  // P <-> c.
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
			//        A                    A         |         A                  A           |
			//      / P \                 /Sl\       |       / P \               /Sr\         |
			// C  Po     Pi            Ii      Io    | C   Pi      Po         Io      Ii      |
			// - /Nb\   /Sb\   --->   /Pb\    /Sb\   | +  /Sb\    /Nb\ --->  /Sb\    /Pb\     |
			//   No Ni  Si So        Po Pi    Si So  |   So  Si   Ni No     So  Si  Pi   Po   |
			//   n  n /Slr\ n+1    /Nb\ n+1  n+1 n+1 |  n+1 /Srr\ n  n      n+1 n+1 n+1 /Nb\  |
			//        Ii Io (e)    No Ni (c) (d) (e) |  (e) Io Ii           (e) (d) (c) Ni No |
			//       n+1 n+1       n  n              |      n+1 n+1                     n  n  |
			//       (c) (d)                         |      (d) (c)                           |
			rbtreeNodeBase*   inner       = *siblingInner;
			rbtreeNodeBase    **innerParent, **innerInner, **innerOuter;
			rbtreeNodeBase::extfetchLinks(inner, innerParent, innerOuter, innerInner, !chirality);
			rbtreeNodeBase*   c           = *innerInner;
			rbtreeNodeBase*   d           = *innerOuter;
			
			*parentInner = c; *(c -> extparent()) = parent;   // P <-> c.
			*siblingInner = d; *(d -> extparent()) = sibling; // Sb <-> d.
			*innerInner = parent; *parentParent = inner;      // P <-> Sir.
			*innerOuter = sibling; *siblingParent = inner;    // Sir <-> Sb.
			*subroot = inner; *innerParent = ancestor;        // A <-> Sir.
			(*inner).swapColor(*parent);                      // Sir,P -> Si,Pr.
			parent -> flipColor();                            // Pr -> Pb.
			break;
		}
		
		// Now both inner node and outer node is black. And sibling is black.
		if(red(parent)) {
			// Red parent while triple black. (Terminate condition).
			// C   / Pr \             / Pb \     | C    / Pr \            / Pb \    |
			// - /Nb\  b/Sb\b ---> /Nb\  b/Sr\b  | + b/Sb\b  /Nb\ ---> b/Sr\b  /Nb\ |
			//   n  n  n+1 n+1     n  n  n+1 n+1 |   n+1 n+1 n  n      n+1 n+1 n  n |
			(*sibling).swapColor(*parent);
			break;
		} else {
			// Black parent while triple black.
			// C   / Pb \             / Pb \     | C    / Pb \            / Pb \    |
			// - /Nb\  b/Sb\b ---> /Nb\  b/Sr\b  | + b/Sb\b  /Nb\ ---> b/Sr\b  /Nb\ |
			//   n  n  n+1 n+1     n  n  n+1 n+1 |   n+1 n+1 n  n      n+1 n+1 n  n |
			// Parent becomes the new double black node, as the subtree have decreased
			// its black height by 1.
			sibling -> flipColor();
			node = parent;
		}
	}
}

/// Implementation for rbtreeNodeBase::sentinelPrune().
void rbtreeNodeBase::sentinelPrune(rbtreeNodeBase* root) noexcept {
	rbtreeNodeBase* node = root -> type.sentinel.root;
	if(node == nullptr) return;
	root -> type.sentinel.root = nullptr;
	
	// The iteration will eventually back to the sentinel, so is the condition.
	rbtreeFlag nodeType;
	while((nodeType = static_cast<rbtreeFlag>((node -> flags & rbfTypeMask))) != rbfOrphan) {
		
		// Convert the mulext node into single node before going further.
		if(nodeType == rbfMulext) {
			// The previous and next node for 
			rbtreeNodeBase* left = node -> type.mulext.front -> type.mulint.previous;
			rbtreeNodeBase* right = node -> type.mulext.back -> type.mulint.next;
			
			// Separate the mulint nodes as a chain first.
			node -> type.mulext.front -> type.mulint.previous = nullptr;
			node -> type.mulext.back -> type.mulint.next = nullptr;
			node -> type.mulext.front -> type.mulint.mulext = nullptr;
			node -> type.mulext.back -> type.mulint.mulext = nullptr;
			
			// Iteratively reset the node status.
			for(rbtreeNodeBase* mulint = node -> type.mulext.front; mulint != nullptr;) {
				rbtreeNodeBase* mulintNext = mulint -> type.mulint.next;
				mulint -> type.mulint.previous = nullptr;
				mulint -> type.mulint.next = nullptr;
				mulint -> flags = rbfOrphan;
				mulint = mulintNext;
			}
			
			// Now we can safely set the node to single node.
			nodeType = rbfSingle;
			node -> flags = nodeType; // The color is not important anymore.
			node -> type.single.left = left;
			node -> type.single.right = right;
		}
		
		// Check whether there's some children of current node.
		// As the next time the node comes back to their parent, they should be removed 
		// from the parent, and each subtree should only be visited once, so simply 
		// remove that node from its parent's link.
		if(node -> type.single.left != nullptr) {
			rbtreeNodeBase* left = node -> type.single.left;
			node -> type.single.left = nullptr;
			node = left;
		}
		else if(node -> type.single.right != nullptr) {
			rbtreeNodeBase* right = node -> type.single.right;
			node -> type.single.right = nullptr;
			node = right;
		}
		else {
			node -> flags = rbfOrphan;
			rbtreeNodeBase* parent = node -> type.single.parent;
			node -> type.single.parent = nullptr;
			node = parent;
		}
	}
}

/// Implementation for rbtreeNodeBase::insert().
void rbtreeNodeBase::insert(rbtreeNodeBase* target, int relation) noexcept {
	if(relation == 0) {
		// Judge what to do with the node, can only be either single or mulext.
		if((target -> flags & rbfTypeMask) == rbfSingle) {
			// Make current node the mulint node.
			flags = static_cast<rbtreeFlag>(rbfMulint | rbfMulintFront | rbfMulintBack);
			type.mulint.previous = target -> type.single.left;
			type.mulint.next = target -> type.single.right;
			type.mulint.mulext = target;
			
			// Elevate the target node if it is currently not a mulext node.
			target -> flags = static_cast<rbtreeFlag>(
				((target -> flags | rbfTypeMask) ^ rbfTypeMask) | rbfMulext);
			target -> type.mulext.back = this;
			target -> type.mulext.front = this;
		} else {
			rbtreeNodeBase** front = &(type.mulext.front);
			
			// Make current node the mulint node and the queue front.
			flags = static_cast<rbtreeFlag>(rbfMulint | rbfMulintFront);
			type.mulint.mulext = target;
			
			// Make current front node an internal node.
			(*front) -> flags = static_cast<rbtreeFlag>(((*front) -> flags) ^ rbfMulintFront);
			(*front) -> type.mulint.mulext = nullptr;
			
			// Insert current node as the new front.
			type.mulint.previous = (*front) -> type.mulint.previous;
			type.mulint.next = (*front);
			(*front) -> type.mulint.previous = this;
			(*front) = this;
		}
	} else {
		// Insert the node into its parent's field first.
		if(relation < 0) *(target -> left()) = this;
		else *(target -> right()) = this;
		
		// Update the parent node of this node.
		type.single.parent = target;
		flags = static_cast<rbtreeFlag>(rbfSingle | rbfColorRed);
		if(red(target)) doubleRedResolve(this);
	}
}

/// Implementation for rbtreeNodeBase::extswap().
void rbtreeNodeBase::extswap(rbtreeNodeBase& b) noexcept {
	rbtreeNodeBase& a = *this;
	rbtreeNodeBase* nilParent;	// Just an eye candy for updating nil nodes.
	
	// Fetch the tree nodes from each nodes.
	rbtreeNodeBase **aParentField, **aLeftField, **aRightField;
	rbtreeNodeBase::extfetchLinks(&a, aParentField, aLeftField, aRightField);
	rbtreeNodeBase *aParent = *aParentField, *aLeft = *aLeftField, *aRight = *aRightField;
	
	rbtreeNodeBase **bParentField, **bLeftField, **bRightField;
	rbtreeNodeBase::extfetchLinks(&b, bParentField, bLeftField, bRightField);
	rbtreeNodeBase *bParent = *bParentField, *bLeft = *bLeftField, *bRight = *bRightField;
	
	// Fetch the external links, notice that there're maybe some nil nodes as children.
	rbtreeNodeBase **aParentRefer = a.referred();
	rbtreeNodeBase **bParentRefer = b.referred();
	rbtreeNodeBase **aLeftRefer = aLeft != nullptr? aLeft -> extparent() : &nilParent;
	rbtreeNodeBase **bLeftRefer = bLeft != nullptr? bLeft -> extparent() : &nilParent;
	rbtreeNodeBase **aRightRefer = aRight != nullptr? aRight -> extparent() : &nilParent;
	rbtreeNodeBase **bRightRefer = bRight != nullptr? bRight -> extparent() : &nilParent;
	
	// Perform swapping by updating the field links.
	*aParentField = bParent; *bParentField = aParent;
	*aLeftField = bLeft; *bLeftField = aLeft;
	*aRightField = bRight; *bRightField = aRight;
	*aParentRefer = *aLeftRefer = *aRightRefer = &b; 
	*bParentRefer = *bLeftRefer = *bRightRefer = &a;
}

/// Implementation for rbtreeNodeBase::nullswap().
void rbtreeNodeBase::nullswap(rbtreeNodeBase& null) noexcept {
	// Copy the node internal status first.
	switch((flags & rbfTypeMask)) {
		// The case that the node is a stand alone node.
		case rbfSingle: {
			// Copy the flags and pointers into the null object.
			null.flags = flags;
			null.type.single.parent = type.single.parent;
			null.type.single.left = type.single.left;
			null.type.single.right = type.single.right;
			
			// Update the external pointers.
			if(null.type.single.left != nullptr)
				*(null.type.single.left -> extparent()) = &null;
			if(null.type.single.right != nullptr)
				*(null.type.single.right -> extparent()) = &null;
			*(referred()) = &null;
		} break;
		
		// The case that the node is the mulint node.
		case rbfMulint: {
			// Copy the flags and pointers into the null object.
			null.flags = flags;
			null.type.mulint.mulext = type.mulint.mulext;
			null.type.mulint.previous = type.mulint.previous;
			null.type.mulint.next = type.mulint.next;
			
			// Update the previous node or mulext node.
			if((flags & rbfMulintFront) != 0) {
				type.mulint.mulext -> type.mulext.front = &null;
			} else {
				type.mulint.previous -> type.mulint.next = &null;
			}
			
			// Update the next node of mulext node.
			if((flags & rbfMulintBack) != 0) {
				type.mulint.mulext -> type.mulext.back = &null;
			} else {
				type.mulint.next -> type.mulint.previous = &null;
			}
		} break;
		
		// The case that the node is the mulext node.
		case rbfMulext: {
			// Copy the flags and pointers into the null object.
			null.flags = flags;
			null.type.mulext.parent = type.mulext.parent;
			null.type.mulext.front = type.mulext.front;
			null.type.mulext.back = type.mulext.back;
			
			// Update the references to front and back node.
			null.type.mulext.front -> type.mulint.mulext = &null;
			if(null.type.mulext.front -> type.mulint.previous != nullptr)
				*(null.type.mulext.front -> type.mulint.previous -> extparent()) = &null;
			null.type.mulext.back -> type.mulint.mulext = &null;
			if(null.type.mulext.back -> type.mulint.next != nullptr)
				*(null.type.mulext.back -> type.mulint.mulext -> extparent()) = &null;
			*(referred()) = &null;
		} break;
		
		// There's nothing needs to be done, because the node is an orphan.
		// Sentinel node should be swapped by the container manually.
		default: 
			return;	// Return directly.
	}
	
	// Make the current node a null node.
	flags = rbfOrphan;
	type.sentinel._1 = nullptr;
	type.sentinel.root = nullptr;
	type.sentinel._2 = nullptr;
}

} // namespace exotic.

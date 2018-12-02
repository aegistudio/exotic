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

// Aliasing some types so that the code could be more readable.
typedef rbtreeNodeBase* rbtreeParent;
typedef rbtreeNodeBase* rbtreeLeftChild;
typedef rbtreeNodeBase* rbtreeRightChild;

/// Implementation for rbtreeNodeBase::doubleRedResolve().
void rbtreeNodeBase::doubleRedResolve() noexcept {
	rbtreeNodeBase* node = this;
	
	// Iteratively resolve red parent.
	while(red(node) && !node -> isRoot()) {
		// The parent node of current node.
		rbtreeParent*     nodeParent  = node -> parent();
		rbtreeParent      parent      = *nodeParent;
		rbtreeLeftChild*  parentLeft  = parent -> left();
		rbtreeRightChild* parentRight = parent -> right();
		
		// The grand parent node of current node.
		rbtreeParent*     parentParent = parent -> parent();
		rbtreeParent      grand        = *parentParent;
		rbtreeLeftChild*  grandLeft    = grand -> left();
		rbtreeRightChild* grandRight   = grand -> right();
		
		// Judge what shape is the subtree.
		if(parent == *grandLeft) {
			// It is the case that parent and node are inside left subtree of grand.
			
			// Judge how to rotate by the uncle of current node.
			rbtreeRightChild uncle = *grandRight;
			if(red(uncle)) {
				// Red uncle resolve.
				//     /Gb\           /Gb\             /Gr\
				//   /Pr  Ur   or   Pr\   Ur  --->   Pb    Ub
				//  Nr                 Nr            Nr (either left or right).
				parent -> flipColor();
				uncle  -> flipColor();
				grand  -> flipColor();
				node = grand;
			} else {
				// Will change the subroot when it is black uncle.
				rbtreeParent*    grandParent = grand -> parent();
				rbtreeParent     ancestor    = *grandParent;
				rbtreeNodeBase** subroot     = grand -> referred();
				
				// Check whether it is inner or outer node.
				if(node == *parentLeft) {
					// Black uncle resolve 1 (Terminate condition).
					//      A                 A
					//     /Gb\              /Pb\
					//   /Pr\  Ub   --->   Nr   /Gr\
					//  Nr   c                 c    Ub
					*subroot = parent; *parentParent = ancestor; // P <-> A
					rbtreeNodeBase* c = *parentRight;            // may be null.
					*parentRight = grand; *grandParent = parent; // P <-> G.
					if(*grandLeft = c) *(c -> parent()) = grand; // G <-> c (assign, test null).
					parent -> flipColor();                       // Pr -> Pb.
					grand -> flipColor();                        // Gb -> Gr.
					node = parent;
					break;
				} else {
					// Black uncle resolve 2 (Terminate condition).
					//      A                  A
					//     /Gb\               /Nb\
					//   /Pr\  Ub   --->   /Pr\  /Gr\
					//  a  /Nr\            a  b  c   Ub
					//     b  c
					rbtreeLeftChild*  nodeLeft  = node -> left();
					rbtreeRightChild* nodeRight = node -> right();
					rbtreeLeftChild   b         = *nodeLeft;        // may be null.
					rbtreeRightChild  c         = *nodeRight;       // may be null.
					*subroot = node; *nodeParent = ancestor;        // N <-> A.
					*nodeLeft = parent; *parentParent = node;       // N <-> P.
					*nodeRight = grand; *grandParent = node;        // N <-> G.
					if(*parentRight = b) *(b -> parent()) = parent; // P <-> b (assign, test null).
					if(*grandLeft = c) *(c -> parent()) = grand;    // G <-> c (assign, test null).
					node -> flipColor();                            // Nr -> Nb.
					grand -> flipColor();                           // Gb -> Gr.
					break;
				}
			}
		} else {
			// It is the case that parent and node are inside right subtree of grand.
			
			// Judge how to rotate by the uncle of current node.
			rbtreeLeftChild uncle = *grandLeft;
			if(red(uncle)) {
				// Red uncle resolve.
				//    /Gb\            /Gb\           /Gr\
				//  Ur   Pr\    or  Ur   /Pr  --->  Ub   Pb
				//         Nr            Nr              Nr (either on left or right).
				parent -> flipColor();
				uncle -> flipColor();
				grand -> flipColor();
				node = grand;
			} else {
				// Will change the subroot when it is black uncle.
				rbtreeParent*    grandParent = grand -> parent();
				rbtreeParent     ancestor    = *grandParent;
				rbtreeNodeBase** subroot     = grand -> referred();
				
				// Check whether it is inner or outer node.
				if(node == *parentRight) {
					// Black uncle resolve 1 (Terminate condition).
					//      A                 A
					//     /Gb\              /Pb\
					//   Ub  /Pr\  --->    /Gr\  Nr
					//      c   Nr        Ub   c
					*subroot = parent; *parentParent = ancestor;  // P <-> A
					rbtreeNodeBase* c = *parentLeft;              // may be null.
					*parentLeft = grand; *grandParent = parent;   // P <-> G.
					if(*grandRight = c) *(c -> parent()) = grand; // G <-> c (assign, test null).
					parent -> flipColor();                        // Pr -> Pb.
					grand -> flipColor();                         // Gb -> Gr.
					node = parent;
					break;
				} else {
					// Black uncle resolve 2 (Terminate condition).
					//      A                      A
					//     /Gb\                   /Nb\
					//   Ub  /Pr\  Ub   --->   /Gr\  /Pr\
					//     /Nr\ e              Ub c  d  e
					//     c  d
					rbtreeLeftChild*  nodeLeft  = node -> left();
					rbtreeRightChild* nodeRight = node -> right();
					rbtreeLeftChild   c         = *nodeLeft;       // may be null.
					rbtreeRightChild  d         = *nodeRight;      // may be null.
					*subroot = node; *nodeParent = ancestor;       // N <-> A.
					*nodeRight = parent; *parentParent = node;     // N <-> P.
					*nodeLeft = grand; *grandParent = node;        // N <-> G.
					if(*grandRight = c) *(c -> parent()) = grand;  // G <-> c (assign, test null).
					if(*parentLeft = d) *(d -> parent()) = parent; // G <-> d (assign, test null).
					node -> flipColor();                           // Nr -> Nb.
					grand -> flipColor();                          // Gb -> Gr.
					break;
				}
			}
		}
	}
	
	// Increase the black height if the node is red root.
	if(red(node) && node -> isRoot()) node -> flipColor();
}	

/// Implementation for rbtreeNodeBase::doubleBlackResolve().
void rbtreeNodeBase::doubleBlackResolve() noexcept {
	rbtreeNodeBase* node = this;
	
	// Iteratively resolve double black node.
	while(!node -> isRoot()) {
		// The parent node of current node.
		rbtreeParent*     nodeParent  = node -> parent();
		rbtreeParent      parent      = *nodeParent;
		rbtreeLeftChild*  parentLeft  = parent -> left();
		rbtreeRightChild* parentRight = parent -> right();
		
		// The ancestor node of parent.
		rbtreeParent*     parentParent = parent -> parent();
		rbtreeParent      ancestor     = *parentParent;
		rbtreeNodeBase**  subroot      = parent -> referred();
		
		// Fetch the sibling node according to the node status.
		// The sibling will never be null otherwise the black height of current
		// subtree is 0, which is impossible.
		if(*parentLeft == node) {
			rbtreeNodeBase*   sibling       = *parentRight;
			rbtreeParent*     siblingParent = sibling -> parent();
			rbtreeLeftChild*  siblingLeft   = sibling -> left();
			rbtreeRightChild* siblingRight  = sibling -> right();
			
			// Check whether sibling is red, notice the operation could be performed
			// in-place (so no breaking loop is required).
			if(red(sibling)) {
				// Red sibling resolve. (note that the left child of sibling
				// will always be black, and will always have children).
				//      A                   A
				//     /Pb\                /Sb\
				//  /Nb\  /Sr\    --->   /Pr\ n+2
				//  n  n n+2 n+2       /Nb\ n+2
				//       (c)           n  n (c)
				rbtreeNodeBase* c = *siblingLeft;
				*parentRight = c; *(c -> parent()) = parent;     // Pb <-> c.
				*siblingLeft = parent; *parentParent = sibling;  // Pb <-> Sr.
				*subroot = sibling; *siblingParent = ancestor;   // Sr <-> A.
				parent -> flipColor();                           // Pb <-> Pr.
				sibling -> flipColor();                          // Sr <-> Sb.
				
				// Perform in-place substitution of sibling.
				sibling       = c;
				siblingLeft   = c -> left();
				siblingRight  = c -> right();
			}
			
			// Now sibling node must be red, but its children may not be.
			if(red(*siblingRight)) {
				// Red outer node of sibling. (Terminate condition).
				//       A                       A
				//     / P \                   / S \
				//  /Nb\   /Sb\     --->   /Pr\    /Srb\
				//  n  n n+1 /Srr\       /Nb\ n+1  n+1 n+1
				//       (c) n+1 n+1     n  n (c)
				rbtreeNodeBase* c = *siblingLeft;
				*parentRight = c; *(c -> parent()) = parent;    // P <-> c.
				*siblingLeft = parent; *parentParent = sibling; // P <-> Sb.
				*subroot = sibling; *siblingParent = ancestor;  // S <-> A.
				(*sibling).swapColor(*parent);                  // Sb,P -> S,Pb.
				parent -> flipColor();                          // Pb -> Pr.
				(*siblingRight) -> flipColor();                 // Srr -> Srb.
				break;
			}
			
			// Now the outer node of sibling is black, but its inner node may not be.
			if(red(*siblingLeft)) {
				// Red inner node of sibling. (Terminate condition).
				// This is a double rotation, and result will be shown.
				//       A                      A
				//     / P \                   /Sl\
				//  /Nb\   /Sb\     --->   /Pb\    /Sb\
				//  n  n /Slr\ n+1      /Nb\ n+1  n+1 n+1
				//      n+1 n+1(e)      n  n (c)  (d) (e)
				//      (c) (d)         (c, d, e may not be null as n >= 1).
				rbtreeNodeBase*   inner       = *siblingLeft;
				rbtreeParent*     innerParent = inner -> parent();
				rbtreeLeftChild*  innerLeft   = inner -> left();
				rbtreeRightChild* innerRight  = inner -> right();
				rbtreeLeftChild   c           = *innerLeft;
				rbtreeRightChild  d           = *innerRight;
				
				*parentRight = c; *(c -> parent()) = parent;   // P <-> c.
				*siblingLeft = d; *(d -> parent()) = sibling;  // Sb <-> d.
				*innerLeft = parent; *parentParent = inner;    // P <-> Slr.
				*innerRight = sibling; *siblingParent = inner; // Slr <-> Sb.
				*subroot = inner; *innerParent = ancestor;     // A <-> Slr.
				(*inner).swapColor(*parent);                   // Slr,P -> Sl,Pr.
				parent -> flipColor();                         // Pr -> Pb.
				break;
			}
			
			// Now both inner node and outer node is black. And sibling is black.
			if(red(parent)) {
				// Red parent while triple black. (Terminate condition).
				//    / Pr \              / Pb \
				//  /Nb\  b/Sb\b  -->  /Nb\  b/Sr\b
				//  n  n  n+1 n+1      n  n  n+1 n+1
				(*sibling).swapColor(*parent);
				break;
			} else {
				// Black parent while triple black.
				//    / Pb \              / Pb \
				//  /Nb\  b/Sb\b  -->  /Nb\  b/Sr\b
				//  n  n  n+1 n+1      n  n  n+1 n+1
				// Parent becomes the new double black node, as the subtree have decreased
				// its black height by 1.
				sibling -> flipColor();
				node = parent;
			}
		} else {
			rbtreeNodeBase*   sibling       = *parentLeft;
			rbtreeParent*     siblingParent = sibling -> parent();
			rbtreeLeftChild*  siblingLeft   = sibling -> left();
			rbtreeRightChild* siblingRight  = sibling -> right();
			
			// Check whether sibling is red, notice the operation could be performed
			// in-place (so no breaking loop is required).
			if(red(sibling)) {
				// Red sibling resolve. (note that the right child of sibling
				// will always be black, and will always have children).
				//       A                   A
				//      /Pb\                /Sb\
				//   /Sr\  /Nb\    --->   n+2 /Pr\
				// n+2 n+2 n  n              n+2 /Nb\
				//     (c)                   (c) n  n
				rbtreeNodeBase* c = *siblingRight;
				*parentLeft = c; *(c -> parent()) = parent;       // Pb <-> c.
				*siblingRight = parent; *parentParent = sibling;  // Pb <-> Sr.
				*subroot = sibling; *siblingParent = ancestor;    // Sr <-> A.
				parent -> flipColor();                            // Pb <-> Pr.
				sibling -> flipColor();                           // Sr <-> Sb.
				
				// Perform in-place substitution of sibling.
				sibling      = c;
				siblingLeft  = c -> left();
				siblingRight = c -> right();
			}
			
			// Now sibling node must be red, but its children may not be.
			if(red(*siblingLeft)) {
				// Red outer node of sibling. (Terminate condition).
				//        A                      A
				//      / P \                  / S \
				//    /Sb\   /Nb\   --->   /Slb\    /Pr\
				// /Slr\ n+1 n  n          n+1 n+1 n+1 /Nb\
				// n+1 n+1 (c)                     (c) n  n
				rbtreeNodeBase* c = *siblingRight;
				*parentRight = c; *(c -> parent()) = parent;     // P <-> c.
				*siblingRight = parent; *parentParent = sibling; // P <-> Sb.
				*subroot = sibling; *siblingParent = ancestor;   // S <-> A.
				(*sibling).swapColor(*parent);                   // Sb,P -> S,Pb.
				parent -> flipColor();                           // Pb -> Pr.
				(*siblingLeft) -> flipColor();                   // Slr -> Slb.
				break;
			}
			
			// Now the outer node of sibling is black, but its inner node may not be.
			if(red(*siblingRight)) {
				// Red inner node of sibling. (Terminate condition).
				// This is a double rotation, and result will be shown.
				//        A                       A
				//      / P \                    /Sr\
				//   /Sb\    /Nb\     --->   /Sb\    /Pb\
				// n+1 /Srr\ n  n           n+1 n+1 n+1 /Nb\ 
				// (a) n+1 n+1              (a) (b) (c) n  n
				//     (b) (c)         (a, b, c may not be null as n >= 1).
				rbtreeNodeBase*   inner       = *siblingRight;
				rbtreeParent*     innerParent = inner -> parent();
				rbtreeLeftChild*  innerLeft   = inner -> left();
				rbtreeRightChild* innerRight  = inner -> right();
				rbtreeLeftChild   b           = *innerLeft;
				rbtreeRightChild  c           = *innerRight;
				
				*parentLeft = c; *(c -> parent()) = parent;    // P <-> c.
				*siblingRight = b; *(b -> parent()) = sibling; // Sb <-> d.
				*innerRight = parent; *parentParent = inner;   // P <-> Srr.
				*innerLeft = sibling; *siblingParent = inner;  // Srr <-> Sb.
				*subroot = inner; *innerParent = ancestor;     // A <-> Srr.
				(*inner).swapColor(*parent);                   // Srr,P -> Sr,Pr.
				parent -> flipColor();                         // Pr -> Pb.
				break;
			}
			
			// Now both inner node and outer node is black. And sibling is black.
			if(red(parent)) {
				// Red parent while triple black. (Terminate condition).
				//    / Pr \            / Pb \
				// b/Sb\b  /Nb\ ---> b/Sr\b  /Nb\
				// n+1 n+1 n  n      n+1 n+1 n  n
				(*sibling).swapColor(*parent);
				break;
			} else {
				// Black parent while triple black.
				//    / Pb \             / Pb \
				// b/Sb\b  /Nb\ --->  b/Sr\b  /Nb\
				// n+1 n+1 n  n       n+1 n+1 n  n
				// Parent becomes the new double black node, as the subtree have decreased
				// its black height by 1.
				sibling -> flipColor();
				node = parent;
			}
		}
	}
}

} // namespace exotic.
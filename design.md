# Design

This file illustrate what aspects does the library take into consideration, so the user
can customize their code respectively.

## Terminologies

**Containers** and **Nodes**: As a data structure library, there are basically containers and 
nodes. The nodes store the information for maintaining the library, and the container provides 
access point for manipulating the data structure.

**Objects** and **Accesses**: the nodes are built into the actual user objects as a field. And 
the container usually provides manipulation methods on objects rather than their field, as 
manipulation on objects are much more intuitive. The access tells which how to find the node
field in an object and which object contains the node.

**Contexts**: In our library, containers do not actually hold the nodes, otherwise you will 
prefer the STL instead. The node can be held by relational containers, like `std::list` or 
`std::map`. The node can also be held by sequential containers, like a simple array or a 
`std::vector`. How the actual elements are stored is called the context of object.

**Scope**: The scope describes the relation of lifecycle between a container and its internal 
nodes. In the library, we discuss about three scopes: The container might be decoupled from 
the lifecycle of its internal nodes, and the container might live longer than all of its 
internal nodes (including newly created nodes), and the container might live just as long 
as all of its node. The scope will affect the destructive behavior of both container and node.

(Pointer was once a consideration but has been removed because we've found no way of addressing 
pointers to memory addresses and pointers using array index properly, as the latter one requires 
a context to work with. We may waste up some space without support for pointers, but the code 
is still valid and easier to manage.)
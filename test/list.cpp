/**
 * @file test/list.cpp
 * @author Haoran Luo
 * @brief Test cases around exotic::list.
 *
 * This file defines test cases around exotic::list. It verifies aspects about
 * the correctness of basic container operations (push, pop, iterate, insert,
 * remove, etc.), the correctness of list node's life cycle, and whether it is
 * compatible with STL containers.
 */
#include "gtest/gtest.h"
#include "exotic/list.hpp"
#include <vector>

namespace exotic {

/// The testing structure for this test case.
struct testObject {
	// Some garbage data inside the victim.
	char garbage[11];
	
	// The list node to test, which is embedded in the victim.
	exotic::listNode<> listNode;
	defineNode(listNodeId, testObject, listNode);
	
	// Some garbage data inside the victim.
	char garbage2[23];
	
	// The default nothrow constructor.
	testObject() noexcept: listNode() {}
	
	// The nothrow move constructor.
	testObject(testObject&& obj) noexcept: listNode(std::move(obj.listNode)) {}	
};

typedef exotic::list<exotic::testObject::listNodeId> testList;

} // namespace exotic.

/// Defining the swap method of the objects.
namespace std {
void swap(exotic::testObject& a, exotic::testObject& b) {
	using std::swap;
	swap(a.listNode, b.listNode);
}
} // namespace std.

/// List.PushingPopping: test for the pushing and popping functions at the 
/// front and back.
TEST(List, PushingPopping) {
	// A small test for 4 element's pushing popping.
	exotic::testObject small[4];
	exotic::testList smallList;
	EXPECT_TRUE(smallList.pushFront(small[0]));  // {0}.
	EXPECT_TRUE(smallList.pushFront(small[1]));  // {1, 0}.
	EXPECT_FALSE(smallList.pushFront(small[0])); // Already in list.
	EXPECT_TRUE(smallList.pushBack(small[2]));   // {1, 0, 2}.
	EXPECT_TRUE(smallList.pushBack(small[3]));   // {1, 0, 2, 3}.
	EXPECT_EQ(&small[1], smallList.popFront());  // {0, 2, 3}.
	EXPECT_EQ(&small[0], smallList.popFront());  // {2, 3}.
	EXPECT_EQ(&small[2], smallList.popFront());  // {3}.
	EXPECT_EQ(&small[3], smallList.popFront());  // {}.
	EXPECT_EQ(nullptr, smallList.popFront());    // {}.
	EXPECT_EQ(nullptr, smallList.popBack());     // {}.
	// Verified the pushFront(), popFront(), pushBack(), popBack() functions.
}

/// List.IteratorVisting: test for the correctness of iterators.
TEST(List, IteratorVisiting) {
	// Testing for a non-empty list.
	constexpr size_t numObjects = 6385;
	exotic::testObject objects[numObjects];
	exotic::testList list;
	for(int i = 0; i < numObjects; ++ i) EXPECT_TRUE(list.pushBack(objects[i]));
	
	// Iterate from the back with forward iterator.
	{
		size_t i = 0;
		for(auto fi = list.begin(); fi != list.end(); ++ fi) {
			EXPECT_EQ(&objects[i], &(*fi));
			++ i;
		}
		EXPECT_EQ(numObjects, i);
	}
	
	// Iterate from the front with const forward iterator.
	{
		size_t i = 0;
		for(auto cfi = list.cbegin(); cfi != list.cend(); ++ cfi) {
			EXPECT_EQ(&objects[i], &(*cfi));
			++ i;
		}
		EXPECT_EQ(numObjects, i);
	}
	
	// Iterate from the back with reverse iterator.
	{
		size_t j = numObjects;
		for(auto bi = list.rbegin(); bi != list.rend(); ++ bi) {
			-- j;
			EXPECT_EQ(&objects[j], &(*bi));
		}
		EXPECT_EQ(0, j);
	}
	
	// Iterate from the end with const reverse iterator.
	{
		size_t j = numObjects;
		for(auto bi = list.crbegin(); bi != list.crend(); ++ bi) {
			-- j;
			EXPECT_EQ(&objects[j], &(*bi));
		}
		EXPECT_EQ(0, j);
	}
	
	// Iterate using the for(:) loop.
	{
		size_t i = 0;
		for(const auto& obj : list) {
			EXPECT_EQ(&objects[i], &obj);
			++ i;
		}
		EXPECT_EQ(numObjects, i);
	}
	
	// Testing for an empty list. No matter what kind of iteration, no node is expected.
	exotic::testList empty;
	EXPECT_EQ(empty.begin(), empty.end());
	EXPECT_EQ(empty.cbegin(), empty.cend());
	EXPECT_EQ(empty.rbegin(), empty.rend());
	EXPECT_EQ(empty.crbegin(), empty.crend());
	for(const auto& obj : empty) ASSERT_TRUE(false);
	
	// Verified that (c)begin(), (c)end(), (c)rbegin(), (c)rend() function.
}

/// List.ObjectLifecycle: test whether the creation, swapping and destroying of objects
/// will turn the exotic::list into undefined state.
TEST(List, ObjectLifecycle) {
	exotic::testList list;
	constexpr size_t numObjects = 4235;
	
	// The external list, which will not destroyed during the running of the list.
	exotic::testObject externalObjects[numObjects];
	
	// Some objects might get swapped inside the list.
	exotic::testObject swappingObject[numObjects];
	
	// Begining to insert some temporary nodes between the external nodes.
	{
		// The internal object, which will get destroyed after the right bracket.
		exotic::testObject internalObjects[numObjects];
		
		// Interleavingly insert the external and internal objects.
		for(size_t i = 0; i < numObjects; ++ i) {
			list.pushBack(externalObjects[i]);
			list.pushBack(internalObjects[i]);
		}
		
		// Verified that the order is now interleaved.
		auto it = list.begin();
		for(size_t i = 0; i < numObjects; ++ i) {
			EXPECT_EQ(&externalObjects[i], &(*it)); ++ it;
			EXPECT_EQ(&internalObjects[i], &(*it)); ++ it;
		}
		EXPECT_EQ(it, list.end());
	}
	
	// Verify that only the external nodes survives.
	{
		auto it = list.begin();
		for(size_t i = 0; i < numObjects; ++ i) {
			EXPECT_EQ(&externalObjects[i], &(*it)); ++ it;
		}
		EXPECT_EQ(it, list.end());
	}
	
	// Attempt to swap nodes with the swapping objects.
	for(size_t i = 0; i < numObjects; ++ i) 
		if((i & 1) != 0) std::swap(externalObjects[i], swappingObject[i]);
	
	// Verify that the swap operation does not break the list.
	{
		auto it = list.begin();
		for(size_t i = 0; i < numObjects; ++ i) {
			if((i & 1) != 0) EXPECT_EQ(&swappingObject[i], &(*it));
			else EXPECT_EQ(&externalObjects[i], &(*it));
			++ it;
		}
		EXPECT_EQ(it, list.end());
	}
	// Verified that exotic::listNode will not break the container when it get 
	// destructed or swapped.
}

/// List.VectorCompatibility: test for whether the containers will get violated after 
/// vector get recapacitized.
TEST(List, VectorCompatibility) {
	// Testing for objects stored inside a vector.
	constexpr size_t numVectorObjects = 5123;
	std::vector<exotic::testObject> objects;
	exotic::testList list;
	
	// Iteratively insert objects into the vector.
	for(size_t i = 0; i < numVectorObjects; ++ i) {
		objects.emplace_back();
		list.pushFront(objects[i]);
	}
	
	// Verify the final result of insertion.
	{
		size_t i = numVectorObjects;
		for(auto fi = list.begin(); fi != list.end(); ++ fi) {
			-- i;
			EXPECT_EQ(&(*fi), &objects[i]);
		}
		EXPECT_EQ(0, i);
	}
	// Verified that exotic::list is compatible with std::vector.
}

/// List.IteratorModifying: test for whether the containers will get violated after
/// modifying elements via iterators, and whether the modification is applied.
TEST(List, IteratorModifying) {
	// Interleaving elements.
	exotic::testList interleaved;
	constexpr size_t numInterleavedObjects = 1421;
	exotic::testObject interleavedFront[numInterleavedObjects];
	
	// Insert the front elements into the container first.
	for(size_t i = 0; i < numInterleavedObjects; ++ i) {
		EXPECT_TRUE(interleaved.pushBack(interleavedFront[i]));
	}
		
	// Insert whe elements with forward iterator.
	{
		exotic::testObject interleavedBack[numInterleavedObjects];
		exotic::testObject interleavedBorder;
		
		// Interleave the back elements into the container then.
		auto it = interleaved.begin();
		for(size_t i = 0; i < numInterleavedObjects; ++ i) {
			// The successfulness of insetion must be ensured.
			EXPECT_TRUE(interleaved.insert(it, interleavedBack[i]) != interleaved.end());
			++ it;
		}
		EXPECT_EQ(it, interleaved.end());
		EXPECT_TRUE(interleaved.insert(it, interleavedBorder) != interleaved.end());
		
		// Check whether the list has been interleaved.
		auto cit = interleaved.cbegin();
		for(size_t i = 0; i < numInterleavedObjects; ++ i) {
			EXPECT_EQ(&(*cit), &interleavedBack[i]); ++ cit;
			EXPECT_EQ(&(*cit), &interleavedFront[i]); ++ cit;
		}
		EXPECT_EQ(&(*cit), &interleavedBorder); ++ cit;
		EXPECT_EQ(cit, interleaved.cend());
		
		// Remove elements with interleaved forward iterator.
		auto rmit = interleaved.begin();
		for(size_t i = 0; i < numInterleavedObjects; ++ i) {
			rmit = interleaved.erase(rmit);
			ASSERT_TRUE(rmit != interleaved.end());
			++ rmit;
		}
		EXPECT_EQ(rmit, interleaved.from(interleavedBorder));
		
		// The remained elements should be the front ones.
		auto rmcit = interleaved.cbegin();
		for(size_t i = 0; i < numInterleavedObjects; ++ i) {
			EXPECT_EQ(&(*rmcit), &interleavedFront[i]); ++ rmcit;
		}
		EXPECT_EQ(&(*rmcit), &interleavedBorder); ++ rmcit;
		EXPECT_EQ(rmcit, interleaved.cend());
	}
	
	// Insert whe elements with backward iterator.
	{
		exotic::testObject interleavedBack[numInterleavedObjects];
		exotic::testObject interleavedBorder;
		
		// Interleave the back elements into the container then.
		auto it = interleaved.rbegin();
		for(size_t i = 0; i < numInterleavedObjects; ++ i) {
			// The successfulness of insetion must be ensured.
			EXPECT_TRUE(interleaved.insert(it, interleavedBack[i]) != interleaved.rend());
			++ it;
		}
		EXPECT_EQ(it, interleaved.rend());
		EXPECT_TRUE(interleaved.insert(it, interleavedBorder) != interleaved.rend());
		
		// Check whether the list has been interleaved.
		auto cit = interleaved.cbegin();
		EXPECT_EQ(&(*cit), &interleavedBorder); ++ cit;
		for(size_t i = 0; i < numInterleavedObjects; ++ i) {
			EXPECT_EQ(&(*cit), &interleavedFront[i]); ++ cit;
			EXPECT_EQ(&(*cit), &interleavedBack[numInterleavedObjects - 1 - i]); ++ cit;
		}
		EXPECT_EQ(cit, interleaved.cend());
		
		// Remove elements with interleaved backward iterator.
		auto rmit = interleaved.rbegin();
		for(size_t i = 0; i < numInterleavedObjects; ++ i) {
			rmit = interleaved.erase(rmit);
			ASSERT_TRUE(rmit != interleaved.rend());
			++ rmit;
		}
		EXPECT_EQ(rmit, interleaved.rfrom(interleavedBorder));
		
		// The remained elements should be the front ones.
		auto rmcit = interleaved.cbegin();
		EXPECT_EQ(&(*rmcit), &interleavedBorder); ++ rmcit;
		for(size_t i = 0; i < numInterleavedObjects; ++ i) {
			EXPECT_EQ(&(*rmcit), &interleavedFront[i]); ++ rmcit;
		}
		EXPECT_EQ(rmcit, interleaved.cend());
	}
	
	// Verfied that exotic::list has correctly implemented insert().
}
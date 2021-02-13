#pragma once
#ifndef _RTC_RAWKDTREE_H_
#define _RTC_RAWKDTREE_H_

#include <rtu/common.h>
#include <rtc/AABB.h>
#include <rtc/SplitPlane.h>
#include <rtu/refcounting.h>
#include <vector>

namespace rtc {

struct RawKdNode : public rtu::RefCounted
{
	typedef std::vector<unsigned int> Elements;

	RawKdNode();
	// Leaf node
	RawKdNode( const Elements& ids );
	// Internal node
	RawKdNode( const SplitPlane& plane, RawKdNode* leftChild, RawKdNode* rightChild );

	inline bool isLeaf();

	SplitPlane split;
	rtu::ref_ptr<RawKdNode> left;
	rtu::ref_ptr<RawKdNode> right;
	Elements elements;
};

inline bool RawKdNode::isLeaf()
{
	return ( left == NULL ) && ( right == NULL );
}

//////////////////////////////////////////////////////////////////////////

struct RawKdTree : public rtu::RefCounted
{
	struct Statistics
	{
		void reset();

		unsigned int nodeCount;
		unsigned int leafCount;
		unsigned int treeDepth;
		unsigned int elemIdCount;
		// TODO: other useful stats
	};

	rtu::ref_ptr<RawKdNode> root;
	AABB bbox;
	Statistics stats;
};

} // namespace rtc

#endif // _RTC_RAWKDTREE_H_

#pragma once
#ifndef _RTC_KDTREE_H_
#define _RTC_KDTREE_H_

#include <rtu/common.h>
#include <rtc/SplitPlane.h>
#include <rtc/AABB.h>

namespace rtc {

struct KdNode
{
	void setInternalNode( const SplitPlane& plane, const KdNode* leftChild );
	void setLeafNode( unsigned int elementStart, unsigned int elementCount );

	inline unsigned int isLeaf() const;
	inline unsigned int axis() const;
	inline float splitPos() const;
	inline const KdNode* leftChild() const;
	inline unsigned	int elemStart() const;
	inline unsigned	int elemCount() const;

private:
	//--- If internal node ---
	// bits 0..1 : split axis
	// bits 2..30 : pointer to left child
	// bit 31 (sign) : flag whether node is a leaf
	//--- If leaf node ---
	// bits 0..30 : number of elements stored in leaf
	// bit 31 (sign) : flag whether node is a leaf
	unsigned int _data;

	//--- If internal node ---
	// split position along specified axis
	//--- If leaf node ---
	// offset to start of elements
	union
	{
		float _split;
		unsigned int _elements;
	};
};

inline unsigned int KdNode::isLeaf() const
{
	return ( _data & 0x80000000 );
}

inline unsigned int KdNode::axis() const
{
	return ( _data & 0x3 );
}

inline float KdNode::splitPos() const
{
	return _split;
}

inline const KdNode* KdNode::leftChild() const
{
#pragma warning( disable : 4312 )
	return reinterpret_cast<const KdNode*>( _data & 0x7FFFFFFC );
#pragma warning( default : 4312 )
}

inline unsigned	int KdNode::elemStart() const
{
	return _elements;
}

inline unsigned	int KdNode::elemCount() const
{
	return ( _data & 0x7FFFFFFF );
}

//////////////////////////////////////////////////////////////////////////

struct KdTree
{
	KdTree();
	// Cannot make destructor to avoid accidental deletion of data.
	// Example: resize in vector<geometry> will _shallow_ copy two geometries, and their kdtree.
	// One of them will be destroyed, thus deleting the other's data.
	// Should make a deep copy operator instead

	void erase();

	AABB bbox;
	KdNode* root;
	unsigned int* elements;
};

} // namespace rtc

#endif // _RTC_KDTREE_H_

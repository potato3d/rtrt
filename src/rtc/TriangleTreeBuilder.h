#pragma once
#ifndef _RTC_TRIANGLETREEBUILDER_H_
#define _RTC_TRIANGLETREEBUILDER_H_

#include <rtu/common.h>
#include <rtc/RawKdTree.h>
#include <rtc/Geometry.h>

namespace rtc {

class TriangleTreeBuilder
{
public:
	TriangleTreeBuilder();

	RawKdTree* buildTree( Geometry* geometry );

private:
	enum Side
	{
		LEFT,
		RIGHT,
		BOTH,
		INVALID // triangle outside box
	};

	struct SahResult
	{
		float cost;
		Side side;
	};

	struct Event
	{
		enum Type
		{
			END,
			PLANAR,
			START
		};

		unsigned int triangleId;
		float position;
		Type type;
	};

	struct EventOrder
	{
		inline bool operator()( const Event& first, const Event& second ) const;
	};

	RawKdNode* leafNode( const RawKdNode::Elements& triangles, unsigned int treeDepth );
	void getTriangleVertices( rtu::float3& v0, rtu::float3& v1, rtu::float3& v2, unsigned int triangleId ) const;
	bool terminate( const SahResult& bestResult, unsigned int triangleCount ) const;
	void sah( SahResult& result, const SplitPlane& plane, const AABB& bbox, unsigned int nL, unsigned int nP, unsigned int nR );

	// O( n log^2 n ) implementation
	RawKdNode* recursiveBuild( const RawKdNode::Elements& triangles, const AABB& bbox, unsigned int treeDepth );
	void findPlane( SplitPlane& plane, SahResult& sahResult, unsigned int& triangleCount, 
		            const RawKdNode::Elements& triangles, const AABB& bbox );
	void partition( RawKdNode::Elements& left, RawKdNode::Elements& right, const SahResult& sahResult, 
		            const SplitPlane& plane, const RawKdNode::Elements& triangles );

	Geometry* _geometry;
	RawKdTree::Statistics* _stats;

	std::vector< std::vector<Event> > _events;
	std::vector<Side> _triangleSides;
	std::vector<AABB> _triangleBoxes;

	float _traversalCost;
	float _intersectionCost;
};

inline bool TriangleTreeBuilder::EventOrder::operator()( const TriangleTreeBuilder::Event& first, 
	                                                     const TriangleTreeBuilder::Event& second ) const
{
	return ( first.position < second.position ) || ( ( first.position == second.position ) && ( first.type < second.type ) );
}

} // namespace rtc

#endif // _RTC_TRIANGLETREEBUILDER_H_

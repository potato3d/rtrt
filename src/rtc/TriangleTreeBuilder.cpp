#include <rtc/TriangleTreeBuilder.h>
#include <rtu/stl.h>
#include <algorithm>

namespace rtc {

TriangleTreeBuilder::TriangleTreeBuilder()
: _events( 3 ), _traversalCost( 1.0f ), _intersectionCost( 1.4f )
{
	// empty
}

RawKdTree* TriangleTreeBuilder::buildTree( Geometry* geometry )
{
	// Store geometry reference
	_geometry = geometry;
	
	unsigned int triangleCount = _geometry->triDesc.size();

	// Reset triangle data
	rtu::vectorExactResize( _triangleSides, triangleCount );
	rtu::vectorExactResize( _triangleBoxes, triangleCount );

	// Pre-allocate event data
	_events[0].reserve( triangleCount*2 );
	_events[1].reserve( triangleCount*2 );
	_events[2].reserve( triangleCount*2 );

	// Create triangle id vector for entire scene
	RawKdNode::Elements initialTriangles( triangleCount );

	for( unsigned int i = 0; i < triangleCount; ++i )
	{
		initialTriangles[i] = i;
	}
	
	// Create new raw kd tree
	RawKdTree* tree = new RawKdTree();
	tree->bbox.buildFrom( &_geometry->vertices[0], _geometry->vertices.size() );
	_stats = &tree->stats;
	_stats->reset();

	// Recursive tree build
	tree->root = recursiveBuild( initialTriangles, tree->bbox, 0 );

	// Cleanup
	rtu::vectorFreeMemory( _triangleSides );
	rtu::vectorFreeMemory( _triangleBoxes );
	rtu::vectorFreeMemory( _events[0] );
	rtu::vectorFreeMemory( _events[1] );
	rtu::vectorFreeMemory( _events[2] );

	return tree;
}

// Private methods

RawKdNode* TriangleTreeBuilder::leafNode( const RawKdNode::Elements& triangles, unsigned int treeDepth )
{
	if( treeDepth > _stats->treeDepth )
		_stats->treeDepth = treeDepth;

	_stats->elemIdCount += triangles.size();
	++_stats->leafCount;

	return new RawKdNode( triangles );
}

bool TriangleTreeBuilder::terminate( const SahResult& bestResult, unsigned int triangleCount ) const
{
	// TODO: may be improved by allowing further subdivision due to overestimation of sah cost function
	return ( bestResult.cost ) >= ( _intersectionCost * triangleCount );
}

void TriangleTreeBuilder::sah( SahResult& result, const SplitPlane& plane, const AABB& bbox, 
							   unsigned int nL, unsigned int nP, unsigned int nR )
{
	AABB left;
	AABB right;
	bbox.split( left, right, plane );

	const float invTotalArea = 1.0f / bbox.surfaceArea();
	const float pL = left.surfaceArea() * invTotalArea;
	const float pR = right.surfaceArea() * invTotalArea;

	const float invTotalExtension = 1.0f / ( bbox.maxv[plane.axis] - bbox.minv[plane.axis] );
	const float leftExtensionRatio = ( left.maxv[plane.axis] - left.minv[plane.axis] ) * invTotalExtension;
	const float rightExtensionRatio = ( right.maxv[plane.axis] - right.minv[plane.axis] ) * invTotalExtension;

	unsigned int numLeft = nL + nP;
	unsigned int numRight = nR;
	float leftCost = _traversalCost + _intersectionCost * ( pL*numLeft + pR*numRight );

	// Favor splits that cut off empty space by biasing the cost function
	if( numLeft == 0 )
		leftCost *= 0.8f + 0.2f * rightExtensionRatio;
	else if( numRight == 0 )
		leftCost *= 0.8f + 0.2f * leftExtensionRatio;

	numLeft = nL;
	numRight = nP + nR;
	float rightCost = _traversalCost + _intersectionCost * ( pL*numLeft + pR*numRight );

	// Favor splits that cut off empty space by biasing the cost function
	if( numLeft == 0 )
		rightCost *= 0.8f + 0.2f * rightExtensionRatio;
	else if( numRight == 0 )
		rightCost *= 0.8f + 0.2f * leftExtensionRatio;

	if( leftCost <= rightCost )
	{
		result.cost = leftCost;
		result.side = LEFT;
	}
	else
	{
		result.cost = rightCost;
		result.side = RIGHT;
	}
}

RawKdNode* TriangleTreeBuilder::recursiveBuild( const RawKdNode::Elements& triangles, const AABB& bbox, unsigned int treeDepth )
{
	SplitPlane plane;
	SahResult sahResult;
	unsigned int triangleCount = triangles.size();
	++_stats->nodeCount;

	// Compute cost-optimized split plane for given set of triangles and enclosing bounding box
	findPlane( plane, sahResult, triangleCount, triangles, bbox );

	// Further subdivision does not pay off if even	the best split is more costly then not splitting at all
	if( terminate( sahResult, triangleCount ) )
		return leafNode( triangles, treeDepth );

	// Split current bounding box according to chosen split plane
	AABB leftBox;
	AABB rightBox;
	bbox.split( leftBox, rightBox, plane );

	// Partition current triangles into both child bounding boxes
	RawKdNode::Elements leftTriangles;
	RawKdNode::Elements rightTriangles;
	partition( leftTriangles, rightTriangles, sahResult, plane, triangles );

	// Recursive tree build for both children
	RawKdNode* left  = recursiveBuild( leftTriangles, leftBox, treeDepth + 1 );
	rtu::vectorFreeMemory( leftTriangles );
	RawKdNode* right = recursiveBuild( rightTriangles, rightBox, treeDepth + 1 );
	rtu::vectorFreeMemory( rightTriangles );

	return new RawKdNode( plane, left, right );
}

void TriangleTreeBuilder::findPlane( SplitPlane& plane, SahResult& sahResult, unsigned int& triangleCount, 
									 const RawKdNode::Elements& triangles, const AABB& bbox )
{
	// TODO: may put some of these as members of class, if it improves performance
	SplitPlane currentPlane;
	SahResult currentSahResult;

	rtu::float3 v0;
	rtu::float3 v1;
	rtu::float3 v2;

	Event event;
	EventOrder predicate;
	unsigned int nL;
	unsigned int nP;
	unsigned int nR;
	unsigned int numStartEvents;
	unsigned int numPlanarEvents;
	unsigned int numEndEvents;

	const std::vector<rtu::float3>& vertices = _geometry->vertices;

	// Reset data, but preserve memory allocation
	triangleCount = 0;
	_events[0].clear();
	_events[1].clear();
	_events[2].clear();

	// Clip triangles to current bbox (perfect splits)
	// Compute unsorted event lists
	// Reset triangle classifications
	for( unsigned int t = 0, limit = triangles.size(); t < limit; ++t )
	{
		unsigned int triangleId = triangles[t];
		// Get triangle bbox
		AABB& currentBox = _triangleBoxes[triangleId];
		// Get triangle vertices and update bbox
		const TriDesc& triDesc = _geometry->triDesc[triangleId];
		bbox.clipTriangle( currentBox, vertices[triDesc.v0], 
			                           vertices[triDesc.v1], 
									   vertices[triDesc.v2] );

		// Degenerate box, skip triangle
		if( currentBox.isDegenerate() )
		{
			_triangleSides[triangleId] = INVALID;
			continue;
		}

		// Validate triangle
		++triangleCount;
		_triangleSides[triangleId] = BOTH;

		// Insert triangle events in lists
		for( unsigned int k = 0; k < 3; ++k )
		{
			std::vector<Event>& currentEvents = _events[k];

			if( !currentBox.isPlanar( k ) )
			{
				// Add two distinct events
				event.triangleId = triangles[t];
				event.position = currentBox.minv[k];
				event.type = Event::START;
				currentEvents.push_back( event );
				event.position = currentBox.maxv[k];
				event.type = Event::END;
				currentEvents.push_back( event );
			}
			else
			{
				// Add one planar event
				event.triangleId = triangles[t];
				event.position = currentBox.minv[k];
				event.type = Event::PLANAR;
				currentEvents.push_back( event );
			}
		}
	}

	// Sort event lists
	std::sort( _events[0].begin(), _events[0].end(), predicate );
	std::sort( _events[1].begin(), _events[1].end(), predicate );
	std::sort( _events[2].begin(), _events[2].end(), predicate );

	// Reset best classification
	plane.position = rtu::mathf::MAX_VALUE;
	plane.axis = 0;
	sahResult.cost = rtu::mathf::MAX_VALUE;
	sahResult.side = LEFT;

	// Iterate through all 3 axis computing SAH for each sorted event
	for( unsigned int k = 0; k < 3; ++k )
	{
		// Set current axis and get events
		currentPlane.axis = k;
		const std::vector<Event>& currentEvents = _events[k];

		// Start with all triangles on the right
		nL = 0;
		nP = 0;
		nR = triangleCount;

		// Iteratively sweep plane over all split candidates
		for( unsigned int i = 0, eventCount = currentEvents.size(); i < eventCount; /*empty*/ )
		{
			currentPlane.position = currentEvents[i].position;
			numStartEvents = 0;
			numPlanarEvents = 0;
			numEndEvents = 0;

			while( ( i < eventCount ) && ( currentPlane.position == currentEvents[i].position ) && 
				   ( currentEvents[i].type == Event::END ) )
			{
				++numEndEvents;
				++i;
			}

			while( ( i < eventCount ) && ( currentPlane.position == currentEvents[i].position ) && 
				   ( currentEvents[i].type == Event::PLANAR ) )
			{
				++numPlanarEvents;
				++i;
			}

			while( ( i < eventCount ) && ( currentPlane.position == currentEvents[i].position ) && 
				   ( currentEvents[i].type == Event::START ) )
			{
				++numStartEvents;
				++i;
			}

			// Update recurrence
			nP = numPlanarEvents;
			nR -= numPlanarEvents;
			nR -= numEndEvents;
			
			// Compute SAH and save best result
			sah( currentSahResult, currentPlane, bbox, nL, nP, nR );

			if( currentSahResult.cost < sahResult.cost )
			{
				sahResult = currentSahResult;
				plane = currentPlane;
			}

			// Update recurrence
			nL += numStartEvents;
			nL += numPlanarEvents;
		}
	}
}

void TriangleTreeBuilder::partition( RawKdNode::Elements& left, RawKdNode::Elements& right, const SahResult& sahResult, 
									 const SplitPlane& plane, const RawKdNode::Elements& triangles )
{
	const std::vector<Event>& bestEvents = _events[plane.axis];

	// Iterate through events of chosen axis and reclassify triangles, if possible
	for( unsigned int i = 0, size = bestEvents.size(); i < size; ++i )
	{
		const Event& event = bestEvents[i];
		if( ( event.type == Event::END ) && ( event.position <= plane.position ) )
		{
			_triangleSides[event.triangleId] = LEFT;
		}
		else if( ( event.type == Event::START ) && ( event.position >= plane.position ) )
		{
			_triangleSides[event.triangleId] = RIGHT;
		}
		else if( event.type == Event::PLANAR )
		{
			if( ( event.position < plane.position ) || ( ( event.position == plane.position ) && ( sahResult.side == LEFT ) ) )
			{
				_triangleSides[event.triangleId] = LEFT;
			}
			if( ( event.position > plane.position ) || ( ( event.position == plane.position ) && ( sahResult.side == RIGHT ) ) )
			{
				_triangleSides[event.triangleId] = RIGHT;
			}
		}
	}

	// Iterate through triangle sides and partition triangles
	for( unsigned int i = 0, size = triangles.size(); i < size; ++i )
	{
		const unsigned int triangleId = triangles[i];

		if( _triangleSides[triangleId] == LEFT )
		{
			left.push_back( triangleId );
		}
		else if( _triangleSides[triangleId] == RIGHT )
		{
			right.push_back( triangleId );
		}
		else if( _triangleSides[triangleId] == BOTH )
		{
			left.push_back( triangleId );
			right.push_back( triangleId );
		}
		// else INVALID, so skip triangle
	}
}

} // namespace rtc

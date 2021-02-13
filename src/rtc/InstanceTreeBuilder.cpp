#include <rtc/InstanceTreeBuilder.h>
#include <rtu/stl.h>

namespace rtc {

RawKdTree* InstanceTreeBuilder::buildTree( const std::vector<Instance>& instances )
{
	// Create instance vector for entire scene
	unsigned int instanceCount = instances.size();

	if( instanceCount == 0 )
		return new RawKdTree();

	RawKdNode::Elements initialInstances( instanceCount );
	_instances = &instances;

	RawKdTree* tree = new RawKdTree();
	_stats = &tree->stats;
	_stats->reset();

	AABB& sceneBox = tree->bbox;
	sceneBox = instances[0].bbox;
	initialInstances[0] = 0;

	// Store instance ids and compute scene bounding box
	for( unsigned int i = 1; i < instanceCount; ++i )
	{
		initialInstances[i] = i;
		sceneBox.expandBy( instances[i].bbox );
	}

	// Recursive tree build
	tree->root = recursiveBuild( initialInstances, tree->bbox, 0 );
	return tree;
}

// Private methods

RawKdNode* InstanceTreeBuilder::recursiveBuild( const RawKdNode::Elements& instances, const AABB& bbox, unsigned int treeDepth )
{
	++_stats->nodeCount;

	// Check trivial case
	if( instances.size() == 1 )
		return leafNode( instances, treeDepth );
	
	// Find axis in descending order of maximum extent
	unsigned int orderedAxis[3];
	orderAxis( orderedAxis, bbox );

	unsigned int axis;
	SplitPlane plane;
	float distance;
	unsigned int currentInstanceCount = instances.size();
	const std::vector<Instance>& originalInstances = *_instances;

	// Best split data
	float minDistance = rtu::mathf::MAX_VALUE;
	unsigned int bestInstance;

	float leftMinDistance = rtu::mathf::MAX_VALUE;
	unsigned int leftBestInstance;
	unsigned int leftCount = 0;
	float leftSplitPosition;

	float rightMinDistance = rtu::mathf::MAX_VALUE;
	unsigned int rightBestInstance;
	unsigned int rightCount = 0;
	float rightSplitPosition;

	// Compute the center of current bbox
	rtu::float3& center = bbox.minv + bbox.maxv;
	center *= 0.5f;

	// Child elements, if any
	RawKdNode::Elements leftInstances;
	RawKdNode::Elements rightInstances;

	// For each axis
	for( unsigned int k = 0; k < 3; ++k )
	{
		axis = orderedAxis[k];
		leftInstances.clear();
		rightInstances.clear();

		// Get "best" split position: closest object border to center of bbox
		for( unsigned int i = 0; i < currentInstanceCount; ++i )
		{
			const unsigned int instanceId = instances[i];
			const AABB& currentBox = originalInstances[instanceId].bbox;

			distance = currentBox.minv[axis] - center[axis];
			if( distance <= 0.0f )
			{
				++leftCount;
				distance = rtu::mathf::abs( distance );
				if( distance < leftMinDistance )
				{
					leftMinDistance = distance;
					leftBestInstance = instanceId;
					leftSplitPosition = currentBox.minv[axis];
				}
			}
			else
			{
				++rightCount;
				distance = rtu::mathf::abs( distance );
				if( distance < rightMinDistance )
				{
					rightMinDistance = distance;
					rightBestInstance = instanceId;
					rightSplitPosition = currentBox.minv[axis];
				}
			}

			distance = currentBox.maxv[axis] - center[axis];
			if( distance <= 0.0f )
			{
				++leftCount;
				distance = rtu::mathf::abs( distance );
				if( distance < leftMinDistance )
				{
					leftMinDistance = distance;
					leftBestInstance = instanceId;
					leftSplitPosition = currentBox.maxv[axis];
				}
			}
			else
			{
				++rightCount;
				distance = rtu::mathf::abs( distance );
				if( distance < rightMinDistance )
				{
					rightMinDistance = distance;
					rightBestInstance = instanceId;
					rightSplitPosition = currentBox.maxv[axis];
				}
			}
		}

		// Get the split position that lies between cell center and object median
		if( leftCount > rightCount )
		{
			minDistance = leftMinDistance;
			bestInstance = leftBestInstance;
			plane.position = leftSplitPosition;
		}
		else
		{
			minDistance = rightMinDistance;
			bestInstance = rightBestInstance;
			plane.position = rightSplitPosition;
		}

		// Partition instances according to best split position
		for( unsigned int i = 0; i < currentInstanceCount; ++i )
		{
			const unsigned int instanceId = instances[i];
			const AABB& currentBox = originalInstances[instanceId].bbox;

			if( ( currentBox.minv[axis] <= plane.position ) && ( currentBox.maxv[axis] <= plane.position ) )
			{
				// Completely on left side
				leftInstances.push_back( instanceId );
			}
			else if( ( currentBox.minv[axis] >= plane.position ) && ( currentBox.maxv[axis] >= plane.position ) )
			{
				// Completely on right side
				rightInstances.push_back( instanceId );
			}
			else
			{
				// On both sides
				leftInstances.push_back( instanceId );
				rightInstances.push_back( instanceId );
			}
		}

		// Check if best split was able to partition instances
		if( ( leftInstances.size() >= currentInstanceCount ) || ( rightInstances.size() >= currentInstanceCount ) )
			continue;

		// Split current bounding box according to chosen split plane
		plane.axis = axis;
		AABB leftBox;
		AABB rightBox;
		bbox.split( leftBox, rightBox, plane );

		// Recursive tree build for both children
		RawKdNode* left  = recursiveBuild( leftInstances, leftBox, treeDepth + 1 );
		rtu::vectorFreeMemory( leftInstances );
		RawKdNode* right = recursiveBuild( rightInstances, rightBox, treeDepth + 1 );
		rtu::vectorFreeMemory( rightInstances );

		return new RawKdNode( plane, left, right );
	}

	// No valid split plane could be found
	return leafNode( instances, treeDepth );
}

void InstanceTreeBuilder::orderAxis( unsigned int* axis, const AABB& bbox )
{
	// Find axis in descending order of maximum extent
	rtu::float3& diagonal = bbox.maxv - bbox.minv;

	if( diagonal.x > diagonal.y )
	{
		if( diagonal.x > diagonal.z )
		{
			axis[0] = 0; // x
			if( diagonal.y > diagonal.z )
			{
				axis[1] = 1; // y
				axis[2] = 2; // z
			}
			else
			{
				axis[1] = 2; // z
				axis[2] = 1; // y
			}
		}
		else
		{
			axis[0] = 2; // z
			axis[1] = 0; // x
			axis[2] = 1; // y
		}
	}
	else
	{
		if( diagonal.y > diagonal.z )
		{
			axis[0] = 1; // y
			if( diagonal.x > diagonal.z )
			{
				axis[1] = 0; // x
				axis[2] = 2; // z
			}
			else
			{
				axis[1] = 2; // z
				axis[2] = 0; // x
			}
		}
		else
		{
			axis[0] = 2; // z
			axis[1] = 1; // y
			axis[2] = 0; // x
		}
	}
}

RawKdNode* InstanceTreeBuilder::leafNode( const RawKdNode::Elements& instances, unsigned int treeDepth )
{
	if( treeDepth > _stats->treeDepth )
		_stats->treeDepth = treeDepth;

	_stats->elemIdCount += instances.size();
	++_stats->leafCount;

	return new RawKdNode( instances );
}

} // namespace rtc

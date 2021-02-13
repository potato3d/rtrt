#include <rtc/KdTreeBuilder.h>
#include <queue>

namespace rtc {

TriangleTreeBuilder KdTreeBuilder::_triangleTreeBuilder;
InstanceTreeBuilder KdTreeBuilder::_instanceTreeBuilder;

void KdTreeBuilder::buildTree( Geometry* geometry )
{
	// Create kd-Tree using current geometry data. Ref_ptr will delete object in the end of this method.
	rtu::ref_ptr<RawKdTree> tree = _triangleTreeBuilder.buildTree( geometry );

	// Create accelerated kd tree for ray tracing
	convertRawTree( geometry->kdTree, tree.get() );
}

void KdTreeBuilder::buildTree( KdTree& result, const std::vector<Instance>& instances )
{
	// Rebuild instance kd tree
	rtu::ref_ptr<RawKdTree> tree = _instanceTreeBuilder.buildTree( instances );

	// Create accelerated kd tree for ray tracing
	convertRawTree( result, tree.get() );
}

void KdTreeBuilder::convertRawTree( KdTree& result, RawKdTree* tree )
{
	// Store bounding box
	result.bbox = tree->bbox;

	// TODO: avoid reallocation if new size <= current size
	// Create optimized nodes
	if( result.root != NULL )
		delete result.root;
	result.root = new KdNode[tree->stats.nodeCount];

	// Create triangle ids
	if( result.elements != NULL )
		delete result.elements;
	result.elements = new unsigned int[tree->stats.elemIdCount];

	// Store triangle ids and setup optimized nodes
	unsigned int dstNode = 0;
	unsigned int dstElemId = 0;
	unsigned int elemIdCount = 0;
    unsigned int childId = 1;

	RawKdNode* current;
	std::queue<RawKdNode*> next;
	next.push( tree->root.get() );

	while( !next.empty() )
	{
		current = next.front();
		next.pop();

		if( !current->isLeaf() )
		{
			result.root[dstNode].setInternalNode( current->split, &result.root[childId] );
			next.push( current->left.get() );
			next.push( current->right.get() );

			// Update variables for next iteration
            childId += 2;
            ++dstNode;
		}
		else
		{
			// Store element ids
			elemIdCount = current->elements.size();
			for( unsigned int t = 0; t < elemIdCount; ++t )
			{
				result.elements[dstElemId+t] = current->elements[t];
			}

			// Update optimized node
			result.root[dstNode].setLeafNode( dstElemId, elemIdCount );

			// Update variables for next iteration
			++dstNode;
			dstElemId += elemIdCount;
		}
	}
}

} // namespace rtc

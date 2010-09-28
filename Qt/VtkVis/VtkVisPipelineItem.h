/**
 * \file VtkVisPipelineItem.h
 * 17/2/2010 LB Initial implementation
 *
 */


#ifndef VTKVISPIPELINEITEM_H
#define VTKVISPIPELINEITEM_H

// ** INCLUDES **
#include "TreeItem.h"
#include "Configure.h"

#include <QList>
#ifdef OGS_USE_OPENSG
#include <OpenSG/OSGNode.h>
#endif // OGS_USE_OPENSG

class vtkAlgorithm;
class vtkPointSet;
class vtkDataSetMapper;
class vtkActor;
class vtkRenderer;
class VtkAlgorithmProperties;

/**
 * \brief An item in the VtkVisPipeline containing a graphic object to be visualised.
 *
 * Any VTK-object (source-items, filter-items, etc.) need to be put into a VtkPipelineItem
 * to be assigned a mapper, an actor and its visualisation properties (colour, etc.).
 */
class VtkVisPipelineItem : public TreeItem
{
public:
	/// Constructor for a source/filter object.
#ifdef OGS_USE_OPENSG
	VtkVisPipelineItem(vtkRenderer* renderer, vtkAlgorithm* algorithm,
		TreeItem* parentItem, vtkPointSet* input, OSG::NodePtr parentNode,
		const QList<QVariant> data = QList<QVariant>());
#else // OGS_USE_OPENSG
	VtkVisPipelineItem(vtkRenderer* renderer, vtkAlgorithm* algorithm,
		TreeItem* parentItem, vtkPointSet* input,
		const QList<QVariant> data = QList<QVariant>());
#endif // OGS_USE_OPENSG

	~VtkVisPipelineItem();

	QVariant data(int column) const;
	bool setData(int column, const QVariant &value);

	/// Returns the algorithm object
	vtkAlgorithm* algorithm() const { return _algorithm; }

	/// Returns the actor
	vtkActor* actor() const { return _actor; }

	/// Returns the mapper
	vtkDataSetMapper* mapper() const { return _mapper; }

	/// Returns if the VTK object is visible in the visualization.
	bool isVisible() const;

	/// Sets the visibility of the VTK object in the visualization.
	void setVisible(bool visible);

	/// Writes this algorithm's vtkDataSet (i.e. vtkPolyData or vtkUnstructuredGrid) to a vtk-file.
	int writeToFile(const std::string &filename) const;

private:
	vtkAlgorithm* _algorithm;
	vtkPointSet* _input;
	vtkDataSetMapper* _mapper;
	vtkActor* _actor;
	vtkRenderer* _renderer;
#ifdef OGS_USE_OPENSG
	OSG::NodePtr _parentNode;
#endif // OGS_USE_OPENSG

	/// Initalises vtkMapper and vtkActor necessary for visualization of
	/// the item and sets the item's properties.
	void Initialize();

	/// Sets pre-set properties on vtkActor and on vtkMapper
	void setVtkProperties(VtkAlgorithmProperties* vtkProps);
};

#endif // VTKVISPIPELINEITEM_H


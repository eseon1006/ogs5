/**
 * \file VtkMeshSource.h
 * 19/03/2010 KR Initial implementation
 *
 */


#ifndef VTKMESHSOURCE_H
#define VTKMESHSOURCE_H

// ** INCLUDES **
#include <map>

#include <vtkUnstructuredGridAlgorithm.h>

#include "Color.h"
#include "GridAdapter.h"
#include "VtkAlgorithmProperties.h"

/**
 * \brief VTK source object for the visualisation of unstructured grids
 */
class VtkMeshSource : public vtkUnstructuredGridAlgorithm, public VtkAlgorithmProperties
{

public:
	/// Create new objects with New() because of VTKs object reference counting.
	static VtkMeshSource* New();

	vtkTypeRevisionMacro(VtkMeshSource, vtkUnstructuredGridAlgorithm);

	const char* GetMaterialArrayName() const { return _matName; }

	/// Returns the colour lookup table generated for material groups. 
	/// This method should only be called after setColorsFromMaterials().
	const std::map<std::string, GEOLIB::Color>& getColorLookupTable() const { return _colorLookupTable; };


	/// Sets the nodes and elements of the mesh that should be visualised
	void setMesh(const std::vector<GEOLIB::Point*> *nodes, const std::vector<GridAdapter::Element*> *elems) 
	{ 
		_nodes = nodes; 
		_elems = elems;
	};

	/// Prints the mesh data to an output stream.
	void PrintSelf(ostream& os, vtkIndent indent);

	/** 
	 * \brief Generates random colors based on the material scalar value. 
	 * Each element of the mesh is assigned an RGB-value based on its material group.
	 * This method should only be called after setMesh()!
	 */
	void setColorsFromMaterials();

protected:
	VtkMeshSource();
	~VtkMeshSource() {};

	/// Computes the unstructured grid data object.
	int RequestData(vtkInformation* request, 
		            vtkInformationVector** inputVector, 
					vtkInformationVector* outputVector);



	const std::vector<GEOLIB::Point*> *_nodes;
	const std::vector<GridAdapter::Element*> *_elems;

private:
	/// The colour table for material groups. This table is generated in the setColorsFromMaterials() method.
	std::map<std::string, GEOLIB::Color> _colorLookupTable;

	const char* _matName;

};

#endif // VTKMESHSOURCE_H

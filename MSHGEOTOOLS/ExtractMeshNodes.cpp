/*
 * ExtractMeshNodes.cpp
 *
 *  Created on: Dec 3, 2010
 *      Author: TF
 */

// STL
#include <algorithm>

#include "ExtractMeshNodes.h"

// BASELIB
#include "quicksort.h"

// GEO
#include "Point.h"

// MathLib
#include "MathTools.h"

namespace MeshLib {

ExtractMeshNodes::ExtractMeshNodes(const CFEMesh* msh) :
	_msh (msh), _gli_pnt_offset (0)
{
}

void ExtractMeshNodes::writeMeshNodeIDs (std::ostream& os, std::ostream& gli_out, const GEOLIB::Polygon& polygon)
{
	// get all nodes of mesh
	const std::vector<MeshLib::CNode*>& msh_nodes (_msh->getNodeVector());

	std::vector<size_t> node_indices;

	for (size_t j(0); j<msh_nodes.size(); j++) {
		if (msh_nodes[j]->Interior()) {
			GEOLIB::Point pnt (msh_nodes[j]->X(), msh_nodes[j]->Y(), 0);
			if (polygon.isPntInPolygon(pnt)) {
				node_indices.push_back (j);
			}
		}
	}
	// write data
	for (size_t k(0); k<node_indices.size(); k++) {
		os << node_indices[k] << std::endl;
	}

	for (size_t k(0); k<node_indices.size(); k++) {
		gli_out << k + _gli_pnt_offset << " " << msh_nodes[node_indices[k]]->X() << " " <<  msh_nodes[node_indices[k]]->Y() << " " << msh_nodes[node_indices[k]]->Z() << std::endl;
	}
	_gli_pnt_offset += node_indices.size();
}

void ExtractMeshNodes::writeTopSurfaceMeshNodeIDs (std::ostream& os, std::ostream& gli_out, const GEOLIB::Polygon& polygon)
{
	// get all nodes of mesh
	const std::vector<MeshLib::CNode*>& msh_nodes (_msh->getNodeVector());

	std::vector<GEOLIB::PointWithID> nodes_as_points;

	for (size_t j(0); j<msh_nodes.size(); j++) {
//		if (msh_nodes[j]->Interior()) {
			GEOLIB::Point pnt (msh_nodes[j]->X(), msh_nodes[j]->Y(), 0);
			if (polygon.isPntInPolygon(pnt)) {
				nodes_as_points.push_back (GEOLIB::PointWithID (msh_nodes[j]->X(), msh_nodes[j]->Y(), msh_nodes[j]->Z(), j));
			}
//		}
	}

	std::vector<size_t> perm;
	for (size_t k(0); k<nodes_as_points.size(); k++) {
		perm.push_back(k);
	}
	Quicksort<GEOLIB::PointWithID> (nodes_as_points, 0, nodes_as_points.size(), perm);

	double eps (sqrt(std::numeric_limits<double>::min()));

	// write data
	for (size_t k(1); k<nodes_as_points.size(); k++) {
		const GEOLIB::PointWithID& p0 (nodes_as_points[k-1]);
		const GEOLIB::PointWithID& p1 (nodes_as_points[k]);
		if (fabs (p0[0]-p1[0]) > eps || fabs (p0[1]-p1[1]) > eps) {
			os << p0.getID() << std::endl;
		}
	}
	// write last point
	os << nodes_as_points[nodes_as_points.size()-1].getID() << std::endl;

	size_t n_nodes (0);
	gli_out.precision (14);
	for (size_t k(1); k<nodes_as_points.size(); k++) {
		const GEOLIB::PointWithID& p0 (nodes_as_points[k-1]);
		const GEOLIB::PointWithID& p1 (nodes_as_points[k]);
		if (fabs (p0[0]-p1[0]) > eps || fabs (p0[1]-p1[1]) > eps) {
			gli_out << n_nodes + _gli_pnt_offset << " " << std::scientific << p0 << " $NAME " << p0.getID() << std::endl;
			n_nodes++;
		}
	}
	// write last point
	gli_out << n_nodes + _gli_pnt_offset << " " << std::scientific << nodes_as_points[nodes_as_points.size()-1] << " $NAME " << nodes_as_points[nodes_as_points.size()-1].getID() << std::endl;
	n_nodes++;
	_gli_pnt_offset += n_nodes;
}

void ExtractMeshNodes::getOrthogonalProjectedMeshNodesAlongPolyline (
		GEOLIB::Polyline const& polyline,
		std::vector<GEOLIB::PointWithID>& nodes_as_points) const
{
	// get all nodes of mesh
	std::vector<MeshLib::CNode*> const & msh_nodes (_msh->getNodeVector());
	size_t number_of_msh_nodes (msh_nodes.size());

	size_t number_of_ply_pnts (polyline.getNumberOfPoints());
	if (polyline.isClosed())
		number_of_ply_pnts--;

	for (size_t k(0); k<number_of_ply_pnts; k++) {
		GEOLIB::Point proj_ply_pnt ((*(polyline.getPoint(k)))[0], (*(polyline.getPoint(k)))[1], 0.0);
		for (size_t j(0); j<number_of_msh_nodes; j++) {
			GEOLIB::Point mesh_pnt (msh_nodes[j]->X(), msh_nodes[j]->Y(), 0);
			if (MathLib::sqrDist (&mesh_pnt, &proj_ply_pnt) < std::numeric_limits<double>::epsilon()) {
				nodes_as_points.push_back (GEOLIB::PointWithID (msh_nodes[j]->X(), msh_nodes[j]->Y(), msh_nodes[j]->Z(), j));
			}
		}
	}

	std::vector<size_t> perm;
	for (size_t k(0); k<nodes_as_points.size(); k++) {
		perm.push_back(k);
	}
	Quicksort<GEOLIB::PointWithID> (nodes_as_points, 0, nodes_as_points.size(), perm);
}

void ExtractMeshNodes::getTopMeshNodesAlongPolylineAsPoints(
		const GEOLIB::Polyline& polyline,
		std::vector<GEOLIB::Point*>& top_points) const
{
	std::vector<GEOLIB::PointWithID> nodes_as_points;
	this->getOrthogonalProjectedMeshNodesAlongPolyline (polyline, nodes_as_points);

//	std::ofstream out ("MeshNodesAsPoints.gli");
//	out << "#POINTS" << std::endl;
//	for (size_t k(0); k<nodes_as_points.size(); k++) {
//		out << k << " " << nodes_as_points[k] << " $NAME MeshNodeID" << nodes_as_points[k].getID () << std::endl;
//	}
//	out << "#STOP" << std::endl;
//	out.close();

	double eps (std::numeric_limits<double>::epsilon());
	// collect data (lowest points with same x and y coodinates)
	size_t upper_bound (nodes_as_points.size()-1);
	for (size_t k(0); k<upper_bound; k++) {
		const GEOLIB::PointWithID& p0 (nodes_as_points[k]);
		const GEOLIB::PointWithID& p1 (nodes_as_points[k+1]);
		if (fabs (p0[0]-p1[0]) > eps || fabs (p0[1]-p1[1]) > eps) {
			top_points.push_back (new GEOLIB::Point (nodes_as_points[k].getData()));
		}
	}
	top_points.push_back (new GEOLIB::Point (nodes_as_points[upper_bound].getData()));
}

void ExtractMeshNodes::getBottomMeshNodesAlongPolylineAsPoints(
		const GEOLIB::Polyline& polyline,
		std::vector<GEOLIB::Point*>& bottom_points) const
{
	std::vector<GEOLIB::PointWithID> nodes_as_points;
	this->getOrthogonalProjectedMeshNodesAlongPolyline (polyline, nodes_as_points);

	double eps (std::numeric_limits<double>::epsilon());
	// collect data (lowest points with same x and y coodinates)
	bottom_points.push_back (new GEOLIB::Point (nodes_as_points[0].getData()));
	size_t upper_bound (nodes_as_points.size()-1);
	for (size_t k(0); k<upper_bound; k++) {
		const GEOLIB::PointWithID& p0 (nodes_as_points[k]);
		const GEOLIB::PointWithID& p1 (nodes_as_points[k+1]);
		if (fabs (p0[0]-p1[0]) > eps || fabs (p0[1]-p1[1]) > eps) {
			bottom_points.push_back (new GEOLIB::Point (nodes_as_points[k+1].getData()));
		}
	}
}

void ExtractMeshNodes::getPolygonFromPolyline (const GEOLIB::Polyline& polyline,
		GEOLIB::GEOObjects* geo_obj, std::string const& name, GEOLIB::Polygon* & polygon) const
{
	std::vector<GEOLIB::Point*> top_polygon_pnts;
	this->getTopMeshNodesAlongPolylineAsPoints (polyline, top_polygon_pnts);

	std::vector<GEOLIB::Point*> bottom_polygon_pnts;
	this->getBottomMeshNodesAlongPolylineAsPoints (polyline, bottom_polygon_pnts);

	// append new points to then end of the points vector
	std::vector<size_t>* top_ids (new std::vector<size_t>);
	geo_obj->appendPointVec (top_polygon_pnts, name, top_ids);
	std::vector<size_t>* bottom_ids (new std::vector<size_t>);
	geo_obj->appendPointVec (bottom_polygon_pnts, name, bottom_ids);

	// create (an empty) polygon
	polygon = new GEOLIB::Polygon (*(geo_obj->getPointVec (name)));

	std::vector<GEOLIB::Point*> const* original_pnts (geo_obj->getPointVec(name));

	// *** add ids of new points to polygon
	// for top polyline sort points along polyline
	size_t s (top_ids->size());
	for (size_t j(0); j<polyline.getNumberOfPoints(); j++) {
		for (size_t k(0); k<s; k++) {
			GEOLIB::Point *test_pnt (new GEOLIB::Point (*(*original_pnts)[(*top_ids)[k]]));
			(*test_pnt)[2] = 0.0;
			if (MathLib::sqrDist (polyline.getPoint(j), test_pnt) < std::numeric_limits<double>::epsilon()) {
				polygon->addPoint ((*top_ids)[k]);
				k=s;
			}
			delete test_pnt;
		}
	}

	// for bottom polyline sort points along polyline in reverse order
	s = bottom_ids->size();
	for (size_t j(polyline.getNumberOfPoints()-1); j>1; j--) {
		for (size_t k(0); k<s; k++) {
			GEOLIB::Point *test_pnt (new GEOLIB::Point (*(*original_pnts)[(*bottom_ids)[k]]));
			(*test_pnt)[2] = 0.0;
			if (MathLib::sqrDist (polyline.getPoint(j), test_pnt) < std::numeric_limits<double>::epsilon()) {
				polygon->addPoint ((*bottom_ids)[k]);
				k=s;
			}
			delete test_pnt;
		}
	}

	// close polygon
	polygon->addPoint (polygon->getPointID(0));
	polygon->initialise();

	//
//	std::ofstream out ("SurfaceBC.gli");
//	out << "#POINTS" << std::endl;
//	for (size_t k(0); k<polygon->getNumberOfPoints(); k++) {
//		out << k << " " << *(polygon->getPoint(k)) << std::endl;
//	}
//	out << "POLYLINE" << std::endl;
//	out << "\t$NAME" << std::endl;
//	out << "\t\tPolygonForBC" << std::endl;
//	out << "\t$POINTS" << std::endl;
//	for (size_t k(0); k<polygon->getNumberOfPoints(); k++) {
//		out << "\t" << k << std::endl;
//	}
//	out << "#STOP" << std::endl;
//	out.close ();

	delete top_ids;
	delete bottom_ids;
}

void ExtractMeshNodes::writeMesh2DNodeIDAndArea (std::ostream& os, std::ostream& gli_out, const GEOLIB::Polygon& polygon)
{
	// get all nodes of mesh
	const std::vector<MeshLib::CNode*>& msh_nodes (_msh->getNodeVector());

	// store node id
	std::vector<size_t> node_ids;

	for (size_t j(0); j<msh_nodes.size(); j++) {
		GEOLIB::Point pnt (msh_nodes[j]->X(), msh_nodes[j]->Y(), 0);
		if (polygon.isPntInPolygon(pnt)) {
			node_ids.push_back (j);
		}
	}
	std::sort (node_ids.begin(), node_ids.end());

	size_t n_nodes (node_ids.size());
	std::vector<double> areas (n_nodes, 0.0);
	// in order to compute the area we need the mesh elements
	const std::vector<MeshLib::CElem*>& msh_elem (_msh->getElementVector());
	for (size_t k(0); k<n_nodes; k++) {
		// get all associated mesh elements
		std::vector<size_t> const & mesh_elem_ids(msh_nodes[node_ids[k]]->getConnectedElementIDs());
		size_t n_mesh_elem_ids (mesh_elem_ids.size());
		// get areas for mesh elements
		double area (0.0);
		for (size_t j(0); j<n_mesh_elem_ids; j++) {
			// check if all mesh nodes of the element are inside the polygon
			std::vector<size_t> mesh_element_node_indices;
			msh_elem[mesh_elem_ids[j]]->getNodeIndices (mesh_element_node_indices);
			size_t n_of_nodes_of_element (mesh_element_node_indices.size());
			bool found (true);
			for (size_t i(0); i<n_of_nodes_of_element && found; i++) {
				std::vector<size_t>::iterator it(std::find (node_ids.begin(), node_ids.end(), mesh_element_node_indices[i]));
				if (it == node_ids.end())
					found = false;
			}
			if (found)
				area += msh_elem[mesh_elem_ids[j]]->calcVolume();
		}
		areas[k] = area/3.0;
	}

	// write ids and areas
	for (size_t k(0); k<n_nodes; k++) {
		if (areas[k] > std::numeric_limits<double>::epsilon())
			os << node_ids[k] << " " << areas[k] << std::endl;
	}

	n_nodes = 0;
	gli_out.precision (14);
	for (std::vector<size_t>::const_iterator it (node_ids.begin()); it != node_ids.end(); it++) {
		gli_out << n_nodes + _gli_pnt_offset << " " << std::scientific
			<< msh_nodes[*it]->X() << " " << msh_nodes[*it]->Y()<< " " << msh_nodes[*it]->Z()
			<< " $NAME " << *it << std::endl;
		n_nodes++;
	}
	_gli_pnt_offset += n_nodes;
}

void ExtractMeshNodes::writeMesh2DNodeIDAndArea (std::ostream& os, std::ostream& gli_out, const GEOLIB::Polygon& bounding_polygon, std::vector<GEOLIB::Polygon*> const& holes)
{
	// get all nodes of mesh
	const std::vector<MeshLib::CNode*>& msh_nodes (_msh->getNodeVector());

	// store node id
	std::vector<size_t> node_ids;

	const size_t n_holes (holes.size());
	for (size_t j(0); j<msh_nodes.size(); j++) {
		GEOLIB::Point pnt (msh_nodes[j]->X(), msh_nodes[j]->Y(), 0);
		if (bounding_polygon.isPntInPolygon(pnt)) {
			bool is_not_in_hole (true);

			for (size_t k(0); k<n_holes && is_not_in_hole; k++) {
				if ((holes[k])->isPntInPolygon(pnt))
					is_not_in_hole = false;
			}

			if (is_not_in_hole)
				node_ids.push_back (j);
		}
	}
	std::sort (node_ids.begin(), node_ids.end());

	size_t n_nodes (node_ids.size());
	std::vector<double> areas (n_nodes, 0.0);
	// in order to compute the area we need the mesh elements
	const std::vector<MeshLib::CElem*>& msh_elem (_msh->getElementVector());
	for (size_t k(0); k<n_nodes; k++) {
		// get all associated mesh elements
		std::vector<size_t> const & mesh_elem_ids(msh_nodes[node_ids[k]]->getConnectedElementIDs());
		size_t n_mesh_elem_ids (mesh_elem_ids.size());
		// get areas for mesh elements
		double area (0.0);
		for (size_t j(0); j<n_mesh_elem_ids; j++) {
			// check if all mesh nodes of the element are inside the polygon
			std::vector<size_t> mesh_element_node_indices;
			msh_elem[mesh_elem_ids[j]]->getNodeIndices (mesh_element_node_indices);
			size_t n_of_nodes_of_element (mesh_element_node_indices.size());
			bool found (true);
			for (size_t i(0); i<n_of_nodes_of_element && found; i++) {
				std::vector<size_t>::iterator it(std::find (node_ids.begin(), node_ids.end(), mesh_element_node_indices[i]));
				if (it == node_ids.end())
					found = false;
			}
			if (found)
				area += msh_elem[mesh_elem_ids[j]]->calcVolume();
		}
		areas[k] = area/3.0;
	}

	// write ids and areas
	for (size_t k(0); k<n_nodes; k++) {
		if (areas[k] > std::numeric_limits<double>::epsilon())
			os << node_ids[k] << " " << areas[k] << std::endl;
	}

	n_nodes = 0;
	gli_out.precision (14);
	for (std::vector<size_t>::const_iterator it (node_ids.begin()); it != node_ids.end(); it++) {
		gli_out << n_nodes + _gli_pnt_offset << " " << std::scientific
			<< msh_nodes[*it]->X() << " " << msh_nodes[*it]->Y()<< " " << msh_nodes[*it]->Z()
			<< " $NAME " << *it << std::endl;
		n_nodes++;
	}
	_gli_pnt_offset += n_nodes;
}


void ExtractMeshNodes::writeNearestMeshNodeToPoint (std::ostream& os, std::ostream& gli_out, GEOLIB::Point const & pnt)
{
	size_t node_id (_msh->GetNODOnPNT (&pnt));
	os << node_id << std::endl;

	gli_out.precision (14);
	gli_out << _gli_pnt_offset << " " << std::scientific
		<< (_msh->getNodeVector()[node_id])->X() << " "
		<< (_msh->getNodeVector()[node_id])->Y() << " "
		<< (_msh->getNodeVector()[node_id])->Z() << std::endl;

	_gli_pnt_offset++;
}

} // end namespace MeshLib
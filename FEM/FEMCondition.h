/**
 * \file FEMCondition.h
 * 25/11/2010 KR inital implementation
 *
 */

#ifndef FEMCONDITION_H
#define FEMCONDITION_H

#include "GeoInfo.h"
#include "ProcessInfo.h"
#include "DistributionInfo.h"
#include "GeoObject.h"


#include <vector>

/** 
 * \brief Adapter class for handling FEM Conditions in the user Interface
 */
class FEMCondition : public GeoInfo, public ProcessInfo, public DistributionInfo
{
public:
// Specifier for types of FEM Conditions
	enum CondType {
		UNSPECIFIED        = 0,
		BOUNDARY_CONDITION = 1,
		INITIAL_CONDITION  = 2,
		SOURCE_TERM        = 3
	};

	FEMCondition(CondType = UNSPECIFIED);
	~FEMCondition() {};

	CondType getCondType() { return _type; };

	const std::vector<double> getDisValue() { return _disValue; };
	const std::string& getGeoName() { return _geoName; };

	void setDisValue(std::vector<double> disValue) { for (size_t i=0; i<disValue.size(); i++) _disValue.push_back(disValue[i]); };
	void setDisValue(double disValue) { _disValue.push_back(disValue); };
	void setGeoName(const std::string geoName) { _geoName = geoName; };

private:
	CondType _type;
	GEOLIB::GeoObject* _geoObject;
	std::vector<double> _disValue;
	std::string _geoName;
};

/** 
 * \brief Adapter class for handling Boundary Conditions in the user Interface
 */
class BoundaryCondition : public FEMCondition
{
public:
	BoundaryCondition() : _tim_type(0) {};
	~BoundaryCondition() {};

	size_t getTimType() {return _tim_type; };
	void setTimType(size_t value) { _tim_type = value; };


private:
	size_t _tim_type;
};

/** 
 * \brief Adapter class for handling Initial Conditions in the user Interface
 */
class InitialCondition : public FEMCondition
{
public:
	InitialCondition() {};
	~InitialCondition() {};
};

/** 
 * \brief Adapter class for handling Source Terms in the user Interface
 */
class SourceTerm : public FEMCondition
{
public:
	SourceTerm() : _tim_type(0) {};
	~SourceTerm() {};

	size_t getTimType() {return _tim_type; };
	void setTimType(size_t value) { _tim_type = value; };

private:
	size_t _tim_type;
};


#endif //FEMCONDITION_H
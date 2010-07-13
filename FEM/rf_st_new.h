/**************************************************************************
FEMLib - Object: Source Terms ST
Task: class implementation
Programing:
01/2003 OK Implementation
last modified
**************************************************************************/
#ifndef rf_st_new_INC
#define rf_st_new_INC

#include "Configure.h"

// C++ STL
#include <list>
#include <fstream>
#include <string>
#include <vector>

// GEO
#include "GeoType.h"

#include "rf_pcs.h" // TF
#include "GeoInfo.h" // TF

//#include "gs_project.h" // TF
#include "rf_node.h"
#include "geo_ply.h"

typedef struct {
  std::vector<double> value_reference;
  std::vector<double> last_source_value;
  //double value_store[10][5000];
  double** value_store;//[PCS_NUMBER_MAX*2]First ref no processes(two fields per process..time, value), second ref no values
} NODE_HISTORY;

//========================================================================
class CSourceTerm : public GeoInfo
{
    std::string fname;
public:
    int CurveIndex;
    std::vector<int> PointsHaveDistribedBC;
    std::vector<double> DistribedBC;
    std::vector<double> DistBC_KRiverBed;
    std::vector<double> DistBC_WRiverBed;
    std::vector<double> DistBC_TRiverBed;
    std::vector<double> DistBC_BRiverBed;
    std::vector<double> node_value_vectorA;
    std::vector<double> node_value_vectorB;
    std::vector<double> node_value_vectorC;
    std::vector<double> node_value_vectorD;
    std::vector<double> node_value_vectorE;
    std::vector<double> node_value_vectorArea;
    std::vector<double*> normal2surface;
    std::vector<double*> pnt_parameter_vector;
    std::vector<int> element_st_vector;
private:
    CGLPolyline *plyST;
    friend class CSourceTermGroup;
public:
    std::string pcs_pv_name;
    std::string pcs_type_name;
    CRFProcess *m_pcs;
    int pcs_number;
    std::string pcs_type_name_cond;
    std::string pcs_pv_name_cond;
    double coup_leakance, rill_height;
    double sorptivity, constant, rainfall, rainfall_duration, moistureDeficit;
    bool conditional, node_averaging, no_surface_water_pressure;
    bool river;
    bool COUPLING_SWITCH;
    double normaldepth_slope;
    bool critical_depth, channel;
    double channel_width;
    std::string geo_prop_name;
    int geo_node_number;
    double geo_node_value;
    std::string geo_type_name;
    int msh_node_number;

    /**
     * REMOVE CANDIDATE
     * @return
     */
    const std::string & getGeoName();

private:
    std::string geo_name;

public:
    std::string dis_type_name;
    int component;
    int dis_type;
    int fct_method;
    double dis_prop[3];
    double *nodes;
    std::vector<int> node_number_vector;
    std::vector<double> node_value_vector;
    std::vector<int> node_renumber_vector;
    std::string fct_name;
    int curve;
    std::string tim_type_name;
    bool analytical;
    int analytical_material_group;
    int resolution;
    double st_area;
    double analytical_diffusion;
    size_t number_of_terms;
    int no_an_sol;
    int max_no_terms;
    double factor;
    double analytical_porosity;
    double analytical_tortousity;
    double analytical_linear_sorption_Kd;
    double analytical_matrix_density;
    std::string msh_type_name;

    CSourceTerm(void);
    ~CSourceTerm(void);
    ios::pos_type Read(std::ifstream *in, const GEOLIB::GEOObjects & geo_obj, const std::string & unique_name);
    void Write(fstream*);
    void SetDISType(void);
    void SetGEOType(void);
    void EdgeIntegration(CFEMesh *m_msh, std::vector<long> & nodes_on_ply, std::vector<double> & node_value_vector);
    void FaceIntegration(CFEMesh *m_msh, std::vector<long> & nodes_on_sfc, std::vector<double> & node_value_vector);
    void DomainIntegration(CFEMesh *m_msh, std::vector<long> & nodes_in_dom, std::vector<double> & node_value_vector);
    void SetPolyline(CGLPolyline *Polyline)
    {
        plyST = Polyline;
    }

    inline void DirectAssign(long ShiftInNodeVector);
    void SetNOD2MSHNOD(std::vector<long> & nodes, std::vector<long> & conditional_nodes);
    double GetNodeLastValue(long n, int idx);
    void SetNodePastValue(long n, int idx, int pos, double value);
    void SetNodeLastValue(long n, int idx, double value);
    double GetNodePastValue(long, int, int);
    double GetNodePastValueReference(long n, int idx);
    void CreateHistoryNodeMemory(NODE_HISTORY *nh);
    void DeleteHistoryNodeMemory();
    void SetSurfaceNodeVectorConditional(std::vector<long> & sfc_nod_vector, std::vector<long> & sfc_nod_vector_cond);
    void SetPolylineRiverNodeValueVectors(CGLPolyline *m_ply, int number_of_nodes);
    void InterpolatePolylineRiverNodeValueVector(CGLPolyline *m_ply, std::vector<double> & Distribed, std::vector<double> & ply_nod_vector);
    void SetNodeValues(std::vector<long> & nodes, std::vector<long> & nodes_cond, std::vector<double> & node_values, const int ShiftInNodeVector);
    void SetNOD();
    ios::pos_type ReadDistributionType(ifstream *st_file);
    ios::pos_type ReadGeoType(std::ifstream *st_file, const GEOLIB::GEOObjects& geo_obj, const std::string& unique_name);
};

//========================================================================
class CSourceTermGroup
{
  public:
    CSourceTermGroup() {} //WW
    CSourceTermGroup* Get(std::string);
    void Set(CRFProcess* m_pcs, const int ShiftInNodeVector, std::string this_pv_name="");
//WW    std::vector<CNodeValue*>group_vector;
    /**
     * \brief process type for the physical process
     * possible values are
     * <table>
     * <tr><td>LIQUID_FLOW</td> <td>H process (incompressible flow)</td></tr>
     * <tr><td>GROUNDWATER_FLOW</td> <td>H process (incompressible flow)</td></tr>
     * <tr><td>RIVER_FLOW</td> <td>H process (incompressible flow)</td></tr>
     * <tr><td>RICHARDS_FLOW</td> <td>H process (incompressible flow)</td></tr>
     * <tr><td>OVERLAND_FLOW</td> <td>process (incompressible flow)</td></tr>
     * <tr><td>GAS_FLOW</td> <td>H process (compressible flow)</td></tr>
     * <tr><td>TWO_PHASE_FLOW</td> <td>H2 process (incompressible/compressible flow)</td></tr>
     * <tr><td>COMPONENTAL_FLOW</td> <td>H2 process (incompressible/compressible flow)</td></tr>
     * <tr><td>HEAT_TRANSPORT</td> <td>T process (single/multi-phase flow)</td></tr>
     * <tr><td>DEFORMATION</td> <td>M process (single/multi-phase flow)</td></tr>
     * <tr><td>MASS_TRANSPORT</td> <td>C process (single/multi-phase flow)</td></tr>
     * </table>
     */
    std::string pcs_name;
    std::string pcs_type_name; //OK
    std::string pcs_pv_name; //OK
    CFEMesh* m_msh;
    CFEMesh* m_msh_cond;
//WW    std::vector<CSourceTerm*>st_group_vector; //OK
    //WW double GetConditionalNODValue(int,CSourceTerm*); //OK
    //WW double GetRiverNODValue(int,CSourceTerm*, long msh_node); //MB
    //WW double GetCriticalDepthNODValue(int,CSourceTerm*, long msh_node); //MB
    //WW double GetNormalDepthNODValue(int,CSourceTerm*, long msh_node); //MB JOD
	 //WW Changed from the above
//    double GetAnalyticalSolution(CSourceTerm *m_st,long node_number, std::string process);//CMCD
    // TRANSFER OF DUAL RICHARDS
    std::string fct_name; //YD
  private:
	void SetPNT(CRFProcess* m_pcs, CSourceTerm* m_st, const int ShiftInNodeVector); // JOD
	void SetLIN(CRFProcess* m_pcs, CSourceTerm* m_st, const int ShiftInNodeVector);  // JOD
    void SetPLY(CSourceTerm* m_st, const int ShiftInNodeVector); //OK
	void SetDMN(CSourceTerm* m_st, const int ShiftInNodeVector);  // JOD
	void SetSFC(CSourceTerm* m_st, const int ShiftInNodeVector);  // JOD
	void SetCOL(CSourceTerm *m_st, const int ShiftInNodeVector); // JOD
	void SetPolylineNodeVector(CGLPolyline* m_ply, std::vector<long>&ply_nod_vector); // JOD
	void SetPolylineNodeVectorConditional(CSourceTerm* m_st, CGLPolyline* m_ply,
		std::vector<long>&ply_nod_vector, std::vector<long>&ply_nod_vector_cond);
    void SetPolylineNodeValueVector(CSourceTerm* m_st, CGLPolyline* m_ply, std::vector<long>&ply_nod_vector, std::vector<long>&ply_nod_vector_cond
										  , std::vector<double>&ply_nod_val_vector);
    void SetSurfaceNodeVector(Surface* m_sfc, std::vector<long>&sfc_nod_vector); // JOD
	void SetSurfaceNodeValueVector( CSourceTerm* m_st, Surface* m_sfc, std::vector<long>&sfc_nod_vector, std::vector<double>&sfc_nod_val_vector);
    void AreaAssembly(CSourceTerm* m_st,  std::vector<long>&ply_nod_vector_cond,
									std::vector<double>& ply_nod_val_vector);
};

extern CSourceTermGroup* STGetGroup(std::string pcs_type_name,std::string pcs_pv_name);
extern list<CSourceTermGroup*> st_group_list;

/**
 * read source term file
 * @param file_base_name base file name (without extension) containing the source terms
 * @param geo_obj object of class GEOObjects managing the geometric entities
 * @param unique_name unique name to access the geometric entities in geo_obj
 * @return true if source terms found in file, else false
 */
bool STRead(const std::string& file_base_name, const GEOLIB::GEOObjects& geo_obj, const std::string& unique_name);

extern void STWrite(std::string);
#define ST_FILE_EXTENSION ".st"
extern void STCreateFromPNT();
extern std::vector<CSourceTerm*> st_vector;
extern void STDelete();
void STCreateFromLIN(std::vector<CGLLine*>);
CSourceTerm* STGet(std::string);
extern void STGroupDelete(std::string pcs_type_name,std::string pcs_pv_name);
extern void STGroupsDelete(void);//Haibing;
extern long aktueller_zeitschritt;
extern double aktuelle_zeit;
extern std::vector<std::string>analytical_processes;
extern CSourceTerm* STGet(const std::string&, const std::string&, const std::string&); //OK

// WW moved here
extern  double GetAnalyticalSolution(long node_number, CSourceTerm *m_st);//CMCD, WW
extern  void GetRiverNODValue(double& value, CNodeValue* cnodev,CSourceTerm* m_st);
extern	double GetConditionalNODValue(CSourceTerm* m_st, CNodeValue* cnodev);
extern  void GetCriticalDepthNODValue(double& value, CSourceTerm*, long msh_node); //MB
extern  void GetCouplingNODValue(double& value, CSourceTerm* m_st, CNodeValue* cnodev); // JOD
extern  void GetCouplingNODValueNewton(double& value, CSourceTerm* m_st, CNodeValue* cnodev); // JOD
#ifndef NEW_EQS //WW. 06.11.2008
extern  void GetNormalDepthNODValue(double& value, CSourceTerm*, long msh_node); //MB JOD
extern  void GetCouplingNODValuePicard(double& value, CSourceTerm* m_st, CNodeValue* cnodev); // JOD
#endif
extern  double CalcCouplingValue(double factor, double h_this, double h_cond, double z_cond, CSourceTerm* m_st); // JOD
extern  void GetCouplingNODValueMixed(double& value, CSourceTerm* m_st, CNodeValue* cnodev); // JOD
extern  void GetCouplingFieldVariables(double* h_this, double* h_cond, double* h_shifted, double* h_averaged, double* z_this, double* z_cond, CSourceTerm* m_st, CNodeValue* cnodev);// JOD
extern  double GetRelativeCouplingPermeability(CRFProcess* m_pcs, double head, CSourceTerm* m_st, long msh_node); // JOD
extern  void GetPhilipNODValue(double& value,CSourceTerm* m_st); // JOD
extern  void GetGreenAmptNODValue(double& value, CSourceTerm* m_st, long msh_node); // JOD
extern  void GetNODValue(double& value, CNodeValue* cnodev,CSourceTerm* m_st); // JOD
#endif

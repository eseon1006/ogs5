/**************************************************************************
FEMLib - Object: MAT-SP
Task: class implementation
Programing:
08/2004 WW Implementation
last modified:
**************************************************************************/
#ifndef rf_msp_new_INC
#define rf_msp_new_INC

// C++ STL
#include <fstream>
#include <string>
#include <vector>

#define MSP_FILE_EXTENSION ".msp"

using namespace std;

namespace FiniteElement{class element; class CFiniteElementVec; 
                         class CFiniteElementStd; class ElementValue_DM;} 
namespace Math_Group {class Matrix;} 
namespace process{class CRFProcessDeformation;}

#if defined(WIN32)
class CMATGroupEditorDataEdit; //WW
#endif
namespace SolidProp{

  using FiniteElement::CFiniteElementVec;
  using FiniteElement::ElementValue_DM;
  using FiniteElement::CFiniteElementStd;
  using FiniteElement::ElementValue;
  using Math_Group::Matrix;
  using process::CRFProcessDeformation;
  using ::CRFProcess;
/*---------------------------------------------------------------*/
class CSolidProperties
{
  private:
    // Material parameters
    double PoissonRatio;
    int Youngs_mode;
    Matrix *data_Youngs;
    double ThermalExpansion;
    //
    double s_tol;   //16.06.2008 WW
    double f_tol;   //16.06.2008 WW
    double biot_const;
    double grav_const; //WW
    Matrix *data_Density;
    // 
    Matrix *data_Capacity;
    Matrix *data_Conductivity;
    // 
    Matrix *data_Plasticity;
    Matrix *data_Creep;
    //
    int Density_mode;
    //
    int Capacity_mode;
    int Conductivity_mode;
    int Plasticity_type;
    double primary_variable[10]; //CMCD
    double primary_variable_t0[10];//CMCD
    double primary_variable_t1[10];//CMCD
    // Creep property
    // 1. Stationary Norton model
    int Creep_mode;
    //
	bool axisymmetry;     
    
    int mode;//CMCD
	// Swelling pressure
	int SwellingPressureType; 
	double Max_SwellingPressure; 
    //
    string CurveVariable_Conductivity;
    int CurveVariableType_Conductivity;
    // Secondary data 
    // Elasticity
    double E;      // Youngs moduls calculated from data_Youngs
    double Lambda;
    double G;      // Shear stress modulus
    double K;      // Bulk modulus
    // Plasticity
    double dl2; 
    // 2. Single yield surface
    Matrix *d2G_dSdS;     
    Matrix *d2G_dSdM;
    Matrix *LocalJacobi;  // To store local Jacobi matrix 
    Matrix *inv_Jac;      // To store the inverse of the  Jacobi matrix 
    Matrix *sumA_Matrix;  
    double *rhs_l;        // To store local unknowns of 15 
    double *x_l;          // To store local unknowns of 15 
    int  *Li;              
    void AllocateMemoryforSYS();
    void ResizeMatricesSYS(const int Dim);

    // Direct stress integration for Drucker-Prager
    double *devS;
    double *dFds;
    double *dGds;
    double *D_dFds;
    double *D_dGds;
    // Mini linear solver
    void Gauss_Elimination(const int DimE, Matrix& AA, int *L,  double *xx); 
    void Gauss_Back(const int DimE, Matrix& AA, double * rhs, int *L, double *xx); 
    // Thermal properties
    int thermal_conductivity_tensor_type;
    double thermal_conductivity_tensor[9];
    string thermal_conductivity_tensor_type_name;
    // Handles. May be used by GUI
    string solid_name;
    //-------------------------------------------------------------
    // Numeric
    double CalulateValue(const Matrix *data, const double x) const;
    double Kronecker(const int ii, const int jj);

    // Friends that can access to this data explicitly
    friend bool MSPRead(string file_base_name);
    friend void MSPWrite(string);    
    //WW
    friend class FiniteElement::CFiniteElementVec;
    friend class FiniteElement::CFiniteElementStd;
    friend class FiniteElement::ElementValue;
    friend class process::CRFProcessDeformation;
    friend class ::CRFProcess;
#if defined(WIN32)  //15.03.2008 WW
    friend class ::CMATGroupEditorDataEdit;
#endif
    //WW
  public:    
    //
    CSolidProperties();
    ~CSolidProperties();
    ios::pos_type Read(ifstream*);
    FiniteElement::CFiniteElementStd *Fem_Ele_Std;//CMCD
    string name;
    // IO
    string file_base_name;
    // Output
    void Write(fstream*);
    void CalPrimaryVariable(vector<string>& pcs_name_vector);//CMCD
    
    //-------------------------------------------------------------
    // Access to data
    //-------------------------------------------------------------
    // 1. Density
    double Density(double refence = 0.0);
    // 2. Thermal
    double Heat_Capacity(double refence = 0.0);
    // Boiling model
    double Heat_Capacity(double temperature, double porosity, double Sat);
	int GetCapacityModel() const {return Capacity_mode;}
	bool CheckTemperature_in_PhaseChange(const double T0, const double T1);
    double Enthalpy(double temperature, const double latent_factor );
    double Heat_Conductivity(double refence = 0.0);
    void HeatConductivityTensor(const int dim, double* tensor, int group);
	//   int GetCapacityMode() {return Capacity_mode;};  ??
    // 3. Elasticity
	#ifdef RFW_FRACTURE
      double Youngs_Modulus(CElem* elem, double refence = 0.0);
      double Get_Youngs_Min_Aperture(CElem *elem); //RFW, for fracture calc
    #endif
    #ifndef RFW_FRACTURE
      double Youngs_Modulus(double refence = 0.0); 
    #endif 
    double Poisson_Ratio() const {return PoissonRatio;}
    double Thermal_Expansion() const {return ThermalExpansion;}
    // 4. Plasticity
    int Plastictity() const {return Plasticity_type;}
    double GetPlasticParameter(const int index) {return (*data_Plasticity)(index);} 
    // 5. Creep 
    int CreepModel() const {return Creep_mode;}
    double GetCreepParameter(const int index) {return (*data_Creep)(index);} 
    // Initilize density
    void NullDensity(); 

    
    //-------------------------------------------------------------
    // Manipulators of data
    //-------------------------------------------------------------
    // 1. Elasticity
    #ifndef RFW_FRACTURE
    void Calculate_Lame_Constant();
    #endif
    #ifdef RFW_FRACTURE
    void  Calculate_Lame_Constant(CElem* elem);
    #endif
    // For thermal elastic model
    void ElasticConsitutive(const int Dimension, Matrix *D_e) const;
    // 2. Plasticity
    // 2.1 Drucker-Prager
    double GetAngleCoefficent_DP(const double Angle);
    double GetYieldCoefficent_DP(const double Angle);
    void CalulateCoefficent_DP();
    bool StressIntegrationDP(const int GPiGPj, const ElementValue_DM *ele_val, 
           double *TryStress, double& dPhi, const int Update);
    void ConsistentTangentialDP(Matrix *Dep, const double dPhi, const int Dim);
    bool DirectStressIntegrationDP(const int GPiGPj, 
            const ElementValue_DM *ele_val, double *TryStress, const int Update);
    void TangentialDP(Matrix *Dep);
    // 2.2 Single yield surface model
    void dF_dNStress(double *dFdS, const double *DevS, const double *S_Invariants, 
                     const double *MatN1, const int LengthStrs);
    void dF_dStress(double *dFdS, const double *RotV, const double *S_Invariants,
                     const double *MatN1, const int LengthStrs);
    void dF_dMat(double *dFdM, const double *S_Invariants, const double *MatN1);
    void dG_dNStress(double *dGdS, const double *DevS, const double *S_Invariants, 
                     const double *MatN1, const int LengthStrs);
    void dG__dNStress_dNStress(const double *DevS, const double *S_Invariants, 
                               const double *MatN1, const int LengthStrs );
    void dG__dStress_dStress(const double *DevS, const double *RotV, 
                             const double *S_Invariants, const double *MatN1,
                             const int LengthStrs);
    void dG_dSTress_dMat(const double *DevS, const double *S_Invariants, 
                         const double *MatN1, const int LengthStrs);
    void dfun2(const double *DevS, const double *RotV, const double *S_Invariants, 
               const double *MatN1, const int LengthStrs);


    int CalStress_and_TangentialMatrix_SYS(const int GPiGPj, const ElementValue_DM *ele_val, 
                const Matrix *De,  Matrix *D_ep, double *dStress,  
                const int Update);
    // 2.2 Cam-clay model
    void CalStress_and_TangentialMatrix_CC(const int GPiGPj,
          const ElementValue_DM *ele_val, double *dStrain,  Matrix *Dep, const int Update);
    // Substep integration. 16.05.2008 WW  
    void CalStress_and_TangentialMatrix_CC_SubStep(const int GPiGPj,
          const ElementValue_DM *ele_val, double *dStrain,  Matrix *Dep, const int Update);
    // Parameter function for thermal elatic model. Last modifed on 15.03.2008 //WW
    double TEPSwellingParameter(const double mean_stress); 
    void TEPSwellingParameter_kis(const double suction); 
    // Strain inrement by creep
    void AddStain_by_Creep(const int ns, double *stress_n, double *dstrain, double temperature=0.0);

    // Plasticity
    // 1. Drucker-Prager
    double Al; 
    double Xi;
    double Y0; 
    double BetaN;
    double Hard;
    double Hard_Loc;

    vector<string>  capacity_pcs_name_vector; //WW
    vector<string>  conductivity_pcs_name_vector; //WW  


};

}// end namespace
extern vector<SolidProp::CSolidProperties*> msp_vector;
extern bool MSPRead(string file_base_name);
extern void MSPWrite(string);
extern void MSPDelete();
extern vector<string> msp_key_word_vector; //OK
extern void MSPStandardKeywords(); //OK
extern SolidProp::CSolidProperties* MSPGet(string); //OK

extern double StressNorm(const double *s, const int Dim);
extern double TensorMutiplication2(const double *s1, const double *s2, const int Dim);
extern double TensorMutiplication3(const double *s1, const double *s2, const double *s3, const int Dim);
extern double DeviatoricStress(double *Stress);

#endif

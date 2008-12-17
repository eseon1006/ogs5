/**************************************************************************/
/* ROCKFLOW - Modul: rf_apl
                                                                          */
/* Aufgabe:
   ROCKFLOW-Applikation
                                                                          */
/* Programmaenderungen:
   12/1999   OK           aus calc.c abgeleitet
   02/2000   C.Thorenz    Ausgabe konfigurieren
   --/1999   AH           Listen und zugehoerige Objekte:
                          rfoutput.h, rfsolver.h, rfiter.h, rfinitc.h,
                          rfbc.h, rfsousin.h, rfpriref.h rfsystim.h
                          Uebernahme von rfmat.h in material.h.
   10/2001   AH           Neues Modell-Konzept fuer Inverses Modellieren.
                          Neue Listen und zugehoerige Objekte: 
                          rfdb.h, rfidb.h, rfmodel.h, rfod.h,
                          rfiv.h, rfii.h, rfim.h
   11/2002   OK           polyline list
   02/2003   RK           gesaeubert
   03/2003   RK           Liste fuer Rockflow-Modelle in Version rf3818 entfernt
   05/2003 OK Process list
   05/2003 OK GEOConfigELEGeometry -> ELEConfigJacobianMatrix
                                                                          */
/**************************************************************************/

#include "stdafx.h" /* MFC */
// C++
#include <iostream>
#include "makros.h"
#define TEST_RF_APL
#include "renumber.h"
/* Object Lists */
#include "rf_out_new.h"
#include "rfiter.h"   /* Construction: Iteration process list */
#include "tools.h"    /* Construction: Functions data */
#include "nodes.h"    /* Construction: Node array */
#include "elements.h" /* Construction: Element array */
#include "rf_pcs.h" //OK_MOD"  /* Construction: Model list */
#include "rf_tim_new.h"
#include "rfmat_cp.h"

#ifdef PCS_OBJECTS
#include "rf_pcs.h"   /* Construction: process list */
#include "rf_react.h"//SB
#endif

/* Objects */
#include "files.h"    /* Construction: ReadData - from file */
#include "solver.h"   /* Renumber */
#include "femlib.h"   /* CalcElementsGeometry */
#include "loop_pcs.h"

/* This */
#include "rf_apl.h"

/* Tools */
#include "break.h"
#include "tools.h"

#include "par_ddc.h"
#ifdef CHEMAPP
  #include "eqlink.h"
#endif
#if defined(USE_MPI) //12.09.2007 WW
#include <mpi.h>
clock_t elapsed_time_preloop ; 
#endif


/* Interne (statische) Deklarationen */
void RFPost_FEM(void);
void RFPost_Model(void);
int ExecuteRFTimeLoop(void);
  /* Prototypen fuer Zeitschleifen */
IntFuncVoid PreTimeLoop;
IntFuncVoid PostTimeLoop;

int mshType=0;  //WW To be removed
 
/* Definitionen */


/**************************************************************************/
/* ROCKFLOW - Funktion: RF_FEM
                                                                          */
/* Aufgabe:
   FEM Applikation - Rockflow
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E char *dateiname: Dateiname der Eingabedatei ( ohne Extension )
                                                                          */
/* Ergebnis:
   0 bei Fehler, sonst 1 (dient nur zum Abbrechen der Funktion)
                                                                          */
/* Programmaenderungen:
   07/1996   MSR  Erste Version (RF-ADT)
   08/1999   OK   RF-FEM Applikation
   02/2000   RK   2D 9-Knoten-Element generieren
   10/2001   AH   Inverse Modellierung
                  Neues Konzept fuer Rockflow-Modelle

   04/2002   OK   erst Objekt-Konfiguration (RFConfigObjects) 
                  dann Modell-Konfiguration (SetRockflowModelEx)
   08/2002   MK   nein, Objekt-Konfiguration (RFConfigObjects) 
                  basiert auf Modell-Konfiguration (SetRockflowModelEx)
                  z. B. bei Ausgabe-Konfiguration (ConfigOutput())
   09/2002   OK   Kapselung von Pre/Post-Funktionen 
                  für FEM Applikation und Modelle
                  (dies vereinfacht die Einbettung von RF-FEM in andere Applikationen)
   09/2002   OK   ToDo: Trennung zwischen modellabhängigen und -unabhängigen
                        Objekten notwendig
   03/2003   RK   Liste fuer Rockflow-Modelle in Version rf3818 entfernt                     
                                                                          */
/**************************************************************************/
int RF_FEM (char *dateiname)
{
    /*--------------------------------------------------------------------*/
    /* 1 Objekte und Verknüpfungen für RF-FEM Applikation konstruieren */
    RFPre_FEM(dateiname);
    /*--------------------------------------------------------------------*/
      /* 2.1 RF-Objekte konfigurieren, Verknüpfungen zwischen RF-Objekten */
      RFPre_Model();
      /* 2.2 Zeitschleife */ 
      //OK-INV inv_flag=ExecuteInverseMethodNew ("ROCKFLOW",ExecuteRFTimeLoop);
#if defined(USE_MPI) //WW
      elapsed_time_preloop += clock();
      //
      cout<<" CPU Time elaspsed before time loop: "
          <<(double)elapsed_time_preloop/CLOCKS_PER_SEC<<"\n"; 
#endif
      ExecuteRFTimeLoop(); //OK
#if defined(USE_MPI) //WW
      // count CPU time after time loop
      elapsed_time_preloop = -clock();
#endif 
      /* 2.3 RF-Objekte destruieren */
      RFPost_Model();
    /*--------------------------------------------------------------------*/
    /* 3 Objekte der RF-FEM Applikation destruieren */
    RFPost_FEM();
#if defined(USE_MPI)
    elapsed_time_preloop += clock(); 
    //
    cout<<" CPU Time elaspsed after time loop: "
      <<(double)elapsed_time_preloop/CLOCKS_PER_SEC<<"\n"; 
#endif
    return 1;
}


/**************************************************************************/
/* ROCKFLOW - Funktion: RFPre_FEM
                                                                          */
/* Aufgabe:
   RF Objekte und Verknüpfungen konstruieren
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E char *dateiname: Dateiname der Eingabedatei ( ohne Extension )
                                                                          */
/* Ergebnis:
   0 bei Fehler, sonst 1 (dient nur zum Abbrechen der Funktion)
                                                                          */
/* Programmaenderungen:
   09/2002   OK   Kapselung
   07/2004 OK PCSRead
   08/2005 WW MSH Read                                                                       */
/**************************************************************************/
int RFPre_FEM(char* dateiname)
{
  int error_rfd=0;
  int error_rfi=0;
  /*-----------------------------------------------------------------------*/
  /* Initialisierungen und Konstruktionen vor Einlesen */
   /* Initialisierung 'globaler' Variablen - ToDo */
   /* Adaptation */
   /* Datenstruktur fuer Dateizugriff */
//OK  CreateFileData(dateiname);
  /*-----------------------------------------------------------------------*/
  /* RF-Objekte aus RFD- und RFI-Eingabedateien erzeugen.                  */
  /* Dateien fuer Inverses Modellieren lesen und auswerten falls vorhanden.*/
  mshType = ReadData(dateiname); //WW
  if (mshType==0) {
    error_rfd = 1;
    error_rfi = 1;
    return 0;
  }
  //......................................................................
  DOMRead(dateiname);
  #ifdef RFW_FRACTURE
  for(int i=0; i<(int)mmp_vector.size(); ++i)
    {
      if( mmp_vector[i]->frac_num >0)
      {
        MSHSetFractureElements();
      }
    }
  #endif
  if(mshType==100) return 1;
  /*-----------------------------------------------------------------------*/
  /* RF-Objekte konfigurieren, Verknüpfungen zwischen RF-Objekten */
  cout << "---------------------------------------------" << endl;
  cout << "Data configuration" << endl;
//OK_OUT  ConfigFileData(dateiname);
  ConfigRFFiles(dateiname);
  cout << "->Topology" << endl;
  //OK MSHTopology(); //OK
  ConfigTopology(); /* Fehler beim mehrmaligen Einlesen OK 06.11.1999 */
                      /* Reihenfolge: vor Renumber */
  ELEConfigJacobianMatrix();
  ELECalcEleMidPoint(); //SB 

  return 1;
}


/**************************************************************************/
/* ROCKFLOW - Funktion: RFPost_FEM
                                                                          */
/* Aufgabe:
   RF Objekte destruieren
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
                                                                          */
/* Ergebnis:
                                                                          */
/* Programmaenderungen:
   09/2002   OK   Kapselung
                                                                          */
/**************************************************************************/
void RFPost_FEM(void)
{
//OK  DestroyAdaptationData();
//OK  DestroyFileData();
//OK  DestroyFiles();
  pcs_vector.clear();
}
/**************************************************************************/
/* ROCKFLOW - Funktion: RFPre_Model
                                                                          */
/* Aufgabe:
   RF-Objekte konfigurieren, Verknüpfungen zwischen RF-Objekten
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E char *dateiname: Dateiname der Eingabedatei
   E ROCKFLOW_MODEL_PROPERTIES *rfmp: Model
                                                                          */
/* Ergebnis:
   void
                                                                          */
/* Programmaenderungen:
   09/2002   OK   Kapselung
   03/2003   RK   Liste fuer Rockflow-Modelle in Version rf3818 entfernt
   11/2004 OK MOD stuff removed
02/2005 MB  ComputeNeighborsofElement                                     */
/**************************************************************************/
void RFPre_Model()
{
  // Set LOP function pointer
  LOPConfig_PCS();
  //
//OK  if(mshType!=100&&mshType!=0) InitGridAdaptation();   
  /* RF-Objekte konfigurieren, Verknüpfungen zwischen RF-Objekten */
  RFConfigObjects();
  /* RF-Objekte konfigurieren */
  if(mshType!=100&&mshType!=0) RFConfigRenumber();
  /* Bilanzobjekte initialisieren, Files oeffnen, Header schreiben */
//OK4026  BalanceInitAllObjects(dateiname);
  /* Inverse-Objekte konfigurieren */
//OK4026  ConfigInverseMethod();
  /* Datenbank oeffnen */
//OK4026  OpenDataBase("INVERSE-ROCKFLOW");
  //OK_MOD
}


/**************************************************************************/
/* ROCKFLOW - Funktion: RFPost_Models
                                                                          */
/* Aufgabe:
   RF-Objekte destruieren
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E ROCKFLOW_MODEL_PROPERTIES *rfmp: Model
                                                                          */
/* Ergebnis:
   void
                                                                          */
/* Programmaenderungen:
   09/2002   OK   Kapselung
                                                                          */
/**************************************************************************/
void RFPost_Model(void)
{
  /* Datenbank schliessen */
//OK  CloseDataBase("INVERSE-ROCKFLOW");
  /* Objekte destruieren */
  DestroyRFObjects();
}

/**************************************************************************
FEMLib-Method: ExecuteRFTimeLoop
Task: Time loop execution
Programing:
07/1996 MSR Erste Version (RF-ADT)
08/1999 OK  for RF-FEM Applikation
12/1999 CT  Korrigiert: Uebergabe der Variablen an OutputTimeStepResults als Zeiger
10/2001 AH  Inverses Modellieren   
08/2004 OK TIM objects
06/2005 OK Write IC
08/2005 MB GetHeterogeneousFields to loop_pcs
**************************************************************************/
int ExecuteRFTimeLoop(void)
{
  bool heat_flag = false;
  int i = 0, j;
  double ct = 0.0;
  //----------------------------------------------------------------------
/*OK
  if(mshType!=100&&mshType!=0)  //WW
  {
    GetHeterogeneousFields(); //SB:todo Fallunterscheidung
    CalcElementsGeometry(); // WW moved this here This will be removed is fem_geo is ready
  }
*/
  //======================================================================
  // Creation of processes
#ifdef USE_MPI //WW
  if(myrank==0)
  {
#endif
  cout << "---------------------------------------------" << endl;
  cout << "Data configuration: PreTimeLoop" << endl;
#ifdef USE_MPI //WW
  }
#endif

  if (!PreTimeLoop())
    DisplayMsgLn("Error in ConfigModel: PreTimeLoop");
  /* modellspezifische Ergebnisse fuer Inverses Modellieren speichern */
//OK  SaveAllModelObservationData();
  //------------------------------------------------------------------------
//OK_Please put this to PCS configuration
  // Set flags for which stability processes should be checked   CMCD GEOSYS4
  CRFProcess *m_pcs = NULL;
  int no_processes = (int)pcs_vector.size();
  for(i=0;i<no_processes;i++){
    m_pcs = pcs_vector[i];
    if(m_pcs->pcs_type_name.compare("HEAT_TRANSPORT")==0)heat_flag = true;
  }
  //======================================================================
  CTimeDiscretization *m_tim = NULL;
  if(time_vector.size()>0)
    m_tim = time_vector[0];
  else{
    cout << "Error in ExecuteRFTimeLoop: no time discretization data !" << endl;
    return 0;
  }
  //----------------------------------------------------------------------
  // Write IC
  //OK OUTData(m_tim->time_current,aktueller_zeitschritt);
  //----------------------------------------------------------------------
  // SB/MX ToDo nach SetIC
 /* if(cp_vec.size()>0){ //OK//CMCD OUT FOR MASS TRANSPORT AND HEAT TRANSPORT 4213, example frac_an
	REACT *rc = NULL; //SB
    rc = new REACT;
    rc->CreateREACT();//SB
	rc->InitREACT();
    rc->ExecuteReactions();
	REACT_vec.push_back(rc);
  }*/
  //----------------------------------------------------------------------
  int no_time_steps = (int)m_tim->time_step_vector.size();
  // Output initial values. WW
#if defined(USE_MPI)  //12.09.2007 WW
  if(myrank==0) 
#endif
    OUTData(0.0,aktueller_zeitschritt); 
#if defined(USE_MPI)  //12.09.2007 WW
  if(myrank==0) 
  {
#endif
  cout << "*********************************************" << endl;
  cout << "Start of simulation" << endl;
#ifdef USE_MPI //WW
  }
#endif
  //======================================================================
  while(m_tim->time_current < m_tim->time_end) 
  {
    //----------------------------------------------------------------------
    // Recalculation  YD 06.06
    if(m_tim->time_control_name.find("ADAPTIVE")!=string::npos)
    {
      if(m_tim->repeat)
      {
        m_tim->time_current -= m_tim->time_step_length;
        aktueller_zeitschritt--; 
        aktuelle_zeit = m_tim->time_current; 
        cout << "REPEAT TIME STEP" << endl;
        for(j=0;j<no_processes;j++){
                  m_pcs = pcs_vector[j];
	          m_pcs->PrimaryVariableReload(); // kg44 should be done here, as repeat is changed before transport is executed again
        }
      }
    }
   	ct = 1.0;
    // Time step calculation
    dt = m_tim->CalcTimeStep();
     if ((m_tim->time_current + dt) > m_tim->time_end) {
                         dt= m_tim->time_end - m_tim->time_current; // kg44 make sure we hit the last point
     }
    //----------------------------------------------------------------------
     if(dt < m_tim->min_time_step){
       // cout << "TIME step is too small, calculation will be stopped" << endl; // HS/KG, changed for adaptive time step.
       cout << "TIME step is set to the smallest time step. " << endl;
       dt=m_tim->min_time_step;
     }
     m_tim->time_step_length=dt; // kg44 for ADAPTIVE
    //----------------------------------------------------------------------
	//Include stability check CMCD GEOSYS4
	if ((heat_flag)&&(aktueller_zeitschritt > 0)) {
		ct = Checkcourant_PCS();
		if (m_tim->time_control_name == "COURANT_MAX") dt = ct;
		if (m_tim->time_control_name == "COURANT_CHECK"){
			if (ct < dt) dt = ct;
		}
		if (m_tim->time_control_name == "COURANT_MANIPULATE"){
			dt = ct * m_tim->time_control_manipulate;
		}
	}
    // Timestepping for overland flow
    // ToDo MB
    m_tim->time_current += dt;
    aktueller_zeitschritt++; // ToDo
    aktuelle_zeit = m_tim->time_current; // ToDo
#if defined(USE_MPI)  //12.09.2007 WW
  if(myrank==0) 
  {
#endif
    cout << "*********************************************" << endl;
    cout << "TIME step " << m_tim->step_current+1 << ": " << m_tim->time_current << endl;
#ifdef USE_MPI //WW
  }
#endif
    //----------------------------------------------------------------------
    // Time step excution 
    //
    if(LOPTimeLoop_PCS())  // &dt_sum. WW
      OUTData(m_tim->time_current,aktueller_zeitschritt);
		//----------------------------------------------------------------------
    // Data output
    /* modellspezifische Ergebnisse fuer Inverses Modellieren speichern */
//OK    SaveAllModelObservationData();
    /* Ausgabe fuer Bilanzobjekte */
//OK    BalanceOverAllGeometryObjects();
    /* Ergebnisausgabe */
    /*
    if(IsSynCron())			//17.09.2007 WW
#if defined(USE_MPI)  //12.09.2007 WW
    {
      if(myrank==0) 
#endif
       OUTData(m_tim->time_current,aktueller_zeitschritt);
#if defined(USE_MPI)  //12.09.2007 WW
      // MPI_Barrier (MPI_COMM_WORLD); //WW
    }
#endif
    */ 
    #ifdef RFW_FRACTURE 
      for(int i=0; i<(int)mmp_vector.size(); ++i)
        {
          if( mmp_vector[i]->frac_num >0)
          {
            MSHResetFractureElements(); // cout << "MSHResetFractureElements\n";
          }
        }
    #endif
    // update current time step number
    if(m_tim->time_control_name.find("ADAPTIVE")==string::npos) //WW
    {
      if(m_tim->step_current==no_time_steps)
        break;
    }
    if(aktuelle_zeit>=m_tim->time_end)		//WW
      break;
    m_tim->step_current++; // Moved here by WW
  }
  //========================================================================
  /* modellspezifische Voreinstellungen aufheben */
  if (!PostTimeLoop())
    return 0;

  return 1;
}


/**************************************************************************/
/* ROCKFLOW - Funktion: CreateObjectLists
                                                                          */
/* Aufgabe:
   Konstruktor fuer Objekt-Listen
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   - void -
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   08/1999    OK   Implementierung
   12/1999    OK   Anpassung an rf3260
   10/2001    AH   Inverse Modellierung
                   Eingabe-Datenbank
                   Rockflow-Modell-Liste
                   Alle noetigen Aenderungen sind ausgeklammert.
   03/2003    RK   Rockflow-Modell-Liste entfernt (Version rf3818)               
   04/2003    MK   MeshGeneration-Liste      
   05/2003    OK   Process list
   11/2004 OK old lists removed
                                                                          */
/**************************************************************************/
void CreateObjectLists(void)
{
  /* Data base */
//OK  CreateDataBaseList();
//OK  CreateInputDataBaseList();
  /* Inverse Data */
//OK  CreateInverseMethodList();
//OK  CreateObservationDataList();
//OK  CreateInverseVariableList();
//OK  CreateInverseIterationList();
  /* RFD Objekte */
  CreateIterationPropertiesList();
//OK  CreateSystemTimeList();
  /* RFI Objekte - Geometrie und Topologie */
  CreateNodeList();
  ElCreateElementList();
//OK  CreateRefineElementList();
//OK  CreateMeshGenerationList();
//OK  CreateEdgeList();
//OK  CreatePlainList();
}


/**************************************************************************/
/* ROCKFLOW - Funktion: DestroyObjectLists
                                                                          */
/* Aufgabe:
   Destruktor fuer Objekt-Listen
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   - void -
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   08/1999    OK   Implementierung
   12/1999    OK   Anpassung an rf3260
   10/2001    AH   Inverse Modellierung
                   Eingabe-Datenbank
                   Rockflow-Modell-Liste
                   Alle noetigen Aenderungen sind ausgeklammert.
   03/2003    RK   Rockflow-Modell-Liste entfernt (Version rf3818)                         
   04/2003    MK   MeshGeneration-Liste    
   06/2003 OK Processes list             
                                                                          */
/**************************************************************************/
void DestroyObjectLists(void)
{
  /* RFD Objekte */
//WW  DestroyIterationPropertiesList();
//OK  DestroySystemTimeList();
  /* RFI Objekte - Geometrie und Topologie */
//WW  DestroyNodeList();
 //WW ElDestroyElementList();
//OK  DestroyRefineElementList();
//OK  DestroyMeshGenerationList();
//OK  DestroyEdgeList();
//OK  DestroyPlainList();
  /* Inverse Data */
//OK  DestroyInverseMethodList();
//OK  DestroyObservationDataList();
//OK  DestroyInverseVariableList();
//OK  DestroyInverseIterationList();
  /* Data base */
//OK  DestroyDataBaseList();
//OK  DestroyInputDataBaseList();
  // only if transport is calculated:
  if(GetRFProcessProcessingAndActivation("MT") && GetRFProcessNumComponents()>0) {
    DestroyREACT(); //SB
	cp_vec.clear(); // Destroy component properties vector
  }
  // PCS
  pcs_vector.clear();
#ifdef CHEMAPP
  if (Eqlink_vec.size()>0){ 
    Eqlink_vec[0]->DestroyMemory();
    Eqlink_vec.clear();
  }
#endif
}

/**************************************************************************/
/* ROCKFLOW - Funktion: ConfigObjects
                                                                          */
/* Aufgabe:
   Objekte konfigurieren
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   - void -
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   08/1999   OK   Implementierung
   10/2001   AH   Inverse Modellierung
                  Alle Aenderungen sind ausgeklammert.
   04/2002   OK   Trennung von RFConfigObjects und RFConfigRenumber                  
                                                                          */
/**************************************************************************/
void RFConfigObjects(void)
{
  ConfigSolverProperties();
  ConfigIterationProperties();
}

void RFConfigRenumber(void)
{
  RenumberStart();
  RenumberNodes();  /* darf nur einmal aufgerufen werden */
}


/**************************************************************************/
/* ROCKFLOW - Funktion: ConfigRFFiles
                                                                          */
/* Aufgabe:
   File-Namen konfigurieren
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   E: char *dateiname
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   08/1999   OK   Implementierung
                                                                          */
/**************************************************************************/
void ConfigRFFiles(char *dateiname)
{
    static int str_length;

    /**************************************************************************/
    /* dateiname wird spaeter noch benoetigt, zB fuer Restart */
    str_length = (int)strlen(dateiname) + 5;
    file_name = (char *) Malloc(str_length);
    strcpy(file_name,dateiname);
    /* Bilanzobjekte initialisieren, Files oeffnen, Header schreiben */
}

/**************************************************************************/
/* ROCKFLOW - Funktion: DestroyObjects
                                                                          */
/* Aufgabe:
   Destruktor fuer Objekte
     Beim Lesen von RFD und RFI Dateien erfolgt
     die Konstruktion der Objekte in ReadData()
                                                                          */
/* Formalparameter: (E: Eingabe; R: Rueckgabe; X: Beides)
   - void -
                                                                          */
/* Ergebnis:
   - void -
                                                                          */
/* Programmaenderungen:
   08/1999   OK   Implementierung
   10/2001   AH   Inverse Modellierung
                  Alle Aenderungen sind ausgeklammert.
                                                                          */
/**************************************************************************/
void DestroyRFObjects(void)
{
  DestroyFunctionsData();
  RenumberEnd();
}

/**************************************************************************
ROCKFLOW - Funktion: RFCreateObjectListsNew
Task: Create all RF lists
Programing:
01/2003   OK   Implementation
**************************************************************************/
void RFCreateObjectListsNew(void)
{
    /* Data base */
//OK    CreateDataBaseList();
//OK    CreateInputDataBaseList();
    /* Inverse Data */
//OK    CreateInverseMethodList();
//OK    CreateObservationDataList();
//OK    CreateInverseVariableList();
//OK    CreateInverseIterationList();
    /* RFD Objekte */
    CreateIterationPropertiesList();
//OK    CreateSystemTimeList();
    /* RFI Objekte - Geometrie und Topologie */
    CreateNodeList();
    ElCreateElementList();
//OK    CreateRefineElementList();
//OK    CreateEdgeList();
//OK    CreatePlainList();
}

/**************************************************************************
ROCKFLOW - Funktion: 
Task: 
Programing:
10/2003 OK Implementation
**************************************************************************/
int RFExecuteTimeLoop(void)
{
  return 1;
}



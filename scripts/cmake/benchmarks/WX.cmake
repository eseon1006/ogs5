include(test/Benchmark)

Benchmark(AUTHOR WX
	PATH M/excavation/3D_Time_Controlled/3D_Excav_Time_Controlled
	REQUIRED_CMAKE_OPTIONS OGS_FEM
	RUNTIME
	OUTPUT_FILES 3D_Excav_Time_Controlled_domain_hex.tec
)

Benchmark(AUTHOR WX
	PATH M/3D_oedometer_mohr_coulomb
	REQUIRED_CMAKE_OPTIONS OGS_FEM
	RUNTIME
	OUTPUT_FILES 3D_oedometer_mohr_coulomb_domain_hex.tec
)

Benchmark(AUTHOR WX
	PATH H2/LabGasInjec/H2_Permeability_GasPressure
	REQUIRED_CMAKE_OPTIONS OGS_FEM
	RUNTIME
	OUTPUT_FILES H2_Permeability_GasPressure_domain_MULTI_PHASE_FLOW_quad.tec
)

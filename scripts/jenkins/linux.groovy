def runLinux() {
	def configs = [
		[name:"FEM", cmakeOptions:"-DOGS_NO_EXTERNAL_LIBS=ON -DOGS_PACKAGING=ON -DCMAKE_BUILD_TYPE=Release", target:"package", artifacts:"*.tar.gz"],
		[name:"FEM_SP"],
		[name:"FEM_GEMS"],
		[name:"FEM_PQC"],
		[name:"FEM_IPQC"],
		[name:"FEM_BRNS"],
		[name:"FEM_MKL", cmakeOptions:"-DMKL_DIR=/opt/intel/mkl"],
		[name:"FEM_LIS"],
		[name:"FEM_MPI"],
		[name:"FEM_PETSC"],
		[name:"FEM_PETSC_GEMS"]
	]

	stage 'Building'
	def buildTasks = [:]
	// configs.each { // does not work: https://github.com/cloudbees/groovy-cps/issues/9
	for(i = 0; i < configs.size(); i++) {
		def configName = configs[i].name
		def cmakeOptions = configs[i].cmakeOptions ? configs[i].cmakeOptions : ''
		cmakeOptions += ' -DNUMDIFF_TOOL_PATH=/usr/local/numdiff/5.8.1-1/bin/numdiff'
		def target = configs[i].target ? configs[i].target : ''
		def artifacts = configs[i].artifacts
		def buildDir = 'build_' + configName
		buildTasks[configName] = {
			build buildDir, '-DOGS_' + configName +  '=ON ' + cmakeOptions, target
			if(artifacts)
				archive buildDir + '/' + artifacts
		}
	}
	parallel buildTasks

	stage 'Benchmarking'
	def benchmarkTasks = [:]
	for(i = 0; i < configs.size(); i++) {
		def configName = configs[i].name
		def buildDir = 'build_' + configName
		benchmarkTasks[configName] = {
			dir(buildDir) {
				catchError {
					sh """module () { eval `/usr/local/modules/3.2.10-1/Modules/3.2.10/bin/modulecmd sh \$*`; }

set +x
export MODULEPATH=\"/global/apps/modulefiles:\$MODULEPATH\"
module load cmake/3.1.3-1
module load gcc/4.8.1-3
module load boost/1.55.0-4
module load doxygen/1.8.7-1_gcc_4.8.1
module load lapack/3.5.0-1_gcc_4.8.1
module unload python
module load openmpi/gcc/1.8.4-2
module load petsc/3.5_maint_gcc_4.8.1-3_openmpi_gcc_1.8.2-1_gcc_4.8.1_CentOS6_envinf
set -x
nice -n 5 make benchmarks-short-normal-long"""
				}
				archive '**/*.numdiff'
			}
		}
	}
	parallel benchmarkTasks
}

// Builds target with cmakeOptions in buildDir
def build(buildDir, cmakeOptions, target) {
	catchError {
		sh """module () { eval `/usr/local/modules/3.2.10-1/Modules/3.2.10/bin/modulecmd sh \$*`; }

set +x
export MODULEPATH=\"/global/apps/modulefiles:\$MODULEPATH\"
module load cmake/3.1.3-1
module load gcc/4.8.1-3
module load boost/1.55.0-4
module load doxygen/1.8.7-1_gcc_4.8.1
module load lapack/3.5.0-1_gcc_4.8.1
module unload python
module load openmpi/gcc/1.8.4-2
module load petsc/3.5_maint_gcc_4.8.1-3_openmpi_gcc_1.8.2-1_gcc_4.8.1_CentOS6_envinf
set -x

ln -s /opt/ogs/ogs5-libs Libs || :
rm -rf ${buildDir} && mkdir ${buildDir} && cd ${buildDir}
cmake ../ogs ${cmakeOptions} -DCMAKE_BUILD_TYPE=Release
nice -n 5 make -j 2 ${target}"""
	}
}

// Check out branch from url into directory
def checkout(url, branch, directory) {
	stage 'Checkout'
	checkout([$class: 'GitSCM',
		branches: [[name: "*/${branch}"]],
		doGenerateSubmoduleConfigurations: false,
		extensions:
			[[$class: 'RelativeTargetDirectory', relativeTargetDir: "${directory}"]],
		submoduleCfg: [],
		userRemoteConfigs:
			[[credentialsId: '6c1dad0d-9b3c-44c2-a6bb-669562045187', url: "${url}"]]])
}

return this;

# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.22

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:

#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:

# Disable VCS-based implicit rules.
% : %,v

# Disable VCS-based implicit rules.
% : RCS/%

# Disable VCS-based implicit rules.
% : RCS/%,v

# Disable VCS-based implicit rules.
% : SCCS/s.%

# Disable VCS-based implicit rules.
% : s.%

.SUFFIXES: .hpux_make_needs_suffix_list

# Command-line flag to silence nested $(MAKE).
$(VERBOSE)MAKESILENT = -s

#Suppress display of executed commands.
$(VERBOSE).SILENT:

# A target that is always out of date.
cmake_force:
.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = /usr/bin/cmake

# The command to remove a file.
RM = /usr/bin/cmake -E rm -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/sean/Grad_Research/Anderson_Acceleration/step-57-KINSol

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/sean/Grad_Research/Anderson_Acceleration/step-57-KINSol

# Include any dependencies generated for this target.
include CMakeFiles/step-57.dir/depend.make
# Include any dependencies generated by the compiler for this target.
include CMakeFiles/step-57.dir/compiler_depend.make

# Include the progress variables for this target.
include CMakeFiles/step-57.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/step-57.dir/flags.make

CMakeFiles/step-57.dir/step-57.cc.o: CMakeFiles/step-57.dir/flags.make
CMakeFiles/step-57.dir/step-57.cc.o: step-57.cc
CMakeFiles/step-57.dir/step-57.cc.o: CMakeFiles/step-57.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/sean/Grad_Research/Anderson_Acceleration/step-57-KINSol/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object CMakeFiles/step-57.dir/step-57.cc.o"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/step-57.dir/step-57.cc.o -MF CMakeFiles/step-57.dir/step-57.cc.o.d -o CMakeFiles/step-57.dir/step-57.cc.o -c /home/sean/Grad_Research/Anderson_Acceleration/step-57-KINSol/step-57.cc

CMakeFiles/step-57.dir/step-57.cc.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/step-57.dir/step-57.cc.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/sean/Grad_Research/Anderson_Acceleration/step-57-KINSol/step-57.cc > CMakeFiles/step-57.dir/step-57.cc.i

CMakeFiles/step-57.dir/step-57.cc.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/step-57.dir/step-57.cc.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/sean/Grad_Research/Anderson_Acceleration/step-57-KINSol/step-57.cc -o CMakeFiles/step-57.dir/step-57.cc.s

# Object files for target step-57
step__57_OBJECTS = \
"CMakeFiles/step-57.dir/step-57.cc.o"

# External object files for target step-57
step__57_EXTERNAL_OBJECTS =

step-57: CMakeFiles/step-57.dir/step-57.cc.o
step-57: CMakeFiles/step-57.dir/build.make
step-57: /home/sean/Software/deal.II/installed/lib/libdeal_II.g.so.9.5.0-pre
step-57: /usr/lib/x86_64-linux-gnu/libtrilinos_pike-blackbox.so
step-57: /usr/lib/x86_64-linux-gnu/libtrilinos_trilinoscouplings.so
step-57: /usr/lib/x86_64-linux-gnu/libtrilinos_piro.so
step-57: /usr/lib/x86_64-linux-gnu/libtrilinos_rol.so
step-57: /usr/lib/x86_64-linux-gnu/libtrilinos_stokhos_muelu.so
step-57: /usr/lib/x86_64-linux-gnu/libtrilinos_stokhos_muelu_pce_serial.so
step-57: /usr/lib/x86_64-linux-gnu/libtrilinos_stokhos_muelu_mp_16_serial.so
step-57: /usr/lib/x86_64-linux-gnu/libtrilinos_stokhos_ifpack2.so
step-57: /usr/lib/x86_64-linux-gnu/libtrilinos_stokhos_ifpack2_pce_serial.so
step-57: /usr/lib/x86_64-linux-gnu/libtrilinos_stokhos_ifpack2_mp_16_serial.so
step-57: /usr/lib/x86_64-linux-gnu/libtrilinos_stokhos_amesos2.so
step-57: /usr/lib/x86_64-linux-gnu/libtrilinos_stokhos_xpetra.so
step-57: /usr/lib/x86_64-linux-gnu/libtrilinos_stokhos_xpetra_pce_serial.so
step-57: /usr/lib/x86_64-linux-gnu/libtrilinos_stokhos_xpetra_mp_16_serial.so
step-57: /usr/lib/x86_64-linux-gnu/libtrilinos_stokhos_tpetra.so
step-57: /usr/lib/x86_64-linux-gnu/libtrilinos_stokhos_tpetraext_pce_serial.so
step-57: /usr/lib/x86_64-linux-gnu/libtrilinos_stokhos_tpetra_pce_serial.so
step-57: /usr/lib/x86_64-linux-gnu/libtrilinos_stokhos_tpetra_sd_pce_serial.so
step-57: /usr/lib/x86_64-linux-gnu/libtrilinos_stokhos_tpetraext_mp_16_serial.so
step-57: /usr/lib/x86_64-linux-gnu/libtrilinos_stokhos_tpetra_mp_16_serial.so
step-57: /usr/lib/x86_64-linux-gnu/libtrilinos_stokhos_tpetra_sd_mp_16_serial.so
step-57: /usr/lib/x86_64-linux-gnu/libtrilinos_stokhos_sacado.so
step-57: /usr/lib/x86_64-linux-gnu/libtrilinos_stokhos.so
step-57: /usr/lib/x86_64-linux-gnu/libtrilinos_rythmos.so
step-57: /usr/lib/x86_64-linux-gnu/libtrilinos_shylu_ddcommon.so
step-57: /usr/lib/x86_64-linux-gnu/libtrilinos_shylu_ddfrosch.so
step-57: /usr/lib/x86_64-linux-gnu/libtrilinos_shylu_ddbddc.so
step-57: /usr/lib/x86_64-linux-gnu/libtrilinos_zoltan2sphynx.so
step-57: /usr/lib/x86_64-linux-gnu/libtrilinos_muelu-adapters.so
step-57: /usr/lib/x86_64-linux-gnu/libtrilinos_muelu-interface.so
step-57: /usr/lib/x86_64-linux-gnu/libtrilinos_muelu.so
step-57: /usr/lib/x86_64-linux-gnu/libtrilinos_moertel.so
step-57: /usr/lib/x86_64-linux-gnu/libtrilinos_locatpetra.so
step-57: /usr/lib/x86_64-linux-gnu/libtrilinos_locathyra.so
step-57: /usr/lib/x86_64-linux-gnu/libtrilinos_locaepetra.so
step-57: /usr/lib/x86_64-linux-gnu/libtrilinos_localapack.so
step-57: /usr/lib/x86_64-linux-gnu/libtrilinos_loca.so
step-57: /usr/lib/x86_64-linux-gnu/libtrilinos_noxepetra.so
step-57: /usr/lib/x86_64-linux-gnu/libtrilinos_noxlapack.so
step-57: /usr/lib/x86_64-linux-gnu/libtrilinos_nox.so
step-57: /usr/lib/x86_64-linux-gnu/libtrilinos_phalanx.so
step-57: /usr/lib/x86_64-linux-gnu/libtrilinos_intrepid2.so
step-57: /usr/lib/x86_64-linux-gnu/libtrilinos_intrepid.so
step-57: /usr/lib/x86_64-linux-gnu/libtrilinos_teko.so
step-57: /usr/lib/x86_64-linux-gnu/libtrilinos_stratimikos.so
step-57: /usr/lib/x86_64-linux-gnu/libtrilinos_stratimikosbelos.so
step-57: /usr/lib/x86_64-linux-gnu/libtrilinos_stratimikosamesos2.so
step-57: /usr/lib/x86_64-linux-gnu/libtrilinos_stratimikosaztecoo.so
step-57: /usr/lib/x86_64-linux-gnu/libtrilinos_stratimikosamesos.so
step-57: /usr/lib/x86_64-linux-gnu/libtrilinos_stratimikosml.so
step-57: /usr/lib/x86_64-linux-gnu/libtrilinos_stratimikosifpack.so
step-57: /usr/lib/x86_64-linux-gnu/libtrilinos_ifpack2-adapters.so
step-57: /usr/lib/x86_64-linux-gnu/libtrilinos_ifpack2.so
step-57: /usr/lib/x86_64-linux-gnu/libtrilinos_anasazitpetra.so
step-57: /usr/lib/x86_64-linux-gnu/libtrilinos_ModeLaplace.so
step-57: /usr/lib/x86_64-linux-gnu/libtrilinos_anasaziepetra.so
step-57: /usr/lib/x86_64-linux-gnu/libtrilinos_anasazi.so
step-57: /usr/lib/x86_64-linux-gnu/libtrilinos_komplex.so
step-57: /usr/lib/x86_64-linux-gnu/libtrilinos_amesos2.so
step-57: /usr/lib/x86_64-linux-gnu/libtrilinos_tacho.so
step-57: /usr/lib/x86_64-linux-gnu/libtrilinos_shylu_nodehts.so
step-57: /usr/lib/x86_64-linux-gnu/libtrilinos_belosxpetra.so
step-57: /usr/lib/x86_64-linux-gnu/libtrilinos_belostpetra.so
step-57: /usr/lib/x86_64-linux-gnu/libtrilinos_belosepetra.so
step-57: /usr/lib/x86_64-linux-gnu/libtrilinos_belos.so
step-57: /usr/lib/x86_64-linux-gnu/libtrilinos_ml.so
step-57: /usr/lib/x86_64-linux-gnu/libtrilinos_ifpack.so
step-57: /usr/lib/x86_64-linux-gnu/libtrilinos_zoltan2.so
step-57: /usr/lib/x86_64-linux-gnu/libtrilinos_pamgen_extras.so
step-57: /usr/lib/x86_64-linux-gnu/libtrilinos_pamgen.so
step-57: /usr/lib/x86_64-linux-gnu/libtrilinos_amesos.so
step-57: /usr/lib/x86_64-linux-gnu/libtrilinos_galeri-xpetra.so
step-57: /usr/lib/x86_64-linux-gnu/libtrilinos_galeri-epetra.so
step-57: /usr/lib/x86_64-linux-gnu/libtrilinos_aztecoo.so
step-57: /usr/lib/x86_64-linux-gnu/libtrilinos_dpliris.so
step-57: /usr/lib/x86_64-linux-gnu/libtrilinos_isorropia.so
step-57: /usr/lib/x86_64-linux-gnu/libtrilinos_xpetra-sup.so
step-57: /usr/lib/x86_64-linux-gnu/libtrilinos_xpetra.so
step-57: /usr/lib/x86_64-linux-gnu/libtrilinos_thyratpetra.so
step-57: /usr/lib/x86_64-linux-gnu/libtrilinos_thyraepetraext.so
step-57: /usr/lib/x86_64-linux-gnu/libtrilinos_thyraepetra.so
step-57: /usr/lib/x86_64-linux-gnu/libtrilinos_thyracore.so
step-57: /usr/lib/x86_64-linux-gnu/libtrilinos_trilinosss.so
step-57: /usr/lib/x86_64-linux-gnu/libtrilinos_tpetraext.so
step-57: /usr/lib/x86_64-linux-gnu/libtrilinos_tpetrainout.so
step-57: /usr/lib/x86_64-linux-gnu/libtrilinos_tpetra.so
step-57: /usr/lib/x86_64-linux-gnu/libtrilinos_kokkostsqr.so
step-57: /usr/lib/x86_64-linux-gnu/libtrilinos_tpetraclassiclinalg.so
step-57: /usr/lib/x86_64-linux-gnu/libtrilinos_tpetraclassicnodeapi.so
step-57: /usr/lib/x86_64-linux-gnu/libtrilinos_tpetraclassic.so
step-57: /usr/lib/x86_64-linux-gnu/libtrilinos_epetraext.so
step-57: /usr/lib/x86_64-linux-gnu/libtrilinos_triutils.so
step-57: /usr/lib/x86_64-linux-gnu/libtrilinos_shards.so
step-57: /usr/lib/x86_64-linux-gnu/libtrilinos_zoltan.so
step-57: /usr/lib/x86_64-linux-gnu/libtrilinos_epetra.so
step-57: /usr/lib/x86_64-linux-gnu/libtrilinos_sacado.so
step-57: /usr/lib/x86_64-linux-gnu/libtrilinos_rtop.so
step-57: /usr/lib/x86_64-linux-gnu/libtrilinos_kokkoskernels.so
step-57: /usr/lib/x86_64-linux-gnu/libtrilinos_teuchoskokkoscomm.so
step-57: /usr/lib/x86_64-linux-gnu/libtrilinos_teuchoskokkoscompat.so
step-57: /usr/lib/x86_64-linux-gnu/libtrilinos_teuchosremainder.so
step-57: /usr/lib/x86_64-linux-gnu/libtrilinos_teuchosnumerics.so
step-57: /usr/lib/x86_64-linux-gnu/libtrilinos_teuchoscomm.so
step-57: /usr/lib/x86_64-linux-gnu/libtrilinos_teuchosparameterlist.so
step-57: /usr/lib/x86_64-linux-gnu/libtrilinos_teuchosparser.so
step-57: /usr/lib/x86_64-linux-gnu/libtrilinos_teuchoscore.so
step-57: /usr/lib/x86_64-linux-gnu/libtrilinos_kokkosalgorithms.so
step-57: /usr/lib/x86_64-linux-gnu/libtrilinos_kokkoscontainers.so
step-57: /usr/lib/x86_64-linux-gnu/libtrilinos_kokkoscore.so
step-57: /usr/lib/x86_64-linux-gnu/libtbb.so
step-57: /usr/lib/x86_64-linux-gnu/libsmumps.so
step-57: /usr/lib/x86_64-linux-gnu/libdmumps.so
step-57: /usr/lib/x86_64-linux-gnu/libcmumps.so
step-57: /usr/lib/x86_64-linux-gnu/libzmumps.so
step-57: /usr/lib/x86_64-linux-gnu/libpord.so
step-57: /usr/lib/x86_64-linux-gnu/libmumps_common.so
step-57: /usr/lib/x86_64-linux-gnu/libptscotch.so
step-57: /usr/lib/x86_64-linux-gnu/libptscotcherr.so
step-57: /usr/lib/x86_64-linux-gnu/libscotch.so
step-57: /usr/lib/x86_64-linux-gnu/libscotcherr.so
step-57: /usr/lib/x86_64-linux-gnu/openmpi/lib/libmpi_cxx.so
step-57: /usr/lib/x86_64-linux-gnu/libumfpack.so
step-57: /usr/lib/x86_64-linux-gnu/libcholmod.so
step-57: /usr/lib/x86_64-linux-gnu/libccolamd.so
step-57: /usr/lib/x86_64-linux-gnu/libcolamd.so
step-57: /usr/lib/x86_64-linux-gnu/libcamd.so
step-57: /usr/lib/x86_64-linux-gnu/libsuitesparseconfig.so
step-57: /usr/lib/x86_64-linux-gnu/libamd.so
step-57: /usr/lib/x86_64-linux-gnu/libmetis.so
step-57: /usr/lib/x86_64-linux-gnu/libadolc.so
step-57: /usr/lib/x86_64-linux-gnu/libboost_iostreams.so
step-57: /usr/lib/x86_64-linux-gnu/libboost_serialization.so
step-57: /usr/lib/x86_64-linux-gnu/libboost_system.so
step-57: /usr/lib/x86_64-linux-gnu/libboost_thread.so
step-57: /usr/lib/x86_64-linux-gnu/libboost_regex.so
step-57: /usr/lib/x86_64-linux-gnu/libboost_chrono.so
step-57: /usr/lib/x86_64-linux-gnu/libboost_date_time.so
step-57: /usr/lib/x86_64-linux-gnu/libboost_atomic.so
step-57: /usr/lib/x86_64-linux-gnu/libparpack.so
step-57: /usr/lib/x86_64-linux-gnu/libarpack.so
step-57: /usr/lib/x86_64-linux-gnu/libassimp.so
step-57: /usr/lib/x86_64-linux-gnu/libgmpxx.so
step-57: /usr/lib/x86_64-linux-gnu/libmpfr.so
step-57: /usr/lib/x86_64-linux-gnu/libgmp.so
step-57: /usr/lib/x86_64-linux-gnu/libgmsh.so
step-57: /usr/lib/x86_64-linux-gnu/libgsl.so
step-57: /usr/lib/x86_64-linux-gnu/libgslcblas.so
step-57: /usr/lib/x86_64-linux-gnu/hdf5/openmpi/libhdf5.so
step-57: /usr/lib/x86_64-linux-gnu/libcrypto.so
step-57: /usr/lib/x86_64-linux-gnu/libcurl.so
step-57: /usr/lib/x86_64-linux-gnu/libsz.so
step-57: /usr/lib/x86_64-linux-gnu/libdl.a
step-57: /usr/lib/x86_64-linux-gnu/libmuparser.so
step-57: /usr/lib/x86_64-linux-gnu/libTKBO.so
step-57: /usr/lib/x86_64-linux-gnu/libTKBool.so
step-57: /usr/lib/x86_64-linux-gnu/libTKBRep.so
step-57: /usr/lib/x86_64-linux-gnu/libTKernel.so
step-57: /usr/lib/x86_64-linux-gnu/libTKFeat.so
step-57: /usr/lib/x86_64-linux-gnu/libTKFillet.so
step-57: /usr/lib/x86_64-linux-gnu/libTKG2d.so
step-57: /usr/lib/x86_64-linux-gnu/libTKG3d.so
step-57: /usr/lib/x86_64-linux-gnu/libTKGeomAlgo.so
step-57: /usr/lib/x86_64-linux-gnu/libTKGeomBase.so
step-57: /usr/lib/x86_64-linux-gnu/libTKHLR.so
step-57: /usr/lib/x86_64-linux-gnu/libTKIGES.so
step-57: /usr/lib/x86_64-linux-gnu/libTKMath.so
step-57: /usr/lib/x86_64-linux-gnu/libTKMesh.so
step-57: /usr/lib/x86_64-linux-gnu/libTKOffset.so
step-57: /usr/lib/x86_64-linux-gnu/libTKPrim.so
step-57: /usr/lib/x86_64-linux-gnu/libTKShHealing.so
step-57: /usr/lib/x86_64-linux-gnu/libTKSTEP.so
step-57: /usr/lib/x86_64-linux-gnu/libTKSTEPAttr.so
step-57: /usr/lib/x86_64-linux-gnu/libTKSTEPBase.so
step-57: /usr/lib/x86_64-linux-gnu/libTKSTEP209.so
step-57: /usr/lib/x86_64-linux-gnu/libTKSTL.so
step-57: /usr/lib/x86_64-linux-gnu/libTKTopAlgo.so
step-57: /usr/lib/x86_64-linux-gnu/libTKXSBase.so
step-57: /usr/lib/x86_64-linux-gnu/libp4est.so
step-57: /usr/lib/x86_64-linux-gnu/libsc.so
step-57: /usr/lib/x86_64-linux-gnu/libscalapack-openmpi.so
step-57: /usr/lib/x86_64-linux-gnu/liblapack.so
step-57: /usr/lib/x86_64-linux-gnu/libblas.so
step-57: /usr/lib/x86_64-linux-gnu/libmpi_usempif08.so
step-57: /usr/lib/x86_64-linux-gnu/libmpi_usempi_ignore_tkr.so
step-57: /usr/lib/x86_64-linux-gnu/libmpi_mpifh.so
step-57: /usr/lib/x86_64-linux-gnu/openmpi/lib/libmpi.so
step-57: /usr/lib/x86_64-linux-gnu/libopen-rte.so
step-57: /usr/lib/x86_64-linux-gnu/libopen-pal.so
step-57: /usr/lib/x86_64-linux-gnu/libhwloc.so
step-57: /usr/lib/x86_64-linux-gnu/libevent_core.so
step-57: /usr/lib/x86_64-linux-gnu/libevent_pthreads.so
step-57: /usr/lib/x86_64-linux-gnu/libz.so
step-57: /usr/lib/x86_64-linux-gnu/libsundials_idas.so
step-57: /usr/lib/x86_64-linux-gnu/libsundials_arkode.so
step-57: /usr/lib/x86_64-linux-gnu/libsundials_kinsol.so
step-57: /usr/lib/x86_64-linux-gnu/libsundials_nvecserial.so
step-57: /usr/lib/x86_64-linux-gnu/libsundials_nvecparallel.so
step-57: CMakeFiles/step-57.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/sean/Grad_Research/Anderson_Acceleration/step-57-KINSol/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX executable step-57"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/step-57.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/step-57.dir/build: step-57
.PHONY : CMakeFiles/step-57.dir/build

CMakeFiles/step-57.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/step-57.dir/cmake_clean.cmake
.PHONY : CMakeFiles/step-57.dir/clean

CMakeFiles/step-57.dir/depend:
	cd /home/sean/Grad_Research/Anderson_Acceleration/step-57-KINSol && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/sean/Grad_Research/Anderson_Acceleration/step-57-KINSol /home/sean/Grad_Research/Anderson_Acceleration/step-57-KINSol /home/sean/Grad_Research/Anderson_Acceleration/step-57-KINSol /home/sean/Grad_Research/Anderson_Acceleration/step-57-KINSol /home/sean/Grad_Research/Anderson_Acceleration/step-57-KINSol/CMakeFiles/step-57.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/step-57.dir/depend

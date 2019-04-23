HIP_PATH?= $(wildcard /opt/rocm/hip)
HIPCC=$(HIP_PATH)/bin/hipcc
# HIPCC=hipcc

square:	square.cpp
	hipcc square.cpp -o square
vadd:	vadd_hip.cpp
	hipcc vadd_hip.cpp -o vadd_hip
diffusion:	diffusion_hip.cpp
	hipcc -ffast-math -O3 diffusion_hip.cpp -o diffusion

# all: square.hip.out


# square.cuda.out : square.cu
#	nvcc square.cu -o $@

#hipify square.cu > square.cpp
# Then review & finish port in square.cpp
#

#  --amdgpu-target=gfx803
# square.hip.out: square.hipref.cpp
#	$(HIPCC) $(CXXFLAGS) square.hipref.cpp -o $@




clean:
	rm -f *.o *.out square vadd_hip diffusion

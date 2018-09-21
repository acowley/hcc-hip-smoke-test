diffusion: diffusion.cpp
	hcc `clamp-config --cxxflags --ldflags` -O3 diffusion.cpp -o diffusion

diffusion_hcc: diffusion_hcc.cpp
	hcc `hcc-config --cxxflags --ldflags` -O3 diffusion_hcc.cpp -o diffusion_hcc

saxpy: saxpy.cpp
	hcc `hcc-config --cxxflags --ldflags` -O3 saxpy.cpp -o saxpy

# -amdgpu-target=gfx803

clean:
	rm -f diffusion diffusion_hcc saxpy

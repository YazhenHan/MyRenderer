#pragma once
#include "base.hpp"

MyMesh simple_mesh_smoother(MyMesh mesh);

MyMesh loop_subdivision(MyMesh mesh);

MyMesh qem_simplification(MyMesh mesh);

MyMesh local_minimal_surface(MyMesh mesh, float lambda = 0.001, int in_num = 100);

MyMesh global_minimal_surface(MyMesh mesh);
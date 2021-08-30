#pragma once
#include "base.hpp"

void simple_mesh_smoother(MyMesh& mesh);

void local_laplace_smoother(MyMesh& mesh, double lambda = 0.001, int in_num = 100);

void global_laplace_smoother(MyMesh& mesh);

void loop_subdivision(MyMesh& mesh);

void qem_simplification(MyMesh& mesh);
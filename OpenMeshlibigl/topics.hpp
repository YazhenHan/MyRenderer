#pragma once
#ifndef TOPICS_HPP
#define TOPICS_HPP

#include "base.hpp"


void loop();

void qemSimplification(MyMesh& mesh, float ratio);

MyMesh local_minimal_surface(MyMesh mesh, float lambda = 0.001, int it_num = 100);

MyMesh global_minimal_surface(MyMesh mesh);

#endif // !TOPICS_HPP

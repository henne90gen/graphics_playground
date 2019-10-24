#pragma once

#define INFO_BASIC(var) INFO(#var "=" << var)
#define INFO_VEC2(var) INFO(#var "={" << var.x << ", " << var.y << "}")
#define INFO_VEC3(var) INFO(#var "={" << var.x << ", " << var.y << ", " << var.z << "}")

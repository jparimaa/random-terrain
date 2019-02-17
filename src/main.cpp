#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>
#include <glm/glm.hpp>

#include <iostream>

int main()
{
    glm::vec3 v(1.0f, 2.0f, 3.0f);
    std::cout << v.x << " " << v.y << " " << v.z << "\n";
    return 0;
}

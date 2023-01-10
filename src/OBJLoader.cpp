#include "OBJLoader.hpp"

OBJData OBJLoader::LoadFile(std::string path)
{
	OBJData data;
    std::string objStr;
    std::ifstream file;
    file.exceptions(std::ifstream::badbit);
    try {
        // Open files
        file.open(path);
        std::string line;
        while (std::getline(file, line))
        {
            std::istringstream s(line);
            std::string header = "";
            s >> header;
            // If vertex
            if (header == "v") {
                glm::vec3 vertex;
                s >> vertex.x;
                s >> vertex.y;
                s >> vertex.z;
                data.vertices.push_back(vertex);
            }
            // If vertex normal
            else if (header == "vn") {
                glm::vec3 normal;
                s >> normal.x;
                s >> normal.y;
                s >> normal.z;
                data.normals.push_back(normal);
            }
            // Read face
            else if (header == "f") {
                GLuint temp;
                int counter = 0;
                while (s >> temp) {
                    if (counter == 0) {}
                    else if (counter == 1) {}
                    else if (counter == 2) {}

                }
            }
            /* anything else is ignored */
        }
        std::stringstream stream;
        // Read file's buffer contents into streams
        stream << file.rdbuf();
        // close file handler
        file.close();
        // Convert stream into string
        objStr = stream.str();
    }
    catch (std::ifstream::failure e)
    {
        std::cerr << "ERROR::OBJLOADER::FILE_NOT_SUCCESFULLY_READ: " << path << std::endl;
        throw "Failed to load OBJ file";
    }
	return data;
}
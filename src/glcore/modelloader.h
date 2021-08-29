#pragma once

struct ObjVertex
{
	glm::vec3 Position;
	glm::vec3 Normal;
	//glm::vec2 TexCoord;
	//glm::vec3 Colour;
};

void loadObj(std::vector<ObjVertex>& vertices, std::vector<uint>& indices, std::string filepath, std::string filepathMaterials = "./");



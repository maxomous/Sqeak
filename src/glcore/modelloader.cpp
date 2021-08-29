
#include <iostream>
#include <vector>
#include <string>
using namespace std;  

#define TINYOBJLOADER_IMPLEMENTATION // define this in only *one* .cc
#include "deps/tinyobjloader/tiny_obj_loader.h"

#include "glcore.h"
#include "modelloader.h"

 
void loadObj(vector<ObjVertex>& vertices, vector<uint>& indices, std::string filepath, std::string filepathMaterials)
{
    vertices.clear();
    indices.clear();
    
    tinyobj::ObjReaderConfig reader_config;
    reader_config.mtl_search_path = filepathMaterials; // Path to material files

    tinyobj::ObjReader reader;

    if (!reader.ParseFromFile(filepath, reader_config)) {
        if (!reader.Error().empty()) {
            std::cerr << "TinyObjReader: " << reader.Error();
        }
        exit(1);
    }

    if (!reader.Warning().empty()) {
        std::cout << "TinyObjReader: " << reader.Warning();
    }

    auto& attrib = reader.GetAttrib();
    auto& shapes = reader.GetShapes();
    //auto& materials = reader.GetMaterials();
    
    if(shapes.size() > 1) {
        cerr << "multiple shapes!" << endl;
        exit(1);
    }
    cout << "Shapes: " << shapes.size() << endl;
    
    // Loop over shapes
    for (size_t s = 0; s < shapes.size(); s++) 
    {
        size_t numTriangles = shapes[s].mesh.num_face_vertices.size();
        
        cout << "Faces(triangles): " << numTriangles << " Reserving: " << numTriangles*3 << endl;
        vertices.reserve(numTriangles * 3);
        indices.reserve(numTriangles * 3);
        uint indexBufIndex = 0;
        
        // Loop over faces(polygon)
        size_t index_offset = 0;
        for (size_t f = 0; f < numTriangles; f++) 
        {
            size_t fv = size_t(shapes[s].mesh.num_face_vertices[f]);
            
            if(fv != 3) {
                cerr << "Error: Can only handle triangles, face has " << fv << " sides" << endl;
                exit(1);
            }
            
            // Loop over vertices in the face.
            for (size_t v = 0; v < fv; v++)     // triangle 0-3
            {
                ObjVertex vertex;
                
                // access to vertex
                tinyobj::index_t idx = shapes[s].mesh.indices[index_offset + v];
                tinyobj::real_t vx = attrib.vertices[3*size_t(idx.vertex_index)+0];
                tinyobj::real_t vy = attrib.vertices[3*size_t(idx.vertex_index)+1];
                tinyobj::real_t vz = attrib.vertices[3*size_t(idx.vertex_index)+2];
                vertex.Position = { vx, vy, vz };
                
                // Check if `normal_index` is zero or positive. negative = no normal data
                if (idx.normal_index >= 0) {
                    tinyobj::real_t nx = attrib.normals[3*size_t(idx.normal_index)+0];
                    tinyobj::real_t ny = attrib.normals[3*size_t(idx.normal_index)+1];
                    tinyobj::real_t nz = attrib.normals[3*size_t(idx.normal_index)+2];
                    (void)nx, (void)ny, (void)nz;
                    vertex.Normal = { nx, ny, nz };
                } else {
                    cout << "Warning: Vertex contains no normals" << endl;
                    vertex.Normal = { 0.0f, 0.0f, 0.0f };
                }
                
                // Check if `texcoord_index` is zero or positive. negative = no texcoord data
                if (idx.texcoord_index >= 0) {
                    tinyobj::real_t tx = attrib.texcoords[2*size_t(idx.texcoord_index)+0];
                    tinyobj::real_t ty = attrib.texcoords[2*size_t(idx.texcoord_index)+1];
                    (void)tx, (void)ty;
                    //vertex.TexCoord = { tx, ty };
                } else {
                    cout << "Warning: Vertex contains no texture coords" << endl;
                    //vertex.TexCoord = { 0.0f, 0.0f };
                }
                /*
                // Optional: vertex colors
                tinyobj::real_t red   = attrib.colors[3*size_t(idx.vertex_index)+0];
                tinyobj::real_t green = attrib.colors[3*size_t(idx.vertex_index)+1];
                tinyobj::real_t blue  = attrib.colors[3*size_t(idx.vertex_index)+2];
                (void)red, (void)green, (void)blue;
                //vertex.Colour = { 0.0f, 0.0f };
                */
                vertices.emplace_back(vertex);  
                indices.emplace_back(indexBufIndex++); 
            }
            index_offset += fv;

            // per-face material
            //shapes[s].mesh.material_ids[f];
        }
    }
}

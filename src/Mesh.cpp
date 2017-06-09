// Implementation of Mesh class member functions.

#include <algorithm>
#include <fstream>
#include <iostream>


#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"

#include "core.hpp"
#include "Mesh.hpp"

void import_bounds(Mesh* mesh, std::string dir);

Mesh* Mesh::load_obj(const std::string& dir, const std::string& file) {
    Mesh* mesh = new Mesh();
    mesh->dir = dir;
    std::string err;
    bool result = tinyobj::LoadObj(
        mesh->shapes,
        mesh->materials,
        err,
        (dir+file).c_str(),
        dir.c_str());
    mesh->num_shapes = mesh->shapes.size();

    if (!result) {
        std::fprintf(stderr, "Error in Mesh::load(): %s", err.c_str());
        return nullptr;
    }

    mesh->palette = load_palette(dir+"palette");

    import_bounds(mesh, dir);

    return mesh;
}


// Use all 3 dims for box, determine radius from xz and y for height for cylinder, and determine radius from xyz for cylinder
// Assume box (type = 1) unless manually set otherwise, and then for cylinder (type = 2) or sphere (type = 3), use other dims...


void import_bounds(Mesh* mesh, std::string dir)
{
    Bound bound;
    FILE* file;
    file = fopen((dir+"bound").c_str(), "r");
    if (file != NULL)
    {
        fclose(file);
        std::fstream afile;
        afile.open((dir+"bound").c_str(), std::fstream::in);
        std::string line;

        getline(afile,line);
        int pos = line.find(" ");
        int num = stoi(line.substr(pos,line.size()-pos));
        for (int i = 0; i < num; i++)
        {
            getline(afile, line);

            pos = line.find(" ");

            if      (line.substr(0,pos) == "box")       bound.type = box;
            else if (line.substr(0,pos) == "cylinder")  bound.type = cylinder;
            else if (line.substr(0,pos) == "sphere")    bound.type = sphere;

            line = line.substr(pos+1, line.size()-pos);

            pos = line.find(" ");
            bound.center.x = stof(line.substr(0,pos));
            line = line.substr(pos+1, line.size()-pos);

            pos = line.find(" ");
            bound.center.y = stof(line.substr(0,pos));
            line = line.substr(pos+1, line.size()-pos);

            pos = line.find(" ");
            bound.center.z = stof(line.substr(0,pos));
            line = line.substr(pos+1, line.size()-pos);

            switch (bound.type)
            {
                case box:
                {
                    pos = line.find(" ");
                    bound.dims.x = stof(line.substr(0,pos));
                    line = line.substr(pos+1, line.size()-pos);

                    pos = line.find(" ");
                    bound.dims.y = stof(line.substr(0,pos));
                    line = line.substr(pos+1, line.size()-pos);

                    bound.dims.z = stof(line);
                }
                break;
                case cylinder:
                {
                    pos = line.find(" ");
                    bound.dims.x = stof(line.substr(0,pos));
                    line = line.substr(pos+1, line.size()-pos);

                    bound.dims.y = stof(line);
                    bound.dims.z = 0.0f;
                }
                break;
                case sphere:
                {
                    bound.dims.x = stof(line);
                    bound.dims.y = 0.0f;
                    bound.dims.z = 0.0f;
                }
                break;
            }
        }
        afile.close();
        mesh->bounds.push_back(bound);
        std::cout << "Done\n"; 

    } else
    {
        // Calculate Bounding Box
        glm::vec3 max, min, current, diff, center, dims;
        for (int i = 0; i < mesh->shapes.size(); i++)
        {
            max = glm::vec3(mesh->shapes[i].mesh.positions[0],
                            mesh->shapes[i].mesh.positions[1],
                            mesh->shapes[i].mesh.positions[2]);
            min = max;

            for (int j = 0; j < mesh->shapes[i].mesh.positions.size(); j += 3)
            {
                current = glm::vec3(mesh->shapes[i].mesh.positions[j],
                                    mesh->shapes[i].mesh.positions[j+1],
                                    mesh->shapes[i].mesh.positions[j+2]);

                diff = current - max;
                if (diff.x > 0.0) max.x = current.x;
                if (diff.y > 0.0) max.y = current.y;
                if (diff.z > 0.0) max.z = current.z;

                diff = current - min;
                if (diff.x < 0.0) min.x = current.x;
                if (diff.y < 0.0) min.y = current.y;
                if (diff.z < 0.0) min.z = current.z;
            }

            center = 0.5f * (max + min);
            dims = center - min;
            bound = {box, center, dims};

            mesh->bounds.push_back(bound);
        }

        // Export Bounding Box to file

        std::vector<std::string> bound_type_str = {"box", "cylinder", "sphere"};
        file = fopen((dir+"bound").c_str(), "w");
        fprintf(file, "Bounds: %i\n", mesh->bounds.size());
        for (int i = 0; i < mesh->bounds.size(); i++)
        {
           fprintf(file, "%s %f %f %f ",  
                bound_type_str.at(mesh->bounds.at(i).type).c_str(),
                mesh->bounds.at(i).center.x,
                mesh->bounds.at(i).center.y,
                mesh->bounds.at(i).center.z);
           switch (mesh->bounds.at(i).type)
           {
            case box:
            {
                fprintf(file, "%f %f %f\n",  
                mesh->bounds.at(i).dims.x,
                mesh->bounds.at(i).dims.y,
                mesh->bounds.at(i).dims.z);
            }
            break;
            case cylinder:
            {
                fprintf(file, "%f %f\n",  
                mesh->bounds.at(i).dims.x,    // radius
                mesh->bounds.at(i).dims.y);   // height
            }
            break;
            case sphere:
            {
                fprintf(file, "%f\n",  
                mesh->bounds.at(i).dims.x);   // radius
            }
            break;
           }
        }
        fclose(file);
    }    
}
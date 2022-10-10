// Parser for Anim8or files

#ifndef IRRAN8_H_INCLUDED
#define IRRAN8_H_INCLUDED

#include <iostream>
#include <fstream>
#include <vector>
#include <stack>
#include <string>

using namespace std;

string ltrim(string l_string)
{
    if(l_string.find_first_not_of(" ") != string::npos)
        return l_string.substr(l_string.find_first_not_of(" "));
    return "";
}

string rtrim(string src)
{
    if(src.length()==0)
        return "";

    unsigned long i = 0;
    for(i = src.length()-1; ; i--)
    {
        if(src.substr(i,1).compare(" ") != 0 || i == 0)
            break;
    }
    return src.substr(0,i+1);
}

string trim(string t_string)
{
    return ltrim(rtrim(t_string));
}

struct an8_header
{
    std::string version;
    std::string build;
};

struct an8_description
{
    string text;
};

struct an8_grid
{
    int auto_grid;
    float model_grid_spacing;
    float scene_edit_grid_spacing;
    float ground_grid_size;
};

struct an8_environment
{
    an8_grid grid;
    int32_t framerate;
    int32_t limitplayback;
};

struct an8_texture
{
    string name;
    int32_t invert; //I am not actually sure what this is
    int32_t cubemap; //not sure what this is
    string file[6]; //will only use file[0] unless cubemap is non-zero
};

struct an8_color
{
    uint8_t r;
    uint8_t g;
    uint8_t b;
};

#define AN8_BLEND_MODE_DECAL 0
#define AN8_BLEND_MODE_DARKEN 1
#define AN8_BLEND_MODE_LIGHTEN 2

#define AN8_ALPHA_MODE_NONE 0
#define AN8_ALPHA_MODE_LAYER 1
#define AN8_ALPHA_MODE_FINAL 2

struct an8_texture_param
{
    uint8_t blend_mode;
    uint8_t alpha_mode;
    int32_t percent;
};

struct an8_surface_property
{
    an8_color color;
    float factor;
    string texturename;
    an8_texture_param textureparams;

};

struct an8_map
{
    string texturename;
    string kind;
    an8_texture_param textureparams;
};

struct an8_surface
{
    an8_surface_property ambient;
    an8_surface_property diffuse;
    an8_surface_property specular;
    an8_surface_property emissive;
    uint8_t alpha;
    float brilliance;
    float phongsize;
    std::vector<an8_map> surface_map;
    bool lockambdiff;
};

struct an8_material
{
    string name;
    an8_surface surface;
    an8_surface backsurface;
};

struct an8_point2i
{
    int32_t x;
    int32_t y;
};

struct an8_point2f
{
    double x;
    double y;
};

struct an8_point3f
{
    double x;
    double y;
    double z;
};

struct an8_point4f
{
    double x;
    double y;
    double z;
    double w;
};

struct an8_uv
{
    double u;
    double v;
};

struct an8_edge
{
    uint32_t point_index1;
    uint32_t point_index2;
    int32_t sharpness;
};

struct an8_pointdata
{
    int32_t point_index;
    int32_t mt_index; // index in normals or texcoords depending on value of flags
};

#define AN8_FACE_SF_SHOW_BACK   1
#define AN8_FACE_SF_HAS_NORMALS 2
#define AN8_FACE_SF_HAS_TEXTURE 4

struct an8_facedata
{
    int32_t num_points;
    uint8_t flags; //bitmask
    uint32_t material_index;
    int32_t flat_normal_index; //index in normal array or -1 if no value is stored in an8 file
    vector<an8_pointdata> point_data;
};

struct an8_base
{
    an8_point3f origin;
    an8_point4f orientation;
};

struct an8_mesh
{
    string name;

    an8_base base;
    an8_base pivot;
    an8_material material; //used for new faces added to mesh; only name is used
    float smoothangle;
    vector<string> materialname;
    vector<an8_point3f> points;
    vector<an8_point3f> normals;
    vector<an8_edge> edges; //mostly unused
    vector<an8_uv> texcoords;
    vector<an8_facedata> faces;
};

#define AN8_COMPONENT_TYPE_MESH         0
#define AN8_COMPONENT_TYPE_SPHERE       1
#define AN8_COMPONENT_TYPE_CYLINDER     2
#define AN8_COMPONENT_TYPE_CUBE         3
#define AN8_COMPONENT_TYPE_SUBDIVISION  4
#define AN8_COMPONENT_TYPE_PATH         5
#define AN8_COMPONENT_TYPE_TEXTCOM      6
#define AN8_COMPONENT_TYPE_MODIFIER     7
#define AN8_COMPONENT_TYPE_IMAGE        8
#define AN8_COMPONENT_TYPE_NAMEDOBJECT  9
#define AN8_COMPONENT_TYPE_GROUP        10

struct an8_component
{
    uint8_t type;
    an8_mesh mesh;
    //sphere
    //cylinder
    //cube
    //subdivision
    //path
    //textcom
    //modifier
    //image
    //namedobject
    //group

    an8_base base;
    an8_base pivot;
};

struct an8_object
{
    string name;
    std::vector<an8_material> material;
    std::vector<an8_component> component;
};

/*
struct an8_figure
{

};

struct an8_sequence
{

};

struct an8_scene
{

};
*/

struct an8_project
{
    an8_header header;
    an8_description description;
    an8_environment environment;
    vector<an8_texture> textures;
    vector<an8_material> materials;
    vector<an8_object> objects;

    //figures
    //sequences
    //scenes
};

struct an8_file_block
{
    int type;
    int start_index = -1;
    int end_index = -1;
    string name;
    string obj_name;
    string value;
    an8_file_block* parent;
    vector<an8_file_block> block;
};


void getHeader(an8_project* project, an8_file_block* block)
{
    if(block->block.size() == 0)
        return;

    an8_file_block* c_block;
    for(int i = 0; i < block->block.size(); i++)
    {
        c_block = &block->block[i];
        if(c_block->name.compare("header")==0)
        {
            for(int j = 0; j < c_block->block.size(); j++)
            {
                if(c_block->block[j].name.compare("version")==0)
                    project->header.version = c_block->block[j].obj_name;
                else if(c_block->block[j].name.compare("build")==0)
                    project->header.build = c_block->block[j].obj_name;

                //cout << "header attribute[" << c_block->block[j].name << "] = " << c_block->block[j].obj_name << endl;
            }
            break;
        }
    }

}


void getTextureParam(an8_texture_param* t_param, an8_file_block* block)
{
    if(block->block.size() == 0)
        return;

    an8_file_block* c_block;
    for(int i = 0; i < block->block.size(); i++)
    {
        c_block = &block->block[i];

        if(c_block->name.compare("blendmode")==0)
        {
            if(c_block->value.compare("decal")==0)
                t_param->blend_mode = AN8_BLEND_MODE_DECAL;
            else if(c_block->value.compare("darken")==0)
                t_param->blend_mode = AN8_BLEND_MODE_DARKEN;
            else if(c_block->value.compare("lighten")==0)
                t_param->blend_mode = AN8_BLEND_MODE_LIGHTEN;
        }
        else if(c_block->name.compare("alphamode")==0)
        {
            if(c_block->value.compare("none")==0)
                t_param->alpha_mode = AN8_ALPHA_MODE_NONE;
            else if(c_block->value.compare("layer")==0)
                t_param->alpha_mode = AN8_ALPHA_MODE_LAYER;
            else if(c_block->value.compare("final")==0)
                t_param->alpha_mode = AN8_ALPHA_MODE_FINAL;
        }
        else if(c_block->name.compare("percent")==0)
        {
            t_param->percent = atoi(c_block->value.c_str());
        }
    }

}


void getSurfaceProperty(an8_surface_property* surf_property, an8_file_block* block)
{
    if(block->block.size() == 0)
        return;

    an8_file_block* c_block;
    for(int i = 0; i < block->block.size(); i++)
    {
        c_block = &block->block[i];
        if(c_block->name.compare("rgb")==0)
        {
            string c_value = c_block->value + " "; //adding extra space for delimeter
            //cout << "c_value = " << c_block->value << endl;
            string c_str = "";
            uint8_t c[3];
            int ci = 0;
            for(int j = 0; j < c_value.length(); j++)
            {
                if(c_value.substr(j,1).compare(" ")==0)
                {
                    c[ci] = atoi(c_value.c_str());
                    ci++;

                    if(ci >= 3)
                        break;
                }
                //cout << "header attribute[" << c_block->block[j].name << "] = " << c_block->block[j].obj_name << endl;
            }

            an8_color color;
            color.r = c[0];
            color.g = c[1];
            color.b = c[2];
            //cout << "color = " << (int)c[0] << ", " << (int)c[1] << ", " << (int)c[2] << endl;
            surf_property->color = color;
        }
        else if(c_block->name.compare("factor")==0)
        {
            surf_property->factor = atof(c_block->value.c_str());
        }
        else if(c_block->name.compare("texturename")==0)
        {
            surf_property->texturename = c_block->obj_name;
        }
        else if(c_block->name.compare("textureparams")==0)
        {
            an8_texture_param t_param;

            getTextureParam(&t_param, c_block);

            surf_property->textureparams = t_param;
        }
    }

}

void getMap(an8_map* surface_map, an8_file_block* block)
{
    if(block->block.size() == 0)
        return;

    an8_file_block* c_block;
    for(int i = 0; i < block->block.size(); i++)
    {
        c_block = &block->block[i];
        if(c_block->name.compare("kind")==0)
        {
            surface_map->kind = c_block->obj_name;
        }
        else if(c_block->name.compare("texturename")==0)
        {
            surface_map->texturename = c_block->obj_name;
        }
        else if(c_block->name.compare("textureparams")==0)
        {
            an8_texture_param t_param;
            getTextureParam(&t_param, c_block);
            surface_map->textureparams = t_param;
        }
    }

}

void getSurface(an8_material* mat, an8_file_block* block)
{
    if(block->block.size() == 0)
        return;

    an8_file_block* c_block;
    for(int i = 0; i < block->block.size(); i++)
    {
        c_block = &block->block[i];
        if(c_block->name.compare("surface")==0 || c_block->name.compare("backsurface")==0)
        {
            an8_surface surface;

            for(int j = 0; j < c_block->block.size(); j++)
            {
                if(c_block->block[j].name.compare("ambient")==0)
                {
                    an8_surface_property surf_property;
                    getSurfaceProperty(&surf_property, &c_block->block[j]);
                    surface.ambient = surf_property;
                }
                else if(c_block->block[j].name.compare("diffuse")==0)
                {
                    an8_surface_property surf_property;
                    getSurfaceProperty(&surf_property, &c_block->block[j]);
                    surface.diffuse = surf_property;
                }
                else if(c_block->block[j].name.compare("specular")==0)
                {
                    an8_surface_property surf_property;
                    getSurfaceProperty(&surf_property, &c_block->block[j]);
                    surface.specular = surf_property;
                }
                else if(c_block->block[j].name.compare("emissive")==0)
                {
                    an8_surface_property surf_property;
                    getSurfaceProperty(&surf_property, &c_block->block[j]);
                    surface.emissive = surf_property;
                }
                else if(c_block->block[j].name.compare("alpha")==0)
                {
                    surface.alpha = (uint8_t)atoi(c_block->block[i].value.c_str());
                }
                else if(c_block->block[j].name.compare("brilliance")==0)
                {
                    surface.brilliance = atof(c_block->block[i].value.c_str());
                }
                else if(c_block->block[j].name.compare("phongsize")==0)
                {
                    surface.phongsize = atof(c_block->block[i].value.c_str());
                }
                else if(c_block->block[j].name.compare("map")==0)
                {
                    an8_map surface_map;
                    getMap(&surface_map, &c_block->block[j]);
                    surface.surface_map.push_back(surface_map);
                }
            }

            if(c_block->name.compare("backsurface")==0)
                mat->backsurface = surface;
            else
                mat->surface = surface;
        }
    }

}

void getMaterial(an8_object* obj, an8_file_block* block)
{
    if(block->block.size() == 0)
        return;

    an8_file_block* c_block;
    for(int i = 0; i < block->block.size(); i++)
    {
        c_block = &block->block[i];
        if(c_block->name.compare("material")==0)
        {
            an8_material mat;
            mat.name = c_block->obj_name;
            //cout << "material name: " << mat.name << endl;
            getSurface(&mat, c_block);
            obj->material.push_back(mat);
        }
    }

}

void getBase(an8_base* base, an8_file_block* block)
{
    if(block->block.size() == 0)
        return;

    an8_file_block* c_block;

    base->origin.x = 0;
    base->origin.y = 0;
    base->origin.z = 0;
    base->orientation.x = 0;
    base->orientation.y = 0;
    base->orientation.z = 0;
    base->orientation.w = 0;

    for(int i = 0; i < block->block.size(); i++)
    {
        c_block = &block->block[i];

        if(c_block->name.compare("origin")==0)
        {
            string token = "";
            double v_point[3];
            v_point[0] = 0;
            v_point[1] = 0;
            v_point[2] = 0;
            int vi = 0;
            string c_value = c_block->value + " ";

            //cout << "origin c_Value = " << c_value << endl;

            for(int n = 0; n < c_value.length(); n++)
            {
                string c = c_value.substr(n, 1);

                if(c.compare("(")==0 || c.compare(")")==0 )
                    continue;
                else if(c.compare(" ")==0)
                {
                    v_point[vi] = atof(token.c_str());
                    vi++;
                    token = "";

                    if(vi >= 3)
                        break;
                }
                else
                    token += c;
            }

            base->origin.x = v_point[0];
            base->origin.y = v_point[1];
            base->origin.z = v_point[2];
        }
        else if(c_block->name.compare("orientation")==0)
        {
            string token = "";
            double v_point[4];
            v_point[0] = 0;
            v_point[1] = 0;
            v_point[2] = 0;
            v_point[3] = 0;
            int vi = 0;
            string c_value = c_block->value + " ";

            for(int n = 0; n < c_value.length(); n++)
            {
                string c = c_value.substr(n, 1);

                if(c.compare("(")==0 || c.compare(")")==0 )
                    continue;
                else if(c.compare(" ")==0)
                {
                    v_point[vi] = atof(token.c_str());
                    vi++;
                    token = "";

                    if(vi >= 4)
                        break;
                }
                else
                    token += c;
            }

            base->orientation.x = v_point[0];
            base->orientation.y = v_point[1];
            base->orientation.z = v_point[2];
            base->orientation.w = v_point[3];
        }
    }

}


void getMaterialList(vector<string>* mat_list, an8_file_block* block)
{
    if(block->block.size() == 0)
        return;

    an8_file_block* c_block;
    for(int i = 0; i < block->block.size(); i++)
    {
        c_block = &block->block[i];
        if(c_block->name.compare("materialname")==0)
        {
            //cout << "material list add -> " << c_block->obj_name << endl;
            mat_list->push_back(c_block->obj_name);
        }
    }

}

void getPoints3f(vector<an8_point3f>* points, an8_file_block* block)
{
    double v[3];
    int vi = 0;
    string arg = "";

    bool in_scope = false;

    for(int i = 0; i < block->value.length(); i++)
    {
        string c = block->value.substr(i,1);

        if(c.compare("(")==0)
        {
            in_scope = true;
            arg = "";
            vi = 0;
            v[0] = 0;
            v[1] = 0;
            v[2] = 0;
        }
        else if(c.compare(")")==0)
        {
            in_scope = false;

            if(vi < 3)
            {
                v[2] = atof(arg.c_str());
            }

            an8_point3f p;
            p.x = v[0];
            p.y = v[1];
            p.z = v[2];
            points->push_back(p);
        }
        else if(c.compare(" ")==0 && in_scope)
        {
            v[vi] = atof(arg.c_str());
            arg = "";
            vi++;
        }
        else
            arg += c;
    }

}

void getUV(vector<an8_uv>* texcoords, an8_file_block* block)
{
    double v[2];
    int vi = 0;
    string arg = "";

    bool in_scope = false;

    for(int i = 0; i < block->value.length(); i++)
    {
        string c = block->value.substr(i,1);

        if(c.compare("(")==0)
        {
            in_scope = true;
            arg = "";
            vi = 0;
            v[0] = 0;
            v[1] = 0;
        }
        else if(c.compare(")")==0)
        {
            in_scope = false;

            if(vi < 2)
            {
                v[1] = atof(arg.c_str());
            }

            //cout << "UV [" << texcoords->size() << "] = " << v[0] << ", " << v[1] << endl;

            an8_uv t;
            t.u = v[0];
            t.v = v[1];
            texcoords->push_back(t);
        }
        else if(c.compare(" ")==0 && in_scope)
        {
            v[vi] = atof(arg.c_str());
            arg = "";
            vi++;
        }
        else
            arg += c;
    }

}

void getPoints2i(vector<an8_point2i>* points, an8_file_block* block)
{
    int32_t v[2];
    int vi = 0;
    string arg = "";

    bool in_scope = false;

    for(int i = 0; i < block->value.length(); i++)
    {
        string c = block->value.substr(i,1);

        if(c.compare("(")==0)
        {
            in_scope = true;
            arg = "";
            vi = 0;
            v[0] = 0;
            v[1] = 0;
        }
        else if(c.compare(")")==0)
        {
            in_scope = false;

            if(vi < 2)
            {
                v[1] = atoi(arg.c_str());
            }

            an8_point2i p;
            p.x = v[0];
            p.y = v[1];
            points->push_back(p);
        }
        else if(c.compare(" ")==0 && in_scope)
        {
            v[vi] = atoi(arg.c_str());
            arg = "";
            vi++;
        }
        else
            arg += c;
    }

}

/*
#define AN8_FACE_SF_SHOW_BACK   1
#define AN8_FACE_SF_HAS_NORMALS 2
#define AN8_FACE_SF_HAS_TEXTURE 4

struct an8_facedata
{
    int32_t num_points;
    uint8_t flags; //bitmask
    uint32_t material_index;
    int32_t flat_normal_index; //index in normal array or -1 if no value is stored in an8 file
    vector<an8_pointdata> point_data;
};
*/

void getFaces(vector<an8_facedata>* faces, an8_file_block* block)
{
    int v[4];

    int f_data[4];
    vector<an8_pointdata> point_data;

    int vi = 0;
    int fi = 0;
    string arg = "";

    bool in_pdata = false;
    bool in_scope = false;

    for(int i = 0; i < block->value.length(); i++)
    {
        string c = block->value.substr(i,1);

        if(c.compare("(")==0)
        {
            if(in_pdata)
            {
                in_scope = true;
                arg = "";
                vi = 0;
                v[0] = 0;
                v[1] = 0;
            }
            else
                in_pdata = true;
        }
        else if(c.compare(")")==0)
        {
            if(in_scope)
            {
                in_scope = false;

                if(vi < 2)
                {
                    v[1] = atoi(arg.c_str());
                }

                an8_pointdata p;
                p.point_index = v[0];
                p.mt_index = v[1];
                point_data.push_back(p);
            }
            else
            {
                in_pdata = false;
                fi = 0;
                vi = 0;

                an8_facedata face;
                face.num_points = f_data[0];
                face.flags = f_data[1];
                face.material_index = f_data[2];
                face.flat_normal_index = f_data[3];

                for(int fp = 0; fp < point_data.size(); fp++)
                    face.point_data.push_back(point_data[fp]);


                /*
                if(faces->size() < 5)
                {
                    cout << "Face [" << faces->size() << "] = " <<
                            (int)face.num_points << ", " <<
                            (int)face.flags << ", " <<
                            (int)face.material_index << ", " <<
                            (int)face.flat_normal_index << ", ( ";

                    for(int fp = 0; fp < face.point_data.size(); fp++)
                        cout << "(" << (int)face.point_data[fp].point_index << " " <<
                                       (int)face.point_data[fp].mt_index << ") ";

                    cout << ")" << endl;
                }
                */

                faces->push_back(face);

                point_data.clear();
                arg = "";
            }
        }
        else if(c.compare(" ")==0 && trim(arg).compare("")!=0)
        {
            if(in_pdata && in_scope)
            {
                v[vi] = atoi(arg.c_str());
                arg = "";
                vi++;
            }
            else if(!in_pdata)
            {
                if(fi >= 4)
                    continue;

                //if(faces->size() < 5)
                //    cout << "f_Data insert [" << arg << "]" << endl;
                f_data[fi] = atoi(arg.c_str());
                arg = "";
                fi++;
            }
        }
        else
            arg += c;
    }

}

void getMesh(an8_mesh* mesh, an8_file_block* block)
{
    if(block->block.size() == 0)
        return;

    an8_file_block* c_block;
    for(int i = 0; i < block->block.size(); i++)
    {
        c_block = &block->block[i];

        if(c_block->name.compare("name")==0)
        {
            mesh->name = c_block->obj_name;
        }
        else if(c_block->name.compare("base")==0)
        {
            an8_base base;
            getBase(&base, c_block);
            mesh->base = base;
        }
        else if(c_block->name.compare("pivot")==0)
        {
            an8_base base;
            getBase(&base, c_block);
            mesh->pivot = base;
        }
        else if(c_block->name.compare("material")==0)
        {
            an8_material mat;
            mat.name = c_block->obj_name;
            getSurface(&mat, c_block);
            mesh->material = mat;
        }
        else if(c_block->name.compare("smoothangle")==0)
        {
            mesh->smoothangle = atof(c_block->value.c_str());
        }
        else if(c_block->name.compare("materiallist")==0)
        {
            getMaterialList(&mesh->materialname, c_block);
        }
        else if(c_block->name.compare("points")==0)
        {
            //cout << "load points: " << mesh->name << endl;
            getPoints3f(&mesh->points, c_block);
        }
        else if(c_block->name.compare("texcoords")==0)
        {
            //cout << "load points: " << mesh->name << endl;
            //if(mesh->name.compare("mesh0145")==0)
            getUV(&mesh->texcoords, c_block);
        }
        else if(c_block->name.compare("faces")==0)
        {
            //if(mesh->name.compare("mesh0145")==0)
            getFaces(&mesh->faces, c_block);
        }
    }

}

void getComponent(an8_object* obj, an8_file_block* block)
{
    if(block->block.size() == 0)
        return;

    an8_file_block* c_block;
    for(int i = 0; i < block->block.size(); i++)
    {
        c_block = &block->block[i];
        if(c_block->name.compare("mesh")==0)
        {
            an8_mesh mesh;
            mesh.base.origin.x = 0;
            mesh.base.origin.y = 0;
            mesh.base.origin.z = 0;
            mesh.base.orientation.x = 0;
            mesh.base.orientation.y = 0;
            mesh.base.orientation.z = 0;
            mesh.base.orientation.w = 0;

            getMesh(&mesh, c_block);

            an8_component component;
            component.mesh = mesh;
            component.type = AN8_COMPONENT_TYPE_MESH;

            obj->component.push_back(component);
        }
    }

}

void getObject(an8_project* project, an8_file_block* block)
{
    if(block->block.size() == 0)
        return;

    an8_file_block* c_block;
    for(int i = 0; i < block->block.size(); i++)
    {
        c_block = &block->block[i];
        if(c_block->name.compare("object")==0)
        {
            //cout << "obj_name = " << c_block->obj_name << endl;
            an8_object obj;
            obj.name = c_block->obj_name;
            getMaterial(&obj, c_block);
            getComponent(&obj, c_block);
            project->objects.push_back(obj);
        }

        //if(c_block->name.length() < 30)
          //  cout << "cblock_name = [" << c_block->name << "]" << endl;
    }

}

void loadAN8(std::string an8_project_file)
{
    an8_project project;
    int an8_file_scope = 0;
    string an8_file_content;

    // Read File
    fstream f;
    f.open(an8_project_file.c_str(), fstream::in);

    string f_line = "";

    while( !f.eof() )
    {
        getline(f, f_line);
        an8_file_content += f_line + "\n";
    }

    f.close();
    // End Read File


    int curly_scope = 0;
    string token;
    an8_file_block block;
    block.name = "AN8_PROJECT";
    block.value = "AN8_PROJECT";
    block.parent = NULL;
    an8_file_block* c_block = &block;

    vector<int> block_index;

    an8_header header;

    for(int i = 0; i < an8_file_content.length(); i++)
    {
        string c = an8_file_content.substr(i,1);
        string start_comment = an8_file_content.substr(i,2);

        if(c.compare("\n")==0 || c.compare("\r")==0)
            continue;
        else if(c.compare("\"")==0)
        {
            string value = an8_file_content.substr(i+1);
            int close_str = value.find_first_of("\"");
            value = value.substr(0, close_str);
            c_block->obj_name = value;
            i += close_str+1;
            continue;
        }
        else if(start_comment.compare("/*")==0)
        {
            bool end_comment = false;
            for(; i < an8_file_content.length(); i++)
            {
                if(end_comment)
                    break;

                if(an8_file_content.substr(i,2).compare("*/")==0)
                    end_comment = true;
            }
            continue;
        }
        else if(c.compare("{")==0)
        {
            an8_file_block b;
            b.name = trim(token);
            b.start_index = i+1;
            b.parent = c_block;

            c_block->block.push_back(b);

            int new_index = c_block->block.size()-1;
            block_index.push_back(new_index);

            c_block = &c_block->block[new_index];

            token = "";
            curly_scope++;
        }
        else if(c.compare("}")==0)
        {
            //if(c_block->name.compare("rgb")==0)
            //    cout << "rgb: " << token << endl;

            c_block->value = trim(token);

            block_index.pop_back();

            c_block = &block;

            for(int bi = 0; bi < block_index.size(); bi++)
                c_block = &c_block->block[block_index[bi]];


            token = "";
            curly_scope--;
        }
        else
            token += c;

    }

    getHeader(&project, &block);
    getObject(&project, &block);

    cout <<"Num Objects = " << project.objects.size() << endl << endl;

    for(int i = 0; i < project.objects.size(); i++)
    {
        cout << project.objects[i].name << endl;

        for(int m = 0; m < project.objects[i].material.size(); m++)
        {
            cout << "diffuse = ";
            cout << (int)project.objects[i].material[m].surface.diffuse.color.r << ", " <<
                    (int)project.objects[i].material[m].surface.diffuse.color.g << ", " <<
                    (int)project.objects[i].material[m].surface.diffuse.color.b << endl;

            cout << "specular = ";
            cout << (int)project.objects[i].material[m].surface.specular.color.r << ", " <<
                    (int)project.objects[i].material[m].surface.specular.color.g << ", " <<
                    (int)project.objects[i].material[m].surface.specular.color.b << endl;

            for(int s_map = 0; s_map < project.objects[i].material[m].surface.surface_map.size(); s_map++)
            {
                cout << "map[" << s_map << "]" << endl;
                cout << "    kind = " << project.objects[i].material[m].surface.surface_map[s_map].kind << endl;
                cout << "    tx_name = " << project.objects[i].material[m].surface.surface_map[s_map].texturename << endl;
                cout << "    blend mode = " << (int)project.objects[i].material[m].surface.surface_map[s_map].textureparams.blend_mode << endl;
                cout << "    alpha mode = " << (int)project.objects[i].material[m].surface.surface_map[s_map].textureparams.alpha_mode << endl;
            }
        }

        for(int cmp_index = 0; cmp_index < project.objects[i].component.size(); cmp_index++)
        {
            an8_component cmpt = project.objects[i].component[cmp_index];

            switch(cmpt.type)
            {
                case AN8_COMPONENT_TYPE_MESH:
                    cout << "Mesh data[" << cmp_index << "]: " << cmpt.mesh.name << endl;
                    cout << "    base.origin = " << cmpt.mesh.base.origin.x << ", "
                                                 << cmpt.mesh.base.origin.y << ", "
                                                 << cmpt.mesh.base.origin.z << endl;

                    cout << "    base.orientation = " << cmpt.mesh.base.orientation.x << ", "
                                                      << cmpt.mesh.base.orientation.y << ", "
                                                      << cmpt.mesh.base.orientation.z << ", "
                                                      << cmpt.mesh.base.orientation.w << endl;

                    an8_mesh mesh = cmpt.mesh;

                    for(int mesh_a = 0; mesh_a < mesh.materialname.size(); mesh_a++)
                    {
                        cout << "    material_name = " << mesh.materialname[mesh_a] << endl;
                    }

                    break;
            }
        }

        cout << endl;
    }

}

#endif // IRRAN8_H_INCLUDED

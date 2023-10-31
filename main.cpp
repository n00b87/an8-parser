#include <iostream>
#include <irrlicht/irrlicht.h>
//#include "CSkinnedMesh.h"
#include "an8parser.h"
#include <cmath>


using namespace irr;
using namespace core;
using namespace scene;
using namespace video;
using namespace io;
using namespace gui;


void setAN8Track(an8::an8_element mesh_element)
{
    std::cout << "\n---TRACKS---\n";
    for(int n = 0; n < mesh_element.controller.size(); n++)
    {
        an8::an8_controller controller = mesh_element.controller[n];
        std::cout << "c[" << n << "] = " << controller.track.pointkey.size() << std::endl;
        if(controller.track.track_type == TRACK_TYPE_POINT)
        {
            for(int pkey_i = 0; pkey_i < controller.track.pointkey.size(); pkey_i++)
            {
                an8::an8_pointkey pkey = controller.track.pointkey[pkey_i];
                std::cout << pkey.frame_index << ": " << pkey.value.x << ", " << pkey.value.y << ", " << pkey.value.z << std::endl;
            }
        }
    }
}

bool getAN8Figure(an8::an8_project* p, std::string figure_name, an8::an8_figure* figure)
{
    for(int i = 0; i < p->figures.size(); i++)
    {
        if(p->figures[i].name.compare(figure_name)==0)
        {
            *figure = p->figures[i];
            return true;
        }
    }
    return false;
}

bool getAN8Object(an8::an8_project* p, std::string obj_name, an8::an8_object* obj)
{
    for(int i = 0; i < p->objects.size(); i++)
    {
        if(p->objects[i].name.compare(obj_name)==0)
        {
            *obj = p->objects[i];
            return true;
        }
    }
    return false;
}

//get all the mesh components of an object
void getAN8MeshList(an8::an8_project* p, vector<an8::an8_component>* componentList, std::vector<an8::an8_mesh>* meshList, an8::an8_irr_joint_data* joint_data)
{
    for(int c_index = 0; c_index < componentList->size(); c_index++)
    {
        an8::an8_component c = (*componentList)[c_index];

        if(c.type == AN8_COMPONENT_TYPE_MESH)
        {
            int current_namedobject_index = joint_data->namedobject.size()-1;
            if(current_namedobject_index >= 0)
                joint_data->namedobject[current_namedobject_index].meshList_index.push_back(meshList->size()); //store the index being pushed
            else
                std::cout << "No named object to add mesh index to" << std::endl;

            an8::an8_mesh mesh = c.mesh;
            meshList->push_back(mesh);
        }
        else if(c.type == AN8_COMPONENT_TYPE_NAMEDOBJECT)
        {
            joint_data->namedobject.push_back(c.named_object);

            an8::an8_object obj;
            getAN8Object(p, c.named_object.name, &obj);
            getAN8MeshList(p, &obj.component, meshList, joint_data);
        }
    }
}


scene::SSkinMeshBuffer* addAN8MeshBuffer(an8::an8_project* p, scene::ISkinnedMesh* AnimatedMesh, an8::an8_mesh mesh)
{
    scene::SSkinMeshBuffer* meshBuffer = AnimatedMesh->addMeshBuffer();

    std::cout << "Mesh Info: " << mesh.points.size() << ", " << mesh.texcoords.size() << ", " << mesh.normals.size() << std::endl;

    core::array<video::S3DVertex> vertices;
    vertices.clear();

    video::S3DVertex v;

    //if(mesh.name.compare("test_mesh")!=0)
      //  return false;

    an8::an8_point3f default_normal;
    default_normal.x = 0;
    default_normal.y = 0;
    default_normal.z = 0;

    an8::an8_uv default_texture;
    default_texture.u = 0;
    default_texture.v = 0;

    core::array<irr::u16> indices;
    indices.clear();

    for(int i = 0; i < mesh.faces.size(); i++)
    {
        an8::an8_facedata face = mesh.faces[i];

        int vbuffer_start_index = vertices.size();

        for(int point_index = 0; point_index < face.num_points; point_index++)
        {
            int pi = face.point_data[point_index].point_index;
            an8::an8_point3f point = mesh.points[pi];

            //if(i == 1)
            //std::cout << "face_point[" << point_index << "] = ("
            //            << point.x << ", "
            //            << point.y << ", "
            //            << point.z << ") " << std::endl;

            int normal_index = face.point_data[point_index].normal_index;
            an8::an8_point3f normal = default_normal;

            if(normal_index >= 0)
            {
                normal = mesh.normals[normal_index];
            }

            int texture_index = face.point_data[point_index].texture_index;
            an8::an8_uv texture = default_texture;

            if(texture_index >= 0)
            {
                texture = mesh.texcoords[texture_index];

                //if(i == 4)
                //std::cout << "(" << texture.u << ", " << texture.v << ")" << std::endl << std::endl;
            }

            v.Pos.set(point.x, point.y, point.z);
            //v.Normal.set(normal.x, normal.y, normal.z);
            v.TCoords.set(texture.u, 1-texture.v);
            v.Color.set(255,255,255,255);

            vertices.push_back(v);

            //Note: I could optimize a little by giving each vertex and id and only adding it if its
            //      not already in the array but I am not doing it because I want to set multiple normals
            //      for each vertex since anim8or outputs different normals depending on the face


            //On the fly triangulation
            if(point_index >= 2)
            {
                int vbuffer_index = vbuffer_start_index + point_index;

                indices.push_back(vbuffer_start_index);
                indices.push_back(vbuffer_index-1);
                indices.push_back(vbuffer_index);
            }

            //if(i == 2760)
            //    std::cout << "face_mt[" << point_index << "] = "
            //            << face.point_data[point_index].normal_index << ", "
            //            << face.point_data[point_index].texture_index << std::endl << std::endl;

        }
        //break;


    }

    if(indices.size() > 0)
    {
        //std::cout << "ADD VERTS TO BUFFER: " << indices.size() << std::endl << std::endl;

        for(int i = 0; i < vertices.size(); i++)
            meshBuffer->Vertices_Standard.push_back(vertices[i]);

        for(int i = 0; i < indices.size(); i++)
            meshBuffer->Indices.push_back(indices[i]);

        //meshBuffer->append((video::S3DVertex*)&vertices[0], vertices.size(), (irr::u16*)&indices[0], indices.size());
        //std::cout << "MeshBuffer Size = " << AnimatedMesh->getMeshBuffer(0)->getIndexCount() << std::endl << std::endl;
    }
    else
    {
        meshBuffer->drop();
        return NULL;
    }


    return meshBuffer;
}

//get the bone data for a figure
bool getAN8BoneNode(an8::an8_project* p, scene::ISkinnedMesh* AnimatedMesh, scene::ISkinnedMesh::SJoint* inJoint, an8::an8_bone bone, an8::an8_bone parent_bone)
{
	scene::ISkinnedMesh::SJoint *joint = AnimatedMesh->addJoint(inJoint);
	joint->Name = bone.name.c_str();

	f32 position[3], scale[3], rotation[4];


	//The root bone won't have a position despite whats in the *.an8 file
	joint->LocalMatrix.makeIdentity();

	if(inJoint)
    {
        inJoint->Children.push_back(joint);
        //Set Global Transform
        joint->LocalMatrix.setRotationDegrees(inJoint->LocalMatrix.getRotationDegrees());
        joint->LocalMatrix.setTranslation(inJoint->LocalMatrix.getTranslation());

        //Set Local Transform
        core::vector3df tvec(0,parent_bone.length,0);
        core::quaternion orient(parent_bone.orientation.x,
                                parent_bone.orientation.y,
                                parent_bone.orientation.z,
                                parent_bone.orientation.w);
        core::vector3df rvec;
        orient.toEuler(rvec);
        tvec.rotateXYBy(rvec.Z);
        tvec.rotateYZBy(rvec.X);
        tvec.rotateXZBy(rvec.Y);

        orient = core::quaternion(bone.orientation.x,
                                  bone.orientation.y,
                                  bone.orientation.z,
                                  bone.orientation.w);
        orient.toEuler(rvec);

        joint->LocalMatrix.setTranslation(tvec);
        joint->LocalMatrix.setRotationDegrees(rvec);
    }



	//Build LocalMatrix:
	//core::matrix4 positionMatrix;
	//positionMatrix.setTranslation( joint->Animatedposition );

	//core::matrix4 rotationMatrix;
	//joint->Animatedrotation.getMatrix_transposed(rotationMatrix);

	//joint->LocalMatrix = positionMatrix * rotationMatrix;

	//if (inJoint)
		//joint->GlobalMatrix = inJoint->GlobalMatrix * joint->LocalMatrix;
	//else
    //joint->GlobalMatrix = joint->LocalMatrix;

    //----------DEBUG STUFF-----------------
    core::vector3df v = joint->LocalMatrix.getTranslation();
    core::vector3df rv = joint->LocalMatrix.getRotationDegrees();
    //v.rotateXYBy(-1 * joint->LocalMatrix.getRotationDegrees().Z);
    //v.rotateYZBy(joint->LocalMatrix.getRotationDegrees().X);
    //v.rotateXZBy(joint->LocalMatrix.getRotationDegrees().Y);

    core::vector3df gv = joint->LocalMatrix.getTranslation();
    core::matrix4 nvm;

    std::string pbname = "NULL";

    if(inJoint)
    {
        pbname = std::string(inJoint->Name.c_str());
        gv = inJoint->GlobalMatrix.getTranslation();
        core::matrix4 nvm = inJoint->GlobalMatrix;
    }

    core::vector3df nv = nvm.getTranslation();

    std::cout << "DEBUG VECTOR[" << bone.name << " in " << pbname << "]: " << v.X << ", " << v.Y << ", " << v.Z << std::endl
                                  //<< gv.X << ", " << gv.Y << ", " << gv.Z << std::endl
                                  << rv.X << ", " << rv.Y << ", " << rv.Z << std::endl << std::endl;
    //----------DEBUG STUFF-----------------

    //Get Meshes  NOTE: a bone can have multiple meshes
    an8::an8_object obj;
    std::vector<an8::an8_mesh> meshList;

    an8::an8_irr_joint_data joint_data;
    joint_data.joint = joint;
    joint_data.bone = bone;

    getAN8MeshList(p, &bone.component, &meshList, &joint_data);


    //std::cout << "test on " << bone.name << ", " << bone.component.size() << std::endl;

    scene::SSkinMeshBuffer* meshBuffer;



    for(int i = 0; i < meshList.size(); i++)
    {
        std::cout << "bone[" << bone.name << "] -> " << meshList[i].name << std::endl;

        meshBuffer = addAN8MeshBuffer(p, AnimatedMesh, meshList[i]);

        if(meshBuffer)
        {
            //calculate bone influence for vertices
            meshList[i].meshBuffer = meshBuffer;

            an8::an8_namedobject n = joint_data.namedobject[0];
            an8::an8_weightedBy w = n.weightedBy[0];
            std::string bone_name = w.bone_name;

            for(int n = 0; n < meshBuffer->Vertices_Standard.size(); n++)
            {
                video::S3DVertex vert = meshBuffer->Vertices_Standard[n];
                core::vector3df point_vector = vert.Pos;

                //point_vector = point_vector;
            }


        }
    }

    //string obj_name =


    //Get Keys

    //Get Animations

    //Add Child Bones

    //std::cout << "start: " << bone.name << ": " << bone.bone.size() << std::endl;
    for(int i = 0; i < bone.bone.size(); i++)
    {
        getAN8BoneNode(p, AnimatedMesh, joint, bone.bone[i], bone);
    }
    //std::cout << "end: " << bone.name << std::endl;

    return true;
}


void loadAN8Mesh(an8::an8_mesh a_mesh, scene::ISkinnedMesh* mesh)
{

}


scene::IAnimatedMesh* loadAN8Scene(IrrlichtDevice* device, an8::an8_project a_file, std::string scene_name)
{
    int scene_index = -1;

    for(int i = 0; i < a_file.scenes.size(); i++)
    {
        if(a_file.scenes[i].name.compare(scene_name)==0)
        {
            scene_index = i;
            break;
        }
    }

    if(scene_index < 0)
        return NULL;

    an8::an8_scene mesh_scene = a_file.scenes[scene_index];


    scene::ISceneManager* smgr = device->getSceneManager();

    scene::ISkinnedMesh* mesh = smgr->createSkinnedMesh();

    //tmp variables
    an8::an8_element element;
    an8::an8_figure figure;
    an8::an8_object obj;

    std::vector<an8::an8_mesh> meshList;


    //Make magic happen
    for(int i = 0; i < mesh_scene.elements.size(); i++)
    {
        std::cout << "i = " << mesh_scene.elements[i].name << std::endl;

        element = mesh_scene.elements[i];

        switch(mesh_scene.elements[i].element_type)
        {
            case ELEMENT_TYPE_FIGURE:

                std::cout << "obj_name = " << element.obj_name << std::endl;

                if(!getAN8Figure(&a_file, element.obj_name, &figure))
                    continue;

                a_file.node_figure = figure;

                getAN8BoneNode(&a_file, mesh, (scene::ISkinnedMesh::SJoint*)0, figure.bone, figure.bone);




                //setAN8Track(mesh_scene.elements[i]);
                break;

            case ELEMENT_TYPE_OBJECT:

                std::cout << "obj_name = " << element.obj_name << std::endl;

                break;
        }
    }



    mesh->finalize();

    irr::scene::IMeshManipulator *meshManip = device->getSceneManager()->getMeshManipulator();
    meshManip->flipSurfaces(mesh);

    return mesh;
}









enum
{
	// I use this ISceneNode ID to indicate a scene node that is
	// not pickable by getSceneNodeAndCollisionPointFromRay()
	ID_IsNotPickable = 0,

	// I use this flag in ISceneNode IDs to indicate that the
	// scene node can be picked by ray selection.
	IDFlag_IsPickable = 1 << 0,

	// I use this flag in ISceneNode IDs to indicate that the
	// scene node can be highlighted.  In this example, the
	// homonids can be highlighted, but the level mesh can't.
	IDFlag_IsHighlightable = 1 << 1
};

int main()
{
	// ask user for driver
	video::E_DRIVER_TYPE driverType=EDT_OPENGL;

	// create device
	IrrlichtDevice *device =
		createDevice(driverType, core::dimension2d<u32>(640, 480), 16, false);

	if (device == 0)
		return 1; // could not create selected driver.

	video::IVideoDriver* driver = device->getVideoDriver();
	scene::ISceneManager* smgr = device->getSceneManager();

	device->getFileSystem()->addFileArchive("media/map-20kdm2.pk3");

	scene::IAnimatedMesh* q3levelmesh = smgr->getMesh("20kdm2.bsp");
	scene::IMeshSceneNode* q3node = 0;

	// The Quake mesh is pickable, but doesn't get highlighted.
	if (q3levelmesh)
		q3node = smgr->addOctreeSceneNode(q3levelmesh->getMesh(0), 0, IDFlag_IsPickable);

	/*
	So far so good, we've loaded the quake 3 level like in tutorial 2. Now,
	here comes something different: We create a triangle selector. A
	triangle selector is a class which can fetch the triangles from scene
	nodes for doing different things with them, for example collision
	detection. There are different triangle selectors, and all can be
	created with the ISceneManager. In this example, we create an
	OctreeTriangleSelector, which optimizes the triangle output a little
	bit by reducing it like an octree. This is very useful for huge meshes
	like quake 3 levels. After we created the triangle selector, we attach
	it to the q3node. This is not necessary, but in this way, we do not
	need to care for the selector, for example dropping it after we do not
	need it anymore.
	*/

	scene::ITriangleSelector* selector = 0;

	if (q3node)
	{
		q3node->setPosition(core::vector3df(-1350,-130,-1400));

		selector = smgr->createOctreeTriangleSelector(
				q3node->getMesh(), q3node, 128);
		q3node->setTriangleSelector(selector);
		// We're not done with this selector yet, so don't drop it.
	}


	/*
	We add a first person shooter camera to the scene so that we can see and
	move in the quake 3 level like in tutorial 2. But this, time, we add a
	special animator to the camera: A Collision Response animator. This
	animator modifies the scene node to which it is attached to in order to
	prevent it moving through walls, and to add gravity to it. The
	only thing we have to tell the animator is how the world looks like,
	how big the scene node is, how much gravity to apply and so on. After the
	collision response animator is attached to the camera, we do not have to do
	anything more for collision detection, anything is done automatically.
	The rest of the collision detection code below is for picking. And please
	note another cool feature: The collision response animator can be
	attached also to all other scene nodes, not only to cameras. And it can
	be mixed with other scene node animators. In this way, collision
	detection and response in the Irrlicht engine is really easy.

	Now we'll take a closer look on the parameters of
	createCollisionResponseAnimator(). The first parameter is the
	TriangleSelector, which specifies how the world, against collision
	detection is done looks like. The second parameter is the scene node,
	which is the object, which is affected by collision detection, in our
	case it is the camera. The third defines how big the object is, it is
	the radius of an ellipsoid. Try it out and change the radius to smaller
	values, the camera will be able to move closer to walls after this. The
	next parameter is the direction and speed of gravity.  We'll set it to
	(0, -10, 0), which approximates to realistic gravity, assuming that our
	units are metres. You could set it to (0,0,0) to disable gravity. And the
	last value is just a translation: Without this, the ellipsoid with which
	collision detection is done would be around the camera, and the camera would
	be in the middle of the ellipsoid. But as human beings, we are used to have our
	eyes on top of the body, with which we collide with our world, not in
	the middle of it. So we place the scene node 50 units over the center
	of the ellipsoid with this parameter. And that's it, collision
	detection works now.
	*/

	// Set a jump speed of 3 units per second, which gives a fairly realistic jump
	// when used with the gravity of (0, -10, 0) in the collision response animator.
	scene::ICameraSceneNode* camera =
		smgr->addCameraSceneNodeFPS(0, 100.0f, .3f, ID_IsNotPickable, 0, 0, true, 3.f);
	camera->setPosition(core::vector3df(50,50,-60));
	camera->setTarget(core::vector3df(-70,30,-60));

	if (selector)
	{
		scene::ISceneNodeAnimator* anim = smgr->createCollisionResponseAnimator(
			selector, camera, core::vector3df(30,50,30),
			core::vector3df(0,-10,0), core::vector3df(0,30,0));
		selector->drop(); // As soon as we're done with the selector, drop it.
		camera->addAnimator(anim);
		anim->drop();  // And likewise, drop the animator when we're done referring to it.
	}

	// Now I create three animated characters which we can pick, a dynamic light for
	// lighting them, and a billboard for drawing where we found an intersection.

	// First, let's get rid of the mouse cursor.  We'll use a billboard to show
	// what we're looking at.
	device->getCursorControl()->setVisible(false);

	// Add the billboard.
	scene::IBillboardSceneNode * bill = smgr->addBillboardSceneNode();
	bill->setMaterialType(video::EMT_TRANSPARENT_ADD_COLOR );
	bill->setMaterialTexture(0, driver->getTexture("media/particle.bmp"));
	bill->setMaterialFlag(video::EMF_LIGHTING, false);
	bill->setMaterialFlag(video::EMF_ZBUFFER, false);
	bill->setSize(core::dimension2d<f32>(20.0f, 20.0f));
	bill->setID(ID_IsNotPickable); // This ensures that we don't accidentally ray-pick it

	/* Add 3 animated hominids, which we can pick using a ray-triangle intersection.
	They all animate quite slowly, to make it easier to see that accurate triangle
	selection is being performed. */
	scene::IAnimatedMeshSceneNode* node = 0;

	video::SMaterial material;


	an8::an8_project p = an8::loadAN8("/home/n00b/Projects/an8_irrlicht/assets/test2.an8");

	scene::IAnimatedMesh* test_mesh = loadAN8Scene(device, p, "scene01");

	// Add an MD2 node, which uses vertex-based animation.
	node = smgr->addAnimatedMeshSceneNode(test_mesh,
						0, IDFlag_IsPickable | IDFlag_IsHighlightable);
	node->setPosition(core::vector3df(-90,-15,-140)); // Put its feet on the floor.
	node->setScale(core::vector3df(1.6f)); // Make it appear realistically scaled
	//node->setMD2Animation(scene::EMAT_POINT);
	//node->setAnimationSpeed(20.f);
	material.setTexture(0, driver->getTexture("assets/knight_f_texture.bmp"));
	material.Lighting = false;
	//material.NormalizeNormals = true;
	node->getMaterial(0) = material;

	// Now create a triangle selector for it.  The selector will know that it
	// is associated with an animated node, and will update itself as necessary.
	selector = smgr->createTriangleSelector(node);
	node->setTriangleSelector(selector);
	selector->drop(); // We're done with this selector, so drop it now.








	// Just do the same as we did above.
	selector = smgr->createTriangleSelector(node);
	node->setTriangleSelector(selector);
	selector->drop();

	material.setTexture(0, 0);
	material.Lighting = false;

	// Add a light, so that the unselected nodes aren't completely dark.
	scene::ILightSceneNode * light = smgr->addLightSceneNode(0, core::vector3df(-60,100,400),
		video::SColorf(1.0f,1.0f,1.0f,1.0f), 600.0f);
	light->setID(ID_IsNotPickable); // Make it an invalid target for selection.

	// Remember which scene node is highlighted
	scene::ISceneNode* highlightedSceneNode = 0;
	scene::ISceneCollisionManager* collMan = smgr->getSceneCollisionManager();
	int lastFPS = -1;

	// draw the selection triangle only as wireframe
	material.Wireframe=true;

	if(0)
	while(device->run())
	if (device->isWindowActive())
	{
		driver->beginScene(true, true, 0);
		smgr->drawAll();

		// Unlight any currently highlighted scene node
		if (highlightedSceneNode)
		{
			//highlightedSceneNode->setMaterialFlag(video::EMF_LIGHTING, true);
			highlightedSceneNode = 0;
		}

		// All intersections in this example are done with a ray cast out from the camera to
		// a distance of 1000.  You can easily modify this to check (e.g.) a bullet
		// trajectory or a sword's position, or create a ray from a mouse click position using
		// ISceneCollisionManager::getRayFromScreenCoordinates()
		core::line3d<f32> ray;
		ray.start = camera->getPosition();
		ray.end = ray.start + (camera->getTarget() - ray.start).normalize() * 1000.0f;

		// Tracks the current intersection point with the level or a mesh
		core::vector3df intersection;
		// Used to show with triangle has been hit
		core::triangle3df hitTriangle;

		// This call is all you need to perform ray/triangle collision on every scene node
		// that has a triangle selector, including the Quake level mesh.  It finds the nearest
		// collision point/triangle, and returns the scene node containing that point.
		// Irrlicht provides other types of selection, including ray/triangle selector,
		// ray/box and ellipse/triangle selector, plus associated helpers.
		// See the methods of ISceneCollisionManager
		scene::ISceneNode * selectedSceneNode =
			collMan->getSceneNodeAndCollisionPointFromRay(
					ray,
					intersection, // This will be the position of the collision
					hitTriangle, // This will be the triangle hit in the collision
					IDFlag_IsPickable, // This ensures that only nodes that we have
							// set up to be pickable are considered
					0); // Check the entire scene (this is actually the implicit default)

		// If the ray hit anything, move the billboard to the collision position
		// and draw the triangle that was hit.
		if(selectedSceneNode)
		{
			bill->setPosition(intersection);

			// We need to reset the transform before doing our own rendering.
			driver->setTransform(video::ETS_WORLD, core::matrix4());
			driver->setMaterial(material);
			driver->draw3DTriangle(hitTriangle, video::SColor(0,255,0,0));

			// We can check the flags for the scene node that was hit to see if it should be
			// highlighted. The animated nodes can be highlighted, but not the Quake level mesh
			if((selectedSceneNode->getID() & IDFlag_IsHighlightable) == IDFlag_IsHighlightable)
			{
				highlightedSceneNode = selectedSceneNode;

				// Highlighting in this case means turning lighting OFF for this node,
				// which means that it will be drawn with full brightness.
				//highlightedSceneNode->setMaterialFlag(video::EMF_LIGHTING, false);
			}
		}

		// We're all done drawing, so end the scene.
		driver->endScene();

		int fps = driver->getFPS();

		if (lastFPS != fps)
		{
			core::stringw str = L"Collision detection example - Irrlicht Engine [";
			str += driver->getName();
			str += "] FPS:";
			str += fps;

			device->setWindowCaption(str.c_str());
			lastFPS = fps;
		}
	}

	device->drop();

	return 0;
}

/*
**/


#!BPY

"""
Name: 'Mini3d file Exporter'
Blender: 263a
Group: 'Export'
Tooltip: 'Mini3d Exporter (m3d)'
"""

import bpy
import sys
import struct
import math
import mathutils
from operator import itemgetter


# todo: break the write count out of writenameindexmap

########### WRITE MESH ########################################################

def writeMesh(mesh, file):
    fw = file.write
    
    #make sure mesh has tesselated faces
    mesh.update(calc_tessface=True)
    
    # grab vertices
    vertices=[]
    for vert in mesh.vertices:
            vertices.append({'co': vert.co, 
            'no': vert.normal, 
            'uv': [0, 0], 
            'grp': vert.groups })
    
    # grab indices
    indices=[]
    for face in mesh.tessfaces:
        indices.append(face.vertices[0])
        indices.append(face.vertices[1])
        indices.append(face.vertices[2])

        if len(face.vertices) > 3:
            indices.append(face.vertices[0])
            indices.append(face.vertices[2])
            indices.append(face.vertices[3])
    
        #update texture coordinates
        if len(mesh.tessface_uv_textures) > 0:
            uv = mesh.tessface_uv_textures[0].data[face.index]
            vertices[face.vertices[0]]['uv'] = uv.uv1
            vertices[face.vertices[1]]['uv'] = uv.uv2
            vertices[face.vertices[2]]['uv'] = uv.uv3
    
            if len(face.vertices) > 3:
                vertices[face.vertices[3]]['uv'] = uv.uv4

    # write the size of a vertex in bytes
    fw(struct.pack('=H', 64))
    
    # write the number of vertices
    fw(struct.pack('=H', len(vertices)))

    # write the result to file            
    for vert in vertices:
        fw(struct.pack('=8f', 
            vert['co'][0], vert['co'][1], vert['co'][2],
            vert['no'][0], vert['no'][1], vert['no'][2],
            vert['uv'][0], 1.0 - vert['uv'][1]))
		
        vertex_groups = [(grp.group, grp.weight) for grp in vert['grp']]
		
        # add make sure there are at least 4 vertex groups
        for x in range(len(vertex_groups),4):
            vertex_groups.append((0,0))
        
        sorted_vertex_groups = sorted(vertex_groups, key=itemgetter(1), reverse=True)
        fw(struct.pack('=4f', 
			float(sorted_vertex_groups[0][0]),
			float(sorted_vertex_groups[1][0]),
			float(sorted_vertex_groups[2][0]),
			float(sorted_vertex_groups[3][0])))
        fw(struct.pack('=4f',             
			sorted_vertex_groups[0][1],
			sorted_vertex_groups[1][1],
			sorted_vertex_groups[2][1],
			sorted_vertex_groups[3][1]))

    # write number of indices
    fw(struct.pack('=H', len(indices)))
    for index in indices:
        fw(struct.pack('=H', index))
        
    
########### WRITE ARMATURE ####################################################
			
def writeArmature(armature, file):
    fw = file.write
    
    bones = armature.bones
	
	# get indices for all bones
    bone_indices = {}
    for bone in bones:
        bone_indices[bone] = len(bone_indices)
        
    # write bone count
    fw(struct.pack('=H', len(bones)))

    # write bone data
    for bone in bones:

        offset = mathutils.Vector([0,0,0])
    
        # write parent index
        if bone.parent:
            offset = bone.head_local - bone.parent.head_local
            fw(struct.pack('=H', bone_indices[bone.parent]))
        else:
            offset = bone.head_local
            fw(struct.pack('=H', 0xffff))
        
        # write bone coordinates
        fw(struct.pack('=fff', offset[0], offset[1], offset[2]))

        # write bone roll
        print("Roll: ", getRoll(bone))
        fw(struct.pack('=f', getRoll(bone)))
        
        
def getRoll(bone):
    mat = bone.matrix_local.to_3x3()
    quat = mat.to_quaternion()
    if abs(quat.w) < 1e-4:
        roll = math.pi
    else:
        roll = 2*math.atan2(quat.y, quat.w)
    return roll


########### WRITE ACTION ######################################################

def writeAction(action, file):
    fw = file.write

    quaternion_keyframe_lists = []

    #loop over all groups
    for group in action.groups:
        # find all channels that are quaternion rotations
        quaternion_channels = []
        for channel in group.channels:
            pos = channel.data_path.rfind('.')
            if channel.data_path[pos:] == '.rotation_quaternion':
                quaternion_channels.append(channel)
        
        # get all unique keyframes
        quaternion_keyframes = []
        for channel in quaternion_channels:
            for keyframe in channel.keyframe_points:
                timeStamp = int(keyframe.co[0])
                if not (timeStamp, mathutils.Quaternion()) in quaternion_keyframes:
                    quaternion_keyframes.append((int(keyframe.co[0]), mathutils.Quaternion()))

        # sort quaternion keyframes
        quaternion_keyframes.sort()
                
        # set values for all keyframes
        for keyframe in quaternion_keyframes:
            for channel in quaternion_channels:
                keyframe[1][channel.array_index] = channel.evaluate(keyframe[0])

        #normalize quaternions
        for keyframe in quaternion_keyframes:
            keyframe[1].normalize();
            tmp = keyframe[1].w
            keyframe[1].w = keyframe[1].x
            keyframe[1].x = keyframe[1].y
            keyframe[1].y = keyframe[1].z
            keyframe[1].z = tmp
        
        # add initial undefined key frame for all tracks (will never be read, just for padding to make multiplex algorithm work below)
        quaternion_keyframes.insert(0, [0xffff, mathutils.Quaternion()])

        # add a trailing key frame that denotes the end of the track
        quaternion_keyframes.append([0xffff, mathutils.Quaternion()])
        
        # add the keyframe 
        quaternion_keyframe_lists.append(quaternion_keyframes);

        
    # write the number of frames in the animation
    # get the total number of keyframes
    totLen = 0
    for keyframe_list in quaternion_keyframe_lists:
        totLen += len(keyframe_list)

    # write length of the animation stream in bytes
    fw(struct.pack('=H', 10 + 20 * (totLen - len(quaternion_keyframe_lists)) - 16 * len(quaternion_keyframe_lists)))
        
    # write the animation length in frames
    fw(struct.pack('=H', (int)(action.frame_range[1])))
    
    # write the number of joints
    fw(struct.pack('=H', len(quaternion_keyframe_lists)))

    # write the total number of tracks
    fw(struct.pack('=H', len(quaternion_keyframe_lists)))
    
    #todo: all tracks must have at least two keyframes
   
    # write the initial set of keyframes
    for keyframe_list in quaternion_keyframe_lists:
        for y in range(0,3):
            writeKeyFrame(quaternion_keyframe_lists, keyframe_list, file)
            keyframe_list.pop(0)

    # mux the rest of the keyframes into a single list
    # find out what the next lowest keyframe time is
    while getMinTimeStamp(quaternion_keyframe_lists) < 0xffff:
        minTimeStamp = getMinTimeStamp(quaternion_keyframe_lists)
        # write all keyframes with time stamp == minTimeStamp to file
        for keyframe_list in quaternion_keyframe_lists:
            if  keyframe_list[0][0] == minTimeStamp:
                writeKeyFrame(quaternion_keyframe_lists, keyframe_list, file)                
                keyframe_list.pop(0)

    # write the end of stream marker
    fw(struct.pack('=2H', 0, 0))        

    
def getMinTimeStamp(quaternion_keyframe_lists):
    minTimeStamp = 0xffff
    for keyframe_list in quaternion_keyframe_lists:
        if keyframe_list[0][0] < minTimeStamp:
            minTimeStamp = keyframe_list[0][0]

    return minTimeStamp
    
def writeKeyFrame(quaternion_keyframe_lists, keyframe_list, file):
    fw = file.write
    
    # write the keyframe
    index = (quaternion_keyframe_lists.index(keyframe_list) << 4) + 1;
    timeStamp = keyframe_list[1][0]
    print("ts: ", timeStamp, " i: ", index)
    fw(struct.pack('=2H', index, timeStamp))

    #dont write the vector data for the last end of track marker (timeStamp == 0xffff)
    if timeStamp != 0xffff:
        vec = keyframe_list[1][1]
        print(" vec: ", round(vec[0], 2), round(vec[1], 2), round(vec[2], 2), round(vec[3], 2))
        fw(struct.pack('=4f', vec[0], vec[1], vec[2], vec[3]))
    
    
########### WRITE MATERIAL ####################################################
			
def writeMaterial(material, file):
    fw = file.write
        
    # write texture count
    texture_slots = [bpy.data.textures[x.name] for x in material.texture_slots if x]
    print("texture slots: ", len(texture_slots))
    fw(struct.pack('=H', len(texture_slots)))

    # write textures data
    for texture in texture_slots:
        if texture.image:
            fw(struct.pack('=H', bpy.data.images.find(texture.image.name)))
        else:
            fw(struct.pack('=H', 0xffff))

            
########### WRITE IMAGE #######################################################
			
def writeImage(image, file):
        
    # write image name
    writeLengthPrefixedString(image.name, file)

   
########### WRITE LAMP ########################################################

def writeLamp(lampObject, file):
	fw = file.write
	
	lamp = lampObject.data
	
	# position and rotation
	fw(struct.pack('=7f', 
		lampObject.location[0],
		lampObject.location[1],
		lampObject.location[2],
		lampObject.rotation_quaternion[1],
		lampObject.rotation_quaternion[2],
		lampObject.rotation_quaternion[3],
		lampObject.rotation_quaternion[0]))
	
	# shadow buffer settings
	fw(struct.pack('=4f',
		lamp.spot_size - lamp.spot_size * lamp.spot_blend,
		lamp.spot_size,
		lamp.shadow_buffer_clip_start,
		lamp.shadow_buffer_clip_end))
		
	# colors
	fw(struct.pack('=3f',
		lamp.color[0],
		lamp.color[1],
		lamp.color[2]))


########### WRITE CAMERA ######################################################

def writeCamera(cameraObject, file):
	fw = file.write

	camera = cameraObject.data
	
	# position and rotation
	fw(struct.pack('=fffffff', 
		cameraObject.location[0],
		cameraObject.location[1],
		cameraObject.location[2],
		cameraObject.rotation_quaternion[1],
		cameraObject.rotation_quaternion[2],
		cameraObject.rotation_quaternion[3],
		cameraObject.rotation_quaternion[0]))
	
	# render target information
	fw (struct.pack('=ffff', 
		getFov(camera.lens.real, camera.sensor_width)[0],
		float(camera.clip_start),
		float(camera.clip_end),
		float(camera.sensor_width / camera.sensor_height)))

        
def getFov(focal_length, sensor_size):
    return [2 * math.atan( sensor_size / (2 * focal_length))]

        
########### WRITE OBJECT ######################################################

def writeObject(meshObject, file):
    fw = file.write

    # position and rotation
    fw(struct.pack('=10f', 
        meshObject.location[0],
        meshObject.location[1],
        meshObject.location[2],
        meshObject.rotation_quaternion[1],
        meshObject.rotation_quaternion[2],
        meshObject.rotation_quaternion[3],
        meshObject.rotation_quaternion[0],
        meshObject.scale[0],
        meshObject.scale[1],
        meshObject.scale[2]))

    # mesh index information
    fw (struct.pack('=H', bpy.data.meshes.find(meshObject.data.name)))	

    # material index information
    if len(meshObject.material_slots) > 0:
        fw (struct.pack('=H', bpy.data.materials.find(meshObject.material_slots[0].material.name)))
    else:
        fw (struct.pack('=H', 0xffff))


########### WRITE SCENE ######################################################

def writeScene(scene, file):
    fw = file.write

    ## MESH OBJECTS ##
    print("Mesh Objects")

    objects = [obj for obj in scene.objects if obj.type == 'MESH']

    # write the map from names to indices
    obj_map = [(obj.name, objects.index(obj)) for obj in objects]
    obj_map.sort();
    writeNameIndexMap(obj_map, file)

    # write objects in scene
    for obj in objects:
        writeObject(obj, file)		

        
    ## LAMP OBJECTS ##
    print("Lamp Objects")

    lamps = [obj for obj in scene.objects if obj.type == 'LAMP']

    # write the map from names to indices
    lamp_map = [(lamp.name, lamps.index(lamp)) for lamp in lamps]
    lamp_map.sort();
    writeNameIndexMap(lamp_map, file)

    # write lamps in scene
    for lamp in lamps:
        writeLamp(lamp, file)


    ## CAMERA OBJECTS ##
    print("Camera Objects")

    cameras = [obj for obj in scene.objects if obj.type == 'CAMERA']	

    # write the map from names to indices
    camera_map = [(camera.name, cameras.index(camera)) for camera in cameras]
    camera_map.sort();
    writeNameIndexMap(camera_map, file)

    #write cameras in scene
    for camera in cameras:
        writeCamera(camera, file)

        
########### EXPORTER ##########################################################

def exportAll(filename):
    file = open(filename, 'wb')
    fw = file.write

    ## MESHES ##
    print("Meshes")

    # write sorted name index map
    mesh_map = [(mesh.name, bpy.data.meshes.find(mesh.name)) for mesh in bpy.data.meshes]
    mesh_map.sort();
    writeNameIndexMap(mesh_map, file)

    # write meshes
    for mesh in bpy.data.meshes:
        writeMesh(mesh, file)

        
    ## ARMATURES ##
    print("Armatures")

    # write sorted name index map
    armature_map = [(arm.name, bpy.data.armatures.find(arm.name)) for arm in bpy.data.armatures]
    armature_map.sort();
    writeNameIndexMap(armature_map, file)

    # write armatures
    for armature in bpy.data.armatures:
        writeArmature(armature, file)


    ## ACTIONS ##
    print("Actions")
        
    # write sorted name index map
    action_map = [(act.name, bpy.data.actions.find(act.name)) for act in bpy.data.actions]
    action_map.sort();
    writeNameIndexMap(action_map, file)
        
    # write actions
    for action in bpy.data.actions:
        writeAction(action, file)

    writeLengthPrefixedString("MAT", file)
    
    ## MATERIALS ##
    print("Materials")
        
    # write sorted name index map
    material_map = [(mat.name, bpy.data.materials.find(mat.name)) for mat in bpy.data.materials]
    material_map.sort();
    writeNameIndexMap(material_map, file)
        
    # write actions
    for material in bpy.data.materials:
        writeMaterial(material, file)

        
    ## IMAGES ##
    print("Images")
        
    # write sorted name index map
    image_map = [(image.name, bpy.data.images.find(image.name)) for image in bpy.data.images]
    image_map.sort();
    writeNameIndexMap(image_map, file)
        
    # write images
    for image in bpy.data.images:
        writeImage(image, file)

        
    ## SCENES ##
    print("Scenes")
        
    # write sorted name index map
    scene_map = [(scene.name, bpy.data.scenes.find(scene.name)) for scene in bpy.data.scenes]
    scene_map.sort();
    writeNameIndexMap(scene_map, file)
    
    # write scenes
    for scene in bpy.data.scenes:
        writeScene(scene, file)


    file.close()

def writeNameIndexMap(name_index_map, file):
    fw = file.write
    fw(struct.pack('=H', len(name_index_map)))
    for tup in name_index_map:
        length = len(tup[0])
        fw(struct.pack('=H', length))
        fw(struct.pack('='+str(length)+'s', tup[0].encode('UTF-8')))
        fw(struct.pack('=H', tup[1]))
	
def writeLengthPrefixedString(string, file):
    fw = file.write
    length = len(string)
    fw(struct.pack('=H', length))
    fw(struct.pack('='+str(length)+'s', string.encode('UTF-8')))
    
    
exportAll('G:/projects/software/mini3d/source/assets/mirror.m3d')
print('Done')

# filename = 'G:\\projects\\software\\mini3d\\source\\assets\\mini3dexporter.py'
# exec(compile(open(filename).read(), filename, 'exec'))

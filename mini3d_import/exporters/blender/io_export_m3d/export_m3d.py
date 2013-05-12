#!BPY

"""
Name: 'Mini3d file Exporter'
Blender: 263a
Group: 'Export'
Tooltip: 'Mini3d Exporter (m3d)'
"""

import bpy
import re
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
    
    # write name
    writeLengthPrefixedString(mesh.name, file)
    
    #find the vertex attributes for this mesh
    #TODO: attribute data should be stored globally and not in scene
    attributes = None
    if len(mesh.materials) > 0 and len(bpy.data.scenes) > 0:

        size = len(bpy.data.scenes[0].attribute_groups)
        index = bpy.data.scenes[0].active_attribute_group
        
        if size > index:
            attributes = [i.type for i in bpy.data.scenes[0].attribute_groups[index].attributes]
    
    if attributes is None:
        attributes = ['POSITION', 'NORMAL', 'TEXTURE']
    
    # gather texture coordinates
    texCo = [[0,0] for vert in mesh.vertices];

    if len(mesh.tessface_uv_textures) > 0:
        for face in mesh.tessfaces:
            uv = mesh.tessface_uv_textures[0].data[face.index]
            texCo[face.vertices[0]] = uv.uv1
            texCo[face.vertices[1]] = uv.uv2
            texCo[face.vertices[2]] = uv.uv3

            if len(face.vertices) > 3:
                texCo[face.vertices[3]] = uv.uv4           
    
    # write the size of a vertex in bytes
    vertexSizeInBytes = 0;
    for i in range(0, len(attributes)):
        if attributes[i] == 'POSITION': 
            vertexSizeInBytes += 3 * 4
        elif attributes[i] == 'NORMAL': 
            vertexSizeInBytes += 3 * 4
        elif attributes[i] == 'TEXTURE': 
            vertexSizeInBytes += 2 * 4
        elif attributes[i] == 'GROUPS': 
            vertexSizeInBytes += 8 * 4
    
    fw(struct.pack('=H', vertexSizeInBytes))

    # write the number of vertices
    fw(struct.pack('=H', len(mesh.vertices)))
    
    # write vertex data
    for i in range(0, len(mesh.vertices)):
        for j in range(0, len(attributes)):
            if attributes[j] == 'POSITION':
                co = mesh.vertices[i].co
                fw(struct.pack('=3f', co[0], co[1], co[2]))
            elif attributes[j] == 'NORMAL': 
                norm = mesh.vertices[i].normal
                fw(struct.pack('=3f', norm[0], norm[1], norm[2]))
            elif attributes[j] == 'TEXTURE': 
                fw(struct.pack('=2f', texCo[i][0], texCo[i][1]))
            elif attributes[j] == 'GROUPS':
                vertex_groups = [(grp.group, grp.weight) for grp in mesh.vertex.groups]
                
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

    # set indices
    indices=[]
    for face in mesh.tessfaces:
        indices.append(face.vertices[0])
        indices.append(face.vertices[1])
        indices.append(face.vertices[2])

        if len(face.vertices) > 3:
            indices.append(face.vertices[0])
            indices.append(face.vertices[2])
            indices.append(face.vertices[3])

    # write number of indices
    fw(struct.pack('=H', len(indices)))
    for index in indices:
        fw(struct.pack('=H', index))
        
    
########### WRITE ARMATURE ####################################################
			
def writeArmature(armature, file):
    fw = file.write
    
    # write name
    writeLengthPrefixedString(armature.name, file)
    
	# get indices for all bones
    bones = armature.bones
    bone_indices = {}
    for bone in bones:
        bone_indices[bone] = len(bone_indices)
        
    # write bone count
    fw(struct.pack('=H', len(bones)))

    # write bone data
    for bone in bones:

        offset = mathutils.Vector([0,0,0])
        
        #write name
        writeLengthPrefixedString(bone.name, file)
    
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

    # write name
    writeLengthPrefixedString(action.name, file)

    #group fcurves by data_path name
    channels = { fcurve.data_path : [] for fcurve in action.fcurves }
    
    for fcurve in action.fcurves:
        channels[fcurve.data_path].append(fcurve)
    
    fw(struct.pack('=H', len(channels)))
    
    #write channels to file
    for key in channels:

        boneName = ""
        target = ""
        match = re.match('pose.bones\["([^"]*)"\]\.(.*)', key)

        if match:
            boneName = match.groups()[0]
            target = match.groups()[1]
        else:
            target = fcurve.data_path

        #write track bone name
        writeLengthPrefixedString(boneName, file)
        writeLengthPrefixedString(target, file)

        #find all keyframes for all channels in group    
        fcurves = channels[key]
        keyframes = set()
        for fcurve in fcurves:
            for keyframe in fcurve.keyframe_points:
                keyframes.add(keyframe.co[0])

        keyframes = sorted(keyframes)

        #write subindices count
        zero = [i for i in range(0, len(fcurves))]
        fw(struct.pack('=H', len(zero)))
        
        #write keyframe count
        fw(struct.pack('=H', len(keyframes)))

        #evaluate animated value for all collected keyframes
        for keyframe in keyframes:
            values = list(zero)

            for fcurve in fcurves:
                values[fcurve.array_index] = fcurve.evaluate(keyframe)
            
            fw(struct.pack('=f', keyframe / 30.0))
            
            for value in values:
                fw(struct.pack('=f', value))

                
########### WRITE MATERIAL ####################################################
			
def writeMaterial(material, file):
    fw = file.write
        
    # write name
    writeLengthPrefixedString(material.name, file)        
        
    # write texture count
    texture_slots = [bpy.data.textures[x.name] for x in material.texture_slots if x]
    print("texture slots: ", len(texture_slots))
    fw(struct.pack('=H', len(texture_slots)))

    # write textures data
    for texture in texture_slots:
        if texture.type == 'IMAGE' and texture.image:
            fw(struct.pack('=H', bpy.data.images.find(texture.image.name)))
        else:
            fw(struct.pack('=H', 0xffff))

            
########### WRITE IMAGE #######################################################
			
def writeImage(image, file):

    # write name
    writeLengthPrefixedString(image.name, file)
        
    # write image name
    writeLengthPrefixedString(image.name, file)

   
########### WRITE LAMP ########################################################

def writeLamp(lampObject, file):
    fw = file.write

    lamp = lampObject.data

    # write name
    writeLengthPrefixedString(lamp.name, file)

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

    # write name
    writeLengthPrefixedString(camera.name, file)

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

    # write name
    writeLengthPrefixedString(meshObject.name, file)
    
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

    # write name
    writeLengthPrefixedString(scene.name, file)    
    
    ## MESH OBJECTS ##
    print("Mesh Objects")

    # write objects in scene
    objects = [obj for obj in scene.objects if obj.type == 'MESH' and obj.data.export == True]
    fw(struct.pack('=H', len(objects)))

    for obj in objects:
        writeObject(obj, file)		

        
    ## LAMP OBJECTS ##
    print("Lamp Objects")

    # write lamps in scene
    lamps = [obj for obj in scene.objects if obj.type == 'LAMP' and obj.data.export == True]
    fw(struct.pack('=H', len(lamps)))

    for lamp in lamps:
        writeLamp(lamp, file)


    ## CAMERA OBJECTS ##
    print("Camera Objects")

    #write cameras in scene
    cameras = [obj for obj in scene.objects if obj.type == 'CAMERA' and obj.data.export == True]	
    fw(struct.pack('=H', len(cameras)))

    for camera in cameras:
        writeCamera(camera, file)

        
########### EXPORTER ##########################################################

def save(context, filename):
    file = open(filename, 'wb')
    fw = file.write

    ## MESHES ##
    print("Meshes")

    # write meshes
    meshes = [mesh for mesh in bpy.data.meshes if mesh.export == True]
    fw(struct.pack('=H', len(meshes)))
    
    for mesh in meshes:
        writeMesh(mesh, file)

        
    ## ARMATURES ##
    print("Armatures")

    # write armatures
    armatures = [arm for arm in bpy.data.armatures if arm.export == True]
    fw(struct.pack('=H', len(armatures)))
    
    for armature in armatures:
        writeArmature(armature, file)


    ## ACTIONS ##
    print("Actions")
        
    # write actions
    actions = [action for action in bpy.data.actions if action.export == True]
    fw(struct.pack('=H', len(actions)))
        
    for action in actions:
        writeAction(action, file)

    writeLengthPrefixedString("MAT", file)
    
    
    ## MATERIALS ##
    print("Materials")
        
    # write actions
    materials = [mat for mat in bpy.data.materials if mat.export == True]
    fw(struct.pack('=H', len(materials)))
        
    for material in materials:
        writeMaterial(material, file)

        
    ## IMAGES ##
    print("Images")
        
    # write images
    images = [image for image in bpy.data.images]
    fw(struct.pack('=H', len(images)))
        
    for image in images:
        writeImage(image, file)

        
    ## SCENES ##
    print("Scenes")
        
    # write scenes
    scenes = [scene for scene in bpy.data.scenes if scene.export == True]
    fw(struct.pack('=H', len(scenes)))
    
    for scene in scenes:
        writeScene(scene, file)

    file.close()

def writeNameIndexMap(name_index_map, file):
    fw = file.write
    fw(struct.pack('=H', len(name_index_map)))
    i = 0;
    for obj in name_index_map:
        bytes = obj.name.encode('UTF-8')
        fw(struct.pack('=H', len(bytes)))
        fw(bytes)
        fw(struct.pack('=H', i))
        i += 1
	
def writeLengthPrefixedString(string, file):
    fw = file.write
    bytes = string.encode('UTF-8')
    fw(struct.pack('=H', len(bytes)))
    fw(bytes)
    
    

# Copyright (c) <2013> Daniel Peterson
# License: MIT Software License <www.mini3d.org/license>

bl_info = {
    "name": "Export Mini3d file (.m3d)",
    "description": "This script exports file data to a format that can be used with the Mini3d game enigne framework",
    "author": "Daniel Peterson",
    "version": (0, 5),
    "blender": (2, 66, 0),
    "location": "File > Export > Mini3d (.m3d)",
    "warning": "Constantly changing format, visit Mini3d website for info", # used for warning icon and text in addons panel
    "wiki_url": "www.mini3d.org",
    "tracker_url": "",
    "category": "Import-Export" }

import bpy
from .panels import *
from .operator import ExportM3DOperator

def register_props() :
    bpy.types.Object.export = bpy.props.BoolProperty(default=True) 
    bpy.types.Mesh.export = bpy.props.BoolProperty(default=True) 
    bpy.types.Armature.export = bpy.props.BoolProperty(default=True)
    bpy.types.Action.export =  bpy.props.BoolProperty(default=True)
    bpy.types.Material.export = bpy.props.BoolProperty(default=True) 
    bpy.types.Scene.export = bpy.props.BoolProperty(default=True)
    
    bpy.types.WindowManager.active_object = bpy.props.IntProperty() 
    bpy.types.WindowManager.active_mesh = bpy.props.IntProperty()  
    bpy.types.WindowManager.active_armature = bpy.props.IntProperty()  
    bpy.types.WindowManager.active_action = bpy.props.IntProperty()  
    bpy.types.WindowManager.active_material = bpy.props.IntProperty()  
    bpy.types.WindowManager.active_scene = bpy.props.IntProperty()  

    bpy.types.Mesh.attributes = bpy.props.CollectionProperty(type=AttributeProperty)
    bpy.types.Mesh.active_attribute = bpy.props.IntProperty()

    
def unregister_props() :
    del bpy.types.Object.export;
    del bpy.types.Mesh.export;
    del bpy.types.Armature.export
    del bpy.types.Action.export
    del bpy.types.Material.export
    del bpy.types.Scene.export
    
    del bpy.types.WindowManager.active_object
    del bpy.types.WindowManager.active_mesh
    del bpy.types.WindowManager.active_armature
    del bpy.types.WindowManager.active_action 
    del bpy.types.WindowManager.active_material
    del bpy.types.WindowManager.active_scene
    
    del bpy.types.Mesh.attributes
    del bpy.types.Mesh.active_attribute

def menu_func(self, context):
    self.layout.operator(ExportM3DOperator.bl_idname, text="Mini3d (.m3d)")

def register():
    bpy.utils.register_module(__name__)
    register_props()
    bpy.types.INFO_MT_file_export.append(menu_func)
    
def unregister():
    unregister_props()
    
    bpy.utils.unregister_class(bpy.types.ObjectPanel)
    bpy.utils.unregister_class(bpy.types.MeshPanel)
    bpy.utils.unregister_class(bpy.types.MaterialPanel)
    bpy.utils.unregister_class(bpy.types.ScenePanel)
    
    bpy.utils.unregister_module(__name__)
    bpy.types.INFO_MT_file_export.remove(menu_func)

if __name__ == "__main__":
    register()


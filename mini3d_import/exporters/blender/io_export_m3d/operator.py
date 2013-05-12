
import bpy
import sys
import struct
import math
import mathutils

from .panels import AttributeProperty

class ExportM3DOperator(bpy.types.Operator):
    """
    Export to Mini3d format (.m3d)
    """
    bl_idname = "export.m3d"
    bl_label = "ExportM3D"
    filepath = bpy.props.StringProperty(subtype='FILE_PATH')
    my_string = bpy.props.StringProperty(name="String Value")
    my_collection = bpy.props.CollectionProperty(type=AttributeProperty)
    
    def execute(self, context):
        filePath = bpy.path.ensure_ext(self.filepath, ".m3d")

        from . import export_m3d
        export_m3d.save(context, filePath)
        return {'FINISHED'}

    def _checkNO(self, val):
        if val == 'NO': return None
        else: return val

    def invoke(self, context, event):
        if not self.filepath:
            self.filepath = bpy.path.ensure_ext(bpy.data.filepath, ".m3d")
            
        WindowManager = context.window_manager
        WindowManager.fileselect_add(self)
        return {'RUNNING_MODAL'}



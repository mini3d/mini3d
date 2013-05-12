import bpy

vertexAttributes = [("POSITION", "Position", "" , "MAN_TRANS", 0 ),
                      ("NORMAL", "Normal", "", "MANIPUL", 1),
                      ("TANGENT", "Tangent", "", "OUTLINER_OB_EMPTY", 2),
                      ("BITANGENT", "Bitangent", "", "EMPTY_DATA", 3),
                      ("TEXTURE", "Texture Coordinates", "", "GROUP_UVS", 4),
                      ("GROUPS", "Vertex Groups", "", "GROUP_VERTEX", 5),                                            
                      ("COLOR", "Vertex Color", "", "GROUP_VCOL", 6)]
                      
vertexAttributeDict = {i[0]: [i[1], i[3]] for i in vertexAttributes}

class AddAttributeOperator(bpy.types.Operator):
    bl_idname = "mesh.attribute_add"
    bl_label = "Add Vertex Attribute"

    def invoke(self, context, event):
        mesh = bpy.context.object.data
        mesh.attributes.add()
        mesh.active_attribute = len(mesh.attributes) - 1
        return{"FINISHED"}

class RemoveAttributeOperator(bpy.types.Operator):
    bl_idname = "mesh.attribute_remove"
    bl_label = "Remove Vertex Attribute"

    def invoke(self, context, event):
        mesh = bpy.context.object.data
        index = mesh.active_attribute
    
        if index >= 0 and index < len(mesh.attributes):
            mesh.attributes.remove(index)
            mesh.active_attribute -= 1

        return{"FINISHED"}    

class MoveAttributeOperator(bpy.types.Operator):
    bl_idname = "mesh.attribute_move"
    bl_label = "Move Vertex Attribute"
    
    direction = bpy.props.EnumProperty(
                        name="direction",
                        items=(("UP", "Up", ""),
                              ("DOWN", "Down", ""))
                        )
    
    def invoke(self, context, event):
        mesh = bpy.context.object.data
        index = mesh.active_attribute
        
        if self.direction == 'DOWN' and index < len(mesh.attributes) - 1:
            mesh.attributes.move(index, index + 1)
            mesh.active_attribute += 1
        elif self.direction == 'UP' and index > 0:
            mesh.attributes.move(index, index - 1)
            mesh.active_attribute -= 1
        
        return{"FINISHED"}    

class AttributeProperty(bpy.types.PropertyGroup):
    name = bpy.props.StringProperty(name="Name", default="new_attribute")
    type = bpy.props.EnumProperty(
                name="Attribute Type",
                items=vertexAttributes,
                )

class ATTRIBUTE_UL_list(bpy.types.UIList):
    def draw_item(self, context, layout, data, item, icon, active_data, active_propname, index):

        if self.layout_type in {'DEFAULT', 'COMPACT'}:
            layout.label(text=item.name, translate=False, icon_value=icon)
            layout.label(text=vertexAttributeDict[item.type][0], translate=False, icon=vertexAttributeDict[item.type][1])
            
        elif self.layout_type in {'GRID'}:
            layout.alignment = 'CENTER'
            layout.label(text="", icon_value=icon)

class MeshPanel(bpy.types.Panel):
    bl_label = "Mini3d Export"
    bl_space_type = "PROPERTIES"
    bl_region_type = "WINDOW"
    bl_context = "data"

    def draw(self, context):
     
        if context.object.type == 'MESH':
            
            mesh = context.object.data
            collection = mesh.attributes
            size = len(collection)
            index = mesh.active_attribute
            
            #separator
            self.layout.label(text="Vertex Attributes:", translate=False)
            col = self.layout.column()
            row = col.row()
            row.template_list('ATTRIBUTE_UL_list', 'attribute_list', mesh, "attributes", mesh, "active_attribute")

            # Attribute list buttons
            col = row.column(align=True)
            col.operator("mesh.attribute_add", icon='ZOOMIN', text="")
            col.operator("mesh.attribute_remove", icon='ZOOMOUT', text="")
            
            attribute = None
            if size > 0 and index < size:
                attribute = collection[index]

            if attribute:
                col.separator()
                col.operator("mesh.attribute_move", icon='TRIA_UP', text="").direction = 'UP'
                col.operator("mesh.attribute_move", icon='TRIA_DOWN', text="").direction = 'DOWN'

                row = self.layout.row()
                row.prop(attribute, "name")
                row = self.layout.row()
                row.prop(attribute, "type")
                    

class EXPORT_UL_list(bpy.types.UIList):

    def draw_item(self, context, layout, data, item, icon, active_data, active_propname, index):

        if self.layout_type in {'DEFAULT', 'COMPACT'}:
            layout.label(text=item.name, translate=False, icon_value=icon)
            layout.prop(item, "export", text="")

        elif self.layout_type in {'GRID'}:
            layout.alignment = 'CENTER'
            layout.label(text="", icon_value=icon)

class ScenePanel(bpy.types.Panel):
    bl_label = "Mini3d Export"
    bl_space_type = "PROPERTIES"
    bl_region_type = "WINDOW"
    bl_context = "scene"

    def draw(self, context):

        layout = self.layout
    
        layout.label(text="Meshes:")
        layout.template_list('EXPORT_UL_list', '', bpy.data, "meshes", context.window_manager, "active_mesh")

        layout.label(text="Materials:")
        layout.template_list('EXPORT_UL_list', '', bpy.data, "materials", context.window_manager, "active_material")

        layout.label(text="Armatures:")
        layout.template_list('EXPORT_UL_list', '', bpy.data, "armatures", context.window_manager, "active_armature")

        layout.label(text="Actions:")
        layout.template_list('EXPORT_UL_list', '', bpy.data, "actions", context.window_manager, "active_action")

        layout.label(text="Scenes:")
        layout.template_list('EXPORT_UL_list', '', bpy.data, "scenes", context.window_manager, "active_scene")

        index = context.window_manager.active_scene;
        size = len(bpy.data.scenes)
        
        if index >= 0 and index < size:
            scene = bpy.data.scenes[index]
        
            layout.label(text="Objects:")
            layout.template_list('EXPORT_UL_list', '', scene, "objects", context.window_manager, "active_object")
        
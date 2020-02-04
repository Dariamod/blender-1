import bpy
from .. base import SimulationNode
from .. node_builder import NodeBuilder

class ParticleSystemNode(bpy.types.Node, SimulationNode):
    bl_idname = "fn_ParticleSystemNode"
    bl_label = "Particle System"

    def declaration(self, builder: NodeBuilder):
        builder.background_color((0.8, 0.5, 0.4))

        builder.emitters_input("emitters", "Emitters")
        builder.events_input("events", "Events")
        builder.forces_input("forces", "Forces")

    def draw(self, layout):
        layout.prop(self, "name", text="", icon="PHYSICS")

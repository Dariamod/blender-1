from . declaration import (
    FixedSocketDecl,
    ListSocketDecl,
    PackListDecl,
    AnyVariadicDecl,
    TreeInterfaceDecl,
    VectorizedInputDecl,
    VectorizedOutputDecl,
    ControlFlowSocketDecl,

    NoDefaultValue,
)

from bpy.props import *

class NodeBuilder:
    def __init__(self, node):
        self.node = node
        self.input_declarations = []
        self.output_declarations = []
        self._background_color = None

    def _add_input(self, decl):
        self.input_declarations.append(decl)

    def _add_output(self, decl):
        self.output_declarations.append(decl)

    def initialize_decls(self):
        for decl in self.input_declarations:
            decl.init()

        for decl in self.output_declarations:
            decl.init()

    def build(self):
        from . sync import skip_syncing
        with skip_syncing():
            self.node.inputs.clear()
            self.node.outputs.clear()

            for decl in self.input_declarations:
                sockets = decl.build(self.node.inputs)
                assert len(sockets) == decl.amount()

            for decl in self.output_declarations:
                sockets = decl.build(self.node.outputs)
                assert len(sockets) == decl.amount()

            if self._background_color is not None:
                self.node.use_custom_color = True
                self.node.color = self._background_color

    def init_defaults(self):
        for decl, sockets in self.get_sockets_decl_map().iter_decl_with_sockets():
            decl.init_default(sockets)

    def get_sockets_decl_map(self):
        return SocketDeclMap(
            self.node,
            self.input_declarations,
            self.output_declarations)

    def matches_sockets(self):
        if not self._declarations_matches_sockets(self.input_declarations, self.node.inputs):
            return False
        if not self._declarations_matches_sockets(self.output_declarations, self.node.outputs):
            return False
        return True

    def _declarations_matches_sockets(self, declarations, all_sockets):
        sockets_iter = iter(all_sockets)
        for decl in declarations:
            amount = decl.amount()
            try: sockets = [next(sockets_iter) for _ in range(amount)]
            except StopIteration: return False
            if not decl.validate(sockets):
                return False
        if len(tuple(sockets_iter)) > 0:
            return False
        return True

    # General Node Properties
    ###################################

    def background_color(self, color):
        assert len(color) == 3
        self._background_color = color


    # Fixed Data Types
    ###################################

    def fixed_input(self, identifier, name, data_type, *, default=NoDefaultValue):
        decl = FixedSocketDecl(self.node, identifier, name, data_type, default)
        self._add_input(decl)

    def fixed_output(self, identifier, name, data_type, *, default=NoDefaultValue):
        decl = FixedSocketDecl(self.node, identifier, name, data_type, default)
        self._add_output(decl)

    def fixed_pass_through(self, identifier, name, data_type, *, default=NoDefaultValue):
        self.fixed_input(identifier, name, data_type, default=default)
        self.fixed_output(identifier, name, data_type, default=default)


    # Packed List
    ###################################

    @staticmethod
    def PackListProperty():
        return PackListDecl.Property()

    def pack_list_input(self, identifier, prop_name, base_type, default_amount=2):
        decl = PackListDecl(self.node, identifier, prop_name, base_type, default_amount)
        self._add_input(decl)


    # Dynamic List
    ###################################

    @staticmethod
    def DynamicListProperty():
        return ListSocketDecl.Property()

    def dynamic_list_input(self, identifier, name, prop_name):
        decl = ListSocketDecl(self.node, identifier, name, prop_name, "LIST")
        self._add_input(decl)

    def dynamic_list_output(self, identifier, name, prop_name):
        decl = ListSocketDecl(self.node, identifier, name, prop_name, "LIST")
        self._add_output(decl)

    def dynamic_base_input(self, identifier, name, prop_name):
        decl = ListSocketDecl(self.node, identifier, name, prop_name, "BASE")
        self._add_input(decl)

    def dynamic_base_output(self, identifier, name, prop_name):
        decl = ListSocketDecl(self.node, identifier, name, prop_name, "BASE")
        self._add_output(decl)


    # Variadic
    ##################################

    @staticmethod
    def VariadicProperty():
        return AnyVariadicDecl.Property()

    def variadic_input(self, identifier, prop_name, message):
        decl = AnyVariadicDecl(self.node, identifier, prop_name, message)
        self._add_input(decl)

    def variadic_output(self, identifier, prop_name, message):
        decl = AnyVariadicDecl(self.node, identifier, prop_name, message)
        self._add_output(decl)


    # Tree Interface
    ##################################

    def tree_interface_input(self, identifier, tree, in_or_out, *, ignored=set()):
        decl = TreeInterfaceDecl(self.node, identifier, tree, in_or_out, ignored)
        self._add_input(decl)

    def tree_interface_output(self, identifier, tree, in_or_out):
        decl = TreeInterfaceDecl(self.node, identifier, tree, in_or_out, set())
        self._add_output(decl)


    # Vectorized
    ##################################

    @staticmethod
    def VectorizedProperty():
        return VectorizedInputDecl.Property()

    def vectorized_input(self, identifier, prop_name, base_name, list_name, base_type, *, default=NoDefaultValue):
        decl = VectorizedInputDecl(
            self.node, identifier, prop_name,
            base_name, list_name, base_type,
            default)
        self._add_input(decl)

    def vectorized_output(self, identifier, input_prop_names, base_name, list_name, base_type):
        decl = VectorizedOutputDecl(
            self.node, identifier, input_prop_names,
            base_name, list_name, base_type)
        self._add_output(decl)


    # BParticles
    ###################################

    @staticmethod
    def get_particle_type_items(self, context):
        items = []
        for node in self.id_data.nodes:
            if node.bl_idname == "bp_ParticleTypeNode":
                items.append((node.name, node.name, ""))
        items.append(("NONE", "None", ""))
        return items

    @classmethod
    def ParticleTypeProperty(cls):
        from . utils.enum_items_cache import cache_enum_items
        return EnumProperty(items=cache_enum_items(cls.get_particle_type_items))

    @staticmethod
    def draw_particle_type_prop(layout, node, identifier, *, text=""):
        layout.prop(node, identifier, text=text, icon='MOD_PARTICLES')

    def control_flow_input(self, identifier, name):
        decl = ControlFlowSocketDecl(self.node, identifier, name)
        self._add_input(decl)

    def control_flow_output(self, identifier, name):
        decl = ControlFlowSocketDecl(self.node, identifier, name)
        self._add_output(decl)


class SocketDeclMap:
    def __init__(self, node, input_declarations, output_declarations):
        self.node = node
        self._sockets_by_decl = dict()
        self._decl_by_socket = dict()
        self._socket_index_in_decl = dict()

        for decl, sockets in iter_sockets_by_decl(node.inputs, input_declarations):
            self._sockets_by_decl[decl] = sockets
            for i, socket in enumerate(sockets):
                self._decl_by_socket[socket] = decl
                self._socket_index_in_decl[socket] = i

        for decl, sockets in iter_sockets_by_decl(node.outputs, output_declarations):
            self._sockets_by_decl[decl] = sockets
            for i, socket in enumerate(sockets):
                self._decl_by_socket[socket] = decl
                self._socket_index_in_decl[socket] = i

    def get_decl_by_socket(self, socket):
        return self._decl_by_socket[socket]

    def get_socket_index_in_decl(self, socket):
        return self._socket_index_in_decl[socket]

    def get_sockets_by_decl(self, decl):
        return self._sockets_by_decl[decl]

    def iter_decl_with_sockets(self):
        yield from self._sockets_by_decl.items()

    def iter_decls(self):
        yield from self._sockets_by_decl.keys()


def iter_sockets_by_decl(node_sockets, declarations):
    node_sockets_iter = iter(node_sockets)
    for decl in declarations:
        amount = decl.amount()
        sockets_of_decl = tuple(next(node_sockets_iter) for _ in range(amount))
        assert decl.validate(sockets_of_decl)
        yield decl, sockets_of_decl

#include "FN_expression_multi_function.h"
#include "FN_expression_parser.h"
#include "FN_multi_function_network.h"
#include "FN_multi_functions.h"

namespace FN {
namespace Expr {

MFBuilderOutputSocket &build_node(AstNode &ast_node,
                                  MFNetworkBuilder &network_builder,
                                  ResourceCollector &resources)
{
  switch (ast_node.type) {
    case AstNodeType::Less: {
      BLI_assert(false);
      break;
    }
    case AstNodeType::Greater: {
      BLI_assert(false);
      break;
    }
    case AstNodeType::Equal: {
      BLI_assert(false);
      break;
    }
    case AstNodeType::LessOrEqual: {
      BLI_assert(false);
      break;
    }
    case AstNodeType::GreaterOrEqual: {
      BLI_assert(false);
      break;
    }
    case AstNodeType::Plus: {
      MFBuilderOutputSocket &sub1 = build_node(*ast_node.children[0], network_builder, resources);
      MFBuilderOutputSocket &sub2 = build_node(*ast_node.children[1], network_builder, resources);
      const CPPType &type1 = sub1.data_type().single__cpp_type();
      const CPPType &type2 = sub2.data_type().single__cpp_type();

      const MultiFunction *fn = nullptr;
      if (type1 == CPP_TYPE<int>() && type2 == CPP_TYPE<int>()) {
        fn = &resources.construct<MF_Custom_In2_Out1<int, int, int>>(
            "add", "add", [](int a, int b) { return a + b; });
      }
      else if (type1 == CPP_TYPE<int>() && type2 == CPP_TYPE<float>()) {
        fn = &resources.construct<MF_Custom_In2_Out1<int, float, float>>(
            "add", "add", [](int a, float b) { return (float)a + b; });
      }
      else if (type1 == CPP_TYPE<float>() && type2 == CPP_TYPE<int>()) {
        fn = &resources.construct<MF_Custom_In2_Out1<float, int, float>>(
            "add", "add", [](float a, int b) { return a + (float)b; });
      }
      else if (type1 == CPP_TYPE<float>() && type2 == CPP_TYPE<float>()) {
        fn = &resources.construct<MF_Custom_In2_Out1<float, float, float>>(
            "add", "add", [](float a, float b) { return (float)a + b; });
      }
      else {
        BLI_assert(false);
      }

      MFBuilderNode &node = network_builder.add_function(*fn);
      network_builder.add_link(sub1, node.input(0));
      network_builder.add_link(sub2, node.input(1));
      return node.output(0);
    }
    case AstNodeType::Minus: {
      BLI_assert(false);
      break;
    }
    case AstNodeType::Multiply: {
      BLI_assert(false);
      break;
    }
    case AstNodeType::Divide: {
      BLI_assert(false);
      break;
    }
    case AstNodeType::Identifier: {
      BLI_assert(false);
      break;
    }
    case AstNodeType::ConstantInt: {
      ConstantIntNode &int_node = (ConstantIntNode &)ast_node;
      const MultiFunction &fn = resources.construct<MF_ConstantValue<int>>("constant int",
                                                                           int_node.value);
      return network_builder.add_function(fn).output(0);
    }
    case AstNodeType::ConstantFloat: {
      ConstantFloatNode &float_node = (ConstantFloatNode &)ast_node;
      const MultiFunction &fn = resources.construct<MF_ConstantValue<float>>("constant float",
                                                                             float_node.value);
      return network_builder.add_function(fn).output(0);
    }
    case AstNodeType::ConstantString: {
      BLI_assert(false);
      break;
    }
    case AstNodeType::Negate: {
      MFBuilderOutputSocket &sub_output = build_node(
          *ast_node.children[0], network_builder, resources);
      const MultiFunction *fn = nullptr;
      if (sub_output.data_type().single__cpp_type() == CPP_TYPE<int>()) {
        fn = &resources.construct<MF_Custom_In1_Out1<int, int>>(
            "negate", "negate", [](int a) -> int { return -a; });
      }
      else if (sub_output.data_type().single__cpp_type() == CPP_TYPE<float>()) {
        fn = &resources.construct<MF_Custom_In1_Out1<float, float>>(
            "negate", "negate", [](float a) -> float { return -a; });
      }
      else {
        BLI_assert(false);
      }
      MFBuilderNode &node = network_builder.add_function(*fn);
      network_builder.add_link(sub_output, node.input(0));
      return node.output(0);
    }
    case AstNodeType::Power: {
      BLI_assert(false);
      break;
    }
  }
  BLI_assert(false);
  return network_builder.node_by_id(0).output(0);
}

const MultiFunction &expression_to_multi_function(StringRef str, ResourceCollector &resources)
{
  AstNode &ast_node = parse_expression(str, resources.allocator());
  MFNetworkBuilder network_builder;
  MFBuilderOutputSocket &builder_output_socket = build_node(ast_node, network_builder, resources);
  MFBuilderDummyNode &builder_output = network_builder.add_output_dummy("Result",
                                                                        builder_output_socket);

  MFNetwork &network = resources.construct<MFNetwork>("expression network", network_builder);
  const MFInputSocket &output_socket = network.find_dummy_socket(builder_output.input(0));

  Vector<const MFOutputSocket *> inputs;
  Vector<const MFInputSocket *> outputs;
  outputs.append(&output_socket);

  const MultiFunction &fn = resources.construct<MF_EvaluateNetwork>(
      "expression function", inputs, outputs);

  return fn;
}

}  // namespace Expr
}  // namespace FN

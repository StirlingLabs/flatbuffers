/*
 * Copyright 2014 Google Inc. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

// independent from idl_parser, since this code is not needed for most clients

#include <unordered_set>

#include "flatbuffers/code_generators.h"
#include "flatbuffers/flatbuffers.h"
#include "flatbuffers/idl.h"
#include "flatbuffers/util.h"

#if defined(FLATBUFFERS_CPP98_STL)
#  include <cctype>
#endif  // defined(FLATBUFFERS_CPP98_STL)

namespace flatbuffers {

static TypedFloatConstantGenerator CSharpFloatGen("Double.", "Single.", "NaN",
                                                  "PositiveInfinity",
                                                  "NegativeInfinity");
static CommentConfig comment_config = {
  nullptr,
  "///",
  nullptr,
};

namespace csharp {
class CSharpGenerator : public BaseGenerator {
  struct FieldArrayLength {
    std::string name;
    int length;
  };

 public:
  CSharpGenerator(const Parser &parser, const std::string &path,
                  const std::string &file_name)
      : BaseGenerator(parser, path, file_name, "", ".", "cs"),
        cur_name_space_(nullptr) {
    // clang-format off

    // List of keywords retrieved from here:
    // https://docs.microsoft.com/en-us/dotnet/csharp/language-reference/keywords/

    // One per line to ease comparisons to that list are easier

    static const char *const keywords[] = {
      "abstract",
      "as",
      "base",
      "bool",
      "break",
      "byte",
      "case",
      "catch",
      "char",
      "checked",
      "class",
      "const",
      "continue",
      "decimal",
      "default",
      "delegate",
      "do",
      "double",
      "else",
      "enum",
      "event",
      "explicit",
      "extern",
      "false",
      "finally",
      "fixed",
      "float",
      "for",
      "foreach",
      "goto",
      "if",
      "implicit",
      "in",
      "int",
      "interface",
      "internal",
      "is",
      "lock",
      "long",
      "namespace",
      "new",
      "null",
      "object",
      "operator",
      "out",
      "override",
      "params",
      "private",
      "protected",
      "public",
      "readonly",
      "ref",
      "return",
      "sbyte",
      "sealed",
      "short",
      "sizeof",
      "stackalloc",
      "static",
      "string",
      "struct",
      "switch",
      "this",
      "throw",
      "true",
      "try",
      "typeof",
      "uint",
      "ulong",
      "unchecked",
      "unsafe",
      "ushort",
      "using",
      "virtual",
      "void",
      "volatile",
      "while",
      nullptr,
      // clang-format on
    };

    for (auto kw = keywords; *kw; kw++) keywords_.insert(*kw);
  }

  CSharpGenerator &operator=(const CSharpGenerator &);

  bool generate() {
    std::string one_file_code;
    cur_name_space_ = parser_.current_namespace_;

    for (auto it = parser_.enums_.vec.begin(); it != parser_.enums_.vec.end();
         ++it) {
      std::string enumcode;
      auto &enum_def = **it;
      if (!parser_.opts.one_file) cur_name_space_ = enum_def.defined_namespace;
      GenEnum(enum_def, &enumcode, parser_.opts);
      if (parser_.opts.one_file) {
        one_file_code += enumcode;
      } else {
        if (!SaveType(enum_def.name, *enum_def.defined_namespace, enumcode,
                      false, parser_.opts))
          return false;
      }
    }

    for (auto it = parser_.structs_.vec.begin();
         it != parser_.structs_.vec.end(); ++it) {
      std::string declcode;
      auto &struct_def = **it;
      if (!parser_.opts.one_file)
        cur_name_space_ = struct_def.defined_namespace;
      GenStruct(struct_def, &declcode, parser_.opts);
      if (parser_.opts.one_file) {
        one_file_code += declcode;
      } else {
        if (!SaveType(struct_def.name, *struct_def.defined_namespace, declcode,
                      true, parser_.opts))
          return false;
      }
    }

    if (parser_.opts.one_file) {
      return SaveType(file_name_, *parser_.current_namespace_, one_file_code,
                      true, parser_.opts);
    }
    return true;
  }

 private:
  std::unordered_set<std::string> keywords_;

  std::string EscapeKeyword(const std::string &name) const {
    return keywords_.find(name) == keywords_.end() ? name : name + "_";
  }

  std::string Name(const FieldDef &field) const {
    std::string name = MakeCamel(field.name, true);
    return EscapeKeyword(name);
  }

  std::string Name(const Definition &def) const {
    return EscapeKeyword(def.name);
  }

  std::string NamespacedName(const Definition &def) const {
    return WrapInNameSpace(def.defined_namespace, Name(def));
  }

  std::string Name(const EnumVal &ev) const { return EscapeKeyword(ev.name); }

  // Save out the generated code for a single class while adding
  // declaration boilerplate.
  bool SaveType(const std::string &defname, const Namespace &ns,
                const std::string &classcode, bool needs_includes,
                const IDLOptions &options) const {
    if (!classcode.length()) return true;

    std::string code =
        "// <auto-generated>\n"
        "//  " +
        std::string(FlatBuffersGeneratedWarning()) +
        "\n"
        "// </auto-generated>\n\n";

    std::string namespace_name = FullNamespace(".", ns);
    if (!namespace_name.empty()) {
      code += "namespace " + namespace_name + "\n{\n\n";
    }
    if (needs_includes) {
      code += "using global::System;\n";
      code += "using global::System.Collections.Generic;\n";
      code += "using global::BigBuffers;\n\n";
    }
    code += classcode;
    if (!namespace_name.empty()) { code += "\n}\n"; }
    auto filename = NamespaceDir(ns) + defname;
    if (options.one_file) { filename += options.filename_suffix; }
    filename +=
        options.filename_extension.empty() ? ".cs" : options.filename_extension;
    return SaveFile(filename.c_str(), code, false);
  }

  const Namespace *CurrentNameSpace() const { return cur_name_space_; }

  std::string GenTypeBasic(const Type &type, bool enableLangOverrides) const {
    // clang-format off
    static const char * const csharp_typename[] = {
      #define FLATBUFFERS_TD(ENUM, IDLTYPE, CTYPE, JTYPE, GTYPE, NTYPE, ...) \
        #NTYPE,
        FLATBUFFERS_GEN_TYPES(FLATBUFFERS_TD)
      #undef FLATBUFFERS_TD
    };
    // clang-format on

    if (enableLangOverrides) {
      if (IsEnum(type)) return NamespacedName(*type.enum_def);
      if (type.base_type == BASE_TYPE_STRUCT) {
        return "Offset<" + NamespacedName(*type.struct_def) + ">";
      }
    }

    return csharp_typename[type.base_type];
  }

  inline std::string GenTypeBasic(const Type &type) const {
    return GenTypeBasic(type, true);
  }

  std::string GenTypePointer(const Type &type) const {
    switch (type.base_type) {
      case BASE_TYPE_STRING: return "string";
      case BASE_TYPE_VECTOR: return GenTypeGet(type.VectorType());
      case BASE_TYPE_STRUCT: return NamespacedName(*type.struct_def);
      case BASE_TYPE_UNION: return "TTable";
      default: return "Table";
    }
  }

  std::string GenTypeGet(const Type &type) const {
    return IsScalar(type.base_type)
               ? GenTypeBasic(type)
               : (IsArray(type) ? GenTypeGet(type.VectorType())
                                : GenTypePointer(type));
  }

  std::string GenOffsetType(const StructDef &struct_def) const {
    return "Offset<" + NamespacedName(struct_def) + ">";
  }

  std::string GenOffsetConstruct(const StructDef &struct_def,
                                 const std::string &variable_name) const {
    return "new Offset<" + NamespacedName(struct_def) + ">(" + variable_name +
           ")";
  }

  // Casts necessary to correctly read serialized data
  std::string DestinationCast(const Type &type) const {
    if (IsSeries(type)) {
      return DestinationCast(type.VectorType());
    } else {
      if (IsEnum(type)) return "(" + NamespacedName(*type.enum_def) + ")";
    }
    return "";
  }

  // Cast statements for mutator method parameters.
  // In Java, parameters representing unsigned numbers need to be cast down to
  // their respective type. For example, a long holding an unsigned int value
  // would be cast down to int before being put onto the buffer. In C#, one cast
  // directly cast an Enum to its underlying type, which is essential before
  // putting it onto the buffer.
  std::string SourceCast(const Type &type) const {
    if (IsSeries(type)) {
      return SourceCast(type.VectorType());
    } else {
      if (IsEnum(type)) return "(" + GenTypeBasic(type, false) + ")";
    }
    return "";
  }

  std::string SourceCastBasic(const Type &type) const {
    return IsScalar(type.base_type) ? SourceCast(type) : "";
  }

  std::string GenEnumDefaultValue(const FieldDef &field) const {
    auto &value = field.value;
    FLATBUFFERS_ASSERT(value.type.enum_def);
    auto &enum_def = *value.type.enum_def;
    auto enum_val = enum_def.FindByValue(value.constant);
    return enum_val ? (NamespacedName(enum_def) + "." + Name(*enum_val))
                    : value.constant;
  }

  std::string GenDefaultValue(const FieldDef &field,
                              bool enableLangOverrides) const {
    // If it is an optional scalar field, the default is null
    if (field.IsScalarOptional()) { return "null"; }

    auto &value = field.value;
    if (enableLangOverrides) {
      // handles both enum case and vector of enum case
      if (value.type.enum_def != nullptr &&
          value.type.base_type != BASE_TYPE_UNION) {
        return GenEnumDefaultValue(field);
      }
    }

    auto longSuffix = "";
    switch (value.type.base_type) {
      case BASE_TYPE_BOOL: return value.constant == "0" ? "false" : "true";
      case BASE_TYPE_ULONG: return value.constant;
      case BASE_TYPE_UINT:
      case BASE_TYPE_LONG: return value.constant + longSuffix;
      default:
        if (IsFloat(value.type.base_type))
          return CSharpFloatGen.GenFloatConstant(field);
        else
          return value.constant;
    }
  }

  std::string GenDefaultValue(const FieldDef &field) const {
    return GenDefaultValue(field, true);
  }

  std::string GenDefaultValueBasic(const FieldDef &field,
                                   bool enableLangOverrides) const {
    auto &value = field.value;
    if (!IsScalar(value.type.base_type)) {
      if (enableLangOverrides) {
        switch (value.type.base_type) {
          case BASE_TYPE_STRING: return "default(StringOffset)";
          case BASE_TYPE_STRUCT:
            return "default(Offset<" + NamespacedName(*value.type.struct_def) +
                   ">)";
          case BASE_TYPE_VECTOR: return "default(VectorOffset)";
          default: break;
        }
      }
      return "0";
    }
    return GenDefaultValue(field, enableLangOverrides);
  }

  std::string GenDefaultValueBasic(const FieldDef &field) const {
    return GenDefaultValueBasic(field, true);
  }

  void GenEnum(EnumDef &enum_def, std::string *code_ptr,
               const IDLOptions &opts) const {
    std::string &code = *code_ptr;
    if (enum_def.generated) return;

    // Generate enum definitions of the form:
    // public static (final) int name = value;
    // In Java, we use ints rather than the Enum feature, because we want them
    // to map directly to how they're used in C/C++ and file formats.
    // That, and Java Enums are expensive, and not universally liked.
    GenComment(enum_def.doc_comment, code_ptr, &comment_config);

    if (opts.cs_gen_json_serializer && opts.generate_object_based_api) {
      code +=
          "[Newtonsoft.Json.JsonConverter(typeof(Newtonsoft.Json.Converters."
          "StringEnumConverter))]\n";
    }
    // In C# this indicates enumeration values can be treated as bit flags.
    if (enum_def.attributes.Lookup("bit_flags")) {
      code += "[System.FlagsAttribute]\n";
    }
    if (enum_def.attributes.Lookup("private")) {
      code += "internal ";
    } else {
      code += "public ";
    }
    code += "enum " + Name(enum_def);
    code += " : " + GenTypeBasic(enum_def.underlying_type, false);
    code += "\n{\n";
    for (auto it = enum_def.Vals().begin(); it != enum_def.Vals().end(); ++it) {
      auto &ev = **it;
      GenComment(ev.doc_comment, code_ptr, &comment_config, "  ");
      code += "  ";
      code += Name(ev) + " = ";
      code += enum_def.ToString(ev);
      code += ",\n";
    }
    // Close the class
    code += "}\n\n";

    if (opts.generate_object_based_api) {
      GenEnum_ObjectAPI(enum_def, code_ptr, opts);
    }
  }

  bool HasUnionStringValue(const EnumDef &enum_def) const {
    if (!enum_def.is_union) return false;
    for (auto it = enum_def.Vals().begin(); it != enum_def.Vals().end(); ++it) {
      auto &val = **it;
      if (IsString(val.union_type)) { return true; }
    }
    return false;
  }

  // Returns the function name that is able to read a value of the given type.
  [[nodiscard]] std::string GenGetter(const Type &type) const {
    switch (type.base_type) {
      case BASE_TYPE_STRING: return "_model.__string";
      case BASE_TYPE_STRUCT: return "_model.__struct";
      case BASE_TYPE_UNION: return "_model.__union";
      case BASE_TYPE_VECTOR: return GenGetter(type.VectorType());
      case BASE_TYPE_ARRAY: return GenGetter(type.VectorType());
      default: {
        std::string getter = "_model.ByteBuffer.";
        std::string basicType = GenTypeBasic(type, false);
        if (basicType == "byte") {
          getter += "GetByte";
        } else if (basicType != "byte") {
          getter += "Get<" + basicType + ">";
        }
        return getter;
      }
    }
  }

  // Returns the function name that is able to point to a value of the given type.
  [[nodiscard]] std::string GenReffer(const Type &type) const {
    switch (type.base_type) {
      case BASE_TYPE_STRING: return std::string();
      case BASE_TYPE_STRUCT: return "ref _model.__struct";
      case BASE_TYPE_UNION: return "ref _model.__union";
      case BASE_TYPE_VECTOR: return GenReffer(type.VectorType());
      case BASE_TYPE_ARRAY: return GenReffer(type.VectorType());
      default: {
        std::string getter = "ref _model.ByteBuffer.";
        std::string basicType = GenTypeBasic(type, false);
        if (basicType == "byte") {
          getter += "RefByte";
        } else if (basicType != "byte") {
          getter += "Ref<" + basicType + ">";
        }
        return getter;
      }
    }
  }


  [[nodiscard]] std::string GenNullRefThrower(const Type &type) const {
    std::string getter = "ref SchemaModel.ThrowNullRef<";
    getter += GenTypeBasic(type, false) + ">()";
    return getter;
  }

  // Returns the function name that is able to read a value of the given type.
  std::string GenGetterForLookupByKey(flatbuffers::FieldDef *key_field,
                                      const std::string &data_buffer,
                                      const char *num = nullptr) const {
    auto type = key_field->value.type;
    auto dest_mask = "";
    auto dest_cast = DestinationCast(type);
    auto getter = data_buffer + ".Get<";
    if (GenTypeBasic(type, false) != "byte") {
      getter += GenTypeBasic(type, false);
    }
    getter += ">";
    getter = dest_cast + getter + "(" + GenOffsetGetter(key_field, num) + ")" +
             dest_mask;
    return getter;
  }

  // Direct mutation is only allowed for scalar fields.
  // Hence a setter method will only be generated for such fields.
  [[nodiscard]] std::string GenSetter(const Type &type) const {
    if (IsScalar(type.base_type)) {
      std::string setter = "_model.ByteBuffer.Put<";
      setter += GenTypeBasic(type, false);
      setter += ">";
      return setter;
    } else {
      return "";
    }
  }

  // Direct mutation is only allowed for scalar fields.
  // Hence a setter method will only be generated for such fields.
  [[nodiscard]] std::string GenSpanner(const Type &type) const {
    if (IsScalar(type.base_type)) {
      std::string spanner = "return _model.ByteBuffer.GetSpan<";
      spanner += GenTypeBasic(type, false);
      spanner += ">";
      return spanner;
    } else {
      return "";
    }
  }

  // Returns the method name for use with add/put calls.
  [[nodiscard]] std::string GenMethod(const Type &type) const {
    return IsScalar(type.base_type) ? "<" + GenTypeBasic(type, false) + ">"
                                    : (IsStruct(type) ? "Struct" : "Offset");
  }

  // Recursively generate arguments for a constructor, to deal with nested
  // structs.
  void GenStructArgs(const StructDef &struct_def, std::string *code_ptr,
                     const char *nameprefix, size_t array_count = 0) const {
    std::string &code = *code_ptr;
    for (auto it = struct_def.fields.vec.begin();
         it != struct_def.fields.vec.end(); ++it) {
      auto &field = **it;
      const auto &field_type = field.value.type;
      const auto array_field = IsArray(field_type);
      const auto &type = array_field ? field_type.VectorType() : field_type;
      const auto array_cnt = array_field ? (array_count + 1) : array_count;
      if (IsStruct(type)) {
        // Generate arguments for a struct inside a struct. To ensure names
        // don't clash, and to make it obvious these arguments are constructing
        // a nested struct, prefix the name with the field name.
        GenStructArgs(*field_type.struct_def, code_ptr,
                      (nameprefix + (EscapeKeyword(field.name) + "_")).c_str(),
                      array_cnt);
      } else {
        code += ", ";
        if (array_cnt > 0) {
          code += "StirlingLabs.Utilities.ReadOnlyBigSpan<";
          code += GenTypeBasic(type);
          code += ">";
        } else {
          code += GenTypeBasic(type);
          if (field.IsScalarOptional())
            code += "?";
        }
        code += " ";
        code += nameprefix;
        code += Name(field);
      }
    }
  }

  // Recusively generate struct construction statements of the form:
  // builder.putType(name);
  // and insert manual padding.
  void GenStructBody(const StructDef &struct_def, std::string *code_ptr,
                     const char *nameprefix, size_t index = 0,
                     bool in_array = false) const {
    std::string &code = *code_ptr;
    std::string indent((index + 1) * 2, ' ');
    code += indent + "  builder.Prep(";
    code += NumToString(struct_def.minalign) + ", ";
    code += NumToString(struct_def.bytesize) + ");\n";
    for (auto it = struct_def.fields.vec.rbegin();
         it != struct_def.fields.vec.rend(); ++it) {
      auto &field = **it;
      const auto &field_type = field.value.type;
      if (field.padding) {
        code += indent + "  builder.Pad(";
        code += NumToString(field.padding) + ");\n";
      }
      if (IsStruct(field_type)) {
        GenStructBody(*field_type.struct_def, code_ptr,
                      (nameprefix + (field.name + "_")).c_str(), index,
                      in_array);
      } else {
        const auto &type =
            IsArray(field_type) ? field_type.VectorType() : field_type;
        const auto index_var = "_idx" + NumToString(index);


        auto argname = nameprefix + MakeCamel(field.name, true);

        if (IsArray(field_type) && field_type.fixed_length > 0) {
          auto lenStr = NumToString(field_type.fixed_length);
          code += indent + "  if(" + lenStr + " != "+argname+".LongLength)\n";
          code += indent + "    throw new System.ArgumentException(\"Must be of length "+lenStr+"\", nameof("+argname+"));\n";
        }

        code += indent + "  builder.Put";
        code += GenMethod(type) + "(";
        code += SourceCast(type);
          auto argname = nameprefix + Name(field);
        code += argname;
        code += ");\n";
      }
    }
  }
  std::string GenOffsetGetter(flatbuffers::FieldDef *key_field,
                              const char *num = nullptr) const {
    std::string key_offset =
        "SchemaModel.__offset(" + NumToString(key_field->value.offset) + ", ";
    if (num) {
      key_offset += num;
      key_offset += ".Value, builder.ByteBuffer)";
    } else {
      key_offset += "bb.Length";
      key_offset += " - tableOffset, bb)";
    }
    return key_offset;
  }

  std::string GenLookupKeyGetter(flatbuffers::FieldDef *key_field) const {
    std::string key_getter = "      ";
    key_getter += "var tableOffset = SchemaModel.";
    key_getter += "__indirect(vectorLocation + sizeof(ulong) * (start + middle)";
    key_getter += ", bb);\n      ";
    if (IsString(key_field->value.type)) {
      key_getter += "var comp = SchemaModel.";
      key_getter += "CompareStrings(";
      key_getter += GenOffsetGetter(key_field);
      key_getter += ", byteKey, bb);\n";
    } else {
      auto get_val = GenGetterForLookupByKey(key_field, "bb");
      key_getter += "var comp = " + get_val + ".CompareTo(key);\n";
    }
    return key_getter;
  }

  std::string GenKeyGetter(flatbuffers::FieldDef *key_field) const {
    std::string key_getter = "";
    auto data_buffer = "builder.ByteBuffer";
    if (IsString(key_field->value.type)) {
      key_getter += "SchemaModel.CompareStrings(";
      key_getter += GenOffsetGetter(key_field, "o1") + ", ";
      key_getter += GenOffsetGetter(key_field, "o2") + ", " + data_buffer + ")";
    } else {
      auto field_getter = GenGetterForLookupByKey(key_field, data_buffer, "o1");
      key_getter += field_getter;
      field_getter = GenGetterForLookupByKey(key_field, data_buffer, "o2");
      key_getter += ".CompareTo(" + field_getter + ")";
    }
    return key_getter;
  }

  bool GenField(std::string &code, FieldDef &field, StructDef &struct_def,
                bool hasRefField) const {
    std::string type_name = GenTypeGet(field.value.type);
    std::string type_name_dest = GenTypeGet(field.value.type);
    std::string conditional_cast = "";
    std::string optional = "";
    if (!struct_def.fixed &&
        (field.value.type.base_type == BASE_TYPE_STRUCT ||
            field.value.type.base_type == BASE_TYPE_UNION ||
            (IsVector(field.value.type) &&
                (field.value.type.element == BASE_TYPE_STRUCT ||
                    field.value.type.element == BASE_TYPE_UNION)))) {
      optional = "?";
      conditional_cast = "(" + type_name_dest + optional + ")";
    }
    if (field.IsScalarOptional()) { optional = "?"; }
    std::string dest_mask = "";
    std::string dest_cast = DestinationCast(field.value.type);
    std::string src_cast = SourceCast(field.value.type);
      std::string field_name_camel = Name(field);
      if (field_name_camel == struct_def.name) { field_name_camel += "_"; }
    std::string method_start = "  public " + type_name_dest + optional + " ";
    if (hasRefField) method_start += "Get";
    method_start += field_name_camel;
    std::string obj = "(new " + type_name + "())";

    // Most field accessors need to retrieve and test the field offset first,
    // this is the prefix code for that:
    auto offset_prefix =
        IsArray(field.value.type)
            ? " { return "
            : (" { var o = _model.__offset(" + NumToString(field.value.offset) +
            "); return o != 0 ? ");
    // Generate the accessors that don't do object reuse.
    if (field.value.type.base_type == BASE_TYPE_STRUCT) {
    } else if (IsVector(field.value.type) &&
        field.value.type.element == BASE_TYPE_STRUCT) {
    } else if (field.value.type.base_type == BASE_TYPE_UNION ||
        (IsVector(field.value.type) &&
            field.value.type.VectorType().base_type == BASE_TYPE_UNION)) {
      method_start += "<TTable>";
      type_name = type_name_dest;
    }
    std::string getter = dest_cast + GenGetter(field.value.type);
    code += method_start;
    std::string default_cast = "";
    // only create default casts for c# scalars or vectors of scalars
    if ((IsScalar(field.value.type.base_type) ||
        (IsVector(field.value.type) &&
            IsScalar(field.value.type.element)))) {
      // For scalars, default value will be returned by GetDefaultValue().
      // If the scalar is an enum, GetDefaultValue() returns an actual c# enum
      // that doesn't need to be casted. However, default values for enum
      // elements of vectors are integer literals ("0") and are still casted
      // for clarity.
      // If the scalar is optional and enum, we still need the cast.
      if ((field.value.type.enum_def == nullptr ||
          IsVector(field.value.type)) ||
          (IsEnum(field.value.type) && field.IsScalarOptional())) {
        default_cast = "(" + type_name_dest + optional + ")";
      }
    }
    std::string member_suffix = "; ";
    if (IsScalar(field.value.type.base_type)) {
      if (hasRefField) {
        code += "()";
      } else {
        code += " { get";
        member_suffix += "} ";
      }
      if (struct_def.fixed) {
        code += " { return " + getter;
        code += "(_model.Offset + ";
        code += NumToString(field.value.offset) + ")";
        code += dest_mask;
      } else {
        code += offset_prefix + getter;
        code += "(o + _model.Offset)" + dest_mask;
        code += " : " + default_cast;
        code += GenDefaultValue(field);
      }
    } else {
      switch (field.value.type.base_type) {
        case BASE_TYPE_STRUCT:
          if (hasRefField) {
            code += "()";
          } else {
            code += " { get";
            member_suffix += "} ";
          }
          if (struct_def.fixed) {
            code += " { return " + obj + ".__assign(" + "_model.";
            code += "Offset + " + NumToString(field.value.offset) + ", ";
            code += "_model.ByteBuffer)";
          } else {
            code += offset_prefix + conditional_cast;
            code += obj + ".__assign(";
            code += field.value.type.struct_def->fixed
                ? "o + _model.Offset"
                : "_model.__indirect(o + _model.Offset)";
            code += ", _model.ByteBuffer) : null";
          }
          break;
        case BASE_TYPE_STRING:
          if (hasRefField) {
            code += "()";
          } else {
            code += " { get";
            member_suffix += "} ";
          }
          code += offset_prefix + getter + "(o + " + "_model.";
          code += "Offset) : null";
          break;
        case BASE_TYPE_ARRAY: FLATBUFFERS_FALLTHROUGH();  // fall thru
        case BASE_TYPE_VECTOR: {
          auto vectortype = field.value.type.VectorType();
          if (vectortype.base_type == BASE_TYPE_UNION) {
            conditional_cast = "(TTable?)";
            getter += "<TTable>";
          }
          code += "(";
          if (vectortype.base_type == BASE_TYPE_STRUCT) {
            getter = obj + ".__assign";
          } else if (vectortype.base_type == BASE_TYPE_UNION) {
          }
          code += "ulong j)";
          const auto body = offset_prefix + conditional_cast + getter + "(";
          if (vectortype.base_type == BASE_TYPE_UNION) {
            code += " where TTable : struct, IBigBufferTable" + body;
          } else {
            code += body;
          }
          std::string index = "_model.";
          if (IsArray(field.value.type)) {
            index += "Offset + " + NumToString(field.value.offset) + " + ";
          } else {
            index += "__vector(o) + ";
          }
          index += "j * " + NumToString(InlineSize(vectortype));
          if (vectortype.base_type == BASE_TYPE_STRUCT) {
            code += vectortype.struct_def->fixed
                ? index
                : "_model.__indirect(" + index + ")";
            code += ", _model.ByteBuffer";
          } else {
            code += index;
          }
          code += ")" + dest_mask;
          if (!IsArray(field.value.type)) {
            code += " : ";
            code +=
                field.value.type.element == BASE_TYPE_BOOL
                    ? "false"
                    : (IsScalar(field.value.type.element) ? default_cast + "0"
                    : "null");
          }
          if (vectortype.base_type == BASE_TYPE_UNION &&
              HasUnionStringValue(*vectortype.enum_def)) {
            code += member_suffix;
            code += "}\n";
              code += "  public string " + Name(field) + "AsString(int j)";
            code += offset_prefix + GenGetter(Type(BASE_TYPE_STRING));
            code += "(" + index + ") : null";
          }
          break;
        }
        case BASE_TYPE_UNION:
          code += "() where TTable : struct, IBigBufferTable";
          code += offset_prefix + "(TTable?)" + getter;
          code += "<TTable>(o + _model.Offset) : null";
          if (HasUnionStringValue(*field.value.type.enum_def)) {
            code += member_suffix;
            code += "}\n";
              code += "  public string " + Name(field) + "AsString()";
            code += offset_prefix + GenGetter(Type(BASE_TYPE_STRING));
            code += "(o + _model.Offset) : null";
          }
          // As<> accesors for Unions
          // Loop through all the possible union types and generate an As
          // accessor that casts to the correct type.
          for (auto uit = field.value.type.enum_def->Vals().begin();
               uit != field.value.type.enum_def->Vals().end(); ++uit) {
            auto val = *uit;
            if (val->union_type.base_type == BASE_TYPE_NONE) { continue; }
            auto union_field_type_name = GenTypeGet(val->union_type);
            code += member_suffix + "}\n";
            if (val->union_type.base_type == BASE_TYPE_STRUCT &&
                val->union_type.struct_def->attributes.Lookup("private")) {
              code += "  internal ";
            } else {
              code += "  public ";
            }
            code += union_field_type_name + " ";
            code += field_name_camel + "As" + val->name + "() { return ";
            code += field_name_camel;
            if (IsString(val->union_type)) {
              code += "AsString()";
            } else {
              code += "<" + union_field_type_name + ">().Value";
            }
          }
          break;
        default: FLATBUFFERS_ASSERT(0);
      }
    }
    code += member_suffix;
    code += "}\n";
    if (IsVector(field.value.type)) {
        code += "  public int " + Name(field);
      code += "Length";
      code += " { get";
      code += offset_prefix;
      code += "_model.__vector_len(o) : 0; ";
      code += "} ";
      code += "}\n";
      // See if we should generate a by-key accessor.
      if (field.value.type.element == BASE_TYPE_STRUCT &&
          !field.value.type.struct_def->fixed) {
        auto &sd = *field.value.type.struct_def;
        auto &fields = sd.fields.vec;
        for (auto kit = fields.begin(); kit != fields.end(); ++kit) {
          auto &key_field = **kit;
          if (key_field.key) {
              auto qualified_name = NamespacedName(sd);
            code += "  public " + qualified_name + "? ";
              code += Name(field) + "ByKey(";
            code += GenTypeGet(key_field.value.type) + " key)";
            code += offset_prefix;
            code += qualified_name + ".__lookup_by_key(";
            code += "_model.__vector(o), key, ";
            code += "_model.ByteBuffer) : null; ";
            code += "}\n";
            break;
          }
        }
      }
    }
    // Generate a ByteBuffer accessor for strings & vectors of scalars.
    if ((IsVector(field.value.type) &&
        IsScalar(field.value.type.VectorType().base_type)) ||
        IsString(field.value.type)) {
      code += "  public StirlingLabs.Utilities.BigSpan<" + GenTypeBasic(field.value.type.VectorType()) +
          "> Get";
        code += Name(field);
      code += "Span() { return ";
      code += "_model.__vector_as_span<" +
          GenTypeBasic(field.value.type.VectorType()) + ">(";
      code += NumToString(field.value.offset);
      code += ", " + NumToString(SizeOf(field.value.type.VectorType().base_type));
      code += "); }\n";

      code += "  public ArraySegment<byte>? Get";
        code += Name(field);
      code += "ByteArraySegment() { return ";
      code += "_model.__vector_as_arraysegment(";
      code += NumToString(field.value.offset);
      code += "); }\n";

      // For direct blockcopying the data into a typed array
      code += "  public ";
      code += GenTypeBasic(field.value.type.VectorType());
      code += "[] Get";
        code += Name(field);
      code += "Array() { ";
      if (IsEnum(field.value.type.VectorType())) {
        // Since __vector_as_array does not work for enum types,
        // fill array using an explicit loop.
        code += "var o = _model.__offset(";
        code += NumToString(field.value.offset);
        code += "); if (o == 0) return null; var p = ";
        code += "_model.__vector(o); var l = ";
        code += "_model.__vector_len(o); ";
        code += GenTypeBasic(field.value.type.VectorType());
        code += "[] a = new ";
        code += GenTypeBasic(field.value.type.VectorType());
        code += "[l]; for (var i = 0uL; i < l; i++) { a[i] = " + getter;
        code += "(p + i * ";
        code += NumToString(InlineSize(field.value.type.VectorType()));
        code += "); } return a;";
      } else {
        code += "return ";
        code += "_model.__vector_as_array<";
        code += GenTypeBasic(field.value.type.VectorType());
        code += ">(";
        code += NumToString(field.value.offset);
        code += ");";
      }
      code += " }\n";
    }
    // generate object accessors if is nested_flatbuffer
    if (field.nested_flatbuffer) {
        auto nested_type_name = NamespacedName(*field.nested_flatbuffer);
      auto nested_method_name =
            Name(field) + "As" + field.nested_flatbuffer->name;
      auto get_nested_method_name = nested_method_name;
      get_nested_method_name = "Get" + nested_method_name;
      conditional_cast = "(" + nested_type_name + "?)";
      obj = "(new " + nested_type_name + "())";
      code += "  public " + nested_type_name + "? ";
      code += get_nested_method_name + "(";
      code += ") { var o = _model.__offset(";
      code += NumToString(field.value.offset) + "); ";
      code += "return o != 0 ? " + conditional_cast + obj + ".__assign(";
      code += "_model.";
      code += "__indirect(_model.__vector(o)), ";
      code += "_model.ByteBuffer) : null; }\n";
    }
    // Generate mutators for scalar fields or vectors of scalars.
    if (parser_.opts.mutable_buffer) {
      auto is_series = (IsSeries(field.value.type));
      const auto &underlying_type =
          is_series ? field.value.type.VectorType() : field.value.type;
      // Boolean parameters have to be explicitly converted to byte
      // representation.
      auto setter_parameter = field.name;
      auto mutator_prefix = MakeCamel("set", true);
      // A vector mutator also needs the index of the vector element it should
      // mutate.
      auto mutator_params = (is_series ? "(ulong j, " : "(") +
                              GenTypeGet(underlying_type) + " " +
                              EscapeKeyword(field.name) + ") { ";
      auto setter_index =
          is_series
              ? "_model." +
              (IsArray(field.value.type)
                  ? "Offset + " + NumToString(field.value.offset)
                  : "__vector(o)") +
              +" + j * " + NumToString(InlineSize(underlying_type))
              : (struct_def.fixed
              ? "_model.Offset + " + NumToString(field.value.offset)
              : "o + _model.Offset");
      if (IsScalar(underlying_type.base_type) && !IsUnion(field.value.type)) {
        code += "  public ";
        code += struct_def.fixed ? "void " : "bool ";
          code += mutator_prefix + Name(field);
        code += mutator_params;
        if (struct_def.fixed) {
          code += GenSetter(underlying_type) + "(" + setter_index + ", ";
          code += src_cast + setter_parameter + "); }\n";
        } else {
          code += "var o = _model.__offset(";
          code += NumToString(field.value.offset) + ");";
          code += " if (o != 0) { " + GenSetter(underlying_type);
          code += "(" + setter_index + ", " + src_cast + setter_parameter +
              "); return true; } else { return false; } }\n";
        }
      }
    }
    if (parser_.opts.java_primitive_has_method &&
        IsScalar(field.value.type.base_type) && !struct_def.fixed) {
      auto vt_offset_constant = "  public static final ulong VT_" +
          MakeScreamingCamel(field.name) + " = " +
          NumToString(field.value.offset) + ";";

      code += vt_offset_constant;
      code += "\n";
    }
    return true;
  }


  bool GenRefField(std::string &code, FieldDef &field, StructDef &struct_def) const {
    std::string type_name = GenTypeGet(field.value.type);
    std::string type_name_dest = GenTypeGet(field.value.type);
    std::string conditional_cast = "";
    std::string optional = "";
    bool is_optional = false;
    if (!struct_def.fixed &&
        (field.value.type.base_type == BASE_TYPE_STRUCT ||
            field.value.type.base_type == BASE_TYPE_UNION ||
            (IsVector(field.value.type) &&
                (field.value.type.element == BASE_TYPE_STRUCT ||
                    field.value.type.element == BASE_TYPE_UNION)))) {
      is_optional = true;
      optional = "?";
      conditional_cast = "(" + type_name_dest + optional + ")";
    }
    std::string dest_mask = "";
    std::string dest_cast = DestinationCast(field.value.type);
    std::string src_cast = SourceCast(field.value.type);
    std::string field_name_camel = MakeCamel(field.name, true);

    std::string method_start = "  public ref ";
    if (!parser_.opts.mutable_buffer) method_start += "readonly ";
    method_start += type_name_dest + " " + field_name_camel;

    std::string obj = "(new " + type_name + "())";

    // Most field accessors need to retrieve and test the field offset first,
    // this is the prefix code for that:
    auto isArrayType = IsArray(field.value.type);
    auto ref_offset_prefix =
        isArrayType
            ? " { return "
            : (" { var o = _model.__offset(" + NumToString(field.value.offset) +
            "); return ref (o != 0 ? ");
    auto value_offset_prefix =
        isArrayType
            ? " { return "
            : (" { var o = _model.__offset(" + NumToString(field.value.offset) +
            "); return (o != 0 ? ");
    // Generate the accessors that don't do object reuse.
    if (field.value.type.base_type == BASE_TYPE_STRUCT) {
    } else if (IsVector(field.value.type) &&
        field.value.type.element == BASE_TYPE_STRUCT) {
    } else if (field.value.type.base_type == BASE_TYPE_UNION ||
        (IsVector(field.value.type) &&
            field.value.type.VectorType().base_type == BASE_TYPE_UNION)) {
      method_start += "<TTable>";
      type_name = type_name_dest;
    }

    bool createdRefField = false;

    std::string reffer = GenReffer(field.value.type);

    if (!reffer.empty()
        && !is_optional
        && field.value.type.base_type != BASE_TYPE_STRUCT
        && field.value.type.base_type != BASE_TYPE_STRING
        && field.value.type.base_type != BASE_TYPE_UNION) {
      reffer = dest_cast + reffer;

      createdRefField = true;
      code += method_start;
      std::string default_cast = "";
      // only create default casts for c# scalars or vectors of scalars
      if ((IsScalar(field.value.type.base_type) ||
          (IsVector(field.value.type) &&
              IsScalar(field.value.type.element)))) {
        // For scalars, default value will be returned by GetDefaultValue().
        // If the scalar is an enum, GetDefaultValue() returns an actual c# enum
        // that doesn't need to be casted. However, default values for enum
        // elements of vectors are integer literals ("0") and are still casted
        // for clarity.
        // If the scalar is optional and enum, we still need the cast.
        if ((field.value.type.enum_def == nullptr ||
            IsVector(field.value.type)) ||
            (IsEnum(field.value.type) && field.IsScalarOptional())) {
          default_cast = "(" + type_name_dest + optional + ")";
        }
      }
      std::string member_suffix = "; ";
      if (IsScalar(field.value.type.base_type)) {
        code += " { get";
        member_suffix += "} ";
        if (struct_def.fixed) {
          code += " { return " + reffer;
          code += "(_model.Offset + ";
          code += NumToString(field.value.offset) + ")";
          code += dest_mask;
        } else {
          code += ref_offset_prefix + reffer;
          code += "(o + _model.Offset)" + dest_mask;
          code += " : " + GenNullRefThrower(field.value.type);
          if (!isArrayType) code += ")";
        }
      } else {
        switch (field.value.type.base_type) {
          case BASE_TYPE_STRUCT:
            code += " { get";
            member_suffix += "} ";
            if (struct_def.fixed) {
              code += " { return " + obj + ".__assign(" + "_model.";
              code += "Offset + " + NumToString(field.value.offset) + ", ";
              code += "_model.ByteBuffer)";
            } else {
              code += ref_offset_prefix + conditional_cast;
              code += obj + ".__assign(";
              code += field.value.type.struct_def->fixed
                  ? "o + _model.Offset"
                  : "_model.__indirect(o + _model.Offset)";
              code += ", _model.ByteBuffer) : " + GenNullRefThrower(field.value.type);
            }
            break;
          case BASE_TYPE_ARRAY: FLATBUFFERS_FALLTHROUGH();  // fall thru
          case BASE_TYPE_VECTOR: {
            auto vectortype = field.value.type.VectorType();
            if (vectortype.base_type == BASE_TYPE_UNION) {
              conditional_cast = "(TTable?)";
              reffer += "<TTable>";
            }
            code += "(";
            if (vectortype.base_type == BASE_TYPE_STRUCT) {
              reffer = obj + ".__assign";
            } else if (vectortype.base_type == BASE_TYPE_UNION) {
            }
            code += "ulong j)";
            const auto body =
                ref_offset_prefix + conditional_cast + reffer + "(";
            if (vectortype.base_type == BASE_TYPE_UNION) {
              code += " where TTable : struct, IBigBufferTable" + body;
            } else {
              code += body;
            }
            std::string index = "_model.";
            if (IsArray(field.value.type)) {
              index += "Offset + " + NumToString(field.value.offset) + " + ";
            } else {
              index += "__vector(o) + ";
            }
            index += "j * " + NumToString(InlineSize(vectortype));
            if (vectortype.base_type == BASE_TYPE_STRUCT) {
              code += vectortype.struct_def->fixed
                  ? index
                  : "_model.__indirect(" + index + ")";
              code += ", _model.ByteBuffer";
            } else {
              code += index;
            }
            code += ")" + dest_mask;
            if (!IsArray(field.value.type)) {
              code += " : ";
              code += GenNullRefThrower(field.value.type.VectorType());
              code += ")";
            }
            break;
          }
          case BASE_TYPE_STRING: FLATBUFFERS_ASSERT(0);
          case BASE_TYPE_UNION: FLATBUFFERS_ASSERT(0);
          default: FLATBUFFERS_ASSERT(0);
        }
      }
      code += member_suffix;
      code += "}\n";
    } else {
    }
    // Generate mutators for scalar fields or vectors of scalars.
    if (parser_.opts.mutable_buffer) {
      auto is_series = (IsSeries(field.value.type));
      const auto &underlying_type =
          is_series ? field.value.type.VectorType() : field.value.type;
      // A vector mutator also needs the index of the vector element it should
      // mutate.

      auto spanner_index =
          is_series
              ? "_model." +
              (IsArray(field.value.type)
                  ? "Offset + " + NumToString(field.value.offset)
                  : "__vector(o)")
              : (struct_def.fixed
              ? "_model.Offset + " + NumToString(field.value.offset)
              : "o + _model.Offset");
      auto spanner_params = "() { ";
      auto span_size = field.value.type.fixed_length;
      if (IsScalar(underlying_type.base_type) && !IsUnion(field.value.type) && span_size > 1) {
        createdRefField = true;

        code += "  public StirlingLabs.Utilities.BigSpan<"+GenTypeBasic(underlying_type)+"> ";
        code += MakeCamel(field.name, true);
        code += spanner_params;
        if (struct_def.fixed ) {
          code += GenSpanner(underlying_type) + "("+spanner_index+", "+NumToString(span_size)+"); }\n";
        } else {
          code += "var o = _model.__offset(";
          code += NumToString(field.value.offset) + ");";
          code += " if (o != 0) { " + GenSpanner(underlying_type);
          code += "("+spanner_index+", 1); } else { return default; } }\n";
        }
      }
    }
    if (parser_.opts.java_primitive_has_method &&
        IsScalar(field.value.type.base_type) && !struct_def.fixed) {
      auto vt_offset_constant = "  public static final ulong VT_" +
          MakeScreamingCamel(field.name) + " = " +
          NumToString(field.value.offset) + ";";

      code += vt_offset_constant;
      code += "\n";
    }
    return createdRefField;
  }

  void GenStruct(StructDef &struct_def, std::string *code_ptr,
                 const IDLOptions &opts) const {
    if (struct_def.generated) return;
    std::string &code = *code_ptr;

    // Generate a struct accessor class, with methods of the form:
    // public type name() { return bb.getType(i + offset); }
    // or for tables of the form:
    // public type name() {
    //   int o = __offset(offset); return o != 0 ? bb.getType(o + i) : default;
    // }
    GenComment(struct_def.doc_comment, code_ptr, &comment_config);
    if (struct_def.attributes.Lookup("private")) {
      code += "internal ";
    } else {
      code += "public ";
    }

    // generate a partial class for this C# struct/table
    code += "partial ";

    // model composition
    code += "struct " + struct_def.name;
    code += " : ";
    code += struct_def.fixed ? "IBigBufferStruct" : "IBigBufferTable";
    code += "\n{\n";
    code += "  internal ";
    code += struct_def.fixed ? "Struct" : "Table";
    code += " _model;\n";

    // interface implementation
    code += "  ";
    code += struct_def.fixed ? "Struct" : "Table";
    code += " ";
    code += struct_def.fixed ? "IBigBufferStruct" : "IBigBufferTable";
    code += ".Model => _model;\n";

    if (struct_def.fixed) {
      // static ByteSize
      code += "  public static ulong ByteSize => ";
      code += NumToString(struct_def.bytesize);
      code +=";\n";
      // interface implementation
      code += "  ulong ";
      code += "IBigBufferStruct";
      code += ".ByteSize => ByteSize;\n";
    }

    code += "  public ByteBuffer ByteBuffer { get { return _model.ByteBuffer; } }\n";

    if (!struct_def.fixed) {
      // Generate verson check method.
      // Force compile time error if not using the same version runtime.
      code += "  public static void ValidateVersion() {";
      code += " BigBuffers.Constants.";
      code += "VERSION_2_0_0(); ";
      code += "}\n";

      // Generate a special accessor for the table that when used as the root
      // of a FlatBuffer
      std::string method_name = "GetRootAs" + struct_def.name;
      std::string method_signature =
          "  public static " + struct_def.name + " " + method_name;

      // create convenience method that doesn't require an existing object
      code += method_signature + "(ByteBuffer _bb) ";
      code += "{ return " + method_name + "(_bb, new " + struct_def.name +
              "()); }\n";

      // create method that allows object reuse
      code +=
          method_signature + "(ByteBuffer _bb, " + struct_def.name + " obj) { ";
      code += "return (obj.__assign(_bb.Position, _bb)); }\n";
      if (parser_.root_struct_def_ == &struct_def) {
        if (parser_.file_identifier_.length()) {
          // Check if a buffer has the identifier.
          code += "  public static ";
          code += "bool " + struct_def.name;
          code += "BufferHasIdentifier(ByteBuffer _bb) { return ";
          code += "SchemaModel.__has_identifier(_bb, \"";
          code += parser_.file_identifier_;
          code += "\"); }\n";
        }
      }
    }
    // Generate the __init method that sets the field in a pre-existing
    // accessor object. This is to allow object reuse.
    code += "  public void __init(ulong _i, ByteBuffer _bb) ";
    code += "{ ";
    code += "_model = new ";
    code += struct_def.fixed ? "Struct" : "Table";
    code += "(_i, _bb); ";
    code += "}\n";
    code +=
        "  public " + struct_def.name + " __assign(ulong _i, ByteBuffer _bb) ";
    code += "{ __init(_i, _bb); return this; }\n\n";
    for (auto it = struct_def.fields.vec.begin();
         it != struct_def.fields.vec.end(); ++it) {


      auto &field = **it;
      if (field.deprecated) continue;
      GenComment(field.doc_comment, code_ptr, &comment_config, "  ");

      auto madeRefField = GenRefField(code, field, struct_def);
      if (!GenField(code, field, struct_def, madeRefField))
        continue;
    }
    code += "\n";
    auto struct_has_create = false;
    std::set<flatbuffers::FieldDef *> field_has_create_set;
    flatbuffers::FieldDef *key_field = nullptr;
    if (struct_def.fixed) {
      struct_has_create = true;
      // create a struct constructor function
      code += "  public static " + GenOffsetType(struct_def) + " ";
      code += "Create";
      code += struct_def.name + "(BigBufferBuilder builder";
      GenStructArgs(struct_def, code_ptr, "");
      code += ") {\n";
      code += "    var start = builder.Offset;\n";
      GenStructBody(struct_def, code_ptr, "");
      code += "    return ";
      code += GenOffsetConstruct(struct_def, "start");
      code += ";\n  }\n";
    } else {
      // Generate a method that creates a table in one go. This is only possible
      // when the table has no struct fields, since those have to be created
      // inline, and there's no way to do so in Java.
      bool has_no_struct_fields = true;
      int num_fields = 0;
      for (auto it = struct_def.fields.vec.begin();
           it != struct_def.fields.vec.end(); ++it) {
        auto &field = **it;
        if (field.deprecated) continue;
        if (IsStruct(field.value.type)) {
          has_no_struct_fields = false;
        } else {
          num_fields++;
        }
      }
      // JVM specifications restrict default constructor params to be < 255.
      // Longs and doubles take up 2 units, so we set the limit to be < 127.
      if ((has_no_struct_fields || opts.generate_object_based_api) &&
          num_fields && num_fields < 127) {
        struct_has_create = true;
        // Generate a table constructor of the form:
        // public static int createName(BigBufferBuilder builder, args...)
        code += "  public static " + GenOffsetType(struct_def) + " ";
        code += "Create" + struct_def.name;
        code += "(BigBufferBuilder builder";
        for (auto it = struct_def.fields.vec.begin();
             it != struct_def.fields.vec.end(); ++it) {
          auto &field = **it;
          if (field.deprecated) continue;
          code += ",\n      ";
          if (IsStruct(field.value.type) && opts.generate_object_based_api) {
            code += WrapInNameSpace(
                field.value.type.struct_def->defined_namespace,
                GenTypeName_ObjectAPI(field.value.type.struct_def->name, opts));
            code += " ";
            code += EscapeKeyword(field.name);
            code += " = null";
          } else {
            code += GenTypeBasic(field.value.type);
            if (field.IsScalarOptional()) { code += "?"; }
            code += " ";
            code += EscapeKeyword(field.name);
            if (!IsScalar(field.value.type.base_type)) code += "Offset";

            code += " = ";
            code += GenDefaultValueBasic(field);
          }
        }
        code += ") {\n    builder.";
        code += "StartTable(";
        code += NumToString(struct_def.fields.vec.size()) + ");\n";
        for (size_t size = struct_def.sortbysize ? sizeof(largest_scalar_t) : 1;
             size; size /= 2) {
          for (auto it = struct_def.fields.vec.rbegin();
               it != struct_def.fields.vec.rend(); ++it) {
            auto &field = **it;
            if (!field.deprecated &&
                (!struct_def.sortbysize ||
                 size == SizeOf(field.value.type.base_type))) {
              code += "    " + struct_def.name + ".";
              code += "Add";
              code += Name(field) + "(builder, ";
              if (IsStruct(field.value.type) &&
                  opts.generate_object_based_api) {
                code += GenTypePointer(field.value.type) + ".Pack(builder, " +
                        EscapeKeyword(field.name) + ")";
              } else {
                code += EscapeKeyword(field.name);
                if (!IsScalar(field.value.type.base_type)) code += "Offset";
              }

              code += ");\n";
            }
          }
        }
        code += "    return " + struct_def.name + ".";
        code += "End" + struct_def.name;
        code += "(builder);\n  }\n\n";
      }
      // Generate a set of static methods that allow table construction,
      // of the form:
      // public static void addName(FlatBufferBuilder builder, short name)
      // { builder.addShort(id, name, default); }
      // Unlike the Create function, these always work.
      code += "  public static void Start";
      code += struct_def.name;
      code += "(BigBufferBuilder builder) { builder.";
      code += "StartTable(";
      code += NumToString(struct_def.fields.vec.size()) + "); }\n";
      for (auto it = struct_def.fields.vec.begin();
           it != struct_def.fields.vec.end(); ++it) {
        auto &field = **it;
        if (field.deprecated) continue;
        if (field.key) key_field = &field;
        code += "  public static void Add";
        code += Name(field);
        code += "(BigBufferBuilder builder, ";
        code += GenTypeBasic(field.value.type);
        auto argname = MakeCamel(field.name, false);
        if (!IsScalar(field.value.type.base_type)) argname += "Offset";
        if (field.IsScalarOptional()) { code += "?"; }
        code += " " + EscapeKeyword(argname) + ") { builder.Add";
        code += GenMethod(field.value.type) + "(";
        if(IsStruct(field.value.type)) {
          code += NumToString(InlineSize(field.value.type));
          code += ", ";
        }
        code += NumToString(it - struct_def.fields.vec.begin()) + ", ";
        code += SourceCastBasic(field.value.type);
        code += EscapeKeyword(argname);
        if (!IsScalar(field.value.type.base_type) &&
            field.value.type.base_type != BASE_TYPE_UNION) {
          code += ".Value";
        }
        if (!field.IsScalarOptional()) {
          // When the scalar is optional, use the builder method that doesn't
          // supply a default value. Otherwise, we to continue to use the
          // default value method.
          code += ", ";
          code += GenDefaultValue(field, false);
        }
        code += "); }\n";
        if (IsVector(field.value.type)) {
          auto vector_type = field.value.type.VectorType();
          auto alignment = InlineAlignment(vector_type);
          auto elem_size = InlineSize(vector_type);
          if (!IsStruct(vector_type)) {
            field_has_create_set.insert(&field);
            code += "  public static VectorOffset ";
            code += "Create";
            code += Name(field);
            code += "Vector(BigBufferBuilder builder, out Placeholder placeholder) ";
            code += "{ return builder.CreateVector(out placeholder); }\n";

            code += "  public static void ";
            code += "Fill";
            code += MakeCamel(field.name);
            code += "Vector(Placeholder placeholder, ";
            code += GenTypeBasic(vector_type) + "[] data) ";
            code += "{ placeholder.Fill((StirlingLabs.Utilities.ReadOnlyBigSpan<";
            code += GenTypeBasic(vector_type) + ">)data, " + NumToString(alignment);
            code += "); }\n";

            code += "  public static void ";
            code += "Fill";
            code += Name(field);
            code += "Vector(Placeholder placeholder, StirlingLabs.Utilities.ReadOnlyBigSpan<";
            code += GenTypeBasic(vector_type) + "> data) ";
            code += "{ placeholder.Fill(data, ";
            code += NumToString(alignment);
            code += "); }\n";
          }
          // Generate a method to start a vector, data to be added manually
          // after.
          code += "  public static void Start";
          code += Name(field);
          code += "Vector(BigBufferBuilder builder, ulong numElems) ";
          code += "{ builder.StartVector(";
          code += NumToString(elem_size);
          code += ", numElems); }\n";
        }
      }
      code += "  public static " + GenOffsetType(struct_def) + " ";
      code += "End" + struct_def.name;
      code += "(BigBufferBuilder builder) {\n    var o = builder.";
      code += "EndTable();\n";
      for (auto it = struct_def.fields.vec.begin();
           it != struct_def.fields.vec.end(); ++it) {
        auto &field = **it;
        if (!field.deprecated && field.IsRequired()) {
          code += "    builder.Required(o, ";
          code += NumToString(field.value.offset);
          code += ");  // " + field.name + "\n";
        }
      }
      code += "    return " + GenOffsetConstruct(struct_def, "o") + ";\n  }\n";
      if (parser_.root_struct_def_ == &struct_def) {
        std::string size_prefix[] = { "", "SizePrefixed" };
        for (int i = 0; i < 2; ++i) {
          code += "  public static void ";
          code += "Finish" + size_prefix[i] + struct_def.name;
          code +=
              "Buffer(BigBufferBuilder builder, " + GenOffsetType(struct_def);
          code += " offset) {";
          code += " builder.Finish" + size_prefix[i] + "(offset";
          code += ".Value";

          if (parser_.file_identifier_.length())
            code += ", \"" + parser_.file_identifier_ + "\"";
          code += "); }\n";
        }
      }
    }
    // Only generate key compare function for table,
    // because `key_field` is not set for struct
    if (struct_def.has_key && !struct_def.fixed) {
      FLATBUFFERS_ASSERT(key_field);
      code += "\n  public static VectorOffset ";
      code += "CreateSortedVectorOf" + struct_def.name;
      code += "(BigBufferBuilder builder, ";
      code += "Offset<" + struct_def.name + ">";
      code += "[] offsets) {\n";
      code += "    Array.Sort(offsets, (Offset<" + struct_def.name +
              "> o1, Offset<" + struct_def.name + "> o2) => " +
              GenKeyGetter(key_field);
      code += ");\n";
      code += "    return builder.CreateVectorOfTables(offsets);\n  }\n";

      code += "\n  public static " + struct_def.name + "?";
      code += " __lookup_by_key(";
      code += "ulong vectorLocation, ";
      code += GenTypeGet(key_field->value.type);
      code += " key, ByteBuffer bb) {\n";
      if (IsString(key_field->value.type)) {
        code += "    byte[] byteKey = ";
        code += "System.Text.Encoding.UTF8.GetBytes(key);\n";
      }
      code += "    var span = ";
      code += "bb.Get<ulong>(vectorLocation - sizeof(ulong));\n";
      code += "    var start = 0uL;\n";
      code += "    while (span != 0uL) {\n";
      code += "      var middle = span >> 1;\n";
      code += GenLookupKeyGetter(key_field);
      code += "      if (comp > 0) {\n";
      code += "        span = middle;\n";
      code += "      } else if (comp < 0) {\n";
      code += "        middle++;\n";
      code += "        start += middle;\n";
      code += "        span -= middle;\n";
      code += "      } else {\n";
      code += "        return ";
      code += "new " + struct_def.name + "()";
      code += ".__assign(tableOffset, bb);\n";
      code += "      }\n    }\n";
      code += "    return null;\n";
      code += "  }\n";
    }

    if (opts.generate_object_based_api) {
      GenPackUnPack_ObjectAPI(struct_def, code_ptr, opts, struct_has_create,
                              field_has_create_set);
    }
    code += "}\n\n";

    if (opts.generate_object_based_api) {
      GenStruct_ObjectAPI(struct_def, code_ptr, opts);
    }
  }

  void GenVectorAccessObject(StructDef &struct_def,
                             std::string *code_ptr) const {
    auto &code = *code_ptr;
    // Generate a vector of structs accessor class.
    code += "\n";
    code += "  ";
    if (!struct_def.attributes.Lookup("private")) code += "public ";
    code += "static struct Vector : BaseVector\n{\n";

    // Generate the __assign method that sets the field in a pre-existing
    // accessor object. This is to allow object reuse.
    std::string method_indent = "    ";
    code += method_indent + "public Vector ";
    code += "__assign(ulong _vector, ulong _element_size, ByteBuffer _bb) { ";
    code += "__reset(_vector, _element_size, _bb); return this; }\n\n";

    auto type_name = struct_def.name;
    auto method_start = method_indent + "public " + type_name + " Get";
    // Generate the accessors that don't do object reuse.
    code += method_start + "(ulong j) { return Get";
    code += "(new " + type_name + "(), j); }\n";
    code += method_start + "(" + type_name + " obj, ulong j) { ";
    code += " return obj.__assign(";
    code += struct_def.fixed ? "_model.__element(j)"
                             : "_model.__indirect(_model.__element(j), bb)";
    code += ", _model.ByteBuffer); }\n";
    // See if we should generate a by-key accessor.
    if (!struct_def.fixed) {
      auto &fields = struct_def.fields.vec;
      for (auto kit = fields.begin(); kit != fields.end(); ++kit) {
        auto &key_field = **kit;
        if (key_field.key) {
          auto nullable_annotation =
              parser_.opts.gen_nullable ? "@Nullable " : "";
          code += method_indent + nullable_annotation;
          code += "public " + type_name + "? ";
          code += "GetByKey(";
          code += GenTypeGet(key_field.value.type) + " key) { ";
          code += " return __lookup_by_key(null, ";
          code += "_model.__vector(), key, ";
          code += "_model.ByteBuffer); ";
          code += "}\n";
          code += method_indent + nullable_annotation;
          code += "public " + type_name + "?" + " ";
          code += "GetByKey(";
          code += type_name + "? obj, ";
          code += GenTypeGet(key_field.value.type) + " key) { ";
          code += " return __lookup_by_key(obj, ";
          code += "_model.__vector(), key, ";
          code += "_model.ByteBuffer); ";
          code += "}\n";
          break;
        }
      }
    }
    code += "  }\n";
  }

  void GenEnum_ObjectAPI(EnumDef &enum_def, std::string *code_ptr,
                         const IDLOptions &opts) const {
    auto &code = *code_ptr;
    if (enum_def.generated) return;
    if (!enum_def.is_union) return;
    if (enum_def.attributes.Lookup("private")) {
      code += "internal ";
    } else {
      code += "public ";
    }
    auto union_name = enum_def.name + "Union";
    code += "class " + union_name + " {\n";
    // Type
    code += "  public " + enum_def.name + " Type { get; set; }\n";
    // Value
    code += "  public object Value { get; set; }\n";
    code += "\n";
    // Constructor
    code += "  public " + union_name + "() {\n";
    code += "    this.Type = " + enum_def.name + "." +
            enum_def.Vals()[0]->name + ";\n";
    code += "    this.Value = null;\n";
    code += "  }\n\n";
    // As<T>
    code += "  public T As<T>() where T : class { return this.Value as T; }\n";
    // As, From
    for (auto it = enum_def.Vals().begin(); it != enum_def.Vals().end(); ++it) {
      auto &ev = **it;
      if (ev.union_type.base_type == BASE_TYPE_NONE) continue;
      auto type_name = GenTypeGet_ObjectAPI(ev.union_type, opts);
      std::string accessibility =
          (ev.union_type.base_type == BASE_TYPE_STRUCT &&
           ev.union_type.struct_def->attributes.Lookup("private"))
              ? "internal"
              : "public";
      // As
      code += "  " + accessibility + " " + type_name + " As" + ev.name +
              "() { return this.As<" + type_name + ">(); }\n";
      // From
      auto lower_ev_name = ev.name;
      std::transform(lower_ev_name.begin(), lower_ev_name.end(),
                     lower_ev_name.begin(), CharToLower);
      code += "  " + accessibility + " static " + union_name + " From" +
              ev.name + "(" + type_name + " _" + lower_ev_name +
              ") { return new " + union_name + "{ Type = " + Name(enum_def) +
              "." + Name(ev) + ", Value = _" + lower_ev_name + " }; }\n";
      }
    code += "\n";
    // Pack()
    code += "  public static ulong Pack(BigBuffers.BigBufferBuilder builder, " +
            union_name + " _o) {\n";
    code += "    switch (_o.Type) {\n";
    for (auto it = enum_def.Vals().begin(); it != enum_def.Vals().end(); ++it) {
      auto &ev = **it;
      if (ev.union_type.base_type == BASE_TYPE_NONE) {
        code += "      default: return 0;\n";
      } else {
        code += "      case " + Name(enum_def) + "." + Name(ev) + ": return ";
        if (IsString(ev.union_type)) {
          code += "builder.CreateString(_o.As" + ev.name + "()).Value;\n";
        } else {
          code += GenTypeGet(ev.union_type) + ".Pack(builder, _o.As" + ev.name +
                  "()).Value;\n";
        }
      }
    }
    code += "    }\n";
    code += "  }\n";
    code += "}\n\n";
    // JsonConverter
    if (opts.cs_gen_json_serializer) {
      if (enum_def.attributes.Lookup("private")) {
        code += "internal ";
      } else {
        code += "public ";
      }
      code += "class " + union_name +
              "_JsonConverter : Newtonsoft.Json.JsonConverter {\n";
      code += "  public override bool CanConvert(System.Type objectType) {\n";
      code += "    return objectType == typeof(" + union_name +
              ") || objectType == typeof(System.Collections.Generic.List<" +
              union_name + ">);\n";
      code += "  }\n";
      code +=
          "  public override void WriteJson(Newtonsoft.Json.JsonWriter writer, "
          "object value, "
          "Newtonsoft.Json.JsonSerializer serializer) {\n";
      code += "    var _olist = value as System.Collections.Generic.List<" +
              union_name + ">;\n";
      code += "    if (_olist != null) {\n";
      code += "      writer.WriteStartArray();\n";
      code +=
          "      foreach (var _o in _olist) { this.WriteJson(writer, _o, "
          "serializer); }\n";
      code += "      writer.WriteEndArray();\n";
      code += "    } else {\n";
      code += "      this.WriteJson(writer, value as " + union_name +
              ", serializer);\n";
      code += "    }\n";
      code += "  }\n";
      code += "  public void WriteJson(Newtonsoft.Json.JsonWriter writer, " +
              union_name +
              " _o, "
              "Newtonsoft.Json.JsonSerializer serializer) {\n";
      code += "    if (_o == null) return;\n";
      code += "    serializer.Serialize(writer, _o.Value);\n";
      code += "  }\n";
      code +=
          "  public override object ReadJson(Newtonsoft.Json.JsonReader "
          "reader, "
          "System.Type objectType, "
          "object existingValue, Newtonsoft.Json.JsonSerializer serializer) "
          "{\n";
      code +=
          "    var _olist = existingValue as System.Collections.Generic.List<" +
          union_name + ">;\n";
      code += "    if (_olist != null) {\n";
      code += "      for (var _j = 0; _j < _olist.Count; ++_j) {\n";
      code += "        reader.Read();\n";
      code +=
          "        _olist[_j] = this.ReadJson(reader, _olist[_j], "
          "serializer);\n";
      code += "      }\n";
      code += "      reader.Read();\n";
      code += "      return _olist;\n";
      code += "    } else {\n";
      code += "      return this.ReadJson(reader, existingValue as " +
              union_name + ", serializer);\n";
      code += "    }\n";
      code += "  }\n";
      code += "  public " + union_name +
              " ReadJson(Newtonsoft.Json.JsonReader reader, " + union_name +
              " _o, Newtonsoft.Json.JsonSerializer serializer) {\n";
      code += "    if (_o == null) return null;\n";
      code += "    switch (_o.Type) {\n";
      for (auto it = enum_def.Vals().begin(); it != enum_def.Vals().end();
           ++it) {
        auto &ev = **it;
        if (ev.union_type.base_type == BASE_TYPE_NONE) {
          code += "      default: break;\n";
        } else {
          auto type_name = GenTypeGet_ObjectAPI(ev.union_type, opts);
          code += "      case " + Name(enum_def) + "." + Name(ev) +
                  ": _o.Value = serializer.Deserialize<" + type_name +
                  ">(reader); break;\n";
        }
      }
      code += "    }\n";
      code += "    return _o;\n";
      code += "  }\n";
      code += "}\n\n";
    }
  }

  std::string GenTypeName_ObjectAPI(const std::string &name,
                                    const IDLOptions &opts) const {
    return opts.object_prefix + name + opts.object_suffix;
  }

  void GenUnionUnPack_ObjectAPI(const EnumDef &enum_def, std::string *code_ptr,
                                const std::string &camel_name,
                                bool is_vector) const {
    auto &code = *code_ptr;
    std::string varialbe_name = "_o." + camel_name;
    std::string type_suffix = "";
    std::string func_suffix = "()";
    std::string indent = "    ";
    if (is_vector) {
      varialbe_name = "_o_" + camel_name;
      type_suffix = "(_j)";
      func_suffix = "(_j)";
      indent = "      ";
    }
    if (is_vector) {
      code += indent + "var " + varialbe_name + " = new ";
    } else {
      code += indent + varialbe_name + " = new ";
    }
    code += NamespacedName(enum_def) + "Union();\n";
    code += indent + varialbe_name + ".Type = this." + camel_name + "Type" +
            type_suffix + ";\n";
    code +=
        indent + "switch (this." + camel_name + "Type" + type_suffix + ") {\n";
    for (auto eit = enum_def.Vals().begin(); eit != enum_def.Vals().end();
         ++eit) {
      auto &ev = **eit;
      if (ev.union_type.base_type == BASE_TYPE_NONE) {
        code += indent + "  default: break;\n";
      } else {
        code += indent + "  case " + NamespacedName(enum_def) + "." + ev.name +
                ":\n";
        code += indent + "    " + varialbe_name + ".Value = this." + camel_name;
        if (IsString(ev.union_type)) {
          code += "AsString" + func_suffix + ";\n";
        } else {
          code += "<" + GenTypeGet(ev.union_type) + ">" + func_suffix;
          code += ".HasValue ? this." + camel_name;
          code += "<" + GenTypeGet(ev.union_type) + ">" + func_suffix +
                  ".Value.UnPack() : null;\n";
        }
        code += indent + "    break;\n";
      }
    }
    code += indent + "}\n";
    if (is_vector) {
      code += indent + "_o." + camel_name + ".Add(" + varialbe_name + ");\n";
    }
  }

  void GenPackUnPack_ObjectAPI(
      StructDef &struct_def, std::string *code_ptr, const IDLOptions &opts,
      bool struct_has_create,
      const std::set<FieldDef *> &field_has_create) const {
    auto &code = *code_ptr;
    auto struct_name = GenTypeName_ObjectAPI(struct_def.name, opts);
    // UnPack()
    code += "  public " + struct_name + " UnPack() {\n";
    code += "    var _o = new " + struct_name + "();\n";
    code += "    this.UnPackTo(_o);\n";
    code += "    return _o;\n";
    code += "  }\n";
    // UnPackTo()
    code += "  public void UnPackTo(" + struct_name + " _o) {\n";
    for (auto it = struct_def.fields.vec.begin();
         it != struct_def.fields.vec.end(); ++it) {
      auto &field = **it;
      if (field.deprecated) continue;
      auto camel_name = Name(field);
      auto start = "    _o." + camel_name + " = ";
      switch (field.value.type.base_type) {
        case BASE_TYPE_STRUCT: {
          auto fixed = struct_def.fixed && field.value.type.struct_def->fixed;
          if (fixed) {
            code += start + "this." + camel_name + ".UnPack();\n";
          } else {
            code += start + "this." + camel_name + ".HasValue ? this." +
                    camel_name + ".Value.UnPack() : null;\n";
          }
          break;
        }
        case BASE_TYPE_ARRAY: {
          auto type_name = GenTypeGet_ObjectAPI(field.value.type, opts);
          auto length_str = NumToString(field.value.type.fixed_length);
          auto unpack_method = field.value.type.struct_def == nullptr
                                   ? ""
                                   : field.value.type.struct_def->fixed
                                         ? ".UnPack()"
                                         : "?.UnPack()";
          code += start + "new " + type_name.substr(0, type_name.length() - 1) +
                  length_str + "];\n";
          code += "    for (var _j = 0; _j < " + length_str + "; ++_j) { _o." +
                  camel_name + "[_j] = this." + camel_name + "(_j)" +
                  unpack_method + "; }\n";
          break;
        }
        case BASE_TYPE_VECTOR:
          if (field.value.type.element == BASE_TYPE_UNION) {
            code += start + "new " +
                    GenTypeGet_ObjectAPI(field.value.type, opts) + "();\n";
            code += "    for (var _j = 0; _j < this." + camel_name +
                    "Length; ++_j) {\n";
            GenUnionUnPack_ObjectAPI(*field.value.type.enum_def, code_ptr,
                                     camel_name, true);
            code += "    }\n";
          } else if (field.value.type.element != BASE_TYPE_UTYPE) {
            auto fixed = field.value.type.struct_def == nullptr;
            code += start + "new " +
                    GenTypeGet_ObjectAPI(field.value.type, opts) + "();\n";
            code += "    for (var _j = 0; _j < this." + camel_name +
                    "Length; ++_j) {";
            code += "_o." + camel_name + ".Add(";
            if (fixed) {
              code += "this." + camel_name + "(_j)";
            } else {
              code += "this." + camel_name + "(_j).HasValue ? this." +
                      camel_name + "(_j).Value.UnPack() : null";
            }
            code += ");}\n";
          }
          break;
        case BASE_TYPE_UTYPE: break;
        case BASE_TYPE_UNION: {
          GenUnionUnPack_ObjectAPI(*field.value.type.enum_def, code_ptr,
                                   camel_name, false);
          break;
        }
        default: {
          code += start + "this." + camel_name + ";\n";
          break;
        }
      }
    }
    code += "  }\n";
    // Pack()
    code += "  public static " + GenOffsetType(struct_def) +
            " Pack(BigBufferBuilder builder, " + struct_name + " _o) {\n";
    code += "    if (_o == null) return default(" + GenOffsetType(struct_def) +
            ");\n";
    for (auto it = struct_def.fields.vec.begin();
         it != struct_def.fields.vec.end(); ++it) {
      auto &field = **it;
      if (field.deprecated) continue;
      auto camel_name = Name(field);
      // pre
      switch (field.value.type.base_type) {
        case BASE_TYPE_STRUCT: {
          if (!field.value.type.struct_def->fixed) {
            code += "    var _" + field.name + " = _o." + camel_name +
                    " == null ? default(" +
                    GenOffsetType(*field.value.type.struct_def) +
                    ") : " + GenTypeGet(field.value.type) +
                    ".Pack(builder, _o." + camel_name + ");\n";
          } else if (struct_def.fixed && struct_has_create) {
            std::vector<FieldArrayLength> array_lengths;
            FieldArrayLength tmp_array_length = {
              field.name,
              field.value.type.fixed_length,
            };
            array_lengths.push_back(tmp_array_length);
            GenStructPackDecl_ObjectAPI(*field.value.type.struct_def, code_ptr,
                                        array_lengths);
          }
          break;
        }
        case BASE_TYPE_STRING: {
          std::string create_string =
              field.shared ? "CreateSharedString" : "CreateString";
          code += "    var _" + field.name + " = _o." + camel_name +
                  " == null ? default(StringOffset) : "
                  "builder." +
                  create_string + "(_o." + camel_name + ");\n";
          break;
        }
        case BASE_TYPE_VECTOR: {
          if (field_has_create.find(&field) != field_has_create.end()) {
            auto property_name = camel_name;
            auto gen_for_loop = true;
            std::string array_name = "__" + field.name;
            std::string array_type = "";
            std::string to_array = "";
            switch (field.value.type.element) {
              case BASE_TYPE_STRING: {
                std::string create_string =
                    field.shared ? "CreateSharedString" : "CreateString";
                array_type = "StringOffset";
                to_array += "builder." + create_string + "(_o." +
                            property_name + "[_j])";
                break;
              }
              case BASE_TYPE_STRUCT:
                array_type = "Offset<" + GenTypeGet(field.value.type) + ">";
                to_array = GenTypeGet(field.value.type) + ".Pack(builder, _o." +
                           property_name + "[_j])";
                break;
              case BASE_TYPE_UTYPE:
                property_name = camel_name.substr(0, camel_name.size() - sizeof(uoffset_t));
                array_type = NamespacedName(*field.value.type.enum_def);
                to_array = "_o." + property_name + "[_j].Type";
                break;
              case BASE_TYPE_UNION:
                array_type = "ulong";
                to_array = NamespacedName(*field.value.type.enum_def) +
                           "Union.Pack(builder,  _o." + property_name + "[_j])";
                break;
              default: gen_for_loop = false; break;
            }
            code += "    var _" + field.name + " = default(VectorOffset);\n";
            code += "    if (_o." + property_name + " != null) {\n";
            if (gen_for_loop) {
              code += "      var " + array_name + " = new " + array_type + "[_o." + property_name + ".Count];\n";
              code += "      for (var _j = 0; _j < " + array_name + ".Length; ++_j) { ";
              code += array_name + "[_j] = " + to_array + "; }\n";
            } else {
              code += "      var " + array_name + " = _o." + property_name + ".ToArray();\n";
            }
            code += "      _" + field.name + " = Create" + camel_name + "Vector(builder, " + array_name + ");\n";
            code += "    }\n";
          } else {
            auto vector_type = field.value.type.VectorType();
            auto alignment = InlineAlignment(vector_type);
            auto pack_method =
                field.value.type.struct_def == nullptr
                    ? "builder.Add" + GenMethod(field.value.type.VectorType()) +
                          "(_o." + camel_name + "[_j]);"
                    : GenTypeGet(field.value.type) + ".Pack(builder, _o." +
                          camel_name + "[_j]);";
            code += "    var _" + field.name + " = default(VectorOffset);\n";
            code += "    if (_o." + camel_name + " != null) {\n";
            code += "      Start" + camel_name + "Vector(builder, _o." + camel_name + ".Count);\n";
            code += "      for (var _j = _o." + camel_name + ".Count - 1; _j >= 0; --_j) { " + pack_method + " }\n";
            code += "      _" + field.name + " = builder.EndVector("+NumToString(alignment)+");\n";
            code += "    }\n";
          }
          break;
        }
        case BASE_TYPE_ARRAY: {
          if (field.value.type.struct_def != nullptr) {
            std::vector<FieldArrayLength> array_lengths;
            FieldArrayLength tmp_array_length = {
              field.name,
              field.value.type.fixed_length,
            };
            array_lengths.push_back(tmp_array_length);
            GenStructPackDecl_ObjectAPI(*field.value.type.struct_def, code_ptr,
                                        array_lengths);
          } else {
            code += "    var _" + field.name + " = _o." + camel_name + ";\n";
          }
          break;
        }
        case BASE_TYPE_UNION: {
          code += "    var _" + field.name + "_type = _o." + camel_name +
                  " == null ? " + NamespacedName(*field.value.type.enum_def) +
                  ".NONE : " + "_o." + camel_name + ".Type;\n";
          code +=
              "    var _" + field.name + " = _o." + camel_name +
              " == null ? 0 : " + GenTypeGet_ObjectAPI(field.value.type, opts) +
              ".Pack(builder, _o." + camel_name + ");\n";
          break;
        }
        default: break;
      }
    }
    if (struct_has_create) {
      // Create
      code += "    return Create" + struct_def.name + "(\n";
      code += "      builder";
      for (auto it = struct_def.fields.vec.begin();
           it != struct_def.fields.vec.end(); ++it) {
        auto &field = **it;
        if (field.deprecated) continue;
        auto camel_name = Name(field);
        switch (field.value.type.base_type) {
          case BASE_TYPE_STRUCT: {
            if (struct_def.fixed) {
              GenStructPackCall_ObjectAPI(*field.value.type.struct_def,
                                          code_ptr,
                                          "      _" + field.name + "_");
            } else {
              code += ",\n";
              if (field.value.type.struct_def->fixed) {
                if (opts.generate_object_based_api)
                  code += "      _o." + camel_name;
                else
                  code += "      " + GenTypeGet(field.value.type) +
                          ".Pack(builder, _o." + camel_name + ")";
              } else {
                code += "      _" + field.name;
              }
            }
            break;
          }
          case BASE_TYPE_ARRAY: {
            if (field.value.type.struct_def != nullptr) {
              GenStructPackCall_ObjectAPI(*field.value.type.struct_def,
                                          code_ptr,
                                          "      _" + field.name + "_");
            } else {
              code += ",\n";
              code += "      _" + field.name;
            }
            break;
          }
          case BASE_TYPE_UNION: FLATBUFFERS_FALLTHROUGH();   // fall thru
          case BASE_TYPE_UTYPE: FLATBUFFERS_FALLTHROUGH();   // fall thru
          case BASE_TYPE_STRING: FLATBUFFERS_FALLTHROUGH();  // fall thru
          case BASE_TYPE_VECTOR: {
            code += ",\n";
            code += "      _" + field.name;
            break;
          }
          default:  // scalar
            code += ",\n";
            code += "      _o." + camel_name;
            break;
        }
      }
      code += ");\n";
    } else {
      // Start, End
      code += "    Start" + struct_def.name + "(builder);\n";
      for (auto it = struct_def.fields.vec.begin();
           it != struct_def.fields.vec.end(); ++it) {
        auto &field = **it;
        if (field.deprecated) continue;
        auto camel_name = Name(field);
        switch (field.value.type.base_type) {
          case BASE_TYPE_STRUCT: {
            if (field.value.type.struct_def->fixed) {
              code += "    Add" + camel_name + "(builder, " +
                      GenTypeGet(field.value.type) + ".Pack(builder, _o." +
                      camel_name + "));\n";
            } else {
              code +=
                  "    Add" + camel_name + "(builder, _" + field.name + ");\n";
            }
            break;
          }
          case BASE_TYPE_STRING: FLATBUFFERS_FALLTHROUGH();  // fall thru
          case BASE_TYPE_ARRAY: FLATBUFFERS_FALLTHROUGH();   // fall thru
          case BASE_TYPE_VECTOR: {
            code +=
                "    Add" + camel_name + "(builder, _" + field.name + ");\n";
            break;
          }
          case BASE_TYPE_UTYPE: break;
          case BASE_TYPE_UNION: {
            code += "    Add" + camel_name + "Type(builder, _" + field.name +
                    "_type);\n";
            code +=
                "    Add" + camel_name + "(builder, _" + field.name + ");\n";
            break;
          }
          // scalar
          default: {
            code +=
                "    Add" + camel_name + "(builder, _o." + camel_name + ");\n";
            break;
          }
        }
      }
      code += "    return End" + struct_def.name + "(builder);\n";
    }
    code += "  }\n";
  }

  void GenStructPackDecl_ObjectAPI(
      const StructDef &struct_def, std::string *code_ptr,
      std::vector<FieldArrayLength> &array_lengths) const {
    auto &code = *code_ptr;
    for (auto it = struct_def.fields.vec.begin();
         it != struct_def.fields.vec.end(); ++it) {
      auto &field = **it;
      auto is_array = IsArray(field.value.type);
      const auto &field_type =
          is_array ? field.value.type.VectorType() : field.value.type;
      FieldArrayLength tmp_array_length = {
        field.name,
        field_type.fixed_length,
      };
      array_lengths.push_back(tmp_array_length);
      if (field_type.struct_def != nullptr) {
        GenStructPackDecl_ObjectAPI(*field_type.struct_def, code_ptr,
                                    array_lengths);
      } else {
        std::vector<FieldArrayLength> array_only_lengths;
        for (size_t i = 0; i < array_lengths.size(); ++i) {
          if (array_lengths[i].length > 0) {
            array_only_lengths.push_back(array_lengths[i]);
          }
        }
        std::string name;
        for (size_t i = 0; i < array_lengths.size(); ++i) {
          name += "_" + array_lengths[i].name;
        }
        code += "    var " + name + " = ";
        if (array_only_lengths.size() > 0) {
          code += "new " + GenTypeBasic(field_type) + "[";
          for (size_t i = 0; i < array_only_lengths.size(); ++i) {
            if (i != 0) { code += ","; }
            code += NumToString(array_only_lengths[i].length);
          }
          code += "];\n";
          code += "    ";
          // initialize array
          for (size_t i = 0; i < array_only_lengths.size(); ++i) {
            auto idx = "idx" + NumToString(i);
            code += "for (var " + idx + " = 0; " + idx + " < " +
                    NumToString(array_only_lengths[i].length) + "; ++" + idx +
                    ") {";
          }
          for (size_t i = 0; i < array_only_lengths.size(); ++i) {
            auto idx = "idx" + NumToString(i);
            if (i == 0) {
              code += name + "[" + idx;
            } else {
              code += "," + idx;
            }
          }
          code += "] = _o";
          for (size_t i = 0, j = 0; i < array_lengths.size(); ++i) {
            code += "." + MakeCamel(array_lengths[i].name, true);
            if (array_lengths[i].length <= 0) continue;
            code += "[idx" + NumToString(j++) + "]";
          }
          code += ";";
          for (size_t i = 0; i < array_only_lengths.size(); ++i) {
            code += "}";
          }
        } else {
          code += "_o";
          for (size_t i = 0; i < array_lengths.size(); ++i) {
            code += "." + MakeCamel(array_lengths[i].name, true);
          }
          code += ";";
        }
        code += "\n";
      }
      array_lengths.pop_back();
    }
  }

  void GenStructPackCall_ObjectAPI(const StructDef &struct_def,
                                   std::string *code_ptr,
                                   std::string prefix) const {
    auto &code = *code_ptr;
    for (auto it = struct_def.fields.vec.begin();
         it != struct_def.fields.vec.end(); ++it) {
      auto &field = **it;
      const auto &field_type = field.value.type;
      if (field_type.struct_def != nullptr) {
        GenStructPackCall_ObjectAPI(*field_type.struct_def, code_ptr,
                                    prefix + field.name + "_");
      } else {
        code += ",\n";
        code += prefix + field.name;
      }
    }
  }

  std::string GenTypeGet_ObjectAPI(flatbuffers::Type type,
                                   const IDLOptions &opts) const {
    auto type_name = GenTypeGet(type);
    // Replace to ObjectBaseAPI Type Name
    switch (type.base_type) {
      case BASE_TYPE_STRUCT: FLATBUFFERS_FALLTHROUGH();  // fall thru
      case BASE_TYPE_ARRAY: FLATBUFFERS_FALLTHROUGH();   // fall thru
      case BASE_TYPE_VECTOR: {
        if (type.struct_def != nullptr) {
          auto type_name_length = type.struct_def->name.length();
          auto new_type_name =
              GenTypeName_ObjectAPI(type.struct_def->name, opts);
          type_name.replace(type_name.length() - type_name_length,
                            type_name_length, new_type_name);
        } else if (type.element == BASE_TYPE_UNION) {
          type_name = NamespacedName(*type.enum_def) + "Union";
        }
        break;
      }

      case BASE_TYPE_UNION: {
        type_name = NamespacedName(*type.enum_def) + "Union";
        break;
      }
      default: break;
    }

    switch (type.base_type) {
      case BASE_TYPE_ARRAY: {
        type_name = type_name + "[]";
        break;
      }
      case BASE_TYPE_VECTOR: {
        type_name = "List<" + type_name + ">";
        break;
      }
      default: break;
    }
    return type_name;
  }

  void GenStruct_ObjectAPI(StructDef &struct_def, std::string *code_ptr,
                           const IDLOptions &opts) const {
    auto &code = *code_ptr;
    if (struct_def.attributes.Lookup("private")) {
      code += "internal ";
    } else {
      code += "public ";
    }
    if (struct_def.attributes.Lookup("csharp_partial")) {
      // generate a partial class for this C# struct/table
      code += "partial ";
    }
    auto class_name = GenTypeName_ObjectAPI(struct_def.name, opts);
    code += "class " + class_name;
    code += "\n{\n";
    // Generate Properties
    for (auto it = struct_def.fields.vec.begin();
         it != struct_def.fields.vec.end(); ++it) {
      auto &field = **it;
      if (field.deprecated) continue;
      if (field.value.type.base_type == BASE_TYPE_UTYPE) continue;
      if (field.value.type.element == BASE_TYPE_UTYPE) continue;
      auto type_name = GenTypeGet_ObjectAPI(field.value.type, opts);
      if (field.IsScalarOptional()) type_name += "?";
      auto camel_name = Name(field);
      if (opts.cs_gen_json_serializer) {
        if (IsUnion(field.value.type)) {
          auto utype_name = NamespacedName(*field.value.type.enum_def);
          code +=
              "  [Newtonsoft.Json.JsonProperty(\"" + field.name + "_type\")]\n";
          if (IsVector(field.value.type)) {
            code += "  internal " + utype_name + "[] " + camel_name + "Type {\n";
            code += "    get {\n";
            code += "      if (this." + camel_name + " == null) return null;\n";
            code += "      var _o = new " + utype_name + "[this." + camel_name +
                    ".Count];\n";
            code +=
                "      for (var _j = 0; _j < _o.Length; ++_j) { _o[_j] = "
                "this." +
                camel_name + "[_j].Type; }\n";
            code += "      return _o;\n";
            code += "    }\n";
            code += "    set {\n";
            code += "      this." + camel_name + " = new List<" + utype_name +
                    "Union>();\n";
            code += "      for (var _j = 0; _j < value.Length; ++_j) {\n";
            code += "        var _o = new " + utype_name + "Union();\n";
            code += "        _o.Type = value[_j];\n";
            code += "        this." + camel_name + ".Add(_o);\n";
            code += "      }\n";
            code += "    }\n";
            code += "  }\n";
          } else {
            code += "  internal " + utype_name + " " + camel_name + "Type {\n";
            code += "    get {\n";
            code += "      return this." + camel_name + " != null ? this." +
                    camel_name + ".Type : " + utype_name + ".NONE;\n";
            code += "    }\n";
            code += "    set {\n";
            code += "      this." + camel_name + " = new " + utype_name +
                    "Union();\n";
            code += "      this." + camel_name + ".Type = value;\n";
            code += "    }\n";
            code += "  }\n";
          }
        }
        code += "  [Newtonsoft.Json.JsonProperty(\"" + field.name + "\")]\n";
        if (IsUnion(field.value.type)) {
          auto union_name =
              (IsVector(field.value.type))
                  ? GenTypeGet_ObjectAPI(field.value.type.VectorType(), opts)
                  : type_name;
          code += "  [Newtonsoft.Json.JsonConverter(typeof(" + union_name +
                  "_JsonConverter))]\n";
        }
        if (field.attributes.Lookup("hash")) {
          code += "  [Newtonsoft.Json.JsonIgnore()]\n";
        }
      }
      code += "  public " + type_name + " " + camel_name + " { get; set; }\n";
    }
    // Generate Constructor
    code += "\n";
    code += "  public " + class_name + "() {\n";
    for (auto it = struct_def.fields.vec.begin();
         it != struct_def.fields.vec.end(); ++it) {
      auto &field = **it;
      if (field.deprecated) continue;
      if (field.value.type.base_type == BASE_TYPE_UTYPE) continue;
      if (field.value.type.element == BASE_TYPE_UTYPE) continue;
      code += "    this." + Name(field) + " = ";
      auto type_name = GenTypeGet_ObjectAPI(field.value.type, opts);
      if (IsScalar(field.value.type.base_type)) {
        code += GenDefaultValue(field) + ";\n";
      } else {
        switch (field.value.type.base_type) {
          case BASE_TYPE_STRUCT: {
            if (IsStruct(field.value.type)) {
              code += "new " + type_name + "();\n";
            } else {
              code += "null;\n";
            }
            break;
          }
          case BASE_TYPE_ARRAY: {
            code += "new " + type_name.substr(0, type_name.length() - 1) +
                    NumToString(field.value.type.fixed_length) + "];\n";
            break;
          }
          default: {
            code += "null;\n";
            break;
          }
        }
      }
    }
    code += "  }\n";
    // Generate Serialization
    if (opts.cs_gen_json_serializer &&
        parser_.root_struct_def_ == &struct_def) {
      code += "\n";
      code += "  public static " + class_name +
              " DeserializeFromJson(string jsonText) {\n";
      code += "    return Newtonsoft.Json.JsonConvert.DeserializeObject<" +
              class_name + ">(jsonText);\n";
      code += "  }\n";
      code += "  public string SerializeToJson() {\n";
      code +=
          "    return Newtonsoft.Json.JsonConvert.SerializeObject(this, "
          "Newtonsoft.Json.Formatting.Indented);\n";
      code += "  }\n";
    }
    if (parser_.root_struct_def_ == &struct_def) {
      code += "  public static " + class_name +
              " DeserializeFromBinary(byte[] fbBuffer) {\n";
      code += "    return " + struct_def.name + ".GetRootAs" + struct_def.name +
              "(new ByteBuffer(fbBuffer)).UnPack();\n";
      code += "  }\n";
      code += "  public byte[] SerializeToBinary() {\n";
      code += "    var fbb = new BigBufferBuilder(0x10000);\n";
      code += "    " + struct_def.name + ".Finish" + struct_def.name +
              "Buffer(fbb, " + struct_def.name + ".Pack(fbb, this));\n";
      code += "    return fbb.ByteBuffer.ToSizedArray();\n";
      code += "  }\n";
    }
    code += "}\n\n";
  }

  // This tracks the current namespace used to determine if a type need to be
  // prefixed by its namespace
  const Namespace *cur_name_space_;
};
}  // namespace csharp

bool GenerateCSharp(const Parser &parser, const std::string &path,
                    const std::string &file_name) {
  csharp::CSharpGenerator generator(parser, path, file_name);
  return generator.generate();
}

}  // namespace flatbuffers

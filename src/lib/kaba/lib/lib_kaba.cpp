#include "../kaba.h"
#include "lib.h"
#include "list.h"
#include "shared.h"
#include "../dynamic/exception.h"
#include "../dynamic/dynamic.h"


namespace kaba {

string function_link_name(Function *f);



KABA_LINK_GROUP_BEGIN

class KabaContext : public Context {
public:
	shared<Module> __load_module__(const string &filename, bool just_analyse) {
		KABA_EXCEPTION_WRAPPER( return load_module(filename, just_analyse); );
		return nullptr;
	}

	shared<Module> __create_from_source__(const string &source, bool just_analyse) {
		KABA_EXCEPTION_WRAPPER( return create_module_for_source(source, just_analyse); );
		return nullptr;
	}

	void __execute_single_command__(const string &cmd) {
		KABA_EXCEPTION_WRAPPER( execute_single_command(cmd); );
	}
};

KABA_LINK_GROUP_END


void show_func(Function *f) {
	bool v = config.verbose;
	config.verbose = true;
	f->show("");
	config.verbose = v;
}

class ClassX : public Class {
public:
	string repr() const {
		return class_repr(this);
	}
};

class FunctionX : public Function {
public:
	string repr() const {
		return func_repr(this);
	}
};

template<class T>
class XSharedArray : public shared_array<T> {
public:
	void __init__() {
		new(this) shared_array<T>;
	}
};

extern Array<shared<Module>> loading_module_stack;
Package* get_package_containing_module(Module* m);

Module* get_current_module() {
	return loading_module_stack.back().get();
}

Package* get_current_package() {
	auto m = get_current_module();
	return get_package_containing_module(m);
}

void SIAddPackageKaba(Context *c) {
	add_internal_package(c, "kaba", "1");


	common_types._class = add_type  ("Class", sizeof(Class));
	auto TypeClassP = add_type_p_raw(common_types._class);
	common_types.class_ref = add_type_ref(common_types._class);
	auto TypeClassPList = add_type_list(TypeClassP);
	auto TypeClassRefList = add_type_list(common_types.class_ref);
	lib_create_list<Class*>(TypeClassPList);
	lib_create_list<Class*>(TypeClassRefList);

	common_types.function = add_type  ("Function", sizeof(Function));
	auto TypeFunctionP = add_type_p_raw(common_types.function);
	common_types.function_ref = add_type_ref(common_types.function);
	auto TypeFunctionRefList = add_type_list(common_types.function_ref);
	common_types.function_code = add_type  ("code", 32); // whatever
	common_types.function_code_ref = add_type_ref(common_types.function_code);
	common_types.special_function = add_type  ("SpecialFunction", sizeof(SpecialFunction));
	//TypeSpecialFunctionP = add_type_p(TypeSpecialFunction);
	common_types.special_function_ref = add_type_ref(common_types.special_function);
	auto TypeSpecialFunctionRefList = add_type_list(common_types.special_function_ref);
	auto TypeStatement = add_type  ("Statement", sizeof(Statement));
	auto TypeStatementRef = add_type_ref(TypeStatement);
	auto TypeStatementRefList = add_type_list(TypeStatementRef);
	lib_create_list<Function*>(TypeFunctionRefList);
	lib_create_list<SpecialFunction*>(TypeSpecialFunctionRefList);
	lib_create_list<Statement*>(TypeStatementRefList);
		

	auto TypeModule = add_type  ("Module", sizeof(Module));
	auto TypeModuleXfer = add_type_p_xfer(TypeModule);
	auto TypeModuleShared = add_type_p_shared(TypeModule);
	auto TypeModuleSharedList = add_type_list(TypeModuleShared);
	auto TypeModuleRef = add_type_ref(TypeModule);
	auto TypeModuleRefList = add_type_list(TypeModuleRef);
	lib_create_list<shared<Module>>(TypeModuleSharedList);
	lib_create_list<Module*>(TypeModuleRefList);

	auto TypePackage = add_type  ("Package", sizeof(Package));
	auto TypePackageP = add_type_p_raw(TypePackage);
	auto TypePackageRef = add_type_ref(TypePackage);
	auto TypePackageRefList = add_type_list(TypePackageRef);
	lib_create_list<Package*>(TypePackageRefList);

	auto TypeContext = add_type  ("Context", sizeof(Context));
	auto TypeContextRef = add_type_ref(TypeContext);
	auto TypeContextXfer = add_type_p_xfer(TypeContext);
	
	auto TypeClassElement = add_type("ClassElement", sizeof(ClassElement));
	auto TypeClassElementList = add_type_list(TypeClassElement);
	auto TypeVariable = add_type("Variable", sizeof(Variable));
	auto TypeVariableRef = add_type_ref(TypeVariable);
	auto TypeVariableRefList = add_type_list(TypeVariableRef);
	auto TypeConstant = add_type("Constant", sizeof(Constant));
	auto TypeConstantRef = add_type_ref(TypeConstant);
	auto TypeConstantRefList = add_type_list(TypeConstantRef);
	lib_create_list<ClassElement>(TypeClassElementList);
	lib_create_list<Variable*>(TypeVariableRefList);
	lib_create_list<Constant*>(TypeConstantRefList);
	
	lib_create_pointer_xfer(TypeContextXfer);
	lib_create_pointer_shared<Module>(TypeModuleShared, TypeModuleXfer);
	
	add_class(TypeClassElement);
		class_add_element("name", common_types.string, &ClassElement::name);
		class_add_element("type", common_types.class_ref, &ClassElement::type);
		class_add_element("offset", common_types.i32, &ClassElement::offset);


	add_class(common_types._class);
		class_add_element("name", common_types.string, &Class::name);
		class_add_element("size", common_types.i32, &Class::size);
		class_add_element("from_template", TypeClassP, &Class::from_template);
		class_add_element("parent", TypeClassP, &Class::parent);
		class_add_element("param", TypeClassRefList, &Class::param);
		class_add_element("namespace", TypeClassP, &Class::name_space);
		class_add_element("elements", TypeClassElementList, &Class::elements);
		class_add_element("functions", TypeFunctionRefList, &Class::functions);
		class_add_element("classes", TypeClassRefList, &Class::classes);
		class_add_element("constants", TypeConstantRefList, &Class::constants);
		class_add_element("static_variables", TypeVariableRefList, &Class::static_variables);
		class_add_element(Identifier::SharedCount, common_types.i32, &Class::_pointer_ref_counter);
		class_add_func("is_derived_from", common_types._bool, &Class::is_derived_from, Flags::Pure);
			func_add_param("c", common_types.class_ref);
		class_add_func("is_pointer_raw", common_types._bool, &Class::is_pointer_raw, Flags::Pure);
		class_add_func("is_some_pointer", common_types._bool, &Class::is_some_pointer, Flags::Pure);
		class_add_func("is_some_pointer_not_null", common_types._bool, &Class::is_some_pointer_not_null, Flags::Pure);
		class_add_func("is_list", common_types._bool, &Class::is_list, Flags::Pure);
		class_add_func("is_array", common_types._bool, &Class::is_array, Flags::Pure);
		class_add_func("is_dict", common_types._bool, &Class::is_dict, Flags::Pure);
		class_add_func("is_enum", common_types._bool, &Class::is_enum, Flags::Pure);
		class_add_func("is_namespace", common_types._bool, &Class::is_namespace, Flags::Pure);
		class_add_func("is_interface", common_types._bool, &Class::is_interface, Flags::Pure);
		class_add_func("is_struct", common_types._bool, &Class::is_struct, Flags::Pure);
		class_add_func("is_template", common_types._bool, &Class::is_template, Flags::Pure);
		class_add_func("get_func", TypeFunctionP, &Class::get_func, Flags::Pure); // selfref
			func_add_param("name", common_types.string);
			func_add_param("return_type", TypeClassP);
			func_add_param("params", TypeClassPList);
		class_add_func("long_name", common_types.string, &Class::long_name, Flags::Pure);
		class_add_func("cname", common_types.string, &Class::cname, Flags::Pure);
			func_add_param("ns", TypeClassP);
		class_add_func(Identifier::func::Str, common_types.string, &ClassX::repr, Flags::Pure);

//	add_class(common_types.class_ref);
//		class_add_func(Identifier::Func::STR, common_types.string, &class_repr, Flags::PURE);

	add_class(common_types.function);
		class_add_element("name", common_types.string, &Function::name);
		class_add_func("long_name", common_types.string, &Function::long_name, Flags::Pure);
		class_add_func("cname", common_types.string, &Function::cname, Flags::Pure);
			func_add_param("ns", TypeClassP);
		class_add_func("signature", common_types.string, &Function::signature, Flags::Pure);
			func_add_param("ns", TypeClassP);
		class_add_element("namespace", TypeClassP, &Function::name_space);
		class_add_element("num_params", common_types.i32, &Function::num_params);
		class_add_element("var", TypeVariableRefList, &Function::var);
		class_add_element("param_type", TypeClassRefList, &Function::literal_param_type);
		class_add_element("return_type", common_types.class_ref, &Function::literal_return_type);
		class_add_func("is_static", common_types._bool, &Function::is_static, Flags::Pure);
		class_add_func("is_pure", common_types._bool, &Function::is_pure, Flags::Pure);
		class_add_func("is_mutable", common_types._bool, &Function::is_mutable, Flags::Pure);
		class_add_func("is_extern", common_types._bool, &Function::is_extern, Flags::Pure);
		class_add_func("is_selfref", common_types._bool, &Function::is_selfref, Flags::Pure);
		class_add_func("throws_exceptions", common_types._bool, &Function::throws_exceptions, Flags::Pure);
		class_add_func("is_unimplemented", common_types._bool, &Function::is_unimplemented, Flags::Pure);
		class_add_element("virtual_index", common_types.i32, &Function::virtual_index);
		class_add_element("inline_index", common_types.i32, &Function::inline_no);
		class_add_element("code", common_types.function_code_ref, &Function::address);
		class_add_func(Identifier::func::Str, common_types.string, &FunctionX::repr, Flags::Pure);

	//add_class(common_types.functionP);

	add_class(TypeVariable);
		class_add_element("name", common_types.string, &Variable::name);
		class_add_element("type", common_types.class_ref, &Variable::type);
		class_add_func("is_mutable", common_types._bool, &Variable::is_mutable, Flags::Pure);
		class_add_func("is_extern", common_types._bool, &Variable::is_extern, Flags::Pure);
		
	add_class(TypeConstant);
		class_add_element("name", common_types.string, &Constant::name);
		class_add_element("type", common_types.class_ref, &Constant::type);

	add_class(TypePackage);
		class_add_element("name", common_types.string, &Package::name);
		class_add_element("version", common_types.string, &Package::version);
		class_add_element("directory", common_types.path, &Package::directory);
		class_add_element("directory_dynamic", common_types.path, &Package::directory_dynamic);
		class_add_element("is_installed", common_types._bool, &Package::is_installed);
		class_add_element("is_internal", common_types._bool, &Package::is_internal);
		class_add_element("main_module", TypeModuleRef, &Package::main_module);
		class_add_element("auto_import", common_types._bool, &Package::auto_import);

	add_class(TypeModule);
		class_add_element("name", common_types.string, &Module::filename);
		class_add_element(Identifier::SharedCount, common_types.i32, &Module::_pointer_ref_counter);
		class_add_func("classes", TypeClassRefList, &Module::classes, Flags::Pure);
		class_add_func("functions", TypeFunctionRefList, &Module::functions, Flags::Pure);
		class_add_func("variables", TypeVariableRefList, &Module::variables, Flags::Pure);
		class_add_func("constants", TypeConstantRefList, &Module::constants, Flags::Pure);
		class_add_func("base_class", common_types.class_ref, &Module::base_class, Flags::Pure);
		//class_add_func("delete", common_types._void, &remove_module, Flags::STATIC);
		//	func_add_param("script", TypeModule);
	
	add_class(TypeStatement);
		class_add_element("name", common_types.string, &Statement::name);
		class_add_element("id", common_types.i32, &Statement::id);
		class_add_element("num_params", common_types.i32, &Statement::num_params);
	
	add_class(common_types.special_function);
		class_add_element("name", common_types.string, &SpecialFunction::name);
		class_add_element("id", common_types.i32, &SpecialFunction::id);
		class_add_element("min_params", common_types.i32, &SpecialFunction::min_params);
		class_add_element("max_params", common_types.i32, &SpecialFunction::max_params);

	add_class(TypeContext);
		class_add_element("packages", TypePackageRefList, &Context::internal_packages);
		class_add_func(Identifier::func::Delete, common_types._void, &generic_delete<Context>, Flags::Mutable);
		class_add_func("load_module", TypeModuleShared, &KabaContext::__load_module__, Flags::RaisesExceptions | Flags::Mutable);
			func_add_param("filename", common_types.path);
			func_add_param("just_analize", common_types._bool);
		class_add_func("create_module_for_source", TypeModuleShared, &KabaContext::__create_from_source__, Flags::RaisesExceptions | Flags::Mutable);
			func_add_param("source", common_types.string);
			func_add_param("just_analize", common_types._bool);
		class_add_func("execute_single_command", common_types._void, &KabaContext::__execute_single_command__, Flags::RaisesExceptions | Flags::Mutable);
			func_add_param("cmd", common_types.string);
		class_add_func("get_dynamic_type", TypeClassP, &Context::get_dynamic_type, Flags::Pure);
			func_add_param("p", common_types.pointer);
		class_add_func("create", TypeContextXfer, &Context::create, Flags::Static);

	add_func("disassemble", common_types.string, &Asm::disassemble, Flags::Static | Flags::Pure);
		func_add_param("p", common_types.pointer);
		func_add_param("length", common_types.i32);
		func_add_param("comments", common_types._bool);
	add_func("show_func", common_types._void, &show_func, Flags::Static);
		func_add_param("f", common_types.function);

	add_func("link_name", common_types.string, &function_link_name, Flags::Static);
		func_add_param("f", common_types.function);

	add_func("this_module", TypeModuleRef, &get_current_module, Flags::Static | Flags::Pure);
	add_func("this_package", TypePackageP, &get_current_package, Flags::Static | Flags::Pure);
	add_func("install_root", common_types.path, &Context::installation_root, Flags::Static | Flags::Pure);
	add_func("packages_root", common_types.path, &Context::packages_root, Flags::Static | Flags::Pure);

	add_ext_var("default_context", TypeContextRef, (void*)&default_context);
	add_ext_var("statements", TypeStatementRefList, (void*)&Statements);
	add_ext_var("special_functions", TypeSpecialFunctionRefList, (void*)&special_functions);
	add_ext_var("kaba_version", common_types.string, (void*)&Version);
}

}

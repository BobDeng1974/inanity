#include "State.hpp"
#include "Function.hpp"
#include "thunks.ipp"
#include "../../meta/Constructor.ipp"
#include "../../meta/Function.ipp"
#include "../../meta/Method.ipp"
#include "../../File.hpp"

BEGIN_INANITY_V8

State::State()
{
	// create isolate
	isolate = v8::Isolate::New();
	isolate->SetData(this);

	v8::Isolate::Scope isolateScope(isolate);

	v8::HandleScope handleScope(isolate);

	// create context
	context.Reset(isolate, v8::Context::New(isolate));
}

State::~State()
{
	{
		Scope scope(this);

		for(Classes::iterator i = classes.begin(); i != classes.end(); ++i)
			i->second.Reset();
		classes.clear();

		for(Instances::iterator i = instances.begin(); i != instances.end(); ++i)
			InternalWipeInstance(i);
		instances.clear();
	}

	context.Dispose();
	isolate->Dispose();
}

State::Scope::Scope(State* state) : state(state), handleScope(state->isolate)
{
	state->isolate->Enter();
	v8::Local<v8::Context> context = v8::Local<v8::Context>::New(state->isolate, state->context);
	context->Enter();
}

State::Scope::~Scope()
{
	v8::Local<v8::Context> context = v8::Local<v8::Context>::New(state->isolate, state->context);
	context->Exit();
	state->isolate->Exit();
}

v8::Local<v8::FunctionTemplate> State::GetClassTemplate(Meta::ClassBase* classMeta)
{
	// if class template is already created, return it
	{
		Classes::iterator i = classes.find(classMeta);
		if(i != classes.end())
			return v8::Local<v8::FunctionTemplate>::New(isolate, i->second);
	}

	// register class

	// create template for global class object
	v8::Local<v8::FunctionTemplate> classTemplate = v8::FunctionTemplate::New();

	// set a name of the class
	classTemplate->SetClassName(v8::String::New(classMeta->GetFullName()));

	// inherit from parent class
	Meta::ClassBase* parentClassMeta = classMeta->GetParent();
	if(parentClassMeta)
		classTemplate->Inherit(GetClassTemplate(parentClassMeta));

	// set constructor callback
	{
		Meta::ConstructorBase* constructor = classMeta->GetConstructor();
		if(constructor)
		{
			ConstructorExtensionBase* extension = constructor->GetV8Extension();
			classTemplate->SetCallHandler(extension->GetThunk());
		}
	}

	// add static methods to template
	const Meta::ClassBase::StaticMethods& staticMethods = classMeta->GetStaticMethods();
	for(size_t i = 0; i < staticMethods.size(); ++i)
	{
		Meta::FunctionBase* function = staticMethods[i];

		FunctionExtensionBase* extension = function->GetV8Extension();

		classTemplate->Set(
			v8::String::New(function->GetName()),
			v8::FunctionTemplate::New(extension->GetThunk())
		);
	}

	// add non-static methods to prototype
	v8::Local<v8::ObjectTemplate> prototypeTemplate = classTemplate->PrototypeTemplate();
	const Meta::ClassBase::Methods& methods = classMeta->GetMethods();
	for(size_t i = 0; i < methods.size(); ++i)
	{
		Meta::MethodBase* method = methods[i];

		MethodExtensionBase* extension = method->GetV8Extension();

		prototypeTemplate->Set(
			v8::String::New(method->GetName()),
			v8::FunctionTemplate::New(extension->GetThunk())
		);
	}

	// set one memory cell for storing 'this' pointer
	v8::Local<v8::ObjectTemplate> instanceTemplate = classTemplate->InstanceTemplate();
	instanceTemplate->SetInternalFieldCount(1);

	// parse full class name, and store the class object
	{
		const char* fullName = classMeta->GetFullName();

		// current container
		v8::Local<v8::Context> context = v8::Local<v8::Context>::New(isolate, this->context);
		v8::Local<v8::Object> container = context->Global();

		for(size_t i = 0; fullName[i]; )
		{
			size_t j;
			for(j = i + 1; fullName[j] && fullName[j] != '.'; ++j);

			// get a part of the name
			v8::Local<v8::String> pathPart = v8::String::New(fullName + i, j - i);

			// if this is a last part
			if(!fullName[j])
			{
				// put an object into container
				container->Set(pathPart, classTemplate->GetFunction());
				// exit
				break;
			}
			// else it's a transient part
			else
			{
				// get a next container, create if needed
				v8::Local<v8::Value> nextContainer = container->Get(pathPart);
				if(nextContainer->IsUndefined())
				{
					nextContainer = v8::Object::New();
					container->Set(pathPart, nextContainer);
				}

				// move to next container
				container = nextContainer->ToObject();
			}

			i = j + 1;
		}
	}

	// remember class template
	classes[classMeta].Reset(isolate, classTemplate);

	return classTemplate;
}

void State::InternalRegisterInstance(Object* object, v8::Local<v8::Object> instance)
{
#ifdef _DEBUG
	if(instances.find(object) != instances.end())
		THROW("V8 instance already registered");
#endif

	// create persistent handle to get notification when object dies
	v8::Persistent<v8::Object>& persistentObject = instances[object];
	persistentObject.Reset(isolate, instance);
	persistentObject.SetWeak(object, &InstanceWeakCallback);

	// increase reference count
	object->Reference();
}

void State::InternalUnregisterInstance(Object* object)
{
	// find an object in instances
	Instances::iterator i = instances.find(object);
	if(i != instances.end())
	{
		// object is found

		// wipe it
		InternalWipeInstance(i);

		// delete from cache
		instances.erase(i);
	}
}

void State::InternalWipeInstance(Instances::iterator i)
{
	// clear reference to it from script
	v8::Local<v8::Object> instance = v8::Local<v8::Object>::New(isolate, i->second);
	instance->SetInternalField(0, v8::Undefined());

	// destroy persistent handle
	i->second.Reset();

	// dereference object
	i->first->Dereference();
}

void State::InstanceWeakCallback(const v8::WeakCallbackData<v8::Object, Object>& data)
{
	State::GetFromIsolate(data.GetIsolate())->InternalUnregisterInstance(data.GetParameter());
}

void State::Register(Meta::ClassBase* classMeta)
{
	Scope scope(this);

	GetClassTemplate(classMeta);
}

void State::UnregisterInstance(Object* object)
{
	Scope scope(this);

	InternalUnregisterInstance(object);
}

v8::Local<v8::Object> State::ConvertObject(Meta::ClassBase* classMeta, Object* object)
{
	// check if the object is already in cache
	Instances::const_iterator i = instances.find(object);
	if(i != instances.end())
		return v8::Local<v8::Object>::New(isolate, i->second);

	// get meta
	v8::Local<v8::FunctionTemplate> classTemplate = GetClassTemplate(classMeta);

	// wrap object into external
	v8::Local<v8::Value> external = v8::External::New(object);

	// create an instance of the class
	// constructor thunk is called
	v8::Local<v8::Object> instance = classTemplate->GetFunction()->NewInstance(1, &external);

	return instance;
}

v8::Isolate* State::GetIsolate() const
{
	return isolate;
}

State* State::GetCurrent()
{
	return GetFromIsolate(v8::Isolate::GetCurrent());
}

State* State::GetFromIsolate(v8::Isolate* isolate)
{
	return (State*)isolate->GetData();
}

ptr<Script::Function> State::LoadScript(ptr<File> file)
{
	Scope scope(this);

	v8::Local<v8::Script> script = v8::Script::Compile(
		v8::String::New(
			(const char*)file->GetData(),
			file->GetSize()));

	return NEW(Function(this, script));
}

END_INANITY_V8
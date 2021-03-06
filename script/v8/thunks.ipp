#ifndef ___INANITY_SCRIPT_V8_THUNKS_IPP___
#define ___INANITY_SCRIPT_V8_THUNKS_IPP___

#include "thunks.hpp"
#include "values.hpp"
#include "State.hpp"
#include "v8lib.hpp"
#include "../../meta/Tuple.hpp"
#include "../../meta/Callable.hpp"
#include "../../Exception.hpp"
#include <sstream>

BEGIN_INANITY_V8

struct ArgGettingState
{
	const v8::FunctionCallbackInfo<v8::Value>& info;
	int index;

	ArgGettingState(const v8::FunctionCallbackInfo<v8::Value>& info, bool getThis)
	: info(info), index(getThis ? -1 : 0) {}

	template <typename ArgType>
	inline typename Value<ArgType>::ValueType Get()
	{
		return Value<ArgType>::From(index >= 0 ? info[index++] : (++index, info.This().As<v8::Value>()));
	}
};

template <typename CallerType, typename ReturnType, typename Args>
struct CallAndReturn
{
	static inline void Do(const v8::FunctionCallbackInfo<v8::Value>& info, const Args& args)
	{
		info.GetReturnValue().Set(Value<ReturnType>::To(CallerType::Call(args)));
	}
};
template <typename CallerType, typename Args>
struct CallAndReturn<CallerType, void, Args>
{
	static inline void Do(const v8::FunctionCallbackInfo<v8::Value>& info, const Args& args)
	{
		CallerType::Call(args);
	}
};

template <typename CalleeType, CalleeType callee>
struct CalleeThunk
{
	typedef typename Meta::Callable<CalleeType>::Args Args;
	typedef typename Meta::Callable<CalleeType>::ReturnType ReturnType;

	struct Helper
	{
		static inline ReturnType Call(const Args& args)
		{
			return Meta::Callable<CalleeType>::Call(callee, args);
		}
	};

	static inline void Thunk(const v8::FunctionCallbackInfo<v8::Value>& info)
	{
		try
		{
			ArgGettingState state(info, (bool)Meta::Callable<CalleeType>::isMethod);
			CallAndReturn<Helper, ReturnType, Args>::Do(info, Args(state));
		}
		catch(Exception* exception)
		{
			std::ostringstream stream;
			MakePointer(exception)->PrintStack(stream);
			v8::Isolate* isolate = info.GetIsolate();
			isolate->ThrowException(
				v8::Exception::Error(
					v8::String::NewFromUtf8(isolate, stream.str().c_str())));
		}
	}
};

/// Thunk for classes without constructor.
template <typename ClassType>
void DummyConstructorThunk(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	v8::Local<v8::Object> instance = info.This();
	v8::Isolate* isolate = info.GetIsolate();
	State* state = State::GetFromIsolate(isolate);

	// check if the constructor called from NewInstance()
	{
		v8::Local<v8::Value> param = info[0];
		if(param->IsExternal())
		{
			// put the pointer to internal field
			instance->SetInternalField(0, param);

			// we are here only for new objects
			// so register instance
			state->InternalRegisterInstance(
				(Object*)v8::External::Cast(*param)->Value(),
				Meta::MetaOf<MetaProvider,
				ClassType>(), instance
			);

			return;
		}
	}

	// if we are there, constructor was called from javascript
	isolate->ThrowException(
		v8::Exception::Error(
			v8::String::NewFromUtf8(isolate, "Class have no constructor")));
}

template <typename ClassType, typename... A>
struct ConstructorThunk
{
	typedef Meta::CallableConstructor<ClassType, A...> Callable;
	typedef typename Callable::Args Args;
	typedef typename Callable::ReturnType ReturnType;

	static inline void Thunk(const v8::FunctionCallbackInfo<v8::Value>& info)
	{
		// constructor could be called from javascript,
		// or from C++ InstanceTemplate()->NewInstance() in order
		// to wrap C++ object into javascript object.

		v8::Local<v8::Object> instance = info.This();
		v8::Isolate* isolate = info.GetIsolate();
		State* state = State::GetFromIsolate(isolate);

		// check if the constructor called from NewInstance()
		{
			v8::Local<v8::Value> param = info[0];
			if(param->IsExternal())
			{
				// put the pointer to internal field
				instance->SetInternalField(0, param);

				// we are here only for new objects
				// so register instance
				state->InternalRegisterInstance(
					(Object*)v8::External::Cast(*param)->Value(),
					Meta::MetaOf<MetaProvider, ClassType>(),
					instance
				);

				return;
			}
		}

		// if we are there, constructor was called from javascript

		// check that function is called with 'new'
		if(!info.IsConstructCall())
		{
			isolate->ThrowException(
				v8::Exception::Error(
					v8::String::NewFromUtf8(isolate, "Class constructor should be called with 'new'")));
			return;
		}

		try
		{
			// create an object
			ArgGettingState argGettingState(info, false);
			ReturnType object = Callable::Call(Args(argGettingState));

			// store a pointer to the object
			instance->SetInternalField(0, v8::External::New(isolate, object));

			// register instance in state
			state->InternalRegisterInstance(
				object,
				Meta::MetaOf<MetaProvider, ClassType>(),
				instance
			);
		}
		catch(Exception* exception)
		{
			std::ostringstream stream;
			stream << Meta::MetaOf<MetaProvider, ClassType>()->GetFullName();
			stream << " instance constructor failed:\n";
			MakePointer(exception)->PrintStack(stream);
			isolate->ThrowException(
				v8::Exception::Error(
					v8::String::NewFromUtf8(isolate, stream.str().c_str())));
		}
	}
};

END_INANITY_V8

#endif

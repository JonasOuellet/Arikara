#pragma once
#include <vector>

typedef intptr_t callbackID;

/*
	Delegate with one Parameters;
*/
template <typename Ret, typename Param0>
class Callback
{
public:
	virtual Ret invoke(Param0 param0) = 0;
};

template <typename Ret, typename Param0>
class StaticFunctionCallback : public Callback<Ret, Param0>
{
private:
	Ret(*func_)(Param0);

public:
	StaticFunctionCallback(Ret(*func)(Param0))
		: func_(func)
	{}

	virtual Ret invoke(Param0 param0)
	{
		return (*func_)(param0);
	}
};

template <typename Ret, typename Param0, typename T, typename Method>
class MethodCallback : public Callback<Ret, Param0>
{
private:
	void *object_;
	Method method_;

public:
	MethodCallback(void *object, Method method)
		: object_(object)
		, method_(method)
	{}

	virtual Ret invoke(Param0 param0)
	{
		T *obj = static_cast<T *>(object_);
		return (obj->*method_)(param0);
	}
};

template <typename Ret, typename Param0>
class Delegate
{
private:
	Callback<Ret, Param0> *callback_;

public:
	Delegate() {};

	Delegate(Ret(*func)(Param0))
		:callback_(new StaticFunctionCallback<Ret, Param0>(func))
	{}

	template <typename T, typename Method>
	Delegate(T *object, Method method)
		: callback_(new MethodCallback<Ret, Param0, T, Method>(object, method))
	{}

	~Delegate(void) { if (callback_)
                        delete callback_; 
                    }

	void addCallback(Ret(*func)(Param0)) { callback_ = new StaticFunctionCallback<Ret, Param0>(func); };
	
	template <typename T, typename Method>
	void addCallback(T *object, Method method) {
		callback_ = new MethodCallback<Ret, Param0, T, Method>(object, method);
	};

	Ret operator()(Param0 param0)
	{
		return callback_->invoke(param0);
	}
};



template <typename Ret, typename Param0>
class MultiDelegate
{
private:
	std::vector<StaticFunctionCallback<Ret, Param0>> callback_;

public:
	MultiDelegate() {};

	~MultiDelegate(void) {
		/*for (Callback<Ret, Param0>* call : callback_)
		{
			delete call;
		}
		callback_.clear();*/
	}

	callbackID addCallback(Ret(*func)(Param0)) 
	{
		StaticFunctionCallback<Ret, Param0>tmp(func);
		callback_.push_back(tmp);
		//return reinterpret_cast<callbackID>(callback_.end()-1);
		return reinterpret_cast<callbackID>(&callback_[callback_.size() - 1]);
	};

	/*template <typename T, typename Method>
	callbackID addCallback(T *object, Method method) {
		MethodCallback tmp* = new MethodCallback<Ret, Param0, T, Method>(object, method);
		callback_.push_back(tmp);
		return reinterpret_cast<callbackID>(tmp);
	};*/

	bool removeCallback(callbackID pId)
	{
		/*std::vector<StaticFunctionCallback<Ret, Param0>>::iterator iter;
		for (iter = callback_.begin(); iter != callback_.end(); iter++)
		{
			if (reinterpret_cast<callbackID>(iter) == callbackID)
			{
				callback_.erase(iter);
				return true;
			}
		}*/
		for (size_t x = 0; x < callback_.size(); x++)
		{
			if (reinterpret_cast<callbackID>(&callback_[x]) == pId)
			{
				callback_.erase(callback_.begin()+x);
				return true;
			}
		}
		return false;
	}

	void operator()(Param0 param0)
	{
		for (StaticFunctionCallback<Ret, Param0> call : callback_)
		{
			call.invoke(param0);
		}
	}
};
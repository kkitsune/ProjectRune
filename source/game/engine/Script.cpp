#include <iostream>

#include "Script.hpp"

using namespace std;

ScriptContext::ScriptContext()
{
	_ctx = duk_create_heap_default();
}

ScriptContext::~ScriptContext()
{
	duk_destroy_heap(_ctx);
}

void ScriptContext::evaluate(string const& code)
{
	if(duk_peval_string(_ctx, code.c_str()) != 0)
		cout << duk_safe_to_string(_ctx, -1) << endl;
}

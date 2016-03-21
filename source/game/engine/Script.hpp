#pragma once

#include "duktape/duktape.h"

#include <functional>
#include <string>

class ScriptContext
{
public:
	ScriptContext();

	~ScriptContext();

	ScriptContext(ScriptContext const& other) = delete;

	ScriptContext(ScriptContext&& other) = default;

	ScriptContext& operator=(ScriptContext const& other) = delete;

	ScriptContext& operator=(ScriptContext&& other) = default;

	void evaluate(std::string const& code);

private:
	duk_context* _ctx;
};

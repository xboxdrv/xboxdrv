#include <iostream>

#include "environment.hpp"
#include "namespace.hpp"
#include "symbol.hpp"

void init_environment_key(EnvironmentPtr env);

int main()
{
  EnvironmentPtr env(new Environment);

  init_environment_key(env);

  std::cout << env->lookup_symbol("xbox", "lt")->str() << std::endl;

  return 0;
}

/* EOF */
